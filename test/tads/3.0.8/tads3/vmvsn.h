/* $Header$ */

/* 
 *   Copyright (c) 1999, 2002 Michael J. Roberts.  All Rights Reserved.
 *   
 *   Please see the accompanying license file, LICENSE.TXT, for information
 *   on using and copying this software.  
 */
/*
Name
  vmvsn.h - VM Version Information
Function
  
Notes
  
Modified
  07/12/99 MJRoberts  - Creation
*/

#ifndef VMVSN_H
#define VMVSN_H

/*
 *   The VM version number.  A VM program can obtain this value through
 *   the get_vm_vsn() function in the T3VM intrinsic function set.
 *   
 *   The value is encoded as a 32-bit value with the major version number
 *   in the high-order 16 bits, the minor version number in the next 8
 *   bits, and the patch release number in the low-order 8 bits.  So, the
 *   release 1.2.3 would be encoded as 0x00010203.  
 */
#define T3VM_VSN_NUMBER  0x00030008

/*
 *   The VM identification string 
 */
#define T3VM_IDENTIFICATION "mjr-T3"

/*
 *   The VM banner string.  A VM program can obtain this value through the
 *   get_vm_banner() function in the T3VM intrinsic function set. 
 */
/* copyright-date-string */
#define T3VM_BANNER_STRING \
    "T3 VM 3.0.8 - Copyright 1999, 2004 Michael J. Roberts"

/*
 *   The VM short version string.  This contains merely the version
 *   number, in display format.  
 */
#define T3VM_VSN_STRING "3.0.8"

#endif /* VMVSN_H */

