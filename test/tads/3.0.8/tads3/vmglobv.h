/* 
 *   Copyright (c) 2002 by Michael J. Roberts.  All Rights Reserved.
 *   
 *   Please see the accompanying license file, LICENSE.TXT, for information
 *   on using and copying this software.  
 */
/*
Name
  vmglobv.h - global variable definitions
Function
  Defines the global variables for the T3 VM
Notes
  This file is NOT protected against multiple inclusion, because it's
  designed to be included more than once with different definitions for
  the macros.
Modified
  09/18/02 MJRoberts  - Creation
*/


VM_GLOBALS_BEGIN
   
    /* object memory manager */
    VM_GLOBAL_OBJDEF(class CVmMemory, mem)

    /* variable-size block heap manager */
    VM_GLOBAL_OBJDEF(class CVmVarHeap, varheap)

    /* object table */
    VM_GLOBAL_PREOBJDEF(class CVmObjTable, obj_table)

    /* undo manager */
    VM_GLOBAL_OBJDEF(class CVmUndo, undo)

    /* constant pool manager */
    VM_GLOBAL_PREOBJDEF(class CVmPool_CLASS, const_pool)

    /* code pool manager */
    VM_GLOBAL_PREOBJDEF(class CVmPool_CLASS, code_pool)

    /* the stack */
    VM_GLOBAL_PRECOBJDEF(class CVmStack, stk,
                         (VM_STACK_SIZE, vm_init_stack_reserve()))

    /* metaclass dependency table for loaded image file */
    VM_GLOBAL_OBJDEF(class CVmMetaTable, meta_table)

    /* built-in function set table */
    VM_GLOBAL_OBJDEF(class CVmBifTable, bif_table)

    /* source file list (for debugger) */
    VM_GLOBAL_OBJDEF(class CVmSrcfTable, srcf_table)

    /* global symbol table (for debugger) */
    VM_GLOBAL_OBJDEF(class CTcPrsSymtab, sym_table)

    /* byte code interpreter */
    VM_GLOBAL_PREOBJDEF(class CVmRun, interpreter)

    /* size of each exception table entry in the image file */
    VM_GLOBAL_VARDEF(size_t, exc_entry_size)

    /* size of each debugger source line entry in the image file */
    VM_GLOBAL_VARDEF(size_t, line_entry_size)

    /* size of header of each method's debug table */
    VM_GLOBAL_VARDEF(size_t, dbg_hdr_size)

    /* size of each debugger local symbol header */
    VM_GLOBAL_VARDEF(size_t, dbg_lclsym_hdr_size)

    /* debug record format version */
    VM_GLOBAL_VARDEF(int, dbg_fmt_vsn)

    /* debugger API */
    VM_GLOBAL_OBJDEF(class CVmDebug, debugger)

    /* pre-defined objects and properties */
    VM_GLOBAL_PREOBJDEF(struct CVmPredef, predef)

    /* preinit mode flag */
    VM_GLOBAL_VARDEF(int, preinit_mode)

    /* flag: error subsystem initialized outside of VM globals */
    VM_GLOBAL_VARDEF(int, err_pre_inited)

    /* resource loader for program-loaded resources */
    VM_GLOBAL_OBJDEF(class CResLoader, res_loader)

    /* TADS built-in function globals */
    VM_GLOBAL_OBJDEF(class CVmBifTADSGlobals, bif_tads_globals)

    /* host application interface */
    VM_GLOBAL_OBJDEF(class CVmHostIfc, host_ifc)

    /* image file loader */
    VM_GLOBAL_OBJDEF(class CVmImageLoader, image_loader)

    /* name of the UI character set, if specified explicitly */
    VM_GLOBAL_OBJDEF(char, disp_cset_name)

    /* 
     *   Character mappings to and from the local filename character set.
     *   This is the character set that's used by the local file system to
     *   represent filenames.  Note that this isn't the set for the
     *   *contents* of files - just for their names. 
     */
    VM_GLOBAL_OBJDEF(class CCharmapToUni, cmap_from_fname)
    VM_GLOBAL_OBJDEF(class CCharmapToLocal, cmap_to_fname)

    /* 
     *   Character mappings to and from the default local file-contents
     *   character set.
     *   
     *   At best, this is only a default.  In actual practice files can be in
     *   any character set (or even in no character sets at all, or in
     *   multiple character sets, in the case of binary or structured files).
     *   Even so, on most systems, there will be a character set that most
     *   ordinary text files use; this is that set.  
     */
    VM_GLOBAL_OBJDEF(class CCharmapToUni, cmap_from_file)
    VM_GLOBAL_OBJDEF(class CCharmapToLocal, cmap_to_file)

    /* 
     *   Character mappings to and from the local character set for the
     *   console/display user interface.  This character set is used for
     *   formatting output to the display and reading input from the
     *   keyboard.  
     */
    VM_GLOBAL_OBJDEF(class CCharmapToUni, cmap_from_ui)
    VM_GLOBAL_OBJDEF(class CCharmapToLocal, cmap_to_ui)

    /* user interface primary console */
    VM_GLOBAL_OBJDEF(class CVmConsoleMain, console)

    /* BigNumber package temporary register cache */
    VM_GLOBAL_OBJDEF(class CVmBigNumCache, bignum_cache)

    /* TadsObject inheritance path analysis queue */
    VM_GLOBAL_OBJDEF(class CVmObjTadsInhQueue, tadsobj_queue)

VM_GLOBALS_END

