#charset "us-ascii"

/* 
 *   Copyright (c) 1999, 2002 Michael J. Roberts
 *   
 *   This file is part of TADS 3 
 */

/*
 *   T3 intrinsic function set definition
 */

#ifndef T3_H
#define T3_H

/* 
 *   include the LookupTable intrinsic class, since t3GetGlobalSymbols()
 *   returns an instance of this class 
 */
#include "lookup.h"


/* 
 *   define the T3 system interface 
 */
intrinsic 't3vm/010004'
{
    /* run garbage collection */
    t3RunGC();

    /* 
     *   Set the default output function or method.  The return value is the
     *   old function pointer or method, depending on which one is being set
     *   with this call.  (If 'val' is a function pointer, the return value
     *   will be the old function; if 'val' is a property ID, the return
     *   value is the old method.)
     *   
     *   The special values T3SetSayNoFunc and T3SetSayNoMethod can be passed
     *   to the function to remove any existing function or method,
     *   respectively, and are returned when appropriate to indicate that
     *   there was no previous setting.  
     */
    t3SetSay(val);

    /* get the VM version number */
    t3GetVMVsn();

    /* get the VM identifier string */
    t3GetVMID();

    /* get the VM banner string */
    t3GetVMBanner();

    /* get the preinitialization mode flag */
    t3GetVMPreinitMode();

    /* debugger trace operations */
    t3DebugTrace(mode, ...);

    /*
     *   Get the global symbol table.  If a symbol table is available, this
     *   returns a LookupTable object; otherwise, it returns nil.
     *   
     *   This call will return a valid object value when pre-initialization
     *   is running during program building, or when the program has been
     *   compiled for debugging.  When a program compiled for release (i.e.,
     *   no debug information) is run under the interpreter, this will
     *   return nil, because no symbol information is available.
     *   
     *   Note that programs can, if they wish, get a reference to this
     *   object during pre-initialization, then keep the reference (by
     *   storing it in an object property, for example) so that it is
     *   available during normal execution under the interpreter.  If the
     *   program is compiled for release, and it does not keep a reference
     *   in this manner, the garbage collector will automatically delete the
     *   object when pre-initialization is completed.  This allows programs
     *   that wish to keep the symbol information around at run-time to do
     *   so, while not burdening programs that don't need the information
     *   with the extra memory the symbols consume.  
     */
    t3GetGlobalSymbols();

    /*
     *   Allocate a new property.  Returns a new property not yet used
     *   anywhere in the program.  Note that property ID's are a somewhat
     *   limited resource - only approximately 65,000 total are available,
     *   including all of the properties that the program defines
     *   statically.  
     */
    t3AllocProp();

    /*
     *   Get a stack trace.  This returns a list of T3StackInfo objects.
     *   Each object represents a nesting level in the call stack.  The
     *   first element in the list represents the currently active level
     *   (i.e., the level that called this function), the second element
     *   represents the caller of the first element, and so on. 
     */
    t3GetStackTrace();
}

/*
 *   t3DebugTrace() mode flags 
 */

/* check to see if the debugger is present */
#define T3DebugCheck     1

/* break into the debugger */
#define T3DebugBreak     2

/*
 *   t3SetSay() special values.  These can be passed in lieu of a function
 *   pointer or property ID when the caller wants to remove any existing
 *   function or method rather than install a new one.  
 */
#define T3SetSayNoFunc    1
#define T3SetSayNoMethod  2


#endif /* T3_H */
