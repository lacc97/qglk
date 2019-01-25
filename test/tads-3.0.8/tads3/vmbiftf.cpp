#error This file is no longer used.

#ifdef RCSID
static char RCSid[] =
"$Header$";
#endif

/* 
 *   Copyright (c) 1999, 2002 Michael J. Roberts.  All Rights Reserved.
 *   
 *   Please see the accompanying license file, LICENSE.TXT, for information
 *   on using and copying this software.  
 */
/*
Name
  vmbiftf.cpp - T3 TADS intrinsic function set - file operations
Function
  These functions implement the file operations in the TADS intrinsic
  function set (fopen, fread, fwrite, etc).  These are separated into
  this source file merely to keep file sizes manageable; the file
  manipulation intrinsics are pretty hefty all by themselves.
Notes
  
Modified
  07/28/99 MJRoberts  - Creation
*/

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "t3std.h"
#include "os.h"
#include "utf8.h"
#include "charmap.h"
#include "vmbiftad.h"
#include "vmstack.h"
#include "vmerr.h"
#include "vmerrnum.h"
#include "vmglob.h"
#include "vmpool.h"
#include "vmobj.h"
#include "vmstr.h"
#include "vmlst.h"
#include "vmrun.h"
#include "vmregex.h"
#include "vmhost.h"
#include "vmcset.h"
#include "vmbytarr.h"


/* ------------------------------------------------------------------------ */
/* 
 *   Service routine for file routines - get and validate a file number.
 *   Pops a file number off the stack, validates that the file number is
 *   within range and that a file is open on the handle, and returns a
 *   pointer to the file information structure.  
 */
static CVmBifTadsFile *pop_file_handle(VMG0_)
{
    vm_val_t val;
    long fnum;

    /* pop the file number */
    G_interpreter->pop_int(vmg_ &val);
    fnum = val.val.intval;

    /* make sure it's a valid file number */
    if (fnum < 0 || fnum >= VMBIFTADS_FILES_MAX
        || G_bif_tads_globals->fp[fnum].fp == 0)
        err_throw(VMERR_BAD_FILE_HANDLE);

    /* return the file array pointer */
    return &G_bif_tads_globals->fp[fnum];
}

/* ------------------------------------------------------------------------ */
/*
 *   file object implementation 
 */

/*
 *   close the file and release its resources 
 */
void CVmBifTadsFile::close()
{
    /* if we have a system file handle, close it */
    if (fp != 0)
        osfcls(fp);

    /* if we have a read map, release our reference on it */
    if (charmap_read != 0)
        charmap_read->release_ref();

    /* if we have a write map, release our reference on it */
    if (charmap_write != 0)
        charmap_write->release_ref();

    /* delete our read buffer, if we allocated one */
    if (read_buf != 0)
        t3free(read_buf);

    /* clear all of the resource pointers */
    fp = 0;
    charmap_read = 0;
    charmap_write = 0;
}


/* ------------------------------------------------------------------------ */
/*
 *   file open - fopen(file, mode, charmap?)
 *   
 *   The mode can be one of these:
 *   
 *.  r - read; the file must already exist
 *.  w - write; truncates any existing file or creates a new file
 *.  r+ - read/write; creates the file if it doesn't exist; no truncation
 *.  w+ - read/write; creates a new file or truncates an existing file 
 *   
 *   In addition, a mode suffix can be added:
 *   
 *.  t - text mode: writes everything as strings, reads lines of text
 *.  b - binary mode: reads and writes in TADS-specific tagged data format
 *.  r - raw mode: reads and writes bytes via ByteArray values
 *   
 *   If the charmap is provided, it's a text string giving the character
 *   mapping name to use.  The character mapping is meaningful only in text
 *   mode; it is not used in the binary or raw modes.  The following special
 *   mappings are available:
 *   
 *.  utf8 - Unicode UTF-8 encoding
 *.  unicodel - 16-bit Unicode little-endian
 *.  unicodeb - 16-bit Unicode bit-endian
 *.  us-ascii - plain ASCII
 *.  iso-8859-1 - ISO 8859-1 (Latin-1)
 *   
 *   Other character mappings refer to external .TCM files giving a mapping
 *   to a local character set.  For example, CP437 loads the code page 437
 *   (IBM PC US code page) mapping file CP437.TCM.  
 */
/*   Operations are allowed only if they conform to the current I/O safety
 *   level.  The safety level can be set by the user on the command line
 *   when running the game, and some implementations may allow the setting
 *   to be saved as a preference.  The possible levels are:
 *   
 *.  0 - minimum safety - read and write in any directory
 *.  1 - read in any directory, write in current directory
 *.  2 - read-only access in any directory
 *.  3 - read-only access in current directory only
 *.  4 - maximum safety - no file I/O allowed
 *    
 *   When operations are allowed only in the current directory, the
 *   operations will fail if the filename contains any sort of path
 *   specifier (for example, on Unix, any file that contains a '/' is
 *   considered to have a path specifier, and will always fail if
 *   operations are only allowed in the current directory).  
 */
void CVmBifTADS::file_open(VMG_ uint argc)
{
    char fname[OSFNMAX];
    const char *mode;
    char cmap[OSFNMAX];
    CVmObjCharSet *charset_obj;
    size_t mode_len;
    int fnum;
    char main_mode;
    int bin_mode;
    int raw_mode;
    int rw_mode;
    int in_same_dir;
    osfildef *fp;
    CVmBifTadsFile *finfo;
    int safety;

    /* check arguments */
    check_argc_range(vmg_ argc, 2, 3);

    /* presume binary mode */
    bin_mode = TRUE;
    raw_mode = FALSE;

    /* presume we're only going to read or write, not both */
    rw_mode = FALSE;

    /* copy the string into our buffer, so we can null-terminate it */
    pop_str_val_fname(vmg_ fname, sizeof(fname));

    /* get the mode string */
    mode = pop_str_val(vmg0_);
    mode_len = vmb_get_len(mode);
    mode += VMB_LEN;
    if (mode_len < 1)
        goto bad_mode;

    /* there's no character mapping yet */
    cmap[0] = '\0';
    charset_obj = 0;

    /* if there's a character map name specified, pop it */
    if (argc > 2)
    {
        /* 
         *   if we have a CharacterSet object, share its mappings;
         *   otherwise, the argument must be a string naming a mapping 
         */
        if (G_stk->get(0)->typ == VM_OBJ
            && CVmObjCharSet::is_charset(vmg_ G_stk->get(0)->val.obj))
        {
            /* 
             *   we know it's a CharacterSet object, so cast it; leave the
             *   argument on the stack for now, for gc protection 
             */
            charset_obj = (CVmObjCharSet *)
                          vm_objp(vmg_ G_stk->get(0)->val.obj);
        }
        else
        {
            /* it's not a CharacterSet, so it must be a name string */
            pop_str_val_buf(vmg_ cmap, sizeof(cmap));
        }
    }

    /* allocate a file number for the file */
    for (fnum = 0, finfo = G_bif_tads_globals->fp ;
         fnum < VMBIFTADS_FILES_MAX ; ++fnum, ++finfo)
    {
        /* if this descriptor hasn't been used, take it */
        if (finfo->fp == 0)
            break;
    }

    /* if we failed to allocate a file, return nil to indicate failure */
    if (fnum == VMBIFTADS_FILES_MAX)
    {
        /* return nil to indicate failure */
        retval_nil(vmg0_);
        goto done;
    }

    /* presume we won't need a read buffer */
    finfo->read_buf = 0;

    /* parse the main mode flag */
    switch(*mode)
    {
    case 'w':
    case 'W':
        main_mode = 'w';
        break;

    case 'r':
    case 'R':
        main_mode = 'r';
        break;

    default:
        goto bad_mode;
    }

    /* skip the main mode, and check for a '+' flag */
    ++mode;
    --mode_len;
    if (mode_len > 0 && *mode == '+')
    {
        /* note the read/write mode */
        rw_mode = TRUE;

        /* skip the speciifer */
        ++mode;
        --mode_len;
    }

    /* check for a binary/text specifier */
    if (mode_len > 0)
    {
        switch(*mode)
        {
        case 'b':
        case 'B':
            /* they want binary mode - set the flag */
            bin_mode = TRUE;
            break;

        case 't':
        case 'T':
            /* they want text mode - clear the binary flag */
            bin_mode = FALSE;
            break;

        case 'r':
        case 'R':
            /* they want raw mode - set the flags accordingly */
            raw_mode = TRUE;
            bin_mode = FALSE;
            break;

        default:
            goto bad_mode;
        }

        /* skip the binary/text specifier */
        ++mode;
        --mode_len;
    }

    /* it's an error if there's anything left unparsed */
    if (mode_len > 0)
        goto bad_mode;

    /* 
     *   Check to see if the file is in the current directory - if not, we
     *   may have to disallow the operation based on safety level
     *   settings.  If the file has any sort of directory prefix, assume
     *   it's not in the same directory; if not, it must be.  This is
     *   actually overly conservative, since the path may be a relative
     *   path or even an absolute path that points to the current
     *   directory, but the important thing is whether we're allowing
     *   files to specify paths at all.  
     */
    in_same_dir = (os_get_root_name(fname) == fname);

    /* get the current file safety level from the host application */
    safety = G_host_ifc->get_io_safety();

    /* check for conformance with the safety level setting */
    switch(main_mode)
    {
    case 'w':
        /* 
         *   writing - we must be safety level below WRITE_NONE to write
         *   at all, and we must be at level MINIMUM to write a file
         *   that's not in the current directory 
         */
        if (safety >= VM_IO_SAFETY_WRITE_NONE
            || (!in_same_dir && safety > VM_IO_SAFETY_MINIMUM))
        {
            /* this operation is not allowed - return failure */
            retval_nil(vmg0_);
            goto done;
        }
        break;

    case 'r':
        /*
         *   reading - we can't read at all if the safety level isn't
         *   READ_CUR or below, and we must be at level WRITE_NONE or
         *   lower to read from a file not in the current directory 
         */
        if (safety > VM_IO_SAFETY_READ_CUR
            || (!in_same_dir && safety > VM_IO_SAFETY_WRITE_NONE))
        {
            /* this operation is not allowed - return failure */
            retval_nil(vmg0_);
            goto done;
        }
        break;

    default:
        /* 
         *   fail the operation, as a code maintenance measure to make
         *   sure that we add appropriate cases to this switch (even if
         *   merely to allow the operation unconditionally) in the event
         *   that more modes are added in the future 
         */
        goto bad_mode;
    }

    /* try opening the file */
    switch(main_mode)
    {
    case 'w':
        /* check for binary/raw vs text mode */
        if (bin_mode || raw_mode)
        {
            /* 
             *   binary or raw mode -- allow read/write or just writing, but
             *   in either case truncate the file if it already exists, and
             *   create a new file if it doesn't exist 
             */
            if (rw_mode)
                fp = osfoprwtb(fname, OSFTDATA);
            else
                fp = osfopwb(fname, OSFTDATA);
        }
        else
        {
            /* text mode - don't allow read/write on a text file */
            if (rw_mode)
                goto bad_mode;

            /* open the file */
            fp = osfopwt(fname, OSFTTEXT);
        }
        break;

    case 'r':
        /* check for binary/raw vs text mode */
        if (bin_mode || raw_mode)
        {
            /*
             *   binary mode -- allow read/write or just reading; leave
             *   any existing file intact 
             */
            if (rw_mode)
                fp = osfoprwb(fname, OSFTDATA);
            else
                fp = osfoprb(fname, OSFTDATA);
        }
        else
        {
            /* text mode -- only allow reading */
            if (rw_mode)
                goto bad_mode;

            /* allocate a read buffer */
            finfo->read_buf_siz = 512;
            finfo->read_buf = (char *)t3malloc(finfo->read_buf_siz);

            /* if we couldn't allocate the buffer, throw a memory error */
            if (finfo->read_buf == 0)
                err_throw(VMERR_OUT_OF_MEMORY);

            /* there's nothing in the read buffer yet */
            finfo->read_rem = 0;

            /* 
             *   open the file - note that we open the file in binary mode
             *   even though we're reading text, since we want to perform
             *   all newline and other conversions ourselves 
             */
            fp = osfoprb(fname, OSFTTEXT);
        }
        break;

    default:
        goto bad_mode;
    }

    /* if we couldn't open it, return nil */
    if (fp == 0)
    {
        retval_nil(vmg0_);
        goto done;
    }

    /* if we're in text mode, create a character mapping */
    if (!bin_mode && !raw_mode)
    {
        /* 
         *   if a map file is specified, load it; otherwise, use plain
         *   ASCII character maps 
         */
        if (cmap[0] != '\0')
        {
            /* 
             *   open the input character map (converts from the local
             *   character set to unicode utf-8) 
             */
            finfo->charmap_read =
                CCharmapToUni::load(G_host_ifc->get_cmap_res_loader(), cmap);

            /* 
             *   open the output character map (converts from our internal
             *   unicode utf-8 character representation to the local
             *   character set) 
             */
            finfo->charmap_write =
                CCharmapToLocal::load(G_host_ifc->get_cmap_res_loader(),
                                      cmap);

            /* 
             *   if we couldn't load them, close the file and indicate
             *   failure 
             */
            if (finfo->charmap_read == 0 || finfo->charmap_write == 0)
            {
                /* release the file's resources */
                finfo->close();

                /* return failure */
                retval_nil(vmg0_);
                goto done;
            }
        }
        else if (charset_obj != 0)
        {
            /* 
             *   We have a CharacterSet object - retrieve its character
             *   mappings.  Note that these getters will throw an exception
             *   if the mappings aren't available, so we don't have to worry
             *   about null returns.  
             */
            finfo->charmap_read = charset_obj->get_to_uni(vmg0_);
            finfo->charmap_write = charset_obj->get_to_local(vmg0_);

            /* 
             *   add a reference to each mapper, to make sure the mappings
             *   stay valid at least until we close the file, at which point
             *   we'll release our references 
             */
            finfo->charmap_read->add_ref();
            finfo->charmap_write->add_ref();
        }
        else
        {
            /* create a plain ASCII reader and writer */
            finfo->charmap_read = new CCharmapToUniASCII();
            finfo->charmap_write = new CCharmapToLocalASCII();
        }
    }

    /* store the flags */
    finfo->flags = 0;
    if (bin_mode)
        finfo->flags |= VMBIFTADS_FILE_BINARY;
    if (raw_mode)
        finfo->flags |= VMBIFTADS_FILE_RAW;

    /* remember the file handle */
    finfo->fp = fp;

    /* return the file number (i.e., the slot number) */
    retval_int(vmg_ (long)fnum);

done:
    /* if we have a character set object on the stack, pop it */
    if (charset_obj != 0)
        G_stk->discard();

    /* we're finished */
    return;

bad_mode:
    /* 
     *   come here when we encounter a bad mode flag or an invalid
     *   combination of modes - throw an error indicating an invalid value
     *   was passed to the function 
     */
    err_throw(VMERR_BAD_VAL_BIF);
}

/* ------------------------------------------------------------------------ */
/*
 *   file close
 */
void CVmBifTADS::file_close(VMG_ uint argc)
{
    CVmBifTadsFile *finfo;
    
    /* check arguments */
    check_argc(vmg_ argc, 1);

    /* pop the file handle */
    finfo = pop_file_handle(vmg0_);

    /* release the file's resources */
    finfo->close();
}

/* ------------------------------------------------------------------------ */
/*
 *   file tell - get seek position of file
 */
void CVmBifTADS::file_tell(VMG_ uint argc)
{
    CVmBifTadsFile *finfo;

    /* check arguments */
    check_argc(vmg_ argc, 1);

    /* pop the file handle */
    finfo = pop_file_handle(vmg0_);

    /* return the seek position */
    retval_int(vmg_ osfpos(finfo->fp));
}

/* ------------------------------------------------------------------------ */
/*
 *   file seek - set seek position
 */
void CVmBifTADS::file_seek(VMG_ uint argc)
{
    CVmBifTadsFile *finfo;
    vm_val_t val;

    /* check arguments */
    check_argc(vmg_ argc, 2);

    /* pop the file handle */
    finfo = pop_file_handle(vmg0_);

    /* pop the seek position */
    G_interpreter->pop_int(vmg_ &val);

    /* set the new seek position */
    osfseek(finfo->fp, val.val.intval, OSFSK_SET);
}

/* ------------------------------------------------------------------------ */
/*
 *   file seek end - set seek position to end of file
 */
void CVmBifTADS::file_seekeof(VMG_ uint argc)
{
    CVmBifTadsFile *finfo;

    /* check arguments */
    check_argc(vmg_ argc, 1);

    /* pop the file handle */
    finfo = pop_file_handle(vmg0_);

    /* seek to the end of the file */
    osfseek(finfo->fp, 0, OSFSK_END);
}

/* ------------------------------------------------------------------------ */
/*
 *   file write
 */
void CVmBifTADS::file_write(VMG_ uint argc)
{
    CVmBifTadsFile *finfo;
    vm_val_t val;
    const char *constp;
    vm_val_t new_str;

    /* make sure we have at least the file ID argument */
    if (argc < 1)
        err_throw(VMERR_WRONG_NUM_OF_ARGS);

    /* pop the file handle */
    finfo = pop_file_handle(vmg0_);

    /* get the value to write */
    G_stk->pop(&val);

    /* write the value according to the mode */
    if ((finfo->flags & VMBIFTADS_FILE_RAW) != 0)
    {
        CVmObjByteArray *arr;
        unsigned long idx;
        unsigned long len;

        /* 
         *   check arguments - we require a ByteArray argument, and can
         *   optionally have a starting index and length argument 
         */
        check_argc_range(vmg_ argc, 2, 4);

        /* make sure the byte array argument is really a byte array */
        if (val.typ != VM_OBJ
            || !CVmObjByteArray::is_byte_array(vmg_ val.val.obj))
            err_throw(VMERR_BAD_TYPE_BIF);

        /* we know it's a byte array, so cast it */
        arr = (CVmObjByteArray *)vm_objp(vmg_ val.val.obj);

        /* assume we'll write the entire byte array */
        idx = 1;
        len = arr->get_element_count();

        /* if we have a starting index, retrieve it */
        if (argc >= 3)
            idx = (unsigned long)pop_int_val(vmg0_);

        /* if we have a length, retrieve it */
        if (argc >= 4)
            len = (unsigned long)pop_int_val(vmg0_);

        /* 
         *   write the bytes to the file - on success (zero write_to_file
         *   return), return nil, on failure (non-zero write_to_file
         *   return), return true 
         */
        if (arr->write_to_file(finfo->fp, idx, len))
            goto ret_error;
    }
    else if ((finfo->flags & VMBIFTADS_FILE_BINARY) != 0)
    {
        char buf[32];

        /* check arguments - we need one more, for the value to write */
        check_argc(vmg_ argc, 2);

        /* see what type of data we want to put */
        switch(val.typ)
        {
        case VM_INT:
            /* put the type in the buffer */
            buf[0] = (int)VM_INT;

            /* add the value in INT4 format */
            oswp4(buf + 1, val.val.intval);

            /* write out the type prefix plus the value */
            if (osfwb(finfo->fp, buf, 5))
                goto ret_error;

            /* done */
            break;

        case VM_ENUM:
            /* put the type in the buffer */
            buf[0] = (int)VM_ENUM;

            /* add the value in INT4 format */
            oswp4(buf + 1, val.val.enumval);

            /* write out the type prefix plus the value */
            if (osfwb(finfo->fp, buf, 5))
                goto ret_error;

            /* done */
            break;

        case VM_SSTRING:
            /* get the string value pointer */
            constp = val.get_as_string(vmg0_);

        write_binary_string:
            /* write the type prefix byte */
            buf[0] = (int)VM_SSTRING;
            if (osfwb(finfo->fp, buf, 1))
                goto ret_error;
            
            /* write the string, including the length prefix */
            if (osfwb(finfo->fp, constp, vmb_get_len(constp) + VMB_LEN))
                goto ret_error;

            /* done */
            break;

        case VM_OBJ:
            /* 
             *   Cast it to a string value and write that out.  Note that
             *   we can ignore garbage collection for any new string we've
             *   created, since we're just calling the OS-level file
             *   writer, which will never invoke garbage collection.  
             */
            constp = vm_objp(vmg_ val.val.obj)
                     ->cast_to_string(vmg_ val.val.obj, &new_str);
            goto write_binary_string;

        case VM_TRUE:
            /* 
             *   All we need for this is the type prefix.  Note that we
             *   can't write nil because we'd have no way of reading it
             *   back in - a nil return from file_read indicates that
             *   we've reached the end of the file.  So there's no point
             *   in writing nil to a file.  
             */
            buf[0] = (int)VM_TRUE;
            if (osfwb(finfo->fp, buf, 1))
                goto ret_error;

            /* done */
            break;

        default:
            /* other types are not acceptable */
            err_throw(VMERR_BAD_TYPE_BIF);
        }
    }
    else
    {
        char conv_buf[128];
        vm_val_t new_str;
        
        /* check arguments - we need one more, for the string to write */
        check_argc(vmg_ argc, 2);

        /* convert the value to a string */
        constp = CVmObjString::cvt_to_str(vmg_ &new_str,
                                          conv_buf, sizeof(conv_buf),
                                          &val, 10);

        /* write the string value through our character mapper */
        if (finfo->charmap_write->write_file(finfo->fp, constp + VMB_LEN,
                                             vmb_get_len(constp)))
            goto ret_error;
    }

    /* success - return nil */
    retval_nil(vmg0_);
    return;

ret_error:
    /* on error, return true */
    retval_true(vmg0_);
}

/* ------------------------------------------------------------------------ */
/*
 *   file read
 */
void CVmBifTADS::file_read(VMG_ uint argc)
{
    CVmBifTadsFile *finfo;
    vm_val_t val;

    /* make sure we have at least the file handle argument */
    if (argc < 1)
        err_throw(VMERR_WRONG_NUM_OF_ARGS);

    /* pop the file handle */
    finfo = pop_file_handle(vmg0_);

    /* read a data item or a line, depending on the file mode */
    if ((finfo->flags & VMBIFTADS_FILE_RAW) != 0)
    {
        CVmObjByteArray *arr;
        unsigned long idx;
        unsigned long len;

        /* 
         *   raw read - we require a ByteArray object argument, and
         *   optionally a starting index and length 
         */
        check_argc_range(vmg_ argc, 2, 4);

        /* get the destination ByteArray argument */
        G_stk->pop(&val);

        /* make sure it's really a ByteArray object */
        if (val.typ != VM_OBJ
            || !CVmObjByteArray::is_byte_array(vmg_ val.val.obj))
            err_throw(VMERR_BAD_TYPE_BIF);

        /* we know it's a byte array object, so cast it */
        arr = (CVmObjByteArray *)vm_objp(vmg_ val.val.obj);

        /* presume we'll try to fill the entire array */
        idx = 1;
        len = arr->get_element_count();

        /* if we have a starting index argument, retrieve it */
        if (argc >= 3)
            idx = (unsigned long)pop_int_val(vmg0_);

        /* if we have a length argument, retrieve it */
        if (argc >= 4)
            len = (unsigned long)pop_int_val(vmg0_);

        /* 
         *   read the data into the array, and return the number of bytes we
         *   read 
         */
        retval_int(vmg_ arr->read_from_file(finfo->fp, idx, len));
    }
    else if ((finfo->flags & VMBIFTADS_FILE_BINARY) != 0)
    {
        char buf[32];
        CVmObjString *str_obj;

        /* only one argument allowed in binary mode */
        check_argc(vmg_ argc, 1);

        /* read the type flag */
        if (osfrb(finfo->fp, buf, 1))
            goto ret_error;

        /* see what we have */
        switch((vm_datatype_t)buf[0])
        {
        case VM_INT:
            /* read the INT4 value */
            if (osfrb(finfo->fp, buf, 4))
                goto ret_error;
            val.set_int(osrp4(buf));
            break;

        case VM_ENUM:
            /* read the UINT4 value */
            if (osfrb(finfo->fp, buf, 4))
                goto ret_error;
            val.set_enum(osrp4(buf));
            break;

        case VM_SSTRING:
            /* read the string's length */
            if (osfrb(finfo->fp, buf, 2))
                goto ret_error;

            /* allocate a new string of the required size */
            val.set_obj(CVmObjString::create(vmg_ FALSE, osrp2(buf)));
            str_obj = (CVmObjString *)vm_objp(vmg_ val.val.obj);

            /* read the rest of the string into the object's buffer */
            if (osfrb(finfo->fp, str_obj->cons_get_buf(), osrp2(buf)))
                goto ret_error;

            /* success */
            break;

        case VM_TRUE:
            /* it's a simple 'true' value */
            val.set_true();
            break;

        default:
            /* invalid data - return failure */
            goto ret_error;
        }

        /* return the value */
        retval(vmg_ &val);
    }
    else
    {
        CVmObjString *str;
        size_t str_len;

        /* only one argument allowed in text mode */
        check_argc(vmg_ argc, 1);

        /* we haven't yet constructed a string */
        str = 0;
        str_len = 0;

        /* 
         *   assume we'll fail to read anything, in which case we'll
         *   return nil 
         */
        val.set_nil();

        /* 
         *   push the nil value - we'll always keep our intermediate value
         *   on the stack so that the garbage collector will know it's
         *   referenced 
         */
        G_stk->push(&val);

        /*
         *   The file is in text mode.  Read a line of text from the file
         *   into our buffer.  Keep going until we read an entire line; we
         *   might have to read the line in chunks, since the line might
         *   end up being longer than our buffer.  
         */
        for (;;)
        {
            wchar_t found_nl;
            char *start;
            size_t new_len;
            size_t nl_len;

            /* replenish the read buffer if it's empty */
            if (finfo->read_rem == 0)
            {
                /* read more text */
                finfo->read_rem = finfo->charmap_read->read_file(
                    finfo->fp, finfo->read_buf, finfo->read_buf_siz);

                /* if we read nothing, we're at the end of the file */
                if (finfo->read_rem == 0)
                    break;

                /* read from the start of the buffer */
                finfo->read_ptr.set(finfo->read_buf);
            }

            /* note where we started this chunk */
            start = finfo->read_ptr.getptr();
            
            /* scan for and remove any trailing newline */
            for (found_nl = '\0' ; finfo->read_rem != 0 ;
                 finfo->read_ptr.inc(&finfo->read_rem))
            {
                wchar_t cur;

                /* get the current character */
                cur = finfo->read_ptr.getch();

                /* 
                 *   check for a newline (note that 0x2028 is the unicode
                 *   line separator character) 
                 */
                if (cur == '\n' || cur == '\r' || cur == 0x2028)
                {
                    /* note the newline */
                    found_nl = cur;
                    
                    /* no need to look any further */
                    break;
                }
            }

            /* note the length of the current segment */
            new_len = finfo->read_ptr.getptr() - start;

            /* 
             *   if there's a newline character, include an extra byte for
             *   the '\n' we'll include in the result 
             */
            nl_len = (found_nl != '\0');

            /* 
             *   If this is our first segment, construct a new string from
             *   this chunk; otherwise, add to the existing string.
             *   
             *   Note that in either case, if we found a newline in the
             *   buffer, we will NOT add the actual newline we found to the
             *   result string.  Rather, we'll add a '\n' character to the
             *   result string, no matter what kind of newline we found.
             *   This ensures that the data read uses a consistent format,
             *   regardless of the local system convention where the file
             *   was created.  
             */
            if (str == 0)
            {
                /* create our first segment's string */
                val.set_obj(CVmObjString::
                            create(vmg_ FALSE, new_len + nl_len));
                str = (CVmObjString *)vm_objp(vmg_ val.val.obj);

                /* copy the segment into the string object */
                memcpy(str->cons_get_buf(), start, new_len);

                /* add a '\n' if we found a newline */
                if (found_nl != '\0')
                    *(str->cons_get_buf() + new_len) = '\n';

                /* this is the length of the string so far */
                str_len = new_len + nl_len;

                /* 
                 *   replace the stack placeholder with our string, so the
                 *   garbage collector will know it's still in use 
                 */
                G_stk->discard();
                G_stk->push(&val);
            }
            else
            {
                CVmObjString *new_str;
                
                /* 
                 *   create a new string to hold the contents of the old
                 *   string plus the new buffer 
                 */
                val.set_obj(CVmObjString::create(vmg_ FALSE,
                    str_len + new_len + nl_len));
                new_str = (CVmObjString *)vm_objp(vmg_ val.val.obj);

                /* copy the old string into the new string */
                memcpy(new_str->cons_get_buf(),
                       str->get_as_string(vmg0_) + VMB_LEN, str_len);

                /* add the new chunk after the copy of the old string */
                memcpy(new_str->cons_get_buf() + str_len, start, new_len);

                /* add the newline if necessary */
                if (found_nl != '\0')
                    *(new_str->cons_get_buf() + str_len + new_len) = '\n';

                /* the new string now replaces the old string */
                str = new_str;
                str_len += new_len + nl_len;

                /* 
                 *   replace our old intermediate value on the stack with
                 *   the new string - the old string isn't needed any
                 *   more, so we can leave it unreferenced, but we are
                 *   still using the new string 
                 */
                G_stk->discard();
                G_stk->push(&val);
            }

            /* if we found a newline in this segment, we're done */
            if (found_nl != '\0')
            {
                /* skip the newline in the input */
                finfo->read_ptr.inc(&finfo->read_rem);
                
                /* 
                 *   if the buffer is empty, fill it again, so we can check
                 *   for a complementing newline character 
                 */
                if (finfo->read_rem == 0)
                {
                    /* read more text */
                    finfo->read_rem = finfo->charmap_read->read_file(
                        finfo->fp, finfo->read_buf, finfo->read_buf_siz);
                    
                    /* set up to read from the new text */
                    finfo->read_ptr.set(finfo->read_buf);
                }

                /* 
                 *   check for a complementary newline character, for
                 *   systems that use \n\r or \r\n pairs 
                 */
                if (finfo->read_rem != 0)
                {
                    wchar_t nxt;
                    
                    /* get the next character */
                    nxt = finfo->read_ptr.getch();
                    
                    /* check for a complementary character */
                    if ((found_nl == '\n' && nxt == '\r')
                        || (found_nl == '\r' && nxt == '\n'))
                    {
                        /* 
                         *   we have a pair sequence - skip the second
                         *   character of the sequence 
                         */
                        finfo->read_ptr.inc(&finfo->read_rem);
                    }
                }

                /* we've found the newline, so we're done with the string */
                break;
            }
        }

        /* return the string or nil value we constructed */
        retval(vmg_ &val);

        /* 
         *   we now can discard the string we've been keeping on the stack
         *   to for garbage collection protection 
         */
        G_stk->discard();
    }

    /* success - return with the value we set in R0 */
    return;

ret_error:
    /* return nil to indicate that we couldn't read anything */
    retval_nil(vmg0_);
}

