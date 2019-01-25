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
  vmbiftio.cpp - TADS Input/Output functions
Function
  
Notes
  
Modified
  02/08/00 MJRoberts  - Creation
*/

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "t3std.h"
#include "os.h"
#include "utf8.h"
#include "charmap.h"
#include "vmbiftio.h"
#include "vmstack.h"
#include "vmerr.h"
#include "vmerrnum.h"
#include "vmglob.h"
#include "vmpool.h"
#include "vmobj.h"
#include "vmstr.h"
#include "vmlst.h"
#include "vmrun.h"
#include "vmfile.h"
#include "vmconsol.h"
#include "vmstrres.h"
#include "vmvsn.h"
#include "vmhost.h"
#include "vmpredef.h"
#include "vmcset.h"
#include "vmfilobj.h"


/* ------------------------------------------------------------------------ */
/*
 *   Display a value 
 */
void CVmBifTIO::say(VMG_ uint argc)
{
    /* write the value to the main console */
    say_to_console(vmg_ G_console, argc);
}

/*
 *   Display the value or values at top of stack to the given console 
 */
void CVmBifTIO::say_to_console(VMG_ CVmConsole *console, uint argc)
{
    vm_val_t val;
    const char *str;
    char buf[30];
    size_t len;
    vm_val_t new_str;

    /* presume we won't need to create a new string value */
    new_str.set_nil();

    /* display each argument */
    for ( ; argc != 0 ; --argc)
    {
        /* get our argument */
        G_stk->pop(&val);

        /* see what we have */
        switch(val.typ)
        {
        case VM_SSTRING:
            /* get the string */
            str = G_const_pool->get_ptr(val.val.ofs);

            /* the original value is our string */
            new_str = val;

        disp_str:
            /* push the string to protect from garbage collection */
            G_stk->push(&new_str);

            /* display the string through the output formatter */
            console->format_text(vmg_ str + 2, osrp2(str));

            /* discard the saved string now that we no longer need it */
            G_stk->discard();

            /* done */
            break;

        case VM_OBJ:
            /* convert it to a string */
            str = vm_objp(vmg_ val.val.obj)
                  ->cast_to_string(vmg_ val.val.obj, &new_str);

            /* go display it */
            goto disp_str;

        case VM_INT:
            /* convert it to a string */
            sprintf(buf + 2, "%ld", val.val.intval);

            /* set its length */
            len = strlen(buf + 2);
            oswp2(buf, len);

            /* display it */
            str = buf;
            goto disp_str;

        case VM_NIL:
            /* display nothing */
            break;

        default:
            /* other types are invalid */
            err_throw(VMERR_BAD_TYPE_BIF);
        }
    }
}

/* ------------------------------------------------------------------------ */
/*
 *   Logging - turn on or off output text capture 
 */

#define LOG_SCRIPT   1
#define LOG_CMD      2

void CVmBifTIO::logging(VMG_ uint argc)
{
    vm_val_t fname_val;
    int log_type;
    
    /* check arguments */
    check_argc_range(vmg_ argc, 1, 2);

    /* retrieve the filename argument */
    G_stk->pop(&fname_val);

    /* get the log type argument, if present */
    log_type = (argc >= 2 ? pop_int_val(vmg0_) : LOG_SCRIPT);

    /* 
     *   if they passed us nil, turn off logging; otherwise, start logging
     *   to the filename given by the string 
     */
    if (fname_val.typ == VM_NIL)
    {
        /* turn off the appropriate type of logging */
        switch(log_type)
        {
        case LOG_SCRIPT:
            G_console->close_log_file();
            break;

        case LOG_CMD:
            G_console->close_command_log();
            break;

        default:
            err_throw(VMERR_BAD_VAL_BIF);
        }
    }
    else
    {
        char fname[OSFNMAX];
        
        /* 
         *   get the filename string (converted to the file system
         *   character set) 
         */
        G_stk->push(&fname_val);
        pop_str_val_fname(vmg_ fname, sizeof(fname));

        /* open the appropriate log file */
        switch(log_type)
        {
        case LOG_SCRIPT:
            G_console->open_log_file(fname);
            break;

        case LOG_CMD:
            G_console->open_command_log(fname);
            break;

        default:
            err_throw(VMERR_BAD_VAL_BIF);
        }
    }
}

/* ------------------------------------------------------------------------ */
/*
 *   clearscreen - clear the main display screen
 */
void CVmBifTIO::clearscreen(VMG_ uint argc)
{
    /* check arguments */
    check_argc(vmg_ argc, 0);

    /* ask the console to clear the screen */
    G_console->clear_window(vmg0_);
}

/* ------------------------------------------------------------------------ */
/*
 *   more - show MORE prompt
 */
void CVmBifTIO::more(VMG_ uint argc)
{
    /* check arguments */
    check_argc(vmg_ argc, 0);

    /* 
     *   if we're reading from a script, ignore this request - these types of
     *   interactive pauses are irrelevant when reading a script, since we're
     *   getting our input non-interactively 
     */
    if (G_console->is_reading_script())
        return;

    /* flush the display output */
    G_console->flush_all(vmg_ VM_NL_NONE);

    /* show the MORE prompt */
    G_console->show_more_prompt(vmg0_);
}

/* ------------------------------------------------------------------------ */
/*
 *   input - get a line of input from the keyboard
 */
void CVmBifTIO::input(VMG_ uint argc)
{
    char buf[256];
    
    /* check arguments */
    check_argc(vmg_ argc, 0);

    /* read a line of text from the keyboard */
    if (G_console->read_line(vmg_ buf, sizeof(buf)))
    {
        /* end of file - return nil */
        retval_nil(vmg0_);
    }
    else
    {
        /* return the string */
        retval_str(vmg_ buf);
    }
}

/* ------------------------------------------------------------------------ */
/*
 *   inputkey - read a keystroke
 */
void CVmBifTIO::inputkey(VMG_ uint argc)
{
    char buf[32];
    char c[4];
    size_t len;

    /* check arguments */
    check_argc(vmg_ argc, 0);

    /* flush any output */
    G_console->flush_all(vmg_ VM_NL_INPUT);

    /* get a keystroke */
    c[0] = os_getc_raw();
    len = 1;

    /* if it's an extended key, map it specially */
    if (c[0] == 0)
    {
        char extc;

        /* get the second part of the sequence */
        extc = os_getc_raw();

        /* map the extended key */
        map_ext_key(vmg_ buf, (unsigned char)extc);
    }
    else
    {
        /* continue fetching bytes until we have a full character */
        while (!raw_key_complete(vmg_ c, len) && len < sizeof(c))
        {
            /* 
             *   We don't yet have enough bytes for a complete character, so
             *   read another raw byte.  The keyboard driver should already
             *   have queued up all of the bytes we need to complete the
             *   character sequence, so there should never be a delay from
             *   os_getc_raw() here - it should simply return the next byte
             *   of the sequence immediately.  
             */
            c[len++] = os_getc_raw();
        }

        /* 
         *   translate the key from the local character set to UTF-8, and
         *   map the extended key code to the portable representation 
         */
        map_raw_key(vmg_ buf, c, len);
    }

    /* reset the [MORE] counter */
    G_console->reset_line_count(FALSE);

    /* return the string */
    retval_str(vmg_ buf);
}

/* ------------------------------------------------------------------------ */
/*
 *   inputevent - read an event
 */
void CVmBifTIO::inputevent(VMG_ uint argc)
{
    char keyname[32];
    int use_timeout;
    unsigned long timeout;
    os_event_info_t info;
    int evt;
    int ele_count;
    vm_obj_id_t lst_obj;
    CVmObjList *lst;
    vm_val_t val;

    /* check arguments */
    check_argc_range(vmg_ argc, 0, 1);

    /* if there's a timeout argument, get it */
    if (argc == 0)
    {
        /* there's no timeout */
        use_timeout = FALSE;
        timeout = 0;
    }
    else if (G_stk->get(0)->typ == VM_NIL)
    {
        /* the timeout is nil, which is the same as no timeout */
        use_timeout = FALSE;
        timeout = 0;

        /* discard the nil timeout value */
        G_stk->discard();
    }
    else
    {
        /* pop the timeout value */
        timeout = pop_long_val(vmg0_);

        /* note that we have a timeout to use */
        use_timeout = TRUE;
    }

    /* flush any buffered output */
    G_console->flush_all(vmg_ VM_NL_INPUT);

    /* reset the [MORE] counter */
    G_console->reset_line_count(FALSE);

    /* read an event */
    evt = os_get_event(timeout, use_timeout, &info);

    /* figure out how big a list we need to allocate */
    switch(evt)
    {
    case OS_EVT_KEY:
        /* 
         *   we need two elements - one for the event type code, one for
         *   the keystroke string 
         */
        ele_count = 2;
        break;

    case OS_EVT_HREF:
        /* 
         *   we need two elements - one for the event type code, one for
         *   the HREF string 
         */
        ele_count = 2;
        break;

    default:
        /* for anything else, we need only the event type code element */
        ele_count = 1;
        break;
    }

    /* create the return list */
    lst_obj = CVmObjList::create(vmg_ FALSE, ele_count);
    lst = (CVmObjList *)vm_objp(vmg_ lst_obj);

    /* save the list on the stack to protect against garbage collection */
    val.set_obj(lst_obj);
    G_stk->push(&val);

    /* fill in the first element with the event type code */
    val.set_int(evt);
    lst->cons_set_element(0, &val);

    /* set additional elements, according to the event type */
    switch(evt)
    {
    case OS_EVT_KEY:
        /* map the extended or ordinary key, as appropriate */
        if (info.key[0] == 0)
        {
            /* it's an extended key */
            map_ext_key(vmg_ keyname, info.key[1]);
        }
        else
        {
            char c[4];
            size_t len;

            /* fetch more bytes until we have a complete character */
            for (c[0] = info.key[0], len = 1 ;
                 !raw_key_complete(vmg_ c, len) && len < sizeof(c) ; )
            {
                /* 
                 *   Read another input event.  The keyboard driver should
                 *   already have queued up all of the bytes needed to
                 *   complete this character sequence, so there should never
                 *   be a delay from os_get_event() here - it should simply
                 *   return immediately with another OS_EVT_KEY event with
                 *   the next byte of the sequence.  
                 */
                evt = os_get_event(0, FALSE, &info);

                /* 
                 *   if it's not a keystroke event, something's wrong -
                 *   ignore the event and stop trying to read the remaining
                 *   bytes of the character sequence 
                 */
                if (evt != OS_EVT_KEY)
                    break;

                /* store the next byte of the sequence */
                c[len++] = info.key[0];
            }

            /* it's an ordinary key - map it */
            map_raw_key(vmg_ keyname, c, len);
        }
        
        /* create a string for the key name */
        val.set_obj(CVmObjString::create(vmg_ FALSE, keyname,
                                         strlen(keyname)));

        /* add it to the list */
        lst->cons_set_element(1, &val);
        break;

    case OS_EVT_HREF:
        /* create the string for the href text */
        val.set_obj(CVmObjString::create(vmg_ FALSE, info.href,
                                         strlen(info.href)));

        /* add it to the list */
        lst->cons_set_element(1, &val);
        break;

    default:
        /* other event types have no extra data */
        break;
    }

    /* return the list */
    retval_obj(vmg_ lst_obj);

    /* we can drop the garbage collection protection now */
    G_stk->discard();
}


/* ------------------------------------------------------------------------ */
/*
 *   Service routine: Map an "extended" keystroke from raw os_getc_raw()
 *   notation to a UTF-8 key name.  The caller should pass the second byte of
 *   the extended two-byte raw sequence.  
 */
int CVmBifTIO::map_ext_key(VMG_ char *namebuf, int extc)
{
    /*
     *   Portable key names for the extended keystrokes.  We map the extended
     *   key codes to these strings, so that the TADS code can access arrow
     *   keys and the like.  
     */
    static const char *ext_key_names[] =
    {
        "[up]",                                               /* CMD_UP - 1 */
        "[down]",                                           /* CMD_DOWN - 2 */
        "[right]",                                         /* CMD_RIGHT - 3 */
        "[left]",                                           /* CMD_LEFT - 4 */
        "[end]",                                             /* CMD_END - 5 */
        "[home]",                                           /* CMD_HOME - 6 */
        "[del-eol]",                                        /* CMD_DEOL - 7 */
        "[del-line]",                                       /* CMD_KILL - 8 */
        "[del]",                                             /* CMD_DEL - 9 */
        "[scroll]",                                         /* CMD_SCR - 10 */
        "[page up]",                                       /* CMD_PGUP - 11 */
        "[page down]",                                     /* CMD_PGDN - 12 */
        "[top]",                                            /* CMD_TOP - 13 */
        "[bottom]",                                         /* CMD_BOT - 14 */
        "[f1]",                                              /* CMD_F1 - 15 */
        "[f2]",                                              /* CMD_F2 - 16 */
        "[f3]",                                              /* CMD_F3 - 17 */
        "[f4]",                                              /* CMD_F4 - 18 */
        "[f5]",                                              /* CMD_F5 - 19 */
        "[f6]",                                              /* CMD_F6 - 20 */
        "[f7]",                                              /* CMD_F7 - 21 */
        "[f8]",                                              /* CMD_F8 - 22 */
        "[f9]",                                              /* CMD_F9 - 23 */
        "[f10]",                                            /* CMD_F10 - 24 */
        "[?]",                              /* invalid key - CMD_CHOME - 25 */
        "[tab]",                                            /* CMD_TAB - 26 */
        "[?]",                               /* invalid key - shift-F2 - 27 */
        "[?]",                                 /* not used (obsoleted) - 28 */
        "[word-left]",                                /* CMD_WORD_LEFT - 29 */
        "[word-right]",                              /* CMD_WORD_RIGHT - 30 */
        "[del-word]",                                  /* CMD_WORDKILL - 31 */
        "[eof]",                                            /* CMD_EOF - 32 */
        "[break]"                                         /* CMD_BREAK - 33 */
    };

    /* if it's in the key name array, use the array entry */
    if (extc >= 1
        && extc <= (int)sizeof(ext_key_names)/sizeof(ext_key_names[0]))
    {
        /* use the array name */
        strcpy(namebuf, ext_key_names[extc - 1]);
        return TRUE;
    }

    /* if it's in the ALT key range, generate an ALT key name */
    if ((unsigned char)extc >= CMD_ALT && (unsigned char)extc <= CMD_ALT + 25)
    {
        /* generate an ALT key name */
        strcpy(namebuf, "[alt-?]");
        namebuf[5] = (char)(extc - CMD_ALT + 'a');
        return TRUE;
    }

    /* it's not a valid key - use '[?]' as the name */
    strcpy(namebuf, "[?]");
    return FALSE;
}

/*
 *   Service routine: Map a keystroke from the raw notation, consisting of a
 *   normal keystroke in the local character set or an extended command key
 *   using a CMD_xxx code, to UTF-8.  If the keystroke is a control character
 *   or any CMD_xxx code, we'll map the key to a high-level keystroke name
 *   enclosed in square brackets.  
 */
int CVmBifTIO::map_raw_key(VMG_ char *namebuf, const char *c, size_t len)
{
    size_t outlen;

    /* if it's a control character, give it a portable key name */
    if (len == 1 && ((c[0] >= 1 && c[0] <= 27) || c[0] == 127))
    {
        switch(c[0])
        {
        case 10:
        case 13:
            /* 
             *   return an ASCII 10 (regardless of local newline conventions
             *   - this is the internal string representation, which we
             *   define to use ASCII 10 to represent a newline everywhere) 
             */
            namebuf[0] = 10;
            namebuf[1] = '\0';
            return TRUE;

        case 9:
            /* return ASCII 9 for TAB characters */
            namebuf[0] = 9;
            namebuf[1] = '\0';
            return TRUE;

        case 8:
        case 127:
            /* return '[bksp]' for backspace/del characters */
            strcpy(namebuf, "[bksp]");
            return TRUE;

        case 27:
            /* return '[esc]' for the escape key */
            strcpy(namebuf, "[esc]");
            return TRUE;

        default:
            /* return '[ctrl-X]' for other control characters */
            strcpy(namebuf, "[ctrl-?]");
            namebuf[6] = (char)(c[0] + 'a' - 1);
            return TRUE;
        }
    }

    /* map the character to wide Unicode */
    outlen = 32;
    G_cmap_from_ui->map(&namebuf, &outlen, c, len);

    /* null-terminate the result */
    *namebuf = '\0';

    /* successfully mapped */
    return TRUE;
}

/*
 *   Service routine: determine if a raw byte sequence forms a complete
 *   character in the local character set.  
 */
int CVmBifTIO::raw_key_complete(VMG_ const char *c, size_t len)
{
    /* ask the local character mapper if it's a complete character */
    return G_cmap_from_ui->is_complete_char(c, len);
}


/* ------------------------------------------------------------------------ */
/*
 *   Standard system button labels for bifinpdlg() 
 */
#define BIFINPDLG_LBL_OK      1
#define BIFINPDLG_LBL_CANCEL  2
#define BIFINPDLG_LBL_YES     3
#define BIFINPDLG_LBL_NO      4

/*
 *   inputdialog - run a dialog
 */
void CVmBifTIO::inputdialog(VMG_ uint argc)
{
    int icon_id;
    char prompt[256];
    char label_buf[256];
    vm_val_t label_val[10];
    const char *labels[10];
    int std_btns;
    int btn_cnt;
    const char *listp;
    char *dst;
    size_t dstrem;
    int default_resp;
    int cancel_resp;
    int resp;
    
    /* check arguments */
    check_argc(vmg_ argc, 5);

    /* get the icon number */
    icon_id = pop_int_val(vmg0_);

    /* get the prompt string */
    pop_str_val_buf(vmg_ prompt, sizeof(prompt));

    /* there aren't any buttons yet */
    btn_cnt = 0;

    /* check for the button type */
    if (G_stk->get(0)->typ == VM_INT)
    {
        /* get the standard button set ID */
        std_btns = pop_int_val(vmg0_);
    }
    else
    {
        size_t i;
        size_t cnt;
        vm_val_t *valp;
        
        /* we're not using any standard button set */
        std_btns = 0;

        /* get the button label list */
        listp = pop_list_val(vmg0_);

        /* 
         *   run through the list and get the button items into our array
         *   (we do this rather than traversing the list directly so that
         *   we don't have to worry about a constant list's data being
         *   paged out) 
         */
        cnt = vmb_get_len(listp);

        /* limit the number of elements to our private array size */
        if (cnt > sizeof(label_val)/sizeof(label_val[0]))
            cnt = sizeof(label_val)/sizeof(label_val[0]);

        /* skip the list length prefix */
        listp += VMB_LEN;

        /* copy the list */
        for (i = cnt, valp = label_val ; i > 0 ;
             --i, listp += VMB_DATAHOLDER, ++valp)
        {
            /* get this element into our array */
            vmb_get_dh(listp, valp);
        }

        /* set up to write into our label buffer */
        dst = label_buf;
        dstrem = sizeof(label_buf);

        /* now build our internal button list from the array elements */
        for (i = 0, valp = label_val ; i < cnt ; ++i, ++valp)
        {
            const char *p;

            /* 
             *   We could have a number or a string in each element.  If
             *   the element is a number, it refers to a standard label.
             *   If it's a string, use the string directly. 
             */
            if ((p = valp->get_as_string(vmg0_)) != 0)
            {
                size_t copy_len;
                
                /* 
                 *   it's a string - make a copy in the label buffer,
                 *   making sure to leave space for null termination 
                 */
                copy_len = vmb_get_len(p);
                if (copy_len > dstrem - 1)
                    copy_len = utf8_ptr::s_trunc(p + VMB_LEN, dstrem - 1);
                memcpy(dst, p + VMB_LEN, copy_len);

                /* null-terminate the buffer */
                dst[copy_len++] = '\0';

                /* set this button to point to the converted text */
                labels[btn_cnt++] = dst;

                /* skip past this label */
                dst += copy_len;
                dstrem -= copy_len;
            }
            else if (valp->typ == VM_INT)
            {
                int id;
                int resid;
                char rscbuf[128];
                
                /* it's a standard system label ID - get the ID */
                id = (int)valp->val.intval;

                /* translate it to the appropriate string resource */
                switch(id)
                {
                case BIFINPDLG_LBL_OK:
                    resid = VMRESID_BTN_OK;
                    break;

                case BIFINPDLG_LBL_CANCEL:
                    resid = VMRESID_BTN_CANCEL;
                    break;

                case BIFINPDLG_LBL_YES:
                    resid = VMRESID_BTN_YES;
                    break;

                case BIFINPDLG_LBL_NO:
                    resid = VMRESID_BTN_NO;
                    break;

                default:
                    resid = 0;
                    break;
                }

                /* 
                 *   if we got a valid resource ID, load the resource;
                 *   otherwise, skip this button 
                 */
                if (resid != 0
                    && !os_get_str_rsc(resid, rscbuf, sizeof(rscbuf)))
                {
                    /* set this button to point to the converted text */
                    labels[btn_cnt++] = dst;

                    /* convert the resource text to UTF-8 */
                    G_cmap_from_ui->map(&dst, &dstrem,
                                        rscbuf, strlen(rscbuf));

                    /* null-terminate the converted text */
                    *dst++ = '\0';
                    --dstrem;
                }
            }
        }
    }

    /* get the default response */
    if (G_stk->get(0)->typ == VM_NIL)
    {
        /* discard the nil argument */
        G_stk->discard();

        /* there's no default response */
        default_resp = 0;
    }
    else
    {
        /* get the default response index */
        default_resp = pop_int_val(vmg0_);
    }

    /* get the cancel response */
    if (G_stk->get(0)->typ == VM_NIL)
    {
        /* discard the nil argument */
        G_stk->discard();

        /* there's no cancel response */
        cancel_resp = 0;
    }
    else
    {
        /* get the cancel response index */
        cancel_resp = pop_int_val(vmg0_);
    }

    /* flush output before showing the dialog */
    G_console->flush_all(vmg_ VM_NL_INPUT);

    /* show the dialog */
    resp = G_console->input_dialog(vmg_ icon_id, prompt,
                                   std_btns, labels, btn_cnt,
                                   default_resp, cancel_resp);

    /* return the result */
    retval_int(vmg_ resp);
}

/* ------------------------------------------------------------------------ */
/*
 *   askfile - ask for a filename via a standard file dialog
 */
void CVmBifTIO::askfile(VMG_ uint argc)
{
    char prompt[256];
    int dialog_type;
    os_filetype_t file_type;
    long flags;
    int result;
    char fname[OSFNMAX*3 + 1];
    vm_obj_id_t lst_obj;
    CVmObjList *lst;
    vm_val_t val;
    
    /* check arguments */
    check_argc(vmg_ argc, 4);

    /* get the prompt string */
    pop_str_val_buf(vmg_ prompt, sizeof(prompt));

    /* get the dialog type and file type */
    dialog_type = pop_int_val(vmg0_);
    file_type = (os_filetype_t)pop_int_val(vmg0_);

    /* pop the flags */
    flags = pop_long_val(vmg0_);

    /* ask for a file */
    result = G_console->askfile(vmg_ prompt, strlen(prompt),
                                fname, sizeof(fname), dialog_type, file_type);

    /* 
     *   Allocate a list to store the return value.  If we successfully
     *   got a filename, we need a two-element list - one element for the
     *   success code and another for the string with the filename.  If we
     *   didn't succeed in getting the filename, we only need a single
     *   element, which will contain the error code. 
     */
    lst_obj = CVmObjList::create(vmg_ FALSE,
                                 result == OS_AFE_SUCCESS ? 2 : 1);
    lst = (CVmObjList *)vm_objp(vmg_ lst_obj);

    /* save it on the stack as protection against garbage collection */
    val.set_obj(lst_obj);
    G_stk->push(&val);

    /* set the first element to the result code */
    val.set_int(result);
    lst->cons_set_element(0, &val);

    /* if we got a filename, set the second element to the filename string */
    if (result == OS_AFE_SUCCESS)
    {
        /* create a string for the filename */
        val.set_obj(CVmObjString::create(vmg_ FALSE, fname, strlen(fname)));

        /* store the string as the second list element */
        lst->cons_set_element(1, &val);
    }

    /* return the list */
    retval_obj(vmg_ lst_obj);

    /* we no longer need the garbage collector protection */
    G_stk->discard();
}


/* ------------------------------------------------------------------------ */
/*
 *   timeDelay - pause for a specified interval
 */
void CVmBifTIO::timedelay(VMG_ uint argc)
{
    long delay_ms;
    
    /* check arguments */
    check_argc(vmg_ argc, 1);

    /* get the delay time in milliseconds */
    delay_ms = pop_long_val(vmg0_);

    /* flush any pending output */
    G_console->flush_all(vmg_ VM_NL_NONE);

    /* ask the system code to pause */
    os_sleep_ms(delay_ms);
}

/* ------------------------------------------------------------------------ */
/*
 *   systemInfo
 */
void CVmBifTIO::sysinfo(VMG_ uint argc)
{
    int info;
    long result;

    /* make sure we have at least one argument */
    if (argc < 1)
        err_throw(VMERR_WRONG_NUM_OF_ARGS);

    /* get the information type code */
    info = pop_int_val(vmg0_);

    /* see what we have */
    switch(info)
    {
    case SYSINFO_SYSINFO:
        /* there are no additional arguments for this information type */
        check_argc(vmg_ argc, 1);

        /* system information is supported in this version - return true */
        retval_true(vmg0_);
        break;

    case SYSINFO_VERSION:
        /* there are no additional arguments for this information type */
        check_argc(vmg_ argc, 1);

        /* return the VM version string, formatted as a string */
        {
            char buf[30];

            sprintf(buf, "%d.%d.%d",
                    (int)((T3VM_VSN_NUMBER >> 16) & 0xffff),
                    (int)((T3VM_VSN_NUMBER >> 8) & 0xff),
                    (int)(T3VM_VSN_NUMBER & 0xff));
            retval_str(vmg_ buf);
        }
        break;

    case SYSINFO_OS_NAME:
        /* there are no additional arguments for this information type */
        check_argc(vmg_ argc, 1);

        /* return the OS name as a string */
        retval_str(vmg_ OS_SYSTEM_NAME);
        break;

    case SYSINFO_HTML:
    case SYSINFO_JPEG:
    case SYSINFO_PNG:
    case SYSINFO_WAV:
    case SYSINFO_MIDI:
    case SYSINFO_WAV_MIDI_OVL:
    case SYSINFO_WAV_OVL:
    case SYSINFO_PREF_IMAGES:
    case SYSINFO_PREF_SOUNDS:
    case SYSINFO_PREF_MUSIC:
    case SYSINFO_PREF_LINKS:
    case SYSINFO_MPEG:
    case SYSINFO_MPEG1:
    case SYSINFO_MPEG2:
    case SYSINFO_MPEG3:
    case SYSINFO_LINKS_HTTP:
    case SYSINFO_LINKS_FTP:
    case SYSINFO_LINKS_NEWS:
    case SYSINFO_LINKS_MAILTO:
    case SYSINFO_LINKS_TELNET:
    case SYSINFO_PNG_TRANS:
    case SYSINFO_PNG_ALPHA:
    case SYSINFO_OGG:
    case SYSINFO_MNG:
    case SYSINFO_MNG_TRANS:
    case SYSINFO_MNG_ALPHA:
    case SYSINFO_TEXT_COLORS:
    case SYSINFO_TEXT_HILITE:
    case SYSINFO_BANNERS:
    case SYSINFO_INTERP_CLASS:
        /* there are no additional arguments for these information types */
        check_argc(vmg_ argc, 1);

        /* ask the OS layer for the information */
        if (os_get_sysinfo(info, 0, &result))
        {
            /* we got a valid result - return it */
            retval_int(vmg_ result);
        }
        else
        {
            /* 
             *   the information type is not known to the OS layer -
             *   return nil to indicate that the information isn't
             *   available 
             */
            retval_nil(vmg0_);
        }
        break;

    case SYSINFO_HTML_MODE:
        /*
         *   This sysinfo flag is explicitly not used in TADS 3, since we're
         *   always in HTML mode.  (We make this case explicit to call
         *   attention to the fact that it was not accidentally omitted, but
         *   is intentionally not used.)  
         */
        /* fall through to default case */
        
    default:
        /*
         *   Other codes fail harmlessly with a nil return value.  Pop all
         *   remaining arguments and return nil.  (Note that we discard
         *   only (argc-1) arguments because we've already popped the
         *   first argument.)  
         */
        G_stk->discard(argc - 1);
        retval_nil(vmg0_);
        break;
    }
}

/* ------------------------------------------------------------------------ */
/*
 *   status_mode - set the status line mode
 */
void CVmBifTIO::status_mode(VMG_ uint argc)
{
    int mode;
    
    /* check arguments */
    check_argc(vmg_ argc, 1);

    /* pop the mode value */
    mode = pop_int_val(vmg0_);

    /* set the new status mode in the console */
    G_console->set_statusline_mode(vmg_ mode);
}

/* ------------------------------------------------------------------------ */
/*
 *   status_right - set the string in the right half of the status line 
 */
void CVmBifTIO::status_right(VMG_ uint argc)
{
    char msg[256];

    /* check arguments */
    check_argc(vmg_ argc, 1);

    /* pop the status string */
    pop_str_val_ui(vmg_ msg, sizeof(msg));

    /* set the string */
    os_strsc(msg);
}

/* ------------------------------------------------------------------------ */
/*
 *   res_exists - check to see if an external resource can be loaded
 *   through the host application 
 */
void CVmBifTIO::res_exists(VMG_ uint argc)
{
    const char *res_name;
    int result;

    /* check arguments */
    check_argc(vmg_ argc, 1);

    /* pop the resource name */
    res_name = pop_str_val(vmg0_);

    /* ask the host application if the resource can be loaded */
    result = G_host_ifc->resfile_exists(res_name + VMB_LEN, osrp2(res_name));

    /* return the result */
    retval_bool(vmg_ result);
}

/* ------------------------------------------------------------------------ */
/*
 *   set_script_file flags 
 */

/* read in 'quiet' mode - do not dipslay output while reading the script */
#define VMBIFTADS_SCRIPT_QUIET     0x0001

/* turn off 'more' mode while reading the script */
#define VMBIFTADS_SCRIPT_NONSTOP   0x0002

/*
 *   set_script_file - open a command input scripting file 
 */
void CVmBifTIO::set_script_file(VMG_ uint argc)
{
    char fname[OSFNMAX];
    int flags;

    /* check arguments */
    check_argc_range(vmg_ argc, 1, 2);

    /* if the filename is nil, close the current script file */
    if (G_stk->get(0)->typ == VM_NIL)
    {
        int old_more_mode;
        
        /* discard the nil filename */
        G_stk->discard();
        
        /* pop the flags if present - they're superfluous in this case */
        if (argc >= 2)
            G_stk->discard();

        /* close the script file */
        old_more_mode = G_console->close_script_file();

        /* restore the MORE mode in effect when the script was opened */
        G_console->set_more_state(old_more_mode);
    }
    else
    {
        /* 
         *   get the filename string (converted to the file system
         *   character set) 
         */
        pop_str_val_fname(vmg_ fname, sizeof(fname));
    
        /* if they provided flags, pop the flags */
        flags = 0;
        if (argc >= 2)
            flags = pop_int_val(vmg0_);

        /* open the script file */
        G_console->open_script_file(fname,
                                    (flags & VMBIFTADS_SCRIPT_QUIET) != 0,
                                    !(flags & VMBIFTADS_SCRIPT_NONSTOP));
    }
}

/* ------------------------------------------------------------------------ */
/*
 *   get_charset selectors 
 */

/* display character set */
#define VMBIFTADS_CHARSET_DISPLAY  0x0001

/* file system character set for filenames */
#define VMBIFTADS_CHARSET_FILENAME 0x0002

/* typical character set for text file contents */
#define VMBIFTADS_CHARSET_FILECONTENTS 0x0003

/*
 *   get_charset - get a local character set name 
 */
void CVmBifTIO::get_charset(VMG_ uint argc)
{
    char csname[32];
    int which;

    /* check arguments */
    check_argc(vmg_ argc, 1);

    /* get the character set selector */
    which = pop_int_val(vmg0_);

    /* map the selector to the appropriate value */
    switch(which)
    {
    case VMBIFTADS_CHARSET_DISPLAY:
        /* 
         *   if there was an explicit character set parameter specified at
         *   start-up time, use that 
         */
        if (G_disp_cset_name != 0)
        {
            /* there's an explicit parameter - return it */
            retval_str(vmg_ G_disp_cset_name);

            /* we're done */
            return;
        }

        /* no explicit setting - use the OS default character set */
        which = OS_CHARMAP_DISPLAY;
        break;

    case VMBIFTADS_CHARSET_FILENAME:
        which = OS_CHARMAP_FILENAME;
        break;

    case VMBIFTADS_CHARSET_FILECONTENTS:
        which = OS_CHARMAP_FILECONTENTS;
        break;

    default:
        /* others are unrecognized; simply return nil for these */
        retval_nil(vmg0_);
        return;
    }

    /* get the character set */
    os_get_charmap(csname, which);

    /* return a string with the name */
    retval_str(vmg_ csname);
}

/* ------------------------------------------------------------------------ */
/*
 *   flush_output - flush the display output
 */
void CVmBifTIO::flush_output(VMG_ uint argc)
{
    /* we take no arguments */
    check_argc(vmg_ argc, 0);

    /* flush output */
    G_console->flush(vmg_ VM_NL_NONE);

    /* immediately update the display */
    G_console->update_display(vmg0_);
}

/* ------------------------------------------------------------------------ */
/*
 *   input_timeout - get a line of input from the keyboard, with an optional
 *   timeout 
 */
void CVmBifTIO::input_timeout(VMG_ uint argc)
{
    char buf[256];
    long timeout;
    int use_timeout;
    int evt;
    int ele_count;
    vm_obj_id_t lst_obj;
    CVmObjList *lst;
    vm_val_t val;

    /* check arguments */
    check_argc_range(vmg_ argc, 0, 1);

    /* if there's a timeout argument, retrieve it */
    if (argc == 0)
    {
        /* no arguments - there's no timeout */
        use_timeout = FALSE;
        timeout = 0;
    }
    else if (G_stk->get(0)->typ == VM_NIL)
    {
        /* 
         *   there's a timeout argument, but it's nil, so this means there's
         *   no timeout
         */
        use_timeout = FALSE;
        timeout = 0;

        /* discard the argument */
        G_stk->discard();
    }
    else
    {
        /* we have a timeout specified */
        use_timeout = TRUE;
        timeout = pop_long_val(vmg0_);
    }

    /* read a line of text from the keyboard */
    evt = G_console->read_line_timeout(vmg_ buf, sizeof(buf),
                                       timeout, use_timeout);

    /* figure out how big a list we'll return */
    switch(evt)
    {
    case OS_EVT_LINE:
    case OS_EVT_TIMEOUT:
        /* two elements - the event type, and the line of text */
        ele_count = 2;
        break;

    default:
        /* for anything else, we need only the event type code */
        ele_count = 1;
        break;
    }

    /* create the return list */
    lst_obj = CVmObjList::create(vmg_ FALSE, ele_count);
    lst = (CVmObjList *)vm_objp(vmg_ lst_obj);

    /* save the list on the stack to protect against garbage collection */
    val.set_obj(lst_obj);
    G_stk->push(&val);

    /* fill in the first element with the event type code */
    val.set_int(evt);
    lst->cons_set_element(0, &val);

    /* set additional elements, according to the event type */
    switch(evt)
    {
    case OS_EVT_LINE:
    case OS_EVT_TIMEOUT:
        /* the second element is the line of text we read */
        val.set_obj(CVmObjString::create(vmg_ FALSE, buf, strlen(buf)));
        lst->cons_set_element(1, &val);
        break;

    default:
        /* other event types have no extra data */
        break;
    }

    /* return the list */
    retval_obj(vmg_ lst_obj);

    /* we can drop the garbage collection protection now */
    G_stk->discard();
}

/* ------------------------------------------------------------------------ */
/*
 *   input_cancel - cancel input previously interrupted by timeout
 */
void CVmBifTIO::input_cancel(VMG_ uint argc)
{
    vm_val_t val;
    int reset;
    
    /* check arguments */
    check_argc(vmg_ argc, 1);

    /* get the 'reset' flag */
    G_stk->pop(&val);
    reset = (val.typ == VM_TRUE);

    /* cancel the line */
    G_console->read_line_cancel(vmg_ reset);
}

/* ------------------------------------------------------------------------ */
/*
 *   Banner Window Functions 
 */

/*
 *   create a banner 
 */
void CVmBifTIO::banner_create(VMG_ uint argc)
{
    int parent_id;
    int other_id;
    int where;
    int wintype;
    int align;
    int siz;
    int siz_units;
    unsigned long style;
    int hdl;

    /* check arguments */
    check_argc_range(vmg_ argc, 7, 8);

    /* retrieve the 'parent' parameter */
    if (argc == 7)
    {
        /* 
         *   there's no parent argument - this is an obsolete format for the
         *   arguments, but accept it for now, and simply treat it as
         *   equivalent to a nil parent 
         */
        parent_id = 0;
    }
    else if (G_stk->get(0)->typ == VM_NIL)
    {
        /* parent is nil - use zero as the ID and discard the nil */
        parent_id = 0;
        G_stk->discard();
    }
    else
    {
        /* retrieve the parent ID */
        parent_id = pop_int_val(vmg0_);
    }

    /* retrieve the 'where' parameter */
    where = pop_int_val(vmg0_);

    /* retrieve the 'other' parameter, if it's needed for the 'where' */
    switch(where)
    {
    case OS_BANNER_BEFORE:
    case OS_BANNER_AFTER:
        /* we need another banner ID for the relative insertion point */
        other_id = pop_int_val(vmg0_);
        break;

    default:
        /* we don't need 'other' for this insertion point */
        other_id = 0;
        G_stk->discard();
        break;
    }

    /* retrieve the window type argument */
    wintype = pop_int_val(vmg0_);

    /* retrieve the alignment argument */
    align = pop_int_val(vmg0_);

    /* retrieve the size (as a percentage of the full screen size) */
    if (G_stk->get(0)->typ == VM_NIL)
    {
        /* nil size - use zero for the size */
        siz = 0;
        siz_units = OS_BANNER_SIZE_ABS;

        /* discard the size and size units */
        G_stk->discard();
        G_stk->discard();
    }
    else
    {
        /* retrieve the size and size units as integer values */
        siz = pop_int_val(vmg0_);
        siz_units = pop_int_val(vmg0_);
    }

    /* retrieve the flags */
    style = pop_long_val(vmg0_);

    /* try creating the banner */
    hdl = G_console->get_banner_manager()->create_banner(
        vmg_ parent_id, where, other_id, wintype,
        align, siz, siz_units, style);

    /* 
     *   If we succeeded, return the handle; otherwise, return nil.  A banner
     *   handle of zero indicates failure. 
     */
    if (hdl != 0)
        retval_int(vmg_ hdl);
    else
        retval_nil(vmg0_);
}

/*
 *   delete a banner 
 */
void CVmBifTIO::banner_delete(VMG_ uint argc)
{
    /* check arguments */
    check_argc(vmg_ argc, 1);

    /* delete the banner */
    G_console->get_banner_manager()->delete_banner(pop_int_val(vmg0_));
}

/*
 *   clear a window 
 */
void CVmBifTIO::banner_clear(VMG_ uint argc)
{
    int id;
    CVmConsole *console;

    /* check arguments */
    check_argc(vmg_ argc, 1);

    /* get the banner ID */
    id = pop_int_val(vmg0_);

    /* get the banner - if it's invalid, throw an error */
    console = G_console->get_banner_manager()->get_console(id);
    if (console == 0)
        err_throw(VMERR_BAD_VAL_BIF);

    /* tell the console that we're clearing it */
    console->clear_window(vmg0_);
}

/*
 *   write values to a banner 
 */
void CVmBifTIO::banner_say(VMG_ uint argc)
{
    CVmConsole *console;

    /* check arguments */
    check_argc_range(vmg_ argc, 1, 32767);

    /* get the banner - if it's invalid, throw an error */
    console =
        G_console->get_banner_manager()->get_console(pop_int_val(vmg0_));
    if (console == 0)
        err_throw(VMERR_BAD_VAL_BIF);

    /* 
     *   write the argument(s) to the console (note that the first argument,
     *   which we've already retrieved, is the console handle, so don't count
     *   it among the arguments to display) 
     */
    say_to_console(vmg_ console, argc - 1);
}

/*
 *   flush text to a banner 
 */
void CVmBifTIO::banner_flush(VMG_ uint argc)
{
    CVmConsole *console;

    /* check arguments */
    check_argc(vmg_ argc, 1);

    /* get the banner - if it's invalid, throw an error */
    console =
        G_console->get_banner_manager()->get_console(pop_int_val(vmg0_));
    if (console == 0)
        err_throw(VMERR_BAD_VAL_BIF);

    /* flush the console */
    console->flush(vmg_ VM_NL_NONE);

    /* immediately update the display */
    console->update_display(vmg0_);
}

/*
 *   set the banner size 
 */
void CVmBifTIO::banner_set_size(VMG_ uint argc)
{
    int id;
    void *hdl;
    int siz;
    int siz_units;
    int is_advisory;

    /* check arguments */
    check_argc(vmg_ argc, 4);

    /* get the banner ID */
    id = pop_int_val(vmg0_);

    /* get the size and size units */
    siz = pop_int_val(vmg0_);
    siz_units = pop_int_val(vmg0_);

    /* get the is-advisory flag */
    is_advisory = pop_bool_val(vmg0_);

    /* get the banner - if it's invalid, throw an error */
    hdl = G_console->get_banner_manager()->get_os_handle(id);
    if (hdl == 0)
        err_throw(VMERR_BAD_VAL_BIF);

    /* set the size */
    os_banner_set_size(hdl, siz, siz_units, is_advisory);
}

/*
 *   size a banner to its contents in one or both dimensions 
 */
void CVmBifTIO::banner_size_to_contents(VMG_ uint argc)
{
    int id;
    void *hdl;
    CVmConsole *console;

    /* check arguments */
    check_argc(vmg_ argc, 1);

    /* get the banner ID */
    id = pop_int_val(vmg0_);

    /* get the banner - if it's invalid, throw an error */
    hdl = G_console->get_banner_manager()->get_os_handle(id);
    console = G_console->get_banner_manager()->get_console(id);
    if (hdl == 0 || console == 0)
        err_throw(VMERR_BAD_VAL_BIF);

    /* make sure we've flushed any pending output to the banner */
    console->flush(vmg_ VM_NL_NONE);

    /* set the size */
    os_banner_size_to_contents(hdl);
}

/*
 *   move the output position in a text grid banner
 */
void CVmBifTIO::banner_goto(VMG_ uint argc)
{
    int id;
    void *hdl;
    CVmConsole *console;
    int row, col;

    /* check arguments */
    check_argc(vmg_ argc, 3);

    /* get the banner ID */
    id = pop_int_val(vmg0_);

    /* get the coordinates */
    row = pop_int_val(vmg0_);
    col = pop_int_val(vmg0_);

    /* make sure the values are valid */
    if (row < 1 || col < 1)
        err_throw(VMERR_BAD_VAL_BIF);

    /* get the banner - if it's invalid, throw an error */
    hdl = G_console->get_banner_manager()->get_os_handle(id);
    console = G_console->get_banner_manager()->get_console(id);
    if (hdl == 0 || console == 0)
        err_throw(VMERR_BAD_VAL_BIF);

    /* 
     *   make sure we've flushed and discarded any pending output (since we
     *   don't want any pending output to show up at the new cursor position)
     */
    console->flush(vmg_ VM_NL_NONE);
    console->empty_buffers(vmg0_);

    /* move the cursor, adjusting from 1-based to 0-based coordinates */
    os_banner_goto(hdl, row - 1, col - 1);
}

/*
 *   set the text color in a banner 
 */
void CVmBifTIO::banner_set_text_color(VMG_ uint argc)
{
    int id;
    void *hdl;
    CVmConsole *console;
    os_color_t fg, bg;

    /* check arguments */
    check_argc(vmg_ argc, 3);

    /* get the banner ID */
    id = pop_int_val(vmg0_);

    /* get the foreground and background color values */
    fg = (os_color_t)pop_long_val(vmg0_);
    bg = (os_color_t)pop_long_val(vmg0_);

    /* get the banner - if it's invalid, throw an error */
    hdl = G_console->get_banner_manager()->get_os_handle(id);
    console = G_console->get_banner_manager()->get_console(id);
    if (hdl == 0 || console == 0)
        err_throw(VMERR_BAD_VAL_BIF);

    /* set the text output color in the console's formatter */
    console->set_text_color(vmg_ fg, bg);
}

/*
 *   set the screen color in a banner 
 */
void CVmBifTIO::banner_set_screen_color(VMG_ uint argc)
{
    int id;
    void *hdl;
    CVmConsole *console;
    os_color_t color;

    /* check arguments */
    check_argc(vmg_ argc, 2);

    /* get the banner ID */
    id = pop_int_val(vmg0_);

    /* get the body color */
    color = (os_color_t)pop_long_val(vmg0_);

    /* get the banner - if it's invalid, throw an error */
    hdl = G_console->get_banner_manager()->get_os_handle(id);
    console = G_console->get_banner_manager()->get_console(id);
    if (hdl == 0 || console == 0)
        err_throw(VMERR_BAD_VAL_BIF);

    /* set the body color in the console */
    console->set_body_color(vmg_ color);
}

/* service routine: store an integer in a list under construction */
static void set_list_int(CVmObjList *lst, size_t idx, long intval)
{
    vm_val_t val;
    
    /* set the value */
    val.set_int(intval);

    /* store it in the list */
    lst->cons_set_element(idx, &val);
}

/*
 *   get information on a banner
 */
void CVmBifTIO::banner_get_info(VMG_ uint argc)
{
    int id;
    void *hdl;
    os_banner_info_t info;
    CVmConsoleBanner *console;

    /* check arguments */
    check_argc(vmg_ argc, 1);

    /* get the banner ID */
    id = pop_int_val(vmg0_);

    /* get the banner - if it's invalid, throw an error */
    hdl = G_console->get_banner_manager()->get_os_handle(id);
    console = G_console->get_banner_manager()->get_console(id);
    if (hdl == 0 || console == 0)
        err_throw(VMERR_BAD_VAL_BIF);

    /* get information on the banner */
    if (console->get_banner_info(&info))
    {
        vm_obj_id_t lst_obj;
        CVmObjList *lst;
        vm_val_t val;
        
        /* set up a return list with space for six entries */
        lst_obj = CVmObjList::create(vmg_ FALSE, 6);
        lst = (CVmObjList *)vm_objp(vmg_ lst_obj);

        /* save the list on the stack to protect against garbage collection */
        val.set_obj(lst_obj);
        G_stk->push(&val);

        /* 
         *   return the values: [align, style, rows, columns, pix_height,
         *   pix_width] 
         */
        set_list_int(lst, 0, info.align);
        set_list_int(lst, 1, info.style);
        set_list_int(lst, 2, info.rows);
        set_list_int(lst, 3, info.columns);
        set_list_int(lst, 4, info.pix_height);
        set_list_int(lst, 5, info.pix_width);

        /* return the list */
        retval_obj(vmg_ lst_obj);

        /* discard our gc protection */
        G_stk->discard();
    }
    else
    {
        /* no information available - return nil */
        retval_nil(vmg0_);
    }
}

/* ------------------------------------------------------------------------ */
/*
 *   Log Console Functions
 */

/*
 *   create a log console 
 */
void CVmBifTIO::log_console_create(VMG_ uint argc)
{
    char fname[OSFNMAX];
    osfildef *fp;
    CCharmapToLocal *cmap;
    int width;
    int hdl;
    
    /* check arguments */
    check_argc(vmg_ argc, 3);

    /* retrieve the log file name */
    pop_str_val_fname(vmg_ fname, sizeof(fname));

    /* 
     *   Retrieve the character mapper, which can be given as either a
     *   CharacterSet object or a string giving the character set name.  
     */
    if (G_stk->get(0)->typ == VM_OBJ
        && CVmObjCharSet::is_charset(vmg_ G_stk->get(0)->val.obj))
    {
        vm_obj_id_t obj;
        
        /* retrieve the CharacterSet reference */
        obj = CVmBif::pop_obj_val(vmg0_);

        /* retrieve the character mapper from the character set */
        cmap = ((CVmObjCharSet *)vm_objp(vmg_ obj))->get_to_local(vmg0_);

        /* add our reference to it */
        cmap->add_ref();
    }
    else
    {
        const char *str;
        size_t len;
        char *nm;

        /* it's not a CharacterSet, so it must be a character set name */
        str = G_stk->get(0)->get_as_string(vmg0_);
        if (str == 0)
            err_throw(VMERR_BAD_TYPE_BIF);
        
        /* get the length and skip the length prefix */
        len = vmb_get_len(str);
        str += VMB_LEN;

        /* get a null-terminated version of the name */
        nm = lib_copy_str(str, len);

        /* 
         *   Create a character mapping for the given name.  Note that this
         *   will automatically add a reference to the mapper on our behalf,
         *   so we don't have to add our own extra reference. 
         */
        cmap = CCharmapToLocal::load(G_host_ifc->get_cmap_res_loader(), nm);

        /* done with the null-terminated version of the name string */
        lib_free_str(nm);

        /* discard the argument */
        G_stk->discard();
    }

    /* if we didn't get a character map, use us-ascii by default */
    if (cmap == 0)
        cmap = CCharmapToLocal::load(G_host_ifc->get_cmap_res_loader(),
                                     "us-ascii");

    err_try
    {
        /* make sure the file safety level allows the operation */
        CVmObjFile::check_safety_for_open(vmg_ fname, VMOBJFILE_ACCESS_WRITE);
        
        /* open the file for writing (in text mode) */
        fp = osfopwt(fname, OSFTLOG);
    
        /* if that failed, we can't contine */
        if (fp == 0)
            G_interpreter->throw_new_class(vmg_ G_predef->file_creation_exc,
                                           0, "error creating log file");

        /* retrieve the width */
        width = pop_int_val(vmg0_);

        /* create the log console */
        hdl = G_console->get_log_console_manager()->create_log_console(
            fname, fp, cmap, width);
    }
    err_finally
    {
        /* 
         *   release our character map reference - if we succeeded in
         *   creating the log console, it will have added its own reference
         *   by now 
         */
        cmap->release_ref();
    }
    err_end;

    /* 
     *   If we succeeded, return the handle; otherwise, return nil.  A handle
     *   of zero indicates failure.  
     */
    if (hdl != 0)
        retval_int(vmg_ hdl);
    else
        retval_nil(vmg0_);
}

/*
 *   close (delete) a log console
 */
void CVmBifTIO::log_console_close(VMG_ uint argc)
{
    int handle;
    CVmConsole *console;
    
    /* check arguments */
    check_argc(vmg_ argc, 1);

    /* get the handle */
    handle = pop_int_val(vmg0_);

    /* get the console based on the handle */
    console = G_console->get_log_console_manager()->get_console(handle);
    if (console == 0)
        err_throw(VMERR_BAD_VAL_BIF);

    /* flush the console */
    console->flush(vmg_ VM_NL_NONE);

    /* delete the console */
    G_console->get_log_console_manager()->delete_log_console(handle);
}

/*
 *   write values to a log console
 */
void CVmBifTIO::log_console_say(VMG_ uint argc)
{
    CVmConsole *console;

    /* check arguments */
    check_argc_range(vmg_ argc, 1, 32767);

    /* get the banner - if it's invalid, throw an error */
    console = G_console->get_log_console_manager()
              ->get_console(pop_int_val(vmg0_));
    if (console == 0)
        err_throw(VMERR_BAD_VAL_BIF);

    /*
     *   write the argument(s) to the console (note that the first argument,
     *   which we've already retrieved, is the console handle, so don't count
     *   it among the arguments to display) 
     */
    say_to_console(vmg_ console, argc - 1);
}

