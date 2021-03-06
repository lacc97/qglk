#charset "us-ascii"

/*
 *   Copyright (c) 2000, 2002 Michael J. Roberts
 *   
 *   This file is part of TADS 3.  
 */

#ifndef _VECTOR_H_
#define _VECTOR_H_

/* include our base class definition */
#include "systype.h"


/*
 *   'Vector' metaclass 
 */
intrinsic class Vector 'vector/030004': Collection
{
    /* 
     *   Create a list with the same elements as the vector.  If 'start' is
     *   specified, it's the index of the first element we store; we'll
     *   store elements starting at index 'start'.  If 'cnt' is specified,
     *   it gives the maximum number of elements for the new list; by
     *   default, we'll store all of the elements from 'start' to the last
     *   element.  
     */
    toList(start?, cnt?);

    /* get the number of elements in the vector */
    length();

    /* 
     *   Copy from another vector or list.  Elements are copied from the
     *   source vector or list starting at the element given by 'src_start',
     *   and are copied into 'self' starting at the index given by
     *   'dst_start'.  At most 'cnt' values are copied, but we stop when we
     *   reach the last element of either the source or destination values.  
     */
    copyFrom(src, src_start, dst_start, cnt);

    /* 
     *   Fill with a given value, starting at the given element (the first
     *   element if not specified), and running for the given number of
     *   elements (the remaining existing elements of the vector, if not not
     *   specified).  The vector is expanded if necessary.  
     */
    fillValue(val, start?, cnt?);

    /*
     *   Select a subset of the vector.  Returns a new vector consisting
     *   only of the elements of this vector for which the callback function
     *   returns true.  
     */
    subset(func);

    /*
     *   Apply a callback function to each element of the vector.  For each
     *   element of the vector, invokes the callback, and replaces the
     *   element with the return value of the callback.  Modifies the vector
     *   in-place, and returns 'self'.  
     */
    applyAll(func);

    /* 
     *   Find the first element for which the given condition is true.
     *   Apply the callback function (which encodes the condition to
     *   evaluate) to each element in turn, starting with the first.  For
     *   each element, if the callback returns nil, proceed to the next
     *   element; otherwise, stop and return the index of the element.  If
     *   the callback never returns true for any element, we'll return nil.  
     */
    indexWhich(cond);

    /* 
     *   Invoke the callback func(val) on each element, in order from first
     *   to last.  No return value.  
     */
    forEach(func);

    /* 
     *   Invoke the callback func(index, val) on each element, in order from
     *   first to last.  No return value.  
     */
    forEachAssoc(func);

    /*
     *   Apply the callback function to each element of this vector, and
     *   return a new vector consisting of the results.  Effectively maps
     *   the vector to a new vector using the given function, leaving the
     *   original vector unchanged.  
     */
    mapAll(func);

    /* get the index of the first match for the given value */
    indexOf(val);

    /* 
     *   Find the first element for which the given condition is true, and
     *   return the value of the element.  
     */
    valWhich(cond);

    /* find the last element with the given value, and return its index */
    lastIndexOf(val);

    /* 
     *   Find the last element for which the condition is true, and return
     *   the index of the element.  Applies the callback to each element in
     *   turn, starting with the last element and working backwards.  For
     *   each element, if the callback returns nil, proceeds to the previous
     *   element; otherwise, stops and returns the index of the element.  If
     *   the callback never returns true for any element, we'll return nil.  
     */
    lastIndexWhich(cond);

    /* 
     *   Find the last element for which the condition is true, and return
     *   the value of the element 
     */
    lastValWhich(cond);

    /* count the number of elements with the given value */
    countOf(val);

    /* count the number of elements for which the callback returns true */
    countWhich(cond);

    /* create a new vector consisting of the unique elements of this vector */
    getUnique();

    /*
     *   append the elements of the list or vector 'val' to the elements of
     *   this vector, then remove repeated elements in the result; returns a
     *   new vector with the unique elements of the combination 
     */
    appendUnique(val);

    /* 
     *   Sort the vector in place; returns 'self'.  If the 'descending'
     *   flag is provided and is not nil, we'll sort the vector in
     *   descending order rather than ascending order.
     *   
     *   If the 'comparisonFunction' argument is provided, it must be a
     *   callback function; the callback takes two arguments, and returns
     *   an integer less than zero if the first argument value is less
     *   than the second, zero if they're equal, and an integer greater
     *   than zero if the first is greater than the second.  If no
     *   'comparisonFunction' argument is provided, or it's provided and
     *   its value is nil, we'll simply compare the vector elements as
     *   ordinary values.  The comparison function can be provided for
     *   caller-defined orderings, such as ordering a set of objects.  
     */
    sort(descending?, comparisonFunction?);

    /* 
     *   Set the length - if this is shorter than the current length,
     *   existing items will be discarded; if it's longer, the newly added
     *   slots will be set to nil.  Returns 'self'.
     */
    setLength(newElementCount);

    /* 
     *   Insert one or more elements at the given index.  If the index is
     *   1, the elements will be inserted before the first existing
     *   element.  If the index is one higher than the number of elements,
     *   the elements will be inserted after all existing elements.
     *   
     *   Note that a list value argument will simply be inserted as a
     *   single element.
     *   
     *   Returns 'self'.  
     */
    insertAt(startingIndex, val, ...);

    /*
     *   Delete the element at the given index, reducing the length of the
     *   vector by one element.  Returns 'self'.  
     */
    removeElementAt(index);

    /*
     *   Delete the range of elements starting at startingIndex and ending
     *   at endingIndex.  The elements at the ends of the range are
     *   included in the deletion.  If startingIndex == endingIndex, only
     *   one element is removed.  Reduces the length of the vector by the
     *   number of elements removed.  Returns 'self'.  
     */
    removeRange(startingIndex, endingIndex);

    /* 
     *   Append an element to the vector.  This works just like insertAt()
     *   with a starting index one higher than the length of the vector.
     *   This has almost the same effect as the '+' operator, but treats a
     *   list value like any other value by simply inserting the list as a
     *   single new element (rather than appending each item in the list
     *   individually, as the '+' operator would).  
     */
    append(val);

    /*
     *   Prepend an element.  This works like insertAt() with a starting
     *   index of 1. 
     */
    prepend(val);

    /*
     *   Append all elements from a list or vector.  This works like
     *   append(val), except that if 'val' is a list or vector, the elements
     *   of 'val' will be appended individually, like the '+' operator does.
     *   The difference between this method and the '+' operator is that
     *   this method modifies this Vector by adding the new elements
     *   directly to the existing Vector object, whereas the '+' operator
     *   creates a new Vector to store the result.  
     */
    appendAll(val);

    /*
     *   Remove an element by value.  Each element of the vector matching
     *   the given value is removed.  The vector is modified in-place.  The
     *   return value is 'self'.  
     */
    removeElement(val);
}

#endif /* _VECTOR_H_ */

