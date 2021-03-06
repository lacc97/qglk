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
  vmerrmsg.cpp - T3 VM error message strings
Function
  Defines the message text for VM errors.  All error text is isolated into
  this module for easy replacement in translated versions.
Notes
  
Modified
  05/13/00 MJRoberts  - Creation
*/

#include <stdarg.h>
#include <stdlib.h>

#include "t3std.h"
#include "vmerr.h"

/* ------------------------------------------------------------------------ */
/*
 *   Enable or disable verbose messages.  To conserve memory, verbose
 *   messages can be omitted.  To omit verbose messages, the platform
 *   makefile should define the preprocessor symbol VMERR_OMIT_VERBOSE. 
 */
#ifdef VMERR_OMIT_VERBOSE
# define VMsg(msg) ""
#else
# define VMsg(msg) msg
#endif

/*
 *   To conserve even more memory, the messages can be omitted entirely.  To
 *   disable all compiled-in messages, define VMERR_OMIT_MESSAGES. 
 */


/* ------------------------------------------------------------------------ */
/*
 *   T3 VM Error Messages 
 *   
 *   The messages must be sorted by message number, so that we can perform
 *   a binary search to look up a message by number.  
 */
const err_msg_t vm_messages_english[] =
{
#ifdef VMERR_OMIT_MESSAGES

    { 0, 0, 0 }

#else /* VMERR_OMIT_MESSAGES */
    
    { VMERR_READ_FILE,
    "error reading file",
    VMsg("Error reading file.  The file might be corrupted or a media error "
         "might have occurred.") },

    { VMERR_WRITE_FILE,
    "error writing file",
    VMsg("Error writing file.  The media might be full, or another media "
         "error might have occurred.") },

    { VMERR_FILE_NOT_FOUND,
    "file not found",
    VMsg("Error opening file.  The specified file might not exist, you "
         "might not have sufficient privileges to open the file, or "
         "a sharing violation might have occurred.") },

    { VMERR_CREATE_FILE,
    "error creating file",
    VMsg("Error creating file.  You might not have sufficient privileges "
         "to open the file, or a sharing violation might have occurred.") },

    { VMERR_OBJ_IN_USE,
    "object ID in use - the image/save file might be corrupted",
    VMsg("An object ID requested by the image/save file is already in use "
         "and cannot be allocated to the file.  This might indicate that "
         "the file is corrupted.") },

    { VMERR_OUT_OF_MEMORY,
    "out of memory",
    VMsg("Out of memory.  Try making more memory available by closing "
         "other applications if possible.") },

    { VMERR_NO_MEM_FOR_PAGE,
    "out of memory allocating pool page",
    VMsg("Out of memory allocating pool page.  Try making more memory "
         "available by closing other applications.") },

    { VMERR_BAD_POOL_PAGE_SIZE,
    "invalid page size - file is not valid",
    VMsg("Invalid page size.  The file being loaded is not valid.") },

    { VMERR_OUT_OF_PROPIDS,
    "no more property ID's are available",
    VMsg("Out of property ID's.  No more properties can be allocated.") },

    { VMERR_CIRCULAR_INIT,
    "circular initialization dependency in intrinsic class (internal error)",
    VMsg("Circular initialization dependency detected in intrinsic class.  "
         "This indicates an internal error in the interpreter.  Please "
         "report this error to the interpreter's maintainer.") },

    { VMERR_UNKNOWN_METACLASS,
    "intrinsic class \"%s\" is not available",
    VMsg("This image file requires an intrinsic class with the identifier "
         "\"%s\", but the class is not available in this interpreter.  This "
         "program cannot be executed with this interpreter.") },

    { VMERR_UNKNOWN_FUNC_SET,
    "intrinsic function set \"%s\" is not available",
    VMsg("This image file requires a function set with the identifier "
         "\"%s\", but the function set is not available in this "
         "intepreter.  This program cannot be executed with this "
         "interpreter.") },

    { VMERR_READ_PAST_IMG_END,
    "reading past end of image file",
    VMsg("Reading past end of image file.  The image file might "
         "be corrupted.") },

    { VMERR_NOT_AN_IMAGE_FILE,
    "not an image file (invalid signature)",
    VMsg("This file is not a valid image file - the file has an invalid "
         "signature.  The image file might be corrupted.") },

    { VMERR_UNKNOWN_IMAGE_BLOCK,
    "unknown block type - image file is incompatible with this interpreter "
        "version",
    VMsg("Unknown block type.  This image file is incompatible with this "
         "version of the interpreter.") },

    { VMERR_IMAGE_BLOCK_TOO_SMALL,
    "data block too small",
    VMsg("A data block in the image file is too small.  The image file might "
         "be corrupted.") },

    { VMERR_IMAGE_POOL_BEFORE_DEF,
    "invalid image file: pool page before pool definition",
    VMsg("This image file is invalid because it specifies a pool page "
         "before the pool's definition.  The image file might be "
         "corrupted.") },

    { VMERR_IMAGE_POOL_BAD_PAGE,
    "invalid image file: pool page our of range of definition",
    VMsg("This image file is invalid because it specifies a pool page "
         "outside of the range of the pool's definition.  The image file "
         "might be corrupted.") },

    { VMERR_IMAGE_BAD_POOL_ID,
    "invalid image file: invalid pool ID",
    VMsg("This image file is invalid because it specifies an invalid "
         "pool ID.  The image file might be corrupted.") },

    { VMERR_LOAD_BAD_PAGE_IDX,
    "invalid image file: bad page index",
    VMsg("This image file is invalid because it specifies an invalid "
         "page index.  The image file might be corrupted.") },

    { VMERR_LOAD_UNDEF_PAGE,
    "loading undefined pool page",
    VMsg("The program is attempting to load a pool page that is not present "
         "in the image file.  The image file might be corrupted.") },

    { VMERR_IMAGE_POOL_REDEF,
    "invalid image file: pool is defined more than once",
    VMsg("This image file is invalid because it defines a pool more than "
         "once.  The image file might be corrupted.") },

    { VMERR_IMAGE_METADEP_REDEF,
    "invalid image file: multiple intrinsic class dependency tables found",
    VMsg("This image file is invalid because it contains multiple "
         "intrinsic class tables.  The image file might be corrupted.") },

    { VMERR_IMAGE_NO_METADEP,
    "invalid image file: no intrinsic class dependency table found",
    VMsg("This image file is invalid because it contains no intrinsic class "
         "tables.  The image file might be corrupted.") },

    { VMERR_IMAGE_FUNCDEP_REDEF,
    "invalid image file: multiple function set dependency tables found",
    VMsg("This image file is invalid because it contains multiple "
         "function set tables.  The image file might be corrupted.") },

    { VMERR_IMAGE_NO_FUNCDEP,
    "invalid image file: no function set dependency table found",
    VMsg("This image file is invalid because it contains no function set "
         "tables.  The image file might be corrupted.") },

    { VMERR_IMAGE_ENTRYPT_REDEF,
    "invalid image file: multiple entrypoints found",
    VMsg("This image file is invalid because it contains multiple entrypoint "
         "definitions.  The image file might be corrupted.") },

    { VMERR_IMAGE_NO_ENTRYPT,
    "invalid image file: no entrypoint found",
    VMsg("This image file is invalid because it contains no entrypoint "
         "specification.  The image file might be corrupted.") },

    { VMERR_IMAGE_INCOMPAT_VSN,
    "incompatible image file format version",
    VMsg("This image file has an incompatible format version.  You must "
         "obtain a newer version of the interpreter to execute this "
         "program.") },

    { VMERR_IMAGE_NO_CODE,
    "image contains no code",
    VMsg("This image file contains no executable code.  The file might "
         "be corrupted.") },

    { VMERR_IMAGE_INCOMPAT_HDR_FMT,
    "incomptabile image file format: method header too old",
    VMsg("This image file has an incompatible method header format.  "
         "This is an older image file version which this interpreter "
         "does not support.") },

    { VMERR_UNAVAIL_INTRINSIC,
    "unavailable intrinsic function called (index %d in function set \"%s\"",
    VMsg("Unavailable intrinsic function called (the function is at "
         "index %d in function set \"%s\").  This function is not available "
         "in this version of the interpreter and cannot be called when "
         "running the program with this version.  This normally indicates "
         "that the \"preinit\" function, or code invoked by preinit, "
         "called an intrinsic that is invalid during compile-time "
         "pre-initialization.") },

    { VMERR_UNKNOWN_METACLASS_INTERNAL,
    "unknown internal intrinsic class ID %x",
    VMsg("Unknown internal intrinsic class ID %x.  This indicates an "
         "internal error in the interpreter.  Please report this error "
         "to the interpreter's maintainer.") },

    { VMERR_XOR_MASK_BAD_IN_MEM,
    "page mask is not allowed for in-memory image file",
    VMsg("This image file cannot be loaded from memory because it contains "
         "masked data.  Masked data is not valid with in-memory files.  "
         "This probably indicates that the program file was not installed "
         "properly; you must convert this program file for in-memory use "
         "before you can load the program with this version of the "
         "interpreter.") },

    { VMERR_NO_IMAGE_IN_EXE,
    "no embedded image file found in executable",
    VMsg("This executable does not contain an embedded image file.  The "
         "application might not be configured properly or might need to "
         "be rebuilt.  Re-install the application or obtain an updated "
         "version from the application's author.") },

    { VMERR_OBJ_SIZE_OVERFLOW,
    "object size exceeds hardware limits of this computer",
    VMsg("An object defined in this program file exceeds the hardware limits "
         "of this computer.  This program cannot be executed on this type "
         "of computer or operating system.  Contact the program's author "
         "for assistance.") },

    { VMERR_METACLASS_TOO_OLD,
    "intrinsic class \"%s\" version is not available - latest available "
             "version is \"%s\"",
    VMsg("This program needs the intrinsic class \"%s\".  This VM "
         "implementation does not provide a sufficiently recent version "
         "of this intrinsic class; the latest version available in this "
         "VM is \"%s\".  This program cannot run with this version of the "
         "VM; you must use a more recent version of the VM to execute this "
         "program.") },

    { VMERR_INVAL_METACLASS_DATA,
    "invalid intrinsic class data - image file may be corrupted",
    VMsg("Invalid data were detected in an intrinsic class.  This might "
         "indicate that the image file has been corrupted.  You might need "
         "to re-install the program.") },

    { VMERR_BAD_STATIC_NEW,
    "invalid object - class does not allow loading",
    VMsg("An object in the image file cannot be loaded because its class "
         "does not allow creation of objects of the class.  This usually "
         "means that the class is abstract and cannot be instantiated "
         "as a concrete object.") },

    { VMERR_FUNCSET_TOO_OLD,
    "function set \"%s\" version is not available - latest available "
        "version is \"%s\"",
     VMsg("This program needs the function set \"%s\".  This VM "
          "implementation does not provide a sufficiently recent version "
          "of this function set; the latest version available in this VM "
          "is \"%s\".  This program cannot run with this version of the "
          "VM; you must use a more recent version of the VM to execute "
          "this program.") },

    { VMERR_INVAL_EXPORT_TYPE,
    "exported symbol \"%s\" is of incorrect datatype",
    VMsg("The exported symbol \"%s\" is of the incorrect datatype.  Check "
         "the program and the library version.") },

    { VMERR_INVAL_IMAGE_MACRO,
    "invalid data in macro definitions in image file (error code %d)",
    VMsg("The image file contains invalid data in the macro symbols "
         "in the debugging records: macro loader error code %d.  This "
         "might indicate that the image file is corrupted.") },

    { VMERR_NO_MAINRESTORE,
    "this program is not capable of restoring a saved state on startup",
    VMsg("This program is not capable of restoring a saved state on "
         "startup.  To restore the saved state, you must run the program "
         "normally, then use the appropriate command or operation within "
         "the running program to restore the saved position file.") },

    { VMERR_INVALID_SETPROP,
    "property cannot be set for object",
    VMsg("Invalid property change - this property cannot be set for this "
         "object.  This normally indicates that the object is of a type "
         "that does not allow setting of properties at all, or at least "
         "of certain properties.  For example, a string object does not "
         "allow setting properties at all.") },

    { VMERR_NOT_SAVED_STATE,
    "file is not a valid saved state file",
    VMsg("This file is not a valid saved state file.  Either the file was "
         "not created as a saved state file, or its contents have been "
         "corrupted.") },

    { VMERR_WRONG_SAVED_STATE,
    "saved state is for a different program or a different version of "
        "this program",
        VMsg("This file does not contain saved state information for "
             "this program.  The file was saved by another program, or "
             "by a different version of this program; in either case, it "
             "cannot be restored with this version of this program.") },

    { VMERR_SAVED_META_TOO_LONG,
    "intrinsic class name in saved state file is too long",
    VMsg("An intrinsic class name in the saved state file is too long.  "
         "The file might be corrupted, or might have been saved by an "
         "incompatible version of the interpreter.") },

    { VMERR_RECURSIVE_VM_CALL,
    "operation is not valid in a recursive VM invocation",
    VMsg("The operation is not valid in the current "
         "context, because the VM has been invoked recursively.  This "
         "indicates an error in the program.") },

    { VMERR_SAVED_STACK_OVERFLOW,
    "stack overflow restoring saved state file",
    VMsg("The stack stored in the saved state file is too large for this "
         "interpreter.  The state file might have been saved by an "
         "incompatible version of the interpreter.") },

    { VMERR_SAVED_OBJ_ID_INVALID,
    "invalid object ID in saved state file",
    VMsg("The saved state file contains an invalid object ID.  The saved "
         "state file might be corrupted.") },

    { VMERR_BAD_SAVED_STATE,
    "saved state file is corrupted (incorrect checksum)",
    VMsg("The saved state file's checksum is invalid.  This usually "
         "indicates that the file has been corrupted (which could be "
         "due to a media error, modification by another application, "
         "or a file transfer that lost or changed data).") },

    { VMERR_BAD_SAVED_META_DATA,
    "invalid intrinsic class data in saved state file",
    VMsg("The saved state file contains intrinsic class data that "
         "is not valid.  This usually means that the file was saved "
         "with an incompatible version of the interpreter program.") },

    { VMERR_NO_STR_CONV,
    "cannot convert value to string",
    VMsg("This value cannot be converted to a string.") },

    { VMERR_CONV_BUF_OVF,
    "string conversion buffer overflow",
    VMsg("An internal buffer overflow occurred converting this value to "
         "a string.") },

    { VMERR_BAD_TYPE_ADD,
    "invalid datatypes for addition operator",
    VMsg("Invalid datatypes for addition operator.  The values being added "
         "cannot be combined in this manner.") },

    { VMERR_NUM_VAL_REQD,
    "numeric value required",
    VMsg("Invalid value type - a numeric value is required.") },

    { VMERR_INT_VAL_REQD,
    "integer value required",
    VMsg("Invalid value type - an integer value is required.") },

    { VMERR_NO_LOG_CONV,
    "cannot convert value to logical (true/nil)",
    VMsg("This value cannot be converted to a logical (true/nil) value.") },

    { VMERR_BAD_TYPE_SUB,
    "invalid datatypes for subtraction operator",
    VMsg("Invalid datatypes for subtraction operator.  The values used "
         "cannot be combined in this manner.") },

    { VMERR_DIVIDE_BY_ZERO,
    "division by zero",
    VMsg("Arithmetic error - Division by zero.") },

    { VMERR_INVALID_COMPARISON,
    "invalid comparison",
    VMsg("Invalid comparison - these values cannot be compared "
         "to one another.") },

    { VMERR_OBJ_VAL_REQD,
    "object value required",
    VMsg("An object value is required.") },

    { VMERR_PROPPTR_VAL_REQD,
    "property pointer required",
    VMsg("A property pointer value is required.") },

    { VMERR_LOG_VAL_REQD,
    "logical value required",
    VMsg("A logical (true/nil) value is required.") },

    { VMERR_FUNCPTR_VAL_REQD,
    "function pointer required",
    VMsg("A function pointer value is required.") },

    { VMERR_CANNOT_INDEX_TYPE,
    "invalid index operation - this type of value cannot be indexed",
    VMsg("This type of value cannot be indexed.") },

    { VMERR_INDEX_OUT_OF_RANGE,
    "index out of range",
    VMsg("The index value is out of range for the value being indexed (it is "
         "too low or too high).") },

    { VMERR_BAD_METACLASS_INDEX,
    "invalid intrinsic class index",
    VMsg("The intrinsic class index is out of range.  This probably "
         "indicates that the image file is corrupted.") },

    { VMERR_BAD_DYNAMIC_NEW,
    "invalid dynamic object creation (intrinsic class does not support NEW)",
    VMsg("This type of object cannot be dynamically created, because the "
           "intrinsic class does not support dynamic creation.") },

    { VMERR_OBJ_VAL_REQD_SC,
    "object value required for base class",
    VMsg("An object value must be specified for the base class of a dynamic "
         "object creation operation.  The superclass value is of a "
         "non-object type.") },

    { VMERR_STRING_VAL_REQD,
    "string value required",
    VMsg("A string value is required.") },

    { VMERR_LIST_VAL_REQD,
    "list value required",
    VMsg("A list value is required.") },

    { VMERR_DICT_NO_CONST,
    "list or string reference found in dictionary (entry \"%s\") - this "
        "dictionary cannot be saved in the image file",
     VMsg("A dictionary entry (for the string \"%s\") referred to a string "
          "or list value for its associated value data.  This dictionary "
          "cannot be stored in the image file, so the image file cannot "
          "be created.  Check dictionary word additions and ensure that "
          "only objects are added to the dictionary.") },

    { VMERR_INVAL_OBJ_TYPE,
    "invalid object type - cannot convert to required object type",
    VMsg("An object is not of the correct type.  The object specified "
         "cannot be converted to the required object type.") },

    { VMERR_NUM_OVERFLOW,
    "numeric overflow",
    VMsg("A numeric calculation overflowed the limits of the datatype.") },

    { VMERR_BAD_TYPE_MUL, 
    "invalid datatypes for multiplication operator",
    VMsg("Invalid datatypes for multiplication operator.  The values "
         "being added cannot be combined in this manner.") },

    { VMERR_BAD_TYPE_DIV, 
    "invalid datatypes for division operator",
    VMsg("Invalid datatypes for division operator.  The values being added "
         "cannot be combined in this manner.") },

    { VMERR_BAD_TYPE_NEG,
    "invalid datatype for arithmetic negation operator",
    VMsg("Invalid datatype for arithmetic negation operator.  The value "
         "cannot be negated.") },

    { VMERR_OUT_OF_RANGE,
    "value is out of range",
    VMsg("A value that was outside of the legal range of inputs was "
         "specified for a calculation.") },

    { VMERR_STR_TOO_LONG,
    "string is too long",
    VMsg("A string value is limited to 65535 bytes in length.  This "
         "string exceeds the length limit.") },

    { VMERR_LIST_TOO_LONG,
    "list too long",
    VMsg("A list value is limited to about 13100 elements.  This list "
         "exceeds the limit.") },

    { VMERR_TREE_TOO_DEEP_EQ,
    "maximum equality test/hash recursion depth exceeded",
    VMsg("This equality comparison or hash calculation is too complex and "
         "cannot be performed.  This usually indicates that a value "
         "contains circular references, such as a Vector that contains "
         "a reference to itself, or to another Vector that contains a "
         "reference to the first one.  This type of value cannot be "
         "compared for equality or used in a LookupTable.") },

    { VMERR_WRONG_NUM_OF_ARGS,
    "wrong number of arguments",
    VMsg("The wrong number of arguments was passed to a function or method "
         "in the invocation of the function or method.") },

    { VMERR_WRONG_NUM_OF_ARGS_CALLING,
    "argument mismatch calling %s - function definition is incorrect",
    VMsg("The number of arguments doesn't match the number expected "
         "calling %s.  Check the function or method and correct the "
         "number of parameters that it is declared to receive.") },

    { VMERR_NIL_DEREF,
    "nil object reference",
     VMsg("The value 'nil' was used to reference an object property.  Only "
          "valid object references can be used in property evaluations.") },

    { VMERR_CANNOT_CREATE_INST,
    "cannot create instance of object - object is not a class",
    VMsg("An instance of this object cannot be created, because this "
         "object is not a class.") },

    { VMERR_ILLEGAL_NEW,
    "cannot create instance - class does not allow dynamic construction",
    VMsg("An instance of this class cannot be created, because this class "
         "does not allow dynamic construction.") },

    { VMERR_INVALID_OPCODE,
    "invalid opcode - possible image file corruption",
    VMsg("Invalid instruction opcode - the image file might be corrupted.") },

    { VMERR_UNHANDLED_EXC,
    "unhandled exception",
    VMsg("An exception was thrown but was not caught by the program.  "
         "The interpreter is terminating execution of the program.") },

    { VMERR_STACK_OVERFLOW,
    "stack overflow",
    VMsg("Stack overflow.  This indicates that function or method calls "
         "were nested too deeply; this might have occurred because of "
         "unterminated recursion, which can happen when a function or method "
         "calls itself (either directly or indirectly).") },

    { VMERR_BAD_TYPE_BIF,
    "invalid type for intrinsic function argument",
    VMsg("An invalid datatype was provided for an intrinsic "
         "function argument.") },

    { VMERR_SAY_IS_NOT_DEFINED,
    "default output function is not defined",
    VMsg("The default output function is not defined.  Implicit string "
         "display is not allowed until a default output function "
         "is specified.") },

    { VMERR_BAD_VAL_BIF,
    "invalid value for intrinsic function argument",
    VMsg("An invalid value was specified for an intrinsic function argument.  "
         "The value is out of range or is not an allowed value.") },

    { VMERR_BREAKPOINT,
    "breakpoint encountered",
    VMsg("A breakpoint instruction was encountered, and no debugger is "
         "active.  The compiler might have inserted this breakpoint to "
         "indicate an invalid or unreachable location in the code, so "
         "executing this instruction probably indicates an error in the "
         "program.") },

    { VMERR_CALLEXT_NOT_IMPL,
    "external function calls are not implemented in this version",
    VMsg("This version of the interpreter does not implement external "
         "function calls.  This program requires an interpreter that "
         "provides external function call capabilities, so this program "
         "is not compatible with this interpreter.") },

    { VMERR_INVALID_OPCODE_MOD,
    "invalid opcode modifier - possible image file corruption",
    VMsg("Invalid instruction opcode modifier - the image file might "
         "be corrupted.") },

    /*
     *   Note - do NOT use the VMsg() macro on this message, since we always
     *   want to have this verbose message available. 
     */
    { VMERR_NO_CHARMAP_FILE,
    "No mapping file available for local character set \"%.32s\"",
    "[Warning: no mapping file is available for the local character set "
        "\"%.32s\".  The system will use a default ASCII character set "
        "mapping instead, so accented characters will be displayed without "
        "their accents.]" },

    { VMERR_UNHANDLED_EXC_PARAM,
    "Unhandled exception: %s",
    VMsg("Unhandled exception: %s") },

    { VMERR_VM_EXC_PARAM,
    "VM Error: %s",
    VMsg("VM Error: %s") },

    { VMERR_VM_EXC_CODE,
    "VM Error: code %d",
    VMsg("VM Error: code %d") },

    { VMERR_EXC_IN_STATIC_INIT,
    "Exception in static initializer for %s.%s: %s",
    VMsg("An exception occurred in the static initializer for "
         "%s.%s: %s") },

    { VMERR_INTCLS_GENERAL_ERROR,
    "intrinsic class exception: %s",
    VMsg("Exception in intrinsic class method: %s") },

    { VMERR_DBG_ABORT,
    "'abort' signal",
    VMsg("'abort' signal") },

    { VMERR_DBG_RESTART,
    "'restart' signal",
    VMsg("'restart' signal") },

    { VMERR_BAD_FILE_HANDLE,
    "invalid file handle",
    VMsg("An invalid value was specified for the file handle in a call to "
         "a file manipulation function.") },

    { VMERR_BAD_FRAME,
    "invalid frame in debugger local/parameter evaluation",
    VMsg("An invalid stack frame was specified in a debugger local/parameter "
         "evaluation.  This probably indicates an internal problem in the "
         "debugger.") },

    { VMERR_BAD_SPEC_EVAL,
    "invalid speculative expression",
    VMsg("This expression cannot be executed speculatively.  (This does not "
         "indicate a problem; it's merely an internal condition in the "
         "debugger.)") },

    { VMERR_INVAL_DBG_EXPR,
    "invalid debugger expression",
    VMsg("This expression cannot be evaluated in the debugger.") },

    { VMERR_NO_IMAGE_DBG_INFO,
    "image file has no debugging information - recompile for debugging",
    VMsg("The image file has no debugging information.  You must recompile "
         "the source code for this program with debugging information in "
         "order to run the program under the debugger.") },

    { VMERR_BIGNUM_NO_REGS,
    "out of temporary floating point registers (calculation too complex)",
    VMsg("The interpreter is out of temporary floating point registers.  "
         "This probably indicates that an excessively complex calculation "
         "has been attempted.") }

#endif /* VMERR_OMIT_MESSAGES */
};

/* size of the english message array */
size_t vm_message_count_english =
    sizeof(vm_messages_english)/sizeof(vm_messages_english[0]);

/* 
 *   the actual message array - we'll initialize this to the
 *   english list that's linked in, but if we find an external message
 *   file, we'll use the external file instead 
 */
const err_msg_t *vm_messages = vm_messages_english;

/* message count - initialize to the english message array count */
size_t vm_message_count =
    sizeof(vm_messages_english)/sizeof(vm_messages_english[0]);

/* ------------------------------------------------------------------------ */
/*
 *   we don't need the VMsg() (verbose message) cover macro any more 
 */
#undef VMsg
