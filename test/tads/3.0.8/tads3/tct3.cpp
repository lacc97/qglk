#ifdef RCSID
static char RCSid[] =
"$Header: d:/cvsroot/tads/tads3/tct3.cpp,v 1.5 1999/07/11 00:46:58 MJRoberts Exp $";
#endif

/* 
 *   Copyright (c) 1999, 2002 Michael J. Roberts.  All Rights Reserved.
 *   
 *   Please see the accompanying license file, LICENSE.TXT, for information
 *   on using and copying this software.  
 */
/*
Name
  tct3.cpp - TADS 3 Compiler - T3 VM Code Generator
Function
  Generate code for the T3 VM
Notes
  
Modified
  05/08/99 MJRoberts  - Creation
*/

#include <stdio.h>
#include <assert.h>

#include "t3std.h"
#include "os.h"
#include "tcprs.h"
#include "tct3.h"
#include "tcgen.h"
#include "vmtype.h"
#include "vmwrtimg.h"
#include "vmfile.h"
#include "tcmain.h"
#include "tcerr.h"
#include "vmbignum.h"
#include "vmrunsym.h"
#include "tct3unas.h"


/* ------------------------------------------------------------------------ */
/*
 *   T3 Target Code Generator class 
 */

/*
 *   initialize the code generator 
 */
CTcGenTarg::CTcGenTarg()
{
    /* 
     *   we haven't written any instructions yet - fill the pipe with
     *   no-op instructions so that we don't think we can combine the
     *   first instruction with anything previous 
     */
    last_op_ = OPC_NOP;
    second_last_op_ = OPC_NOP;

    /* 
     *   we haven't seen any strings or lists yet - set the initial
     *   maximum lengths to zero 
     */
    max_str_len_ = 0;
    max_list_cnt_ = 0;

    /* we haven't generated any code yet */
    max_bytecode_len_ = 0;

    /* there are no metaclasses defined yet */
    meta_head_ = meta_tail_ = 0;
    meta_cnt_ = 0;

    /* no function sets defined yet */
    fnset_head_ = fnset_tail_ = 0;
    fnset_cnt_ = 0;

    /* 
     *   Add the built-in metaclass entries.  The order of these entries
     *   is fixed to match the TCT3_METAID_xxx constants - if this order
     *   changes, those constants must change to match.  
     */
    add_meta("tads-object");
    add_meta("list");
    add_meta("dictionary2/030000");
    add_meta("grammar-production/030000");
    add_meta("vector");
    add_meta("anon-func-ptr");
    add_meta("int-class-mod/030000");

    /* start at the first valid property ID */
    next_prop_ = 1;

    /* start at the first valid object ID */
    next_obj_ = 1;

    /* allocate an initial sort buffer */
    sort_buf_size_ = 4096;
    sort_buf_ = (char *)t3malloc(sort_buf_size_);

    /* not in a constructor */
    in_constructor_ = FALSE;

    /* no debug line record pointers yet */
    debug_line_cnt_ = 0;
    debug_line_head_ = debug_line_tail_ = 0;

    /* normal (non-debug) evaluation mode */
    eval_for_debug_ = FALSE;
    speculative_ = FALSE;
    debug_stack_level_ = 0;
}

/*
 *   delete the code generator 
 */
CTcGenTarg::~CTcGenTarg()
{
    /* delete all of the metaclass list entries */
    while (meta_head_ != 0)
    {
        tc_meta_entry *nxt;
        
        /* remember the next item */
        nxt = meta_head_->nxt;

        /* delete this item */
        t3free(meta_head_);

        /* move on */
        meta_head_ = nxt;
    }

    /* delete all of the function set list entries */
    while (fnset_head_ != 0)
    {
        tc_fnset_entry *nxt;

        /* remember the next item */
        nxt = fnset_head_->nxt;

        /* delete this item */
        t3free(fnset_head_);

        /* move on */
        fnset_head_ = nxt;
    }

    /* delete our sort buffer */
    t3free(sort_buf_);

    /* delete the debug line record pointers */
    while (debug_line_head_ != 0)
    {
        tct3_debug_line_page *nxt;

        /* remember the next one */
        nxt = debug_line_head_->nxt;

        /* delete this one */
        delete debug_line_head_;

        /* move on */
        debug_line_head_ = nxt;
    }
}

/*
 *   Add an entry to the metaclass dependency table 
 */
int CTcGenTarg::add_meta(const char *nm, size_t len,
                         CTcSymMetaclass *sym)
{
    tc_meta_entry *ent;
    size_t extra_len;
    const char *extra_ptr;
    const char *p;
    size_t rem;

    /* 
     *   if the name string doesn't contain a slash, allocate enough space
     *   to add an implied version suffix of "/000000" 
     */
    for (p = nm, rem = len ; rem != 0 && *p != '/' ; ++p, --rem) ;
    if (rem == 0)
    {
        /* we didn't find a version suffix - add space for one */
        extra_len = 7;
        extra_ptr = "/000000";
    }
    else
    {
        /* 
         *   there's already a version suffix - but make sure we have
         *   space for a six-character string 
         */
        if (rem < 7)
        {
            /* add zeroes to pad out to a six-place version string */
            extra_len = 7 - rem;
            extra_ptr = "000000";
        }
        else
        {
            /* we need nothing extra */
            extra_len = 0;
            extra_ptr = 0;
        }
    }
    
    /* allocate a new entry for the item */
    ent = (tc_meta_entry *)t3malloc(sizeof(tc_meta_entry) + len + extra_len);
    if (ent == 0)
        err_throw(TCERR_CODEGEN_NO_MEM);

    /* copy the name into the entry */
    memcpy(ent->nm, nm, len);

    /* add any extra version suffix information */
    if (extra_len != 0)
        memcpy(ent->nm + len, extra_ptr, extra_len);

    /* null-terminate the name string in the entry */
    ent->nm[len + extra_len] = '\0';

    /* remember the symbol */
    ent->sym = sym;

    /* link the entry in at the end of the list */
    ent->nxt = 0;
    if (meta_tail_ != 0)
        meta_tail_->nxt = ent;
    else
        meta_head_ = ent;
    meta_tail_ = ent;

    /* count the entry, returning the index of the entry in the list */
    return meta_cnt_++;
}

/*
 *   Find a metaclass index given the global identifier.
 */
tc_meta_entry *CTcGenTarg::find_meta_entry(const char *nm, size_t len,
                                           int update_vsn, int *entry_idx)
{
    tc_meta_entry *ent;
    int idx;
    size_t name_len;
    const char *p;
    const char *vsn;
    size_t vsn_len;
    size_t rem;

    /* find the version suffix, if any */
    for (rem = len, p = nm ; rem != 0 && *p != '/' ; --rem, ++p) ;

    /* note the length of the name portion (up to the '/') */
    name_len = len - rem;

    /* note the version string, if there is one */
    if (rem != 0)
    {
        vsn = p + 1;
        vsn_len = rem - 1;
    }
    else
    {
        vsn = 0;
        vsn_len = 0;
    }

    /* search the existing entries */
    for (idx = 0, ent = meta_head_ ; ent != 0 ; ent = ent->nxt, ++idx)
    {
        size_t ent_name_len;
        char *ent_vsn;

        /* find the version suffix in the entry */
        for (ent_vsn = ent->nm ; *ent_vsn != '\0' && *ent_vsn != '/' ;
             ++ent_vsn) ;

        /* the name is the part up to the '/' */
        ent_name_len = ent_vsn - ent->nm;

        /* note the length of the name and the version suffix */
        if (*ent_vsn == '/')
        {
            /* the version is what follows the '/' */
            ++ent_vsn;
        }
        else
        {
            /* there is no version suffix */
            ent_vsn = 0;
        }

        /* if this is the one, return it */
        if (ent_name_len == name_len && memcmp(ent->nm, nm, name_len) == 0)
        {
            /* 
             *   if this version number is higher than the version number
             *   we previously recorded, remember the new, higher version
             *   number 
             */
            if (update_vsn && ent_vsn != 0 && strlen(ent_vsn) == vsn_len
                && memcmp(vsn, ent_vsn, vsn_len) > 0)
            {
                /* store the new version string */
                memcpy(ent_vsn, vsn, vsn_len);
            }

            /* tell the caller the index, and return the entry */
            *entry_idx = idx;
            return ent;
        }
    }

    /* we didn't find it */
    return 0;
}


/*
 *   Find a metaclass symbol given the global identifier 
 */
class CTcSymMetaclass *CTcGenTarg::find_meta_sym(const char *nm, size_t len)
{
    tc_meta_entry *ent;
    int idx;

    /* find the entry */
    ent = find_meta_entry(nm, len, TRUE, &idx);

    /* 
     *   if we found it, return the associated metaclass symbol; if
     *   there's no entry, there's no symbol 
     */
    if (ent != 0)
        return ent->sym;
    else
        return 0;
}


/*
 *   Find or add a metaclass entry 
 */
int CTcGenTarg::find_or_add_meta(const char *nm, size_t len,
                                 CTcSymMetaclass *sym)
{
    tc_meta_entry *ent;
    int idx;

    /* find the entry */
    ent = find_meta_entry(nm, len, TRUE, &idx);

    /* if we found it, return the index */
    if (ent != 0)
    {
        /* 
         *   found it - if it didn't already have a symbol mapping, use
         *   the new symbol; if there is a symbol in the table entry
         *   already, however, do not change it 
         */
        if (ent->sym == 0)
            ent->sym = sym;

        /* return the index */
        return idx;
    }

    /* we didn't find an existing entry - add a new one */
    return add_meta(nm, len, sym);
}

/*
 *   Get the symbol for a given metaclass dependency table entry 
 */
CTcSymMetaclass *CTcGenTarg::get_meta_sym(int meta_idx)
{
    tc_meta_entry *ent;

    /* find the list entry at the given index */
    for (ent = meta_head_ ; ent != 0 && meta_idx != 0 ;
         ent = ent->nxt, --meta_idx) ;

    /* if we didn't find the entry, do nothing */
    if (ent == 0 || meta_idx != 0)
        return 0;

    /* return this entry's symbol */
    return ent->sym;
}

/*
 *   Get the external name for the given metaclass index
 */
const char *CTcGenTarg::get_meta_name(int meta_idx) const
{
    tc_meta_entry *ent;

    /* find the list entry at the given index */
    for (ent = meta_head_ ; ent != 0 && meta_idx != 0 ;
         ent = ent->nxt, --meta_idx) ;

    /* if we didn't find the entry, do nothing */
    if (ent == 0 || meta_idx != 0)
        return 0;

    /* return this entry's external name */
    return ent->nm;
}

/*
 *   Set the symbol for a given metaclass dependency table entry 
 */
void CTcGenTarg::set_meta_sym(int meta_idx, class CTcSymMetaclass *sym)
{
    tc_meta_entry *ent;
    
    /* find the list entry at the given index */
    for (ent = meta_head_ ; ent != 0 && meta_idx != 0 ;
         ent = ent->nxt, --meta_idx) ;

    /* if we didn't find the entry, do nothing */
    if (ent == 0 || meta_idx != 0)
        return;

    /* set this entry's symbol */
    ent->sym = sym;
}

/*
 *   Add an entry to the function set dependency table 
 */
int CTcGenTarg::add_fnset(const char *nm, size_t len)
{
    tc_fnset_entry *ent;
    const char *sl;
    size_t sl_len;
    size_t idx;

    /* find the version part of the new name, if present */
    for (sl = nm, sl_len = len ; sl_len != 0 && *sl != '/' ; ++sl, --sl_len) ;

    /* look for an existing entry with the same name prefix */
    for (idx = 0, ent = fnset_head_ ; ent != 0 ; ent = ent->nxt, ++idx)
    {
        char *ent_sl;
        
        /* find the version part of this entry's name, if present */
        for (ent_sl = ent->nm ; *ent_sl != '\0' && *ent_sl != '/' ;
             ++ent_sl) ;

        /* check to see if the prefixes match */
        if (ent_sl - ent->nm == sl - nm
            && memcmp(ent->nm, nm, sl - nm) == 0)
        {
            /*
             *   This one matches.  Keep the one with the higher version
             *   number.  If one has a version number and the other doesn't,
             *   keep the one with the version number.  
             */
            if (*ent_sl == '/' && sl_len != 0)
            {
                /* 
                 *   Both have version numbers - keep the higher version.
                 *   Limit the version length to 6 characters plus the
                 *   slash. 
                 */
                if (sl_len > 7)
                    sl_len = 7;

                /* check if the new version number is higher */
                if (memcmp(sl, ent_sl, sl_len) > 0)
                {
                    /* the new one is higher - copy it over the old one */
                    memcpy(ent_sl, sl, sl_len);
                }

                /* 
                 *   in any case, we're going to keep the existing entry, so
                 *   we're done - just return the existing entry's index 
                 */
                return idx;
            }
            else if (*ent_sl == '/')
            {
                /* 
                 *   only the old entry has a version number, so keep it and
                 *   ignore the new definition - this means we're done, so
                 *   just return the existing item's index 
                 */
                return idx;
            }
            else
            {
                /* 
                 *   Only the new entry has a version number, so store the
                 *   new version number.  To do this, simply copy the new
                 *   entry over the old entry, but limit the version number
                 *   field to 7 characters including the slash.  
                 */
                if (sl_len > 7)
                    len -= (sl_len - 7);

                /* copy the new value */
                memcpy(ent->nm, nm, len);

                /* done - return the existing item's index */
                return idx;
            }
        }
    }

    /* 
     *   Allocate a new entry for the item.  Always allocate space for a
     *   version number, even if the entry doesn't have a version number -
     *   if the part from the slash on is 7 characters or more, add nothing,
     *   else add enough to pad it out to seven characters.  
     */
    ent = (tc_fnset_entry *)t3malloc(sizeof(tc_fnset_entry) + len
                                     + (sl_len < 7 ? 7 - sl_len : 0));
    if (ent == 0)
        err_throw(TCERR_CODEGEN_NO_MEM);

    /* copy the name into the entry */
    memcpy(ent->nm, nm, len);
    ent->nm[len] = '\0';
    
    /* link the entry in at the end of the list */
    ent->nxt = 0;
    if (fnset_tail_ != 0)
        fnset_tail_->nxt = ent;
    else
        fnset_head_ = ent;
    fnset_tail_ = ent;

    /* count the entry, returning the index of the entry in the list */
    return fnset_cnt_++;
}

/*
 *   get a function set's name given its index 
 */
const char *CTcGenTarg::get_fnset_name(int idx) const
{
    tc_fnset_entry *ent;

    /* scan the linked list to find the given index */
    for (ent = fnset_head_ ; idx != 0 && ent != 0 ; ent = ent->nxt, --idx) ;

    /* return the one we found */
    return ent->nm;
}

/*
 *   Determine if we can skip an opcode because it is unreachable from the
 *   previous instruction.  
 */
int CTcGenTarg::can_skip_op()
{
    /* 
     *   if the previous instruction was a return or throw of some kind,
     *   we can skip any subsequent opcodes until a label is defined 
     */
    switch(last_op_)
    {
    case OPC_RET:
    case OPC_RETVAL:
    case OPC_RETTRUE:
    case OPC_RETNIL:
    case OPC_THROW:
    case OPC_JMP:
    case OPC_LRET:
        /* it's a return, throw, or jump - this new op is unreachable */
        return TRUE;

    default:
        /* this new op is reachable */
        return FALSE;
    }
}

/*
 *   Remove the last JMP instruction 
 */
void CTcGenTarg::remove_last_jmp()
{
    /* a JMP instruction is three bytes long, so back up three bytes */
    G_cs->dec_ofs(3);
}

/*
 *   Add a line record 
 */
void CTcGenTarg::add_line_rec(CTcTokFileDesc *file, long linenum)
{
    /* include line records only in debug mode */
    if (G_debug)
    {
        /* 
         *   clear the peephole, to ensure that the line boundary isn't
         *   blurred by code optimization 
         */
        clear_peephole();

        /* add the record to the code stream */
        G_cs->add_line_rec(file, linenum);
    }
}

/*
 *   Write an opcode to the output stream.  We'll watch for certain
 *   combinations of opcodes being generated, and apply peephole
 *   optimization when we see sequences that can be collapsed to more
 *   efficient single instructions.  
 */
void CTcGenTarg::write_op(uchar opc)
{
    int prv_len;
    int op_len;

    /* write the new opcode byte to the output stream */
    G_cs->write((char)opc);

    /* we've only written one byte so far for the current instruction */
    op_len = 1;

    /* presume the previous instruction length is just one byte */
    prv_len = 1;

    /* 
     *   check for pairs of instructions that we can reduce to more
     *   efficient single instructions 
     */
try_combine:
    switch(opc)
    {
    case OPC_JF:
        /* 
         *   if the last instruction was a comparison, we can use the
         *   opposite compare-and-jump instruction 
         */
        switch(last_op_)
        {
        case OPC_NOT:
            /* invert the sense of the test */
            opc = OPC_JT;
            goto combine;

        combine:
            /* 
             *   delete the new opcode we wrote, since we're going to combine
             *   it with the preceding opcode 
             */
            G_cs->dec_ofs(op_len);

            /* overwrite the preceding opcode with the new combined opcode */
            G_cs->write_at(G_cs->get_ofs() - prv_len, opc);

            /* roll back our internal peephole */
            last_op_ = second_last_op_;
            second_last_op_ = OPC_NOP;

            /* 
             *   we've deleted our own opcode, so the current (most recent)
             *   instruction in the output stream has the length of the
             *   current opcode 
             */
            op_len = prv_len;

            /* presume the previous opcode is one byte again */
            prv_len = 1;

            /* 
             *   go back for another try, since we may be able to do a
             *   three-way combination (for example, GT/NOT/JT would
             *   change to GT/JF, which would in turn change to JLE) 
             */
            goto try_combine;

        case OPC_EQ:
            opc = OPC_JNE;
            goto combine;

        case OPC_NE:
            opc = OPC_JE;
            goto combine;
            
        case OPC_LT:
            opc = OPC_JGE;
            goto combine;
            
        case OPC_LE:
            opc = OPC_JGT;
            goto combine;
            
        case OPC_GT:
            opc = OPC_JLE;
            goto combine;
            
        case OPC_GE:
            opc = OPC_JLT;
            goto combine;

        case OPC_GETR0:
            opc = OPC_JR0F;
            goto combine;
        }
        break;

    case OPC_JE:
        /* 
         *   if we just pushed nil, convert the PUSHNIL + JE to JNIL, since
         *   we simply want to jump if a value is nil 
         */
        if (last_op_ == OPC_PUSHNIL)
        {
            /* convert it to a jump-if-nil */
            opc = OPC_JNIL;
            goto combine;
        }
        break;

    case OPC_JNE:
        /* if we just pushed nil, convert to JNOTNIL */
        if (last_op_ == OPC_PUSHNIL)
        {
            /* convert to jump-if-not-nil */
            opc = OPC_JNOTNIL;
            goto combine;
        }
        break;

    case OPC_JT:
        /* 
         *   if the last instruction was a comparison, we can use a
         *   compare-and-jump instruction 
         */
        switch(last_op_)
        {
        case OPC_NOT:
            /* invert the sense of the test */
            opc = OPC_JF;
            goto combine;
            
        case OPC_EQ:
            opc = OPC_JE;
            goto combine;

        case OPC_NE:
            opc = OPC_JNE;
            goto combine;

        case OPC_LT:
            opc = OPC_JLT;
            goto combine;

        case OPC_LE:
            opc = OPC_JLE;
            goto combine;

        case OPC_GT:
            opc = OPC_JGT;
            goto combine;

        case OPC_GE:
            opc = OPC_JGE;
            goto combine;

        case OPC_GETR0:
            opc = OPC_JR0T;
            goto combine;
        }
        break;

    case OPC_NOT:
        /* 
         *   If the previous instruction was a comparison test of some
         *   kind, we can invert the sense of the test.  If the previous
         *   instruction was a BOOLIZE op, we can eliminate it entirely,
         *   because the NOT will perform the same conversion before
         *   negating the value.  If the previous was a NOT, we're
         *   inverting an inversion; we can simply perform a single
         *   BOOLIZE to get the same effect.  
         */
        switch(last_op_)
        {
        case OPC_EQ:
            opc = OPC_NE;
            goto combine;

        case OPC_NE:
            opc = OPC_EQ;
            goto combine;

        case OPC_GT:
            opc = OPC_LE;
            goto combine;

        case OPC_GE:
            opc = OPC_LT;
            goto combine;

        case OPC_LT:
            opc = OPC_GE;
            goto combine;

        case OPC_LE:
            opc = OPC_GT;
            goto combine;

        case OPC_BOOLIZE:
            opc = OPC_NOT;
            goto combine;

        case OPC_NOT:
            opc = OPC_BOOLIZE;
            goto combine;
        }
        break;

    case OPC_RET:
        /* 
         *   If we're writing a return instruction immediately after
         *   another return instruction, we can skip the additional
         *   instruction, since it will never be reached.  This case
         *   typically arises only when we generate the catch-all RET
         *   instruction at the end of a function. 
         */
        switch(last_op_)
        {
        case OPC_RET:
        case OPC_RETVAL:
        case OPC_RETNIL:
        case OPC_RETTRUE:
            /* simply suppress this additional RET instruction */
            return;
        }
        break;

    case OPC_RETNIL:
        /* we don't need to write two RETNIL's in a row */
        if (last_op_ == OPC_RETNIL)
            return;
        break;

    case OPC_RETTRUE:
        /* we don't need to write two RETTRUE's in a row */
        if (last_op_ == OPC_RETTRUE)
            return;
        break;

    case OPC_RETVAL:
        /* check the last opcode */
        switch(last_op_)
        {
        case OPC_GETR0:
            /* 
             *   if we just pushed R0 onto the stack, we can compress the
             *   GETR0 + RETVAL sequence into a simple RET, since RET leaves
             *   the R0 value unchanged 
             */
            opc = OPC_RET;
            goto combine;

        case OPC_PUSHNIL:
            /* PUSHNIL + RET can be converted to RETNIL */
            opc = OPC_RETNIL;
            goto combine;

        case OPC_PUSHTRUE:
            /* PUSHTRUE + RET can be converted to RETTRUE */
            opc = OPC_RETTRUE;
            goto combine;
        }
        break;

    case OPC_SETLCL1:
        /* we can combine this with a preceding GETR0 */
        if (last_op_ == OPC_GETR0)
        {
            /* generate a combined SETLCL1R0 */
            opc = OPC_SETLCL1R0;
            goto combine;
        }
        break;

    case OPC_GETPROP:
        /* check the previous instruction for combination possibilities */
        switch(last_op_)
        {
        case OPC_GETLCL1:
            /* get property of one-byte-addressable local */
            opc = OPC_GETPROPLCL1;

            /* overwrite the preceding two-byte instruction */
            prv_len = 2;
            goto combine;

        case OPC_GETR0:
            /* get property of R0 */
            opc = OPC_GETPROPR0;
            goto combine;
        }
        break;

    case OPC_CALLPROP:
        /* check the previous instruction */
        switch(last_op_)
        {
        case OPC_GETR0:
            /* call property of R0 */
            opc = OPC_CALLPROPR0;
            goto combine;
        }
        break;

    case OPC_INDEX:
        /* we can combine small integer constants with INDEX */
        switch(last_op_)
        {
        case OPC_PUSH_0:
        case OPC_PUSH_1:
            /* 
             *   We can combine these into IDXINT8, but we must write an
             *   extra byte for the index value.  Go back and plug in the
             *   extra index value byte, and add another byte at the end of
             *   the stream to compensate for the insertion.  (We're just
             *   going to remove and overwrite everything after the inserted
             *   byte, so don't bother actually fixing up that part with real
             *   data; we merely need to make sure we have the right number
             *   of bytes in the stream.)  
             */
            G_cs->write_at(G_cs->get_ofs() - 1,
                           last_op_ == OPC_PUSH_0 ? 0 : 1);
            G_cs->write(0);

            /* combine the instructions */
            opc = OPC_IDXINT8;
            prv_len = 2;
            goto combine;

        case OPC_PUSHINT8:
            /* combine the PUSHINT8 + INDEX into IDXINT8 */
            opc = OPC_IDXINT8;
            prv_len = 2;
            goto combine;
        }
        break;

    case OPC_IDXINT8:
        /* we can replace GETLCL1 + IDXINT8 with IDXLCL1INT8 */
        if (last_op_ == OPC_GETLCL1)
        {
            uchar idx;

            /* rewrite the GETLCL1 to add the index operand */
            idx = G_cs->get_byte_at(G_cs->get_ofs() - 1);
            G_cs->write_at(G_cs->get_ofs() - 2, idx);

            /* add another byte to compensate for the insertion */
            G_cs->write(0);

            /* go back and combine into what's now a three-byte opcode */
            opc = OPC_IDXLCL1INT8;
            prv_len = 3;
            goto combine;
        }
        break;

    case OPC_SETIND:
        /* we can replace SETLCL1 + <small int> + SETIND with SETINDLCL1I8 */
        if (second_last_op_ == OPC_SETLCL1)
        {
            uchar idx;

            /* check the middle opcode */
            switch(last_op_)
            {
            case OPC_PUSHINT8:
                /* 
                 *   go back and put the index value in the right spot in the
                 *   third instruction back 
                 */
                idx = G_cs->get_byte_at(G_cs->get_ofs() - 2);
                G_cs->write_at(G_cs->get_ofs() - 3, idx);

                /* 
                 *   Go back and combine into what's now a 3-byte
                 *   instruction: we'll remove the SETIND and the
                 *   PUSHINT8+val, for three bytes removed, but we're adding
                 *   one byte, so we have a net current opcode length (to
                 *   remove) of two bytes.  Since we're combining three
                 *   instructions into one, we're losing our second-to-last
                 *   opcode.  
                 */
                opc = OPC_SETINDLCL1I8;
                second_last_op_ = OPC_NOP;
                op_len = 2;
                prv_len = 3;
                goto combine;

            case OPC_PUSH_0:
                idx = 0;
                goto combine_setind;

            case OPC_PUSH_1:
                idx = 1;

            combine_setind:
                /* go back and add the index value */
                G_cs->write_at(G_cs->get_ofs() - 2, idx);

                /* 
                 *   go back and combine the instructions - we're removing a
                 *   net of one byte, since we're removing two one-byte
                 *   instructions and extending the old 2-byte instruction
                 *   into a 3-byte instruction 
                 */
                opc = OPC_SETINDLCL1I8;
                second_last_op_ = OPC_NOP;
                op_len = 1;
                prv_len = 3;
                goto combine;
            }
        }
        break;

    default:
        /* write this instruction as-is */
        break;
    }
    
    /* remember the last opcode we wrote */
    second_last_op_ = last_op_;
    last_op_ = opc;
}

/*
 *   Write a CALLPROP instruction, combining with preceding opcodes if
 *   possible.  
 */
void CTcGenTarg::write_callprop(int argc, int varargs, vm_prop_id_t prop)
{
    /* 
     *   if the previous instruction was GETLCL1, combine it with the
     *   CALLPROP to form a single CALLPROPLCL1 instruction 
     */
    if (last_op_ == OPC_GETLCL1)
    {
        uchar lcl;

        /* get the local variable ID from the GETLCL1 instruction */
        lcl = G_cs->get_byte_at(G_cs->get_ofs() - 1);

        /* back up and delete the GETLCL1 instruction */
        G_cs->dec_ofs(2);

        /* roll back the peephole for the instruction deletion */
        last_op_ = second_last_op_;
        second_last_op_ = OPC_NOP;

        /* write the varargs modifier if appropriate */
        if (varargs)
            write_op(OPC_VARARGC);

        /* write the CALLPROPLCL1 */
        write_op(OPC_CALLPROPLCL1);
        G_cs->write((char)argc);
        G_cs->write(lcl);
        G_cs->write_prop_id(prop);
    }
    else
    {
        /* generate the varargs modifier if appropriate */
        if (varargs)
            write_op(OPC_VARARGC);

        /* we have arguments - generate a CALLPROP */
        write_op(OPC_CALLPROP);
        G_cs->write((char)argc);
        G_cs->write_prop_id(prop);
    }

    /* callprop removes arguments and the object */
    note_pop(argc + 1);
}

/*
 *   Note a string's length 
 */
void CTcGenTarg::note_str(size_t len)
{
    /* if it's the longest so far, remember it */
    if (len > max_str_len_)
    {
        /* 
         *   flag an warning the length plus overhead would exceed 32k
         *   (only do this the first time we cross this limit) 
         */
        if (len > (32*1024 - VMB_LEN)
            && max_str_len_ <= (32*1024 - VMB_LEN))
            G_tok->log_warning(TCERR_CONST_POOL_OVER_32K);

        /* remember the length */
        max_str_len_ = len;
    }
}

/*
 *   note a list's length 
 */
void CTcGenTarg::note_list(size_t element_count)
{
    /* if it's the longest list so far, remember it */
    if (element_count > max_list_cnt_)
    {
        /* flag a warning if the stored length would be over 32k */
        if (element_count > ((32*1024 - VMB_LEN) / VMB_DATAHOLDER)
            && max_list_cnt_ <= ((32*1024 - VMB_LEN) / VMB_DATAHOLDER))
            G_tok->log_warning(TCERR_CONST_POOL_OVER_32K);

        /* remember the length */
        max_list_cnt_ = element_count;
    }
}

/*
 *   Note a bytecode block length 
 */
void CTcGenTarg::note_bytecode(ulong len)
{
    /* if it's the longest bytecode block yet, remember it */
    if (len > max_bytecode_len_)
    {
        /* flag a warning the first time we go over 32k */
        if (len > 32*1024 && max_bytecode_len_ < 32*1024)
            G_tok->log_warning(TCERR_CODE_POOL_OVER_32K);
        
        /* remember the new length */
        max_bytecode_len_ = len;
    }
}

/*
 *   Add a string to the constant pool 
 */
void CTcGenTarg::add_const_str(const char *str, size_t len,
                               CTcDataStream *ds, ulong ofs)
{
    CTcStreamAnchor *anchor;
    
    /* 
     *   Add an anchor for the item, and add a fixup for the reference
     *   from ds@ofs to the item. 
     */
    anchor = G_ds->add_anchor(0, 0);
    CTcAbsFixup::add_abs_fixup(anchor->fixup_list_head_, ds, ofs);

    /* write the length prefix */
    G_ds->write2(len);

    /* write the string bytes */
    G_ds->write(str, len);

    /* note the length of the string stored */
    note_str(len);
}

/*
 *   Add a list to the constant pool 
 */
void CTcGenTarg::add_const_list(CTPNList *lst,
                                CTcDataStream *ds, ulong ofs)
{
    int i;
    CTPNListEle *cur;
    ulong dst;
    CTcStreamAnchor *anchor;

    /* 
     *   Add an anchor for the item, and add a fixup for the reference
     *   from ds@ofs to the item.  
     */
    anchor = G_ds->add_anchor(0, 0);
    CTcAbsFixup::add_abs_fixup(anchor->fixup_list_head_, ds, ofs);

    /* 
     *   Reserve space for the list.  We need to do this first, because
     *   the list might contain elements which themselves must be written
     *   to the data stream; we must therefore reserve space for the
     *   entire list before we start writing its elements. 
     */
    dst = G_ds->reserve(2 + lst->get_count()*VMB_DATAHOLDER);

    /* set the length prefix */
    G_ds->write2_at(dst, lst->get_count());
    dst += 2;

    /* store the elements */
    for (i = 0, cur = lst->get_head() ; cur != 0 ;
         ++i, cur = cur->get_next(), dst += VMB_DATAHOLDER)
    {
        CTcConstVal *ele;

        /* get this element */
        ele = cur->get_expr()->get_const_val();

        /* write it to the element buffer */
        write_const_as_dh(G_ds, dst, ele);
    }

    /* make sure the list wasn't corrupted */
    if (i != lst->get_count())
        G_tok->throw_internal_error(TCERR_CORRUPT_LIST);

    /* note the number of elements in the list */
    note_list(lst->get_count());
}

/*
 *   Generate a BigNumber object 
 */
vm_obj_id_t CTcGenTarg::gen_bignum_obj(const char *txt, size_t len)
{
    vm_obj_id_t id;
    long size_ofs;
    long end_ofs;
    long num_ofs;
    CTcDataStream *str = G_bignum_stream;
    int exp;
    int decpt;
    utf8_ptr p;
    size_t rem;
    int dig[2];
    char dig_idx;
    int sig;
    size_t tot_digits;
    size_t prec;
    int neg;
    int val_zero;
    uchar flags;

    /* generate a new object ID for the BigNumber */
    id = new_obj_id();

    /* 
     *   add the object ID to the non-symbol object list - this is
     *   necessary to ensure that the object ID is fixed up during linking 
     */
    G_prs->add_nonsym_obj(id);

    /* 
     *   generate the object data to the BigNumber data stream
     */

    /* 
     *   write the OBJS header - object ID plus byte count for
     *   metaclass-specific data 
     */
    str->write_obj_id(id);
    size_ofs = str->get_ofs();
    str->write2(0);

    /* 
     *   write the metaclass-specific data for the BigNumber metaclass 
     */

    /* remember where the number starts */
    num_ofs = str->get_ofs();

    /* write placeholders for the precision, exponent, and flags */
    str->write2(0);
    str->write2(0);
    str->write(0);

    /* start at the beginning of the number's text */
    p.set((char *)txt);
    rem = len;

    /* presume the value won't be zero */
    val_zero = FALSE;

    /* check for leading sign indicators */
    for (neg = FALSE ; rem != 0 ; p.inc(&rem))
    {
        /* if it's a sign, note it and keep scanning */
        if (p.getch() == '-')
        {
            /* negative sign - note it and keep going */
            neg = !neg;
        }
        else if (p.getch() == '+')
        {
            /* positive sign - ignore it and keep going */
        }
        else
        {
            /* not a sign character - stop scanning for signs */
            break;
        }
    }

    /* scan the digits of the number */
    for (exp = 0, sig = FALSE, decpt = FALSE, prec = 0, tot_digits = 0,
         dig_idx = 0 ; rem != 0 ; p.inc(&rem))
    {
        wchar_t ch;
        
        /* get this character */
        ch = p.getch();

        /* see what we have */
        if (is_digit(ch))
        {
            /* 
             *   if it's non-zero, it's definitely significant; otherwise,
             *   it's significant only if we've seen a significant digit
             *   already 
             */
            if (ch != '0')
                sig = TRUE;

            /* count it in the total digits whether or not its significant */
            ++tot_digits;
            
            /* if the digit is significant, add it to the number */
            if (sig)
            {
                /* add another digit to our buffer */
                dig[dig_idx++] = value_of_digit(ch);

                /* count the precision */
                ++prec;

                /* 
                 *   if we haven't found the decimal point yet, count the
                 *   exponent change
                 */
                if (!decpt)
                    ++exp;
                
                /* 
                 *   if we have two digits now, write out another byte of
                 *   the number 
                 */
                if (dig_idx == 2)
                {
                    /* write out this digit pair */
                    str->write((char)((dig[0] << 4) + dig[1]));

                    /* the buffer is now empty */
                    dig_idx = 0;
                }
            }
            else if (decpt)
            {
                /* 
                 *   we have a leading insignificant zero following the
                 *   decimal point - decrease the exponent 
                 */
                --exp;
            }
        }
        else if (!decpt && ch == '.')
        {
            /* we've found the decimal point - note it */
            decpt = TRUE;
        }
        else if (ch == 'e' || ch == 'E')
        {
            int neg_exp = FALSE;
            long acc;
            
            /* we've found our exponent - check for a sign */
            p.inc(&rem);
            if (rem != 0)
            {
                if (p.getch() == '-')
                {
                    /* note the sign and skip the '-' */
                    neg_exp = TRUE;
                    p.inc(&rem);
                }
                else if (p.getch() == '+')
                {
                    /* skip the '+' */
                    p.inc(&rem);
                }
            }

            /* scan the digits */
            for (acc = 0 ; rem != 0 ; p.inc(&rem))
            {
                long new_acc;
                
                /* if this isn't a digit, we're done */
                if (!is_digit(p.getch()))
                    break;

                /* add in this digit */
                new_acc = acc*10 + value_of_digit(p.getch());
                if ((!neg_exp && new_acc > 32767)
                    || (neg_exp && new_acc > 32768))
                    break;

                /* set the new accumulator */
                acc = new_acc;
            }

            /* set the sign */
            if (neg_exp)
                acc = -acc;

            /* 
             *   add this exponent to the exponent we derived for the
             *   number itself 
             */
            exp = (int)(exp + acc);

            /* 
             *   since we scanned the string in our own loop, make sure we
             *   haven't reached the end of the buffer - if we have, we're
             *   done with the outer loop now 
             */
            if (rem == 0)
                break;
        }
        else
        {
            /* 
             *   anything else is invalid, so we've reached the end of the
             *   number - stop scanning
             */
            break;
        }
    }

    /* if we have a pending digit, write it out */
    if (dig_idx == 1)
        str->write((char)(dig[0] << 4));

    /* 
     *   if we had no significant digits, the number is all zeroes, so in
     *   this special case treat all of the zeroes as significant 
     */
    if (prec == 0 && tot_digits != 0)
    {
        /* note that the value is zero */
        val_zero = TRUE;
        
        /* use the zeroes as significant digits */
        prec = tot_digits;
        
        /* write out the zeroes */
        for ( ; tot_digits > 1 ; tot_digits -= 2)
            str->write(0);
        if (tot_digits > 0)
            str->write(0);

        /* 
         *   the exponent for the value zero is always 1 (this is a
         *   normalization rule) 
         */
        exp = 1;
    }

    /* construct the flags */
    flags = 0;
    if (neg)
        flags |= VMBN_F_NEG;
    if (val_zero)
        flags |= VMBN_F_ZERO;

    /* go back and fix up the precision, exponent, and flags values */
    str->write2_at(num_ofs, prec);
    str->write2_at(num_ofs + 2, exp);
    str->write_at(num_ofs + 4, flags);

    /* fix up the size */
    end_ofs = str->get_ofs();
    str->write2_at(size_ofs, end_ofs - size_ofs - 2);

    /* return the new object ID */
    return id;
}

/*
 *   Convert a constant value from a CTcConstVal (compiler internal
 *   representation) to a vm_val_t (interpreter representation). 
 */
void CTcGenTarg::write_const_as_dh(CTcDataStream *ds, ulong ofs,
                                   const CTcConstVal *src)
{
    vm_val_t val;
    char buf[VMB_DATAHOLDER];

    /* convert according to the value's type */
    switch(src->get_type())
    {
    case TC_CVT_NIL:
        val.set_nil();
        break;

    case TC_CVT_TRUE:
        val.set_true();
        break;

    case TC_CVT_INT:
        val.set_int(src->get_val_int());
        break;

    case TC_CVT_FLOAT:
        /* generate the BigNumber object */
        val.set_obj(gen_bignum_obj(src->get_val_float(),
                                   src->get_val_float_len()));

        /* add a fixup for the object ID */
        if (G_keep_objfixups)
            CTcIdFixup::add_fixup(&G_objfixup, ds, ofs + 1, val.val.obj);
        break;

    case TC_CVT_SSTR:
        /* 
         *   Store the string in the constant pool.  Note that our fixup
         *   is at the destination stream offset plus one, since the
         *   DATAHOLDER has the type byte followed by the offset value.  
         */
        add_const_str(src->get_val_str(), src->get_val_str_len(),
                      ds, ofs + 1);
        
        /* 
         *   set the offset to zero for now - the fixup that
         *   add_const_str() generates will take care of supplying the
         *   real value 
         */
        val.set_sstring(0);
        break;

    case TC_CVT_LIST:
        /* 
         *   Store the sublist in the constant pool.  Our fixup is at the
         *   destination stream offset plus one, since the DATAHOLDER has
         *   the type byte followed by the offset value.  
         */
        add_const_list(src->get_val_list(), ds, ofs + 1);

        /* 
         *   set the offset to zero for now - the fixup that
         *   add_const_list() generates will take care of supplying the
         *   real value 
         */
        val.set_list(0);
        break;

    case TC_CVT_OBJ:
        /* set the object ID value */
        val.set_obj((vm_obj_id_t)src->get_val_obj());

        /* 
         *   add a fixup (at the current offset plus one, for the type
         *   byte) if we're keeping object ID fixups 
         */
        if (G_keep_objfixups)
            CTcIdFixup::add_fixup(&G_objfixup, ds, ofs + 1,
                                  src->get_val_obj());
        break;

    case TC_CVT_ENUM:
        /* set the enum value */
        val.set_enum(src->get_val_enum());

        /* add a fixup */
        if (G_keep_enumfixups)
            CTcIdFixup::add_fixup(&G_enumfixup, ds, ofs + 1,
                                  src->get_val_enum());
        break;

    case TC_CVT_PROP:
        /* set the property ID value */
        val.set_propid((vm_prop_id_t)src->get_val_prop());

        /* 
         *   add a fixup (at the current offset plus one, for the type
         *   byte) if we're keeping property ID fixups 
         */
        if (G_keep_propfixups)
            CTcIdFixup::add_fixup(&G_propfixup, ds, ofs + 1,
                                  src->get_val_prop());
        break;

    case TC_CVT_FUNCPTR:
        /* 
         *   use a placeholder value of zero for now - a function's final
         *   address is never known until after all code generation has
         *   been completed (the fixup will take care of supplying the
         *   correct value when the time comes) 
         */
        val.set_fnptr(0);

        /* 
         *   Add a fixup.  The fixup is at the destination stream offset
         *   plus one, because the DATAHOLDER has a type byte followed by
         *   the function pointer value.  
         */
        src->get_val_funcptr_sym()->add_abs_fixup(ds, ofs + 1);
        break;

    case TC_CVT_ANONFUNCPTR:
        /* use a placeholder of zero for now, until we fix up the pointer */
        val.set_fnptr(0);

        /* add a fixup for the code body */
        src->get_val_anon_func_ptr()->add_abs_fixup(ds, ofs + 1);
        break;

    case TC_CVT_VOCAB_LIST:
        /* 
         *   it's an internal vocabulary list type - this is used as a
         *   placeholder only, and will be replaced during linking with an
         *   actual vocabulary string list 
         */
        val.typ = VM_VOCAB_LIST;
        break;
    }

    /* write the vm_val_t in DATA_HOLDER format into the stream */
    vmb_put_dh(buf, &val);
    ds->write_at(ofs, buf, VMB_DATAHOLDER);
}

/*
 *   Write a DATAHOLDER at the current offset in a stream
 */
void CTcGenTarg::write_const_as_dh(CTcDataStream *ds, 
                                   const CTcConstVal *src)
{
    /* write to the current stream offset */
    write_const_as_dh(ds, ds->get_ofs(), src);
}

/*
 *   Notify that parsing is finished 
 */
void CTcGenTarg::parsing_done()
{
    /* nothing special to do */
}


/*
 *   notify the code generator that we're replacing an object 
 */
void CTcGenTarg::notify_replace_object(ulong stream_ofs)
{
    uint flags;

    /* set the 'replaced' flag in the flags prefix */
    flags = G_os->read2_at(stream_ofs);
    flags |= TCT3_OBJ_REPLACED;
    G_os->write2_at(stream_ofs, flags);
}


/*
 *   Set the starting offset of the current method 
 */
void CTcGenTarg::set_method_ofs(ulong ofs)
{
    /* tell the exception table object about it */
    get_exc_table()->set_method_ofs(ofs);

    /* remember it in the code stream */
    G_cs->set_method_ofs(ofs);
}

/*
 *   Add a debug line table to our list 
 */
void CTcGenTarg::add_debug_line_table(ulong ofs)
{
    size_t idx;
    uchar *p;

    /* calculate the index of the next free entry on its page */
    idx = (size_t)(debug_line_cnt_ % TCT3_DEBUG_LINE_PAGE_SIZE);
    
    /* 
     *   if we've completely filled the last page, allocate a new one - we
     *   know we've exhausted the page if we're at the start of a new page
     *   (i.e., the index is zero) 
     */
    if (idx == 0)
    {
        tct3_debug_line_page *pg;
        
        /* allocate the new page */
        pg = (tct3_debug_line_page *)t3malloc(sizeof(*pg));

        /* link it in at the end of the list */
        pg->nxt = 0;
        if (debug_line_tail_ == 0)
            debug_line_head_ = pg;
        else
            debug_line_tail_->nxt = pg;
        debug_line_tail_ = pg;
    }

    /* get a pointer to the entry */
    p = debug_line_tail_->line_ofs + (idx * TCT3_DEBUG_LINE_REC_SIZE);

    /* 
     *   set this entry - one byte for the code stream ID, then a UINT4
     *   with the offset in the stream
     */
    *p = G_cs->get_stream_id();
    oswp4(p + 1, ofs);

    /* count it */
    ++debug_line_cnt_;
}

/* ------------------------------------------------------------------------ */
/*
 *   Generic T3 node 
 */

/* 
 *   generate a jump-ahead instruction, returning a new label which serves
 *   as the jump destination 
 */
CTcCodeLabel *CTcPrsNode::gen_jump_ahead(uchar opc)
{
    CTcCodeLabel *lbl;

    /* 
     *   check to see if we should suppress the jump for peephole
     *   optimization 
     */
    if (G_cg->can_skip_op())
        return 0;

    /* emit the opcode */
    G_cg->write_op(opc);

    /* allocate a new label */
    lbl = G_cs->new_label_fwd();

    /* 
     *   write the forward offset to the label (this will generate a fixup
     *   record attached to the label, so that we'll come back and fix it
     *   up when the real offset is known) 
     */
    G_cs->write_ofs2(lbl, 0);

    /* return the forward label */
    return lbl;
}

/*
 *   Allocate a new label at the current write position 
 */
CTcCodeLabel *CTcPrsNode::new_label_here()
{
    /* 
     *   suppress any peephole optimizations at this point -- someone
     *   could jump directly to this instruction, so we can't combine an
     *   instruction destined for this point with anything previous 
     */
    G_cg->clear_peephole();

    /* create and return a label at the current position */
    return G_cs->new_label_here();
}

/* 
 *   define the position of a code label 
 */
void CTcPrsNode::def_label_pos(CTcCodeLabel *lbl)
{
    /* if the label is null, ignore it */
    if (lbl == 0)
        return;

    /* 
     *   try eliminating a jump-to-next-instruction sequence: if the last
     *   opcode was a JMP to this label, remove the last instruction
     *   entirely 
     */
    if (G_cg->get_last_op() == OPC_JMP
        && G_cs->has_fixup_at_ofs(lbl, G_cs->get_ofs() - 2))
    {
        /* remove the fixup pointing to the preceding JMP */
        G_cs->remove_fixup_at_ofs(lbl, G_cs->get_ofs() - 2);
        
        /* the JMP is unnecessary - remove it */
        G_cg->remove_last_jmp();
    }
    
    /* define the label position and apply the fixup */
    G_cs->def_label_pos(lbl);

    /* 
     *   whenever we define a label, we must suppress any peephole
     *   optimizations at this point - someone could jump directly to this
     *   instruction, so we can't combine an instruction destined for this
     *   point with anything previous 
     */
    G_cg->clear_peephole();
}

/*
 *   Generate code for an if-else conditional test.  The default
 *   implementation is to evaluate the expression, and jump to the false
 *   branch if the expression is false (or jump to the true part if the
 *   expression is true and there's no false part).
 */
void CTcPrsNode::gen_code_cond(CTcCodeLabel *then_label,
                               CTcCodeLabel *else_label)
{
    /* generate our expression code */
    gen_code(FALSE, TRUE);

    /* 
     *   if we have a 'then' part, jump to the 'then' part if the condition
     *   is true; otherwise, jump to the 'else' part if the condition is
     *   false 
     */
    if (then_label != 0)
    {
        /* we have a 'then' part, so jump if true to the 'then' part */
        G_cg->write_op(OPC_JT);
        G_cs->write_ofs2(then_label, 0);
    }
    else
    {
        /* we have an 'else' part, so jump if false to the 'else' */
        G_cg->write_op(OPC_JF);
        G_cs->write_ofs2(else_label, 0);
    }

    /* the JF or JT pops an element off the stack */
    G_cg->note_pop();
}

/*
 *   generate code for assignment to this node 
 */
int CTcPrsNode::gen_code_asi(int, tc_asitype_t, CTcPrsNode *,
                             int ignore_error)
{
    /* 
     *   if ignoring errors, the caller is trying to assign if possible
     *   but doesn't require it to be possible; simply return false to
     *   indicate that nothing happened if this is the case 
     */
    if (ignore_error)
        return FALSE;
    
    /* we should never get here - throw an internal error */
    G_tok->throw_internal_error(TCERR_GEN_BAD_LVALUE);
    return FALSE;
}

/*
 *   generate code for taking the address of this node 
 */
void CTcPrsNode::gen_code_addr()
{
    /* we should never get here - throw an internal error */
    G_tok->throw_internal_error(TCERR_GEN_BAD_ADDR);
}

/*
 *   Generate code to call the expression as a function or method.  
 */
void CTcPrsNode::gen_code_call(int discard, int argc, int varargs)
{
    /* function/method calls are never valid in speculative mode */
    if (G_cg->is_speculative())
        err_throw(VMERR_BAD_SPEC_EVAL);
    
    /*
     *   For default nodes, assume that the result of evaluating the
     *   expression contained in the node is a method or function pointer.
     *   First, generate code to evaluate the expression, which should
     *   yield an appropriate pointer value. 
     */
    gen_code(FALSE, FALSE);

    /* 
     *   if we have a varargs list, modify the call instruction that
     *   follows to make it a varargs call 
     */
    if (varargs)
    {
        /* swap the top of the stack to get the arg counter back on top */
        G_cg->write_op(OPC_SWAP);

        /* write the varargs modifier */
        G_cg->write_op(OPC_VARARGC);
    }

    /* generate an indirect function call through the pointer */
    G_cg->write_op(OPC_PTRCALL);
    G_cs->write((char)argc);

    /* PTRCALL pops the arguments plus the function pointer */
    G_cg->note_pop(argc + 1);

    /* 
     *   if the caller isn't going to discard the return value, push the
     *   result, which is sitting in R0 
     */
    if (!discard)
    {
        G_cg->write_op(OPC_GETR0);
        G_cg->note_push();
    }
}

/*
 *   Generate code for operator 'new' applied to this node
 */
void CTcPrsNode::gen_code_new(int, int, int, int, int)
{
    /* operator 'new' cannot be applied to a default node */
    G_tok->log_error(TCERR_INVAL_NEW_EXPR);
}

/*
 *   Generate code for a member evaluation 
 */
void CTcPrsNode::gen_code_member(int discard,
                                 CTcPrsNode *prop_expr, int prop_is_expr,
                                 int argc, int varargs)
{
    /* evaluate my own expression to yield the object value */
    gen_code(FALSE, FALSE);

    /* if we have an argument counter, put it back on top */
    if (varargs)
        G_cg->write_op(OPC_SWAP);

    /* use the generic code to generate the rest */
    s_gen_member_rhs(discard, prop_expr, prop_is_expr, argc, varargs);
}

/*
 *   Generic code to generate the rest of a member expression after the
 *   left side of the '.' has been generated.  This can be used for cases
 *   where the left of the '.' is an arbitrary expression, and hence must
 *   be evaluated at run-time.
 */
void CTcPrsNode::s_gen_member_rhs(int discard,
                                  CTcPrsNode *prop_expr, int prop_is_expr,
                                  int argc, int varargs)
{
    vm_prop_id_t prop;

    /* we can't call methods with argument in speculative mode */
    if (argc != 0 && G_cg->is_speculative())
        err_throw(VMERR_BAD_SPEC_EVAL);

    /* get or generate the property ID value */
    prop = prop_expr->gen_code_propid(FALSE, prop_is_expr);

    /* 
     *   if we got a property ID, generate a simple GETPROP or CALLPROP
     *   instruction; otherwise, generate a PTRCALLPROP instruction
     */
    if (prop != VM_INVALID_PROP)
    {
        /* 
         *   we have a constant property ID - generate a GETPROP or
         *   CALLPROP with the property ID as constant data 
         */
        if (argc == 0)
        {
            /* no arguments - generate a GETPROP */
            G_cg->write_op(G_cg->is_speculative()
                           ? OPC_GETPROPDATA : OPC_GETPROP);
            G_cs->write_prop_id(prop);

            /* this pops an object element */
            G_cg->note_pop();
        }
        else
        {
            /* write the CALLPROP instruction */
            G_cg->write_callprop(argc, varargs, prop);
        }
    }
    else
    {
        if (G_cg->is_speculative())
        {
            /* 
             *   speculative - use PTRGETPROPDATA to ensure we don't cause
             *   any side effects 
             */
            G_cg->write_op(OPC_PTRGETPROPDATA);
        }
        else
        {
            /* 
             *   if we have a varargs list, modify the call instruction
             *   that follows to make it a varargs call 
             */
            if (varargs)
            {
                /* swap to get the arg counter back on top */
                G_cg->write_op(OPC_SWAP);
                
                /* write the varargs modifier */
                G_cg->write_op(OPC_VARARGC);
            }

            /* a property pointer is on the stack - write a PTRCALLPROP */
            G_cg->write_op(OPC_PTRCALLPROP);
            G_cs->write((int)argc);
        }

        /* 
         *   ptrcallprop/ptrgetpropdata removes arguments, the object, and
         *   the property 
         */
        G_cg->note_pop(argc + 2);
    }

    /* if we're not discarding the result, push it from R0 */
    if (!discard)
    {
        G_cg->write_op(OPC_GETR0);
        G_cg->note_push();
    }
}

/*
 *   Generate code to get the property ID of the expression.
 */
vm_prop_id_t CTcPrsNode::gen_code_propid(int check_only, int is_expr)
{
    /* 
     *   simply evaluate the expression normally, anticipating that this
     *   will yield a property ID value at run-time 
     */
    if (!check_only)
        gen_code(FALSE, FALSE);

    /* tell the caller that there's no constant ID available */
    return VM_INVALID_PROP;
}

/* ------------------------------------------------------------------------ */
/*
 *   "self" 
 */

/*
 *   generate code 
 */
void CTPNSelf::gen_code(int discard, int)
{
    /* it's an error if we're not in a method context */
    if (!G_cs->is_self_available())
        G_tok->log_error(TCERR_SELF_NOT_AVAIL);
    
    /* if we're not discarding the result, push the "self" object */
    if (!discard)
    {
        G_cg->write_op(OPC_PUSHSELF);
        G_cg->note_push();
    }
}

/*
 *   evaluate a property 
 */
void CTPNSelf::gen_code_member(int discard,
                               CTcPrsNode *prop_expr, int prop_is_expr,
                               int argc, int varargs)
{
    vm_prop_id_t prop;
    
    /* make sure "self" is available */
    if (!G_cs->is_self_available())
        G_tok->log_error(TCERR_SELF_NOT_AVAIL);

    /* don't allow arguments in speculative eval mode */
    if (argc != 0 && G_cg->is_speculative())
        err_throw(VMERR_BAD_SPEC_EVAL);

    /* generate the property value */
    prop = prop_expr->gen_code_propid(FALSE, prop_is_expr);

    /* 
     *   if we got a property ID, generate a simple GETPROPSELF or
     *   CALLPROPSELF; otherwise, generate a PTRCALLPROPSELF 
     */
    if (prop != VM_INVALID_PROP)
    {
        /* 
         *   we have a constant property ID - generate a GETPROPDATA,
         *   GETPROPSELF, or CALLPROPSELF with the property ID as
         *   immediate data 
         */
        if (G_cg->is_speculative())
        {
            /* speculative - use GETPROPDATA */
            G_cg->write_op(OPC_PUSHSELF);
            G_cg->write_op(OPC_GETPROPDATA);
            G_cs->write_prop_id(prop);

            /* we pushed one element (self) and popped it back off */
            G_cg->note_push();
            G_cg->note_pop();
        }
        else if (argc == 0)
        {
            /* no arguments - generate a GETPROPSELF */
            G_cg->write_op(OPC_GETPROPSELF);
            G_cs->write_prop_id(prop);
        }
        else
        {
            /* add a varargs modifier if appropriate */
            if (varargs)
                G_cg->write_op(OPC_VARARGC);
            
            /* we have arguments - generate a CALLPROPSELF */
            G_cg->write_op(OPC_CALLPROPSELF);
            G_cs->write((char)argc);
            G_cs->write_prop_id(prop);

            /* this removes arguments */
            G_cg->note_pop(argc);
        }
    }
    else
    {
        /* 
         *   a property pointer is on the stack - use PTRGETPROPDATA or
         *   PTRCALLPROPSELF, depending on the speculative mode 
         */
        if (G_cg->is_speculative())
        {
            /* speculative - use PTRGETPROPDATA after pushing self */
            G_cg->write_op(OPC_PUSHSELF);
            G_cg->write_op(OPC_PTRGETPROPDATA);

            /* we pushed self then removed self and the property ID */
            G_cg->note_push();
            G_cg->note_pop(2);
        }
        else
        {
            /* 
             *   if we have a varargs list, modify the call instruction
             *   that follows to make it a varargs call 
             */
            if (varargs)
            {
                /* swap to get the arg counter back on top */
                G_cg->write_op(OPC_SWAP);
                
                /* write the varargs modifier */
                G_cg->write_op(OPC_VARARGC);
            }

            /* a prop pointer is on the stack - write a PTRCALLPROPSELF */
            G_cg->write_op(OPC_PTRCALLPROPSELF);
            G_cs->write((int)argc);
            
            /* this removes arguments and the property pointer */
            G_cg->note_pop(argc + 1);
        }
    }

    /* if the result is needed, push it */
    if (!discard)
    {
        G_cg->write_op(OPC_GETR0);
        G_cg->note_push();
    }
}


/*
 *   generate code for an object before a '.' 
 */
vm_obj_id_t CTPNSelf::gen_code_obj_predot(int *is_self)
{
    /* make sure "self" is available */
    if (!G_cs->is_self_available())
        G_tok->log_error(TCERR_SELF_NOT_AVAIL);

    /* tell the caller that this is "self" */
    *is_self = TRUE;
    return VM_INVALID_OBJ;
}


/* ------------------------------------------------------------------------ */
/*
 *   "replaced" 
 */

/*
 *   evaluate 'replaced' on its own - this simply yields a function pointer
 *   to the modified base code 
 */
void CTPNReplaced::gen_code(int discard, int for_condition)
{
    CTcSymFunc *mod_base;

    /* get the modified base function symbol */
    mod_base = G_cs->get_code_body()->get_func_sym();
    mod_base = (mod_base != 0 ? mod_base->get_mod_base() : 0);

    /* make sure we're in a 'modify func()' context */
    if (mod_base == 0)
        G_tok->log_error(TCERR_REPLACED_NOT_AVAIL);

    /* this expression yields a pointer to the modified base function */
    G_cg->write_op(OPC_PUSHFNPTR);

    /* add a fixup for the current code location */
    if (mod_base != 0)
        mod_base->add_abs_fixup(G_cs);

    /* write a placeholder offset - arbitrarily use zero */
    G_cs->write4(0);

    /* note the push */
    G_cg->note_push();
}

/*
 *   'replaced()' call - this invokes the modified base code 
 */
void CTPNReplaced::gen_code_call(int discard, int argc, int varargs)
{
    CTcSymFunc *mod_base;

    /* get the modified base function symbol */
    mod_base = G_cs->get_code_body()->get_func_sym();
    mod_base = (mod_base != 0 ? mod_base->get_mod_base() : 0);

    /* make sure we're in a 'modify func()' context */
    if (mod_base == 0)
        G_tok->log_error(TCERR_REPLACED_NOT_AVAIL);

    /* write the varargs modifier if appropriate */
    if (varargs)
        G_cg->write_op(OPC_VARARGC);

    /* generate the call instruction and argument count */
    G_cg->write_op(OPC_CALL);
    G_cs->write((char)argc);

    /* generate a fixup for the call to the modified base code */
    if (mod_base != 0)
        mod_base->add_abs_fixup(G_cs);

    /* add a placeholder for the function address */
    G_cs->write4(0);

    /* call removes arguments */
    G_cg->note_pop(argc);

    /* make sure the argument count is correct */
    if (mod_base != 0
        && (mod_base->is_varargs() ? argc < mod_base->get_argc()
                                   : argc != mod_base->get_argc()))
        G_tok->log_error(TCERR_WRONG_ARGC_FOR_FUNC,
                         (int)mod_base->get_sym_len(), mod_base->get_sym(),
                         mod_base->get_argc(), argc);

    /* if we're not discarding, push the return value from R0 */
    if (!discard)
    {
        G_cg->write_op(OPC_GETR0);
        G_cg->note_push();
    }
}

/* ------------------------------------------------------------------------ */
/*
 *   "targetprop" 
 */

/*
 *   generate code 
 */
void CTPNTargetprop::gen_code(int discard, int)
{
    /* it's an error if we're not in a method context */
    if (!G_cs->is_self_available())
        G_tok->log_error(TCERR_TARGETPROP_NOT_AVAIL);

    /* if we're not discarding the result, push the target property ID */
    if (!discard)
    {
        G_cg->write_op(OPC_PUSHCTXELE);
        G_cs->write(PUSHCTXELE_TARGPROP);
        G_cg->note_push();
    }
}

/* ------------------------------------------------------------------------ */
/*
 *   "targetobj" 
 */

/*
 *   generate code 
 */
void CTPNTargetobj::gen_code(int discard, int)
{
    /* it's an error if we're not in a method context */
    if (!G_cs->is_self_available())
        G_tok->log_error(TCERR_TARGETOBJ_NOT_AVAIL);

    /* if we're not discarding the result, push the target object ID */
    if (!discard)
    {
        G_cg->write_op(OPC_PUSHCTXELE);
        G_cs->write(PUSHCTXELE_TARGOBJ);
        G_cg->note_push();
    }
}

/* ------------------------------------------------------------------------ */
/*
 *   "definingobj" 
 */

/*
 *   generate code 
 */
void CTPNDefiningobj::gen_code(int discard, int)
{
    /* it's an error if we're not in a method context */
    if (!G_cs->is_self_available())
        G_tok->log_error(TCERR_DEFININGOBJ_NOT_AVAIL);

    /* if we're not discarding the result, push the defining object ID */
    if (!discard)
    {
        G_cg->write_op(OPC_PUSHCTXELE);
        G_cs->write(PUSHCTXELE_DEFOBJ);
        G_cg->note_push();
    }
}


/* ------------------------------------------------------------------------ */
/*
 *   "inherited" 
 */
void CTPNInh::gen_code(int, int)
{
    /* 
     *   we should never be asked to generate an "inherited" node
     *   directly; these nodes should always be generated as part of
     *   member evaluation 
     */
    G_tok->throw_internal_error(TCERR_GEN_CODE_INH);
}

/*
 *   evaluate a property 
 */
void CTPNInh::gen_code_member(int discard,
                              CTcPrsNode *prop_expr, int prop_is_expr,
                              int argc, int varargs)
{
    vm_prop_id_t prop;

    /* 
     *   make sure "self" is available - we obviously can't inherit
     *   anything if we're not in an object's method 
     */
    if (!G_cs->is_self_available())
        G_tok->log_error(TCERR_SELF_NOT_AVAIL);

    /* don't allow 'inherited' in speculative evaluation mode */
    if (G_cg->is_speculative())
        err_throw(VMERR_BAD_SPEC_EVAL);

    /* generate the property value */
    prop = prop_expr->gen_code_propid(FALSE, prop_is_expr);

    /* 
     *   if we got a property ID, generate a simple INHERIT;
     *   otherwise, generate a PTRINHERIT 
     */
    if (prop != VM_INVALID_PROP)
    {
        /* generate a varargs modifier if necessary */
        if (varargs)
            G_cg->write_op(OPC_VARARGC);
        
        /* we have a constant property ID - generate a regular INHERIT */
        G_cg->write_op(OPC_INHERIT);
        G_cs->write((char)argc);
        G_cs->write_prop_id(prop);

        /* this removes arguments */
        G_cg->note_pop(argc);
    }
    else
    {
        /* 
         *   if we have a varargs list, modify the call instruction that
         *   follows to make it a varargs call 
         */
        if (varargs)
        {
            /* swap the top of the stack to get the arg counter back on top */
            G_cg->write_op(OPC_SWAP);
            
            /* write the varargs modifier */
            G_cg->write_op(OPC_VARARGC);
        }

        /* a property pointer is on the stack - write a PTRINHERIT */
        G_cg->write_op(OPC_PTRINHERIT);
        G_cs->write((int)argc);

        /* this removes arguments and the property pointer */
        G_cg->note_pop(argc + 1);
    }

    /* if the result is needed, push it */
    if (!discard)
    {
        G_cg->write_op(OPC_GETR0);
        G_cg->note_push();
    }
}

/* ------------------------------------------------------------------------ */
/*
 *   "inherited class"
 */
void CTPNInhClass::gen_code(int discard, int for_condition)
{
    /* 
     *   we should never be asked to generate an "inherited" node
     *   directly; these nodes should always be generated as part of
     *   member evaluation 
     */
    G_tok->throw_internal_error(TCERR_GEN_CODE_INH);
}

/*
 *   evaluate a property 
 */
void CTPNInhClass::gen_code_member(int discard,
                                   CTcPrsNode *prop_expr, int prop_is_expr,
                                   int argc, int varargs)
{
    vm_prop_id_t prop;
    CTcSymbol *objsym;
    vm_obj_id_t obj;

    /* 
     *   make sure "self" is available - we obviously can't inherit
     *   anything if we're not in an object's method 
     */
    if (!G_cs->is_self_available())
        G_tok->log_error(TCERR_SELF_NOT_AVAIL);

    /* don't allow 'inherited' in speculative evaluation mode */
    if (G_cg->is_speculative())
        err_throw(VMERR_BAD_SPEC_EVAL);

    /* get the superclass name symbol */
    objsym = G_cs->get_symtab()->find_or_def_undef(sym_, len_, FALSE);
    
    /* if it's not an object, we can't inherit from it */
    obj = objsym->get_val_obj();
    if (obj == VM_INVALID_OBJ)
    {
        G_tok->log_error(TCERR_INH_NOT_OBJ, (int)len_, sym_);
        return;
    }

    /* generate the property value */
    prop = prop_expr->gen_code_propid(FALSE, prop_is_expr);

    /* 
     *   if we got a property ID, generate a simple EXPINHERIT; otherwise,
     *   generate a PTREXPINHERIT 
     */
    if (prop != VM_INVALID_PROP)
    {
        /* add a varargs modifier if needed */
        if (varargs)
            G_cg->write_op(OPC_VARARGC);
        
        /* we have a constant property ID - generate a regular EXPINHERIT */
        G_cg->write_op(OPC_EXPINHERIT);
        G_cs->write((char)argc);
        G_cs->write_prop_id(prop);
        G_cs->write_obj_id(obj);

        /* this removes argumnts */
        G_cg->note_pop(argc);
    }
    else
    {
        /* 
         *   if we have a varargs list, modify the call instruction that
         *   follows to make it a varargs call 
         */
        if (varargs)
        {
            /* swap the top of the stack to get the arg counter back on top */
            G_cg->write_op(OPC_SWAP);
            
            /* write the varargs modifier */
            G_cg->write_op(OPC_VARARGC);
        }
        
        /* a property pointer is on the stack - write a PTREXPINHERIT */
        G_cg->write_op(OPC_PTREXPINHERIT);
        G_cs->write((int)argc);
        G_cs->write_obj_id(obj);

        /* this removes arguments and the property pointer */
        G_cg->note_pop(argc + 1);
    }

    /* if the result is needed, push it */
    if (!discard)
    {
        G_cg->write_op(OPC_GETR0);
        G_cg->note_push();
    }
}

/* ------------------------------------------------------------------------ */
/*
 *   "delegated" 
 */
void CTPNDelegated::gen_code(int discard, int for_condition)
{
    /* 
     *   we should never be asked to generate a "delegated" node directly;
     *   these nodes should always be generated as part of member evaluation 
     */
    G_tok->throw_internal_error(TCERR_GEN_CODE_DELEGATED);
}

/*
 *   evaluate a property 
 */
void CTPNDelegated::gen_code_member(int discard,
                                    CTcPrsNode *prop_expr, int prop_is_expr,
                                    int argc, int varargs)
{
    vm_prop_id_t prop;

    /* 
     *   make sure "self" is available - we obviously can't delegate
     *   anything if we're not in an object's method 
     */
    if (!G_cs->is_self_available())
        G_tok->log_error(TCERR_SELF_NOT_AVAIL);

    /* don't allow 'delegated' in speculative evaluation mode */
    if (G_cg->is_speculative())
        err_throw(VMERR_BAD_SPEC_EVAL);

    /* generate the delegatee expression */
    delegatee_->gen_code(FALSE, FALSE);

    /* if we have an argument counter, put it back on top */
    if (varargs)
        G_cg->write_op(OPC_SWAP);

    /* generate the property value */
    prop = prop_expr->gen_code_propid(FALSE, prop_is_expr);

    /* 
     *   if we got a property ID, generate a simple DELEGATE; otherwise,
     *   generate a PTRDELEGATE 
     */
    if (prop != VM_INVALID_PROP)
    {
        /* add a varargs modifier if needed */
        if (varargs)
            G_cg->write_op(OPC_VARARGC);

        /* we have a constant property ID - generate a regular DELEGATE */
        G_cg->write_op(OPC_DELEGATE);
        G_cs->write((char)argc);
        G_cs->write_prop_id(prop);

        /* this removes arguments and the object value */
        G_cg->note_pop(argc + 1);
    }
    else
    {
        /* 
         *   if we have a varargs list, modify the call instruction that
         *   follows to make it a varargs call 
         */
        if (varargs)
        {
            /* swap the top of the stack to get the arg counter back on top */
            G_cg->write_op(OPC_SWAP);

            /* write the varargs modifier */
            G_cg->write_op(OPC_VARARGC);
        }

        /* a property pointer is on the stack - write a PTRDELEGATE */
        G_cg->write_op(OPC_PTRDELEGATE);
        G_cs->write((int)argc);

        /* this removes arguments, the object, and the property pointer */
        G_cg->note_pop(argc + 2);
    }

    /* if the result is needed, push it */
    if (!discard)
    {
        G_cg->write_op(OPC_GETR0);
        G_cg->note_push();
    }
}


/* ------------------------------------------------------------------------ */
/*
 *   "argcount" 
 */
void CTPNArgc::gen_code(int discard, int)
{
    /* generate the argument count, if we're not discarding */
    if (!discard)
    {
        if (G_cg->is_eval_for_debug())
        {
            /* generate a debug argument count evaluation */
            G_cg->write_op(OPC_GETDBARGC);
            G_cs->write2(G_cg->get_debug_stack_level());
        }
        else
        {
            /* generate the normal argument count evaluation */
            G_cg->write_op(OPC_GETARGC);
        }

        /* we push one element */
        G_cg->note_push();
    }
}


/* ------------------------------------------------------------------------ */
/*
 *   constant
 */
void CTPNConst::gen_code(int discard, int)
{
    /* if we're discarding the value, do nothing */
    if (discard)
        return;
    
    /* generate the appropriate type of push for the value */
    switch(val_.get_type())
    {
    case TC_CVT_NIL:
        G_cg->write_op(OPC_PUSHNIL);
        break;

    case TC_CVT_TRUE:
        G_cg->write_op(OPC_PUSHTRUE);
        break;

    case TC_CVT_INT:
        /* write the push-integer instruction */
        s_gen_code_int(val_.get_val_int());

        /* s_gen_code_int notes a push, which we'll do also, so cancel it */
        G_cg->note_pop();
        break;

    case TC_CVT_FLOAT:
        /* we'll represent it as a BigNumber object */
        G_cg->write_op(OPC_PUSHOBJ);

        /* generate the BigNumber object and write its ID */
        G_cs->write_obj_id(G_cg->gen_bignum_obj(val_.get_val_float(),
                                                val_.get_val_float_len()));
        break;

    case TC_CVT_SSTR:
        /* write the instruction to push a constant pool string */
        G_cg->write_op(OPC_PUSHSTR);
        
        /* 
         *   add the string to the constant pool, creating a fixup at the
         *   current code stream location 
         */
        G_cg->add_const_str(val_.get_val_str(), val_.get_val_str_len(),
                            G_cs, G_cs->get_ofs());

        /* 
         *   write a placeholder address - this will be corrected by the
         *   fixup that add_const_str() created for us 
         */
        G_cs->write4(0);
        break;

    case TC_CVT_LIST:
        /* write the instruction */
        G_cg->write_op(OPC_PUSHLST);

        /* 
         *   add the list to the constant pool, creating a fixup at the
         *   current code stream location 
         */
        G_cg->add_const_list(val_.get_val_list(), G_cs, G_cs->get_ofs());

        /* 
         *   write a placeholder address - this will be corrected by the
         *   fixup that add_const_list() created for us
         */
        G_cs->write4(0);
        break;

    case TC_CVT_OBJ:
        /* generate the object ID */
        G_cg->write_op(OPC_PUSHOBJ);
        G_cs->write_obj_id(val_.get_val_obj());
        break;

    case TC_CVT_PROP:
        /* generate the property address */
        G_cg->write_op(OPC_PUSHPROPID);
        G_cs->write_prop_id(val_.get_val_prop());
        break;

    case TC_CVT_ENUM:
        /* generate the enum value */
        G_cg->write_op(OPC_PUSHENUM);
        G_cs->write_enum_id(val_.get_val_enum());
        break;

    case TC_CVT_FUNCPTR:
        /* generate the function pointer instruction */
        G_cg->write_op(OPC_PUSHFNPTR);

        /* add a fixup for the function address */
        val_.get_val_funcptr_sym()->add_abs_fixup(G_cs);

        /* write out a placeholder - arbitrarily use zero */
        G_cs->write4(0);
        break;

    case TC_CVT_ANONFUNCPTR:
        /* generate the function pointer instruction */
        G_cg->write_op(OPC_PUSHFNPTR);

        /* add a fixup for the code body address */
        val_.get_val_anon_func_ptr()->add_abs_fixup(G_cs);

        /* write our a placeholder */
        G_cs->write4(0);
        break;

    default:
        /* anything else is an internal error */
        G_tok->throw_internal_error(TCERR_GEN_UNK_CONST_TYPE);
    }

    /* all of these push a value */
    G_cg->note_push();
}

/*
 *   generate code to push an integer value 
 */
void CTPNConst::s_gen_code_int(long intval)
{
    /* push the smallest format that will fit the value */
    if (intval == 0)
    {
        /* write the special PUSH_0 instruction */
        G_cg->write_op(OPC_PUSH_0);
    }
    else if (intval == 1)
    {
        /* write the special PUSH_1 instruction */
        G_cg->write_op(OPC_PUSH_1);
    }
    else if (intval < 127 && intval >= -128)
    {
        /* it fits in eight bits */
        G_cg->write_op(OPC_PUSHINT8);
        G_cs->write((char)intval);
    }
    else
    {
        /* it doesn't fit in 8 bits - use a full 32 bits */
        G_cg->write_op(OPC_PUSHINT);
        G_cs->write4(intval);
    }

    /* however we did it, we left one value on the stack */
    G_cg->note_push();
}

/*
 *   Generate code to apply operator 'new' to the constant.  We can apply
 *   'new' only to constant object values. 
 */
void CTPNConst::gen_code_new(int discard, int argc, int varargs,
                             int /*from_call*/, int is_transient)
{
    /* check the type */
    switch(val_.get_type())
    {
    case TC_CVT_OBJ:
        /* treat this the same as any other 'new' call */
        CTcSymObj::s_gen_code_new(discard, val_.get_val_obj(),
                                  argc, varargs, is_transient);
        break;

    default:
        /* can't apply 'new' to other constant values */
        G_tok->log_error(TCERR_INVAL_NEW_EXPR);
        break;
    }
}

/*
 *   Generate code to make a function call to this expression.  If we're
 *   calling a function, we can generate this directly.  
 */
void CTPNConst::gen_code_call(int discard, int argc, int varargs)
{
    /* check our type */
    switch(val_.get_type())
    {
    case TC_CVT_FUNCPTR:
        /* generate a call to our function symbol */
        val_.get_val_funcptr_sym()->gen_code_call(discard, argc, varargs);
        break;

    default:
        /* other types cannot be called */
        G_tok->log_error(TCERR_CANNOT_CALL_CONST);
        break;
    }
}

/*
 *   generate a property ID expression 
 */
vm_prop_id_t CTPNConst::gen_code_propid(int check_only, int is_expr)
{
    /* check the type */
    switch(val_.get_type())
    {
    case TC_CVT_PROP:
        /* return the constant property ID */
        return val_.get_val_prop();

    default:
        /* other values cannot be used as properties */
        if (!check_only)
            G_tok->log_error(TCERR_INVAL_PROP_EXPR);
        return VM_INVALID_PROP;
    }
}


/*
 *   Generate code for a member evaluation 
 */
void CTPNConst::gen_code_member(int discard,
                                CTcPrsNode *prop_expr, int prop_is_expr,
                                int argc, int varargs)
{
    /* check our constant type */
    switch(val_.get_type())
    {
    case TC_CVT_OBJ:
        /* call the object symbol code to do the work */
        CTcSymObj::s_gen_code_member(discard, prop_expr, prop_is_expr,
                                     argc, val_.get_val_obj(), varargs);
        break;

    case TC_CVT_LIST:
    case TC_CVT_SSTR:
    case TC_CVT_FLOAT:
        /* 
         *   list/string/BigNumber constant - generate our value as
         *   normal, then use the standard member generation 
         */
        gen_code(FALSE, FALSE);

        /* if we have an argument counter, put it back on top */
        if (varargs)
            G_cg->write_op(OPC_SWAP);

        /* use standard member generation */
        CTcPrsNode::s_gen_member_rhs(discard, prop_expr, prop_is_expr,
                                     argc, varargs);
        break;

    default:
        G_tok->log_error(TCERR_INVAL_OBJ_EXPR);
        break;
    }
}


/*
 *   generate code for an object before a '.'  
 */
vm_obj_id_t CTPNConst::gen_code_obj_predot(int *is_self)
{
    /* we're certainly not "self" */
    *is_self = FALSE;

    /* if I don't have an object value, this is illegal */
    if (val_.get_type() != TC_CVT_OBJ)
    {
        G_tok->log_error(TCERR_INVAL_OBJ_EXPR);
        return VM_INVALID_OBJ;
    }

    /* report our constant object value */
    return val_.get_val_obj();
}

/* ------------------------------------------------------------------------ */
/*
 *   debugger constant 
 */
void CTPNDebugConst::gen_code(int discard, int for_condition)
{
    /* if we're discarding the value, do nothing */
    if (discard)
        return;

    /* generate the appropriate type of push for the value */
    switch(val_.get_type())
    {
    case TC_CVT_SSTR:
        /* write the in-line string instruction */
        G_cg->write_op(OPC_PUSHSTRI);
        G_cs->write2(val_.get_val_str_len());
        G_cs->write(val_.get_val_str(), val_.get_val_str_len());

        /* note the value push */
        G_cg->note_push();
        break;

    case TC_CVT_LIST:
        /* we should never have a constant list when debugging */
        assert(FALSE);
        break;

    case TC_CVT_FUNCPTR:
        /* generate the function pointer instruction */
        G_cg->write_op(OPC_PUSHFNPTR);

        /* 
         *   write the actual function address - no need for fixups in the
         *   debugger, since everything's fully resolved 
         */
        G_cs->write4(val_.get_val_funcptr_sym()->get_code_pool_addr());

        /* note the value push */
        G_cg->note_push();
        break;

    case TC_CVT_ANONFUNCPTR:
        /* 
         *   we should never see an anonymous function pointer in the
         *   debugger 
         */
        assert(FALSE);
        break;

    case TC_CVT_FLOAT:
        {
            CTcSymMetaclass *sym;

            /* 
             *   find the 'BigNumber' metaclass - if it's not defined, we
             *   can't create BigNumber values 
             */
            sym = (CTcSymMetaclass *)G_prs->get_global_symtab()
                  ->find("BigNumber", 9);
            if (sym == 0 || sym->get_type() != TC_SYM_METACLASS)
                err_throw(VMERR_INVAL_DBG_EXPR);

            /* push the floating value as an immediate string */
            G_cg->write_op(OPC_PUSHSTRI);
            G_cs->write2(val_.get_val_str_len());
            G_cs->write(val_.get_val_str(), val_.get_val_str_len());

            /* create the new BigNumber object from the string */
            G_cg->write_op(OPC_NEW2);
            G_cs->write2(1);
            G_cs->write2(sym->get_meta_idx());

            /* retrieve the value */
            G_cg->write_op(OPC_GETR0);

            /* 
             *   note the net push of one value (we pushed the argument,
             *   popped the argument, and pushed the new object) 
             */
            G_cg->note_push();
        }
        break;

    default:
        /* handle normally for anything else */
        CTPNConst::gen_code(discard, for_condition);
        break;
    }
}


/* ------------------------------------------------------------------------ */
/*
 *   Generic Unary Operator 
 */

/* 
 *   Generate a unary-operator opcode.  We assume that the opcode has no
 *   side effects other than to compute the result, so we do not generate
 *   the opcode at all if 'discard' is true; we do, however, always
 *   generate code for the subexpression to ensure that its side effects
 *   are performed.
 *   
 *   In most cases, the caller simply should pass through its 'discard'
 *   status, since the result of the subexpression is generally needed
 *   only when the result of the enclosing expression is needed.
 *   
 *   In most cases, the caller should pass FALSE for 'for_condition',
 *   because applying an operator to the result generally requires that
 *   the result be properly converted for use as a temporary value.
 *   However, when the caller knows that its own opcode will perform the
 *   same conversions that a conditional opcode would, 'for_condition'
 *   should be TRUE.  In most cases, the caller's own 'for_condition'
 *   status is not relevant and should thus not be passed through.  
 */
void CTPNUnary::gen_unary(uchar opc, int discard, int for_condition)
{
    /* 
     *   Generate the operand.  Pass through the 'discard' status to the
     *   operand - if the result of the parent operator is being
     *   discarded, then so is the result of this subexpression.  In
     *   addition, pass through the caller's 'for_condition' disposition.  
     */
    sub_->gen_code(discard, for_condition);

    /* apply the operator if we're not discarding the result */
    if (!discard)
        G_cg->write_op(opc);
}

/* ------------------------------------------------------------------------ */
/*
 *   Generic Binary Operator
 */

/*
 *   Generate a binary-operator opcode.
 *   
 *   In most cases, the caller's 'discard' status should be passed
 *   through, since the results of the operands are usually needed if and
 *   only if the results of the enclosing expression are needed.
 *   
 *   In most cases, the caller should pass FALSE for 'for_condition'.
 *   Only when the caller knows that the opcode will perform the same
 *   conversions as a BOOLIZE instruction should it pass TRUE for
 *   'for_condition'.  
 */
void CTPNBin::gen_binary(uchar opc, int discard, int for_condition)
{
    /* 
     *   generate the operands, passing through the discard and
     *   conditional status 
     */
    left_->gen_code(discard, for_condition);
    right_->gen_code(discard, for_condition);

    /* generate our operand if we're not discarding the result */
    if (!discard)
    {
        /* apply the operator */
        G_cg->write_op(opc);

        /* 
         *   boolean operators all remove two values and push one, so
         *   there's a net pop 
         */
        G_cg->note_pop();
    }
}



/* ------------------------------------------------------------------------ */
/*
 *   logical NOT
 */
void CTPNNot::gen_code(int discard, int)
{
    /*
     *   Generate the subexpression and apply the NOT opcode.  Note that
     *   we can compute the subexpression as though we were applying a
     *   condition, because the NOT opcode takes exactly the same kind of
     *   input as any condition opcode; we can thus avoid an extra
     *   conversion in some cases.  
     */
    gen_unary(OPC_NOT, discard, TRUE);
}

/* ------------------------------------------------------------------------ */
/*
 *   Boolean-ize operator
 */
void CTPNBoolize::gen_code(int discard, int for_condition)
{
    /*
     *   If the result will be used for a conditional, there's no need to
     *   generate an instruction to convert the value to boolean.  The opcode
     *   that will be used for the condition will perform exactly the same
     *   conversions that this opcode would apply; avoid the redundant work
     *   in this case, and simply generate the underlying expression
     *   directly.  
     */
    if (for_condition)
    {
        /* generate the underlying expression without modification */
        sub_->gen_code(discard, for_condition);

        /* done */
        return;
    }
    
    /*
     *   Generate the subexpression and apply the BOOLIZE operator.  Since
     *   we're explicitly boolean-izing the value, there's no need for the
     *   subexpression to do the same thing, so the subexpression can
     *   pretend it's generating for a conditional.  
     */
    gen_unary(OPC_BOOLIZE, discard, TRUE);
}


/* ------------------------------------------------------------------------ */
/*
 *   bitwise NOT 
 */
void CTPNBNot::gen_code(int discard, int)
{
    gen_unary(OPC_BNOT, discard, FALSE);
}


/* ------------------------------------------------------------------------ */
/*
 *   arithmetic positive
 */
void CTPNPos::gen_code(int discard, int)
{
    /* 
     *   simply generate our operand, since the operator itself has no
     *   effect 
     */
    sub_->gen_code(discard, FALSE); 
}

/* ------------------------------------------------------------------------ */
/*
 *   unary arithmetic negative
 */
void CTPNNeg::gen_code(int discard, int)
{
    gen_unary(OPC_NEG, discard, FALSE);
}

/* ------------------------------------------------------------------------ */
/*
 *   pre-increment
 */
void CTPNPreInc::gen_code(int discard, int)
{
    /* ask the subnode to generate it */
    if (!sub_->gen_code_asi(discard, TC_ASI_PREINC, 0, FALSE))
    {
        /* 
         *   the subnode didn't handle it - generate code to evaluate the
         *   subnode, increment that value, then assign the result back to
         *   the subnode with a simple assignment 
         */
        sub_->gen_code(FALSE, FALSE);

        /* increment the value at top of stack */
        G_cg->write_op(OPC_INC);

        /* 
         *   generate a simple assignment back to the subexpression; if
         *   we're using the value, let the simple assignment leave its
         *   value on the stack, since the result is the value *after* the
         *   increment 
         */
        sub_->gen_code_asi(discard, TC_ASI_SIMPLE, 0, FALSE);
    }
}

/* ------------------------------------------------------------------------ */
/*
 *   pre-decrement
 */
void CTPNPreDec::gen_code(int discard, int)
{
    /* ask the subnode to generate it */
    if (!sub_->gen_code_asi(discard, TC_ASI_PREDEC, 0, FALSE))
    {
        /* 
         *   the subnode didn't handle it - generate code to evaluate the
         *   subnode, decrement that value, then assign the result back to
         *   the subnode with a simple assignment 
         */
        sub_->gen_code(FALSE, FALSE);

        /* decrement the value at top of stack */
        G_cg->write_op(OPC_DEC);

        /* 
         *   generate a simple assignment back to the subexpression; if
         *   we're using the value, let the simple assignment leave its
         *   value on the stack, since the result is the value *after* the
         *   decrement 
         */
        sub_->gen_code_asi(discard, TC_ASI_SIMPLE, 0, FALSE);
    }
}

/* ------------------------------------------------------------------------ */
/*
 *   post-increment
 */
void CTPNPostInc::gen_code(int discard, int)
{
    /* ask the subnode to generate it */
    if (!sub_->gen_code_asi(discard, TC_ASI_POSTINC, 0, FALSE))
    {
        /* 
         *   the subnode didn't handle it - generate code to evaluate the
         *   subnode, increment that value, then assign the result back to
         *   the subnode with a simple assignment 
         */
        sub_->gen_code(FALSE, FALSE);

        /* 
         *   if we're keeping the result, duplicate the value at top of
         *   stack prior to the increment - since this is a
         *   post-increment, the result is the value *before* the
         *   increment 
         */
        if (!discard)
        {
            G_cg->write_op(OPC_DUP);
            G_cg->note_push();
        }

        /* increment the value at top of stack */
        G_cg->write_op(OPC_INC);

        /* 
         *   Generate a simple assignment back to the subexpression.
         *   Discard the result of this assignment, regardless of whether
         *   the caller wants the result of the overall expression,
         *   because we've already pushed the actual result, which is the
         *   original value before the increment operation.
         */
        sub_->gen_code_asi(TRUE, TC_ASI_SIMPLE, 0, FALSE);
    }
}

/* ------------------------------------------------------------------------ */
/*
 *   post-decrement
 */
void CTPNPostDec::gen_code(int discard, int)
{
    /* ask the subnode to generate it */
    if (!sub_->gen_code_asi(discard, TC_ASI_POSTDEC, 0, FALSE))
    {
        /* 
         *   the subnode didn't handle it - generate code to evaluate the
         *   subnode, decrement that value, then assign the result back to
         *   the subnode with a simple assignment 
         */
        sub_->gen_code(FALSE, FALSE);

        /* 
         *   if we're keeping the result, duplicate the value at top of
         *   stack prior to the decrement - since this is a
         *   post-decrement, the result is the value *before* the
         *   decrement 
         */
        if (!discard)
        {
            G_cg->write_op(OPC_DUP);
            G_cg->note_push();
        }

        /* decrement the value at top of stack */
        G_cg->write_op(OPC_DEC);

        /* 
         *   Generate a simple assignment back to the subexpression.
         *   Discard the result of this assignment, regardless of whether
         *   the caller wants the result of the overall expression,
         *   because we've already pushed the actual result, which is the
         *   original value before the decrement operation.
         */
        sub_->gen_code_asi(TRUE, TC_ASI_SIMPLE, 0, FALSE);
    }
}

/* ------------------------------------------------------------------------ */
/*
 *   operator 'new'
 */
void CTPNNew::gen_code(int discard, int /*for condition*/)
{
    /* 
     *   ask my subexpression to generate the code - at this point we
     *   don't know the number of arguments, so pass in zero for now 
     */
    sub_->gen_code_new(discard, 0, FALSE, FALSE, transient_);
}

/* ------------------------------------------------------------------------ */
/*
 *   operator 'delete'
 */
void CTPNDelete::gen_code(int, int)
{
    /* 'delete' generates no code for T3 VM */
}

/* ------------------------------------------------------------------------ */
/*
 *   comma operator
 */
void CTPNComma::gen_code(int discard, int for_condition)
{
    /* 
     *   Generate each side's code.  Note that the left side is *always*
     *   discarded, regardless of whether the result of the comma operator
     *   will be discarded.  After we generate our subexpressions, there's
     *   nothing left to do, since the comma operator itself doesn't
     *   change anything - we simply use the right operand result as our
     *   result.
     *   
     *   Pass through the 'for_condition' status to the right operand,
     *   since we pass through its result to the caller.  For the left
     *   operand, treat it as a condition - we don't care about the result
     *   value, so don't bother performing any extra conversions on it.  
     */
    left_->gen_code(TRUE, TRUE);
    right_->gen_code(discard, for_condition);
}

/* ------------------------------------------------------------------------ */
/*
 *   logical OR (short-circuit logic)
 */
void CTPNOr::gen_code(int discard, int for_condition)
{
    CTcCodeLabel *lbl;
    
    /* 
     *   First, evaluate the left-hand side; we need the result even if
     *   we're discarding the overall expression, since we will check the
     *   result to see if we should even evaluate the right-hand side.
     *   We're using the value for a condition, so don't bother
     *   boolean-izing it.  
     */
    left_->gen_code(FALSE, TRUE);

    /* 
     *   If the left-hand side is true, there's no need to evaluate the
     *   right-hand side (and, in fact, we're not even allowed to evaluate
     *   the right-hand side because of the short-circuit logic rule).
     *   So, if the lhs is true, we want to jump around the code to
     *   evaluate the rhs, saving the 'true' result if we're not
     *   discarding the overall result. 
     */
    lbl = gen_jump_ahead(discard ? OPC_JT : OPC_JST);

    /* 
     *   Evaluate the right-hand side.  We don't need to save the result
     *   unless we need the result of the overall expression.  Generate
     *   the value as though we were going to booleanize it ourselves,
     *   since we'll do just that (hence pass for_condition = TRUE).  
     */
    right_->gen_code(discard, TRUE); 

    /*
     *   If we discarded the result, we generated a JT which explicitly
     *   popped a value.  If we didn't discard the result, we generated a
     *   JST; this may or may not pop the value.  However, if it doesn't
     *   pop the value (save on true), it will bypass the right side
     *   evaluation, and will thus "pop" that value in the sense that it
     *   will never be pushed.  So, note a pop either way.  
     */
    G_cg->note_pop();

    /* define the label for the jump over the rhs */
    def_label_pos(lbl);

    /* 
     *   if the result is not going to be used directly for a condition,
     *   we must boolean-ize the value 
     */
    if (!for_condition)
        G_cg->write_op(OPC_BOOLIZE);
}

/*
 *   Generate code for the short-circuit OR when used in a condition.  We can
 *   use the fact that we're being used conditionally to avoid actually
 *   pushing the result value onto the stack, instead simply branching to the
 *   appropriate point in the enclosing control structure instead.  
 */
void CTPNOr::gen_code_cond(CTcCodeLabel *then_label,
                           CTcCodeLabel *else_label)
{
    CTcCodeLabel *internal_then;

    /*
     *   First, generate the conditional code for our left operand.  If the
     *   condition is true, we can short-circuit the rest of the expression
     *   by jumping directly to the 'then' label.  If the caller provided a
     *   'then' label, we can jump directly to the caller's 'then' label;
     *   otherwise, we must synthesize our own internal label, which we'll
     *   define at the end of our generated code so that we'll fall through
     *   on true to the enclosing code.  In any case, we want to fall through
     *   if the condition is false, so that control will flow to the code for
     *   our right operand if the left operand is false.  
     */
    internal_then = (then_label == 0 ? G_cs->new_label_fwd() : then_label);
    left_->gen_code_cond(internal_then, 0);

    /* 
     *   Now, generate code for our right operand.  We can generate this code
     *   using the caller's destination labels directly: if we reach this
     *   code at all, it's because the left operand was false, in which case
     *   the result is simply the value of the right operand.  
     */
    right_->gen_code_cond(then_label, else_label);

    /* 
     *   If we created an internal 'then' label, it goes at the end of our
     *   generated code: this ensures that we fall off the end of our code
     *   if the left subexpression is true, which is what the caller told us
     *   they wanted when they gave us a null 'then' label.  If the caller
     *   gave us an explicit 'then' label, we'll have jumped there directly
     *   if the first subexpression was true.  
     */
    if (then_label == 0)
        def_label_pos(internal_then);
}

/* ------------------------------------------------------------------------ */
/*
 *   logical AND (short-circuit logic)
 */
void CTPNAnd::gen_code(int discard, int for_condition)
{
    CTcCodeLabel *lbl;

    /* 
     *   first, evaluate the left-hand side; we need the result even if
     *   we're discarding the overall expression, since we will check the
     *   result to see if we should even evaluate the right-hand side 
     */
    left_->gen_code(FALSE, TRUE);
 
    /* 
     *   If the left-hand side is false, there's no need to evaluate the
     *   right-hand side (and, in fact, we're not even allowed to evaluate
     *   the right-hand side because of the short-circuit logic rule).
     *   So, if the lhs is false, we want to jump around the code to
     *   evaluate the rhs, saving the false result if we're not discarding
     *   the overall result.  
     */
    lbl = gen_jump_ahead(discard ? OPC_JF : OPC_JSF);

    /* 
     *   Evaluate the right-hand side.  We don't need to save the result
     *   unless we need the result of the overall expression.  
     */
    right_->gen_code(discard, TRUE);
 
    /* define the label for the jump over the rhs */
    def_label_pos(lbl);

    /*
     *   If we discarded the result, we generated a JF which explicitly
     *   popped a value.  If we didn't discard the result, we generated a
     *   JSF; this may or may not pop the value.  However, if it doesn't
     *   pop the value (save on false), it will bypass the right side
     *   evaluation, and will thus "pop" that value in the sense that it
     *   will never be pushed.  So, note a pop either way.  
     */
    G_cg->note_pop();

    /* 
     *   if the result is not going to be used directly for a condition,
     *   we must boolean-ize the value 
     */
    if (!for_condition)
        G_cg->write_op(OPC_BOOLIZE);
}

/*
 *   Generate code for the short-circuit AND when used in a condition.  We
 *   can use the fact that we're being used conditionally to avoid actually
 *   pushing the result value onto the stack, instead simply branching to the
 *   appropriate point in the enclosing control structure instead.  
 */
void CTPNAnd::gen_code_cond(CTcCodeLabel *then_label,
                            CTcCodeLabel *else_label)
{
    CTcCodeLabel *internal_else;

    /*
     *   First, generate the conditional code for our left operand.  If the
     *   condition is false, we can short-circuit the rest of the expression
     *   by jumping directly to the 'else' label.  If the caller provided an
     *   'else' label, we can jump directly to the caller's 'else' label;
     *   otherwise, we must synthesize our own internal label, which we'll
     *   define at the end of our generated code so that we'll fall through
     *   on false to the enclosing code.  In any case, we want to fall
     *   through if the condition is true, so that control will flow to the
     *   code for our right operand if the left operand is true.  
     */
    internal_else = (else_label == 0 ? G_cs->new_label_fwd() : else_label);
    left_->gen_code_cond(0, internal_else);

    /* 
     *   Now, generate code for our right operand.  We can generate this code
     *   using the caller's destination labels directly: if we reach this
     *   code at all, it's because the left operand was true, in which case
     *   the result is simply the value of the right operand.  
     */
    right_->gen_code_cond(then_label, else_label);

    /* 
     *   If we created an internal 'else' label, it goes at the end of our
     *   generated code: this ensures that we fall off the end of our code
     *   if the left subexpression is false, which is what the caller told
     *   us they wanted when they gave us a null 'else' label.  If the
     *   caller gave us an explicit 'else' label, we'll have jumped there
     *   directly if the first subexpression was false.  
     */
    if (else_label == 0)
        def_label_pos(internal_else);
}


/* ------------------------------------------------------------------------ */
/*
 *   bitwise OR
 */
void CTPNBOr::gen_code(int discard, int)
{
    gen_binary(OPC_BOR, discard, FALSE);
}

/* ------------------------------------------------------------------------ */
/*
 *   bitwise AND
 */
void CTPNBAnd::gen_code(int discard, int)
{
    gen_binary(OPC_BAND, discard, FALSE);
}

/* ------------------------------------------------------------------------ */
/*
 *   bitwise XOR
 */
void CTPNBXor::gen_code(int discard, int)
{
    gen_binary(OPC_XOR, discard, FALSE);
}

/* ------------------------------------------------------------------------ */
/*
 *   greater-than
 */
void CTPNGt::gen_code(int discard, int)
{
    gen_binary(OPC_GT, discard, FALSE);
}

/* ------------------------------------------------------------------------ */
/*
 *   greater-or-equal
 */
void CTPNGe::gen_code(int discard, int)
{
    gen_binary(OPC_GE, discard, FALSE);
}

/* ------------------------------------------------------------------------ */
/*
 *   less-than
 */
void CTPNLt::gen_code(int discard, int)
{
    gen_binary(OPC_LT, discard, FALSE);
}

/* ------------------------------------------------------------------------ */
/*
 *   less-or-equal
 */
void CTPNLe::gen_code(int discard, int)
{
    gen_binary(OPC_LE, discard, FALSE);
}

/* ------------------------------------------------------------------------ */
/*
 *   compare for equality
 */
void CTPNEq::gen_code(int discard, int)
{
    gen_binary(OPC_EQ, discard, FALSE);
}

/* ------------------------------------------------------------------------ */
/*
 *   compare for inequality
 */
void CTPNNe::gen_code(int discard, int)
{
    gen_binary(OPC_NE, discard, FALSE);
}

/* ------------------------------------------------------------------------ */
/*
 *   'is in' 
 */
void CTPNIsIn::gen_code(int discard, int)
{
    CTPNArglist *lst;
    CTPNArg *arg;
    CTcCodeLabel *lbl_found;
    CTcCodeLabel *lbl_done;

    /* allocate our 'found' label */
    lbl_found = G_cs->new_label_fwd();

    /* 
     *   allocate our 'done' label - we only need to do this if we don't
     *   have a constant true value and we're not discarding the result
     */
    if (!const_true_ && !discard)
        lbl_done = G_cs->new_label_fwd();

    /* generate my left-side expression */
    left_->gen_code(FALSE, FALSE);

    /* the right side is always an argument list */
    lst = (CTPNArglist *)right_;

    /* compare to each element in the list on the right */
    for (arg = lst->get_arg_list_head() ; arg != 0 ;
         arg = arg->get_next_arg())
    {
        /* 
         *   duplicate the left-side value, so we don't have to generate
         *   it again for this comparison 
         */
        G_cg->write_op(OPC_DUP);

        /* generate this list element */
        arg->gen_code(FALSE, FALSE);

        /* if they're equal, jump to the 'found' label */
        G_cg->write_op(OPC_JE);
        G_cs->write_ofs2(lbl_found, 0);

        /* we pushed one more (DUP) and popped two (JE) */
        G_cg->note_push(1);
        G_cg->note_pop(2);
    }

    /* 
     *   Generate the code that comes at the end of all of tests when we
     *   fail to find any matches - we simply discard the left-side value
     *   from the stack, push our 'nil' value, and jump to the end label.
     *   
     *   If we have a constant 'true' value, there's no need to do any of
     *   this, because we know that, even after testing all of our
     *   non-constant values, there's a constant value that makes the
     *   entire expression true, and we can thus just fall through to the
     *   'found' code.
     *   
     *   If we're discarding the result, there's no need to push a
     *   separate value for the result, so we can just fall through to the
     *   common ending code in this case.  
     */
    if (!const_true_ && !discard)
    {
        G_cg->write_op(OPC_DISC);
        G_cg->write_op(OPC_PUSHNIL);
        G_cg->write_op(OPC_JMP);
        G_cs->write_ofs2(lbl_done, 0);
    }

    /* 
     *   Generate the 'found' code - this discards the left-side value and
     *   pushes our 'true' result.  Note that there's no reason to push
     *   our result if we're discarding it.  
     */
    def_label_pos(lbl_found);
    G_cg->write_op(OPC_DISC);

    /* 
     *   if we're discarding the result, just note the pop of the left
     *   value; otherwise, push our result 
     */
    if (discard)
        G_cg->note_pop();
    else
        G_cg->write_op(OPC_PUSHTRUE);

    /* our 'done' label is here, if we needed one */
    if (!const_true_ && !discard)
        def_label_pos(lbl_done);
}

/* ------------------------------------------------------------------------ */
/*
 *   'not in' 
 */
void CTPNNotIn::gen_code(int discard, int)
{
    CTPNArglist *lst;
    CTPNArg *arg;
    CTcCodeLabel *lbl_found;
    CTcCodeLabel *lbl_done;

    /* allocate our 'found' label */
    lbl_found = G_cs->new_label_fwd();

    /* 
     *   allocate our 'done' label - we only need to do this if we don't
     *   have a constant false value 
     */
    if (!const_false_ && !discard)
        lbl_done = G_cs->new_label_fwd();

    /* generate my left-side expression */
    left_->gen_code(FALSE, FALSE);

    /* the right side is always an argument list */
    lst = (CTPNArglist *)right_;

    /* compare to each element in the list on the right */
    for (arg = lst->get_arg_list_head() ; arg != 0 ;
         arg = arg->get_next_arg())
    {
        /* 
         *   duplicate the left-side value, so we don't have to generate
         *   it again for this comparison 
         */
        G_cg->write_op(OPC_DUP);

        /* generate this list element */
        arg->gen_code(FALSE, FALSE);

        /* if they're equal, jump to the 'found' label */
        G_cg->write_op(OPC_JE);
        G_cs->write_ofs2(lbl_found, 0);

        /* we pushed one more (DUP) and popped two (JE) */
        G_cg->note_push(1);
        G_cg->note_pop(2);
    }

    /* 
     *   Generate the code that comes at the end of all of tests when we
     *   fail to find any matches - we simply discard the left-side value
     *   from the stack, push our 'true' value, and jump to the end label.
     *   
     *   If we have a constant 'nil' value, however, there's no need to do
     *   any of this, because we know that, even after testing all of our
     *   non-constant values, there's a matching constant value that makes
     *   the entire expression false (because 'not in' is false if we find
     *   a match), and we can thus just fall through to the 'found' code.  
     */
    if (!const_false_ && !discard)
    {
        G_cg->write_op(OPC_DISC);
        G_cg->write_op(OPC_PUSHTRUE);
        G_cg->write_op(OPC_JMP);
        G_cs->write_ofs2(lbl_done, 0);
    }

    /* 
     *   generate the 'found' code - this discards the left-side value and
     *   pushes our 'nil' result (because the result of 'not in' is false
     *   if we found the value) 
     */
    def_label_pos(lbl_found);
    G_cg->write_op(OPC_DISC);

    /* push the result, or note the pop if we're just discarding it */
    if (discard)
        G_cg->note_pop();
    else
        G_cg->write_op(OPC_PUSHNIL);

    /* our 'done' label is here, if we needed one */
    if (!const_false_ && !discard)
        def_label_pos(lbl_done);
}

/* ------------------------------------------------------------------------ */
/*
 *   bit-shift left
 */
void CTPNShl::gen_code(int discard, int)
{
    gen_binary(OPC_SHL, discard, FALSE);
}

/* ------------------------------------------------------------------------ */
/*
 *   bit-shift right
 */
void CTPNShr::gen_code(int discard, int)
{
    gen_binary(OPC_SHR, discard, FALSE);
}

/* ------------------------------------------------------------------------ */
/*
 *   multiply
 */
void CTPNMul::gen_code(int discard, int)
{
    /* if either side is zero or one, we can apply special handling */
    if (left_->is_const_int(0))
    {
        /* evaluate the right for side effects and discard the result */
        right_->gen_code(TRUE, TRUE);

        /* the result is zero */
        G_cg->write_op(OPC_PUSH_0);
        G_cg->note_push();

        /* done */
        return;
    }
    else if (right_->is_const_int(0))
    {
        /* evaluate the left for side effects and discard the result */
        left_->gen_code(TRUE, TRUE);

        /* the result is zero */
        G_cg->write_op(OPC_PUSH_0);
        G_cg->note_push();

        /* done */
        return;
    }
    else if (left_->is_const_int(1))
    {
        /* 
         *   evaluate the right side - it's the result; note that, because
         *   of the explicit multiplication, we must compute logical
         *   results using assignment (not 'for condition') rules 
         */
        right_->gen_code(discard, FALSE);

        /* done */
        return;
    }
    else if (right_->is_const_int(1))
    {
        /* evaluate the right side - it's the result */
        left_->gen_code(discard, FALSE);

        /* done */
        return;
    }

    /* apply generic handling */
    gen_binary(OPC_MUL, discard, FALSE);
}

/* ------------------------------------------------------------------------ */
/*
 *   divide
 */
void CTPNDiv::gen_code(int discard, int for_cond)
{
    /* if dividing by 1, we can skip the whole thing (except side effects) */
    if (right_->is_const_int(1))
    {
        /* 
         *   simply generate the left side for side effects; actually
         *   doing the arithmetic has no effect 
         */
        left_->gen_code(discard, for_cond);
        return;
    }

    /* if the left side is zero, the result is always zero */
    if (left_->is_const_int(0))
    {
        /* evaluate the right for side effects, but discard the result */
        right_->gen_code(TRUE, TRUE);

        /* the result is zero */
        G_cg->write_op(OPC_PUSH_0);
        G_cg->note_push();
        return;
    }

    /* use generic code generation */
    gen_binary(OPC_DIV, discard, FALSE);
}

/* ------------------------------------------------------------------------ */
/*
 *   modulo
 */
void CTPNMod::gen_code(int discard, int for_condition)
{
    /* if dividing by 1, we can skip the whole thing (except side effects) */
    if (right_->is_const_int(1))
    {
        /* 
         *   simply generate the left side for side effects; actually
         *   doing the arithmetic has no effect 
         */
        left_->gen_code(discard, for_condition);

        /* the result is zero */
        G_cg->write_op(OPC_PUSH_0);
        G_cg->note_push();
        return;
    }

    /* if the left side is zero, the result is always zero */
    if (left_->is_const_int(0))
    {
        /* evaluate the right for side effects, but discard the result */
        right_->gen_code(TRUE, TRUE);

        /* the result is zero */
        G_cg->write_op(OPC_PUSH_0);
        G_cg->note_push();
        return;
    }

    /* use generic processing */
    gen_binary(OPC_MOD, discard, FALSE);
}

/* ------------------------------------------------------------------------ */
/*
 *   subtract
 */
void CTPNSub::gen_code(int discard, int for_cond)
{
    /* check for subtracting 1, which we can accomplish more efficiently */
    if (right_->is_const_int(1))
    {
        /* 
         *   We're subtracting one - use decrement.  The decrement
         *   operator itself has no side effects, so we can pass through
         *   the 'discard' status to the subnode.  
         */
        left_->gen_code(discard, FALSE);

        /* apply decrement if we're not discarding the result */
        if (!discard)
            G_cg->write_op(OPC_DEC);
    }
    else
    {
        /* we can't do anything special - use the general-purpose code */
        gen_binary(OPC_SUB, discard, FALSE);
    }
}

/* ------------------------------------------------------------------------ */
/*
 *   add
 */
void CTPNAdd::gen_code(int discard, int)
{
    /* check for adding 1, which we can accomplish more efficiently */
    if (right_->is_const_int(1))
    {
        /* 
         *   We're adding one - use increment.  The increment operator
         *   itself has no side effects, so we can pass through the
         *   'discard' status to the subnode.  
         */
        left_->gen_code(discard, FALSE);
        
        /* apply increment if we're not discarding the result */
        if (!discard)
            G_cg->write_op(OPC_INC);
    }
    else
    {
        /* we can't do anything special - use the general-purpose code */
        gen_binary(OPC_ADD, discard, FALSE);
    }
}

/* ------------------------------------------------------------------------ */
/*
 *   simple assignment
 */
void CTPNAsi::gen_code(int discard, int)
{
    /* 
     *   Ask the left subnode to generate a simple assignment to the value
     *   on the right.  Simple assignments cannot be refused, so we don't
     *   need to try to do any assignment work ourselves.  
     */
    left_->gen_code_asi(discard, TC_ASI_SIMPLE, right_, FALSE);
}

/* ------------------------------------------------------------------------ */
/*
 *   add and assign
 */
void CTPNAddAsi::gen_code(int discard, int)
{
    /* 
     *   ask the left subnode to generate an add-and-assign; if it can't,
     *   handle it generically 
     */
    if (!left_->gen_code_asi(discard, TC_ASI_ADD, right_, FALSE))
    {
        /* 
         *   there's no special coding for this assignment type -- compute
         *   the result generically, then assign the result as a simple
         *   assignment, which cannot be refused
         */
        gen_binary(OPC_ADD, FALSE, FALSE);
        left_->gen_code_asi(discard, TC_ASI_SIMPLE, 0, FALSE);
    }
}

/* ------------------------------------------------------------------------ */
/*
 *   subtract and assign
 */
void CTPNSubAsi::gen_code(int discard, int)
{
    /* 
     *   ask the left subnode to generate a subtract-and-assign; if it
     *   can't, handle it generically 
     */
    if (!left_->gen_code_asi(discard, TC_ASI_SUB, right_, FALSE))
    {
        /* 
         *   there's no special coding for this assignment type -- compute
         *   the result generically, then assign the result as a simple
         *   assignment, which cannot be refused 
         */
        gen_binary(OPC_SUB, FALSE, FALSE);
        left_->gen_code_asi(discard, TC_ASI_SIMPLE, 0, FALSE);
    }
}

/* ------------------------------------------------------------------------ */
/*
 *   multiply and assign
 */
void CTPNMulAsi::gen_code(int discard, int)
{
    /* 
     *   ask the left subnode to generate a multiply-and-assign; if it
     *   can't, handle it generically 
     */
    if (!left_->gen_code_asi(discard, TC_ASI_MUL, right_, FALSE))
    {
        /* 
         *   there's no special coding for this assignment type -- compute
         *   the result generically, then assign the result as a simple
         *   assignment, which cannot be refused 
         */
        gen_binary(OPC_MUL, FALSE, FALSE);
        left_->gen_code_asi(discard, TC_ASI_SIMPLE, 0, FALSE);
    }
}

/* ------------------------------------------------------------------------ */
/*
 *   divide and assign
 */
void CTPNDivAsi::gen_code(int discard, int)
{
    /* 
     *   ask the left subnode to generate a divide-and-assign; if it
     *   can't, handle it generically 
     */
    if (!left_->gen_code_asi(discard, TC_ASI_DIV, right_, FALSE))
    {
        /* 
         *   there's no special coding for this assignment type -- compute
         *   the result generically, then assign the result as a simple
         *   assignment, which cannot be refused 
         */
        gen_binary(OPC_DIV, FALSE, FALSE);
        left_->gen_code_asi(discard, TC_ASI_SIMPLE, 0, FALSE);
    }
}

/* ------------------------------------------------------------------------ */
/*
 *   modulo and assign
 */
void CTPNModAsi::gen_code(int discard, int)
{
    /* 
     *   ask the left subnode to generate a mod-and-assign; if it can't,
     *   handle it generically 
     */
    if (!left_->gen_code_asi(discard, TC_ASI_MOD, right_, FALSE))
    {
        /* 
         *   there's no special coding for this assignment type -- compute
         *   the result generically, then assign the result as a simple
         *   assignment, which cannot be refused 
         */
        gen_binary(OPC_MOD, FALSE, FALSE);
        left_->gen_code_asi(discard, TC_ASI_SIMPLE, 0, FALSE);
    }
}

/* ------------------------------------------------------------------------ */
/*
 *   bitwise-AND and assign
 */
void CTPNBAndAsi::gen_code(int discard, int)
{
    /* 
     *   ask the left subnode to generate an AND-and-assign; if it can't,
     *   handle it generically 
     */
    if (!left_->gen_code_asi(discard, TC_ASI_BAND, right_, FALSE))
    {
        /* 
         *   there's no special coding for this assignment type -- compute
         *   the result generically, then assign the result as a simple
         *   assignment, which cannot be refused 
         */
        gen_binary(OPC_BAND, FALSE, FALSE);
        left_->gen_code_asi(discard, TC_ASI_SIMPLE, 0, FALSE);
    }
}

/* ------------------------------------------------------------------------ */
/*
 *   bitwise-OR and assign
 */
void CTPNBOrAsi::gen_code(int discard, int)
{
    /* 
     *   ask the left subnode to generate an OR-and-assign; if it can't,
     *   handle it generically 
     */
    if (!left_->gen_code_asi(discard, TC_ASI_BOR, right_, FALSE))
    {
        /* 
         *   there's no special coding for this assignment type -- compute
         *   the result generically, then assign the result as a simple
         *   assignment, which cannot be refused 
         */
        gen_binary(OPC_BOR, FALSE, FALSE);
        left_->gen_code_asi(discard, TC_ASI_SIMPLE, 0, FALSE);
    }
}

/* ------------------------------------------------------------------------ */
/*
 *   bitwise-XOR and assign
 */
void CTPNBXorAsi::gen_code(int discard, int)
{
    /* 
     *   ask the left subnode to generate an XOR-and-assign; if it can't,
     *   handle it generically 
     */
    if (!left_->gen_code_asi(discard, TC_ASI_BXOR, right_, FALSE))
    {
        /* 
         *   there's no special coding for this assignment type -- compute
         *   the result generically, then assign the result as a simple
         *   assignment, which cannot be refused 
         */
        gen_binary(OPC_XOR, FALSE, FALSE);
        left_->gen_code_asi(discard, TC_ASI_SIMPLE, 0, FALSE);
    }
}

/* ------------------------------------------------------------------------ */
/*
 *   bit-shift left and assign
 */
void CTPNShlAsi::gen_code(int discard, int)
{
    /* 
     *   ask the left subnode to generate an shift-left-and-assign; if it
     *   can't, handle it generically 
     */
    if (!left_->gen_code_asi(discard, TC_ASI_SHL, right_, FALSE))
    {
        /* 
         *   there's no special coding for this assignment type -- compute
         *   the result generically, then assign the result as a simple
         *   assignment, which cannot be refused 
         */
        gen_binary(OPC_SHL, FALSE, FALSE);
        left_->gen_code_asi(discard, TC_ASI_SIMPLE, 0, FALSE);
    }
}

/* ------------------------------------------------------------------------ */
/*
 *   bit-shift right and assign
 */
void CTPNShrAsi::gen_code(int discard, int)
{
    /* 
     *   ask the left subnode to generate a shift-right-and-assign; if it
     *   can't, handle it generically 
     */
    if (!left_->gen_code_asi(discard, TC_ASI_SHR, right_, FALSE))
    {
        /* 
         *   there's no special coding for this assignment type -- compute
         *   the result generically, then assign the result as a simple
         *   assignment, which cannot be refused 
         */
        gen_binary(OPC_SHR, FALSE, FALSE);
        left_->gen_code_asi(discard, TC_ASI_SIMPLE, 0, FALSE);
    }
}

/* ------------------------------------------------------------------------ */
/*
 *   subscript a list/array value
 */
void CTPNSubscript::gen_code(int discard, int)
{
    gen_binary(OPC_INDEX, discard, FALSE);
}

/*
 *   assign to a subscripted value
 */
int CTPNSubscript::gen_code_asi(int discard, tc_asitype_t typ,
                                CTcPrsNode *rhs, int)
{
    /* 
     *   If this isn't a simple assignment, tell the caller to emit the
     *   generic code to compute the composite result, then call us again
     *   for a simple assignment.  We can't add any value with specialized
     *   instructions for composite assignments, so there's no point in
     *   dealing with those here. 
     */
    if (typ != TC_ASI_SIMPLE)
        return FALSE;
    
    /* 
     *   Generate the value to assign to the element - that's the right
     *   side of the assignment operator.  If rhs is null, it means the
     *   caller has already done this.  
     */
    if (rhs != 0)
        rhs->gen_code(FALSE, FALSE);

    /* 
     *   if we're not discarding the result, duplicate the value to be
     *   assigned, so that it's left on the stack after we're finished
     *   (this is necessary because we'll consume one copy with the SETIND
     *   instruction) 
     */
    if (!discard)
    {
        G_cg->write_op(OPC_DUP);
        G_cg->note_push();
    }
    
    /* generate the value to be subscripted - that's my left-hand side */
    left_->gen_code(FALSE, FALSE);

    /* generate the index value - that's my right-hand side */
    right_->gen_code(FALSE, FALSE);

    /* generate the assign-to-indexed-value opcode */
    G_cg->write_op(OPC_SETIND);

    /* setind pops three and pushes one - net of pop 2 */
    G_cg->note_pop(2);

    /*
     *   The top value now on the stack is the new container value.  The new
     *   container will be different from the old container in some cases
     *   (with lists, for example, because we must create a new list object
     *   to contain the modified list value).  Therefore, if my left-hand
     *   side is an lvalue, we must assign the new container to the left-hand
     *   side - this makes something like "x[1] = 5" actually change the
     *   value in "x" if "x" is a local variable.  If my left-hand side isn't
     *   an lvalue, don't bother with this step, and simply discard the new
     *   container value.
     *   
     *   Regardless of whether we're keeping the result of the overall
     *   expression, we're definitely not keeping the result of assigning the
     *   new container - the result of the assignment is the value assigned,
     *   not the container.  Thus, discard = true in this call.  
     *   
     *   There's a special case that's handled through the peep-hole
     *   optimizer: if we are assigning to a local variable and indexing with
     *   a constant integer value, we will have converted the whole operation
     *   to a SETINDLCL1I8.  That instruction takes care of assigning the
     *   value back to the rvalue, so we don't need to generate a separate
     *   rvalue assignment.  
     */
    if (G_cg->get_last_op() == OPC_SETINDLCL1I8)
    {
        /* 
         *   no assignment is necessary - we just need to account for the
         *   difference in the stack arrangement with this form of the
         *   assignment, which is that we don't leave the value on the stack 
         */
        G_cg->note_pop();
    }
    else if (!left_->gen_code_asi(TRUE, TC_ASI_SIMPLE, 0, TRUE))
    {
        /* no assignment is possible; discard the new container value */
        G_cg->write_op(OPC_DISC);
        G_cg->note_pop();
    }

    /* handled */
    return TRUE;
}

/* ------------------------------------------------------------------------ */
/*
 *   conditional operator
 */
void CTPNIf::gen_code(int discard, int for_condition)
{
    CTcCodeLabel *lbl_else;
    CTcCodeLabel *lbl_end;

    /* 
     *   Generate the condition value - we need the value regardless of
     *   whether the overall result is going to be used, because we need
     *   it to determine which branch to take.  Generate the subexpression
     *   for a condition, so that we don't perform any extra unnecessary
     *   conversions on it.  
     */
    first_->gen_code(FALSE, TRUE);
 
    /* if the condition is false, jump to the 'else' expression part */
    lbl_else = gen_jump_ahead(OPC_JF);

    /* JF pops a value */
    G_cg->note_pop();

    /* 
     *   Generate the 'then' expression part.  We need its value only if
     *   we're keeping the value of the overall expression, so pass
     *   through the 'discard' value.  Likewise, pass through
     *   'for_condition', since this result is our result.  
     */
    second_->gen_code(discard, for_condition);

    /* 
     *   If this expression has no return value but the other one does,
     *   push nil so that we keep the stack balanced in both branches.
     */
    if (!second_->has_return_value() && third_->has_return_value())
    {
        G_cg->write_op(OPC_PUSHNIL);
        G_cg->note_push();
    }
 
    /* unconditionally jump over the 'else' part */
    lbl_end = gen_jump_ahead(OPC_JMP);

    /* set the label for the 'else' part */
    def_label_pos(lbl_else);

    /* 
     *   Generate the 'else' part.  Pass through 'discard' and
     *   'for_condition', since this result is our result.  
     */
    third_->gen_code(discard, for_condition);

    /* 
     *   If this expression has no return value but the other one does,
     *   push nil so that we keep the stack balanced in both branches.  
     */
    if (!third_->has_return_value() && second_->has_return_value())
    {
        G_cg->write_op(OPC_PUSHNIL);
        G_cg->note_push();
    }
 
    /* 
     *   Because of the jump, we only evaluate one of the two expressions
     *   we generated, so note an extra pop for the branch we didn't take.
     *   Note that if either one pushes a value, both will, since we'll
     *   explicitly have pushed nil for the one that doesn't generate a
     *   value to keep the stack balanced on both branches.
     *   
     *   If neither of our expressions yields a value, don't pop anything
     *   extra, since we won't think we've pushed two values in the course
     *   of generating the two expressions.  
     */
    if (second_->has_return_value() || third_->has_return_value())
        G_cg->note_pop();
 
    /* set the label for the end of the expression */
    def_label_pos(lbl_end);
}

/* ------------------------------------------------------------------------ */
/*
 *   symbol
 */
void CTPNSym::gen_code(int discard, int)
{
    /* 
     *   Look up the symbol; if it's undefined, add a default property
     *   symbol entry if possible.  Then ask the symbol to generate the
     *   code.  
     */
    G_cs->get_symtab()
        ->find_or_def_prop_implied(get_sym_text(), get_sym_text_len(),
                                   FALSE, G_cs->is_self_available())
        ->gen_code(discard);
}

/*
 *   assign to a symbol
 */
int CTPNSym::gen_code_asi(int discard, tc_asitype_t typ, CTcPrsNode *rhs,
                          int ignore_errors)
{
    /* 
     *   Look up the symbol; if it's undefined and there's a "self" object
     *   available, define it as a property by default, since a property
     *   is the only kind of symbol that we could possibly assign to
     *   without having defined anywhere in the program.  Once we have the
     *   symbol, tell it to generate the code for assigning to it.  
     */
    return G_cs->get_symtab()
        ->find_or_def_prop_implied(get_sym_text(), get_sym_text_len(),
                                   FALSE, G_cs->is_self_available())
        ->gen_code_asi(discard, typ, rhs, ignore_errors);
}

/*
 *   take the address of the symbol 
 */
void CTPNSym::gen_code_addr()
{
    /* 
     *   Look up our symbol in the symbol table, then ask the resulting
     *   symbol to generate the appropriate code.  If the symbol isn't
     *   defined, and we have a "self" object available (i.e., we're in
     *   method code), define the symbol by default as a property.
     *   
     *   Note that we look only in the global symbol table, because local
     *   symbols have no address value.  So, even if the symbol is defined
     *   in the local table, ignore the local definition and look at the
     *   global definition.  
     */
    G_prs->get_global_symtab()
        ->find_or_def_prop_explicit(get_sym_text(), get_sym_text_len(),
                                    FALSE)
        ->gen_code_addr();
}

/*
 *   call the symbol 
 */
void CTPNSym::gen_code_call(int discard, int argc, int varargs)
{
    /*
     *   Look up our symbol in the symbol table, then ask the resulting
     *   symbol to generate the appropriate call.  The symbol is
     *   implicitly a property (if in a method context), since that's the
     *   only kind of undefined symbol that we could be calling.  
     */
    G_cs->get_symtab()
        ->find_or_def_prop_implied(get_sym_text(), get_sym_text_len(),
                                   FALSE, G_cs->is_self_available())
        ->gen_code_call(discard, argc, varargs);
}

/*
 *   generate code for 'new' 
 */
void CTPNSym::gen_code_new(int discard, int argc, int varargs,
                           int /*from_call*/, int is_transient)
{
    /*
     *   Look up our symbol, then ask the resulting symbol to generate the
     *   'new' code.  If the symbol is undefined, add an 'undefined' entry
     *   to the table; we can't implicitly create an object symbol. 
     */
    G_cs->get_symtab()
        ->find_or_def_undef(get_sym_text(), get_sym_text_len(), FALSE)
        ->gen_code_new(discard, argc, varargs, is_transient);
}

/*
 *   generate a property ID expression 
 */
vm_prop_id_t CTPNSym::gen_code_propid(int check_only, int is_expr)
{
    CTcSymbol *sym;
    CTcPrsSymtab *symtab;

    /*
     *   Figure out where to look for the symbol.  If the symbol was given
     *   as an expression (in other words, it was explicitly enclosed in
     *   parentheses), look it up in the local symbol table, since it
     *   could refer to a local.  Otherwise, it must refer to a property,
     *   so look only in the global table.
     *   
     *   If the symbol isn't defined already, define it as a property now.
     *   Because the symbol is explicitly on the right side of a member
     *   evaluation, we can define it as a property whether or not there's
     *   a valid "self" in this context.  
     */
    if (is_expr)
    {
        /* it's an expression - look it up in the local symbol table */
        symtab = G_cs->get_symtab();
    }
    else
    {
        /* it's a simple symbol - look only in the global symbol table */
        symtab = G_prs->get_global_symtab();
    }

    /* 
     *   look it up (note that this will always return a valid symbol,
     *   since it will create one if we can't find an existing entry) 
     */
    sym = symtab->find_or_def_prop(get_sym_text(), get_sym_text_len(), FALSE);

    /* ask the symbol to generate the property reference */
    return sym->gen_code_propid(check_only, is_expr);
}

/*
 *   generate code for a member expression 
 */
void CTPNSym::gen_code_member(int discard, CTcPrsNode *prop_expr,
                              int prop_is_expr, int argc, int varargs)
{
    /* 
     *   Look up the symbol, and let it do the work.  There's no
     *   appropriate default for the symbol, so leave it undefined if we
     *   can't find it. 
     */
    G_cs->get_symtab()
        ->find_or_def_undef(get_sym_text(), get_sym_text_len(), FALSE)
        ->gen_code_member(discard, prop_expr, prop_is_expr, argc, varargs);
}

/*
 *   generate code for an object before a '.'  
 */
vm_obj_id_t CTPNSym::gen_code_obj_predot(int *is_self)
{
    /* 
     *   Look up the symbol, and let it do the work.  There's no default
     *   type for the symbol, so leave it undefined if we don't find it. 
     */
    return G_cs->get_symtab()
        ->find_or_def_undef(get_sym_text(), get_sym_text_len(), FALSE)
        ->gen_code_obj_predot(is_self);
}

/* ------------------------------------------------------------------------ */
/*
 *   resolved symbol 
 */
void CTPNSymResolved::gen_code(int discard, int)
{
    /* let the symbol handle it */
    sym_->gen_code(discard);
}

/*
 *   assign to a symbol 
 */
int CTPNSymResolved::gen_code_asi(int discard, tc_asitype_t typ,
                                  CTcPrsNode *rhs,
                                  int ignore_errors)
{
    /* let the symbol handle it */
    return sym_->gen_code_asi(discard, typ, rhs, ignore_errors);
}

/*
 *   take the address of the symbol 
 */
void CTPNSymResolved::gen_code_addr()
{
    /* let the symbol handle it */
    sym_->gen_code_addr();
}

/*
 *   call the symbol 
 */
void CTPNSymResolved::gen_code_call(int discard, int argc, int varargs)
{
    /* let the symbol handle it */
    sym_->gen_code_call(discard, argc, varargs);
}

/*
 *   generate code for 'new' 
 */
void CTPNSymResolved::gen_code_new(int discard, int argc, int varargs,
                                   int /*from_call*/, int is_transient)
{
    /* let the symbol handle it */
    sym_->gen_code_new(discard, argc, varargs, is_transient);
}

/*
 *   generate a property ID expression 
 */
vm_prop_id_t CTPNSymResolved::gen_code_propid(int check_only, int is_expr)
{
    /* let the symbol handle it */
    return sym_->gen_code_propid(check_only, is_expr);
}

/*
 *   generate code for a member expression 
 */
void CTPNSymResolved::gen_code_member(int discard, 
                                      CTcPrsNode *prop_expr, int prop_is_expr,
                                      int argc, int varargs)
{
    /* let the symbol handle it */
    sym_->gen_code_member(discard, prop_expr, prop_is_expr, argc, varargs);
}

/*
 *   generate code for an object before a '.'  
 */
vm_obj_id_t CTPNSymResolved::gen_code_obj_predot(int *is_self)
{
    /* let the symbol handle it */
    return sym_->gen_code_obj_predot(is_self);
}

/* ------------------------------------------------------------------------ */
/*
 *   Debugger local variable symbol 
 */

/*
 *   generate code to evaluate the variable
 */
void CTPNSymDebugLocal::gen_code(int discard, int for_condition)
{
    /* if we're not discarding the value, push the local */
    if (!discard)
    {
        /* generate the debugger local/parameter variable instruction */
        G_cg->write_op(is_param_ ? OPC_GETDBARG : OPC_GETDBLCL);
        G_cs->write2(var_id_);
        G_cs->write2(frame_idx_);

        /* note that we pushed the value */
        G_cg->note_push();

        /* if it's a context local, get the value from the context array */
        if (ctx_arr_idx_ != 0)
        {
            CTPNConst::s_gen_code_int(ctx_arr_idx_);
            G_cg->write_op(OPC_INDEX);

            /* 
             *   the 'index' operation pops two values and pushes one, for a
             *   net of one pop 
             */
            G_cg->note_pop();
        }
    }
}

/*
 *   generate code for assigning to this variable 
 */
int CTPNSymDebugLocal::gen_code_asi(int discard, tc_asitype_t typ,
                                    CTcPrsNode *rhs, int ignore_error)    
{
    /* 
     *   if this isn't a simple assignment, use the generic combination
     *   assignment computation 
     */
    if (typ != TC_ASI_SIMPLE)
        return FALSE;

    /* generate the value to be assigned */
    if (rhs != 0)
        rhs->gen_code(FALSE, FALSE);

    /* 
     *   if we're not discarding the result, duplicate the value so we'll
     *   have a copy after the assignment 
     */
    if (!discard)
    {
        G_cg->write_op(OPC_DUP);
        G_cg->note_push();
    }

    /* check for a context property */
    if (ctx_arr_idx_ == 0)
    {
        /* 
         *   generate the debug-local-set instruction - the operands are
         *   the variable number and the stack frame index 
         */
        G_cg->write_op(is_param_ ? OPC_SETDBARG : OPC_SETDBLCL);
        G_cs->write2(var_id_);
        G_cs->write2(frame_idx_);
    }
    else
    {
        /* get the local containing our context object */
        G_cg->write_op(OPC_GETDBLCL);
        G_cs->write2(var_id_);
        G_cs->write2(frame_idx_);

        /* set the actual variable value in the context object */
        CTPNConst::s_gen_code_int(ctx_arr_idx_);
        G_cg->write_op(OPC_SETIND);
        G_cg->write_op(OPC_DISC);

        /* 
         *   we did three pops (SETIND), then a push (SETIND), then a pop
         *   (DISC) - this is a net of three extra pops
         */
        G_cg->note_pop(3);
    }

    /* the debug-local-set removes the rvalue from the stack */
    G_cg->note_pop();

    /* handled */
    return TRUE;
}

/* ------------------------------------------------------------------------ */
/*
 *   Double-quoted string.  The 'discard' status is irrelevant, because we
 *   evaluate double-quoted strings for their side effects.  
 */
void CTPNDstr::gen_code(int, int)
{
    /* generate the instruction to display it */
    G_cg->write_op(OPC_SAY);

    /* add the string to the constant pool, creating a fixup here */
    G_cg->add_const_str(str_, len_, G_cs, G_cs->get_ofs());

    /* write a placeholder value, which will be corrected by the fixup */
    G_cs->write4(0);
}

/* ------------------------------------------------------------------------ */
/*
 *   Double-quoted debug string 
 */
void CTPNDebugDstr::gen_code(int, int)
{
    /* generate code to push the in-line string */
    G_cg->write_op(OPC_PUSHSTRI);
    G_cs->write2(len_);
    G_cs->write(str_, len_);

    /* write code to display the value */
    G_cg->write_op(OPC_SAYVAL);

    /* note that we pushed the string and then popped it */
    G_cg->note_push();
    G_cg->note_pop();
}

/* ------------------------------------------------------------------------ */
/*
 *   Double-quoted string embedding 
 */

/*
 *   create an embedding 
 */
CTPNDstrEmbed::CTPNDstrEmbed(CTcPrsNode *sub)
    : CTPNDstrEmbedBase(sub)
{
}

/*
 *   Generate code for a double-quoted string embedding 
 */
void CTPNDstrEmbed::gen_code(int, int)
{
    int orig_depth;
    int void_expr;

    /* determine if my expression has a return value */
    void_expr = !sub_->has_return_value();
    
    /* note the stack depth before generating the expression */
    orig_depth = G_cg->get_sp_depth();
    
    /* 
     *   Generate the subexpression, keeping the value if it has one.  If
     *   it's a void expression, don't bother asking it to produce a
     *   value. 
     */
    sub_->gen_code(void_expr, FALSE);

    /* 
     *   If the code generation left anything on the stack, generate code
     *   to display the value via the default display function.  
     */
    if (G_cg->get_sp_depth() > orig_depth)
    {
        /* add a SAYVAL instruction */
        G_cg->write_op(OPC_SAYVAL);

        /* SAYVAL pops the argument value */
        G_cg->note_pop();
    }
}


/* ------------------------------------------------------------------------ */
/*
 *   Argument list
 */
void CTPNArglist::gen_code_arglist(int *varargs)
{
    CTPNArg *arg;
    int i;
    int fixed_cnt;
    int pushed_varargs_counter;

    /* 
     *   scan the argument list for varargs - if we have any, we must
     *   treat all of them as varargs 
     */
    for (*varargs = FALSE, fixed_cnt = 0, arg = get_arg_list_head() ;
         arg != 0 ; arg = arg->get_next_arg())
    {
        /* if this is a varargs argument, we have varargs */
        if (arg->is_varargs())
        {
            /* note it */
            *varargs = TRUE;
        }
        else
        {
            /* count another fixed argument */
            ++fixed_cnt;
        }
    }

    /* 
     *   Push each argument in the list - start with the last element and
     *   work backwards through the list to the first element.  The parser
     *   builds the list in reverse order, so we must merely follow the
     *   list from head to tail.
     *   
     *   We need each argument value to be pushed (hence discard = false),
     *   and we need the assignable value of each argument expression
     *   (hence for_condition = false).  
     */
    for (pushed_varargs_counter = FALSE, i = argc_,
         arg = get_arg_list_head() ; arg != 0 ;
         arg = arg->get_next_arg(), --i)
    {
        int depth;

        /* note the stack depth before generating the value */
        depth = G_cg->get_sp_depth();

        /* 
         *   check for varargs - if this is first varargs argument, push
         *   the counter placeholder 
         */
        if (arg->is_varargs() && !pushed_varargs_counter)
        {
            /* 
             *   write code to push the fixed argument count - we can use
             *   this as a starting point, since we always know we have
             *   this many argument to start with; we'll dynamically add
             *   in the variable count at run-time 
             */
            CTPNConst::s_gen_code_int(fixed_cnt);
            
            /* note that we've pushed the counter */
            pushed_varargs_counter = TRUE;

            /* 
             *   we will take the extra value off when we evaluate the
             *   varargs counter, so simply count it as removed now 
             */
            G_cg->note_pop();
        }

        /* generate the argument's code */
        arg->gen_code(FALSE, FALSE);

        /* 
         *   if we've pushed the variable argument counter value onto the
         *   stack, and this a fixed argument, swap the top two stack
         *   elements to get the argument counter back to the top of the
         *   stack; if this is a varargs argument there's no need, since
         *   it will have taken care of this 
         */
        if (pushed_varargs_counter && !arg->is_varargs())
            G_cg->write_op(OPC_SWAP);

        /* ensure that it generated something */
        if (G_cg->get_sp_depth() <= depth)
            G_tok->log_error(TCERR_ARG_EXPR_HAS_NO_VAL, i);
    }
}

/* ------------------------------------------------------------------------ */
/*
 *   argument list entry
 */
void CTPNArg::gen_code(int, int)
{
    /* 
     *   Generate the argument expression.  We need the value (hence
     *   discard = false), and we need the assignable value (hence
     *   for_condition = false). 
     */
    get_arg_expr()->gen_code(FALSE, FALSE);

    /* 
     *   if this is a list-to-varargs conversion, generate the conversion
     *   instruction 
     */
    if (is_varargs_)
    {
        /* write the opcode */
        G_cg->write_op(OPC_MAKELSTPAR);

        /* note the extra push and pop for the argument count */
        G_cg->note_push();
        G_cg->note_pop();
    }
}

/* ------------------------------------------------------------------------ */
/*
 *   function/method call
 */

/*
 *   create 
 */
CTPNCall::CTPNCall(CTcPrsNode *func, class CTPNArglist *arglist)
    : CTPNCallBase(func, arglist)
{
    /* the T3 instruction set limits calls to 127 arguments */
    if (arglist->get_argc() > 127)
        G_tok->log_error(TCERR_TOO_MANY_CALL_ARGS);
}


/*
 *   generate code 
 */
void CTPNCall::gen_code(int discard, int)
{
    int varargs;
    
    /* push the argument list */
    get_arg_list()->gen_code_arglist(&varargs);

    /* generate an appropriate call instruction */
    get_func()->gen_code_call(discard, get_arg_list()->get_argc(),
                              varargs);
}

/*
 *   Generate code for operator 'new'.  A 'new' with an argument list
 *   looks like a function call: NEW(CALL(object-contents, ARGLIST(...))).
 */
void CTPNCall::gen_code_new(int discard, int argc, int varargs,
                            int from_call, int is_transient)
{
    /* 
     *   if this is a recursive call from another 'call' node, it's not
     *   allowed - we'd be trying to use the result of a call as the base
     *   class of the 'new', which is illegal 
     */
    if (from_call)
    {
        G_tok->log_error(TCERR_INVAL_NEW_EXPR);
        return;
    }
    
    /* generate the argument list */
    get_arg_list()->gen_code_arglist(&varargs);

    /* generate the code for the 'new' call */
    get_func()->gen_code_new(discard, get_arg_list()->get_argc(), varargs,
                             TRUE, is_transient);
}


/* ------------------------------------------------------------------------ */
/*
 *   member property evaluation
 */
void CTPNMember::gen_code(int discard, int)
{
    /* ask the object expression to generate the code */
    get_obj_expr()->gen_code_member(discard, get_prop_expr(), prop_is_expr_,
                                    0, FALSE);
}

/*
 *   assign to member expression
 */
int CTPNMember::gen_code_asi(int discard, tc_asitype_t typ, CTcPrsNode *rhs,
                             int ignore_errors)
{
    int is_self;
    vm_obj_id_t obj;
    vm_prop_id_t prop;

    /* 
     *   if it's not a simple assignment, tell the caller to generate the
     *   generic code to compute the composite value, and then call us
     *   again for a simple assignment 
     */
    if (typ != TC_ASI_SIMPLE)
        return FALSE;

    /* generate the right-hand side, unless the caller has already done so */
    if (rhs != 0)
        rhs->gen_code(FALSE, FALSE);
    
    /* 
     *   if the caller wants to use the assigned value, push a copy --
     *   we'll consume one copy in the SETPROP or related instruction, so
     *   we'll need another copy for the caller 
     */
    if (!discard)
    {
        G_cg->write_op(OPC_DUP);
        G_cg->note_push();
    }

    /* 
     *   Determine what we have on the left: we could have self, a
     *   constant object value, or any other expression.  
     */
    obj = get_obj_expr()->gen_code_obj_predot(&is_self);

    /* 
     *   determine what kind of property expression we have - don't
     *   generate any code for now, since we may need to generate some
     *   more code ahead of the property generation 
     */
    prop = get_prop_expr()->gen_code_propid(TRUE, prop_is_expr_);

    /* determine what we need to do based on the operands */
    if (prop == VM_INVALID_PROP)
    {
        /* 
         *   We're assigning through a property pointer -- we must
         *   generate a PTRSETPROP instruction.
         *   
         *   Before we generate the property expression, we must generate
         *   the object expression.  If we got a constant object, we must
         *   generate code to push that object value; otherwise, the code
         *   to generate the object value is already generated. 
         */
        if (is_self)
        {
            /* self - generate code to push the "self" value */
            G_cg->write_op(OPC_PUSHSELF);
            G_cg->note_push();
        }
        else if (obj != VM_INVALID_OBJ)
        {
            /* constant object - generate code to push the value */
            G_cg->write_op(OPC_PUSHOBJ);
            G_cs->write_obj_id(obj);
            G_cg->note_push();
        }

        /* generate the property value expression */
        get_prop_expr()->gen_code_propid(FALSE, prop_is_expr_);

        /* generate the PTRSETPROP instruction */
        G_cg->write_op(OPC_PTRSETPROP);

        /* ptrsetprop removes three elements */
        G_cg->note_pop(3);
    }
    else
    {
        /* 
         *   We have a constant property value, so we have several
         *   instructions to choose from.  If we're assigning to a
         *   property of "self", use SETPROPSELF.  If we're assigning to a
         *   constant object, use OBJSETPROP.  Otherwise, use the plain
         *   SETPROP. 
         */
        if (is_self)
        {
            /* write the SETPROPSELF */
            G_cg->write_op(OPC_SETPROPSELF);
            G_cs->write_prop_id(prop);

            /* setpropself removes the value */
            G_cg->note_pop();
        }
        else if (obj != VM_INVALID_OBJ)
        {
            /* write the OBJSETPROP */
            G_cg->write_op(OPC_OBJSETPROP);
            G_cs->write_obj_id(obj);
            G_cs->write_prop_id(prop);

            /* objsetprop removes the value */
            G_cg->note_pop();
        }
        else
        {
            /* 
             *   write the normal SETPROP; we already generated the code
             *   to push the object value, so it's where it should be 
             */
            G_cg->write_op(OPC_SETPROP);
            G_cs->write_prop_id(prop);

            /* setprop removes the value and the object */
            G_cg->note_pop(2);
        }
    }

    /* handled */
    return TRUE;
}

/* ------------------------------------------------------------------------ */
/*
 *   member with argument list
 */
void CTPNMemArg::gen_code(int discard, int)
{
    int varargs;
    
    /* push the argument list */
    get_arg_list()->gen_code_arglist(&varargs);

    /* ask the object expression to generate the code */
    get_obj_expr()->gen_code_member(discard, get_prop_expr(), prop_is_expr_,
                                    get_arg_list()->get_argc(),
                                    varargs);
}

/* ------------------------------------------------------------------------ */
/*
 *   construct a list
 */
void CTPNList::gen_code(int discard, int for_condition)
{
    CTPNListEle *ele;
    
    /*
     *   Before we construct the list dynamically, check to see if the
     *   list is constant.  If it is, we need only built the list in the
     *   constant pool, and push its offset.  
     */
    if (is_const())
    {
        /* push the value only if we're not discarding it */
        if (!discard)
        {
            /* write the instruction */
            G_cg->write_op(OPC_PUSHLST);

            /* add the list to the constant pool */
            G_cg->add_const_list(this, G_cs, G_cs->get_ofs());

            /* 
             *   write a placeholder address, which will be corrected by
             *   the fixup that add_const_list() created 
             */
            G_cs->write4(0);

            /* note the push */
            G_cg->note_push();
        }

        /* done */
        return;
    }

    /*
     *   It's not a constant list, so we must generate code to construct a
     *   list dynamically.  Push each element of the list.  We need each
     *   value (hence discard = false), and we require the assignable
     *   value of each expression (hence for_condition = false).  Push the
     *   argument list in reverse order, since the run-time metaclass
     *   requires this ordering.  
     */
    for (ele = get_tail() ; ele != 0 ; ele = ele->get_prev())
        ele->gen_code(FALSE, FALSE);

    /* generate a NEW instruction for an object of metaclass LIST */
    if (get_count() <= 255)
    {
        /* the count will fit in one byte - use the short form */
        G_cg->write_op(OPC_NEW1);
        G_cs->write((char)get_count());
        G_cs->write(TCT3_METAID_LIST);
    }
    else
    {
        /* count doesn't fit in one byte - use the long form */
        G_cg->write_op(OPC_NEW2);
        G_cs->write2(get_count());
        G_cs->write2(TCT3_METAID_LIST);
    }

    /* new1/new2 remove arguments */
    G_cg->note_pop(get_count());

    /* if we're not discarding the value, push it */
    if (!discard)
    {
        G_cg->write_op(OPC_GETR0);
        G_cg->note_push();
    }
}

/* ------------------------------------------------------------------------ */
/*
 *   list element
 */
void CTPNListEle::gen_code(int discard, int for_condition)
{
    /* generate the subexpression */
    expr_->gen_code(discard, for_condition);
}

/* ------------------------------------------------------------------------ */
/*
 *   Basic T3-specific symbol class 
 */

/*
 *   generate code to take the address of a symbol - in general, we cannot
 *   take the address of a symbol, so we'll just log an error
 */
void CTcSymbol::gen_code_addr()
{
    G_tok->log_error(TCERR_NO_ADDR_SYM, (int)get_sym_len(), get_sym());
}

/*
 *   generate code to assign to the symbol - in general, we cannot assign
 *   to a symbol, so we'll just log an error 
 */
int CTcSymbol::gen_code_asi(int, tc_asitype_t, class CTcPrsNode *,
                            int ignore_error)
{
    /* 
     *   if we're ignoring errors, simply return false to indicate that
     *   nothing happened 
     */
    if (ignore_error)
        return FALSE;

    /* log the error */
    G_tok->log_error(TCERR_CANNOT_ASSIGN_SYM, (int)get_sym_len(), get_sym());

    /* 
     *   even though we didn't generate anything, this has been fully
     *   handled - the caller shouldn't attempt to generate any additional
     *   code for this 
     */
    return TRUE;
}

/*
 *   Generate code for calling the symbol.  By default, we can't call a
 *   symbol. 
 */
void CTcSymbol::gen_code_call(int, int, int)
{
    /* log an error */
    G_tok->log_error(TCERR_CANNOT_CALL_SYM, (int)get_sym_len(), get_sym());
}

/*
 *   Generate code for operator 'new' 
 */
void CTcSymbol::gen_code_new(int, int, int, int)
{
    G_tok->log_error(TCERR_INVAL_NEW_EXPR);
}

/* 
 *   evaluate a property ID 
 */
vm_prop_id_t CTcSymbol::gen_code_propid(int check_only, int is_expr)
{
    /* by default, a symbol cannot be used as a property ID */
    if (!check_only)
        G_tok->log_error(TCERR_SYM_NOT_PROP, (int)get_sym_len(), get_sym());

    /* we can't return a valid property ID */
    return VM_INVALID_PROP;
}

/*
 *   evaluate a member expression 
 */
void CTcSymbol::gen_code_member(int discard,
                                CTcPrsNode *prop_expr, int prop_is_expr,
                                int argc, int varargs)
{
    /* by default, a symbol cannot be used as an object expression */
    G_tok->log_error(TCERR_SYM_NOT_OBJ, (int)get_sym_len(), get_sym());
}

/*
 *   generate code for an object expression before a '.' 
 */
vm_obj_id_t CTcSymbol::gen_code_obj_predot(int *is_self)
{
    /* by default, a symbol cannot be used as an object expression */
    G_tok->log_error(TCERR_SYM_NOT_OBJ, (int)get_sym_len(), get_sym());

    /* indicate that we don't have a constant object */
    *is_self = FALSE;
    return VM_INVALID_OBJ;
}



/* ------------------------------------------------------------------------ */
/*
 *   T3-specific function symbol class 
 */

/*
 *   evaluate the symbol 
 */
void CTcSymFunc::gen_code(int discard)
{
    /* 
     *   function address are always unknown during code generation;
     *   generate a placeholder instruction and add a fixup record for it 
     */
    G_cg->write_op(OPC_PUSHFNPTR);

    /* add a fixup for the current code location */
    add_abs_fixup(G_cs);

    /* write a placeholder offset - arbitrarily use zero */
    G_cs->write4(0);

    /* note the push */
    G_cg->note_push();
}

/*
 *   take the address of the function 
 */
void CTcSymFunc::gen_code_addr()
{
    /* 
     *   the address of a function cannot be taken - using the name alone
     *   yields the address 
     */
    G_tok->log_error(TCERR_INVAL_FUNC_ADDR, (int)get_sym_len(), get_sym());
}


/*
 *   call the symbol 
 */
void CTcSymFunc::gen_code_call(int discard, int argc, int varargs)
{
    /* write the varargs modifier if appropriate */
    if (varargs)
        G_cg->write_op(OPC_VARARGC);

    /* generate the call instruction and argument count */
    G_cg->write_op(OPC_CALL);
    G_cs->write((char)argc);

    /* check the mode */
    if (G_cg->is_eval_for_debug())
    {
        /* 
         *   debugger expression compilation - we know the absolute
         *   address already, since all symbols are pre-resolved in the
         *   debugger 
         */
        G_cs->write4(get_code_pool_addr());
    }
    else
    {
        /* 
         *   Normal compilation - we won't know the function's address
         *   until after generation is completed, so add a fixup for the
         *   current location, then write a placeholder for the offset
         *   field.  
         */
        add_abs_fixup(G_cs);
        G_cs->write4(0);
    }

    /* call removes arguments */
    G_cg->note_pop(argc);

    /* make sure the argument count is correct */
    if (varargs_ ? argc < argc_ : argc != argc_)
        G_tok->log_error(TCERR_WRONG_ARGC_FOR_FUNC,
                         (int)get_sym_len(), get_sym(), argc_, argc);

    /* if we're not discarding, push the return value from R0 */
    if (!discard)
    {
        G_cg->write_op(OPC_GETR0);
        G_cg->note_push();
    }
}

/*
 *   Get my code pool address.  Valid only after linking. 
 */
ulong CTcSymFunc::get_code_pool_addr() const
{
    /* check for an absolute address */
    if (abs_addr_valid_)
    {
        /* 
         *   we have an absolute address - this means the symbol was
         *   loaded from a fully-linked image file (specifically, from the
         *   debug records) 
         */
        return abs_addr_;
    }
    else
    {
        /* 
         *   we don't have an absolute address, so our address must have
         *   been determined through a linking step - get the final
         *   address from the anchor 
         */
        return anchor_->get_addr();
    }
}

/*
 *   add a runtime symbol table entry 
 */
void CTcSymFunc::add_runtime_symbol(CVmRuntimeSymbols *symtab)
{
    vm_val_t val;
    
    /* add an entry for our absolute address */
    val.set_fnptr(get_code_pool_addr());
    symtab->add_sym(get_sym(), get_sym_len(), &val);
}


/* ------------------------------------------------------------------------ */
/*
 *   T3-specific object symbol class 
 */

/*
 *   evaluate the symbol 
 */
void CTcSymObj::gen_code(int discard)
{
    /* write code to push the object ID */
    if (!discard)
    {
        /* push the object */
        G_cg->write_op(OPC_PUSHOBJ);
        G_cs->write_obj_id(obj_id_);

        /* note the push */
        G_cg->note_push();
    }
}

/*
 *   take the address of the object
 */
void CTcSymObj::gen_code_addr()
{
    /* act as though we were pushing the object ID directly */
    gen_code(FALSE);
}

/*
 *   Generate a 'new' expression 
 */
void CTcSymObj::gen_code_new(int discard, int argc, int varargs,
                             int is_transient)
{
    /* use our static generator */
    s_gen_code_new(discard, obj_id_, argc, varargs, is_transient);
}

/*
 *   Generate a 'new' expression.  (This is a static method so that this
 *   code can be used by all of the possible expression types to which
 *   'new' can be applied.)
 *   
 *   This type of generation applies only to objects of metaclass TADS
 *   Object.  
 */
void CTcSymObj::s_gen_code_new(int discard, vm_obj_id_t obj_id,
                               int argc, int varargs, int is_transient)
{
    /* 
     *   push the base class object - this is always the first argument
     *   (hence last pushed) to the metaclass constructor 
     */
    G_cg->write_op(OPC_PUSHOBJ);
    G_cs->write_obj_id(obj_id);

    /* note the push */
    G_cg->note_push();

    /* 
     *   note that we can only allow 126 arguments to a constructor,
     *   because we must add the implicit superclass argument 
     */
    if (argc > 126)
        G_tok->log_error(TCERR_TOO_MANY_CTOR_ARGS);

    /* 
     *   if we have varargs, swap the top stack elements to get the
     *   argument count back on top, and then generate the varargs
     *   modifier opcode 
     */
    if (varargs)
    {
        /* swap the top stack elements to get argc back to the top */
        G_cg->write_op(OPC_SWAP);

        /* 
         *   increment the argument count to account for the superclass
         *   object argument 
         */
        G_cg->write_op(OPC_INC);

        /* write the varargs modifier opcode */
        G_cg->write_op(OPC_VARARGC);
    }

    /* 
     *   write the NEW instruction - since we always add TADS Object to
     *   our metaclass table before we start compiling any code, we know
     *   it always has a small metaclass number and will always fit in the
     *   short form of the instruction
     *   
     *   Note that the actual argument count we generate is one higher
     *   than the source code argument list, because we add the implicit
     *   first argument to the metaclass constructor 
     */
    G_cg->write_op(is_transient ? OPC_TRNEW1 : OPC_NEW1);
    G_cs->write((char)(argc + 1));
    G_cs->write((char)TCT3_METAID_TADSOBJ);

    /* new1 removes the arguments */
    G_cg->note_pop(argc + 1);

    /* 
     *   if they're not discarding the value, push the new object
     *   reference, which will be in R0 when the constructor returns 
     */
    if (!discard)
    {
        G_cg->write_op(OPC_GETR0);
        G_cg->note_push();
    }
}

/*
 *   Generate code for a member expression 
 */
void CTcSymObj::gen_code_member(int discard,
                                CTcPrsNode *prop_expr, int prop_is_expr,
                                int argc, int varargs)
{
    s_gen_code_member(discard, prop_expr, prop_is_expr,
                      argc, obj_id_, varargs);
}

/*
 *   Static method to generate code for a member expression.  This is
 *   static so that constant object nodes can share it.  
 */
void CTcSymObj::s_gen_code_member(int discard,
                                  CTcPrsNode *prop_expr, int prop_is_expr,
                                  int argc, vm_obj_id_t obj_id, int varargs)
{
    vm_prop_id_t prop;

    /* 
     *   generate the property expression - don't generate the code right
     *   now even if code generation is necessary, because this isn't the
     *   right place for it; for now, simply check to determine if we're
     *   going to need to generate any code for the property expression 
     */
    prop = prop_expr->gen_code_propid(TRUE, prop_is_expr);

    /* don't allow method calls with arguments in speculative mode */
    if (argc != 0 && G_cg->is_speculative())
        err_throw(VMERR_BAD_SPEC_EVAL);
    
    /* check for a constant property value */
    if (prop != VM_INVALID_PROP)
    {
        /* generate an OBJGETPROP or OBJCALLPROP as appropriate */
        if (G_cg->is_speculative())
        {
            /* speculative evaluation - use GETPROPDATA */
            G_cg->write_op(OPC_PUSHOBJ);
            G_cs->write_obj_id(obj_id);
            G_cg->write_op(OPC_GETPROPDATA);
            G_cs->write_prop_id(prop);

            /* we pushed the object, then popped it */
            G_cg->note_push();
            G_cg->note_pop();
        }
        else if (argc == 0)
        {
            /* no arguments - use OBJGETPROP */
            G_cg->write_op(OPC_OBJGETPROP);
            G_cs->write_obj_id(obj_id);
            G_cs->write_prop_id(prop);
        }
        else
        {
            /* generate a varargs modifier if needed */
            if (varargs)
                G_cg->write_op(OPC_VARARGC);
            
            /* arguments - use OBJCALLPROP */
            G_cg->write_op(OPC_OBJCALLPROP);
            G_cs->write((char)argc);
            G_cs->write_obj_id(obj_id);
            G_cs->write_prop_id(prop);

            /* objcallprop removes arguments */
            G_cg->note_pop(argc);
        }
    }
    else
    {
        /* 
         *   non-constant property value - we must first push the object
         *   value, then push the property value, then write a PTRCALLPROP
         *   instruction 
         */

        /* generate the object push */
        G_cg->write_op(OPC_PUSHOBJ);
        G_cs->write_obj_id(obj_id);

        /* note the pushes */
        G_cg->note_push();

        /* keep the argument counter on top if necessary */
        if (varargs)
            G_cg->write_op(OPC_SWAP);

        /* generate the property push */
        prop_expr->gen_code_propid(FALSE, prop_is_expr);

        /* generate the PTRCALLPROP or PTRGETPROPDATA */
        if (G_cg->is_speculative())
        {
            /* speculative - use the data-only property evaluation */
            G_cg->write_op(OPC_PTRGETPROPDATA);
        }
        else
        {
            /* 
             *   if we have a varargs list, modify the call instruction
             *   that follows to make it a varargs call 
             */
            if (varargs)
            {
                /* swap to get the arg counter back on top */
                G_cg->write_op(OPC_SWAP);
                
                /* write the varargs modifier */
                G_cg->write_op(OPC_VARARGC);
            }
            
            /* normal - call the property */
            G_cg->write_op(OPC_PTRCALLPROP);
            G_cs->write((int)argc);
        }

        /* ptrcallprop removes the arguments, the object, and the property */
        G_cg->note_pop(argc + 2);
    }

    /* if they want the result, push it onto the stack */
    if (!discard)
    {
        G_cg->write_op(OPC_GETR0);
        G_cg->note_push();
    }
}

/*
 *   generate code for an object before a '.'  
 */
vm_obj_id_t CTcSymObj::gen_code_obj_predot(int *is_self)
{
    /* return our constant object reference */
    *is_self = FALSE;
    return obj_id_;
}

/*
 *   add a runtime symbol table entry 
 */
void CTcSymObj::add_runtime_symbol(CVmRuntimeSymbols *symtab)
{
    vm_val_t val;

    /* add our entry */
    val.set_obj(obj_id_);
    symtab->add_sym(get_sym(), get_sym_len(), &val);
}

/* ------------------------------------------------------------------------ */
/*
 *   T3-specific property symbol class 
 */

/*
 *   evaluate the symbol 
 */
void CTcSymProp::gen_code(int discard)
{
    /* 
     *   Evaluating a property is equivalent to calling the property on
     *   the "self" object with no arguments.  If there's no "self"
     *   object, an unqualified property evaluation is not possible, so
     *   log an error if this is the case.  
     */
    if (!G_cs->is_self_available())
    {
        G_tok->log_error(TCERR_PROP_NEEDS_OBJ, (int)get_sym_len(), get_sym());
        return;
    }

    if (G_cg->is_speculative())
    {
        /* push 'self', then evaluate the property in data-only mode */
        G_cg->write_op(OPC_PUSHSELF);
        G_cg->write_op(OPC_GETPROPDATA);
        G_cs->write_prop_id(prop_);

        /* we pushed the 'self' value then popped it again */
        G_cg->note_push();
        G_cg->note_pop();
    }
    else
    {
        /* generate the call to 'self' */
        G_cg->write_op(OPC_GETPROPSELF);
        G_cs->write_prop_id(prop_);
    }

    /* if they're not discarding the value, push the result */
    if (!discard)
    {
        G_cg->write_op(OPC_GETR0);
        G_cg->note_push();
    }
}

/*
 *   evaluate a member expression 
 */
void CTcSymProp::gen_code_member(int discard,
                                 CTcPrsNode *prop_expr, int prop_is_expr,
                                 int argc, int varargs)
{
    /* generate code to evaluate the property */
    gen_code(FALSE);

    /* if we have an argument counter, put it back on top */
    if (varargs)
        G_cg->write_op(OPC_SWAP);

    /* use the standard member generation */
    CTcPrsNode::s_gen_member_rhs(discard, prop_expr, prop_is_expr,
                                 argc, varargs);
}

/*
 *   take the address of the property
 */
void CTcSymProp::gen_code_addr()
{
    /* write code to push the property ID */
    G_cg->write_op(OPC_PUSHPROPID);
    G_cs->write_prop_id(prop_);

    /* note the push */
    G_cg->note_push();
}

/*
 *   assign to a property, implicitly of the "self" object 
 */
int CTcSymProp::gen_code_asi(int discard, tc_asitype_t typ,
                             class CTcPrsNode *rhs, int /*ignore_errors*/)
{
    /* if there's no "self" object, we can't make this assignment */
    if (!G_cs->is_self_available())
    {
        /* log an error */
        G_tok->log_error(TCERR_SETPROP_NEEDS_OBJ,
                         (int)get_sym_len(), get_sym());

        /* 
         *   indicate that we're finished, since there's nothing more we
         *   can do here 
         */
        return TRUE;
    }

    /* 
     *   if it's not a simple assignment, tell the caller to do the
     *   composite work and get back to us with the value to store 
     */
    if (typ != TC_ASI_SIMPLE)
        return FALSE;

    /* 
     *   generate the right-hand side's expression for assignment, unless
     *   the caller has already done so 
     */
    if (rhs != 0)
        rhs->gen_code(FALSE, FALSE);

    /* 
     *   if we're not discarding the value, make a copy - we'll consume a
     *   copy in the SETPROP instruction, so we need one more copy to
     *   return to the enclosing expression 
     */
    if (!discard)
    {
        G_cg->write_op(OPC_DUP);
        G_cg->note_push();
    }

    /* 
     *   write the SETPROP instruction - use the special form to assign to
     *   "self" 
     */
    G_cg->write_op(OPC_SETPROPSELF);
    G_cs->write_prop_id(prop_);

    /* setpropself removes the value */
    G_cg->note_pop();

    /* handled */
    return TRUE;
}

/*
 *   call the symbol 
 */
void CTcSymProp::gen_code_call(int discard, int argc, int varargs)
{
    /* 
     *   if there's no "self", we can't invoke a property without an
     *   explicit object reference 
     */
    if (!G_cs->is_self_available())
    {
        G_tok->log_error(TCERR_PROP_NEEDS_OBJ, (int)get_sym_len(), get_sym());
        return;
    }

    /* don't allow calling with arguments in speculative mode */
    if (argc != 0 && G_cg->is_speculative())
        err_throw(VMERR_BAD_SPEC_EVAL);

    /* generate code to invoke the property of "self" */
    if (G_cg->is_speculative())
    {
        /* push 'self', then get the property in data-only mode */
        G_cg->write_op(OPC_PUSHSELF);
        G_cg->write_op(OPC_GETPROPDATA);
        G_cs->write_prop_id(get_prop());

        /* we pushed 'self' then popped it again */
        G_cg->note_push();
        G_cg->note_pop();
    }
    else if (argc == 0)
    {
        /* use the instruction with no arguments */
        G_cg->write_op(OPC_GETPROPSELF);
        G_cs->write_prop_id(get_prop());
    }
    else
    {
        /* write the varargs modifier if appropriate */
        if (varargs)
            G_cg->write_op(OPC_VARARGC);
        
        /* use the instruction with arguments */
        G_cg->write_op(OPC_CALLPROPSELF);
        G_cs->write((char)argc);
        G_cs->write_prop_id(get_prop());

        /* callpropself removes arguments */
        G_cg->note_pop(argc);
    }

    /* if we're not discarding, push the return value from R0 */
    if (!discard)
    {
        G_cg->write_op(OPC_GETR0);
        G_cg->note_push();
    }
}

/*
 *   generate a property ID expression 
 */
vm_prop_id_t CTcSymProp::gen_code_propid(int check_only, int is_expr)
{
    /*
     *   If I'm to be treated as an expression (which indicates that the
     *   property symbol is explicitly enclosed in parentheses in the
     *   original source code expression), then I must evaluate this
     *   property of self.  Otherwise, I yield literally the property ID. 
     */
    if (is_expr)
    {
        /* generate code unless we're only checking */
        if (!check_only)
        {
            /* evaluate this property of self */
            G_cg->write_op(OPC_GETPROPSELF);
            G_cs->write_prop_id(get_prop());

            /* leave the result on the stack */
            G_cg->write_op(OPC_GETR0);
            G_cg->note_push();
        }

        /* tell the caller to use the stack value */
        return VM_INVALID_PROP;
    }
    else
    {
        /* simple '.prop' - return my property ID as a constant value */
        return get_prop();
    }
}

/*
 *   add a runtime symbol table entry 
 */
void CTcSymProp::add_runtime_symbol(CVmRuntimeSymbols *symtab)
{
    vm_val_t val;

    /* add our entry */
    val.set_propid(get_prop());
    symtab->add_sym(get_sym(), get_sym_len(), &val);
}

/* ------------------------------------------------------------------------ */
/*
 *   Enumerator symbol 
 */

/*
 *   evaluate the symbol 
 */
void CTcSymEnum::gen_code(int discard)
{
    if (!discard)
    {
        /* generate code to push the enum value */
        G_cg->write_op(OPC_PUSHENUM);
        G_cs->write_enum_id(get_enum_id());

        /* note the push */
        G_cg->note_push();
    }
}

/*
 *   add a runtime symbol table entry 
 */
void CTcSymEnum::add_runtime_symbol(CVmRuntimeSymbols *symtab)
{
    vm_val_t val;

    /* add our entry */
    val.set_enum(get_enum_id());
    symtab->add_sym(get_sym(), get_sym_len(), &val);
}


/* ------------------------------------------------------------------------ */
/*
 *   T3-specific local variable/parameter symbol class 
 */

/*
 *   generate code to evaluate the symbol 
 */
void CTcSymLocal::gen_code(int discard)
{
    /* generate code to push the local, if we're not discarding it */
    if (!discard)
    {
        /* 
         *   generate as a context local if required, otherwise as an
         *   ordinary local variable 
         */
        if (is_ctx_local_)
        {
            /* generate the context array lookup */
            if (ctx_var_num_ <= 255 && get_ctx_arr_idx() <= 255)
            {
                /* we can do this whole operation with one instruction */
                G_cg->write_op(OPC_IDXLCL1INT8);
                G_cs->write(ctx_var_num_);
                G_cs->write(get_ctx_arr_idx());

                /* this pushes one value */
                G_cg->note_push();
            }
            else
            {
                /* get our context array */
                s_gen_code_getlcl(ctx_var_num_, FALSE);
                
                /* get our value from the context array */
                CTPNConst::s_gen_code_int(get_ctx_arr_idx());
                G_cg->write_op(OPC_INDEX);
                
                /* the INDEX operation removes two values and pushes one */
                G_cg->note_pop();
            }
        }
        else
        {
            /* generate as an ordinary local */
            s_gen_code_getlcl(get_var_num(), is_param());
        }
    }

    /* 
     *   Mark the value as referenced, whether or not we're generating the
     *   code - the value has been logically referenced in the program
     *   even if the result of evaluating it isn't needed.  
     */
    set_val_used(TRUE);
}

/*
 *   generate code to push a local onto the stack 
 */
void CTcSymLocal::s_gen_code_getlcl(int var_num, int is_param)
{
    /* use the shortest form of the instruction that we can */
    if (var_num <= 255)
    {
        /* 8-bit local number - use the one-byte form */
        G_cg->write_op(is_param ? OPC_GETARG1 : OPC_GETLCL1);
        G_cs->write((char)var_num);
    }
    else
    {
        /* local number won't fit in 8 bits - use the two-byte form */
        G_cg->write_op(is_param ? OPC_GETARG2 : OPC_GETLCL2);
        G_cs->write2(var_num);
    }

    /* note the push */
    G_cg->note_push();
}

/*
 *   assign a value 
 */
int CTcSymLocal::gen_code_asi(int discard, tc_asitype_t typ,
                              class CTcPrsNode *rhs, int ignore_errors)
{
    int adding;
    
    /* mark the variable as having had a value assigned to it */
    set_val_assigned(TRUE);

    /* 
     *   if the assignment is anything but simple, this references the
     *   value as well 
     */
    if (typ != TC_ASI_SIMPLE)
        set_val_used(TRUE);

    /* 
     *   If this is a context variable, use standard assignment (i.e.,
     *   generate the result first, then generate a simple assignment to the
     *   variable).  Otherwise, we might be able to generate a fancy
     *   combined calculate-and-assign sequence, depending on the type of
     *   assignment calculation we're performing.
     */
    if (is_ctx_local_ && typ != TC_ASI_SIMPLE)
    {
        /* 
         *   it's a context local and it's not a simple assignment, so we
         *   can't perform any special calculate-and-assign sequence - tell
         *   the caller to calculate the full result first and then try
         *   again using simple assignment 
         */
        return FALSE;
    }

    /* 
     *   check the type of assignment - we can optimize the code
     *   generation to use more compact instruction sequences for certain
     *   types of assignments 
     */
    switch(typ)
    {
    case TC_ASI_SIMPLE:
        /* 
         *   Simple assignment to local/parameter.  Check for some special
         *   cases: when assigning a constant value of 0, 1, or nil to a
         *   local, we can generate a short instruction 
         */
        if (!is_param() && !is_ctx_local_ && rhs != 0 && rhs->is_const())
        {
            CTcConstVal *cval;

            /* get the constant value */
            cval = rhs->get_const_val();
            
            /* check for nil and 0 or 1 values */
            if (cval->get_type() == TC_CVT_NIL)
            {
                /* it's nil - generate NILLCL1 or NILLCL2 */
                if (get_var_num() <= 255)
                {
                    G_cg->write_op(OPC_NILLCL1);
                    G_cs->write((char)get_var_num());
                }
                else
                {
                    G_cg->write_op(OPC_NILLCL2);
                    G_cs->write2(get_var_num());
                }

                /* if not discarding, leave nil on the stack */
                if (!discard)
                {
                    G_cg->write_op(OPC_PUSHNIL);
                    G_cg->note_push();
                }

                /* handled */
                return TRUE;
            }
            else if (cval->get_type() == TC_CVT_INT
                     && (cval->get_val_int() == 0 
                         || cval->get_val_int() == 1))
            {
                int ival;

                /* get the integer value */
                ival = cval->get_val_int();
                
                /* 0 or 1 - generate ZEROLCLn or ONELCLn */
                if (get_var_num() <= 255)
                {
                    G_cg->write_op(ival == 0 ? OPC_ZEROLCL1 : OPC_ONELCL1);
                    G_cs->write((char)get_var_num());
                }
                else
                {
                    G_cg->write_op(ival == 0 ? OPC_ZEROLCL2 : OPC_ONELCL2);
                    G_cs->write2(get_var_num());
                }

                /* if not discarding, leave the value on the stack */
                if (!discard)
                {
                    G_cg->write_op(ival == 0 ? OPC_PUSH_0 : OPC_PUSH_1);
                    G_cg->note_push();
                }

                /* handled */
                return TRUE;
            }
        }

        /* 
         *   If we got here, we can't generate a specialized constant
         *   assignment - so, first, generate the right-hand side's value
         *   normally.  (If no 'rhs' is specified, the value is already on
         *   the stack.)  
         */
        if (rhs != 0)
            rhs->gen_code(FALSE, FALSE);

        /* leave an extra copy of the value on the stack if not discarding */
        if (!discard)
        {
            G_cg->write_op(OPC_DUP);
            G_cg->note_push();
        }

        /* now assign the value at top of stack to the variable */
        gen_code_setlcl();

        /* handled */
        return TRUE;

    case TC_ASI_ADD:
        adding = TRUE;
        goto add_or_sub;
        
    case TC_ASI_SUB:
        adding = FALSE;

    add_or_sub:
        /* if this is a parameter, there's nothing special we can do */
        if (is_param())
            return FALSE;
        
        /* 
         *   Add/subtract to a local/parameter.  If the right-hand side is a
         *   constant integer value, we might be able to generate a special
         *   instruction to add/subtract it.  
         */
        if (rhs != 0
            && adding
            && rhs->is_const()
            && rhs->get_const_val()->get_type() == TC_CVT_INT)
        {
            long ival;

            /* get the integer value to assign */
            ival = rhs->get_const_val()->get_val_int();

            /* 
             *   if the right-hand side's integer value fits in one byte,
             *   generate the short (8-bit) instruction; otherwise,
             *   generate the long (32-bit) format 
             */
            if (ival == 1)
            {
                /* adding one - increment the local */
                G_cg->write_op(OPC_INCLCL);
                G_cs->write2(get_var_num());
            }
            else if (ival == -1)
            {
                /* subtracting one - decrement the local */
                G_cg->write_op(OPC_DECLCL);
                G_cs->write2(get_var_num());
            }
            else if (ival <= 127 && ival >= -128
                     && get_var_num() <= 255)
            {
                /* fits in 8 bits - use the 8-bit format */
                G_cg->write_op(OPC_ADDILCL1);
                G_cs->write((char)get_var_num());
                G_cs->write((char)ival);
            }
            else
            {
                /* 
                 *   either the value or the variable number doesn't fit
                 *   in 8 bits - use the 32-bit format 
                 */
                G_cg->write_op(OPC_ADDILCL4);
                G_cs->write2(get_var_num());
                G_cs->write4(ival);
            }
        }
        else
        {
            /* 
             *   We don't have a special instruction for the right side,
             *   so generate it normally and add/subtract the value.  (If
             *   there's no 'rhs' value specified, it means that the value
             *   is already on the stack, so there's nothing extra for us
             *   to generate.)  
             */
            if (rhs != 0)
                rhs->gen_code(FALSE, FALSE);
            
            /* write the ADDTOLCL instruction */
            G_cg->write_op(adding ? OPC_ADDTOLCL : OPC_SUBFROMLCL);
            G_cs->write2(get_var_num());

            /* addtolcl/subfromlcl remove the rvalue */
            G_cg->note_pop();
        }

        /* 
         *   if not discarding, push the result onto the stack; do this by
         *   simply evaluating the local, which is the simplest and most
         *   efficient way to obtain the result of the computation 
         */
        if (!discard)
            gen_code(FALSE);

        /* handled */
        return TRUE;

    case TC_ASI_PREINC:
        /* if this is a parameter, there's nothing special we can do */
        if (is_param())
            return FALSE;

        /* generate code to increment the local */
        G_cg->write_op(OPC_INCLCL);
        G_cs->write2(get_var_num());

        /* if we're not discarding, push the local's new value */
        if (!discard)
            gen_code(FALSE);

        /* handled */
        return TRUE;

    case TC_ASI_POSTINC:
        /* if this is a parameter, there's nothing special we can do */
        if (is_param())
            return FALSE;

        /* 
         *   if we're not discarding, push the local's value prior to
         *   incrementing it - this will be the result we'll leave on the
         *   stack 
         */
        if (!discard)
            gen_code(FALSE);

        /* generate code to increment the local */
        G_cg->write_op(OPC_INCLCL);
        G_cs->write2(get_var_num());

        /* handled */
        return TRUE;

    case TC_ASI_PREDEC:
        /* if this is a parameter, there's nothing special we can do */
        if (is_param())
            return FALSE;

        /* generate code to decrement the local */
        G_cg->write_op(OPC_DECLCL);
        G_cs->write2(get_var_num());

        /* if we're not discarding, push the local's new value */
        if (!discard)
            gen_code(FALSE);

        /* handled */
        return TRUE;

    case TC_ASI_POSTDEC:
        /* if this is a parameter, there's nothing special we can do */
        if (is_param())
            return FALSE;

        /* 
         *   if we're not discarding, push the local's value prior to
         *   decrementing it - this will be the result we'll leave on the
         *   stack 
         */
        if (!discard)
            gen_code(FALSE);

        /* generate code to decrement the local */
        G_cg->write_op(OPC_DECLCL);
        G_cs->write2(get_var_num());

        /* handled */
        return TRUE;

    default:
        /* we can't do anything special with other assignment types */
        return FALSE;
    }
}

/*
 *   generate code to assigin the value at top of stack to the local
 *   variable 
 */
void CTcSymLocal::gen_code_setlcl()
{
    /* check to see if we're a context local (as opposed to a stack local) */
    if (is_ctx_local_)
    {
        /* generate the assignment using the appropriate sequence */
        if (ctx_var_num_ <= 255 && get_ctx_arr_idx() <= 255)
        {
            /* we can fit this in a single instruction */
            G_cg->write_op(OPC_SETINDLCL1I8);
            G_cs->write(ctx_var_num_);
            G_cs->write(get_ctx_arr_idx());

            /* this pops the value being assigned */
            G_cg->note_pop();
        }
        else
        {
            /* get our context array */
            s_gen_code_getlcl(ctx_var_num_, FALSE);
            
            /* set our value in the context array */
            CTPNConst::s_gen_code_int(get_ctx_arr_idx());
            G_cg->write_op(OPC_SETIND);
            G_cg->write_op(OPC_DISC);
            
            /* 
             *   the SETIND pops three values and pushes one (for a net two
             *   pops), and the DISC pops one more value, so our total is
             *   three pops 
             */
            G_cg->note_pop(3);
        }
    }
    else
    {
        /* we're just a plain stack variable */
        gen_code_setlcl_stk();
    }
}

/*
 *   Generate code to store the value at the top of the stack into the given
 *   local stack slot.  Note that this routine will not work with a context
 *   local - it only works if the variable is known to be a stack variable.  
 */
void CTcSymLocal::s_gen_code_setlcl_stk(int var_num, int is_param)
{
    /* use the shortest form that will fit our variable index */
    if (var_num <= 255)
    {
        /* use the one-byte instruction */
        G_cg->write_op(is_param ? OPC_SETARG1 : OPC_SETLCL1);
        G_cs->write((char)var_num);
    }
    else
    {
        /* big number - use the two-byte instruction */
        G_cg->write_op(is_param ? OPC_SETARG2 : OPC_SETLCL2);
        G_cs->write2(var_num);
    }

    /* the setarg/setlcl ops remove the rvalue */
    G_cg->note_pop();
}

/*
 *   call the symbol 
 */
void CTcSymLocal::gen_code_call(int discard, int argc, int varargs)
{
    /* 
     *   to call a local, we'll simply evaluate the local normally, then
     *   call through the resulting (presumed) property or function
     *   pointer value 
     */
    gen_code(FALSE);

    /* 
     *   if we have a varargs list, modify the call instruction that
     *   follows to make it a varargs call 
     */
    if (varargs)
    {
        /* swap the top of the stack to get the arg counter back on top */
        G_cg->write_op(OPC_SWAP);

        /* write the varargs modifier */
        G_cg->write_op(OPC_VARARGC);
    }

    /* don't allow this at all in speculative mode */
    if (G_cg->is_speculative())
        err_throw(VMERR_BAD_SPEC_EVAL);

    /* call the result as a function or method pointer */
    G_cg->write_op(OPC_PTRCALL);
    G_cs->write((char)argc);

    /* ptrcall removes the arguments and the function pointer */
    G_cg->note_pop(argc + 1);

    /* if we're not discarding the value, push the result */
    if (!discard)
    {
        G_cg->write_op(OPC_GETR0);
        G_cg->note_push();
    }
}

/*
 *   generate a property ID expression 
 */
vm_prop_id_t CTcSymLocal::gen_code_propid(int check_only, int /*is_expr*/)
{
    /*
     *   treat the local as a property-valued expression; generate the
     *   code for the local, then tell the caller that no constant value
     *   is available, since the local's property ID value should be on
     *   the stack 
     */
    if (!check_only)
        gen_code(FALSE);

    /* tell the caller to use the stack value */
    return VM_INVALID_PROP;
}

/*
 *   evaluate a member expression 
 */
void CTcSymLocal::gen_code_member(int discard,
                                  CTcPrsNode *prop_expr, int prop_is_expr,
                                  int argc, int varargs)
{
    /* generate code to evaluate the local */
    gen_code(FALSE);

    /* if we have an argument counter, put it back on top */
    if (varargs)
        G_cg->write_op(OPC_SWAP);

    /* use the standard member generation */
    CTcPrsNode::s_gen_member_rhs(discard, prop_expr, prop_is_expr,
                                 argc, varargs);
}

/*
 *   write to a debug record 
 */
int CTcSymLocal::write_to_debug_frame()
{
    int flags;
    
    /* 
     *   write my ID - if we're a context variable, we want to write the
     *   context variable ID; otherwise write our stack location as normal 
     */
    if (is_ctx_local_)
        G_cs->write2(ctx_var_num_);
    else
        G_cs->write2(var_num_);

    /* compute my flags */
    flags = 0;
    if (is_param_)
        flags |= 1;
    if (is_ctx_local_)
        flags |= 2;

    /* write my flags */
    G_cs->write2(flags);

    /* write my local context array index */
    G_cs->write2(get_ctx_arr_idx());

    /* write the length of my symbol name */
    G_cs->write2(len_);
    G_cs->write(str_, len_);

    /* we did write this symbol */
    return TRUE;
}

/* ------------------------------------------------------------------------ */
/*
 *   Built-in function symbol
 */

/*
 *   Evaluate the symbol.  Invoking a built-in function without an
 *   argument list is simply a call to the built-in function with no
 *   arguments.  
 */
void CTcSymBif::gen_code(int discard)
{
    /* generate a call */
    gen_code_call(discard, 0, FALSE);
}

/*
 *   Generate code to call the built-in function 
 */
void CTcSymBif::gen_code_call(int discard, int argc, int varargs)
{
    /* don't allow calling built-in functions in speculative mode */
    if (G_cg->is_speculative())
        err_throw(VMERR_BAD_SPEC_EVAL);
    
    /* check for minimum and maximum arguments */
    if (argc < min_argc_)
    {
        G_tok->log_error(TCERR_TOO_FEW_FUNC_ARGS,
                         (int)get_sym_len(), get_sym());
    }
    else if (!varargs_ && argc > max_argc_)
    {
        G_tok->log_error(TCERR_TOO_MANY_FUNC_ARGS,
                         (int)get_sym_len(), get_sym());
    }

    /* write the varargs modifier if appropriate */
    if (varargs)
        G_cg->write_op(OPC_VARARGC);

    /* generate the call */
    if (get_func_set_id() < 4 && get_func_idx() < 256)
    {
        uchar short_ops[] =
            { OPC_BUILTIN_A, OPC_BUILTIN_B, OPC_BUILTIN_C, OPC_BUILTIN_D };
        
        /* 
         *   it's one of the first 256 functions in one of the first four
         *   function sets - we can generate a short instruction 
         */
        G_cg->write_op(short_ops[get_func_set_id()]);
        G_cs->write((char)argc);
        G_cs->write((char)get_func_idx());
    }
    else
    {
        /* it's not in the default set - use the longer instruction */
        if (get_func_idx() < 256)
        {
            /* low function index - write the short form */
            G_cg->write_op(OPC_BUILTIN1);
            G_cs->write((char)argc);
            G_cs->write((char)get_func_idx());
        }
        else
        {
            /* big function index - write the long form */
            G_cg->write_op(OPC_BUILTIN2);
            G_cs->write((char)argc);
            G_cs->write2(get_func_idx());
        }

        /* write the function set ID */
        G_cs->write((char)get_func_set_id());
    }

    /* the built-in functions always remove arguments */
    G_cg->note_pop(argc);

    /* 
     *   if they're not discarding the value, push it - the value is
     *   sitting in R0 after the call returns
     */
    if (!discard)
    {
        G_cg->write_op(OPC_GETR0);
        G_cg->note_push();
    }
}


/* ------------------------------------------------------------------------ */
/*
 *   External function symbol 
 */

/*
 *   evaluate the symbol 
 */
void CTcSymExtfn::gen_code(int /*discard*/)
{
    //$$$ to be implemented
    assert(FALSE);
}

/*
 *   generate a call to the symbol
 */
void CTcSymExtfn::gen_code_call(int /*discard*/, int /*argc*/, int /*varargs*/)
{
    //$$$ to be implemented
    assert(FALSE);
}


/* ------------------------------------------------------------------------ */
/*
 *   Code Label symbol 
 */

/*
 *   evaluate the symbol 
 */
void CTcSymLabel::gen_code(int discard)
{
    /* it's not legal to evaluate a code label; log an error */
    G_tok->log_error(TCERR_CANNOT_EVAL_LABEL,
                     (int)get_sym_len(), get_sym());
}

/* ------------------------------------------------------------------------ */
/*
 *   Metaclass symbol 
 */

/*
 *   generate code for evaluating the symbol 
 */
void CTcSymMetaclass::gen_code(int discard)
{
    /* 
     *   the metaclass name refers to the IntrinsicClass instance
     *   associated with the metaclass 
     */
    G_cg->write_op(OPC_PUSHOBJ);
    G_cs->write_obj_id(class_obj_);

    /* note the push */
    G_cg->note_push();
}

/*
 *   generate code for operator 'new' applied to the metaclass 
 */
void CTcSymMetaclass::gen_code_new(int discard, int argc, int varargs,
                                   int is_transient)
{
    /* if we have varargs, write the modifier */
    if (varargs)
        G_cg->write_op(OPC_VARARGC);
    
    if (meta_idx_ <= 255 && argc <= 255)
    {
        G_cg->write_op(is_transient ? OPC_TRNEW1 : OPC_NEW1);
        G_cs->write((char)argc);
        G_cs->write((char)meta_idx_);
    }
    else
    {
        G_cg->write_op(is_transient ? OPC_TRNEW1 : OPC_NEW2);
        G_cs->write2(argc);
        G_cs->write2(meta_idx_);
    }

    /* new1/new2 remove arguments */
    G_cg->note_pop(argc);

    /* if we're not discarding the value, push it */
    if (!discard)
    {
        G_cg->write_op(OPC_GETR0);
        G_cg->note_push();
    }
}

/* 
 *   generate a member expression 
 */
void CTcSymMetaclass::gen_code_member(int discard, CTcPrsNode *prop_expr,
                                      int prop_is_expr,
                                      int argc, int varargs)
{
    /* generate code to push our class object onto the stack */
    gen_code(FALSE);

    /* if we have an argument counter, put it back on top */
    if (varargs)
        G_cg->write_op(OPC_SWAP);

    /* use the standard member generation */
    CTcPrsNode::s_gen_member_rhs(discard, prop_expr, prop_is_expr,
                                 argc, varargs);
}

/*
 *   add a runtime symbol table entry 
 */
void CTcSymMetaclass::add_runtime_symbol(CVmRuntimeSymbols *symtab)
{
    vm_val_t val;

    /* add our entry */
    val.set_obj(get_class_obj());
    symtab->add_sym(get_sym(), get_sym_len(), &val);
}


/* ------------------------------------------------------------------------ */
/*
 *   Exception Table 
 */

/*
 *   create 
 */
CTcT3ExcTable::CTcT3ExcTable()
{
    /* allocate an initial table */
    exc_alloced_ = 1024;
    table_ = (CTcT3ExcEntry *)t3malloc(exc_alloced_ * sizeof(table_[0]));

    /* no entries are in use yet */
    exc_used_ = 0;

    /* method offset is not yet known */
    method_ofs_ = 0;
}


/*
 *   add an entry to our table 
 */
void CTcT3ExcTable::add_catch(ulong protected_start_ofs,
                              ulong protected_end_ofs,
                              ulong exc_obj_id, ulong catch_block_ofs)
{
    CTcT3ExcEntry *entry;

    /* if necessary, expand our table */
    if (exc_used_ == exc_alloced_)
    {
        /* expand the table a bit */
        exc_alloced_ += 1024;

        /* reallocate the table at the larger size */
        table_ = (CTcT3ExcEntry *)
                 t3realloc(table_, exc_alloced_ * sizeof(table_[0]));
    }

    /* 
     *   set up the new entry - store the offsets relative to the method
     *   header start address 
     */
    entry = table_ + exc_used_;
    entry->start_ofs = protected_start_ofs - method_ofs_;
    entry->end_ofs = protected_end_ofs - method_ofs_;
    entry->exc_obj_id = exc_obj_id;
    entry->catch_ofs = catch_block_ofs - method_ofs_;

    /* consume the new entry */
    ++exc_used_;
}

/*
 *   write our exception table to the code stream 
 */
void CTcT3ExcTable::write_to_code_stream()
{
    CTcT3ExcEntry *entry;
    size_t i;

    /* write the number of entries as a UINT2 */
    G_cs->write2(exc_used_);

    /* write the entries */
    for (i = 0, entry = table_ ; i < exc_used_ ; ++i, ++entry)
    {
        /* write this entry */
        G_cs->write2(entry->start_ofs);
        G_cs->write2(entry->end_ofs);
        G_cs->write_obj_id(entry->exc_obj_id);
        G_cs->write2(entry->catch_ofs);
    }
}


/* ------------------------------------------------------------------------ */
/*
 *   Code body 
 */

/*
 *   generate code 
 */
void CTPNCodeBody::gen_code(int, int)
{
    ulong start_ofs;
    ulong code_start_ofs;
    ulong code_end_ofs;
    CTcStreamAnchor *anchor;
    CTPNCodeBody *old_code_body;
    CTcCodeBodyCtx *cur_ctx;
    int ctx_idx;

    /* if I've been replaced, don't bother generating any code */
    if (replaced_)
        return;

    /* select the appropriate code stream for generation */
    if (is_static_)
    {
        /* it's static - activate the static code stream */
        G_cs = G_cs_static;
    }
    else
    {
        /* normal code - activate the main code stream */
        G_cs = G_cs_main;
    }

    /* if we're in a constructor, set the flag to indicate this */
    G_cg->set_in_constructor(is_constructor_);

    /* 
     *   tell the code stream whether or not 'self' is available within
     *   this code body 
     */
    G_cs->set_self_available(self_valid_);

    /* 
     *   Create an anchor for this function or method in the code stream.
     *   We maintain our own fixup list (in fact, we might even have a
     *   fixup list maintained externally to us, in our symbol table
     *   entry), because a code body might be reachable through multiple
     *   references (a function, for example, has a global symbol table
     *   entry, so fixups might already have been created by the time we
     *   create this anchor).
     *   
     *   Remember my anchor for future use.  
     */
    anchor = G_cs->add_anchor(fixup_owner_sym_, fixup_list_anchor_);

    /* set the anchor in my symbol */
    if (fixup_owner_sym_ != 0)
        fixup_owner_sym_->set_anchor(anchor);

    /* note our start offset */
    start_ofs = G_cs->get_ofs();

    /* we obviously can't combine any past instructions */
    G_cg->clear_peephole();

    /* clear the old line records */
    G_cs->clear_line_recs();

    /* clear the old frame list */
    G_cs->clear_local_frames();

    /* clear the old exception table */
    G_cg->get_exc_table()->clear_table();

    /* tell the code generator our starting offset */
    G_cg->set_method_ofs(start_ofs);

    /* reset the stack depth counters */
    G_cg->reset_sp_depth();

    /* there are no enclosing 'switch' or block statements yet */
    G_cs->set_switch(0);
    G_cs->set_enclosing(0);

    /* set myself as the current code body being generated */
    old_code_body = G_cs->set_code_body(this);

    /* get the 'goto' symbol table for this function */
    G_cs->set_goto_symtab(gototab_);

    /* 
     *   Generate the function header.  At the moment, we don't know the
     *   stack usage, exception table offset, or debug record offset,
     *   since these all come after the byte code; we won't know how big
     *   the byte code is until after we generate it.  For now, write zero
     *   bytes as placeholders for these slots; we'll come back and fix
     *   them up to their real values after we've generated the byte code.
     */
    G_cs->write(argc_ | (varargs_ ? 0x80 : 0));           /* argument count */
    G_cs->write(0);                                   /* reserved zero byte */
    G_cs->write2(0);   /* number of locals - won't know until after codegen */
    G_cs->write2(0);        /* total stack - won't know until after codegen */
    G_cs->write2(0); /* exception table offset - presume no exception table */
    G_cs->write2(0);      /* debug record offset - presume no debug records */

    /* the byte-code instructions start immediately after the method header */
    code_start_ofs = G_cs->get_ofs();

    /* 
     *   Add each local symbol table enclosing the code body's primary
     *   local symbol table to the frame list.  The outermost code body
     *   table can be outside the primary code body table for situations
     *   such as anonymous functions.  Since these tables are outside of
     *   any statements, we must explicitly add them to ensure that they
     *   are assigned debugging frame ID's and are written to the debug
     *   data.
     */
    if (lcltab_ != 0)
    {
        CTcPrsSymtab *tab;

        /* add each frame outside the primary frame to the code gen list */
        for (tab = lcltab_->get_parent() ; tab != 0 ; tab = tab->get_parent())
            G_cs->set_local_frame(tab);
    }

    /* the method's local symbol table is now the active symbol table */
    G_cs->set_local_frame(lcltab_);

    /* if we have a local context, initialize it */
    if (has_local_ctx_)
    {
        /* write code to create the new Vector to store the context locals */
        CTPNConst::s_gen_code_int(local_ctx_arr_size_);
        G_cg->write_op(OPC_DUP);
        G_cg->write_op(OPC_NEW1);
        G_cs->write(2);
        G_cs->write(TCT3_METAID_VECTOR);

        /* retrieve the object value */
        G_cg->write_op(OPC_GETR0);

        /*
         *   we duplicated the vector size argument, then we popped it and
         *   pushed the object; so we have a maximum of one extra push and a
         *   net of zero 
         */
        G_cg->note_push();
        G_cg->note_pop();

        /* store the new object in the context local variable */
        CTcSymLocal::s_gen_code_setlcl_stk(local_ctx_var_, FALSE);

        /* 
         *   go through our symbol table, and copy each parameter that's
         *   also a context local into its context local slot 
         */
        if (lcltab_ != 0)
            lcltab_->enum_entries(&enum_for_param_ctx, this);
    }

    /* 
     *   If we have a varargs-list parameter, generate the code to set up
     *   the list value from the actual parameters.  Note that we must do
     *   this after we set up the local context, in case the varargs list
     *   parameter variable is a context local, in which case it will need
     *   to be stored in the context, in which case we need the context to
     *   be initialized first.  
     */
    if (varargs_list_)
    {
        /* generate the PUSHPARLST instruction to create the list */
        G_cg->write_op(OPC_PUSHPARLST);
        G_cs->write(argc_);

        /* 
         *   we pushed at least one value (the list); we don't know how many
         *   others we might have pushed, but it doesn't matter because the
         *   interpreter is responsible for checking for stack space 
         */
        G_cg->note_push();

        /* store the list in our varargs parameter list local */
        varargs_list_local_->gen_code_setlcl();
    }

    /* 
     *   generate code to initialize each enclosing-context-pointer local
     *   - these variables allow us to find the context objects while
     *   we're running inside this function 
     */
    for (ctx_idx = 0, cur_ctx = ctx_head_ ; cur_ctx != 0 ;
         cur_ctx = cur_ctx->nxt_, ++ctx_idx)
    {
        /* 
         *   Get this context value, stored in the function object
         *   ('self') at index value 2+n (n=0,1,...).  Note that the
         *   context object indices start at 2 because the code pointer
         *   for the function is at index 1.  
         */
        G_cg->write_op(OPC_PUSHSELF);
        CTPNConst::s_gen_code_int(ctx_idx + 2);
        G_cg->write_op(OPC_INDEX);

        /* 
         *   we pushed the object, then popped the object and index and
         *   pushed the indexed value - this is a net of no change with one
         *   maximum push 
         */
        G_cg->note_push();
        G_cg->note_pop();

        /*
         *   If this is context level 1, and this context has a 'self', and
         *   we need either 'self' or the full method context from the
         *   lexically enclosing scope, generate code to load the self or the
         *   full method context (as appropriate) from our local context.
         *   
         *   The enclosing method context is always stored in the context at
         *   level 1, because this is inherently shared context for all
         *   enclosed lexical scopes.  We thus only have to worry about this
         *   for context level 1.  
         */
        if (cur_ctx->level_ == 1
            && self_valid_
            && (self_referenced_ || full_method_ctx_referenced_))
        {
            CTPNCodeBody *outer;
            
            /* 
             *   we just put our context object on the stack in preparation
             *   for storing it - make a duplicate copy of it for our own
             *   purposes 
             */
            G_cg->write_op(OPC_DUP);
            G_cg->note_push();

            /* get the saved method context from the context object */
            CTPNConst::s_gen_code_int(TCPRS_LOCAL_CTX_METHODCTX);
            G_cg->write_op(OPC_INDEX);

            /* 
             *   Load the context.  We must check the outermost context to
             *   determine what it stored, because we must load whatever it
             *   stored.  
             */
            if ((outer = get_outermost_enclosing()) != 0
                && outer->local_ctx_needs_full_method_ctx())
            {
                /* load the full method context */
                G_cg->write_op(OPC_LOADCTX);
            }
            else
            {
                /* load the 'self' object */
                G_cg->write_op(OPC_SETSELF);
            }
            
            /* 
             *   we popped two values and pushed one in the INDEX, then
             *   popped a value in the LOADCTX or SETSELF: the net is removal
             *   of two elements and no additional maximum depth 
             */
            G_cg->note_pop(2);
        }

        /* store the context value in the appropriate local variable */
        CTcSymLocal::s_gen_code_setlcl_stk(cur_ctx->var_num_, FALSE);
    }

    /* 
     *   if we created our own local context, and we have a 'self' object,
     *   and we need access to the 'self' object or the full method context
     *   from anonymous functions that refer to the local context, generate
     *   code to store the appropriate data in the local context 
     */
    if (has_local_ctx_ && self_valid_
        && (local_ctx_needs_self_ || local_ctx_needs_full_method_ctx_))
    {
        /* check to see what we need */
        if (local_ctx_needs_full_method_ctx_)
        {
            /* 
             *   we need the full method context - generate code to store it
             *   and push a reference to it onto the stack 
             */
            G_cg->write_op(OPC_STORECTX);
        }
        else
        {
            /* we only need 'self' - push it */
            G_cg->write_op(OPC_PUSHSELF);
        }

        /* we just pushed one value */
        G_cg->note_push();

        /* assign the value to the context variable */
        if (local_ctx_var_ <= 255 && TCPRS_LOCAL_CTX_METHODCTX <= 255)
        {
            /* we can make the assignment with a single instruction */
            G_cg->write_op(OPC_SETINDLCL1I8);
            G_cs->write(local_ctx_var_);
            G_cs->write(TCPRS_LOCAL_CTX_METHODCTX);

            /* that pops one value */
            G_cg->note_pop();
        }
        else
        {
            /* get the context object */
            CTcSymLocal::s_gen_code_getlcl(local_ctx_var_, FALSE);
            
            /* store the data in the local context object */
            CTPNConst::s_gen_code_int(TCPRS_LOCAL_CTX_METHODCTX);
            G_cg->write_op(OPC_SETIND);

            /* discard the indexed result */
            G_cg->write_op(OPC_DISC);
        
            /* 
             *   the SETIND pops three values and pushes one, then we pop one
             *   more with the DISC - this is a net three pops with no extra
             *   maximum depth 
             */
            G_cg->note_pop(3);
        }
    }

    /* generate the compound statement, if we have one */
    if (stm_ != 0)
        stm_->gen_code(TRUE, TRUE);

#ifdef T3_DEBUG
    if (G_cg->get_sp_depth() != 0)
    {
        printf("---> stack depth is %d after block codegen!\n",
               G_cg->get_sp_depth());
        if (fixup_owner_sym_ != 0)
            printf("---> code block for %.*s\n",
                   (int)fixup_owner_sym_->get_sym_len(),
                   fixup_owner_sym_->get_sym());
    }
#endif

    /* 
     *   Generate a 'return' opcode with no return value - this will ensure
     *   that code that reaches the end of the procedure returns normally.
     *   If this is a constructor, return the 'self' object rather than nil.
     *   
     *   We only need to generate this epilogue if the next instruction would
     *   be reachable.  If it's not reachable, then the code explicitly took
     *   care of all types of exits.  
     */
    if (!G_cg->can_skip_op())
    {
        /* 
         *   add a line record for the implied return at the last source line
         *   of the code body 
         */
        G_cg->add_line_rec(end_desc_, end_linenum_);

        /* write the appropriate return */
        if (is_constructor_)
        {
            /* we're in a constructor - return 'self' */
            G_cg->write_op(OPC_PUSHSELF);
            G_cg->write_op(OPC_RETVAL);
        }
        else
        {
            /* 
             *   Normal method/function - return without a value (explicitly
             *   set R0 to nil, though, so we don't return something returned
             *   from a called function).  
             */
            G_cg->write_op(OPC_RETNIL);
        }
    }

    /* 
     *   release labels allocated for the code block; this will log an error
     *   if any labels are not defined 
     */
    G_cs->release_labels();

    /* 
     *   Eliminate jump-to-jump sequences in the generated code.  Don't
     *   bother if we've found any errors, as the generated code will not
     *   necessarily be valid if this is the case.  
     */
    if (G_tcmain->get_error_count() == 0)
        remove_jumps_to_jumps(code_start_ofs);

    /* note the code block's end point */
    code_end_ofs = G_cs->get_ofs();

    /*
     *   Fix up the local variable count in the function header.  We might
     *   allocate extra locals for internal use while generating code, so
     *   we must wait until after generating our code before we know the
     *   final local count. 
     */
    G_cs->write2_at(start_ofs + 2, local_cnt_);

    /* 
     *   Fix up the total stack space indicator in the function header.
     *   The total stack size must include the locals, as well as stack
     *   space needed for intermediate computations.  
     */
    G_cs->write2_at(start_ofs + 4, G_cg->get_max_sp_depth() + local_cnt_);

    /* 
     *   Generate the exception table, if we have one.  If we have no
     *   exception records, leave the exception table offset set to zero
     *   to indicate that there is no exception table for the method. 
     */
    if (G_cg->get_exc_table()->get_entry_count() != 0)
    {
        /* 
         *   write the exception table offset - it's at the current offset
         *   in the code 
         */
        G_cs->write2_at(start_ofs + 6, G_cs->get_ofs() - start_ofs);

        /* write the table */
        G_cg->get_exc_table()->write_to_code_stream();
    }

    /* remember the head of the nested symbol table list */
    first_nested_symtab_ = G_cs->get_first_frame();

    /* generate debug records if appropriate */
    if (G_debug)
        build_debug_table(start_ofs);

    /*
     *   Tell the code generator our code block byte length so that it can
     *   keep track of the longest single byte-code block; it will use
     *   this to choose the code pool page size when generating the image
     *   file.  
     */
    G_cg->note_bytecode(anchor->get_len(G_cs));

    /* check for unreferenced labels and issue warnings */
    check_unreferenced_labels();

    /* show the disassembly of the code block if desired */
    if (G_disasm_out != 0)
        show_disassembly(start_ofs, code_start_ofs, code_end_ofs);

    /* we're no longer in a constructor, if we ever were */
    G_cg->set_in_constructor(FALSE);

    /* clear the current code body */
    G_cs->set_code_body(old_code_body);

    /* always leave the main code stream active by default */
    G_cs = G_cs_main;
}

/*
 *   disassembly stream source implementation 
 */
class CTcUnasSrcCodeBody: public CTcUnasSrc
{
public:
    CTcUnasSrcCodeBody(CTcCodeStream *str,
                       unsigned long code_start_ofs,
                       unsigned long code_end_ofs)
    {
        /* remember the stream */
        str_ = str;

        /* start at the starting offset */
        cur_ofs_ = code_start_ofs;

        /* remember the ending offset */
        end_ofs_ = code_end_ofs;
    }

    /* read the next byte */
    int next_byte(char *ch)
    {
        /* if there's anything left, return it */
        if (cur_ofs_ < end_ofs_)
        {
            /* return the next byte */
            *ch = str_->get_byte_at(cur_ofs_);
            ++cur_ofs_;
            return 0;
        }
        else
        {
            /* indicate end of file */
            return 1;
        }
    }

    /* get the current offset */
    ulong get_ofs() const { return cur_ofs_; }

protected:
    /* code stream */
    CTcCodeStream *str_;

    /* current offset */
    unsigned long cur_ofs_;

    /* offset of end of code stream */
    unsigned long end_ofs_;
};

/*
 *   Show the disassembly of this code block 
 */
void CTPNCodeBody::show_disassembly(unsigned long start_ofs,
                                    unsigned long code_start_ofs,
                                    unsigned long code_end_ofs)
{
    int argc;
    int locals;
    int total_stk;
    unsigned exc_rel;
    unsigned dbg_rel;

    /* first, dump the header */
    argc = (unsigned char)G_cs->get_byte_at(start_ofs);
    locals = G_cs->readu2_at(start_ofs + 2);
    total_stk = G_cs->readu2_at(start_ofs + 4);
    exc_rel = G_cs->readu2_at(start_ofs + 6);
    dbg_rel = G_cs->readu2_at(start_ofs + 8);
    G_disasm_out->print("%8lx .code\n", start_ofs);
    G_disasm_out->print("         .argcount %d%s\n",
                        (argc & 0x7f),
                        (argc & 0x80) != 0 ? "+" : "");
    G_disasm_out->print("         .locals %d\n", locals);
    G_disasm_out->print("         .maxstack %d\n", total_stk);

    /* set up a code stream reader and dump the code stream */
    CTcUnasSrcCodeBody src(G_cs, code_start_ofs, code_end_ofs);
    CTcT3Unasm::disasm(&src, G_disasm_out);

    /* show the exception table, if there is one */
    if (exc_rel != 0)
    {
        unsigned long exc_ofs;
        unsigned long exc_end_ofs;

        /* get the starting address */
        exc_ofs = start_ofs + exc_rel;

        /* 
         *   get the length - it's the part up to the debug records, or the
         *   part up to the current code offset if there are no debug records
         */
        exc_end_ofs = (dbg_rel != 0 ? start_ofs + dbg_rel : G_cs->get_ofs());

        /* show the table */
        G_disasm_out->print(".exceptions\n");
        CTcUnasSrcCodeBody exc_src(G_cs, exc_ofs, exc_end_ofs);
        CTcT3Unasm::show_exc_table(&exc_src, G_disasm_out, start_ofs);
    }

    /* add a blank line at the end */
    G_disasm_out->print("\n");
}

/*
 *   Run through the generated code stream starting at the given offset (and
 *   running up to the current offset), and eliminate jump-to-jump sequences:
 *   
 *   - whenever we find any jump instruction that points directly to an
 *   unconditional jump, we'll change the first jump so that it points to
 *   the target of the second jump, saving the unnecessary stop at the
 *   intermediate jump
 *   
 *   - whenever we find an unconditional jump to any return or throw
 *   instruction, we'll replace the jump with a copy of the target
 *   return/throw instruction.  
 */
void CTPNCodeBody::remove_jumps_to_jumps(ulong start_ofs)
{
    ulong ofs;
    ulong end_ofs;
    uchar prv_op;
    static const size_t op_siz[] =
    {
        0,                                                 /* 0x00 - unused */

        1,                                             /* 0x01 - OPC_PUSH_0 */
        1,                                             /* 0x02 - OPC_PUSH_1 */
        2,                                           /* 0x03 - OPC_PUSHINT8 */
        5,                                            /* 0x04 - OPC_PUSHINT */
        5,                                            /* 0x05 - OPC_PUSHSTR */
        5,                                            /* 0x06 - OPC_PUSHLST */
        5,                                            /* 0x07 - OPC_PUSHOBJ */
        1,                                            /* 0x08 - OPC_PUSHNIL */
        1,                                           /* 0x09 - OPC_PUSHTRUE */
        3,                                         /* 0x0A - OPC_PUSHPROPID */
        5,                                          /* 0x0B - OPC_PUSHFNPTR */
        0,               /* 0x0C - OPC_PUSHSTRI - variable-size instruction */
        2,                                         /* 0x0D - OPC_PUSHPARLST */
        1,                                         /* 0x0E - OPC_MAKELSTPAR */
        5,                                           /* 0x0F - OPC_PUSHENUM */

        1,                                                 /* 0x10 - unused */
        1,                                                 /* 0x11 - unused */
        1,                                                 /* 0x12 - unused */
        1,                                                 /* 0x13 - unused */
        1,                                                 /* 0x14 - unused */
        1,                                                 /* 0x15 - unused */
        1,                                                 /* 0x16 - unused */
        1,                                                 /* 0x17 - unused */
        1,                                                 /* 0x18 - unused */
        1,                                                 /* 0x19 - unused */
        1,                                                 /* 0x1A - unused */
        1,                                                 /* 0x1B - unused */
        1,                                                 /* 0x1C - unused */
        1,                                                 /* 0x1D - unused */
        1,                                                 /* 0x1E - unused */
        1,                                                 /* 0x1F - unused */

        1,                                                /* 0x20 - OPC_NEG */
        1,                                               /* 0x21 - OPC_BNOT */
        1,                                                /* 0x22 - OPC_ADD */
        1,                                                /* 0x23 - OPC_SUB */
        1,                                                /* 0x24 - OPC_MUL */
        1,                                               /* 0x25 - OPC_BAND */
        1,                                                /* 0x26 - OPC_BOR */
        1,                                                /* 0x27 - OPC_SHL */
        1,                                                /* 0x28 - OPC_SHR */
        1,                                                /* 0x29 - OPC_XOR */
        1,                                                /* 0x2A - OPC_DIV */
        1,                                                /* 0x2B - OPC_MOD */
        1,                                                /* 0x2C - OPC_NOT */
        1,                                            /* 0x2D - OPC_BOOLIZE */
        1,                                                /* 0x2E - OPC_INC */
        1,                                                /* 0x2F - OPC_DEC */

        1,                                                 /* 0x30 - unused */
        1,                                                 /* 0x31 - unused */
        1,                                                 /* 0x32 - unused */
        1,                                                 /* 0x33 - unused */
        1,                                                 /* 0x34 - unused */
        1,                                                 /* 0x35 - unused */
        1,                                                 /* 0x36 - unused */
        1,                                                 /* 0x37 - unused */
        1,                                                 /* 0x38 - unused */
        1,                                                 /* 0x39 - unused */
        1,                                                 /* 0x3A - unused */
        1,                                                 /* 0x3B - unused */
        1,                                                 /* 0x3C - unused */
        1,                                                 /* 0x3D - unused */
        1,                                                 /* 0x3E - unused */
        1,                                                 /* 0x3F - unused */

        1,                                                 /* 0x40 - OPC_EQ */
        1,                                                 /* 0x41 - OPC_NE */
        1,                                                 /* 0x42 - OPC_LT */
        1,                                                 /* 0x43 - OPC_LE */
        1,                                                 /* 0x44 - OPC_GT */
        1,                                                 /* 0x45 - OPC_GE */

        1,                                                 /* 0x46 - unused */
        1,                                                 /* 0x47 - unused */
        1,                                                 /* 0x48 - unused */
        1,                                                 /* 0x49 - unused */
        1,                                                 /* 0x4A - unused */
        1,                                                 /* 0x4B - unused */
        1,                                                 /* 0x4C - unused */
        1,                                                 /* 0x4D - unused */
        1,                                                 /* 0x4E - unused */
        1,                                                 /* 0x4F - unused */

        1,                                             /* 0x50 - OPC_RETVAL */
        1,                                             /* 0x51 - OPC_RETNIL */
        1,                                            /* 0x52 - OPC_RETTRUE */
        1,                                                 /* 0x53 - unused */
        1,                                                /* 0x54 - OPC_RET */

        1,                                                 /* 0x55 - unused */
        1,                                                 /* 0x56 - unused */
        1,                                                 /* 0x57 - unused */

        6,                                               /* 0x58 - OPC_CALL */
        2,                                            /* 0x59 - OPC_PTRCALL */

        1,                                                 /* 0x5A - unused */
        1,                                                 /* 0x5B - unused */
        1,                                                 /* 0x5C - unused */
        1,                                                 /* 0x5D - unused */
        1,                                                 /* 0x5E - unused */
        1,                                                 /* 0x5F - unused */

        3,                                            /* 0x60 - OPC_GETPROP */
        4,                                           /* 0x61 - OPC_CALLPROP */
        2,                                        /* 0x62 - OPC_PTRCALLPROP */
        3,                                        /* 0x63 - OPC_GETPROPSELF */
        4,                                       /* 0x64 - OPC_CALLPROPSELF */
        2,                                    /* 0x65 - OPC_PTRCALLPROPSELF */
        7,                                         /* 0x66 - OPC_OBJGETPROP */
        8,                                        /* 0x67 - OPC_OBJCALLPROP */
        3,                                        /* 0x68 - OPC_GETPROPDATA */
        1,                                     /* 0x69 - OPC_PTRGETPROPDATA */
        4,                                        /* 0x6A - OPC_GETPROPLCL1 */
        5,                                       /* 0x6B - OPC_CALLPROPLCL1 */
        3,                                          /* 0x6C - OPC_GETPROPR0 */
        4,                                         /* 0x6D - OPC_CALLPROPR0 */

        1,                                                 /* 0x6E - unused */
        1,                                                 /* 0x6F - unused */
        1,                                                 /* 0x70 - unused */
        1,                                                 /* 0x71 - unused */

        4,                                            /* 0x72 - OPC_INHERIT */
        2,                                         /* 0x73 - OPC_PTRINHERIT */
        8,                                         /* 0x74 - OPC_EXPINHERIT */
        6,                                      /* 0x75 - OPC_PTREXPINHERIT */
        1,                                            /* 0x76 - OPC_VARARGC */
        4,                                           /* 0x77 - OPC_DELEGATE */
        2,                                        /* 0x78 - OPC_PTRDELEGATE */

        1,                                                 /* 0x79 - unused */
        1,                                                 /* 0x7A - unused */
        1,                                                 /* 0x7B - unused */
        1,                                                 /* 0x7C - unused */
        1,                                                 /* 0x7D - unused */
        1,                                                 /* 0x7E - unused */
        1,                                                 /* 0x7F - unused */

        2,                                            /* 0x80 - OPC_GETLCL1 */
        3,                                            /* 0x81 - OPC_GETLCL2 */
        2,                                            /* 0x82 - OPC_GETARG1 */
        3,                                            /* 0x83 - OPC_GETARG2 */
        1,                                           /* 0x84 - OPC_PUSHSELF */
        5,                                           /* 0x85 - OPC_GETDBLCL */
        5,                                           /* 0x86 - OPC_GETDBARG */
        1,                                            /* 0x87 - OPC_GETARGC */
        1,                                                /* 0x88 - OPC_DUP */
        1,                                               /* 0x89 - OPC_DISC */
        2,                                              /* 0x8A - OPC_DISC1 */
        1,                                              /* 0x8B - OPC_GETR0 */
        3,                                          /* 0x8C - OPC_GETDBARGC */
        1,                                               /* 0x8D - OPC_SWAP */

        2,                                         /* 0x8E - OPC_PUSHCTXELE */

        1,                                                 /* 0x8F - unused */

        0,                 /* 0x90 - OPC_SWITCH - variable-size instruction */
        3,                                                /* 0x91 - OPC_JMP */
        3,                                                 /* 0x92 - OPC_JT */
        3,                                                 /* 0x93 - OPC_JF */
        3,                                                 /* 0x94 - OPC_JE */
        3,                                                /* 0x95 - OPC_JNE */
        3,                                                /* 0x96 - OPC_JGT */
        3,                                                /* 0x97 - OPC_JGE */
        3,                                                /* 0x98 - OPC_JLT */
        3,                                                /* 0x99 - OPC_JLE */
        3,                                                /* 0x9A - OPC_JST */
        3,                                                /* 0x9B - OPC_JSF */
        3,                                               /* 0x9C - OPC_LJSR */
        3,                                               /* 0x9D - OPC_LRET */
        3,                                               /* 0x9E - OPC_JNIL */
        3,                                            /* 0x9F - OPC_JNOTNIL */
        3,                                               /* 0xA0 - OPC_JR0T */
        3,                                               /* 0xA1 - OPC_JR0F */

        1,                                                 /* 0xA2 - unused */
        1,                                                 /* 0xA3 - unused */
        1,                                                 /* 0xA4 - unused */
        1,                                                 /* 0xA5 - unused */
        1,                                                 /* 0xA6 - unused */
        1,                                                 /* 0xA7 - unused */
        1,                                                 /* 0xA8 - unused */
        1,                                                 /* 0xA9 - unused */
        1,                                                 /* 0xAA - unused */
        1,                                                 /* 0xAB - unused */
        1,                                                 /* 0xAC - unused */
        1,                                                 /* 0xAD - unused */
        1,                                                 /* 0xAE - unused */
        1,                                                 /* 0xAF - unused */

        5,                                                /* 0xB0 - OPC_SAY */
        3,                                          /* 0xB1 - OPC_BUILTIN_A */
        3,                                          /* 0xB2 - OPC_BUILTIN_B */
        3,                                          /* 0xB3 - OPC_BUILTIN_C */
        3,                                          /* 0xB4 - OPC_BUILTIN_D */
        3,                                           /* 0xB5 - OPC_BUILTIN1 */
        4,                                           /* 0xB6 - OPC_BUILTIN2 */
        0,      /* 0xB7 - OPC_CALLEXT (reserved; not currently implemented) */
        1,                                              /* 0xB8 - OPC_THROW */
        1,                                             /* 0xB9 - OPC_SAYVAL */

        1,                                              /* 0xBA - OPC_INDEX */
        3,                                        /* 0xBB - OPC_IDXLCL1INT8 */
        2,                                           /* 0xBC - OPC_IDXLINT8 */

        1,                                                 /* 0xBD - unused */
        1,                                                 /* 0xBE - unused */
        1,                                                 /* 0xBF - unused */

        3,                                               /* 0xC0 - OPC_NEW1 */
        5,                                               /* 0xC1 - OPC_NEW2 */
        3,                                             /* 0xC2 - OPC_TRNEW1 */
        5,                                             /* 0xC3 - OPC_TRNEW2 */

        1,                                                 /* 0xC4 - unused */
        1,                                                 /* 0xC5 - unused */
        1,                                                 /* 0xC6 - unused */
        1,                                                 /* 0xC7 - unused */
        1,                                                 /* 0xC8 - unused */
        1,                                                 /* 0xC9 - unused */
        1,                                                 /* 0xCA - unused */
        1,                                                 /* 0xCB - unused */
        1,                                                 /* 0xCC - unused */
        1,                                                 /* 0xCD - unused */
        1,                                                 /* 0xCE - unused */
        1,                                                 /* 0xCF - unused */

        3,                                             /* 0xD0 - OPC_INCLCL */
        3,                                             /* 0xD1 - OPC_DECLCL */
        3,                                           /* 0xD2 - OPC_ADDILCL1 */
        7,                                           /* 0xD3 - OPC_ADDILCL4 */
        3,                                           /* 0xD4 - OPC_ADDTOLCL */
        3,                                         /* 0xD5 - OPC_SUBFROMLCL */
        2,                                           /* 0xD6 - OPC_ZEROLCL1 */
        3,                                           /* 0xD7 - OPC_ZEROLCL2 */
        2,                                            /* 0xD8 - OPC_NILLCL1 */
        3,                                            /* 0xD9 - OPC_NILLCL2 */
        2,                                            /* 0xDA - OPC_ONELCL1 */
        3,                                            /* 0xDB - OPC_ONELCL2 */

        1,                                                 /* 0xDC - unused */
        1,                                                 /* 0xDD - unused */
        1,                                                 /* 0xDE - unused */
        1,                                                 /* 0xDF - unused */

        2,                                            /* 0xE0 - OPC_SETLCL1 */
        3,                                            /* 0xE1 - OPC_SETLCL2 */
        2,                                            /* 0xE2 - OPC_SETARG1 */
        3,                                            /* 0xE3 - OPC_SETARG2 */
        1,                                             /* 0xE4 - OPC_SETIND */
        3,                                            /* 0xE5 - OPC_SETPROP */
        1,                                         /* 0xE6 - OPC_PTRSETPROP */
        3,                                        /* 0xE7 - OPC_SETPROPSELF */
        7,                                         /* 0xE8 - OPC_OBJSETPROP */
        5,                                           /* 0xE9 - OPC_SETDBLCL */
        5,                                           /* 0xEA - OPC_SETDBARG */

        1,                                            /* 0xEB - OPC_SETSELF */
        1,                                            /* 0xEC - OPC_LOADCTX */
        1,                                           /* 0xED - OPC_STORECTX */
        2,                                          /* 0xEE - OPC_SETLCL1R0 */
        3,                                       /* 0xEF - OPC_SETINDLCL1I8 */

        1,                                                 /* 0xF0 - unused */

        1,                                                 /* 0xF1 - OPC_BP */
        1,                                                /* 0xF2 - OPC_NOP */

        1,                                                 /* 0xF3 - unused */
        1,                                                 /* 0xF4 - unused */
        1,                                                 /* 0xF5 - unused */
        1,                                                 /* 0xF6 - unused */
        1,                                                 /* 0xF7 - unused */
        1,                                                 /* 0xF8 - unused */
        1,                                                 /* 0xF9 - unused */
        1,                                                 /* 0xFA - unused */
        1,                                                 /* 0xFB - unused */
        1,                                                 /* 0xFC - unused */
        1,                                                 /* 0xFD - unused */
        1,                                                 /* 0xFE - unused */
        255,                                               /* 0xFF - unused */
    };
    
    /* 
     *   scan the code stream starting at the given offset, continuing
     *   through the current offset 
     */
    prv_op = OPC_NOP;
    for (ofs = start_ofs, end_ofs = G_cs->get_ofs() ; ofs < end_ofs ; )
    {
        uchar op;
        ulong target_ofs;
        ulong orig_target_ofs;
        uchar target_op;
        int done;
        int chain_len;
        
        /* check the byte code instruction at the current location */
        switch(op = G_cs->get_byte_at(ofs))
        {
        case OPC_RETVAL:
            /* 
             *   If our previous opcode was PUSHTRUE or PUSHNIL, we can
             *   replace the previous opcode with RETTRUE or RETNIL.  This
             *   sequence can occur when we generate conditional code that
             *   returns a value; in such cases, we sometimes can't elide
             *   the PUSHx/RETVAL sequence during the original code
             *   generation because the RETVAL itself is the target of a
             *   label and thus must be retained as a separate instruction.
             *   Converting the PUSHTRUE or PUSHNIL here won't do any harm,
             *   as we'll still leave the RETVAL as a separate instruction.
             *   Likewise, if the previous instruction was GET_R0, we can
             *   change it to a simple RET.  
             */
            switch(prv_op)
            {
            case OPC_PUSHTRUE:
                /* convert the PUSHTRUE to a RETTRUE */
                G_cs->write_at(ofs - 1, OPC_RETTRUE);
                break;

            case OPC_PUSHNIL:
                /* convert the PUSHNIL to a RETNIL */
                G_cs->write_at(ofs - 1, OPC_RETNIL);
                break;

            case OPC_GETR0:
                /* convert the GETR0 to a RET */
                G_cs->write_at(ofs - 1, OPC_RET);
                break;
            }

            /* skip the RETVAL */
            ofs += 1;
            break;
            
        case OPC_PUSHSTRI:
            /* 
             *   push in-line string: we have a UINT2 operand giving the
             *   length in bytes of the string, followed by the bytes of the
             *   string, so read the uint2 and then skip that amount plus
             *   three additional bytes (one for the opcode, two for the
             *   uint2 itself) 
             */
            ofs += 3 + G_cs->readu2_at(ofs+1);
            break;

        case OPC_SWITCH:
            /* 
             *   Switch: we have a UINT2 giving the number of cases,
             *   followed by the cases, followed by an INT2; each case
             *   consists of a DATAHOLDER plus a UINT2, for a total of 7
             *   bytes.  The total is thus 5 bytes (the opcode, the case
             *   count UINT2, the final INT2) plus 7 bytes times the number
             *   of cases.  
             */
            ofs += 5 + 7*G_cs->readu2_at(ofs+1);
            break;

        case OPC_JMP:
            /*
             *   Unconditional jump: check for a jump to a RETURN of any
             *   kind or a THROW.  If the destination consists of either of
             *   those, replace the JMP with the target instruction.  If the
             *   destination is an unconditional JMP, iteratively check its
             *   destination.  
             */
            orig_target_ofs = target_ofs = ofs + 1 + G_cs->read2_at(ofs + 1);

            /* 
             *   Iterate through any chain of JMP's we find.  Abort if we
             *   try following a chain longer than 20 jumps, in case we
             *   should encounter any circular chains. 
             */
            for (done = FALSE, chain_len = 0 ; !done && chain_len < 20 ;
                 ++chain_len)
            {
                switch(target_op = G_cs->get_byte_at(target_ofs))
                {
                case OPC_RETVAL:
                    /*
                     *   Check for a special sequence that we can optimize
                     *   even better than the usual.  If we have a GETR0
                     *   followed by a JMP to a RETVAL, then we can
                     *   eliminate the JMP *and* the GETR0, and just convert
                     *   the GETR0 to a RET.
                     */
                    if (prv_op == OPC_GETR0)
                    {
                        /* 
                         *   The GETR0 is the byte before the original JMP:
                         *   simply replace it with a RET.  Note that we can
                         *   leave the original jump intact, in case anyone
                         *   else is pointing to it.  
                         */
                        G_cs->write_at(ofs - 1, OPC_RET);

                        /* we're done iterating the chain of jumps-to-jumps */
                        done = TRUE;
                    }
                    else
                    {
                        /* handle the same as any return instruction */
                        goto any_RET;
                    }
                    break;

                case OPC_RETNIL:
                case OPC_RETTRUE:
                case OPC_RET:
                case OPC_THROW:
                any_RET:
                    /* 
                     *   it's a THROW or RETURN of some kind - simply copy
                     *   it to the current slot; write NOP's over our jump
                     *   offset operand, to make sure the code continues to
                     *   be deterministically readable 
                     */
                    G_cs->write_at(ofs, target_op);
                    G_cs->write_at(ofs + 1, (uchar)OPC_NOP);
                    G_cs->write_at(ofs + 2, (uchar)OPC_NOP);

                    /* we're done iterating the chain of jumps-to-jumps */
                    done = TRUE;
                    break;

                case OPC_JMP:
                    /* 
                     *   We're jumping to another jump - there's no reason
                     *   to stop at the intermediate jump instruction, since
                     *   we can simply jump directly to the destination
                     *   address.  Calculate the new target address, and
                     *   continue iterating, in case this jumps to something
                     *   we can further optimize away.  
                     */
                    target_ofs = target_ofs + 1
                                 + G_cs->read2_at(target_ofs + 1);

                    /* 
                     *   if it's a jump to the original location, it must be
                     *   some unreachable code that generated a circular
                     *   jump; ignore it in this case 
                     */
                    if (target_ofs == ofs)
                        done = TRUE;

                    /* proceed */
                    break;

                default:
                    /* 
                     *   For anything else, we're done with any chain of
                     *   jumps to jumps.  If we indeed found a new target
                     *   address, rewrite the original JMP instruction so
                     *   that it jumps directly to the end of the chain
                     *   rather than going through the intermediate jumps.  
                     */
                    if (target_ofs != orig_target_ofs)
                        G_cs->write2_at(ofs + 1, target_ofs - (ofs + 1));

                    /* we're done iterating */
                    done = TRUE;
                    break;
                }
            }

            /* whatever happened, skip past the jump */
            ofs += 3;

            /* done */
            break;

        case OPC_JT:
        case OPC_JF:
        case OPC_JE:
        case OPC_JNE:
        case OPC_JGT:
        case OPC_JGE:
        case OPC_JLT:
        case OPC_JLE:
        case OPC_JST:
        case OPC_JSF:
        case OPC_JNIL:
        case OPC_JNOTNIL:
        case OPC_JR0T:
        case OPC_JR0F:
            /*
             *   We have a jump (conditional or otherwise).  Check the
             *   target instruction to see if it's an unconditional jump; if
             *   so, then we can jump straight to the target of the second
             *   jump, since there's no reason to stop at the intermediate
             *   jump instruction on our way to the final destination.  Make
             *   this check iteratively, so that we eliminate any chain of
             *   jumps to jumps and land at our final non-jump instruction
             *   in one go. 
             */
            orig_target_ofs = target_ofs = ofs + 1 + G_cs->read2_at(ofs + 1);
            for (done = FALSE, chain_len = 0 ; !done && chain_len < 20 ;
                 ++chain_len)
            {
                uchar target_op;

                /* get the target opcode */
                target_op = G_cs->get_byte_at(target_ofs);

                /* 
                 *   if the target is an unconditional JMP, we can retarget
                 *   the original instruction to jump directly to the target
                 *   of the target JMP, bypassing the target JMP entirely and
                 *   thus avoiding some unnecessary work at run-time 
                 */
                if (target_op == OPC_JMP)
                {
                    /* 
                     *   retarget the original jump to go directly to the
                     *   target of the target JMP 
                     */
                    target_ofs = target_ofs + 1
                                 + G_cs->read2_at(target_ofs + 1);
                    
                    /* 
                     *   continue scanning for more opportunities, as the new
                     *   target could also point to something we can bypass 
                     */
                    continue;
                }
                
                /*
                 *   Certain combinations are special.  If the original
                 *   opcode was a JST or JSF, and the target is a JT or JF,
                 *   we can recode the sequence so that the original opcode
                 *   turns into a more efficient JT or JF and jumps directly
                 *   past the JT or JF.  If we have a JST or JSF jumping to a
                 *   JST or JSF, we can also recode that sequence to bypass
                 *   the second jump.  In both cases, we can recode the
                 *   sequence because the original jump will unequivocally
                 *   determine the behavior at the target jump in such a way
                 *   that we can compact the sequence into a single jump.  
                 */
                switch(op)
                {
                case OPC_JSF:
                    /* 
                     *   the original is a JSF: we can recode a jump to a
                     *   JSF, JST, JF, or JT 
                     */
                    switch(target_op)
                    {
                    case OPC_JSF:
                        /* 
                         *   We're jumping to another JSF.  Since the
                         *   original jump will only reach the target jump if
                         *   the value on the top of the stack is false, and
                         *   will then leave this same value on the stack to
                         *   be tested again with the target JSF, we know the
                         *   target JSF will perform its jump and leave the
                         *   stack unchanged again.  So, we can simply
                         *   retarget the original jump to the target of the
                         *   target JSF.  
                         */
                        target_ofs = target_ofs + 1
                                     + G_cs->read2_at(target_ofs + 1);

                        /* keep scanning for additional opportunities */
                        break;

                    case OPC_JST:
                    case OPC_JT:
                        /*
                         *   We're jumping to a JST or a JT.  Since the JSF
                         *   will only reach the JST/JT on a false value, we
                         *   know the JST/JT will NOT jump - we know for a
                         *   fact it will pop the non-true stack element and
                         *   proceed without jumping.  Therefore, we can
                         *   avoid saving the value from the original JSF,
                         *   which means we can recode the original as the
                         *   simpler JF (which doesn't bother saving the
                         *   false value), and jump on false directly to the
                         *   instruction after the target JST/JT.  
                         */
                        G_cs->write_at(ofs, (uchar)OPC_JF);
                        op = OPC_JF;

                        /* jump to the instruction after the target JST/JT */
                        target_ofs += 3;

                        /* keep looking for more jumps */
                        break;

                    case OPC_JF:
                        /* 
                         *   We're jumping to a JF: we know the JF will jump,
                         *   because we had to have - and then save - a false
                         *   value for the JSF to reach the JF in the first
                         *   place.  Since we know for a fact the target JF
                         *   will remove the false value and jump to its
                         *   target, we can bypass the target JF by recoding
                         *   the original instruction as a simpler JF and
                         *   jumping directly to the target of the target JF.
                         */
                        G_cs->write_at(ofs, (uchar)OPC_JF);
                        op = OPC_JF;

                        /* jump to the tartet of the target JF */
                        target_ofs = target_ofs + 1
                                     + G_cs->read2_at(target_ofs + 1);

                        /* keep scanning for more jumps */
                        break;

                    default:
                        /* can't make any assumptions about other targets */
                        done = TRUE;
                        break;
                    }
                    break;

                case OPC_JST:
                    /* 
                     *   the original is a JST: recode it if the target is a
                     *   JSF, JST, JF, or JT 
                     */
                    switch(target_op)
                    {
                    case OPC_JST:
                        /* JST jumping to JST: jump to the target's target */
                        target_ofs = target_ofs + 1
                                     + G_cs->read2_at(target_ofs + 1);

                        /* keep looking */
                        break;

                    case OPC_JSF:
                    case OPC_JF:
                        /* 
                         *   JST jumping to JSF/JF: the JSF/JF will
                         *   definitely pop the stack and not jump (since the
                         *   original JST will have left a true value on the
                         *   stack), so we can recode the JST as a more
                         *   efficient JT and jump to the instruction after
                         *   the JSF/JF target 
                         */
                        G_cs->write_at(ofs, (uchar)OPC_JT);
                        op = OPC_JT;

                        /* jump to the instruction after the target */
                        target_ofs += 3;

                        /* keep looking */
                        break;

                    case OPC_JT:
                        /* 
                         *   JST jumping to JT: the JT will definitely pop
                         *   and jump, so we can recode the original as a
                         *   simpler JT and jump to the target's target 
                         */
                        G_cs->write_at(ofs, (uchar)OPC_JT);
                        op = OPC_JT;

                        /* jump to the target of the target */
                        target_ofs = target_ofs + 1
                                     + G_cs->read2_at(target_ofs + 1);

                        /* keep scanning */
                        break;

                    default:
                        /* can't make any assumptions about other targets */
                        done = TRUE;
                        break;
                    }
                    break;

                default:
                    /* 
                     *   we can't make assumptions about anything else, so
                     *   we've come to the end of the road - stop scanning 
                     */
                    done = TRUE;
                    break;
                }
            }

            /* 
             *   if we found a chain of jumps, replace our original jump
             *   target with the final jump target, bypassing the
             *   intermediate jumps 
             */
            if (target_ofs != orig_target_ofs)
                G_cs->write2_at(ofs + 1, target_ofs - (ofs + 1));

            /* skip past the jump */
            ofs += 3;

            /* done */
            break;

        default:
            /* 
             *   everything else is a fixed-size instruction, so simply
             *   consult our table of instruction lengths to determine the
             *   offset of the next instruction 
             */
            ofs += op_siz[op];
            break;
        }

        /* remember the preceding opcode */
        prv_op = op;
    }
}

/*
 *   Check for unreferenced local variables 
 */
void CTPNCodeBody::check_locals()
{
    CTcPrsSymtab *tab;
    
    /* check for unreferenced locals in each nested scope */
    for (tab = first_nested_symtab_ ; tab != 0 ; tab = tab->get_list_next())
    {
        /* check this table */
        tab->check_unreferenced_locals();
    }
}

/* 
 *   local symbol table enumerator for checking for parameter symbols that
 *   belong in the local context 
 */
void CTPNCodeBody::enum_for_param_ctx(void *, class CTcSymbol *sym)
{
    /* if this is a local, check it further */
    if (sym->get_type() == TC_SYM_LOCAL || sym->get_type() == TC_SYM_PARAM)
    {
        CTcSymLocal *lcl = (CTcSymLocal *)sym;

        /* 
         *   if it's a parameter, and it's also a context variable, its
         *   value needs to be moved into the context 
         */
        if (lcl->is_param() && lcl->is_ctx_local())
        {
            /* get the actual parameter value from the stack */
            CTcSymLocal::s_gen_code_getlcl(lcl->get_var_num(), TRUE);

            /* store the value in the context variable */
            lcl->gen_code_asi(TRUE, TC_ASI_SIMPLE, 0, TRUE);
        }
    }
}


/* ------------------------------------------------------------------------ */
/*
 *   'return' statement 
 */

/*
 *   generate code 
 */
void CTPNStmReturn::gen_code(int, int)
{
    int val_on_stack;
    int need_gen;

    /* add a line record */
    add_debug_line_rec();

    /* presume we'll generate a value */
    need_gen = TRUE;

    /* generate the return value expression, if appropriate */
    if (expr_ != 0)
    {
        /* 
         *   it's an error if we're in a constructor, because a
         *   constructor implicitly always returns 'self' 
         */
        if (G_cg->is_in_constructor())
            log_error(TCERR_CONSTRUCT_CANNOT_RET_VAL);

        /* check for a constant expression */
        if (expr_->is_const())
        {
            switch(expr_->get_const_val()->get_type())
            {
            case TC_CVT_NIL:
            case TC_CVT_TRUE:
                /* 
                 *   we can use special constant return instructions for
                 *   these, so there's no need to generate the value 
                 */
                need_gen = FALSE;
                break;

            default:
                /* 
                 *   other types don't have constant-return opcodes, so we
                 *   must generate the expression code 
                 */
                need_gen = TRUE;
                break;
            }
        }

        /* if necessary, generate the value */
        if (need_gen)
        {
            int depth;

            /* note the initial stack depth */
            depth = G_cg->get_sp_depth();

            /*  
             *   Generate the value.  We are obviously not discarding the
             *   value, and since returning a value is equivalent to
             *   assigning the value, we must use the stricter assignment
             *   (not 'for condition') rules for logical expressions 
             */
            expr_->gen_code(FALSE, FALSE);

            /* note whether we actually left a value on the stack */
            val_on_stack = (G_cg->get_sp_depth() > depth);
        }
        else
        {
            /* 
             *   we obviously aren't leaving a value on the stack if we
             *   don't generate anything 
             */
            val_on_stack = FALSE;
        }
    }

    /* 
     *   Before we return, let any enclosing statements generate any code
     *   necessary to leave their scope (in particular, we must invoke
     *   'finally' handlers in any enclosing 'try' blocks).
     *   
     *   Note that we generated the expression BEFORE we call any
     *   'finally' handlers.  This is necessary because something we call
     *   in the course of evaluating the return value could have thrown an
     *   exception; if we were to call the 'finally' clauses before
     *   generating the return value, we could invoke the 'finally' clause
     *   twice (once explicitly, once in the handling of the thrown
     *   exception), which would be incorrect.  By generating the
     *   'finally' calls after the return expression, we're sure that the
     *   'finally' blocks are invoked only once - either through the
     *   throw, or else now, after there's no more possibility of a
     *   'throw' before the return.  
     */
    if (G_cs->get_enclosing() != 0)
    {
        int did_save_retval;
        uint fin_ret_lcl;

        /* 
         *   if we're going to generate any subroutine calls, and we have
         *   a return value on the stack, we need to save the return value
         *   in a local to make sure the calculated value isn't affected
         *   by the subroutine call 
         */
        if (val_on_stack
            && G_cs->get_enclosing()->will_gen_code_unwind_for_return()
            && G_cs->get_code_body() != 0)
        {
            /* allocate a local variable to save the return value */
            fin_ret_lcl = G_cs->get_code_body()->alloc_fin_ret_lcl();

            /* save the return value in a stack temporary for a moment */
            CTcSymLocal::s_gen_code_setlcl_stk(fin_ret_lcl, FALSE);

            /* 
             *   note that we saved the return value, so we can retrieve
             *   it later 
             */
            did_save_retval = TRUE;
        }
        else
        {
            /* note that we didn't save the return value */
            did_save_retval = FALSE;
        }

        /* generate the unwind */
        G_cs->get_enclosing()->gen_code_unwind_for_return();

        /* if we saved the return value, retrieve it */
        if (did_save_retval)
            CTcSymLocal::s_gen_code_getlcl(fin_ret_lcl, FALSE);
    }

    /* check for an expression to return */
    if (G_cg->is_in_constructor())
    {
        /* we're in a constructor - return 'self' */
        G_cg->write_op(OPC_PUSHSELF);
        G_cg->write_op(OPC_RETVAL);
    }
    else if (expr_ == 0)
    {
        /* 
         *   there's no expression - generate a simple void return (but
         *   explicitly return nil, so we don't return something left in
         *   R0 from a previous function call we made) 
         */
        G_cg->write_op(OPC_RETNIL);
    }
    else
    {
        /* check for a constant expression */
        if (expr_->is_const())
        {
            switch(expr_->get_const_val()->get_type())
            {
            case TC_CVT_NIL:
                /* generate a RETNIL instruction */
                G_cg->write_op(OPC_RETNIL);
                break;

            case TC_CVT_TRUE:
                /* generate a RETTRUE instruction */
                G_cg->write_op(OPC_RETTRUE);
                break;

            default:
                break;
            }
        }

        /* 
         *   if we needed code generation to evaluate the return value, we
         *   now need to return the value 
         */
        if (need_gen)
        {
            /* 
             *   Other types don't have constant-return opcodes.  We
             *   already generated the expression value (before invoking
             *   the enclosing 'finally' handlers, if any), so the value
             *   is on the stack, and all we need to do is return it.
             *   
             *   If we didn't actually leave a value on the stack, we'll
             *   just return nil.  
             */
            if (val_on_stack)
            {
                /* generate the return-value opcode */
                G_cg->write_op(OPC_RETVAL);
                
                /* RETVAL removes an element from the stack */
                G_cg->note_pop();
            }
            else
            {
                /* 
                 *   The depth didn't change - they must have evaluated an
                 *   expression involving a dstring or void function.
                 *   Return nil instead of the non-existent value.  
                 */
                G_cg->write_op(OPC_RETNIL);
            }
        }
    }
}
    
/* ------------------------------------------------------------------------ */
/*
 *   Static property initializer statement 
 */
void CTPNStmStaticPropInit::gen_code(int, int)
{
    int depth;
    
    /* add a line record */
    add_debug_line_rec();

    /* note the initial stack depth */
    depth = G_cg->get_sp_depth();

    /* generate the expression, keeping the generated value */
    expr_->gen_code(FALSE, FALSE);

    /* ensure that we generated a value; if we didn't, push nil by default */
    if (G_cg->get_sp_depth() <= depth)
    {
        /* push a default nil value */
        G_cg->write_op(OPC_PUSHNIL);
        G_cg->note_push();
    }

    /* 
     *   duplicate the value on the stack, so we can assign it to
     *   initialize the property and also return it 
     */
    G_cg->write_op(OPC_DUP);
    G_cg->note_push();

    /* write the SETPROPSELF to initialize the property */
    G_cg->write_op(OPC_SETPROPSELF);
    G_cs->write_prop_id(prop_);

    /* SETPROPSELF removes the value */
    G_cg->note_pop();

    /* return the value (which we duplicated on the stack) */
    G_cg->write_op(OPC_RETVAL);

    /* RETVAL removes the value */
    G_cg->note_pop();
}


/* ------------------------------------------------------------------------ */
/*
 *   Object Definition Statement 
 */

/*
 *   generate code 
 */
void CTPNStmObject::gen_code(int, int)
{
    CTPNSuperclass *sc;
    CTPNObjProp *prop;
    int sc_cnt;
    ulong start_ofs;
    uint internal_flags;
    uint obj_flags;
    CTcDataStream *str;
    int bad_sc;

    /* if this object has been replaced, don't generate any code for it */
    if (replaced_)
        return;

    /* add an implicit constructor if necessary */
    add_implicit_constructor();

    /* get the appropriate stream for generating the data */
    str = obj_sym_->get_stream();

    /* clear the internal flags */
    internal_flags = 0;

    /* 
     *   if we're a modified object, set the 'modified' flag in the object
     *   header 
     */
    if (modified_)
        internal_flags |= TCT3_OBJ_MODIFIED;

    /* set the 'transient' flag if appropriate */
    if (transient_)
        internal_flags |= TCT3_OBJ_TRANSIENT;

    /* clear the object flags */
    obj_flags = 0;

    /* 
     *   If we're specifically marked as a 'class' object, or we're a
     *   modified object, set the 'class' flag in the object flags.  
     */
    if (is_class_ || modified_)
        obj_flags |= TCT3_OBJFLG_CLASS;

    /* remember our starting offset in the object stream */
    start_ofs = str->get_ofs();

    /* 
     *   store our stream offset in our defining symbol, for storage in
     *   the object file 
     */
    obj_sym_->set_stream_ofs(start_ofs);

    /* write our internal flags */
    str->write2(internal_flags);

    /* 
     *   First, write the per-object image file "OBJS" header - each
     *   object starts with its object ID and the number of bytes in the
     *   object's metaclass-specific data.  For now, write zero as a
     *   placeholder for our data size.  Note that this is a
     *   self-reference: it must be modified if the object is renumbered.  
     */
    str->write_obj_id_selfref(obj_sym_);
    str->write2(0);

    /* write a placeholder for the superclass count */
    str->write2(0);

    /* write the fixed property count */
    str->write2(prop_cnt_);

    /* write the object flags */
    str->write2(obj_flags);

    /*
     *   First, go through the superclass list and verify that each
     *   superclass is actually an object.  
     */
    for (bad_sc = FALSE, sc_cnt = 0, sc = first_sc_ ; sc != 0 ; sc = sc->nxt_)
    {
        CTcSymObj *sc_sym;

        /* look up the superclass in the global symbol table */
        sc_sym = (CTcSymObj *)sc->get_sym();

        /* make sure it's defined, and that it's really an object */
        if (sc_sym == 0)
        {
            /* not defined */
            log_error(TCERR_UNDEF_SYM_SC,
                      (int)sc->get_sym_len(), sc->get_sym_txt(),
                      (int)obj_sym_->get_sym_len(), obj_sym_->get_sym());

            /* note that we have an invalid superclass */
            bad_sc = TRUE;
        }
        else if (sc_sym->get_type() != TC_SYM_OBJ)
        {
            /* log an error */
            log_error(TCERR_SC_NOT_OBJECT,
                      (int)sc_sym->get_sym_len(), sc_sym->get_sym());

            /* note that we have an invalid superclass */
            bad_sc = TRUE;
        }
        else
        {
            /* count the superclass */
            ++sc_cnt;

            /* write the superclass to the object header */
            str->write_obj_id(sc_sym->get_obj_id());
        }
    }

    /* 
     *   If we detected a 'bad template' error when we were parsing the
     *   object definition, and all of our superclasses are valid, report the
     *   template error.
     *   
     *   Do not report this error if we have any undefined or invalid
     *   superclasses, because (1) we've already reported one error for this
     *   object definition (the bad superclass error), and (2) the missing
     *   template is likely just a consequence of the bad superclass, since
     *   we can't have scanned the proper superclass's list of templates if
     *   they didn't tell us the correct superclass to start with.  When they
     *   fix the superclass list and re-compile the code, it's likely that
     *   this will fix the template problem as well, since we'll probably be
     *   able to find the template give the corrected superclass list.
     *   
     *   If we found an undescribed class anywhere in our hierarchy, a
     *   template simply cannot be used with this object; otherwise, the
     *   error is that we failed to find a suitable template 
     */
    if (has_bad_template() && !bad_sc)
        log_error(has_undesc_sc()
                  ? TCERR_OBJ_DEF_CANNOT_USE_TEMPLATE
                  : TCERR_OBJ_DEF_NO_TEMPLATE);

    /* go back and write the superclass count to the header */
    str->write2_at(start_ofs + TCT3_TADSOBJ_HEADER_OFS, sc_cnt);

    /*
     *   Write the properties.  We're required to write the properties in
     *   sorted order of property ID, but we can't do that yet, because
     *   the property ID's aren't finalized until after linking.  For now,
     *   just write them out in the order in which they were defined.  
     */
    for (prop = first_prop_ ; prop != 0 ; prop = prop->nxt_)
    {
        /* make sure we have a valid property symbol */
        if (prop->get_prop_sym() != 0)
        {
            /* write the property ID */
            str->write_prop_id(prop->get_prop_sym()->get_prop());

            /* generate code for the property */
            prop->gen_code(FALSE, FALSE);
        }
    }

    /* 
     *   go back and write the size of our metaclass-specific data - this
     *   goes at offset 4 in the T3 generic metaclass header
     */
    str->write2_at(start_ofs + TCT3_META_HEADER_OFS + 4,
                   str->get_ofs() - (start_ofs + TCT3_META_HEADER_OFS + 6));
}

/*
 *   Check for unreferenced local variables 
 */
void CTPNStmObject::check_locals()
{
    CTPNObjProp *prop;

    /* check for unreferenced locals for each property */
    for (prop = first_prop_ ; prop != 0 ; prop = prop->nxt_)
        prop->check_locals();
}
