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
  vmbt3_d.cpp - T3 VM system interface function set - debugger version
Function
  
Notes
  
Modified
  03/10/00 MJRoberts  - Creation
*/

#include "t3std.h"
#include "vmtype.h"
#include "vmbif.h"
#include "vmbift3.h"
#include "vmstack.h"
#include "vmdbg.h"

/*
 *   Debug Trace 
 */
void CVmBifT3::debug_trace(VMG_ uint argc)
{
    /* make sure we have at least one argument */
    if (argc < 1)
        err_throw(VMERR_WRONG_NUM_OF_ARGS);
    
    /* pop the flags and see what we're being asked to do */
    switch(pop_int_val(vmg0_))
    {
    case T3DBG_CHECK:
        /* check arguments */
        check_argc(vmg_ argc, 1);
        
        /* we're just being asked if the debugger is present - it is */
        retval_true(vmg0_);
        break;

    case T3DBG_BREAK:
        /* check arguments */
        check_argc(vmg_ argc, 1);

        /* tell the debugger to activate single-step mode */
        G_debugger->set_step_in();

        /* tell the caller we were successful */
        retval_true(vmg0_);
        break;

    default:
        /* anything else just returns nil, to allow for future expansion */
        G_stk->discard(argc - 1);
        retval_nil(vmg0_);
        break;
    }
}

