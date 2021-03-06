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
  vmhostsi.cpp - stdio-based VM host application environment
Function
  
Notes
  
Modified
  08/06/99 MJRoberts  - Creation
*/

#include "t3std.h"
#include "os.h"
#include "resload.h"
#include "vmhost.h"
#include "vmhostsi.h"

/*
 *   initialize 
 */
CVmHostIfcStdio::CVmHostIfcStdio(const char *argv0)
{
    char buf[OSFNMAX];


    /* 
     *   Create the resource loader for character mapping files in the
     *   same directory as the executable. 
     */
    os_get_special_path(buf, sizeof(buf), argv0, OS_GSP_T3_RES);
    cmap_loader_ = new CResLoader(buf);

    /* set the executable filename in the loader, if available */
    if (os_get_exe_filename(buf, sizeof(buf), argv0))
        cmap_loader_->set_exe_filename(buf);

    /* 
     *   the default safety level allows reading and writing to the current
     *   directory only 
     */
    io_safety_ = VM_IO_SAFETY_READWRITE_CUR;
}

/*
 *   delete 
 */
CVmHostIfcStdio::~CVmHostIfcStdio()
{
    /* delete our character map resource loader */
    delete cmap_loader_;
}
