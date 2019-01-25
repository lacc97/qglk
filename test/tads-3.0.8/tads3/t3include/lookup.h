#charset "us-ascii"

/* 
 *   Copyright (c) 2000, 2002 Michael J. Roberts
 *   
 *   This file is part of TADS 3.  
 */

#ifndef _LOOKUP_H_
#define _LOOKUP_H_

/* include our base class definitions */
#include "systype.h"

/*
 *   'LookupTable' intrinsic class
 */
intrinsic class LookupTable 'lookuptable/030002': Collection
{
    /* 
     *   Determine if a given key is present in the table.  Returns true if
     *   the key is present, nil if not. 
     */
    isKeyPresent(key);

    /*
     *   Remove an entry from the table.  Removes the key/value pair
     *   associated with the given key, and returns the value that was
     *   associated with the key.  If the key isn't present in the table,
     *   the return value is nil, and the method has no other effect. 
     */
    removeElement(key);

    /*
     *   Apply the given function to each entry, and replace the value of
     *   the entry with the return value of the function.  The callback is
     *   invoked with the key and value as arguments for each entry:
     *   func(key, value).  No return value.
     */
    applyAll(func);

    /*
     *   Invoke the given function with each entry in the table.  The
     *   function is invoked with value of an entry as its argument:
     *   func(value).  Any return value of the function is ignored.  No
     *   return value.  
     */
    forEach(func);

    /*
     *   Get the number of buckets (i.e., slots for unique hash values).
     *   The number of buckets doesn't vary over the life of the table, so
     *   this simply returns the number of buckets that was specified in the
     *   constructor when the table was created.  This can be used to create
     *   a new table with the same parameters as an existing table.  
     */
    getBucketCount();

    /*
     *   Get the number of entries.  This returns the number of key/value
     *   pairs stored in the table.  Note that this is not the same as the
     *   initial capacity specified in the constructor when the table was
     *   created; this is the number of entries actually stored in the
     *   table.  
     */
    getEntryCount();

    /*
     *   Invoke the given function with each entry in the table, passing the
     *   key and value to the callback.  The function is invoked with key
     *   and value of an entry as its arguments: func(key, value).  Any
     *   return value of the function is ignored.  No return value.  
     */
    forEachAssoc(func);

    /*
     *   Make a list of all of my keys or values.  The return value is a
     *   list, in arbitrary order, of all of the keys or values in the table.
     */
    keysToList();
    valsToList();
}

/*
 *   Weak-reference version of the lookup table.  This is similar to the
 *   regular LookupTable, and has the same methods; the only difference is
 *   that this type of table weakly references its values.  A value that is
 *   reachable only through weak references is subject to deletion by the
 *   garbage collector.  A weak-reference lookup table is useful when you
 *   don't want a value's presence in the lookup table to force the value to
 *   stay active, such as when the lookup table is merely a fast index to a
 *   set of values that must be otherwise reachable to be useful.  When the
 *   garbage collector deletes one of our values, the key/value pair for the
 *   value is automatically deleted from the table.  
 */
intrinsic class WeakRefLookupTable 'weakreflookuptable/030000': LookupTable
{
}

/*
 *   LookupTable iterator - this type of iterator is used for LookupTable
 *   instances 
 */
intrinsic class LookupTableIterator 'lookuptable-iterator/030000': Iterator
{
}

#endif /* _LOOKUP_H_ */
