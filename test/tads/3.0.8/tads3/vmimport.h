/* 
 *   Copyright (c) 2001, 2002 Michael J. Roberts.  All Rights Reserved.
 *   
 *   Please see the accompanying license file, LICENSE.TXT, for information
 *   on using and copying this software.  
 */
/*
Name
  vmimport.h - T3 VM imported symbol list
Function
  Defines the list of symbols to import from the image file.  Each
  imported symbol must have an entry in this file.
Notes
  Note that this file is NOT protected against multiple inclusion,
  because it is included in several different ways to generate different
  tables.

  Each includer must define the generator macros (VM_IMPORT, VM_IMP_xxx,
  etc) just before including this file.  The macros are to be defined as
  needed by the includer to generate the appropriate table entries for that
  inclusion.
Modified
  02/23/01 MJRoberts  - Creation
*/


/* if any of our macros are undefined, provide empty definitions now */
#ifndef VM_IMPORT_OBJ
#define VM_IMPORT_OBJ(sym, member_name)
#endif
#ifndef VM_IMPORT_PROP
#define VM_IMPORT_PROP(sym, member_name)
#endif
#ifndef VM_NOIMPORT_OBJ
#define VM_NOIMPORT_OBJ(sym, member_name)
#endif
#ifndef VM_NOIMPORT_PROP
#define VM_NOIMPORT_PROP(sym, member_name)
#endif
#ifndef VM_IMPORT_FUNC
#define VM_IMPORT_FUNC(symm, member_name)
#endif


/* ------------------------------------------------------------------------ */
/*
 *   This is the imported symbol table.  Add symbols here as needed.  Define
 *   each imported symbol as follows:
 *   
 *   VM_IMPORT_OBJ("object_symbol_name", predef_member_name) - import an
 *   object
 *   
 *   VM_IMPORT_PROP("prop_symbol_name", predef_member_name) - import a
 *   property
 *   
 *   VM_NOIMPORT_OBJ("object_symbol_name", predef_member_name) - define a
 *   named object, but do not generate an import for it; objects defined in
 *   this way are created after load.  Any object that the VM creates on its
 *   own after load must be defined in this manner, so that the object can
 *   be identified in saved state files and restored properly.
 *   
 *   VM_NOIMPORT_PROP("prop_symbol_name", predef_member_name) - define a
 *   named property, but do not generate an import for it.  
 */

/* the base class for VM run-time exceptions */
VM_IMPORT_OBJ("RuntimeError", rterr)

/* 
 *   the property in which we store the exception message text for VM
 *   run-time exceptions 
 */
VM_IMPORT_PROP(VM_IMPORT_NAME_RTERRMSG, rterrmsg_prop)

/* the constructor and destructor (finalizer) properties */
VM_IMPORT_PROP("Constructor", obj_construct)
VM_IMPORT_PROP("Destructor", obj_destruct)

/* the last property ID allocated by the compiler */
VM_IMPORT_PROP("LastProp", last_prop)

/* the property used to invoke the function in an anonymous function object */
VM_IMPORT_PROP("ObjectCallProp", obj_call_prop)

/* property invoked when an undefined property is invoked */
VM_IMPORT_PROP("propNotDefined", prop_not_defined_prop)

/* object representing a frame in a stack trace (t3GetStackTrace) */
VM_IMPORT_OBJ("T3StackInfo", stack_info_cls)

/* entrypoint function for restoring a saved state on startup */
VM_IMPORT_FUNC("mainRestore", main_restore_func)

/* 
 *   the objects used to represent constant strings and lists, respectively,
 *   in method calls to such values 
 */
VM_NOIMPORT_OBJ(VM_IMPORT_NAME_CONSTSTR, const_str_obj)
VM_NOIMPORT_OBJ(VM_IMPORT_NAME_CONSTLST, const_lst_obj)

/*
 *   the array object that we use to keep track of the last allocated
 *   property ID (we keep this in an object so that property ID allocations
 *   can easily participate in the undo mechanism and are easily saved and
 *   restored) 
 */
VM_NOIMPORT_OBJ(VM_IMPORT_NAME_LASTPROPOBJ, last_prop_obj)

/* 
 *   the properties that the GrammarProd intrinsic class uses to indicate in
 *   a match tree the first and last token index of each match 
 */
VM_IMPORT_PROP("GrammarProd.firstTokenIndex", gramprod_first_tok)
VM_IMPORT_PROP("GrammarProd.lastTokenIndex", gramprod_last_tok)
VM_IMPORT_PROP("GrammarProd.tokenList", gramprod_token_list)
VM_IMPORT_PROP("GrammarProd.tokenMatchList", gramprod_token_match_list)

/* 
 *   for the CharacterSet intrinsic class, the exception class for unknown
 *   local character mappings 
 */
VM_IMPORT_OBJ("CharacterSet.UnknownCharSetException", charset_unknown_exc)

/*
 *   exceptions for the File intrinsic class 
 */
VM_IMPORT_OBJ("File.FileNotFoundException", file_not_found_exc)
VM_IMPORT_OBJ("File.FileCreationException", file_creation_exc)
VM_IMPORT_OBJ("File.FileOpenException", file_open_exc)
VM_IMPORT_OBJ("File.FileIOException", file_io_exc)
VM_IMPORT_OBJ("File.FileSyncException", file_sync_exc)
VM_IMPORT_OBJ("File.FileClosedException", file_closed_exc)
VM_IMPORT_OBJ("File.FileModeException", file_mode_exc)
VM_IMPORT_OBJ("File.FileSafetyException", file_safety_exc)

/* properties for comparators */
VM_IMPORT_PROP("IfcComparator.calcHash", calc_hash_prop)
VM_IMPORT_PROP("IfcComparator.matchValues", match_values_prop)


/*
 *   now that we've built the table, undefine the macros used to build it,
 *   so that future includers can redefine the macros differently 
 */
#undef VM_IMPORT_OBJ
#undef VM_IMPORT_PROP
#undef VM_NOIMPORT_OBJ
#undef VM_NOIMPORT_PROP
#undef VM_IMPORT_FUNC
