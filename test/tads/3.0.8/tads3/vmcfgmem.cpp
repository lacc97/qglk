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
  vmcfgmem.cpp - T3 VM Configuration - in-memory (non-swapping) memory manager
Function
  
Notes
  
Modified
  10/08/99 MJRoberts  - Creation
*/

#include "vminit.h"
#include "vmglob.h"

/*
 *   initialize 
 */
void vm_initialize(struct vm_globals **vmg, class CVmHostIfc *hostifc,
                   class CVmMainClientIfc *clientifc,
                   const char *charset)
{
    vm_init_in_mem(vmg, hostifc, clientifc, charset);
}

