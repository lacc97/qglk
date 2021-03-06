/* $Header: d:/cvsroot/tads/tads3/VMERRNUM.H,v 1.3 1999/07/11 00:46:58 MJRoberts Exp $ */

/* 
 *   Copyright (c) 1998, 2002 Michael J. Roberts.  All Rights Reserved.
 *   
 *   Please see the accompanying license file, LICENSE.TXT, for information
 *   on using and copying this software.  
 */
/*
Name
  vmerrnum.h - VM error numbers
Function
  
Notes
  VM Error Numbers are in the range 1 to 9999.
Modified
  10/28/98 MJRoberts  - Creation
*/

#ifndef VMERRNUM_H
#define VMERRNUM_H


/* ------------------------------------------------------------------------ */
/*
 *   File errors 
 */

/* error reading file */
#define VMERR_READ_FILE           101

/* error writing file */
#define VMERR_WRITE_FILE          102

/* file not found */
#define VMERR_FILE_NOT_FOUND      103

/* error creating file */
#define VMERR_CREATE_FILE         104


/* ------------------------------------------------------------------------ */
/*
 *   Memory manager errors
 */

/* 
 *   requested object ID is already allocated to another object (this
 *   generally indicates that an image file or a saved state file contains
 *   invalid data) 
 */
#define VMERR_OBJ_IN_USE          201

/* out of memory */
#define VMERR_OUT_OF_MEMORY       202

/* out of memory allocating pool page (use a smaller in-memory cache) */
#define VMERR_NO_MEM_FOR_PAGE     203

/* invalid page size - must be a power of two */
#define VMERR_BAD_POOL_PAGE_SIZE  204

/* no more property ID's */
#define VMERR_OUT_OF_PROPIDS      205

/* circular initialization dependency in intrinsic class (internal error) */
#define VMERR_CIRCULAR_INIT       206


/* ------------------------------------------------------------------------ */
/*
 *   Image file errors 
 */

/* 
 *   Unsupported metaclass -- the image file requires a metaclass that
 *   this VM implementation does not provide.  This may indicate a version
 *   mismatch (a newer version of the VM is required), or that the image
 *   file is designed for a different host application environment.  
 */
#define VMERR_UNKNOWN_METACLASS   301

/*
 *   Unsupported function set "%s" -- the image file requires a function
 *   set that this VM implementation does not provide.  This may indicate
 *   a version mismatch (a newer version of the VM is required), or that
 *   the image file is designed for a different host application
 *   environment.  
 */
#define VMERR_UNKNOWN_FUNC_SET    302

/* attempted to read past end of image file */
#define VMERR_READ_PAST_IMG_END   303

/* not an image file (invalid signature) */
#define VMERR_NOT_AN_IMAGE_FILE   304

/* unrecognized block type in image file */
#define VMERR_UNKNOWN_IMAGE_BLOCK 305

/* data block too small (does not contain required data) */
#define VMERR_IMAGE_BLOCK_TOO_SMALL 306

/* ill-formed image file: pool page before pool definition */
#define VMERR_IMAGE_POOL_BEFORE_DEF 307

/* ill-formed image file: pool page out of range of definition */
#define VMERR_IMAGE_POOL_BAD_PAGE 308

/* invalid pool ID in image file */
#define VMERR_IMAGE_BAD_POOL_ID   309

/* attempted to load a pool page at an invalid offset */
#define VMERR_LOAD_BAD_PAGE_IDX   310

/* attempted to load a pool page that hasn't been defined in the image */
#define VMERR_LOAD_UNDEF_PAGE     311

/* ill-formed image file: pool is defined more than once */
#define VMERR_IMAGE_POOL_REDEF    312

/* file contains more than one metaclass dependency table */
#define VMERR_IMAGE_METADEP_REDEF 313

/* file does not contain a metaclass dependency table */
#define VMERR_IMAGE_NO_METADEP    314

/* file contains more than one than one function set dependency table */
#define VMERR_IMAGE_FUNCDEP_REDEF 315

/* file does not contain a function set dependency table */
#define VMERR_IMAGE_NO_FUNCDEP    316

/* file contains multiple entrypoint definitions */
#define VMERR_IMAGE_ENTRYPT_REDEF 317

/* file does not contain an entrypoint definition */
#define VMERR_IMAGE_NO_ENTRYPT    318

/* incompatible image file format */
#define VMERR_IMAGE_INCOMPAT_VSN  319

/* cannot execute - image contains no code */
#define VMERR_IMAGE_NO_CODE       320

/* image file has incompatible format: method header too old */
#define VMERR_IMAGE_INCOMPAT_HDR_FMT 321

/* 
 *   Unavailable intrinsic function called (index %d in function set "%s")
 *   -- this indicates that a function was invoked in a function set that
 *   provides only a subset of its functions.  This normally can occur
 *   only when running 'preinit' or similar special situations when
 *   intrinsics are resolved on each call rather than during the initial
 *   load. 
 */
#define VMERR_UNAVAIL_INTRINSIC   322

/* unknown internal metaclass ID %d */
#define VMERR_UNKNOWN_METACLASS_INTERNAL 323

/* page mask not allowed for in-memory image file */
#define VMERR_XOR_MASK_BAD_IN_MEM  324

/* no image file found in executable */
#define VMERR_NO_IMAGE_IN_EXE     325

/* object size exceeds hardware limits of this computer */
#define VMERR_OBJ_SIZE_OVERFLOW   326

/* metaclass "%s" version is not available (latest available is "%s") */
#define VMERR_METACLASS_TOO_OLD   327

/* incorrect metaclass data (corrupted image file) */
#define VMERR_INVAL_METACLASS_DATA 328

/* object cannot be created */
#define VMERR_BAD_STATIC_NEW      329

/* function set "%s" version is not available (latest available is "%s") */
#define VMERR_FUNCSET_TOO_OLD     330

/* invalid type for exported symbol %s */
#define VMERR_INVAL_EXPORT_TYPE   331

/* invalid data in image file macro definitions (code %d) */
#define VMERR_INVAL_IMAGE_MACRO   332

/* no "mainRestore" export - cannot restore saved state on startup */
#define VMERR_NO_MAINRESTORE      333


/* ------------------------------------------------------------------------ */
/*
 *   Property-related errors 
 */

/* this property cannot be set for this object */
#define VMERR_INVALID_SETPROP     1001


/* ------------------------------------------------------------------------ */
/*
 *   Saved state file errors 
 */

/* file is not a valid saved state file */
#define VMERR_NOT_SAVED_STATE     1201

/* file was not saved by the same image or image version */
#define VMERR_WRONG_SAVED_STATE   1202

/* metaclass name is too long in saved file */
#define VMERR_SAVED_META_TOO_LONG 1203

/* save/restore/reset is not valid in recursive VM invocation */
#define VMERR_RECURSIVE_VM_CALL 1204

/* stack overflow reading from saved state */
#define VMERR_SAVED_STACK_OVERFLOW 1205

/* object ID in saved state is invalid */
#define VMERR_SAVED_OBJ_ID_INVALID 1206

/* saved state file is corrupted (checksum does not match) */
#define VMERR_BAD_SAVED_STATE     1207

/* invalid data in saved metaclass */
#define VMERR_BAD_SAVED_META_DATA 1208

/* ------------------------------------------------------------------------ */
/*
 *   Data manipulation and conversion errors
 */

/* cannot convert value to a string */
#define VMERR_NO_STR_CONV         2001

/* conversion buffer overflow */
#define VMERR_CONV_BUF_OVF        2002

/* invalid datatype for "add" operator */
#define VMERR_BAD_TYPE_ADD        2003

/* numeric value required */
#define VMERR_NUM_VAL_REQD        2004

/* integer value required */
#define VMERR_INT_VAL_REQD        2005

/* cannot convert to logical value */
#define VMERR_NO_LOG_CONV         2006

/* invalid datatype for "subtract" operator */
#define VMERR_BAD_TYPE_SUB        2007

/* division by zero */
#define VMERR_DIVIDE_BY_ZERO      2008

/* invalid comparison */
#define VMERR_INVALID_COMPARISON  2009

/* object value required */
#define VMERR_OBJ_VAL_REQD        2010

/* property pointer value required */
#define VMERR_PROPPTR_VAL_REQD    2011

/* logical value required */
#define VMERR_LOG_VAL_REQD        2012

/* function pointer value required */
#define VMERR_FUNCPTR_VAL_REQD    2013

/* indexing operation cannot be applied to this datatype */
#define VMERR_CANNOT_INDEX_TYPE   2014

/* index value out of range */
#define VMERR_INDEX_OUT_OF_RANGE  2015

/* metaclass index out of range (probable image file error) */
#define VMERR_BAD_METACLASS_INDEX 2016

/* invalid dynamic object creation (metaclass does not support NEW) */
#define VMERR_BAD_DYNAMIC_NEW     2017

/* object value required for superclass */
#define VMERR_OBJ_VAL_REQD_SC     2018

/* string value required */
#define VMERR_STRING_VAL_REQD     2019

/* list value required */
#define VMERR_LIST_VAL_REQD       2020

/* list or string reference found in dictionary - can't convert to const */
#define VMERR_DICT_NO_CONST       2021

/* invalid object type */
#define VMERR_INVAL_OBJ_TYPE      2022

/* numeric overflow */
#define VMERR_NUM_OVERFLOW        2023

/* invalid datatype for "multiply" operator */
#define VMERR_BAD_TYPE_MUL        2024

/* invalid datatype for "divide" operator */
#define VMERR_BAD_TYPE_DIV        2025

/* invalid datatype for "negate" operator */
#define VMERR_BAD_TYPE_NEG        2026

/* value out of range */
#define VMERR_OUT_OF_RANGE        2027

/* string too large */
#define VMERR_STR_TOO_LONG        2028

/* list too large */
#define VMERR_LIST_TOO_LONG       2029

/* tree depth too large for equality test/hash calculation */
#define VMERR_TREE_TOO_DEEP_EQ    2030


/* ------------------------------------------------------------------------ */
/*
 *   Method and function invocation errors 
 */

/* wrong number of arguments */
#define VMERR_WRONG_NUM_OF_ARGS   2201

/* wrong number of arguments calling %s */
#define VMERR_WRONG_NUM_OF_ARGS_CALLING  2202

/* nil object reference */
#define VMERR_NIL_DEREF           2203


/* ------------------------------------------------------------------------ */
/*
 *   Object creation errors 
 */

/* 
 *   cannot create an instance of the object, because the object is not a
 *   class 
 */
#define VMERR_CANNOT_CREATE_INST  2270

/* class cannot be created */
#define VMERR_ILLEGAL_NEW         2271


/* ------------------------------------------------------------------------ */
/*
 *   General execution errors 
 */

/* invalid opcode */
#define VMERR_INVALID_OPCODE      2301

/* unhandled exception */
#define VMERR_UNHANDLED_EXC       2302

/* stack overflow */
#define VMERR_STACK_OVERFLOW      2303

/* invalid type for built-in function */
#define VMERR_BAD_TYPE_BIF        2304

/* default string output function not defined */
#define VMERR_SAY_IS_NOT_DEFINED  2305

/* invalid value for built-in function (value out of range) */
#define VMERR_BAD_VAL_BIF         2306

/* breakpoint encountered */
#define VMERR_BREAKPOINT          2307

/* external function calls not implemented */
#define VMERR_CALLEXT_NOT_IMPL    2308

/* invalid opcode modifier */
#define VMERR_INVALID_OPCODE_MOD  2309

/* no character mapping file available */
#define VMERR_NO_CHARMAP_FILE     2310

/* unhandled exception: %s */
#define VMERR_UNHANDLED_EXC_PARAM 2311

/* VM Error: %s */
#define VMERR_VM_EXC_PARAM        2312

/* VM Error: code %d */
#define VMERR_VM_EXC_CODE         2313

/* exception executing static initializer for %*s.%*s: %s */
#define VMERR_EXC_IN_STATIC_INIT  2314

/* 
 *   intrinsic class error (this is used as a fallback when an intrinsic
 *   class wants to throw an imported exception class, but the import
 *   doesn't exist; this doesn't provide any real information, but if the
 *   program doesn't export the required class then it's obviously not set
 *   up to receive any information anyway)
 *   
 *   this exception takes a string describing the error 
 */
#define VMERR_INTCLS_GENERAL_ERROR 2315


/* ------------------------------------------------------------------------ */
/*
 *   Debugger interface errors.  These are errors that the debugger throws
 *   in response to the corresponding UI commands, to allow the user some
 *   rough interactive control over execution in the debugger environment. 
 */

/* abort - abort current command */
#define VMERR_DBG_ABORT           2391

/* restart - restart program execution */
#define VMERR_DBG_RESTART         2392
   

/* ------------------------------------------------------------------------ */
/*
 *   TADS built-in function set errors 
 */

/* invalid file handle */
#define VMERR_BAD_FILE_HANDLE     2400

   
/* ------------------------------------------------------------------------ */
/*
 *   Debugger-related errors 
 */

/* invalid frame (in debug local/parameter operation) */
#define VMERR_BAD_FRAME           2500

/* invalid speculative evaluation */
#define VMERR_BAD_SPEC_EVAL       2501

/* invalid in debugger expression */
#define VMERR_INVAL_DBG_EXPR      2502

/* image file has no debugging information (must be recompiled) */
#define VMERR_NO_IMAGE_DBG_INFO   2503
   
/* ------------------------------------------------------------------------ */
/*
 *   BigNumber package errors 
 */
#define VMERR_BIGNUM_NO_REGS      2600

#endif /* VMERRNUM_H */

