/* 
 *   Copyright (c) 2002 by Michael J. Roberts.  All Rights Reserved.
 *   
 *   Please see the accompanying license file, LICENSE.TXT, for information
 *   on using and copying this software.  
 */
/*
Name
  vmcfgfl.cpp - "flat" constant pool configuration
Function
  
Notes
  
Modified
  09/19/02 MJRoberts  - Creation
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
    vm_init_flat(vmg, hostifc, clientifc, charset);
}

