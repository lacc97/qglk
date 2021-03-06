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
  rcmain.cpp - T3 resource compiler main
Function
  
Notes
  
Modified
  01/03/00 MJRoberts  - Creation
*/


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>

#include "os.h"
#include "t3std.h"
#include "rcmain.h"
#include "vmimage.h"


/* 
 *   copy a block of bytes from the input file to the output file 
 */
static int copy_file_bytes(osfildef *fpin, osfildef *fpout, ulong siz)
{
    static char copybuf[16 * 1024];
    size_t cursiz;

    /* copy bytes until we run out */
    while (siz != 0)
    {
        /* we can copy up to one full buffer at a time */
        cursiz = (siz > sizeof(copybuf) ? sizeof(copybuf) : (size_t)siz);

        /* deduct the amount we're copying from the total */
        siz -= cursiz;

        /* read from input, copy to output */
        if (osfrb(fpin, copybuf, cursiz)
            || osfwb(fpout, copybuf, cursiz))
            return 1;
    }

    /* success */
    return 0;
}

/*
 *   Add resources 
 */
int CResCompMain::add_resources(const char *image_fname,
                                const class CRcResList *reslist,
                                class CRcHostIfc *hostifc,
                                int create_new, os_filetype_t file_type)
{
    osfildef *fp = 0;
    osfildef *resfp = 0;
    char buf[OSFNMAX + 128];
    long mres_seek;
    long mres_size;
    CRcResEntry *entry;
    long ofs;
    long contents_siz;

    /* 
     *   if the file doesn't exist, and we're not creating a new file,
     *   it's an error 
     */
    if (osfacc(image_fname) && !create_new)
    {
        /* we can't create a new file - display an error and give up */
        disp_error(hostifc, "image file \"%.*s\" does not exist",
                   (int)OSFNMAX, image_fname);
        goto ret_error;
    }

    /* if we're creating a new file, write the header */
    if (create_new)
    {
        time_t timer;
        struct tm *tblock;
            
        /* create a new image file */
        fp = osfopwb(image_fname, file_type);
        if (fp == 0)
        {
            disp_error(hostifc, "can't create file \"%.*s\"",
                       (int)OSFNMAX, image_fname);
            goto ret_error;
        }

        /* set the version ID in the header */
        oswp2(buf, 1);

        /* set the reserved bytes in the header */
        memset(buf + 2, 0, 32);

        /* set the timestamp in the header */
        timer = time(NULL);
        tblock = localtime(&timer);
        memcpy(buf + 2 + 32, asctime(tblock), 24);

        /* set up an EOF block */
        memcpy(buf + 2 + 32 + 24, "EOF ", 4);
        memset(buf + 2 + 32 + 24 + 4, 0, 4);
        oswp2(buf + 2 + 32 + 24 + 4 + 4, VMIMAGE_DBF_MANDATORY);

        /* write the header and the EOF block */
        if (osfwb(fp, VMIMAGE_SIG, sizeof(VMIMAGE_SIG)-1)
            || osfwb(fp, buf, 2 + 32 + 24 + 10))
        {
            disp_error(hostifc, "%.*s: error writing new file header",
                       (int)OSFNMAX, image_fname);
            goto ret_error;
        }

        /* done with the file - close it for now */
        osfcls(fp);
        fp = 0;
    }

    /* 
     *   open the file for reading and writing, since we'll need to read
     *   through the current contents then write our new data after the
     *   end of the existing file 
     */
    fp = osfoprwb(image_fname, file_type);
    if (fp == 0)
    {
        /* display an error and give up */
        disp_error(hostifc, "can't open file \"%.*s\"",
                   (int)OSFNMAX, image_fname);
        goto ret_error;
    }

    /* read and verify the header */
    if (osfrb(fp, buf, sizeof(VMIMAGE_SIG)-1 + 2 + 32 + 24)
        || memcmp(buf, VMIMAGE_SIG, sizeof(VMIMAGE_SIG)-1) != 0)
    {
        disp_error(hostifc, "%.*s: invalid image file header",
                   (int)OSFNMAX, image_fname);
        goto ret_error;
    }

    /* read and skip the blocks until we reach the "EOF" block */
    for (;;)
    {
        long block_siz;

        /* read the next block */
        if (osfrb(fp, buf, 10))
        {
            disp_error(hostifc, "%.*s: unexpected end of file",
                       (int)OSFNMAX, image_fname);
            goto ret_error;
        }

        /* if it's EOF, we're done */
        if (memcmp(buf, "EOF ", 4) == 0)
            break;
            
        /* read the size of this block */
        block_siz = osrp4(buf + 4);

        /* skip past this block */
        osfseek(fp, block_siz, OSFSK_CUR);
    }

    /* 
     *   we've found the EOF block - make sure we're at the end of the
     *   file 
     */
    mres_seek = osfpos(fp);
    osfseek(fp, 0, OSFSK_END);
    if (mres_seek != osfpos(fp))
    {
        disp_error(hostifc, "%.*s: extra data after end of file",
                   (int)OSFNMAX, image_fname);
        goto ret_error;
    }

    /* 
     *   seek back to the start of the EOF block, so that we can overwrite
     *   it with a new MRES block 
     */
    mres_seek -= 10;
    osfseek(fp, mres_seek, OSFSK_SET);

    /* 
     *   prepare and write the MRES block header, plus the resource entry
     *   count 
     */
    memcpy(buf, "MRES", 4);
    memset(buf + 4, 0, 6);
    oswp2(buf + 10, reslist->get_count());
    if (osfwb(fp, buf, 10 + 2))
    {
        disp_error(hostifc, "%.*s: error writing resource block header",
                   (int)OSFNMAX, image_fname);
        goto ret_error;
    }

    /* 
     *   First, figure out how much space the table of contents itself
     *   will take up.  We need this information so we will know where the
     *   first resource's binary data stream begins.  Note that the
     *   contents offset starts at 2, since the table entry count (a
     *   UINT2) comes before the table's first entry.  
     */
    for (contents_siz = 2, entry = reslist->get_head() ; entry != 0 ;
         entry = entry->get_next())
    {
        /* 
         *   each entry in the table of contents requires a UINT4 (offset
         *   of the data), UINT4 (size of the data), UBYTE (name length),
         *   and the bytes for the name itself 
         */
        contents_siz += 4 + 4 + 1 + strlen(entry->get_url());
    }

    /* build the table of contents */
    for (ofs = contents_siz, entry = reslist->get_head() ; entry != 0 ;
         entry = entry->get_next())
    {
        long res_size;
        size_t url_len;
        char *p;
        size_t rem;
        
        /* open this resource file */
        resfp = osfoprb(entry->get_fname(), OSFTBIN);
        if (resfp == 0)
        {
            disp_error(hostifc, "%.*s: cannot open resource file \"%.*s\"",
                       (int)OSFNMAX, image_fname,
                       (int)OSFNMAX, entry->get_fname());
            goto ret_error;
        }

        /* 
         *   seek to the end of the resource file so we can determine its
         *   size 
         */
        osfseek(resfp, 0, OSFSK_END);
        res_size = osfpos(resfp);

        /* if the entry name is too long, it's an error */
        url_len = strlen(entry->get_url());
        if (url_len > 255)
        {
            disp_error(hostifc,
                       "%.*s: resource name \"%.*s\" for file \"%.*s\" "
                       "is too long",
                       (int)OSFNMAX, image_fname,
                       (int)OSFNMAX, entry->get_url(),
                       (int)OSFNMAX, entry->get_fname());
            goto ret_error;
        }

        /* build this table entry */
        oswp4(buf, ofs);
        oswp4(buf + 4, res_size);
        buf[8] = url_len;
        memcpy(buf + 9, entry->get_url(), url_len);

        /* mask the resource name by xor'ing each byte with 0xff */
        for (p = buf + 9, rem = url_len ; rem != 0 ; --rem, ++p)
            *p ^= 0xFF;

        /* write the entry */
        if (osfwb(fp, buf, 9 + url_len))
        {
            disp_error(hostifc, "%.*s: error writing contents entry for "
                       "resource \"%.*s\"",
                       (int)OSFNMAX, image_fname,
                       (int)OSFNMAX, entry->get_fname());
            goto ret_error;
        }

        /* add the resource's size into the offset so far */
        ofs += res_size;

        /* we're done with this file for now */
        osfcls(resfp);
        resfp = 0;
    }

    /* now copy the resources themselves */
    for (entry = reslist->get_head() ; entry != 0 ;
         entry = entry->get_next())
    {
        long res_size;
        char msg[OSFNMAX*2 + 20];

        /* show what we're doing */
        sprintf(msg, "+ %.*s (%.*s)",
                (int)OSFNMAX, entry->get_fname(),
                (int)OSFNMAX, entry->get_url());
        hostifc->display_status(msg);

        /* open this resource file */
        resfp = osfoprb(entry->get_fname(), OSFTBIN);
        if (resfp == 0)
        {
            disp_error(hostifc,
                       "%.*s: cannot open resource file \"%.*s\"",
                       (int)OSFNMAX, image_fname,
                       (int)OSFNMAX, entry->get_fname());
            goto ret_error;
        }

        /* get the size of the file */
        osfseek(resfp, 0, OSFSK_END);
        res_size = osfpos(resfp);
        osfseek(resfp, 0, OSFSK_SET);

        /* copy the resource file's contents into the image file */
        if (copy_file_bytes(resfp, fp, res_size))
        {
            disp_error(hostifc, "%.*s: error copying resource file \"%.*s\"",
                       (int)OSFNMAX, image_fname,
                       (int)OSFNMAX, entry->get_fname());
            goto ret_error;
        }

        /* we're done with this file for now */
        osfcls(resfp);
        resfp = 0;
    }

    /* 
     *   calculate the size of the MRES data (excluding the 10-byte
     *   standard block header) 
     */
    mres_size = osfpos(fp) - mres_seek - 10;

    /* go back and fix up the MRES block header with the block size */
    osfseek(fp, mres_seek + 4, OSFSK_SET);
    oswp4(buf, mres_size);
    if (osfwb(fp, buf, 4))
    {
        disp_error(hostifc, "%.*s: error writing header size",
                   (int)OSFNMAX, image_fname);
        goto ret_error;
    }

    /* seek back to the end of the file */
    osfseek(fp, 0, OSFSK_END);

    /* set up an EOF block */
    memcpy(buf, "EOF ", 4);
    memset(buf + 4, 0, 4);
    oswp2(buf + 8, VMIMAGE_DBF_MANDATORY);

    /* write the EOF block */
    if (osfwb(fp, buf, 10))
    {
        disp_error(hostifc, "%.*s: error writing end-of-file block",
                   (int)OSFNMAX, image_fname);
        goto ret_error;
    }

    /* done with the file */
    osfcls(fp);
    fp = 0;

    /* success */
    return 0;

ret_error:
    /* close any files we opened */
    if (fp != 0)
        osfcls(fp);
    if (resfp != 0)
        osfcls(resfp);

    /* return an error indication */
    return 1;
}

/*
 *   Format and display an error message 
 */
void CResCompMain::disp_error(class CRcHostIfc *hostifc,
                              const char *msg, ...)
{
    char buf[1024];
    va_list argp;
    
    /* format the message into our buffer */
    va_start(argp, msg);
    vsprintf(buf, msg, argp);
    va_end(argp);

    /* display the formatted message */
    hostifc->display_error(buf);
}

/* ------------------------------------------------------------------------ */
/*
 *   Add a file or directory to a resource list 
 */
void CRcResList::add_file(const char *fname, const char *alias,
                          int recurse)
{
    char url[OSFNMAX];
    char search_file[OSFNMAX];
    int is_dir;
    void *search_ctx;
    
    /* 
     *   if no alias was specified, convert the filename to a URL and use
     *   that as the resource name; otherwise, use the alias without
     *   changes 
     */
    if (alias == 0)
    {
        os_cvt_dir_url(url, sizeof(url), fname, FALSE);
        alias = url;
    }

    /* 
     *   if this is a directory, add one entry for each item in the
     *   directory 
     */
    search_ctx = os_find_first_file("", fname,
                                    search_file, sizeof(search_file),
                                    &is_dir, 0, 0);
    if (search_ctx != 0 && is_dir)
    {
        char fullname[OSFNMAX];
        
        /* cancel the search - we only needed the one matching file */
        os_find_close(search_ctx);

        /* 
         *   search through the contents of the directory, and add each
         *   entry 
         */
        search_ctx = os_find_first_file(fname, 0,
                                        search_file, sizeof(search_file),
                                        &is_dir, fullname, sizeof(fullname));
        while (search_ctx != 0)
        {
            char full_url[OSFNMAX];
            size_t len;
            
            /* 
             *   build the full alias for this file path -- start with the
             *   the alias for the directory itself 
             */
            len = strlen(alias);
            memcpy(full_url, alias, len);

            /* 
             *   add a slash to separate the filename from the directory
             *   prefix, if the directory path alias doesn't already end
             *   in a slash 
             */
            if (len != 0 && full_url[len - 1] != '/')
                full_url[len++] = '/';

            /* add this file name */
            strcpy(full_url + len, search_file);

            /* check whether we found a file or directory */
            if (is_dir)
            {
                os_specfile_t spec_type;
                
                /* check for a special file */
                spec_type = os_is_special_file(search_file);
                
                /* 
                 *   It's a directory - if we're allowed to recurse, add
                 *   all of the directory's contents; otherwise simply
                 *   ignore it.
                 */
                if (recurse
                    && spec_type != OS_SPECFILE_SELF
                    && spec_type != OS_SPECFILE_PARENT)
                {
                    /* add the subdirectory with a recursive call */
                    add_file(fullname, full_url, TRUE);
                }
            }
            else
            {
                /* add a new entry for this file */
                add_element(new CRcResEntry(fullname, full_url));
            }

            /* get the next file */
            search_ctx = os_find_next_file(search_ctx,
                                           search_file, sizeof(search_file),
                                           &is_dir,
                                           fullname, sizeof(fullname));
        }
    }
    else
    {
        /* if the search succeeded, close it */
        if (search_ctx != 0)
            os_find_close(search_ctx);
        
        /* it's not a directory - simply add an entry for the file */
        add_element(new CRcResEntry(fname, alias));
    }
}

