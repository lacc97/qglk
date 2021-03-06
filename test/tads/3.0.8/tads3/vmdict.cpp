#ifdef RCSID
static char RCSid[] =
"$Header$";
#endif

/* 
 *   Copyright (c) 2000, 2002 Michael J. Roberts.  All Rights Reserved.
 *   
 *   Please see the accompanying license file, LICENSE.TXT, for information
 *   on using and copying this software.  
 */
/*
Name
  vmdict.cpp - dictionary metaclass
Function
  
Notes
  
Modified
  01/24/00 MJRoberts  - Creation
*/

#include <stdlib.h>
#include <string.h>

#include "t3std.h"
#include "vmdict.h"
#include "vmerr.h"
#include "vmerrnum.h"
#include "vmtype.h"
#include "vmglob.h"
#include "vmobj.h"
#include "vmundo.h"
#include "vmmcreg.h"
#include "vmfile.h"
#include "vmhash.h"
#include "vmbif.h"
#include "vmmeta.h"
#include "vmstack.h"
#include "vmstr.h"
#include "vmlst.h"
#include "vmrun.h"
#include "vmstrcmp.h"
#include "vmpredef.h"


/* ------------------------------------------------------------------------ */
/*
 *   Dictionary undo record.  Each time we change the dictionary, we track
 *   the change with one of these records - we attach this record to the
 *   standard undo record via the 'ptrval' field. 
 */
struct dict_undo_rec
{
    /* action type */
    enum dict_undo_action action;

    /* object value stored in dictionary record */
    vm_obj_id_t obj;

    /* vocabulary property for word association */
    vm_prop_id_t prop;

    /* length of the word */
    size_t len;

    /* text of the word */
    char txt[1];
};

/* ------------------------------------------------------------------------ */
/*
 *   Comparator-based hash 
 */
class CVmHashFuncComparator: public CVmHashFunc
{
public:
    CVmHashFuncComparator(VMG_ vm_obj_id_t obj)
    {
        globals_ = VMGLOB_ADDR;
        comparator_ = obj;
    }

    /* compute the hash value */
    unsigned int compute_hash(const char *str, size_t len) const
    {
        CVmObject *cobj;
        vm_val_t val;
        vm_obj_id_t srcobj;
        uint argc;
        VMGLOB_PTR(globals_);

        /* get my comparator object */
        cobj = vm_objp(vmg_ comparator_);

        /* create a string object to represent the argument, and push it */
        G_stk->push()->set_obj(CVmObjString::create(vmg_ FALSE, str, len));

        /* invoke the calcHash method */
        argc = 1;
        if (cobj->get_prop(vmg_ G_predef->calc_hash_prop, &val,
                           comparator_, &srcobj, &argc)
            && val.typ == VM_INT)
        {
            /* return the hash value from the comparator */
            return val.val.intval;
        }
        else
        {
            /* no or invalid hash value - throw an error */
            err_throw(VMERR_BAD_TYPE_BIF);
            return FALSE;
        }
    }

private:
    /* my comparator object */
    vm_obj_id_t comparator_;

    /* system globals */
    vm_globals *globals_;
};

/*
 *   StringComparator-based hash 
 */
class CVmHashFuncStrComp: public CVmHashFunc
{
public:
    CVmHashFuncStrComp(CVmObjStrComp *comp) { comparator_ = comp; }

    /* compute the hash value */
    unsigned int compute_hash(const char *str, size_t len) const
    {
        /* ask the StringComparator to do the work */
        return comparator_->calc_hash(str, len);
    }

private:
    /* my StringComparator object */
    CVmObjStrComp *comparator_;
};


/* ------------------------------------------------------------------------ */
/*
 *   dictionary object statics 
 */

/* metaclass registration object */
static CVmMetaclassDict metaclass_reg_obj;
CVmMetaclass *CVmObjDict::metaclass_reg_ = &metaclass_reg_obj;

/* function table */
int (CVmObjDict::
     *CVmObjDict::func_table_[])(VMG_ vm_obj_id_t self,
                                 vm_val_t *retval, uint *argc) =
{
    &CVmObjDict::getp_undef,
    &CVmObjDict::getp_set_comparator,
    &CVmObjDict::getp_find,
    &CVmObjDict::getp_add,
    &CVmObjDict::getp_del,
    &CVmObjDict::getp_is_defined,
    &CVmObjDict::getp_for_each_word
};

/* ------------------------------------------------------------------------ */
/*
 *   Dictionary object implementation 
 */

/* 
 *   create dynamically using stack arguments 
 */
vm_obj_id_t CVmObjDict::create_from_stack(VMG_ const uchar **pc_ptr,
                                          uint argc)
{
    vm_obj_id_t id;
    vm_obj_id_t comp;
    CVmObjDict *obj;
    
    /* check arguments */
    if (argc != 0 && argc != 1)
        err_throw(VMERR_WRONG_NUM_OF_ARGS);

    /* get the Comparator, but leave it on the stack as gc protection */
    if (argc == 0 || G_stk->get(0)->typ == VM_NIL)
        comp = VM_INVALID_OBJ;
    else if (G_stk->get(0)->typ == VM_OBJ)
        comp = G_stk->get(0)->val.obj;
    else
        err_throw(VMERR_BAD_TYPE_BIF);

    /* 
     *   allocate the object ID - this type of construction never creates
     *   a root object 
     */
    id = vm_new_id(vmg_ FALSE, TRUE, TRUE);

    /* create the object */
    obj = new (vmg_ id) CVmObjDict(vmg0_);

    /* set the comparator */
    obj->set_comparator(vmg_ comp);

    /* build an empty initial hash table */
    obj->create_hash_table(vmg0_);

    /* 
     *   mark the object as modified since image load, since it doesn't
     *   come from the image file at all 
     */
    obj->get_ext()->modified_ = TRUE;

    /* discard our gc protection */
    G_stk->discard();
    
    /* return the new ID */
    return id;
}

/* ------------------------------------------------------------------------ */
/*
 *   constructor 
 */
CVmObjDict::CVmObjDict(VMG0_)
{
    /* allocate our extension structure from the variable heap */
    ext_ = (char *)G_mem->get_var_heap()
           ->alloc_mem(sizeof(vm_dict_ext), this);

    /* we have no image data yet */
    get_ext()->image_data_ = 0;
    get_ext()->image_data_size_ = 0;

    /* no hash table yet */
    get_ext()->hashtab_ = 0;

    /* no non-image entries yet */
    get_ext()->modified_ = FALSE;

    /* no comparator yet */
    get_ext()->comparator_ = VM_INVALID_OBJ;
    set_comparator_type(vmg_ VM_INVALID_OBJ);
}

/* ------------------------------------------------------------------------ */
/* 
 *   notify of deletion 
 */
void CVmObjDict::notify_delete(VMG_ int /*in_root_set*/)
{
    /* free our additional data */
    if (ext_ != 0)
    {
        /* free our hash table */
        if (get_ext()->hashtab_ != 0)
            delete get_ext()->hashtab_;

        /* free the extension */
        G_mem->get_var_heap()->free_mem(ext_);
    }
}

/* ------------------------------------------------------------------------ */
/*
 *   Set the comparator object 
 */
void CVmObjDict::set_comparator(VMG_ vm_obj_id_t obj)
{
    /* if this is the same as the current comparator, there's nothing to do */
    if (obj == get_ext()->comparator_)
        return;

    /* remember the new comparator */
    get_ext()->comparator_ = obj;

    /* figure out what kind of compare functions to use */
    set_comparator_type(vmg_ obj);

    /* rebuild the hash tale */
    if (get_ext()->hashtab_ != 0)
        create_hash_table(vmg0_);
}

/*
 *   Set the string-compare function based on the given comparator.
 */
void CVmObjDict::set_comparator_type(VMG_ vm_obj_id_t obj)
{
    /* check what kind of object we have */
    if (obj == VM_INVALID_OBJ)
    {
        /* there's no comparator at all, so simply use an exact comparison */
        get_ext()->comparator_type_ = VMDICT_COMP_NONE;
    }
    else if (CVmObjStrComp::is_strcmp_obj(vmg_ obj))
    {
        /* it's a StringComparator */
        get_ext()->comparator_type_ = VMDICT_COMP_STRCOMP;
    }
    else
    {
        /* it's a generic comparator object */
        get_ext()->comparator_type_ = VMDICT_COMP_GENERIC;
    }

    /* remember the object pointer, if there is one */
    get_ext()->comparator_obj_ = (obj == VM_INVALID_OBJ
                                  ? 0 : vm_objp(vmg_ obj));
}

/* ------------------------------------------------------------------------ */
/*
 *   Rebuild the hash table.  If there's an existing hash table, we'll move
 *   the entries from the old table to the new table, and delete the old
 *   table.  
 */
void CVmObjDict::create_hash_table(VMG0_)
{
    CVmHashTable *new_tab;
    CVmHashFunc *hash_func;
    
    /*
     *   Create our hash function.  If we have a comparator object, base the
     *   hash function on the comparator; use a special hash function if we
     *   specifically have a StringComparator, since we can call these
     *   directly for better efficiency.  If we have no comparator, create a
     *   generic exact string match hash function.  
     */
    if (get_ext()->comparator_ != VM_INVALID_OBJ)
    {
        /* 
         *   use our special StringComparator hash function if possible;
         *   otherwise, use a generic comparator hash function 
         */
        if (CVmObjStrComp::is_strcmp_obj(vmg_ get_ext()->comparator_))
        {
            /* create a StringComparator hash function */
            hash_func = new CVmHashFuncStrComp(
                (CVmObjStrComp *)vm_objp(vmg_ get_ext()->comparator_));
        }
        else
        {
            /* create a generic comparator hash function */
            hash_func = new CVmHashFuncComparator(
                vmg_ get_ext()->comparator_);
        }
    }
    else
    {
        /* create a simple exact-match hash */
        hash_func = new CVmHashFuncCS();
    }

    /* create the hash table */
    new_tab = new CVmHashTable(256, hash_func, TRUE);

    /* if we had a previous hash table, move its contents to the new table */
    if (get_ext()->hashtab_ != 0)
    {
        /* copy the old hash table to the new one */
        get_ext()->hashtab_->move_entries_to(new_tab);
        
        /* delete the old hash table */
        delete get_ext()->hashtab_;
    }

    /* store the new hash table in the extension */
    get_ext()->hashtab_ = new_tab;
}

/* ------------------------------------------------------------------------ */
/*
 *   Check for a match between two strings, using the current comparator.
 *   Returns true if the strings match, false if not; fills in *result_val
 *   with the actual result value from the comparator's matchValues()
 *   function.  
 */
int CVmObjDict::match_strings(VMG_ const vm_val_t *valstrval,
                              const char *valstr, size_t vallen,
                              const char *refstr, size_t reflen,
                              vm_val_t *result_val)
{
    uint argc;
    vm_obj_id_t srcobj;

    /* check what kind of comparator we have */
    switch(get_ext()->comparator_type_)
    {
    case VMDICT_COMP_NONE:
        /* no comparator - compare the strings byte-for-byte */
        if (vallen == reflen && memcmp(valstr, refstr, vallen) == 0)
        {
            /* match - return true */
            result_val->set_int(1);
            return TRUE;
        }
        else
        {
            /* no match - return false */
            result_val->set_int(0);
            return FALSE;
        }

    case VMDICT_COMP_STRCOMP:
        /* match the value directly with the StringComparator */
        result_val->set_int(
            ((CVmObjStrComp *)get_ext()->comparator_obj_)->match_strings(
                valstr, vallen, refstr, reflen));
        
        /* we matched if the result was non-zero */
        return result_val->val.intval;

    case VMDICT_COMP_GENERIC:
        /* 2nd param: push the reference string, as a string object */
        G_stk->push()->set_obj(
            CVmObjString::create(vmg_ FALSE, refstr, reflen));
        
        /* 1st param: push the value string */
        if (valstrval != 0)
        {
            /* push the value string as given */
            G_stk->push(valstrval);
        }
        else
        {
            /* no value string was given - create one and push it */
            G_stk->push()->set_obj(
                CVmObjString::create(vmg_ FALSE, valstr, vallen));
        }
        
        /* call the comparator's matchValues method */
        argc = 2;
        if (get_ext()->comparator_obj_->get_prop(
            vmg_ G_predef->match_values_prop, result_val,
            get_ext()->comparator_, &srcobj, &argc))
        {
            /* got it - we matched if the result isn't 0 or nil */
            return !(result_val->typ == VM_NIL
                     || (result_val->typ == VM_INT
                         && result_val->val.intval == 0));
        }
        else
        {
            /* didn't even find the method - return no match */
            return FALSE;
        }
    }

    /* we should never get here, but just in case... */
    return FALSE;
}

/* ------------------------------------------------------------------------ */
/*
 *   Calculate a hash value with the current comparator 
 */
unsigned int CVmObjDict::calc_hash(VMG_ const vm_val_t *valstrval,
                                   const char *valstr, size_t vallen)
{
    uint argc;
    vm_val_t result;
    vm_obj_id_t srcobj;

    /* check what kind of comparator we have */
    switch(get_ext()->comparator_type_)
    {
    case VMDICT_COMP_NONE:
        /* no comparator - use the hash table's basic hash calculation */
        return get_ext()->hashtab_->compute_hash(valstr, vallen);

    case VMDICT_COMP_STRCOMP:
        /* calculate the hash directly with the StringComparator */
        return ((CVmObjStrComp *)get_ext()->comparator_obj_)->calc_hash(
            valstr, vallen);

    case VMDICT_COMP_GENERIC:
        /* push the value string */
        if (valstrval != 0)
        {
            /* push the value string as given */
            G_stk->push(valstrval);
        }
        else
        {
            /* no value string was given - create one and push it */
            G_stk->push()->set_obj(
                CVmObjString::create(vmg_ FALSE, valstr, vallen));
        }

        /* call the comparator object's calcHash method */
        argc = 1;
        if (get_ext()->comparator_obj_->get_prop(
            vmg_ G_predef->calc_hash_prop, &result,
            get_ext()->comparator_, &srcobj, &argc))
        {
            /* got it - make sure it's an integer */
            if (result.typ != VM_INT)
                err_throw(VMERR_BAD_TYPE_BIF);
            
            /* return the result */
            return (unsigned int)result.val.intval;
        }
        else
        {
            /* didn't even find the method - it's an error */
            err_throw(VMERR_BAD_TYPE_BIF);
            return 0;
        }
    }

    /* we should never get here, but just in case... */
    return 0;
}

/* ------------------------------------------------------------------------ */
/*
 *   Set a property.  We have no settable properties, so simply signal an
 *   error indicating that the set-prop call is invalid.  
 */
void CVmObjDict::set_prop(VMG_ CVmUndo *, vm_obj_id_t,
                          vm_prop_id_t, const vm_val_t *)
{
    /* no settable properties - throw an error */
    err_throw(VMERR_INVALID_SETPROP);
}

/* ------------------------------------------------------------------------ */
/* 
 *   get a property 
 */
int CVmObjDict::get_prop(VMG_ vm_prop_id_t prop, vm_val_t *retval,
                         vm_obj_id_t self, vm_obj_id_t *source_obj,
                         uint *argc)
{
    ushort func_idx;
    
    /* translate the property into a function vector index */
    func_idx = G_meta_table
               ->prop_to_vector_idx(metaclass_reg_->get_reg_idx(), prop);

    /* call the appropriate function */
    if ((this->*func_table_[func_idx])(vmg_ self, retval, argc))
    {
        *source_obj = metaclass_reg_->get_class_obj(vmg0_);
        return TRUE;
    }

    /* inherit default handling from the base object class */
    return CVmObject::get_prop(vmg_ prop, retval, self, source_obj, argc);
}

/* ------------------------------------------------------------------------ */
/*
 *   evaluate property: set the comparator 
 */
int CVmObjDict::getp_set_comparator(VMG_ vm_obj_id_t self, vm_val_t *val,
                                    uint *argc)
{
    static CVmNativeCodeDesc desc(1);
    vm_obj_id_t comp;
    
    /* check arguments */
    if (get_prop_check_argc(val, argc, &desc))
        return TRUE;

    /* get the comparator object */
    switch(G_stk->get(0)->typ)
    {
    case VM_NIL:
        comp = VM_INVALID_OBJ;
        break;

    case VM_OBJ:
        comp = G_stk->get(0)->val.obj;
        break;

    default:
        err_throw(VMERR_BAD_TYPE_BIF);
    }

    /* if there's a global undo object, add undo for the change */
    if (G_undo != 0)
    {
        dict_undo_rec *undo_rec;

        /* create an undo record with the original comparator */
        undo_rec = alloc_undo_rec(DICT_UNDO_COMPARATOR, 0, 0);
        undo_rec->obj = get_ext()->comparator_;

        /* add the undo record */
        add_undo_rec(vmg_ self, undo_rec);
    }

    /* set our new comparator object */
    set_comparator(vmg_ comp);

    /* mark the object as modified since load */
    get_ext()->modified_ = TRUE;

    /* discard arguments */
    G_stk->discard();

    /* no return value - just return nil */
    val->set_nil();

    /* handled */
    return TRUE;
}

/* ------------------------------------------------------------------------ */
/* 
 *   result entry for find_ctx 
 */
struct find_entry
{
    /* matching object */
    vm_obj_id_t obj;

    /* match result code */
    vm_val_t match_result;
};

/*
 *   page of results for find_ctx
 */
const int FIND_ENTRIES_PER_PAGE = 32;
struct find_entry_page
{
    /* next in list of pages */
    find_entry_page *nxt;

    /* array of entries */
    find_entry entry[FIND_ENTRIES_PER_PAGE];
};

/*
 *   enumeration callback context for getp_find
 */
struct find_ctx
{
    /* set up a context that adds results to our internal list */
    find_ctx(VMG_ CVmObjDict *dict_obj, const vm_val_t *strval,
             const char *strp, size_t strl, vm_prop_id_t prop)
    {
        /* remember the dictionary object we're searching */
        dict = dict_obj;

        /* remember the globals */
        globals = VMGLOB_ADDR;

        /* keep a pointer to the search string value */
        this->strval = strval;
        this->strp = strp;
        this->strl = strl;

        /* remember the property */
        voc_prop = prop;

        /* no results yet */
        results_head = 0;
        results_tail = 0;
        result_cnt = 0;

        /* 
         *   set the free pointer to the end of the non-existent current
         *   page, so we know we have to allocate a new page before adding
         *   the first entry 
         */
        result_free = FIND_ENTRIES_PER_PAGE;
    }

    ~find_ctx()
    {
        /* free our list of results pages */
        while (results_head != 0)
        {
            /* remember the current page */
            find_entry_page *cur = results_head;

            /* move on to the next page */
            results_head = results_head->nxt;

            /* delete the current page */
            t3free(cur);
        }
    }

    /* add a result, expanding the result array if necessary */
    void add_result(vm_obj_id_t obj, const vm_val_t *match_result)
    {
        /* 
         *   if we have no pages, or we're out of room on the current page,
         *   allocate a new page 
         */
        if (result_free == FIND_ENTRIES_PER_PAGE)
        {
            find_entry_page *pg;
            
            /* allocate a new page */
            pg = (find_entry_page *)t3malloc(sizeof(find_entry_page));
            
            /* link it in at the end of the list */
            pg->nxt = 0;
            if (results_tail != 0)
                results_tail->nxt = pg;
            else
                results_head = pg;
            results_tail = pg;

            /* reset the free pointer to the start of the new page */
            result_free = 0;
        }

        /* add this result */
        results_tail->entry[result_free].obj = obj;
        results_tail->entry[result_free].match_result = *match_result;
        ++result_free;
        ++result_cnt;
    }

    /* make our results into a list */
    vm_obj_id_t results_to_list(VMG0_)
    {
        vm_obj_id_t lst_id;
        CVmObjList *lst;
        find_entry_page *pg;
        int idx;
        
        /* 
         *   Allocate a list of the appropriate size.  We need two list
         *   entries per result, since we need to store the object and the
         *   match result code for each result.  
         */
        lst_id = CVmObjList::create(vmg_ FALSE, result_cnt * 2);
        lst = (CVmObjList *)vm_objp(vmg_ lst_id);

        /* add all entries */
        for (idx = 0, pg = results_head ; idx < result_cnt ; pg = pg->nxt)
        {
            find_entry *ep;
            int pg_idx;
            
            /* add each entry on this page */
            for (ep = pg->entry, pg_idx = 0 ;
                 idx < result_cnt && pg_idx < FIND_ENTRIES_PER_PAGE ;
                 ++idx, ++pg_idx, ++ep)
            {
                vm_val_t ele;
                    
                /* add this entry to the list */
                ele.set_obj(ep->obj);
                lst->cons_set_element(idx*2, &ele);
                lst->cons_set_element(idx*2 + 1, &ep->match_result);
            }
        }

        /* return the list */
        return lst_id;
    }

    /* head/tail of list of result pages */
    find_entry_page *results_head;
    find_entry_page *results_tail;

    /* next available entry on current results page */
    int result_free;

    /* total number of result */
    int result_cnt;

    /* the string to find */
    const vm_val_t *strval;
    const char *strp;
    size_t strl;

    /* vocabulary property value to match */
    vm_prop_id_t voc_prop;

    /* VM globals */
    vm_globals *globals;

    /* dictionary object we're searching */
    CVmObjDict *dict;
};

/*
 *   Callback for getp_find hash match enumeration
 */
void CVmObjDict::find_cb(void *ctx0, CVmHashEntry *entry0)
{
    find_ctx *ctx = (find_ctx *)ctx0;
    CVmHashEntryDict *entry = (CVmHashEntryDict *)entry0;
    vm_val_t match_val;
    VMGLOB_PTR(ctx->globals);

    /* if this entry matches the search string, process it */
    if (ctx->dict->match_strings(vmg_ ctx->strval, ctx->strp, ctx->strl,
                                 entry->getstr(), entry->getlen(),
                                 &match_val))
    {
        vm_dict_entry *cur;
        
        /* process the items under this entry */
        for (cur = entry->get_head() ; cur != 0 ; cur = cur->nxt_)
        {
            /* 
             *   If it matches the target property, it's a hit.  If we're
             *   searching for any property at all (indicated by the
             *   context property being set to VM_INVALID_PROP), count
             *   everything.
             */
            if (cur->prop_ == ctx->voc_prop
                || ctx->voc_prop == VM_INVALID_PROP)
            {
                /* process the result */
                ctx->add_result(cur->obj_, &match_val);
            }
        }
    }
}

/* ------------------------------------------------------------------------ */
/*
 *   property evaluation - look up a word
 */
int CVmObjDict::getp_find(VMG_ vm_obj_id_t, vm_val_t *val, uint *argc)
{
    static CVmNativeCodeDesc desc(1, 1);
    uint orig_argc = (argc != 0 ? *argc : 0);
    vm_val_t *arg0;
    vm_val_t *arg1;
    const char *strp;
    size_t strl;
    vm_prop_id_t voc_prop;
    unsigned int hash;

    /* check arguments */
    if (get_prop_check_argc(val, argc, &desc))
        return TRUE;

    /* 
     *   make sure the first argument is a string, but leave it on the stack
     *   for gc protection 
     */
    arg0 = G_stk->get(0);
    if ((strp = arg0->get_as_string(vmg0_)) == 0)
        err_throw(VMERR_STRING_VAL_REQD);

    /* scan and skip the string's length prefix */
    strl = vmb_get_len(strp);
    strp += VMB_LEN;

    /* 
     *   get the property ID, which can be omitted or specified as nil to
     *   indicate any property 
     */
    if (orig_argc < 2)
        voc_prop = VM_INVALID_PROP;
    else if ((arg1 = G_stk->get(1))->typ == VM_NIL)
        voc_prop = VM_INVALID_PROP;
    else if (arg1->typ == VM_PROP)
        voc_prop = arg1->val.prop;
    else
        err_throw(VMERR_PROPPTR_VAL_REQD);

    /* calculate the hash value */
    hash = calc_hash(vmg_ arg0, strp, strl);

    /* enumerate everything that matches the string's hash code */
    find_ctx ctx(vmg_ this, arg0, strp, strl, voc_prop);
    get_ext()->hashtab_->enum_hash_matches(hash, &find_cb, &ctx);

    /* build a list out of the results, and return the list */
    val->set_obj(ctx.results_to_list(vmg0_));

    /* discard arguments */
    G_stk->discard(orig_argc);

    /* success */
    return TRUE;
}

/* ------------------------------------------------------------------------ */
/*
 *   property evaluation - add a word
 */
int CVmObjDict::getp_add(VMG_ vm_obj_id_t self, vm_val_t *val, uint *argc)
{
    vm_obj_id_t obj;
    vm_val_t str_val;
    vm_prop_id_t voc_prop;
    CVmObject *objp;
    const char *lst;
    const char *str;
    static CVmNativeCodeDesc desc(3);

    /* check arguments */
    if (get_prop_check_argc(val, argc, &desc))
        return TRUE;

    /* pop the object, word string, and property ID arguments */
    obj = CVmBif::pop_obj_val(vmg0_);
    G_stk->pop(&str_val);
    voc_prop = CVmBif::pop_propid_val(vmg0_);

    /* ensure that the object value isn't a string or list */
    objp = vm_objp(vmg_ obj);
    if (objp->get_as_list() != 0 || objp->get_as_string(vmg0_) != 0)
        err_throw(VMERR_OBJ_VAL_REQD);

    /* if the string argument is a list, add each word from the list */
    if ((lst = str_val.get_as_list(vmg0_)) != 0)
    {
        size_t cnt;
        size_t idx;
        vm_val_t ele_val;

        /* get and skip the list count prefix */
        cnt = vmb_get_len(lst);

        /* iterate over the list, using 1-based indices */
        for (idx = 1 ; idx <= cnt ; ++idx)
        {
            /* get the next value from the list */
            CVmObjList::index_list(vmg_ &ele_val, lst, idx);

            /* get the next string value */
            if ((str = ele_val.get_as_string(vmg0_)) == 0)
                err_throw(VMERR_STRING_VAL_REQD);

            /* set this string */
            getp_add_string(vmg_ self, str, obj, voc_prop);

            /* 
             *   refresh the list pointer, in case it was const pool data
             *   and got swapped out by resolving the string pointer 
             */
            lst = str_val.get_as_list(vmg0_);
        }
    }
    else if ((str = str_val.get_as_string(vmg0_)) != 0)
    {
        /* add the string */
        getp_add_string(vmg_ self, str, obj, voc_prop);
    }
    else
    {
        /* string value required */
        err_throw(VMERR_STRING_VAL_REQD);
    }

    /* there's no return value */
    val->set_nil();

    /* success */
    return TRUE;
}

/*
 *   Service routine for getp_add() - add a single string 
 */
void CVmObjDict::getp_add_string(VMG_ vm_obj_id_t self,
                                 const char *str, vm_obj_id_t obj,
                                 vm_prop_id_t voc_prop)
{
    size_t len;
    int added;

    /* get the string's length prefix */
    len = vmb_get_len(str);
    str += 2;

    /* add the entry */
    added = add_hash_entry(vmg_ str, len, TRUE, obj, voc_prop, FALSE);

    /* 
     *   if there's a global undo object, and we actually added a new
     *   entry, add undo for the change 
     */
    if (added && G_undo != 0)
    {
        dict_undo_rec *undo_rec;

        /* create the undo record */
        undo_rec = alloc_undo_rec(DICT_UNDO_ADD, str, len);
        undo_rec->obj = obj;
        undo_rec->prop = voc_prop;

        /* add the undo record */
        add_undo_rec(vmg_ self, undo_rec);
    }

    /* mark the object as modified since load */
    get_ext()->modified_ = TRUE;
}

/* ------------------------------------------------------------------------ */
/*
 *   property evaluation - delete a word 
 */
int CVmObjDict::getp_del(VMG_ vm_obj_id_t self, vm_val_t *val, uint *argc)
{
    vm_obj_id_t obj;
    vm_val_t str_val;
    vm_prop_id_t voc_prop;
    const char *lst;
    const char *str;
    static CVmNativeCodeDesc desc(3);

    /* check arguments */
    if (get_prop_check_argc(val, argc, &desc))
        return TRUE;

    /* pop the object, word string, and property ID arguments */
    obj = CVmBif::pop_obj_val(vmg0_);
    G_stk->pop(&str_val);
    voc_prop = CVmBif::pop_propid_val(vmg0_);

    /* if the string argument is a list, add each word from the list */
    if ((lst = str_val.get_as_list(vmg0_)) != 0)
    {
        size_t cnt;
        size_t idx;
        vm_val_t ele_val;

        /* get and skip the list count prefix */
        cnt = vmb_get_len(lst);

        /* iterate over the list, using 1-based indices */
        for (idx = 1 ; idx <= cnt ; ++idx)
        {
            /* get the next value from the list */
            CVmObjList::index_list(vmg_ &ele_val, lst, idx);

            /* get the next string value */
            if ((str = ele_val.get_as_string(vmg0_)) == 0)
                err_throw(VMERR_STRING_VAL_REQD);

            /* remove this string */
            getp_del_string(vmg_ self, str, obj, voc_prop);

            /* 
             *   refresh the list pointer, in case it was const pool data
             *   and got swapped out by resolving the string pointer 
             */
            lst = str_val.get_as_list(vmg0_);
        }
    }
    else if ((str = str_val.get_as_string(vmg0_)) != 0)
    {
        /* remove the string */
        getp_del_string(vmg_ self, str, obj, voc_prop);
    }
    else
    {
        /* string value required */
        err_throw(VMERR_STRING_VAL_REQD);
    }

    /* there's no return value */
    val->set_nil();

    /* success */
    return TRUE;
}

/*
 *   Service routine for getp_del - delete a string value 
 */
void CVmObjDict::getp_del_string(VMG_ vm_obj_id_t self,
                                 const char *str, vm_obj_id_t obj,
                                 vm_prop_id_t voc_prop)
{
    size_t len;
    int deleted;

    /* get the string's length prefix */
    len = vmb_get_len(str);
    str += 2;

    /* delete this entry */
    deleted = del_hash_entry(vmg_ str, len, obj, voc_prop);

    /* 
     *   if there's a global undo object, and we actually deleted an
     *   entry, add undo for the change 
     */
    if (deleted && G_undo != 0)
    {
        dict_undo_rec *undo_rec;

        /* create the undo record */
        undo_rec = alloc_undo_rec(DICT_UNDO_DEL, str, len);
        undo_rec->obj = obj;
        undo_rec->prop = voc_prop;

        /* add the undo record */
        add_undo_rec(vmg_ self, undo_rec);
    }

    /* mark the object as modified since load */
    get_ext()->modified_ = TRUE;
}

/* ------------------------------------------------------------------------ */
/*
 *   callback context for is_defined enumeration 
 */
struct isdef_ctx
{
    isdef_ctx(VMG_ CVmObjDict *dict_obj, vm_val_t *filter,
              const vm_val_t *strval, const char *strp, size_t strl)
    {
        /* remember the VM globals and dictionary object */
        globals = VMGLOB_ADDR;
        dict = dict_obj;
        
        /* we haven't yet found a match */
        found = FALSE;

        /* remember the string */
        this->strval = *strval;
        this->strp = strp;
        this->strl = strl;

        /* remember the filter function */
        filter_func = filter;
    }
    
    /* flag: we've found a match */
    int found;

    /* string value we're matching */
    vm_val_t strval;
    const char *strp;
    size_t strl;

    /* our filter callback function, if any */
    const vm_val_t *filter_func;

    /* VM globals */
    vm_globals *globals;

    /* dictionary we're searching */
    CVmObjDict *dict;
};

/*
 *   Callback for getp_is_defined hash match enumeration 
 */
void CVmObjDict::isdef_cb(void *ctx0, CVmHashEntry *entry0)
{
    isdef_ctx *ctx = (isdef_ctx *)ctx0;
    CVmHashEntryDict *entry = (CVmHashEntryDict *)entry0;
    vm_val_t match_val;
    VMGLOB_PTR(ctx->globals);

    /* 
     *   if we've already found a match, don't bother checking anything else
     *   - it could be non-trivial to do the string match, so we can save
     *   some work by skipping that if we've already found a match
     *   previously 
     */
    if (ctx->found)
        return;

    /* 
     *   if this entry matches the search string, and it has at least one
     *   defined object/property associated with it, count it as a match 
     */
    if (entry->get_head() != 0
        && ctx->dict->match_strings(vmg_ &ctx->strval, ctx->strp, ctx->strl,
                                    entry->getstr(), entry->getlen(),
                                    &match_val))
    {
        /* if there's a filter function to invoke, check with it */
        if (ctx->filter_func->typ != VM_NIL)
        {
            vm_val_t *valp;
            
            /* push the match value parameter */
            G_stk->push(&match_val);

            /* call the filter callback */
            G_interpreter->call_func_ptr(vmg_ ctx->filter_func, 1,
                                         "Dictionary.isDefined", 0);

            /* get the result */
            valp = G_interpreter->get_r0();

            /* if it's 0 or nil, it's a rejection of the match */
            if (valp->typ == VM_NIL
                || (valp->typ == VM_INT && valp->val.intval == 0))
            {
                /* it's a rejection of the value - don't count it */
            }
            else
            {
                /* it's an acceptance of the value - count it as found */
                ctx->found = TRUE;
            }
        }
        else
        {
            /* there's no filter, so everything matches */
            ctx->found = TRUE;
        }
    }
}

/*
 *   property evaluation - check to see if a word is defined 
 */
int CVmObjDict::getp_is_defined(VMG_ vm_obj_id_t self, vm_val_t *val,
                                uint *argc)
{
    static CVmNativeCodeDesc desc(1, 1);
    uint orig_argc = (argc == 0 ? 0 : *argc);
    vm_val_t *arg0;
    const char *strp;
    size_t strl;
    vm_val_t filter;
    unsigned int hash;
    
    /* check arguments */
    if (get_prop_check_argc(val, argc, &desc))
        return TRUE;

    /* 
     *   make sure the argument is a string, but leave it on the stack for
     *   gc protection 
     */
    arg0 = G_stk->get(0);
    if ((strp = arg0->get_as_string(vmg0_)) == 0)
        err_throw(VMERR_STRING_VAL_REQD);

    /* read and skip the string's length prefix */
    strl = vmb_get_len(strp);
    strp += VMB_LEN;

    /* if there's a second argument, it's the filter function */
    if (orig_argc >= 2)
    {
        /* retrieve the filter */
        filter = *G_stk->get(1);
    }
    else
    {
        /* there's no filter */
        filter.set_nil();
    }

    /* calculate the hash code */
    hash = calc_hash(vmg_ arg0, strp, strl);

    /* enumerate matches for the string */
    isdef_ctx ctx(vmg_ this, &filter, arg0, strp, strl);
    get_ext()->hashtab_->enum_hash_matches(hash, &isdef_cb, &ctx);

    /* if we found any matches, return true; otherwise, return nil */
    val->set_logical(ctx.found);

    /* discard arguments */
    G_stk->discard(orig_argc);

    /* success */
    return TRUE;
}

/* ------------------------------------------------------------------------ */
/*
 *   callback context for forEachWord enumerator 
 */
struct for_each_word_enum_cb_ctx
{
    /* callback function value */
    vm_val_t *cb_val;

    /* globals */
    vm_globals *vmg;
};

/*
 *   callback for forEachWord enumerator 
 */
static void for_each_word_enum_cb(void *ctx0, CVmHashEntry *entry0)
{
    for_each_word_enum_cb_ctx *ctx = (for_each_word_enum_cb_ctx *)ctx0;
    CVmHashEntryDict *entry = (CVmHashEntryDict *)entry0;
    size_t list_idx;
    vm_obj_id_t str_obj;

    /* set up access to the VM globals through my stashed context */
    VMGLOB_PTR(ctx->vmg);

    /* create a string object for this entry's word string */
    str_obj = CVmObjString::create(vmg_ FALSE,
                                   entry->getstr(), entry->getlen());

    /* scan the entry's list of word associations */
    for (list_idx = 0 ; ; ++list_idx)
    {
        vm_dict_entry *p;
        size_t i;

        /*
         *   Find the list entry at the current list index.  Note that we
         *   scan the list on each iteration for the current index because
         *   we don't want to keep track of any pointers between iterations;
         *   this insulates us from any changes to the underlying list made
         *   in the callback.  
         */
        for (i = 0, p = entry->get_head() ; i < list_idx && p != 0 ;
             p = p->nxt_, ++i) ;

        /* 
         *   if we didn't find a list entry at this index, we're done with
         *   this hashtable entry 
         */
        if (p == 0)
            break;

        /* 
         *   Invoke the callback on this list entry.  The arguments to the
         *   callback are the object, the string, and the vocabulary
         *   property of the association:
         *   
         *   func(obj, str, prop)
         *   
         *   Note that the order of the arguments is the same as for
         *   addWord() and removeWord().  
         */

        /* push the vocabulary property of the association */
        G_stk->push()->set_propid(p->prop_);

        /* push the string of the entry */
        G_stk->push()->set_obj(str_obj);

        /* push the object of the association */
        G_stk->push()->set_obj(p->obj_);

        /* invoke the callback */
        G_interpreter->call_func_ptr(vmg_ ctx->cb_val, 3,
                                     "Dictionary.forEachWord", 0);
    }
}

/*
 *   property evaluation - enumerate each word association 
 */
int CVmObjDict::getp_for_each_word(VMG_ vm_obj_id_t self, vm_val_t *retval,
                                   uint *argc)
{
    static CVmNativeCodeDesc desc(1);
    vm_val_t *cb_val;
    for_each_word_enum_cb_ctx ctx;

    /* check arguments */
    if (get_prop_check_argc(retval, argc, &desc))
        return TRUE;

    /* get a pointer to the callback value, but leave it on the stack */
    cb_val = G_stk->get(0);

    /* push a self-reference for gc protection while we're working */
    G_stk->push()->set_obj(self);

    /* 
     *   enumerate the entries in our underlying hashtable - use the "safe"
     *   enumerator to ensure that we're insulated from any changes to the
     *   hashtable that the callback wants to make 
     */
    ctx.vmg = VMGLOB_ADDR;
    ctx.cb_val = cb_val;
    get_ext()->hashtab_->safe_enum_entries(for_each_word_enum_cb, &ctx);

    /* discard arguments and gc protection */
    G_stk->discard(2);

    /* there's no return value */
    retval->set_nil();

    /* handled */
    return TRUE;
}

/* ------------------------------------------------------------------------ */
/*
 *   Create an undo record 
 */
dict_undo_rec *CVmObjDict::alloc_undo_rec(dict_undo_action action,
                                          const char *txt, size_t len)
{
    size_t alloc_size;
    dict_undo_rec *rec;
    
    /* 
     *   compute the allocation size - start with the structure size, and
     *   add in the length we need to store the string if one is present 
     */
    alloc_size = sizeof(dict_undo_rec) - 1;
    if (txt != 0)
        alloc_size += len;

    /* allocate the record */
    rec = (dict_undo_rec *)t3malloc(alloc_size);

    /* set the action */
    rec->action = action;

    /* if there's a word, copy the text */
    if (txt != 0)
        memcpy(rec->txt, txt, len);

    /* save the word length */
    rec->len = len;

    /* presume we won't store an object or property */
    rec->obj = VM_INVALID_OBJ;
    rec->prop = VM_INVALID_PROP;

    /* return the new record */
    return rec;
}

/*
 *   add an undo record 
 */
void CVmObjDict::add_undo_rec(VMG_ vm_obj_id_t self, dict_undo_rec *rec)
{
    vm_val_t val;

    /* add the record with an empty value */
    val.set_empty();
    if (!G_undo->add_new_record_ptr_key(vmg_ self, rec, &val))
    {
        /* 
         *   we didn't add an undo record, so our extra undo information
         *   isn't actually going to be stored in the undo system - hence
         *   we must delete our extra information 
         */
        t3free(rec);
    }
}

/* 
 *   apply undo 
 */
void CVmObjDict::apply_undo(VMG_ CVmUndoRecord *undo_rec)
{
    if (undo_rec->id.ptrval != 0)
    {
        dict_undo_rec *rec;

        /* get my undo record */
        rec = (dict_undo_rec *)undo_rec->id.ptrval;

        /* take the appropriate action */
        switch(rec->action)
        {
        case DICT_UNDO_ADD:
            /* we added the word, so we must now delete it */
            del_hash_entry(vmg_ rec->txt, rec->len, rec->obj, rec->prop);
            break;

        case DICT_UNDO_DEL:
            /* we deleted the word, so now we must add it */
            add_hash_entry(vmg_ rec->txt, rec->len, TRUE,
                           rec->obj, rec->prop, FALSE);
            break;

        case DICT_UNDO_COMPARATOR:
            /* we changed the comparator object, so change it back */
            set_comparator(vmg_ rec->obj);
            break;
        }

        /* discard my record */
        t3free(rec);

        /* clear the pointer in the undo record so we know it's gone */
        undo_rec->id.ptrval = 0;
    }
}

/*
 *   discard extra undo information 
 */
void CVmObjDict::discard_undo(VMG_ CVmUndoRecord *rec)
{
    /* delete our extra information record */
    if (rec->id.ptrval != 0)
    {
        /* free the record */
        t3free((dict_undo_rec *)rec->id.ptrval);

        /* clear the pointer so we know it's gone */
        rec->id.ptrval = 0;
    }
}

/*
 *   mark references in an undo record 
 */
void CVmObjDict::mark_undo_ref(VMG_ CVmUndoRecord *undo_rec)
{
    /* check our private record if we have one */
    if (undo_rec->id.ptrval != 0)
    {
        dict_undo_rec *rec;

        /* get my undo record */
        rec = (dict_undo_rec *)undo_rec->id.ptrval;

        /* take the appropriate action */
        switch(rec->action)
        {
        case DICT_UNDO_COMPARATOR:
            /* the 'obj' entry is the old comparator object */
            G_obj_table->mark_all_refs(rec->obj, VMOBJ_REACHABLE);
            break;

        case DICT_UNDO_ADD:
        case DICT_UNDO_DEL:
            /* 
             *   these actions use only weak references, so there's nothing
             *   to do here 
             */
            break;
        }
    }
}

/* 
 *   remove stale weak references from an undo record
 */
void CVmObjDict::remove_stale_undo_weak_ref(VMG_ CVmUndoRecord *undo_rec)
{
    /* check our private record if we have one */
    if (undo_rec->id.ptrval != 0)
    {
        dict_undo_rec *rec;

        /* get my undo record */
        rec = (dict_undo_rec *)undo_rec->id.ptrval;

        /* take the appropriate action */
        switch(rec->action)
        {
        case DICT_UNDO_ADD:
        case DICT_UNDO_DEL:
            /* check to see if our object is being deleted */
            if (rec->obj != VM_INVALID_OBJ
                && G_obj_table->is_obj_deletable(rec->obj))
            {
                /* 
                 *   the object is being deleted - since we only weakly
                 *   reference our objects, we must forget about this
                 *   object now, which means we can forget this entire
                 *   record 
                 */
                t3free(rec);
                undo_rec->id.ptrval = 0;
            }
            break;

        case DICT_UNDO_COMPARATOR:
            /* this action does not use weak references */
            break;
        }
    }
}

/* ------------------------------------------------------------------------ */
/*
 *   callback context for weak reference function 
 */
struct enum_hashtab_ctx
{
    /* global context pointer */
    vm_globals *vmg;

    /* dictionary object */
    CVmObjDict *dict;
};

/*
 *   Mark references 
 */
void CVmObjDict::mark_refs(VMG_ uint state)
{
    /* if I have a comparator object, mark it as referenced */
    if (get_ext()->comparator_ != VM_INVALID_OBJ)
        G_obj_table->mark_all_refs(get_ext()->comparator_, state);
}

/* 
 *   Remove weak references that have become stale.  For each object for
 *   which we have a weak reference, check to see if the object is marked
 *   for deletion; if so, remove our reference to the object.  
 */
void CVmObjDict::remove_stale_weak_refs(VMG0_)
{
    enum_hashtab_ctx ctx;
    
    /* iterate over our hash table */
    ctx.vmg = VMGLOB_ADDR;
    ctx.dict = this;
    get_ext()->hashtab_->enum_entries(&remove_weak_ref_cb, &ctx);
}

/*
 *   enumeration callback to eliminate stale weak references 
 */
void CVmObjDict::remove_weak_ref_cb(void *ctx0, CVmHashEntry *entry0)
{
    enum_hashtab_ctx *ctx = (enum_hashtab_ctx *)ctx0;
    CVmHashEntryDict *entry = (CVmHashEntryDict *)entry0;
    vm_dict_entry *cur;
    vm_dict_entry *nxt;

    /* set up for global access through the context */
    VMGLOB_PTR(ctx->vmg);

    /* scan the entry's list */
    for (cur = entry->get_head() ; cur != 0 ; cur = nxt)
    {
        /* remember the next entry in case we delete this one */
        nxt = cur->nxt_;
        
        /* check to see if this object is free - if so, remove this record */
        if (G_obj_table->is_obj_deletable(cur->obj_))
        {
            /* delete the entry */
            entry->del_entry(ctx->dict->get_ext()->hashtab_,
                             cur->obj_, cur->prop_);
        }
    }
}


/* ------------------------------------------------------------------------ */
/* 
 *   load from an image file 
 */
void CVmObjDict::load_from_image(VMG_ vm_obj_id_t self,
                                 const char *ptr, size_t siz)
{
    /* remember where our image data comes from */
    get_ext()->image_data_ = ptr;
    get_ext()->image_data_size_ = siz;

    /* build the hash table from the image data */
    build_hash_from_image(vmg0_);

    /* 
     *   register for post-load initialization, as we might need to rebuild
     *   our hash table once we have access to the comparator object 
     */
    G_obj_table->request_post_load_init(self);
}

/*
 *   Post-load initialization 
 */
void CVmObjDict::post_load_init(VMG_ vm_obj_id_t self)
{
    vm_obj_id_t comp;
    
    /* 
     *   Rebuild the hash table with the comparator, if we have one.  When
     *   we load, reset, or restore, we build a tentative hash table with no
     *   comparator (i.e., the default exact literal comparator), because we
     *   can't assume we have access to the comparator object (as it might
     *   be loaded after us).  So, we register for post-load initialization,
     *   and then we go back and re-install the comparator for real,
     *   rebuilding the hash table with the actual comparator.  
     */
    if ((comp = get_ext()->comparator_) != VM_INVALID_OBJ)
    {
        /* ensure the comparator object is initialized */
        G_obj_table->ensure_post_load_init(vmg_ comp);

        /* set the comparator object type, now that it's loaded */
        set_comparator_type(vmg_ comp);

        /* 
         *   force a rebuild the hash table, so that we build it with the
         *   comparator properly installed 
         */
        create_hash_table(vmg0_);
    }
}

/*
 *   Build the hash table from the image data 
 */
void CVmObjDict::build_hash_from_image(VMG0_)
{
    uint cnt;
    uint i;
    const char *p;
    const char *endp;
    vm_obj_id_t comp;

    /* start reading at the start of the image data */
    p = get_ext()->image_data_;
    endp = p + get_ext()->image_data_size_;

    /* read the comparator object ID */
    comp = (vm_obj_id_t)osrp4(p);
    p += 4;

    /*
     *   Do NOT install the actual comparator object at this point, but
     *   simply build the table tentatively with a nil comparator.  We can't
     *   assume that the comparator object has been loaded yet (as it might
     *   be loaded after us), so we cannot use it to build the hash table.
     *   We have to do something with the data, though, so build the hash
     *   table tentatively with no comparator; we'll rebuild it again at
     *   post-load-init time with the real comparator.
     *   
     *   (This isn't as inefficient as it sounds, as we retain all of the
     *   original hash table entries when we rebuild the table.  All we end
     *   up doing is scanning the entries again to recompute their new hash
     *   values, and reallocating the hash table object itself.)  
     */
    get_ext()->comparator_ = VM_INVALID_OBJ;
    set_comparator_type(vmg_ VM_INVALID_OBJ);

    /* create the new hash table */
    create_hash_table(vmg0_);

    /* read the entry count */
    cnt = osrp2(p);
    p += 2;

    /* scan the entries */
    for (i = 0 ; p < endp && i < cnt ; ++i)
    {
        vm_obj_id_t obj;
        vm_prop_id_t prop;
        uint key_len;
        uint item_cnt;
        const char *key;
        CVmHashEntryDict *entry;
        char key_buf[256];
        char *keyp;
        size_t key_rem;

        /* read the key length */
        key_len = *(unsigned char *)p++;

        /* remember the key pointer */
        key = p;
        p += key_len;

        /* copy the key to our buffer for decoding */
        memcpy(key_buf, key, key_len);

        /* decode the key */
        for (keyp = key_buf, key_rem = key_len ; key_rem != 0 ;
             --key_rem, ++keyp)
            *keyp ^= 0xBD;

        /* read the key from the decoded buffer now */
        key = key_buf;

        /* read the item count */
        item_cnt = osrp2(p);
        p += 2;

        /* find an existing entry for this key */
        entry = (CVmHashEntryDict *)get_ext()->hashtab_->find(key, key_len);

        /* if there's no existing entry, add one */
        if (entry == 0)
        {
            /* create a new entry */
            entry = new CVmHashEntryDict(key, key_len, TRUE, TRUE);

            /* add it to the table */
            get_ext()->hashtab_->add(entry);
        }

        /* read the items */
        for ( ; item_cnt != 0 ; --item_cnt)
        {
            /* read the object ID and property ID */
            obj = (vm_obj_id_t)osrp4(p);
            prop = (vm_prop_id_t)osrp2(p + 4);

            /* 
             *   add the entry - this entry is from the image file, so
             *   mark it as such 
             */
            entry->add_entry(obj, prop, TRUE);

            /* move on to the next item */
            p += 6;
        }
    }

    /* 
     *   Now that we're done building the table, remember the comparator.  We
     *   can't set the type yet, because the object might not be loaded yet -
     *   defer this until post-load initialization time.  
     */
    get_ext()->comparator_ = comp;
}

/* ------------------------------------------------------------------------ */
/*
 *   Add an entry 
 */
int CVmObjDict::add_hash_entry(VMG_ const char *p, size_t len, int copy,
                               vm_obj_id_t obj, vm_prop_id_t prop,
                               int from_image)
{
    CVmHashEntryDict *entry;
    
    /* find an existing entry for this key */
    entry = (CVmHashEntryDict *)get_ext()->hashtab_->find(p, len);

    /* if we didn't find an entry, add one */
    if (entry == 0)
    {
        /* create an entry */
        entry = new CVmHashEntryDict(p, len, copy, from_image);

        /* add it to the table */
        get_ext()->hashtab_->add(entry);
    }

    /* add the obj/prop to the entry's item list */
    return entry->add_entry(obj, prop, from_image);
}

/* ------------------------------------------------------------------------ */
/*
 *   delete an entry 
 */
int CVmObjDict::del_hash_entry(VMG_ const char *str, size_t len,
                               vm_obj_id_t obj, vm_prop_id_t voc_prop)
{
    CVmHashEntryDict *entry;

    /* find the hash table entry for the word */
    entry = (CVmHashEntryDict *)get_ext()->hashtab_->find(str, len);

    /* if we found it, delete the obj/prop entry */
    if (entry != 0)
    {
        /* delete the entry */
        return entry->del_entry(get_ext()->hashtab_, obj, voc_prop);
    }

    /* we didn't find anything to delete */
    return FALSE;
}

/* ------------------------------------------------------------------------ */
/* 
 *   restore to image file state 
 */
void CVmObjDict::reset_to_image(VMG_ vm_obj_id_t self)
{
    /* delete the hash table */
    if (get_ext()->hashtab_ != 0)
    {
        /* delete it */
        delete get_ext()->hashtab_;

        /* forget it */
        get_ext()->hashtab_ = 0;
    }

    /* rebuild the hash table from the image file data */
    build_hash_from_image(vmg0_);

    /* 
     *   register for post-load initialization, as we might need to rebuild
     *   our hash table once we have access to the comparator object 
     */
    G_obj_table->request_post_load_init(self);
}

/* ------------------------------------------------------------------------ */
/* 
 *   determine if the object has been changed since it was loaded 
 */
int CVmObjDict::is_changed_since_load() const
{
    /* 
     *   return true if our 'modified' flag is true - if it is, we must have
     *   been changed since we were loaded 
     */
    return (get_ext()->modified_ != 0);
}

/* ------------------------------------------------------------------------ */
/*
 *   save-file enumeration context 
 */
struct save_file_ctx
{
    /* file object */
    CVmFile *fp;

    /* entry count */
    ulong cnt;

    /* globals */
    vm_globals *vmg;
};

/* 
 *   save to a file 
 */
void CVmObjDict::save_to_file(VMG_ CVmFile *fp)
{
    long cnt_pos;
    long end_pos;
    save_file_ctx ctx;

    /* write the current comparator object */
    fp->write_int4(get_ext()->comparator_);
    
    /* remember the starting seek position and write a placeholder count */
    cnt_pos = fp->get_pos();
    fp->write_int4(0);

    /* enumerate the entries to write out each one */
    ctx.fp = fp;
    ctx.cnt = 0;
    ctx.vmg = VMGLOB_ADDR;
    get_ext()->hashtab_->enum_entries(&save_file_cb, &ctx);

    /* remember our position for a moment */
    end_pos = fp->get_pos();

    /* go back and write out the symbol count prefix */
    fp->set_pos(cnt_pos);
    fp->write_int4(ctx.cnt);

    /* seek back to the end */
    fp->set_pos(end_pos);
}

/*
 *   save file enumeration callback 
 */
void CVmObjDict::save_file_cb(void *ctx0, CVmHashEntry *entry0)
{
    save_file_ctx *ctx = (save_file_ctx *)ctx0;
    CVmHashEntryDict *entry = (CVmHashEntryDict *)entry0;
    vm_dict_entry *cur;
    uint cnt;

    /* set up global access */
    VMGLOB_PTR(ctx->vmg);

    /* write out this entry's name */
    ctx->fp->write_int2(entry->getlen());
    ctx->fp->write_bytes(entry->getstr(), entry->getlen());

    /* count the items in this entry's list */
    for (cnt = 0, cur = entry->get_head() ; cur != 0 ; cur = cur->nxt_)
    {
        /* 
         *   if the referenced object is persistent, count this item;
         *   don't count items that refer to non-persistent items, because
         *   these objects will not be saved to the file and hence will
         *   not be present when the saved state is restored 
         */
        if (G_obj_table->is_obj_persistent(cur->obj_))
            ++cnt;
    }

    /* 
     *   if there are no saveable items for this entry, do not write out
     *   the entry at all 
     */
    if (cnt == 0)
        return;

    /* count this entry */
    ++ctx->cnt;

    /* write the item count */
    ctx->fp->write_int2(cnt);

    /* write out each item in this entry's list */
    for (cur = entry->get_head() ; cur != 0 ; cur = cur->nxt_)
    {
        /* if this item's object is persistent, write out the item */
        if (G_obj_table->is_obj_persistent(cur->obj_))
        {
            /* write out the object and property for this entry */
            ctx->fp->write_int4((long)cur->obj_);
            ctx->fp->write_int2((int)cur->prop_);
        }
    }
}

/* ------------------------------------------------------------------------ */
/* 
 *   restore from a file 
 */
void CVmObjDict::restore_from_file(VMG_ vm_obj_id_t self,
                                   CVmFile *fp, CVmObjFixup *fixups)
{
    ulong cnt;
    vm_obj_id_t comp;

    /* delete the old hash table if we have one */
    if (get_ext()->hashtab_ != 0)
    {
        delete get_ext()->hashtab_;
        get_ext()->hashtab_ = 0;
    }

    /* 
     *   Read the comparator and fix it up to the new object numbering
     *   scheme, but do not install it (as it might not be loaded yet).  
     */
    comp = fixups->get_new_id(vmg_ (vm_obj_id_t)fp->read_int4());

    /* create the new, empty hash table */
    create_hash_table(vmg0_);

    /* read the number of symbols */
    cnt = fp->read_int4();

    /* read the symbols */
    for ( ; cnt != 0 ; --cnt)
    {
        uint len;
        uint read_len;
        char buf[256];
        uint item_cnt;
        
        /* read the symbol length */
        len = fp->read_uint2();

        /* limit the reading to our buffer size */
        read_len = len;
        if (read_len > sizeof(buf))
            read_len = sizeof(buf);

        /* read the string */
        fp->read_bytes(buf, read_len);

        /* skip any extra data */
        if (len > read_len)
            fp->set_pos(fp->get_pos() + len - read_len);

        /* read the item count */
        item_cnt = fp->read_uint2();

        /* read the items */
        for ( ; item_cnt != 0 ; --item_cnt)
        {
            vm_obj_id_t obj;
            vm_prop_id_t prop;
            
            /* read the object and property ID's */
            obj = (vm_obj_id_t)fp->read_int4();
            prop = (vm_prop_id_t)fp->read_uint2();

            /* translate the object ID through the fixup table */
            obj = fixups->get_new_id(vmg_ obj);

            /* add the entry, if it refers to a valid object */
            if (obj != VM_INVALID_OBJ)
                add_hash_entry(vmg_ buf, len, TRUE, obj, prop, FALSE);
        }
    }

    /* 
     *   install the comparator (we can't set its type yet, though, because
     *   the object might not be loaded yet) 
     */
    get_ext()->comparator_ = comp;
    set_comparator_type(vmg_ VM_INVALID_OBJ);

    /*
     *   If we had to restore it, we'll have to save it if the current state
     *   is later saved, even if we don't modify it again.  The fact that
     *   this version was saved before means this version has to be saved
     *   from now on.  
     */
    get_ext()->modified_ = TRUE;

    /* 
     *   register for post-load initialization, so that we can rebuild the
     *   hash table with the actual comparator if necessary 
     */
    G_obj_table->request_post_load_init(self);
}

/* ------------------------------------------------------------------------ */
/*
 *   impedence-matcher callback context 
 */
struct enum_word_props_ctx
{
    /* client callback to invoke */
    void (*cb_func)(VMG_ void *ctx, vm_prop_id_t prop,
                    const vm_val_t *match_val);

    /* string value to match */
    const vm_val_t *strval;
    const char *strp;
    size_t strl;

    /* client callback context */
    void *cb_ctx;

    /* globals */
    vm_globals *globals;

    /* the dictionary object we're searching */
    CVmObjDict *dict;
};

/*
 *   enum_word_props hash table enumeration callback 
 */
void CVmObjDict::enum_word_props_cb(void *ctx0, CVmHashEntry *entry0)
{
    enum_word_props_ctx *ctx = (enum_word_props_ctx *)ctx0;
    CVmHashEntryDict *entry = (CVmHashEntryDict *)entry0;
    vm_val_t match_val;
    VMGLOB_PTR(ctx->globals);

    /* if this entry matches the search string, process it */
    if (ctx->dict->match_strings(vmg_ ctx->strval, ctx->strp, ctx->strl,
                                 entry->getstr(), entry->getlen(),
                                 &match_val))
    {
        vm_dict_entry *cur;
        
        /* process the items under this entry */
        for (cur = entry->get_head() ; cur != 0 ; cur = cur->nxt_)
        {
            /* invoke the callback */
            (*ctx->cb_func)(vmg_ ctx->cb_ctx, cur->prop_, &match_val);
        }
    }
}

/*
 *   Enumerate the properties for which a word is defined 
 */
void CVmObjDict::enum_word_props(VMG_
                                 void (*cb_func)(VMG_ void *, vm_prop_id_t,
                                                 const vm_val_t *),
                                 void *cb_ctx, const vm_val_t *strval,
                                 const char *strp, size_t strl)
{
    enum_word_props_ctx ctx;
    unsigned int hash;

    /* set up the enumeration callback context */
    ctx.strval = strval;
    ctx.strp = strp;
    ctx.strl = strl;
    ctx.cb_func = cb_func;
    ctx.cb_ctx = cb_ctx;
    ctx.globals = VMGLOB_ADDR;
    ctx.dict = this;

    /* calculate the hash code */
    hash = calc_hash(vmg_ strval, strp, strl);

    /* enumerate the matches */
    get_ext()->hashtab_->enum_hash_matches(hash, &enum_word_props_cb, &ctx);
}

