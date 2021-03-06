/* 
 *   Copyright (c) 2002 by Michael J. Roberts.  All Rights Reserved.
 *   
 *   Please see the accompanying license file, LICENSE.TXT, for information
 *   on using and copying this software.  
 */
/*
Name
  vmhosttx.cpp - text-only host interface implementation
Function
  Provides a base class for the T3 VM Host Interface for implementing
  text-only applications.
Notes
  
Modified
  06/16/02 MJRoberts  - Creation
*/

#include "os.h"
#include "t3std.h"
#include "vmhash.h"
#include "vmhost.h"
#include "vmhosttx.h"

/* ------------------------------------------------------------------------ */
/*
 *   Hash table entry for a resource descriptor 
 */
class CResEntry: public CVmHashEntryCI
{
public:
    CResEntry(const char *resname, size_t resnamelen, int copy,
              unsigned long ofs, unsigned long siz, int fileno)
        : CVmHashEntryCI(resname, resnamelen, copy)
    {
        /* remember the file locator information */
        fileno_ = fileno;
        ofs_ = ofs;
        siz_ = siz;
    }

    /* file number (this is an index in the hostifc's ext_ array) */
    int fileno_;

    /* byte offset of the start of the resource data in the file */
    unsigned long ofs_;

    /* byte size of the resource data */
    unsigned long siz_;
};

/* ------------------------------------------------------------------------ */
/*
 *   construction 
 */
CVmHostIfcText::CVmHostIfcText()
{
    /* create our hash table */
    restab_ = new CVmHashTable(128, new CVmHashFuncCI(), TRUE);

    /* allocate an initial set of external resource filename entries */
    ext_max_ = 10;
    ext_ = (char **)t3malloc(ext_max_ * sizeof(ext_[0]));

    /* we use slot zero for the image filename, which we don't know yet */
    ext_cnt_ = 1;
    ext_[0] = 0;

    /* no resource directory specified yet */
    res_dir_ = 0;
}

/*
 *   deletion 
 */
CVmHostIfcText::~CVmHostIfcText()
{
    size_t i;

    /* delete our hash table */
    delete restab_;

    /* delete our external filenames */
    for (i = 0 ; i < ext_cnt_ ; ++i)
        lib_free_str(ext_[i]);

    /* delete our array of external filename entries */
    t3free(ext_);

    /* delete our resource directory path */
    lib_free_str(res_dir_);
}

/* 
 *   set the image file name - we always use resource file slot zero to
 *   store the image file 
 */
void CVmHostIfcText::set_image_name(const char *fname)
{
    /* free any old name string */
    lib_free_str(ext_[0]);

    /* remember the new name */
    ext_[0] = lib_copy_str(fname);
}

/*
 *   set the resource directory 
 */
void CVmHostIfcText::set_res_dir(const char *dir)
{
    /* forget any previous setting, and remember the new path */
    lib_free_str(res_dir_);
    res_dir_ = lib_copy_str(dir);
}

/* 
 *   add a resource file 
 */
int CVmHostIfcText::add_resfile(const char *fname)
{
    /* expand the resource file list if necessary */
    if (ext_cnt_ == ext_max_)
    {
        /* bump up the maximum a bit */
        ext_max_ += 10;

        /* reallocate the entry pointer array */
        ext_ = (char **)t3realloc(ext_, ext_max_ * sizeof(ext_[0]));
    }

    /* store the new entry */
    ext_[ext_cnt_++] = lib_copy_str(fname);

    /* 
     *   return the new entry's file number (we've already bumped the index,
     *   so it's the current count minus one) 
     */
    return ext_cnt_ - 1;
}

/* 
 *   add a resource 
 */
void CVmHostIfcText::add_resource(unsigned long ofs, unsigned long siz,
                                  const char *resname, size_t resnamelen,
                                  int fileno)
{
    CResEntry *entry;
    
    /* create a new entry desribing the resource */
    entry = new CResEntry(resname, resnamelen, TRUE, ofs, siz, fileno);

    /* add it to the table */
    restab_->add(entry);
}

/* 
 *   find a resource 
 */
osfildef *CVmHostIfcText::find_resource(const char *resname,
                                        size_t resnamelen,
                                        unsigned long *res_size)
{
    CResEntry *entry;
    osfildef *fp;

    /* try finding an entry in the resource map */
    entry = (CResEntry *)restab_->find(resname, resnamelen);
    if (entry != 0)
    {
        /* found it - open the file */
        fp = osfoprb(ext_[entry->fileno_], OSFTBIN);

        /* if that succeeded, seek to the start of the resource */
        if (fp != 0)
            osfseek(fp, entry->ofs_, OSFSK_SET);

        /* tell the caller the size of the resource */
        *res_size = entry->siz_;

        /* return the file handle */
        return fp;
    }
    else
    {
        char buf[OSFNMAX];
        char fname[OSFNMAX];
        char path[OSFNMAX];
        
        /* 
         *   There's no entry in the resource map, so convert the resource
         *   name from the URL notation to local file system conventions,
         *   and look for a file with the given name.
         *   
         *   First, make a null-terminated copy of the resource name,
         *   limiting the copy to our buffer size.  
         */
        if (resnamelen > sizeof(buf) - 1)
            resnamelen = sizeof(buf) - 1;
        memcpy(buf, resname, resnamelen);
        buf[resnamelen] = '\0';

        /* convert the resource name to a URL */
        os_cvt_url_dir(fname, sizeof(fname), buf, FALSE);

        /* if there's no image file, we can't proceed */
        if (ext_[0] == 0)
            return 0;

        /* 
         *   get the path to the image file, since external resources are
         *   always relative to the image file 
         */
        os_get_path_name(path, sizeof(path), ext_[0]);

        /* 
         *   build the full path name by combining the image file path with
         *   the relative path we got from the resource name URL, as
         *   converted local file system conventions 
         */
        os_build_full_path(buf, sizeof(buf), path, fname);

        /* try opening the file */
        fp = osfoprb(buf, OSFTBIN);

        /* return failure if we couldn't find the file */
        if (fp == 0)
            return 0;

        /* 
         *   the entire file is the resource data, so figure out how big the
         *   file is, and tell the caller that the resource size is the file
         *   size 
         */
        osfseek(fp, 0, OSFSK_END);
        *res_size = osfpos(fp);

        /* 
         *   seek back to the start of the resource data (which is simply
         *   the start of the file, since the entire file is the resource
         *   data) 
         */
        osfseek(fp, 0, OSFSK_SET);

        /* return the file handle */
        return fp;
    }
}

/* 
 *   determine if a resource exists 
 */
int CVmHostIfcText::resfile_exists(const char *resname, size_t resnamelen)
{
    osfildef *fp;
    unsigned long res_size;

    /* try opening the resource file */
    fp = find_resource(resname, resnamelen, &res_size);

    /* check to see if we successfully opened the resource */
    if (fp != 0)
    {
        /* found it - close the file and return success */
        osfcls(fp);
        return TRUE;
    }
    else
    {
        /* couldn't find it - indicate failure */
        return FALSE;
    }
}

