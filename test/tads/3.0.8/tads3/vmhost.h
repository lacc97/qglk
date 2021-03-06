/* $Header$ */

/* 
 *   Copyright (c) 1999, 2002 Michael J. Roberts.  All Rights Reserved.
 *   
 *   Please see the accompanying license file, LICENSE.TXT, for information
 *   on using and copying this software.  
 */
/*
Name
  vmhost.h - T3 VM host interface
Function
  The host interface defines a set of services that the VM obtains from
  its host application environment.  The VM uses the host interface to
  obtain these services so that the VM doesn't have to make too many
  assumptions about the larger application of which it is a subsystem.
  Every application containing the VM must provide the VM with an
  implementation of this interface.
Notes
  
Modified
  07/29/99 MJRoberts  - Creation
*/

#ifndef VMHOST_H
#define VMHOST_H

/* ------------------------------------------------------------------------ */
/*
 *   I/O Safety Levels.  These are defined as integers, not an enum,
 *   because they form a hierarchy; a higher value imposes all of the
 *   restrictions of all lower values, plus additional restrictions of its
 *   own.  
 */

/* level 0: minimum safety; read/write any file */
const int VM_IO_SAFETY_MINIMUM = 0;

/* level 1: read any file, write only to files in the current directory */
const int VM_IO_SAFETY_READ_ANY_WRITE_CUR = 1;

/* level 2: read/write in current directory only */
const int VM_IO_SAFETY_READWRITE_CUR = 2;

/* level 3: read from current directory only, no writing allowed */
const int VM_IO_SAFETY_READ_CUR = 3;

/* level 4: maximum safety; no file reading or writing allowed */
const int VM_IO_SAFETY_MAXIMUM = 4;


/* ------------------------------------------------------------------------ */
/*
 *   get_image_name return codes 
 */
enum vmhost_gin_t
{
    /* 
     *   get_image_name() call ignored - this is returned if the host
     *   system doesn't have a way of asking the user for an image name,
     *   so the caller must rely on whatever other way it has of finding
     *   the image name (which usually means that it can't find a image
     *   name at all, given that it would only call get_image_name() when
     *   it can't otherwise find the name) 
     */
    VMHOST_GIN_IGNORED,

    /* 
     *   user chose not to supply an image name - this is returned when,
     *   for example, the user cancelled out of a file selector dialog 
     */
    VMHOST_GIN_CANCEL,

    /* 
     *   error - the host system can't prompt for a filename because of
     *   some kind of error (not enough memory to load a dialog, for
     *   example) 
     */
    VMHOST_GIN_ERROR,

    /* success */
    VMHOST_GIN_SUCCESS
};

/* ------------------------------------------------------------------------ */
/*
 *   T3 VM Host Application Interface.  This is an abstract class; it must
 *   be implemented by each application that embeds the VM. 
 */
class CVmHostIfc
{
public:
    virtual ~CVmHostIfc() { }
  
    /* 
     *   Get the file I/O safety level.  This allows the host application
     *   to control the file operations that a program running under the
     *   VM may perform.  See the VM_IO_SAFETY_xxx values above.  
     */
    virtual int get_io_safety() = 0;

    /* 
     *   set the I/O safety level - this should only be done in response
     *   to a user preference setting (such as via a command-line option),
     *   never as a result of some programmatic operation by the executing
     *   image 
     */
    virtual void set_io_safety(int level) = 0;

    /*
     *   Get the resource loader for character mapping tables.  This
     *   resource loader should be set up to load resources out of the VM
     *   executable or out of the directory containing the VM executable,
     *   since these resources are associated with the VM itself, not the
     *   T3 program executing under the VM.  
     */
    virtual class CResLoader *get_cmap_res_loader() = 0;

    /*
     *   Set the image file name.  The VM calls this after it learns the
     *   name of the image file so that the host system can access the
     *   file if necessary. 
     */
    virtual void set_image_name(const char *fname) = 0;

    /*
     *   Set the root directory path for individual resources (such as
     *   individual image and sound resources) that we don't find in the
     *   image file or any attached resource collection file.  If this is
     *   never called, the directory containing the image file should be used
     *   as the resource root directory.  
     */
    virtual void set_res_dir(const char *fname) = 0;

    /*
     *   Add a resource collection file.  The return value is a non-zero file
     *   number assigned by the host system; the VM uses this number in
     *   subsequent calls to add_resource() to add resources from this file.
     *   The VM cannot add any resources for a file until it first adds the
     *   file with this routine.  
     */
    virtual int add_resfile(const char *fname) = 0;

    /* 
     *   Determine if additional resource files are supported - if this
     *   returns true, add_resfile() can be used, otherwise add_resfile()
     *   will have no effect.
     */
    virtual int can_add_resfiles() = 0;

    /*
     *   Add a resource map entry.  'ofs' is the byte offset of the start
     *   of the resource within the file, and 'siz' is the size in bytes
     *   of the resource data.  The resource is stored as contiguous bytes
     *   starting at the given file offset and running for the given size.
     *   The 'fileno' is zero for the image file, or the number assigned
     *   by the host system in add_resfile() for other resource files.  
     */
    virtual void add_resource(unsigned long ofs, unsigned long siz,
                              const char *res_name, size_t res_name_len,
                              int fileno) = 0;

    /*
     *   Find a resource.  Returns an osfildef* handle to the open resource
     *   file if the resource can be found; the file on return will have its
     *   seek position set to the first byte of the resource in the file,
     *   and *res_size will be set to the size in bytes of the resource
     *   data.  If there is no such resource, returns null.  
     */
    virtual osfildef *find_resource(const char *resname, size_t resname_len,
                                    unsigned long *res_size) = 0;

    /*
     *   Get the external resource file path.  If we should look for
     *   resource files in a different location than the image file, the
     *   host system can set this to a path that we should use to look for
     *   resource files.  If this is null, the VM should simply look in
     *   the same directory that contains the image file.  If the host
     *   system provides a path via this routine, the path string must
     *   have a trailing separator character if required, so that we can
     *   directly append a name to this path to form a valid
     *   fully-qualified filename.  
     */
    virtual const char *get_res_path() = 0;

    /*
     *   Determine if a resource exists.  Returns true if so, false if
     *   not.  
     */
    virtual int resfile_exists(const char *res_name, size_t res_name_len) = 0;

    /*
     *   Ask the user to supply an image file name.  We'll call this
     *   routine only if we can't obtain an image file from command line
     *   arguments or from an attachment to the executable.
     */
    virtual vmhost_gin_t get_image_name(char *buf, size_t buflen) = 0;
};

#endif /* VMHOST_H */

