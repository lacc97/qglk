/* $Header: d:/cvsroot/tads/TADS2/appctx.h,v 1.2 1999/05/17 02:52:14 MJRoberts Exp $ */

/* 
 *   Copyright (c) 1997, 2002 Michael J. Roberts.  All Rights Reserved.
 *   
 *   Please see the accompanying license file, LICENSE.TXT, for information
 *   on using and copying this software.  
 */
/*
Name
  appctx.h - Application container context definitions
Function
  Defines structures related to the TADS runtime application
Notes
  
Modified
   9/05/98 CNebel   - Spliced off from trd.h.
*/

#ifndef APPCTX_H
#define APPCTX_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 *   Application container context.  The TADS run-time is a subsystem that
 *   can be invoked from different types of applications; in fact, even
 *   when only the standard stand-alone run-time is considered, multiple
 *   application containers must be supported because of differences
 *   between operating systems.  The application container context is an
 *   optional mechanism that the main application can use to provide
 *   structured interaction between itself and the TADS run-time subsystem.
 *   
 *   The function pointers contained herein are intended to allow the
 *   run-time subsystem to call the host system to notify it of certain
 *   events, or obtain optional services from the host system.  Any of
 *   these function pointers can be null, in which case the run-time
 *   subsystem will skip calling them.
 *   
 *   Note that each function has an associated callback context.  This
 *   allows the host system to recover any necessary context information
 *   when the callback is invoked.  
 */
typedef struct appctxdef appctxdef;
struct appctxdef
{
    /*
     *   Get the .GAM file name.  The run-time will call this only if it
     *   can't find a game file to load through some other means first.
     *   The run-time determines the game file first by looking at the
     *   command line, then by checking to see if a .GAM file is attached
     *   to the executable.  If none of these checks yields a game, the
     *   run-time will call this routine to see if the host system wants
     *   to provide a game.  This routine can be implemented on a GUI
     *   system, for example, to display a dialog prompting the user to
     *   select a game file to open.  A trivial implementation of this
     *   routine (that merely returns false) is okay.
     *   
     *   This routine should return true (any non-zero value) if it
     *   provides the name of a file to open, false (zero) if not.  
     */
    int (*get_game_name)(void *appctxdat, char *buf, size_t buflen);
    void *get_game_name_ctx;

    /* 
     *   Set the .GAM file name.  When the run-time determines the name of
     *   the file it will use to read the game, it calls this routine.
     *   The host system should note the game filename if it will need to
     *   access the game file itself (for example, to load resources).  
     */
    void (*set_game_name)(void *appctxdat, const char *fname);
    void *set_game_name_ctx;

    /*
     *   Set the root path for individual resources.  By default, we use the
     *   directory containing the game file, but this can be used to override
     *   that.  
     */
    void (*set_res_dir)(void *appctxdat, const char *fname);
    void *set_res_dir_ctx;

    /*
     *   Set the resource map address in the game file.  If the .GAM
     *   reader encounters a resource map in the file, it calls this
     *   routine with the seek offset of the first resource.  Each
     *   resource's address is given as an offset from this point.
     *   
     *   fileno is the file number assigned by the host system in
     *   add_resfile.  File number zero is always the .GAM file.  
     */
    void (*set_resmap_seek)(void *appctxdat, unsigned long seekpos,
                            int fileno);
    void *set_resmap_seek_ctx;

    /*
     *   Add a resource entry.  The 'ofs' entry is the byte offset of the
     *   start of the resource, relative to the seek position previously
     *   set with set_resmap_seek.  'siz' is the size of the resource in
     *   bytes; the resource is stored as contiguous bytes starting at the
     *   given offset for the given size.  Note that resources may be
     *   added before the resource map seek position is set, so the host
     *   system must simply store the resource information for later use.
     *   The 'fileno' is zero for the .GAM file, or the number assigned by
     *   the host system in add_resfile for other resource files.  
     */
    void (*add_resource)(void *appctxdat, unsigned long ofs,
                         unsigned long siz, const char *nm, size_t nmlen,
                         int fileno);
    void *add_resource_ctx;

    /*
     *   Find a resource entry.  If the resource can be found, this must
     *   return an osfildef* handle to the resource, with its seek position
     *   set to the first byte of the resource data, and set *res_size to
     *   the size in bytes of the resource data in the file.  If the
     *   resource cannot be found, returns null.  
     */
    osfildef *(*find_resource)(void *appctxdat,
                               const char *resname, size_t resnamelen,
                               unsigned long *res_size);
    void *find_resource_ctx;

    /*
     *   Add a resource file.  The return value is a non-zero file number
     *   assigned by the host system; we'll use this number in subsequent
     *   calls to add_resource to add the resources from this file.
     *   
     *   After calling this routine to add the file, we'll parse the file
     *   and add any resources using add_resource.  
     */
    int (*add_resfile)(void *appctxdat, const char *fname);
    void *add_resfile_ctx;

    /*
     *   Determine if a resource exists.  Returns true if the resource can
     *   be loaded, false if not.  The resource name is in the standard
     *   URL-style format.  
     */
    int (*resfile_exists)(void *appctxdat, const char *res_name,
                          size_t res_name_len);
    void *resfile_exists_ctx;

    /*
     *   Resource file path.  If we should look for resource files in a
     *   different location than the .GAM file, the host system can set
     *   this to a path that we should use to look for resource files.  If
     *   it's null, we'll look in the directory that contains the .GAM
     *   file.  Note that if the path is provided, it must be set up with
     *   a trailing path separator character, so that we can directly
     *   append a name to this path to form a valid fully-qualified
     *   filename.  
     */
    const char *ext_res_path;

    /*
     *   File safety level get/set.  During initialization, we'll call the
     *   host system to tell it the file safety level selected by the user
     *   on the command line; if the host system is saving preference
     *   information, it should temporarily override its saved preferences
     *   and use the command line setting (and it may, if appropriate,
     *   want to save the command line setting as the saved preference
     *   setting, depending on how it handles preferences).  During
     *   execution, any time the game tries to open a file (using the
     *   fopen built-in function), we'll call the host system to ask it
     *   for the current setting, and use this new setting rather than the
     *   original command line setting.
     *   
     *   Refer to bif.c for information on the meanings of the file safety
     *   levels.  
     */
    void (*set_io_safety_level)(void *ctx, int level);
    int  (*get_io_safety_level)(void *ctx);
    void  *io_safety_level_ctx;

    /*
     *   Name of run-time application for usage messages.  If this is
     *   null, the default run-time application name will be used for
     *   usage messages. 
     */
    const char *usage_app_name;
};

#ifdef __cplusplus
}
#endif

#endif
