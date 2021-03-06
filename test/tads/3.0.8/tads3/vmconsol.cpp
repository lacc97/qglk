#ifdef RCSID
static char RCSid[] =
"$Header$";
#endif

/* 
 *   Copyright (c) 1987, 2002 Michael J. Roberts.  All Rights Reserved.
 *   
 *   Please see the accompanying license file, LICENSE.TXT, for information
 *   on using and copying this software.  
 */
/*
Name
  vmconsol.cpp - TADS 3 console input reader and output formatter
Function
  Provides console input and output for the TADS 3 built-in function set
  for the T3 VM, including the output formatter.

  T3 uses the UTF-8 character set to represent character strings.  The OS
  functions use the local character set.  We perform the mapping between
  UTF-8 and the local character set within this module, so that OS routines
  see local characters only, not UTF-8.

  This code is based on the TADS 2 output formatter, but has been
  substantially reworked for C++, Unicode, and the slightly different
  TADS 3 formatting model.
Notes

Returns
  None
Modified
  08/25/99 MJRoberts  - created from TADS 2 output formatter
*/

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "wchar.h"

#include "os.h"
#include "t3std.h"
#include "utf8.h"
#include "charmap.h"
#include "vmuni.h"
#include "vmconsol.h"
#include "vmglob.h"
#include "vmhash.h"


/* ------------------------------------------------------------------------ */
/*
 *   Log-file formatter subclass implementation 
 */

/*
 *   delete 
 */
CVmFormatterLog::~CVmFormatterLog()
{
    /* close any active log file */
    close_log_file();
}

/*
 *   Open a new log file 
 */
int CVmFormatterLog::open_log_file(const char *fname)
{
    /* close any existing log file */
    if (close_log_file())
        return 1;

    /* reinitialize */
    init();

    /* save the filename for later (we'll need it when we close the file) */
    logfname_ = lib_copy_str(fname);

    /* open the new file */
    logfp_ = osfopwt(fname, OSFTLOG);

    /* return success if we successfully opened the file, failure otherwise */
    return (logfp_ == 0);
}

/*
 *   Set the log file to a file opened by the caller 
 */
int CVmFormatterLog::set_log_file(const char *fname, osfildef *fp)
{
    /* close any existing log file */
    if (close_log_file())
        return 1;

    /* reinitialize */
    init();

    /* remember the file */
    logfp_ = fp;

    /* remember the filename */
    logfname_ = lib_copy_str(fname);

    /* success */
    return 0;
}

/*
 *   Close the log file 
 */
int CVmFormatterLog::close_log_file()
{
    /* if we have a file, close it */
    if (logfp_ != 0)
    {
        /* close the handle */
        osfcls(logfp_);

        /* forget about our log file handle */
        logfp_ = 0;

        /* set the system file type to "log file" */
        if (logfname_ != 0)
            os_settype(logfname_, OSFTLOG);
    }

    /* forget the log file name, if we have one */
    if (logfname_ != 0)
    {
        lib_free_str(logfname_);
        logfname_ = 0;
    }

    /* success */
    return 0;
}


/* ------------------------------------------------------------------------ */
/*
 *   Base Formatter 
 */

/*
 *   deletion
 */
CVmFormatter::~CVmFormatter()
{
    /* if we have a table of horizontal tabs, delete it */
    if (tabs_ != 0)
        delete tabs_;

    /* forget the character mapper */
    set_charmap(0);
}

/*
 *   set a new character mapper 
 */
void CVmFormatter::set_charmap(CCharmapToLocal *cmap)
{
    /* add a reference to the new mapper, if we have one */
    if (cmap != 0)
        cmap->add_ref();

    /* release our reference on any old mapper */
    if (cmap_ != 0)
        cmap_->release_ref();

    /* remember the new mapper */
    cmap_ = cmap;
}

/*
 *   Write out a line.  Text we receive is in the UTF-8 character set.
 */
void CVmFormatter::write_text(VMG_ const wchar_t *txt, size_t cnt,
                              const vmcon_color_t *colors, vm_nl_type nl)
{
    /* 
     *   Check the "script quiet" mode - this indicates that we're reading
     *   a script and not echoing output to the display.  If this mode is
     *   on, and we're writing to the display, suppress this write.  If
     *   the mode is off, or we're writing to a non-display stream (such
     *   as a log file stream), show the output as normal.  
     */
    if (!console_->is_quiet_script() || !is_disp_stream_)
    {
        char local_buf[128];
        char *dst;
        size_t rem;

        /*
         *   Check to see if we've reached the end of the screen, and if so
         *   run the MORE prompt.  Note that we don't show a MORE prompt
         *   unless we're in "formatter more mode," since if we're not, then
         *   the OS layer code is taking responsibility for pagination
         *   issues.  We also don't display a MORE prompt when reading from a
         *   script file.
         *   
         *   Note that we suppress the MORE prompt if we're showing a
         *   continuation of a line already partially shown.  We only want to
         *   show a MORE prompt at the start of a new line.
         *   
         *   Skip the MORE prompt if this stream doesn't use it.  
         */
        if (formatter_more_mode()
            && !console_->is_reading_script()
            && console_->is_more_mode()
            && !is_continuation_
            && linecnt_ + 1 >= console_->get_page_length())
        {
            /* set the standard text color */
            set_os_text_color(OS_COLOR_P_TEXT, OS_COLOR_P_TEXTBG);
            set_os_text_attr(0);

            /* display the MORE prompt */
            console_->show_more_prompt(vmg0_);

            /* restore the current color scheme */
            set_os_text_color(os_color_.fg, os_color_.bg);
            set_os_text_attr(os_color_.attr);
        }

        /* count the line if a newline follows */
        if (nl != VM_NL_NONE && nl != VM_NL_NONE_INTERNAL)
            ++linecnt_;

        /* convert and display the text */
        for (dst = local_buf, rem = sizeof(local_buf) - 1 ; cnt != 0 ; )
        {
            size_t cur;
            size_t old_rem;
            wchar_t c;
            
            /* 
             *   if this character is in a new color, write out the OS-level
             *   color switch code 
             */
            if (colors != 0 && !colors->equals(&os_color_))
            {
                /* 
                 *   null-terminate and display what's in the buffer so far,
                 *   so that we close out all of the remaining text in the
                 *   old color and attributes
                 */
                *dst = '\0';
                print_to_os(local_buf);

                /* reset to the start of the local output buffer */
                dst = local_buf;
                rem = sizeof(local_buf) - 1;

                /* set the text attributes, if they changed */
                if (colors->attr != os_color_.attr)
                    set_os_text_attr(colors->attr);

                /* set the color, if it changed */
                if (colors->fg != os_color_.fg
                    || colors->bg != os_color_.bg)
                    set_os_text_color(colors->fg, colors->bg);

                /* 
                 *   Whatever happened, set our new color internally as the
                 *   last color we sent to the OS.  Even if we didn't
                 *   actually do anything, we'll at least know we won't have
                 *   to do anything more until we find another new color. 
                 */
                os_color_ = *colors;
            }

            /* get this character */
            c = *txt;

            /* 
             *   translate non-breaking spaces into ordinary spaces if the
             *   underlying target isn't HTML-based 
             */
            if (!html_target_ && c == 0x00A0)
                c = ' ';

            /* try storing another character */
            old_rem = rem;
            cur = (cmap_ != 0 ? cmap_ : G_cmap_to_ui)->map(c, &dst, &rem);

            /* if that failed, flush the buffer and try again */
            if (cur > old_rem)
            {
                /* null-terminate the buffer */
                *dst = '\0';
                
                /* display the text */
                print_to_os(local_buf);

                /* reset to the start of the local output buffer */
                dst = local_buf;
                rem = sizeof(local_buf) - 1;
            }
            else
            {
                /* we've now consumed this character of input */
                ++txt;
                --cnt;
                if (colors != 0)
                    ++colors;
            }
        }

        /* if we have a partially-filled buffer, display it */
        if (dst > local_buf)
        {
            /* null-terminate and display the buffer */
            *dst = '\0';
            print_to_os(local_buf);
        }

        /* write the appropriate type of line termination */
        switch(nl)
        {
        case VM_NL_NONE:
        case VM_NL_INPUT:
        case VM_NL_NONE_INTERNAL:
            /* no line termination is needed */
            break;

        case VM_NL_NEWLINE:
            /* write a newline */
            print_to_os(html_target_ ? "<BR HEIGHT=0>\n" : "\n");
            break;

        case VM_NL_OSNEWLINE:
            /* 
             *   the OS will provide a newline, but add a space to make it
             *   explicit that we can break the line here 
             */
            print_to_os(" ");
            break;
        }
    }
}

/* ------------------------------------------------------------------------ */
/*
 *   Flush the current line to the display, using the given type of line
 *   termination.
 *   
 *   VM_NL_NONE: flush the current line but do not start a new line; more
 *   text will follow on the current line.  This is used, for example, to
 *   flush text after displaying a prompt and before waiting for user
 *   input.
 *   
 *   VM_NL_INPUT: acts like VM_NL_NONE, except that we flush everything,
 *   including trailing spaces.
 *   
 *   VM_NL_NONE_INTERNAL: same as VM_NL_NONE, but doesn't flush at the OS
 *   level.  This is used when we're only flushing our buffers in order to
 *   clear out space internally, not because we want the underlying OS
 *   renderer to display things immediately.  This distinction is
 *   important in HTML mode, since it ensures that the HTML parser only
 *   sees well-formed strings when flushing.
 *   
 *   VM_NL_NEWLINE: flush the line and start a new line by writing out a
 *   newline character.
 *   
 *   VM_NL_OSNEWLINE: flush the line as though starting a new line, but
 *   don't add an actual newline character to the output, since the
 *   underlying OS display code will handle this.  Instead, add a space
 *   after the line to indicate to the OS code that a line break is
 *   possible there.  (This differs from VM_NL_NONE in that VM_NL_NONE
 *   doesn't add anything at all after the line.)  
 */
void CVmFormatter::flush(VMG_ vm_nl_type nl)
{
    int cnt;
    vm_nl_type write_nl;

    /* null-terminate the current output line buffer */
    linebuf_[linepos_] = '\0';

    /* 
     *   Expand any pending tab.  Allow "anonymous" tabs only if we're
     *   flushing because we're ending the line normally; if we're not
     *   ending the line, we can't handle tabs that depend on the line
     *   ending. 
     */
    expand_pending_tab(vmg_ nl == VM_NL_NEWLINE);

    /* 
     *   note number of characters to display - assume we'll display all of
     *   the characters in the buffer 
     */
    cnt = wcslen(linebuf_);

    /* 
     *   Trim trailing spaces, unless we're about to read input or are doing
     *   an internal flush.  (Show trailing spaces when reading input, since
     *   we won't be able to revise the layout after this point.  Don't trim
     *   on an internal flush either, as this kind of flushing simply empties
     *   out our buffer exactly as it is.)  
     */
    if (nl != VM_NL_INPUT && nl != VM_NL_NONE_INTERNAL)
    {
        /* 
         *   look for last non-space character, but keep any spaces that come
         *   before an explicit non-breaking flag 
         */
        for ( ; cnt > 0 && linebuf_[cnt-1] == ' ' ; --cnt)
        {
            /* don't remove this character if it's marked as non-breaking */
            if ((flagbuf_[cnt-1] & VMCON_OBF_NOBREAK) != 0)
                break;
        }

        /* 
         *   if we're actually doing a newline, discard the trailing spaces
         *   for good - we don't want them at the start of the next line 
         */
        if (nl == VM_NL_NEWLINE)
            linepos_ = cnt;
    }

    /* check the newline mode */
    switch(nl)
    {
    case VM_NL_NONE:
    case VM_NL_NONE_INTERNAL:
        /* no newline - just flush out what we have */
        write_nl = VM_NL_NONE;
        break;

    case VM_NL_INPUT:
        /* no newline - flush out what we have */
        write_nl = VM_NL_NONE;

        /* on input, reset the HTML parsing state */
        html_passthru_state_ = VMCON_HPS_NORMAL;
        break;

    case VM_NL_NEWLINE:
        /* 
         *   We're adding a newline.  We want to suppress redundant
         *   newlines -- we reduce any run of consecutive vertical
         *   whitespace to a single newline.  So, if we have anything in
         *   this line, or we didn't already just write a newline, write
         *   out a newline now; otherwise, write nothing.  
         */
        if (linecol_ != 0 || !just_did_nl_)
        {
            /* add the newline */
            write_nl = VM_NL_NEWLINE;
        }
        else
        {
            /* 
             *   Don't write out a newline after all - the line buffer is
             *   empty, and we just wrote a newline, so this is a
             *   redundant newline that we wish to suppress (so that we
             *   collapse a run of vertical whitespace down to a single
             *   newline).  
             */
            write_nl = VM_NL_NONE;
        }
        break;

    case VM_NL_OSNEWLINE:
        /* 
         *   we're going to depend on the underlying OS output layer to do
         *   line breaking, so we won't add a newline, but we will add a
         *   space, so that the underlying OS layer knows we have a word
         *   break here 
         */
        write_nl = VM_NL_OSNEWLINE;
        break;
    }

    /* 
     *   display the line, as long as we have something buffered to
     *   display; even if we don't, display it if our column is non-zero
     *   and we didn't just do a newline, since this must mean that we've
     *   flushed a partial line and are just now doing the newline 
     */
    if (cnt != 0 || (linecol_ != 0 && !just_did_nl_))
    {
        /* write it out */
        write_text(vmg_ linebuf_, cnt, colorbuf_, write_nl);
    }

    /* check the line ending */
    switch (nl)
    {
    case VM_NL_NONE:
    case VM_NL_INPUT:
        /* we're not displaying a newline, so flush what we have */
        flush_to_os();

        /* 
         *   the subsequent buffer will be a continuation of the current
         *   text, if we've displayed anything at all here 
         */
        is_continuation_ = (linecol_ != 0);
        break;

    case VM_NL_NONE_INTERNAL:
        /* 
         *   internal buffer flush only - subsequent text will be a
         *   continuation of the current line, if there's anything on the
         *   current line 
         */
        is_continuation_ = (linecol_ != 0);
        break;

    default:
        /* we displayed a newline, so reset the column position */
        linecol_ = 0;

        /* the next buffer starts a new line on the display */
        is_continuation_ = FALSE;
        break;
    }

    /* 
     *   Move any trailing characters we didn't write in this go to the start
     *   of the buffer.  
     */
    if (cnt < linepos_)
    {
        size_t movecnt;

        /* calculate how many trailing characters we didn't write */
        movecnt = linepos_ - cnt;

        /* move the characters, colors, and flags */
        memmove(linebuf_, linebuf_ + cnt, movecnt * sizeof(linebuf_[0]));
        memmove(colorbuf_, colorbuf_ + cnt, movecnt * sizeof(colorbuf_[0]));
        memmove(flagbuf_, flagbuf_ + cnt, movecnt * sizeof(flagbuf_[0]));
    }

    /* move the line output position to follow the preserved characters */
    linepos_ -= cnt;

    /* 
     *   If we just output a newline, note it.  If we didn't just output a
     *   newline, but we did write out anything else, note that we're no
     *   longer at the start of a line on the underlying output device.  
     */
    if (nl == VM_NL_NEWLINE)
        just_did_nl_ = TRUE;
    else if (cnt != 0)
        just_did_nl_ = FALSE;

    /* 
     *   if the current buffering color doesn't match the current osifc-layer
     *   color, then we must need to flush just the new color/attribute
     *   settings (this can happen when we have changed the attributes in
     *   preparation for reading input, since we won't have any actual text
     *   to write after the color change) 
     */
    if (!cur_color_.equals(&os_color_))
    {
        /* set the text attributes in the OS window, if they changed */
        if (cur_color_.attr != os_color_.attr)
            set_os_text_attr(cur_color_.attr);

        /* set the color in the OS window, if it changed */
        if (cur_color_.fg != os_color_.fg
            || cur_color_.bg != os_color_.bg)
            set_os_text_color(cur_color_.fg, cur_color_.bg);

        /* set the new osifc color */
        os_color_ = cur_color_;
    }
}

/* ------------------------------------------------------------------------ */
/*
 *   Clear out our buffers 
 */
void CVmFormatter::empty_buffers(VMG0_)
{
    /* reset our buffer pointers */
    linepos_ = 0;
    linecol_ = 0;
    linebuf_[0] = '\0';
    just_did_nl_ = FALSE;
    is_continuation_ = FALSE;

    /* there's no pending tab now */
    pending_tab_align_ = VMFMT_TAB_NONE;

    /* start out at the first line */
    linecnt_ = 0;

    /* reset the HTML lexical state */
    html_passthru_state_ = VMCON_HPS_NORMAL;
}

/* ------------------------------------------------------------------------ */
/*
 *   Immediately update the display window 
 */
void CVmFormatter::update_display(VMG0_)
{
    /* update the display window at the OS layer */
    os_update_display();
}

/* ------------------------------------------------------------------------ */
/*
 *   Display a blank line to the stream
 */
void CVmFormatter::write_blank_line(VMG0_)
{
    /* flush the stream */
    flush(vmg_ VM_NL_NEWLINE);

    /* if generating for an HTML display target, add an HTML line break */
    if (html_target_)
        write_text(vmg_ L"<BR>", 4, 0, VM_NL_NONE);

    /* write out a blank line */
    write_text(vmg_ L"", 0, 0, VM_NL_NEWLINE);
}

/* ------------------------------------------------------------------------ */
/*
 *   Generate a tab for a "\t" sequence in the game text, or a <TAB
 *   MULTIPLE> or <TAB INDENT> sequence parsed in our mini-parser.
 *   
 *   Standard (non-HTML) version: we'll generate enough spaces to take us to
 *   the next tab stop.
 *   
 *   HTML version: if we're in native HTML mode, we'll just generate the
 *   equivalent HTML; if we're not in HTML mode, we'll generate a hard tab
 *   character, which the HTML formatter will interpret as a <TAB
 *   MULTIPLE=4>.  
 */
void CVmFormatter::write_tab(VMG_ int indent, int multiple)
{
    int maxcol;

    /* check to see what the underlying system is expecting */
    if (html_target_)
    {
        char buf[40];

        /* 
         *   the underlying system is HTML - generate an appropriate <TAB>
         *   sequence to produce the desired effect 
         */
        sprintf(buf, "<TAB %s=%d>",
                indent != 0 ? "INDENT" : "MULTIPLE",
                indent != 0 ? indent : multiple);
            
        /* write it out */
        buffer_string(vmg_ buf);
    }
    else if (multiple != 0)
    {
        /* get the maximum column */
        maxcol = get_buffer_maxcol();

        /*
         *   We don't have an HTML target, and we have a tab to an every-N
         *   stop: expand the tab with spaces.  Keep going until we reach
         *   the next tab stop of the given multiple.  
         */
        do
        {
            /* stop if we've reached the maximum column */
            if (linecol_ >= maxcol)
                break;

            /* add another space */
            linebuf_[linepos_] = ' ';
            flagbuf_[linepos_] = cur_flags_;
            colorbuf_[linepos_] = cur_color_;

            /* advance one character in the buffer */
            ++linepos_;

            /* advance the column counter */
            ++linecol_;
        } while ((linecol_ + 1) % multiple != 0);
    }
    else if (indent != 0)
    {
        /* 
         *   We don't have an HTML target, and we just want to add a given
         *   number of spaces.  Simply write out the given number of spaces,
         *   up to our maximum column limit.  
         */
        for (maxcol = get_buffer_maxcol() ;
             indent != 0 && linecol_ < maxcol ; --indent)
        {
            /* add another space */
            linebuf_[linepos_] = ' ';
            flagbuf_[linepos_] = cur_flags_;
            colorbuf_[linepos_] = cur_color_;

            /* advance one character in the buffer and one column */
            ++linepos_;
            ++linecol_;
        }
    }
}


/* ------------------------------------------------------------------------ */
/*
 *   Flush a line 
 */
void CVmFormatter::flush_line(VMG_ int padding)
{
    /* 
     *   check to see if we're using the underlying display layer's line
     *   wrapping 
     */
    if (os_line_wrap_)
    {
        /*
         *   In the HTML version, we don't need the normal *MORE*
         *   processing, since the HTML layer will handle that.
         *   Furthermore, we don't need to provide actual newline breaks
         *   -- that happens after the HTML is parsed, so we don't have
         *   enough information here to figure out actual line breaks.
         *   So, we'll just flush out our buffer whenever it fills up, and
         *   suppress newlines.
         *   
         *   Similarly, if we have OS-level line wrapping, don't try to
         *   figure out where the line breaks go -- just flush our buffer
         *   without a trailing newline whenever the buffer is full, and
         *   let the OS layer worry about formatting lines and paragraphs.
         *   
         *   If we're using padding, use newline mode VM_NL_OSNEWLINE.  If
         *   we don't want padding (which is the case if we completely
         *   fill up the buffer without finding any word breaks), write
         *   out in mode VM_NL_NONE, which just flushes the buffer exactly
         *   like it is.  
         */
        flush(vmg_ padding ? VM_NL_OSNEWLINE : VM_NL_NONE_INTERNAL);
    }
    else
    {
        /*
         *   Normal mode - we process the *MORE* prompt ourselves, and we
         *   are responsible for figuring out where the actual line breaks
         *   go.  Use flush() to generate an actual newline whenever we
         *   flush out our buffer.  
         */
        flush(vmg_ VM_NL_NEWLINE);
    }
}


/* ------------------------------------------------------------------------ */
/*
 *   Write a character to an output stream.  The character is provided to us
 *   as a wide Unicode character.  
 */
void CVmFormatter::buffer_char(VMG_ wchar_t c)
{
    const wchar_t *exp;
    size_t exp_len;

    /* check for a display expansion */
    exp = (cmap_ != 0 ? cmap_ : G_cmap_to_ui)->get_expansion(c, &exp_len);
    if (exp != 0)
    {
        /* write each character of the expansion */
        for ( ; exp_len != 0 ; ++exp, --exp_len)
            buffer_expchar(vmg_ *exp);
    }
    else
    {
        /* there's no expansion - buffer the character as-is */
        buffer_expchar(vmg_ c);
    }
}

/*
 *   Write an expanded character to an output stream.  
 */
void CVmFormatter::buffer_expchar(VMG_ wchar_t c)
{
    int i;
    int cwid;
    unsigned char cflags;
    int shy;
    int qspace;

    /* presume the character takes up only one column */
    cwid = 1;

    /* presume we'll use the current flags for the new character */
    cflags = cur_flags_;

    /* assume it's not a quoted space */
    qspace = FALSE;

    /* 
     *   Check for some special characters.
     *   
     *   If we have an underlying HTML renderer, keep track of the HTML
     *   lexical state, so we know if we're in a tag or in ordinary text.  We
     *   can pass through all of the special line-breaking and spacing
     *   characters to the underlying HTML renderer.
     *   
     *   If our underlying renderer is a plain text renderer, we actually
     *   parse the HTML ourselves, so HTML tags will never make it this far -
     *   the caller will already have interpreted any HTML tags and removed
     *   them from the text stream, passing only the final plain text to us.
     *   However, with a plain text renderer, we have to do all of the work
     *   of line breaking, so we must look at the special spacing and
     *   line-break control characters.  
     */
    if (html_target_)
    {
        /* 
         *   track the lexical state of the HTML stream going to the
         *   underlying renderer 
         */
        switch (html_passthru_state_)
        {
        case VMCON_HPS_MARKUP_END:
        case VMCON_HPS_NORMAL:
            /* check to see if we're starting a markup */
            if (c == '&')
                html_passthru_state_ = VMCON_HPS_ENTITY_1ST;
            else if (c == '<')
                html_passthru_state_ = VMCON_HPS_TAG;
            else
                html_passthru_state_ = VMCON_HPS_NORMAL;
            break;

        case VMCON_HPS_ENTITY_1ST:
            /* check to see what kind of entity we have */
            if (c == '#')
                html_passthru_state_ = VMCON_HPS_ENTITY_NUM_1ST;
            else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
                html_passthru_state_ = VMCON_HPS_ENTITY_NAME;
            else
                html_passthru_state_ = VMCON_HPS_NORMAL;
            break;

        case VMCON_HPS_ENTITY_NUM_1ST:
            /* check to see what kind of number we have */
            if (c == 'x' || c == 'X')
                html_passthru_state_ = VMCON_HPS_ENTITY_HEX;
            else if (c >= '0' && c <= '9')
                html_passthru_state_ = VMCON_HPS_ENTITY_DEC;
            else
                html_passthru_state_ = VMCON_HPS_NORMAL;
            break;

        case VMCON_HPS_ENTITY_HEX:
            /* see if we're done with hex digits */
            if (c == ';')
                html_passthru_state_ = VMCON_HPS_MARKUP_END;
            else if ((c < '0' || c > '9')
                     && (c < 'a' || c > 'f')
                     && (c < 'A' || c > 'F'))
                html_passthru_state_ = VMCON_HPS_NORMAL;
            break;

        case VMCON_HPS_ENTITY_DEC:
            /* see if we're done with decimal digits */
            if (c == ';')
                html_passthru_state_ = VMCON_HPS_MARKUP_END;
            else if (c < '0' || c > '9')
                html_passthru_state_ = VMCON_HPS_NORMAL;
            break;

        case VMCON_HPS_ENTITY_NAME:
            /* see if we're done with alphanumerics */
            if (c == ';')
                html_passthru_state_ = VMCON_HPS_MARKUP_END;
            else if ((c < 'a' || c > 'z')
                     && (c < 'A' || c > 'Z')
                     && (c < '0' || c > '9'))
                html_passthru_state_ = VMCON_HPS_NORMAL;
            break;

        case VMCON_HPS_TAG:
            /* see if we're done with the tag, or entering quoted material */
            if (c == '>')
                html_passthru_state_ = VMCON_HPS_MARKUP_END;
            else if (c == '"')
                html_passthru_state_ = VMCON_HPS_DQUOTE;
            else if (c == '\'')
                html_passthru_state_ = VMCON_HPS_SQUOTE;
            break;

        case VMCON_HPS_SQUOTE:
            /* see if we're done with the quoted material */
            if (c == '\'')
                html_passthru_state_ = VMCON_HPS_NORMAL;
            break;

        case VMCON_HPS_DQUOTE:
            /* see if we're done with the quoted material */
            if (c == '"')
                html_passthru_state_ = VMCON_HPS_NORMAL;
            break;

        default:
            /* ignore other states */
            break;
        }
    }
    else
    {
        /* check for special characters */
        switch(c)
        {
        case 0x00AD:
            /*
             *   The Unicode "soft hyphen" character.  This indicates a
             *   point at which we can insert a hyphen followed by a soft
             *   line break, if it's a convenient point to break the line;
             *   if we don't choose to break the line here, the soft hyphen
             *   is invisible.  
             *   
             *   Don't buffer anything at all; instead, just flag the
             *   preceding character as being a soft hyphenation point, so
             *   that we can insert a hyphen there when we get around to
             *   breaking the line.  
             */
            if (linepos_ != 0)
                flagbuf_[linepos_ - 1] |= VMCON_OBF_SHY;

            /* we don't need to buffer anything, so we're done */
            return;

        case 0xFEFF:
            /*
             *   The Unicode zero-width non-breaking space.  This indicates
             *   a point at which we cannot break the line, even if we could
             *   normally break here.  Flag the preceding character as a
             *   non-breaking point.  Don't buffer anything for this
             *   character, as it's not rendered; it merely controls line
             *   breaking.  
             */
            if (linepos_ != 0)
                flagbuf_[linepos_ - 1] |= VMCON_OBF_NOBREAK;

            /* we don't buffer anything, so we're done */
            return;

        case 0x200B:                                    /* zero-width space */
        case 0x200a:                                          /* hair space */
        case 0x2008:                                   /* punctuation space */
            /* 
             *   Zero-width space: This indicates an explicitly allowed
             *   line-breaking point, but is rendered as invisible.  Flag the
             *   preceding character as an OK-to-break point, but don't
             *   buffer anything, as the zero-width space isn't rendered.  
             *   
             *   Hair and punctuation spaces: Treat these very thin spaces as
             *   invisible in a fixed-width font.  These are normally used
             *   for subtle typographical effects in proportionally-spaced
             *   fonts; for example, for separating a right single quote from
             *   an immediately following right double quote (as in a
             *   quotation within a quotation: I said, "type 'quote'").  When
             *   translating to fixed-pitch type, these special spacing
             *   effects aren't usually necessary or desirable because of the
             *   built-in space in every character cell.
             *   
             *   These spaces cancel any explicit non-breaking flag that
             *   precedes them, since they cause the flag to act on the
             *   space's left edge, while leaving the right edge open for
             *   breaking.  Since we don't actually take up any buffer space,
             *   push our right edge's breakability back to the preceding
             *   character.  
             */
            if (linepos_ != 0)
            {
                flagbuf_[linepos_ - 1] &= ~VMCON_OBF_NOBREAK;
                flagbuf_[linepos_ - 1] |= VMCON_OBF_OKBREAK;
            }

            /* we don't buffer anything, so we're done */
            return;

        case 0x00A0:
            /* non-breaking space - buffer it as given */
            break;
            
        case 0x0015:             /* special internal quoted space character */
        case 0x2005:                                   /* four-per-em space */
        case 0x2006:                                    /* six-per-em space */
        case 0x2007:                                        /* figure space */
        case 0x2009:                                          /* thin space */
            /* 
             *   Treat all of these as non-combining spaces, and render them
             *   all as single ordinary spaces.  In text mode, we are
             *   limited to a monospaced font, so we can't render any
             *   differences among these various thinner-than-normal spaces.
             */
            qspace = TRUE;
            c = ' ';
            break;

        case 0x2002:                                            /* en space */
        case 0x2004:                                  /* three-per-em space */
            /* 
             *   En space, three-per-em space - mark these as non-combining,
             *   and render them as a two ordinary spaces.  In the case of
             *   an en space, we really do want to take up the space of two
             *   ordinary spaces; for a three-per-em space, we want about a
             *   space and a half, but since we're dealing with a monospaced
             *   font, we have to round up to a full two spaces.  
             */
            qspace = TRUE;
            cwid = 2;
            c = ' ';
            break;
            
        case 0x2003:
            /* em space - mark it as non-combining */
            qspace = TRUE;
            
            /* render this as three ordinary spaces */
            cwid = 3;
            c = ' ';
            break;

        default:
            /* 
             *   Translate any whitespace character to a regular space
             *   character.  Note that, once this is done, we don't need to
             *   worry about calling t3_is_space() any more - we can just
             *   check that we have a regular ' ' character.  
             */
            if (t3_is_space(c))
            {
                /* convert it to an ordinary space */
                c = ' ';
                
                /* if we're in obey-whitespace mode, quote this space */
                qspace = obey_whitespace_;
            }
            break;
        }
    }

    /* if it's a quoted space, mark it as such in the buffer flags */
    if (qspace)
        cflags |= VMCON_OBF_QSPACE;

    /* 
     *   Check for the caps/nocaps flags - but only if our HTML lexical state
     *   in the underlying text stream is plain text, because we don't want
     *   to apply these flags to alphabetic characters that are inside tag or
     *   entity text.  
     */
    if (html_passthru_state_ == VMCON_HPS_NORMAL)
    {
        if ((capsflag_ || allcapsflag_) && t3_is_alpha(c))
        {
            /* capsflag is set, so capitalize this character */
            c = t3_to_upper(c);
            
            /* okay, we've capitalized something; clear flag */
            capsflag_ = FALSE;
        }
        else if (nocapsflag_ && t3_is_alpha(c))
        {
            /* nocapsflag is set, so minisculize this character */
            c = t3_to_lower(c);
            
            /* clear the flag now that we've done the job */
            nocapsflag_ = FALSE;
        }
    }

    /*
     *   If this is a space of some kind, we might be able to consolidate it
     *   with a preceding character. 
     */
    if (c == ' ')
    {
        /* ignore ordinary whitespace at the start of a line */
        if (linecol_ == 0 && !qspace)
            return;

        /* 
         *   Consolidate runs of whitespace.  Ordinary whitespace is
         *   subsumed into any type of quoted spaces, but quoted spaces do
         *   not combine.  
         */
        if (linepos_ > 0)
        {
            wchar_t prv;

            /* get the previous character */
            prv = linebuf_[linepos_ - 1];
            
            /* 
             *   if the new character is an ordinary (combining) whitespace
             *   character, subsume it into any preceding space character 
             */
            if (!qspace && prv == ' ')
                return;

            /* 
             *   if the new character is a quoted space, and the preceding
             *   character is a non-quoted space, subsume the preceding
             *   space into the new character 
             */
            if (qspace
                && prv == ' '
                && !(flagbuf_[linepos_ - 1] & VMCON_OBF_QSPACE))
            {
                /* remove the preceding ordinary whitespace */
                --linepos_;
                --linecol_;
            }
        }
    }

    /* if the new character fits in the line, add it */
    if (linecol_ + cwid < get_buffer_maxcol())
    {
        /* buffer this character */
        buffer_rendered(c, cflags, cwid);

        /* we're finished processing the character */
        return;
    }

    /*
     *   The line would overflow if this character were added.
     *   
     *   If we're trying to output any kind of breakable space, just add it
     *   to the line buffer for now; we'll come back later and figure out
     *   where to break upon buffering the next non-space character.  This
     *   ensures that we don't carry trailing space (even trailing en or em
     *   spaces) to the start of the next line if we have an explicit
     *   newline before the next non-space character.  
     */
    if (c == ' ')
    {
        /* 
         *   We're adding a space, so we'll figure out the breaking later,
         *   when we output the next non-space character.  If the preceding
         *   character is any kind of space, don't bother adding the new
         *   one, since any contiguous whitespace at the end of the line has
         *   no effect on the line's appearance.  
         */
        if (linebuf_[linepos_ - 1] == ' ')
        {
            /* 
             *   We're adding a space to a line that already ends in a
             *   space, so we don't really need to add the character.
             *   However, reflect the virtual addition in the output column
             *   position, since the space does affect our column position.
             *   We know that we're adding the new space even though we have
             *   a space preceding, since we wouldn't have gotten this far
             *   if we were going to collapse the space with a run of
             *   whitespace. 
             */
        }
        else
        {
            /* the line doesn't already end in space, so add the space */
            linebuf_[linepos_] = ' ';
            flagbuf_[linepos_] = cflags;
            colorbuf_[linepos_] = cur_color_;

            /* advance one character in the buffer */
            ++linepos_;
        }

        /* 
         *   Adjust the column position for the added space.  Note that we
         *   adjust by the rendered width of the new character even though
         *   we actually added only one character; we only add one character
         *   to the buffer to avoid buffer overflow, but the column position
         *   needs adjustment by the full rendered width.  The fact that the
         *   actual buffer size and rendered width no longer match isn't
         *   important because the difference is entirely in invisible
         *   whitespace at the right end of the line.  
         */
        linecol_ += cwid;

        /* done for now */
        return;
    }
    
    /*
     *   We're adding something other than an ordinary space to the line,
     *   and the new character won't fit, so we must find an appropriate
     *   point to break the line. 
     *   
     *   First, add the new character to the buffer - it could be
     *   significant in how we calculate the break position.  (Note that we
     *   allocate the buffer with space for one extra character after
     *   reaching the maximum line width, so we know we have room for this.)
     */
    linebuf_[linepos_] = c;
    flagbuf_[linepos_] = cur_flags_;

    /* 
     *   if the underlying OS layer is doing the line wrapping, just flush
     *   out the buffer; don't bother trying to do any line wrapping
     *   ourselves, since this work would just be redundant with what the OS
     *   layer has to do anyway 
     */
    if (os_line_wrap_)
    {
        /* flush the line, adding no padding after it */
        flush_line(vmg_ FALSE);

        /* 
         *   we've completely cleared out the line buffer, so reset all of
         *   the line buffer counters 
         */
        linepos_ = 0;
        linecol_ = 0;
        linebuf_[0] = '\0';
        is_continuation_ = FALSE;

        /* we're done */
        goto done_with_wrapping;
    }

    /*
     *   Scan backwards, looking for a break position.  Start at the current
     *   column: we know we can fit everything up to this point on a line on
     *   the underlying display, so this is the rightmost possible position
     *   at which we could break the line.  Keep going until we find a
     *   breaking point or reach the left edge of the line.  
     */
    for (shy = FALSE, i = linepos_ ; i >= 0 ; --i)
    {
        unsigned char f;
        unsigned char prvf;
        
        /* 
         *   There are two break modes: word-break mode and break-anywhere
         *   mode.  The modes are applied to each character, via the buffer
         *   flags.
         *   
         *   In word-break mode, we can break at any ordinary space, at a
         *   soft hyphen, just after a regular hyphen, or at any explicit
         *   ok-to-break point; but we can't break after any character
         *   marked as a no-break point.
         *   
         *   In break-anywhere mode, we can break between any two
         *   characters, except that we can't break after any character
         *   marked as a no-break point.  
         */

        /* get the current character's flags */
        f = flagbuf_[i];

        /* get the preceding character's flags */
        prvf = (i > 0 ? flagbuf_[i-1] : 0);

        /* 
         *   if the preceding character is marked as a no-break point, we
         *   definitely can't break here, so keep looking 
         */
        if ((prvf & VMCON_OBF_NOBREAK) != 0)
            continue;

        /* 
         *   if the preceding character is marked as an explicit ok-to-break
         *   point, we definitely can break here 
         */
        if ((prvf & VMCON_OBF_OKBREAK) != 0)
            break;

        /* 
         *   If the current character is in a run of break-anywhere text,
         *   then we can insert a break just before the current character.
         *   Likewise, if the preceding character is in a run of
         *   break-anywhere text, we can break just after the preceding
         *   character, which is the same as breaking just before the
         *   current character.
         *   
         *   Note that we must test for both cases to properly handle
         *   boundaries between break-anywhere and word-break text.  If
         *   we're switching from word-break to break-anywhere text, the
         *   current character will be marked as break-anywhere, so if we
         *   only tested the previous character, we'd miss this transition.
         *   If we're switching from break-anywhere to word-break text, the
         *   previous character will be marked as break-anywhere, so we'd
         *   miss the fact that we could break right here (rather than
         *   before the previous character) if we didn't test it explicitly.
         */
        if ((f & VMCON_OBF_BREAK_ANY) != 0
            || (i > 0 && (prvf & VMCON_OBF_BREAK_ANY) != 0))
            break;

        /* 
         *   If the preceding character is marked as a soft hyphenation
         *   point, and we're not at the rightmost position, we can break
         *   here with hyphenation.  We can't break with hyphenation at the
         *   last position because hyphenation requires us to actually
         *   insert a hyphen character, and we know that at the last
         *   position we don't have room for inserting another character.  
         */
        if (i > 0 && i < linepos_ && (prvf & VMCON_OBF_SHY) != 0)
        {
            /* note that we're breaking at a soft hyphen */
            shy = TRUE;

            /* we can break here */
            break;
        }

        /* 
         *   we can break to the left of a space (i.e., we can break before
         *   the current character if the current character is a space) 
         */
        if (linebuf_[i] == ' ')
            break;

        /* 
         *   We can also break to the right of a space.  We need to check
         *   for this case separately from checking that the current
         *   charatcer is a space (which breaks to the left of the space),
         *   because we could have a no-break marker on one side of the
         *   space but not on the other side.  
         */
        if (i > 0 && linebuf_[i-1] == ' ')
            break;

        /* 
         *   If we're to the right of a hyphen, we can break here.  However,
         *   don't break in the middle of a set of consecutive hyphens
         *   (i.e., we don't want to break up "--" sequences).
         */
        if (i > 0 && linebuf_[i-1] == '-' && linebuf_[i] != '-')
            break;
    }
    
    /* check to see if we found a good place to break */
    if (i < 0)
    {
        /*
         *   We didn't find a good place to break.  If the underlying
         *   console allows overrunning the line width, simply add the
         *   character, even though it overflows; otherwise, force a break
         *   at the line width, even though it doesn't occur at a natural
         *   breaking point.
         *   
         *   In any case, don't let our buffer fill up beyond its maximum
         *   size.  
         */
        if (!console_->allow_overrun() || linepos_ + 1 >= OS_MAXWIDTH)
        {
            /* 
             *   we didn't find any good place to break, and the console
             *   doesn't allow us to overrun the terminal width - flush the
             *   entire line as-is, breaking arbitrarily in the middle of a
             *   word 
             */
            flush_line(vmg_ FALSE);
            
            /* 
             *   we've completely cleared out the line buffer, so reset all
             *   of the line buffer counters 
             */
            linepos_ = 0;
            linecol_ = 0;
            linebuf_[0] = '\0';
            is_continuation_ = FALSE;
        }
    }
    else
    {
        wchar_t tmpbuf[OS_MAXWIDTH];
        vmcon_color_t tmpcolor[OS_MAXWIDTH];
        unsigned char tmpflags[OS_MAXWIDTH];
        size_t tmpchars;
        int nxti;

        /* null-terminate the line buffer */        
        linebuf_[linepos_] = '\0';

        /* trim off leading spaces on the next line after the break */
        for (nxti = i ; linebuf_[nxti] == ' ' ; ++nxti) ;

        /* 
         *   The next line starts after the break - save a copy.  We actually
         *   have to save a copy of the trailing material outside the buffer,
         *   since we might have to overwrite the trailing part of the buffer
         *   to expand tabs.  
         */
        tmpchars = wcslen(&linebuf_[nxti]);
        memcpy(tmpbuf, &linebuf_[nxti], tmpchars*sizeof(tmpbuf[0]));
        memcpy(tmpcolor, &colorbuf_[nxti], tmpchars*sizeof(tmpcolor[0]));
        memcpy(tmpflags, &flagbuf_[nxti], tmpchars*sizeof(tmpflags[0]));

        /* if we're breaking at a soft hyphen, insert a real hyphen */
        if (shy)
            linebuf_[i++] = '-';
        
        /* trim off trailing spaces */
        for ( ; i > 0 && linebuf_[i-1] == ' ' ; --i)
        {
            /* stop if we've reached a non-breaking point */
            if ((flagbuf_[i-1] & VMCON_OBF_NOBREAK) != 0)
                break;
        }

        /* terminate the buffer after the break point */
        linebuf_[i] = '\0';
        
        /* write out everything up to the break point */
        flush_line(vmg_ TRUE);

        /* move the saved start of the next line into the line buffer */
        memcpy(linebuf_, tmpbuf, tmpchars*sizeof(tmpbuf[0]));
        memcpy(colorbuf_, tmpcolor, tmpchars*sizeof(tmpcolor[0]));
        memcpy(flagbuf_, tmpflags, tmpchars*sizeof(tmpflags[0]));
        linecol_ = linepos_ = tmpchars;
    }
    
done_with_wrapping:
    /* add the new character to buffer */
    buffer_rendered(c, cflags, cwid);
}

/*
 *   Write a rendered character to an output stream buffer.  This is a
 *   low-level internal routine that we call from buffer_expchar() to put
 *   the final rendition of a character into a buffer.
 *   
 *   Some characters render as multiple copies of a single character; 'wid'
 *   gives the number of copies to store.  The caller is responsible for
 *   ensuring that the rendered representation fits in the buffer and in the
 *   available line width.  
 */
void CVmFormatter::buffer_rendered(wchar_t c, unsigned char flags, int wid)
{
    unsigned char flags_before;

    /* note whether or not we have a break before us */
    flags_before = (linepos_ > 0
                    ? flagbuf_[linepos_-1] & VMCON_OBF_NOBREAK
                    : 0);

    /* add the character the given number of times */
    for ( ; wid != 0 ; --wid)
    {
        /* buffer the character */
        linebuf_[linepos_] = c;
        flagbuf_[linepos_] = flags;
        colorbuf_[linepos_] = cur_color_;

        /* 
         *   if this isn't the last part of the character, carry forward any
         *   no-break flag from the previous part of the character; this will
         *   ensure that a no-break to the left of the sequence applies to
         *   the entire sequence 
         */
        if (wid > 1)
            flagbuf_[linepos_] |= flags_before;

        /* advance one character in the buffer */
        ++linepos_;

        /* adjust our column counter */
        ++linecol_;
    }
}

/* ------------------------------------------------------------------------ */
/* 
 *   write out a UTF-8 string
 */
void CVmFormatter::buffer_string(VMG_ const char *txt)
{
    /* write out each character in the string */
    for ( ; utf8_ptr::s_getch(txt) != 0 ; txt += utf8_ptr::s_charsize(*txt))
        buffer_char(vmg_ utf8_ptr::s_getch(txt));
}

/*
 *   write out a wide unicode string 
 */
void CVmFormatter::buffer_wstring(VMG_ const wchar_t *txt)
{
    /* write out each wide character */
    for ( ; *txt != '\0' ; ++txt)
        buffer_char(vmg_ *txt);
}


/* ------------------------------------------------------------------------ */
/*
 *   Get the next wide unicode character in a UTF8-encoded string, and
 *   update the string pointer and remaining length.  Returns zero if no
 *   more characters are available in the string.  
 */
wchar_t CVmFormatter::next_wchar(const char **s, size_t *len)
{
    wchar_t ret;
    size_t charsize;

    /* if there's nothing left, return a null terminator */
    if (*len == 0)
        return 0;

    /* get this character */
    ret = utf8_ptr::s_getch(*s);

    /* advance the string pointer and length counter */
    charsize = utf8_ptr::s_charsize(**s);
    *len -= charsize;
    *s += charsize;

    /* return the result */
    return ret;
}

/* ------------------------------------------------------------------------ */
/*
 *   Display a string of a given length.  The text is encoded as UTF-8
 *   characters.  
 */
int CVmFormatter::format_text(VMG_ const char *s, size_t slen)
{
    wchar_t c;
    int done = FALSE;

    /* get the first character */
    c = next_wchar(&s, &slen);

    /* if we have anything to show, show it */
    while (c != '\0')
    {
        /* 
         *   first, process the character through our built-in text-only HTML
         *   mini-parser, if our HTML mini-parser state indicates that we're
         *   in the midst of parsing a tag 
         */
        if (html_parse_state_ != VMCON_HPS_NORMAL
            || (html_in_ignore_ && c != '&' && c != '<'))
        {
            /* run our HTML parsing until we finish the tag */
            c = resume_html_parsing(vmg_ c, &s, &slen);

            /* proceed with the next character */
            continue;
        }

        /* check for special characters */
        switch(c)
        {
        case 10:
            /* newline */
            flush(vmg_ VM_NL_NEWLINE);
            break;
                    
        case 9:
            /* tab - write an ordinary every-4-columns tab */
            write_tab(vmg_ 0, 4);
            break;

        case 0x000B:
            /* \b - blank line */
            write_blank_line(vmg0_);
            break;
                    
        case 0x000F:
            /* capitalize next character */
            capsflag_ = TRUE;
            nocapsflag_ = FALSE;
            break;

        case 0x000E:
            /* un-capitalize next character */
            nocapsflag_ = TRUE;
            capsflag_ = FALSE;
            break;

        case '<':
        case '&':
            /* HTML markup-start character - process it */
            if (html_target_ || literal_mode_)
            {
                /* 
                 *   The underlying OS renderer interprets HTML mark-up
                 *   sequences, OR we're processing all text literally; in
                 *   either case, we don't need to perform any
                 *   interpretation.  Simply pass through the character as
                 *   though it were any other.  
                 */
                goto normal_char;
            }
            else
            {
                /*
                 *   The underlying target does not accept HTML sequences.
                 *   It appears we're at the start of an "&" entity or a tag
                 *   sequence, so parse it, remove it, and replace it (if
                 *   possible) with a text-only equivalent.  
                 */
                c = parse_html_markup(vmg_ c, &s, &slen);

                /* go back and process the next character */
                continue;
            }
            break;

        case 0x0015:                      /* our own quoted space character */
        case 0x00A0:                                  /* non-breaking space */
        case 0x00AD:                                         /* soft hyphen */
        case 0xFEFF:                       /* non-breaking zero-width space */
        case 0x2002:                                            /* en space */
        case 0x2003:                                            /* em space */
        case 0x2004:                                  /* three-per-em space */
        case 0x2005:                                   /* four-per-em space */
        case 0x2006:                                    /* six-per-em space */
        case 0x2007:                                        /* figure space */
        case 0x2008:                                   /* punctuation space */
        case 0x2009:                                          /* thin space */
        case 0x200a:                                          /* hair space */
        case 0x200b:                                    /* zero-width space */
            /* 
             *   Special Unicode characters.  For HTML targets, write these
             *   as &# sequences - this bypasses character set translation
             *   and ensures that the HTML parser will see them as intended.
             */
            if (html_target_)
            {
                char buf[15];
                char *p;
                
                /* 
                 *   it's an HTML target - render these as &# sequences;
                 *   generate the decimal representation of 'c' (in reverse
                 *   order, hence start with the terminating null byte and
                 *   the semicolon) 
                 */
                p = buf + sizeof(buf) - 1;
                *p-- = '\0';
                *p-- = ';';

                /* generate the decimal representation of 'c' */
                for ( ; c != 0 ; c /= 10)
                    *p-- = (c % 10) + '0';

                /* add the '&#' sequence */
                *p-- = '#';
                *p = '&';

                /* write out the sequence */
                buffer_string(vmg_ p);
            }
            else
            {
                /* for non-HTML targets, treat these as normal */
                goto normal_char;
            }
            break;

        default:
        normal_char:
            /* normal character - write it out */
            buffer_char(vmg_ c);
            break;
        }

        /* move on to the next character, unless we're finished */
        if (done)
            c = '\0';
        else
            c = next_wchar(&s, &slen);
    }

    /* success */
    return 0;
}

/* ------------------------------------------------------------------------ */
/*
 *   Initialize the display object 
 */
CVmConsole::CVmConsole()
{
    /* no script file yet */
    script_fp_ = 0;
    quiet_script_ = FALSE;

    /* no command log file yet */
    command_fp_ = 0;

    /* assume we'll double-space after each period */
    doublespace_ = TRUE;

    /* clear the debug flags */
    outwxflag_ = FALSE;

    /* presume we'll have no log stream */
    log_str_ = 0;
    log_enabled_ = FALSE;
}

/*
 *   Delete the display object 
 */
CVmConsole::~CVmConsole()
{
    /* close any active script file */
    if (script_fp_ != 0)
        osfcls(script_fp_);

    /* close any active command log file */
    close_command_log();

    /* delete the log stream if we have one */
    if (log_str_ != 0)
        delete log_str_;
}

/* ------------------------------------------------------------------------ */
/*
 *   Display a string of a given byte length 
 */
int CVmConsole::format_text(VMG_ const char *p, size_t len)
{
    int ret;

    /* presume we'll return success */
    ret = 0;

    /* if the debugger is showing watchpoints, suppress all output */
    if (outwxflag_)
        return ret;

    /* display the string */
    disp_str_->format_text(vmg_ p, len);

    /* if there's a log file, write to the log file as well */
    if (log_enabled_)
        log_str_->format_text(vmg_ p, len);

    /* return the result from displaying to the screen */
    return ret;
}

/* ------------------------------------------------------------------------ */
/*
 *   Set the text color 
 */
void CVmConsole::set_text_color(VMG_ os_color_t fg, os_color_t bg)
{
    /* set the color in our main display stream */
    disp_str_->set_text_color(vmg_ fg, bg);
}

/*
 *   Set the body color 
 */
void CVmConsole::set_body_color(VMG_ os_color_t color)
{
    /* set the color in the main display stream */
    disp_str_->set_os_body_color(color);
}

/* ------------------------------------------------------------------------ */
/*
 *   Display a blank line 
 */
void CVmConsole::write_blank_line(VMG0_)
{
    /* generate the newline to the standard display */
    disp_str_->write_blank_line(vmg0_);

    /* if we're logging, generate the newline to the log file as well */
    if (log_enabled_)
        log_str_->write_blank_line(vmg0_);
}


/* ------------------------------------------------------------------------ */
/*
 *   outcaps() - sets an internal flag which makes the next letter output
 *   a capital, whether it came in that way or not.  Set the same state in
 *   both formatters (standard and log).  
 */
void CVmConsole::caps()
{
    disp_str_->caps();
    if (log_enabled_)
        log_str_->caps();
}

/*
 *   outnocaps() - sets the next letter to a miniscule, whether it came in
 *   that way or not.  
 */
void CVmConsole::nocaps()
{
    disp_str_->nocaps();
    if (log_enabled_)
        log_str_->nocaps();
}

/*
 *   obey_whitespace() - sets the obey-whitespace mode 
 */
int CVmConsole::set_obey_whitespace(int f)
{
    int ret;

    /* note the original display stream status */
    ret = disp_str_->get_obey_whitespace();

    /* set the stream status */
    disp_str_->set_obey_whitespace(f);
    if (log_enabled_)
        log_str_->set_obey_whitespace(f);

    /* return the original status of the display stream */
    return ret;
}

/* ------------------------------------------------------------------------ */
/*
 *   Open a log file 
 */
int CVmConsole::open_log_file(const char *fname)
{
    /* if there's no log stream, we can't open a log file */
    if (log_str_ == 0)
        return 1;

    /* 
     *   Tell the log stream to open the file.  Set the log file's HTML
     *   source mode flag to the same value as is currently being used in
     *   the main display stream, so that it will interpret source markups
     *   the same way that the display stream is going to.  
     */
    return log_str_->open_log_file(fname);
}

/*
 *   Close the log file 
 */
int CVmConsole::close_log_file()
{
    /* if there's no log stream, there's obviously no file open */
    if (log_str_ == 0)
        return 1;

    /* tell the log stream to close its file */
    return log_str_->close_log_file();
}

#if 0 //$$$
/*
 *   This code is currently unused.  However, I'm leaving it in for now -
 *   the algorithm takes a little thought, so it would be nicer to be able
 *   to uncomment the existing code should we ever need it in the future.  
 */

/* ------------------------------------------------------------------------ */
/*
 *   Write UTF-8 text explicitly to the log file.  This can be used to add
 *   special text (such as prompt text) that would normally be suppressed
 *   from the log file.  When more mode is turned off, we don't
 *   automatically copy text to the log file; any text that the caller
 *   knows should be in the log file during times when more mode is turned
 *   off can be explicitly added with this function.
 *   
 *   If nl is true, we'll add a newline at the end of this text.  The
 *   caller should not include any newlines in the text being displayed
 *   here.  
 */
void CVmConsole::write_to_logfile(VMG_ const char *txt, int nl)
{
    /* if there's no log file, there's nothing to do */
    if (logfp_ == 0)
        return;

    /* 
     *   convert the text from UTF-8 to the local character set and write
     *   the converted text to the log file 
     */
    while (*txt != '\0')
    {
        char local_buf[128];
        size_t src_bytes_used;
        size_t out_bytes;

        /* convert as much as we can (leaving room for a null terminator) */
        out_bytes = G_cmap_to_file->map_utf8(
            local_buf, sizeof(local_buf) - 1,
            txt, strlen(txt), &src_bytes_used);

        /* null-terminate the result */
        local_buf[out_bytes] = '\0';
        
        /* write the converted text */
        os_fprintz(logfp_, local_buf);

        /* skip the text we were able to convert */
        txt += src_bytes_used;
    }

    /* add a newline if desired */
    if (nl)
    {
        /* add a normal newline */
        os_fprintz(logfp_, "\n");

        /* if the logfile is an html target, write an HTML line break */
        if (log_str_ != 0 && log_str_->is_html_target())
            os_fprintz(logfp_, "<BR HEIGHT=0>\n");
    }
}
#endif /* 0 */


/* ------------------------------------------------------------------------ */
/*
 *   Reset the MORE line counter.  This should be called whenever user
 *   input is read, since stopping to read user input makes it unnecessary
 *   to show another MORE prompt until the point at which input was
 *   solicited scrolls off the screen.  
 */
void CVmConsole::reset_line_count(int clearing)
{
    /* reset the MORE counter in the display stream */
    disp_str_->reset_line_count(clearing);
}

/* ------------------------------------------------------------------------ */
/*
 *   Flush the output line.  We'll write to both the standard display and
 *   the log file, as needed.  
 */
void CVmConsole::flush(VMG_ vm_nl_type nl)
{
    /* flush the display stream */
    disp_str_->flush(vmg_ nl);

    /* flush the log stream, if we have an open log file */
    if (log_enabled_)
        log_str_->flush(vmg_ nl);
}

/* ------------------------------------------------------------------------ */
/*
 *   Clear our buffers
 */
void CVmConsole::empty_buffers(VMG0_)
{
    /* tell the formatter to clear its buffer */
    disp_str_->empty_buffers(vmg0_);

    /* same with the log stream, if applicable */
    if (log_enabled_)
        log_str_->empty_buffers(vmg0_);
}

/* ------------------------------------------------------------------------ */
/*
 *   Immediately update the display 
 */
void CVmConsole::update_display(VMG0_)
{
    /* update the display for the main display stream */
    disp_str_->update_display(vmg0_);
}

/* ------------------------------------------------------------------------ */
/*
 *   Open a script file 
 */
void CVmConsole::open_script_file(const char *fname, int quiet,
                                  int script_more_mode)
{
    /* close any existing script file */
    close_script_file();

    /* open the new file */
    script_fp_ = osfoprt(fname, OSFTCMD);
    
    /* 
     *   if we successfully opened the file, remember the quiet setting;
     *   otherwise, we're definitely not reading a quiet script because
     *   we're not reading a script at all 
     */
    quiet_script_ = (script_fp_ != 0 && quiet);

    /*
     *   If we successfully opened a script file, remember the original
     *   MORE mode, and set the MORE mode that the caller wants in effect
     *   while processing the script.  If we didn't successfully open a
     *   script, don't make any change to the MORE mode.  
     */
    if (script_fp_ != 0)
    {
        /* set the MORE mode */
        pre_script_more_mode_ = set_more_state(script_more_mode);

        /* turn on NONSTOP mode in the OS layer */
        if (!script_more_mode)
            os_nonstop_mode(TRUE);
    }
}

/*
 *   Close the script file 
 */
int CVmConsole::close_script_file()
{
    /* if we have a file, close it */
    if (script_fp_ != 0)
    {
        /* close the file */
        osfcls(script_fp_);

        /* forget the script file */
        script_fp_ = 0;

        /* 
         *   we're not reading any script any more, so forget any
         *   quiet-script mode that's in effect 
         */
        quiet_script_ = FALSE;

        /* turn off NONSTOP mode in the OS layer */
        os_nonstop_mode(FALSE);

        /* 
         *   return the MORE mode in effect before we started reading the
         *   script file 
         */
        return pre_script_more_mode_;
    }
    else
    {
        /* 
         *   there's no script file - just return the current MORE mode,
         *   since we're not making any changes 
         */
        return is_more_mode();
    }
}

/* ------------------------------------------------------------------------ */
/*
 *   Open a command log file 
 */
int CVmConsole::open_command_log(const char *fname)
{
    /* close any existing command log file */
    close_command_log();
    
    /* remember the filename */
    strcpy(command_fname_, fname);

    /* open the file */
    command_fp_ = osfopwt(fname, OSFTCMD);

    /* return success if we successfully opened the file */
    return (command_fp_ == 0);
}

/* 
 *   close the active command log file 
 */
int CVmConsole::close_command_log()
{
    /* if there's a command log file, close it */
    if (command_fp_ != 0)
    {
        /* close the file */
        osfcls(command_fp_);

        /* set its file type */
        os_settype(command_fname_, OSFTCMD);

        /* forget the file */
        command_fp_ = 0;
    }

    /* success */
    return 0;
}


/* ------------------------------------------------------------------------ */
/*
 *   Read a line of input from the console.  Fills in the buffer with a
 *   null-terminated string in the UTF-8 character set.  Returns zero on
 *   success, non-zero on end-of-file.  
 */
int CVmConsole::read_line(VMG_ char *buf, size_t buflen)
{
    /* cancel any previous interrupted input */
    read_line_cancel(vmg_ TRUE);

try_again:
    /* use the timeout version, with no timeout specified */
    switch(read_line_timeout(vmg_ buf, buflen, 0, FALSE))
    {
    case OS_EVT_LINE:
        /* success */
        return 0;

    case VMCON_EVT_END_SCRIPT:
        /* 
         *   end of script - we have no way to communicate this result back
         *   to our caller, so simply ignore the result and ask for another
         *   line 
         */
        goto try_again;

    default:
        /* anything else is an error */
        return 1;
    }
}


/* ------------------------------------------------------------------------ */
/*
 *   Static variables for input state.  We keep these statically, because we
 *   might need to use the values across a series of read_line_timeout calls
 *   if timeouts occur. 
 */

/* original 'more' mode, before input began */
static int S_old_more_mode;

/* flag: input is pending from an interrupted read_line_timeout invocation */
static int S_read_in_progress;

/* local buffer for reading input lines */
static char S_read_buf[256];


/*
 *   Read a line of input from the console, with an optional timeout value. 
 */
int CVmConsole::read_line_timeout(VMG_ char *buf, size_t buflen,
                                  unsigned long timeout, int use_timeout)
{
    int echo_text;
    char *outp;
    size_t outlen;
    int evt;
    int resuming;

    /* 
     *   presume we won't echo the text to the display; in most cases, it
     *   will be echoed to the display in the course of reading it from
     *   the keyboard 
     */
    echo_text = FALSE;

    /*
     *   If we're not resuming an interrupted read already in progress,
     *   initialize some display settings. 
     */
    if (!S_read_in_progress)
    {
        /* 
         *   Turn off MORE mode if it's on - we don't want a MORE prompt
         *   showing up in the midst of user input.  
         */
        S_old_more_mode = set_more_state(FALSE);

        /* 
         *   flush the output; don't start a new line, since we might have
         *   displayed a prompt that is to be on the same line with the user
         *   input 
         */
        flush_all(vmg_ VM_NL_INPUT);

        /* 
         *   reset the MORE line counter, since we're reading user input at
         *   the current point and shouldn't pause for a MORE prompt until
         *   the text we're reading has scrolled off the screen 
         */
        reset_line_count(FALSE);

        /* if there's a script file, read from it */
        if (script_fp_ != 0)
        {
            /* try reading a line from the script file */
            if (read_line_from_script(S_read_buf, sizeof(S_read_buf)))
            {
                int was_quiet;

                /* note whether or not we were in quiet mode */
                was_quiet = quiet_script_;

                /* 
                 *   End of script file - return to reading from the
                 *   keyboard.  The return value from close_script_file() is
                 *   the MORE mode that was in effect before we started
                 *   reading the script file; use this when we restore the
                 *   enclosing MORE mode so that we restore the pre-script
                 *   MORE mode when we return.  
                 */
                S_old_more_mode = close_script_file();
                
                /* turn off MORE mode, in case we read from the keyboard */
                set_more_state(FALSE);
                
                /* flush any output we generated while reading the script */
                flush(vmg_ VM_NL_NONE);
                
                /* 
                 *   reset the MORE line counter, since we might have
                 *   generated output while reading the script file 
                 */
                reset_line_count(FALSE);

                /* 
                 *   If we were in quiet mode, let the caller know we've
                 *   finished reading a script, so that the caller can set
                 *   up the display properly for reading from the keyboard.
                 *   
                 *   If we weren't in quiet mode, we'll simply proceed to
                 *   the normal keyboard reading; when not in quiet mode, no
                 *   special display fixup is needed.  
                 */
                if (was_quiet)
                {
                    /* return to the old MORE mode */
                    set_more_state(S_old_more_mode);

                    /* add a blank line to the log file, if necessary */
                    if (log_enabled_)
                        log_str_->print_to_os("\n");

                    /* note in the streams that we've read an input line */
                    disp_str_->note_input_line();
                    if (log_str_ != 0)
                        log_str_->note_input_line();

                    /* 
                     *   generate a synthetic "end of script" event to let
                     *   the caller know we're switching back to regular
                     *   keyboard reading 
                     */
                    return VMCON_EVT_END_SCRIPT;
                }

                /*
                 *   Note that we do not have an event yet - we've merely
                 *   closed the script file, and now we're going to continue
                 *   by reading a line from the keybaord instead.  The call
                 *   to close_script_file() above will have left script_fp_
                 *   == 0, so we'll shortly read an event from the keyboard.
                 *   Thus 'evt' is still not set to any value, because we do
                 *   not yet have an event - this is intentional.  
                 */
            }
            else
            {
                /* 
                 *   we got a line from the script file - if we're not in
                 *   quiet mode, make a note to echo the text to the display 
                 */
                if (!quiet_script_)
                    echo_text = TRUE;

                /* we've read a line */
                evt = OS_EVT_LINE;
            }
        }
    }

    /* 
     *   if reading was already in progress, we're resuming a previously
     *   interrupted read operation 
     */
    resuming = S_read_in_progress;

    /* reading is now in progress */
    S_read_in_progress = TRUE;

    /* 
     *   if we don't have a script file, or we're resuming an interrupted
     *   read operation, read from the keyboard 
     */
    if (script_fp_ == 0 || resuming)
    {
        /* read a line from the keyboard */
        evt = os_gets_timeout((uchar *)S_read_buf, sizeof(S_read_buf),
                              timeout, use_timeout);

        /*
         *   If that failed because timeout is not supported on this
         *   platform, and the caller didn't actually want to use a timeout,
         *   try again with an ordinary os_gets().  If they wanted to use a
         *   timeout, simply return the NOTIMEOUT indication to our caller.  
         */
        if (evt == OS_EVT_NOTIMEOUT && !use_timeout)
        {
            /* perform an ordinary untimed input */
            if (os_gets((uchar *)S_read_buf, sizeof(S_read_buf)) != 0)
            {
                /* success */
                evt = OS_EVT_LINE;
            }
            else
            {
                /* error reading input */
                evt = OS_EVT_EOF;
            }
        }

        /* 
         *   If we actually read a line, notify the display stream that we
         *   read text from the console - it might need to make some
         *   internal bookkeeping adjustments to account for the fact that
         *   we moved the write position around on the display.
         *   
         *   Don't note the input if we timed out, since we haven't finished
         *   reading the line yet in this case.  
         */
        if (evt == OS_EVT_LINE)
        {
            disp_str_->note_input_line();
            if (log_str_ != 0)
                log_str_->note_input_line();
        }
    }

    /* if we got an error, return it */
    if (evt == OS_EVT_EOF)
    {
        set_more_state(S_old_more_mode);
        return evt;
    }

    /* if we finished reading the line, do our line-finishing work */
    if (evt == OS_EVT_LINE)
        read_line_done(vmg0_);

    /* 
     *   Convert the text from the local UI character set to UTF-8.
     *   Reserve space in the output buffer for the null terminator. 
     */
    outp = buf;
    outlen = buflen - 1;
    G_cmap_from_ui->map(&outp, &outlen, S_read_buf, strlen(S_read_buf));

    /* add the null terminator */
    *outp = '\0';

    /* 
     *   If we need to echo the text (because we read it from a script
     *   file), do so now.
     */
    if (echo_text)
    {
        /* show the text */
        format_text(vmg_ buf);

        /* add a newline */
        format_text(vmg_ "\n");
    }

    /* return the event code */
    return evt;
}

/*
 *   Cancel an interrupted input. 
 */
void CVmConsole::read_line_cancel(VMG_ int reset)
{
    /* reset the underling OS layer */
    os_gets_cancel(reset);

    /* do our line-ending work */
    read_line_done(vmg0_);
}

/*
 *   Perform line-ending work.  This is used when we finish reading a line
 *   in read_line_timeout(), or when we cancel an interrupted line, thus
 *   finishing the line, in read_line_cancel(). 
 */
void CVmConsole::read_line_done(VMG0_)
{
    /* if we have a line in progress, finish it off */
    if (S_read_in_progress)
    {
        /* set the original 'more' mode */
        set_more_state(S_old_more_mode);

        /* 
         *   Write the input line, followed by a newline, to the log file.
         *   Note that the text is still in the local character set, so we
         *   can write it directly to the log file.
         *   
         *   If we're reading from a script file in "echo" mode, skip this.
         *   When reading from a script file in "echo" mode, we will manually
         *   copy the input commands to the main console, which will
         *   automatically copy to the main log file.  If we're in quiet
         *   scripting mode, though, we won't do that, so we do need to
         *   capture the input explicitly here.  
         */
        if (log_enabled_ && (script_fp_ == 0 || quiet_script_))
        {
            log_str_->print_to_os(S_read_buf);
            log_str_->print_to_os("\n");
        }
        
        /*
         *   If we have a command log file, log the command (preceded by a
         *   ">" character, to indicate that it's a command, and followed by
         *   a newline) to the command log.  
         */
        if (command_fp_ != 0)
        {
            os_fprintz(command_fp_, ">");
            os_fprintz(command_fp_, S_read_buf);
            os_fprintz(command_fp_, "\n");
        }

        /* note in the streams that we've read an input line */
        disp_str_->note_input_line();
        if (log_str_ != 0)
            log_str_->note_input_line();

        /* clear the in-progress flag */
        S_read_in_progress = FALSE;
    }
}

/*
 *   Read a line of text from the script file, if there is one.  Returns
 *   zero on success, non-zero if we reach the end of the script file or
 *   encounter any other error. 
 */
int CVmConsole::read_line_from_script(char *buf, size_t buflen)
{
    /* if there's no script file, return failure */
    if (script_fp_ == 0)
        return 1;

    /* keep going until we find a line that we like */
    for (;;)
    {
        char c;

        /* 
         *   Read the next character of input.  If it's not a newline,
         *   there's more text on the same line, so read the rest and
         *   determine what to do. 
         */
        c = osfgetc(script_fp_);
        if (c != '\n' && c != '\r')
        {
            /* there's more on this line - read the rest of the line */
            if (osfgets(buf, buflen, script_fp_) == 0)
            {
                /* end of file - return failure */
                return 1;
            }

            /* 
             *   if the line starts with '>', it's a command line;
             *   otherwise, it's a comment or something else, in which
             *   case we'll ignore it and keep looking for a line starting
             *   with '>' 
             */
            if (c == '>')
            {
                size_t len;
                
                /* 
                 *   if there are any trailing newline characters in the
                 *   buffer, remove them
                 */
                len = strlen(buf);
                while (len > 0
                       && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
                    buf[--len] = '\0';

                /* return success */
                return 0;
            }
        }
        else if (c == EOF)
        {
            /* end of file - return failure */
            return 1;
        }
    }
}

/* ------------------------------------------------------------------------ */
/*
 *   Main System Console 
 */

/*
 *   create 
 */
CVmConsoleMain::CVmConsoleMain()
{
    /* create the system banner manager */
    banner_manager_ = new CVmBannerManager();

    /* create the log console manager */
    log_console_manager_ = new CVmLogConsoleManager();

    /* create and initialize our display stream */
    main_disp_str_ = new CVmFormatterMain(this, 256);
    main_disp_str_->init();

    /* initially send text to the main display stream */
    disp_str_ = main_disp_str_;

    /* 
     *   Create and initialize our log stream.  The main console always has a
     *   log stream, even when it's not in use, so that we can keep the log
     *   stream's state synchronized with the display stream in preparation
     *   for activation.  
     */
    log_str_ = new CVmFormatterLog(this, 80);
    log_str_->init();

    /* 
     *   the log stream is initially enabled (this is separate from the log
     *   file being opened; it merely indicates that we send output
     *   operations to the log stream for processing) 
     */
    log_enabled_ = TRUE;

    /* we don't have a statusline formatter until asked for one */
    statline_str_ = 0;

    /* reset statics */
    S_read_in_progress = FALSE;
    S_read_buf[0] = '\0';
}

/*
 *   delete 
 */
CVmConsoleMain::~CVmConsoleMain()
{
    /* delete the system banner manager */
    banner_manager_->delete_obj();

    /* delete the system log console manager */
    log_console_manager_->delete_obj();

    /* delete the display stream */
    delete main_disp_str_;

    /* delete the statusline stream, if we have one */
    if (statline_str_ != 0)
        delete statline_str_;
}

/*
 *   Clear the window 
 */
void CVmConsoleMain::clear_window(VMG0_)
{
    /* flush and empty our output buffer */
    flush(vmg_ VM_NL_NONE);
    empty_buffers(vmg0_);
    
    /* clear the main window */
    oscls();

    /* reset the MORE line counter in the display stream */
    disp_str_->reset_line_count(TRUE);
}

/*
 *   Set statusline mode 
 */
void CVmConsoleMain::set_statusline_mode(VMG_ int mode)
{
    CVmFormatterDisp *str;

    /* 
     *   if we're switching into statusline mode, and we don't have a
     *   statusline stream yet, create one 
     */
    if (mode && statline_str_ == 0)
    {
        /* create and initialize the statusline stream */
        statline_str_ = new CVmFormatterStatline(this);
        statline_str_->init();
    }

    /* get the stream selected by the new mode */
    if (mode)
        str = statline_str_;
    else
        str = main_disp_str_;

    /* if this is already the active stream, we have nothing more to do */
    if (str == disp_str_)
        return;

    /* make the new stream current */
    disp_str_ = str;

    /* 
     *   check which mode we're switching to, so we can do some extra work
     *   specific to each mode 
     */
    if (mode)
    {
        /* 
         *   we're switching to the status line, so disable the log stream -
         *   statusline text is never sent to the log, since the log reflects
         *   only what was displayed in the main text area 
         */
        log_enabled_ = FALSE;
    }
    else
    {
        /*
         *   we're switching back to the main stream, so flush the statusline
         *   so we're sure the statusline text is displayed 
         */

        /* end the line */
        statline_str_->format_text(vmg_ "\n", 1);

        /* flush output */
        statline_str_->flush(vmg_ VM_NL_NONE);

        /* re-enable the log stream, if we have one */
        if (log_str_ != 0)
            log_enabled_ = TRUE;
    }

    /* switch at the OS layer */
    os_status(mode);
}

/*
 *   Flush everything 
 */
void CVmConsoleMain::flush_all(VMG_ vm_nl_type nl)
{
    /* flush our primary console */
    flush(vmg_ nl);

    /* 
     *   Flush each banner we're controlling.  Note that we explicitly flush
     *   the banners with newline mode 'NONE', regardless of the newline mode
     *   passed in by the caller: the caller's mode is for the primary
     *   console, but for the banners we just want to make sure they're
     *   flushed out normally, since whatever we're doing in the primary
     *   console that requires flushing doesn't concern the banners. 
     */
    banner_manager_->flush_all(vmg_ VM_NL_NONE);
}

/* ------------------------------------------------------------------------ */
/*
 *   Handle manager 
 */

/* initialize */
CVmHandleManager::CVmHandleManager()
{
    size_t i;

    /* allocate an initial array of handle slots */
    handles_max_ = 32;
    handles_ = (void **)t3malloc(handles_max_ * sizeof(*handles_));

    /* all slots are initially empty */
    for (i = 0 ; i < handles_max_ ; ++i)
        handles_[i] = 0;
}

/* delete the object - this is the public destructor interface */
void CVmHandleManager::delete_obj()
{
    size_t i;

    /* 
     *   Delete each remaining object.  Note that we need to call the virtual
     *   delete_handle_object routine, so we must do this before reaching the
     *   destructor (once in the base class destructor, we no longer have
     *   access to the subclass virtuals).  
     */
    for (i = 0 ; i < handles_max_ ; ++i)
    {
        /* if this banner is still valid, delete it */
        if (handles_[i] != 0)
            delete_handle_object(i + 1, handles_[i]);
    }

    /* delete the object */
    delete this;
}

/* destructor */
CVmHandleManager::~CVmHandleManager()
{
    /* delete the handle pointer array */
    t3free(handles_);
}

/* 
 *   Allocate a new handle 
 */
int CVmHandleManager::alloc_handle(void *item)
{
    size_t slot;

    /* scan for a free slot */
    for (slot = 0 ; slot < handles_max_ ; ++slot)
    {
        /* if this one is free, use it */
        if (handles_[slot] == 0)
            break;
    }

    /* if we didn't find a free slot, extend the array */
    if (slot == handles_max_)
    {
        size_t i;

        /* allocate a larger array */
        handles_max_ += 32;
        handles_ = (void **)
                   t3realloc(handles_, handles_max_ * sizeof(*handles_));

        /* clear out the newly-allocated slots */
        for (i = slot ; i < handles_max_ ; ++i)
            handles_[i] = 0;
    }

    /* store the new item in our pointer array */
    handles_[slot] = item;

    /* 
     *   convert the slot number to a handle by adjusting it to a 1-based
     *   index, and return the result 
     */
    return slot + 1;
}


/* ------------------------------------------------------------------------ */
/*
 *   Banner manager 
 */

/*
 *   Create a banner 
 */
int CVmBannerManager::create_banner(VMG_ int parent_id,
                                    int where, int other_id,
                                    int wintype, int align,
                                    int siz, int siz_units,
                                    unsigned long style)
{
    void *handle;
    void *parent_handle;
    void *other_handle;
    CVmConsoleBanner *item;

    /* get the parent handle, if provided */
    parent_handle = get_os_handle(parent_id);

    /* get the 'other' handle, if we need it for the 'where' */
    switch(where)
    {
    case OS_BANNER_BEFORE:
    case OS_BANNER_AFTER:
        /* retrieve the handle for the other_id */
        other_handle = get_os_handle(other_id);
        break;

    default:
        /* we don't need 'other' for other 'where' modes */
        other_handle = 0;
        break;
    }

    /* try creating the OS-level banner window */
    handle = os_banner_create(parent_handle, where, other_handle, wintype,
                              align, siz, siz_units, style);

    /* if we couldn't create the OS-level window, return failure */
    if (handle == 0)
        return 0;

    /* create the new console */
    item = new CVmConsoleBanner(handle, wintype, style);

    /* allocate a handle for the new banner, and return the handle */
    return alloc_handle(item);
}

/*
 *   Delete or orphan a banner window.  Deleting and orphaning both sever
 *   all ties from the banner manager (and thus from the T3 program) to the
 *   banner.  Deleting a banner actually gets deletes it at the OS level;
 *   orphaning the banner severs our ties, but hands the banner over to the
 *   OS to do with as it pleases.  On some implementations, the OS will
 *   continue to display the banner after it's orphaned to allow the final
 *   display configuration to remain visible even after the program has
 *   terminated.  
 */
void CVmBannerManager::delete_or_orphan_banner(int banner, int orphan)
{
    CVmConsoleBanner *item;
    void *handle;

    /* if the banner is invalid, ignore the request */
    if ((item = (CVmConsoleBanner *)get_object(banner)) == 0)
        return;

    /* get the OS-level banner handle */
    handle = item->get_os_handle();

    /* delete the banner item */
    delete item;

    /* clear the slot */
    clear_handle(banner);

    /* delete the OS-level banner */
    if (orphan)
        os_banner_orphan(handle);
    else
        os_banner_delete(handle);
}

/*
 *   Get the OS-level handle for the given banner 
 */
void *CVmBannerManager::get_os_handle(int banner)
{
    CVmConsoleBanner *item;

    /* if the banner is invalid, return failure */
    if ((item = (CVmConsoleBanner *)get_object(banner)) == 0)
        return 0;

    /* return the handle from the slot */
    return item->get_os_handle();
}

/*
 *   Flush all banners 
 */
void CVmBannerManager::flush_all(VMG_ vm_nl_type nl)
{
    size_t slot;

    /* flush each banner */
    for (slot = 0 ; slot < handles_max_ ; ++slot)
    {
        /* if this slot has a valid banner, flush it */
        if (handles_[slot] != 0)
            ((CVmConsoleBanner *)handles_[slot])->flush(vmg_ nl);
    }
}

/* ------------------------------------------------------------------------ */
/*
 *   Banner Window Console 
 */
CVmConsoleBanner::CVmConsoleBanner(void *banner_handle, int win_type,
                                   unsigned long style)
{
    CVmFormatterBanner *str;
    os_banner_info_t info;
    int obey_whitespace = FALSE;
    int literal_mode = FALSE;

    /* remember our OS-level banner handle */
    banner_ = banner_handle;

    /* get osifc-level information on the banner */
    if (!os_banner_getinfo(banner_, &info))
        info.os_line_wrap = FALSE;

    /* 
     *   If it's a text grid window, don't do any line wrapping.  Text grids
     *   simply don't have any line wrapping, so we don't want to impose any
     *   at the formatter level.  Set the formatter to "os line wrap" mode,
     *   to indicate that the formatter doesn't do wrapping - even though
     *   the underlying OS banner window won't do any wrapping either, the
     *   lack of line wrapping counts as OS handling of line wrapping.  
     */
    if (win_type == OS_BANNER_TYPE_TEXTGRID)
    {
        /* do not wrap lines in the formatter */
        info.os_line_wrap = TRUE;

        /* use literal mode, and obey whitespace literally */
        literal_mode = TRUE;
        obey_whitespace = TRUE;
    }

    /* create and initialize our display stream */
    disp_str_ = str = new CVmFormatterBanner(banner_handle, this,
                                             win_type, style);
    str->init(info.os_line_wrap, obey_whitespace, literal_mode);

    /* remember our window type */
    win_type_ = win_type;
}

/*
 *   Deletion 
 */
CVmConsoleBanner::~CVmConsoleBanner()
{
    /* delete our display stream */
    delete disp_str_;
}

/*
 *   Clear the banner window 
 */
void CVmConsoleBanner::clear_window(VMG0_)
{
    /* flush and empty our output buffer */
    flush(vmg_ VM_NL_NONE);
    empty_buffers(vmg0_);
    
    /* clear our underlying system banner */
    os_banner_clear(banner_);
    
    /* tell our display stream to zero its line counter */
    disp_str_->reset_line_count(TRUE);
}

/*
 *   Get banner information 
 */
int CVmConsoleBanner::get_banner_info(os_banner_info_t *info)
{
    int ret;
    
    /* get the OS-level information */
    ret = os_banner_getinfo(banner_, info);

    /* make some adjustments if we got valid information back */
    if (ret)
    {
        /* 
         *   check the window type for further adjustments we might need to
         *   make to the data returned from the OS layer 
         */
        switch(win_type_)
        {
        case OS_BANNER_TYPE_TEXTGRID:
            /* 
             *   text grids don't support <TAB> alignment, even if the
             *   underlying OS banner says we do, because we simply don't
             *   support <TAB> (or any other HTML markups) in a text grid
             *   window 
             */
            info->style &= ~OS_BANNER_STYLE_TAB_ALIGN;
            break;

        default:
            /* other types don't require any adjustments */
            break;
        }
    }

    /* return the success indication */
    return ret;
}

/* ------------------------------------------------------------------------ */
/*
 *   Log file console manager 
 */

/*
 *   create a log console 
 */
int CVmLogConsoleManager::create_log_console(const char *fname,
                                             osfildef *fp,
                                             class CCharmapToLocal *cmap,
                                             int width)
{
    CVmConsoleLog *con;
    
    /* create the new console */
    con = new CVmConsoleLog(fname, fp, cmap, width);

    /* allocate a handle for the new console and return the handle */
    return alloc_handle(con);
}

/*
 *   delete log a console 
 */
void CVmLogConsoleManager::delete_log_console(int handle)
{
    CVmConsoleLog *con;
    
    /* if the handle is invalid, ignore the request */
    if ((con = (CVmConsoleLog *)get_object(handle)) == 0)
        return;

    /* delete the console */
    delete con;

    /* clear the slot */
    clear_handle(handle);
}

/* ------------------------------------------------------------------------ */
/*
 *   Log file console 
 */
CVmConsoleLog::CVmConsoleLog(const char *fname, osfildef *fp,
                             class CCharmapToLocal *cmap, int width)
{
    CVmFormatterLog *str;

    /* create our display stream */
    disp_str_ = str = new CVmFormatterLog(this, width);

    /* set the file */
    str->set_log_file(fname, fp);

    /* set the character mapper */
    str->set_charmap(cmap);
}

/*
 *   destroy 
 */
CVmConsoleLog::~CVmConsoleLog()
{
    /* delete our display stream */
    delete disp_str_;
}

