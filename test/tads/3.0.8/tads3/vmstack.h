/* $Header: d:/cvsroot/tads/tads3/VMSTACK.H,v 1.3 1999/07/11 00:46:58 MJRoberts Exp $ */

/* 
 *   Copyright (c) 1998, 2002 Michael J. Roberts.  All Rights Reserved.
 *   
 *   Please see the accompanying license file, LICENSE.TXT, for information
 *   on using and copying this software.  
 */
/*
Name
  vmstack.h - VM stack manager
Function
  
Notes
  
Modified
  10/28/98 MJRoberts  - Creation
*/

#ifndef VMSTACK_H
#define VMSTACK_H

#include "vmtype.h"

/* ------------------------------------------------------------------------ */
/*
 *   VM stack interface
 */

class CVmStack
{
public:
    /* 
     *   Allocate the stack.  The maximum depth that the stack can achieve
     *   is fixed when the stack is created. 
     */
    CVmStack(size_t max_depth, size_t reserve_depth);

    /* initialize */
    void init()
    {
        /* start the stack pointer at the first element */
        sp_ = arr_;
    }

    /* delete the stack */
    ~CVmStack();

    /* 
     *   get the current stack depth - this gives the number of active
     *   elements in the stack
     */
    size_t get_depth() const { return sp_ - arr_; }

    /*
     *   Translate between pointer and index values.  An index value is
     *   simply an integer giving the index in the stack of the given
     *   pointer; this value can be used, for example, for saving a stack
     *   location persistently.
     *   
     *   We return zero for a null stack pointer; we always return
     *   non-zero for a non-null stack pointer.  
     */
    ulong ptr_to_index(vm_val_t *p) const
    {
        /* if it's null, return index 0; otherwise, return a non-zero index */
        return (p == 0 ? 0 : p - arr_ + 1);
    }
    vm_val_t *index_to_ptr(ulong idx) const
    {
        /* if the index is zero, it's null; otherwise, get the pointer */
        return (idx == 0 ? 0 : arr_ + idx - 1);
    }

    /*
     *   Get the depth relative to a given frame pointer.  This returns
     *   the number of items on the stack beyond the given pointer.  If
     *   the given frame pointer is beyond the current stack pointer
     *   (i.e., values have been popped since the frame pointer equalled
     *   the stack pointer), the return value will be negative. 
     */
    int get_depth_rel(vm_val_t *fp) const { return sp_ - fp; }

    /* get the current stack pointer */
    vm_val_t *get_sp() const { return sp_; }

    /* 
     *   Set the current stack pointer.  The pointer must always be a
     *   value previously returned by get_sp().  
     */
    void set_sp(vm_val_t *p) { sp_ = p; }

    /*
     *   Get an element relative to a frame pointer (a frame pointer is a
     *   stack position that was previously obtained via get_sp() and
     *   stored by the caller).  The offset is negative for a value pushed
     *   prior to the frame pointer, zero for the value at the frame
     *   pointer, or positive for values pushed after the frame pointer.  
     */
    static vm_val_t *get_from_frame(vm_val_t *fp, int i)
        { return (fp + i - 1); }

    /* push a value */
    void push(const vm_val_t *val) { *sp_++ = *val; }

    /* 
     *   Push an element, returning a pointer to the element; this can be
     *   used to fill in a new stack element directly, without copying a
     *   value.  The new element is not filled in yet on return, so the
     *   caller should immediately fill in the element with a valid value.  
     */
    vm_val_t *push() { return sp_++; }

    /* 
     *   Push a number of elements: this allocates a block of contiguous
     *   stack elements that the caller can fill in individually.  The stack
     *   elements are uninitialized, so the caller must set the values
     *   immediately on return.  A pointer to the first pushed element is
     *   returned; subsequent elements are addressed at the return value
     *   plus 1, plus 2, and so on.  
     */
    vm_val_t *push(unsigned int n)
    {
        /* remember the current stack pointer, which is what we return */
        vm_val_t *ret = sp_;

        /* allocate the elements */
        sp_ += n;

        /* return the base of the allocated block */
        return ret;
    }

    /* 
     *   Get an element.  Elements are numbered from zero to (depth - 1).
     *   Element number zero is the item most recently pushed onto the
     *   stack; element (depth-1) is the oldest element on the stack.  
     */
    vm_val_t *get(size_t i) const { return (sp_ - i - 1); }

    /* pop the top element off the stack */
    void pop(vm_val_t *val) { *val = *--sp_; }

    /* discard the top element */
    void discard() { --sp_; }

    /* discard a given number of elements from the top of the stack */
    void discard(int n) { sp_ -= n; }

    /*
     *   Probe the stack for a given allocation.  Returns true if the
     *   given number of slots are available, false if not.  Does NOT
     *   actually allocate the space; merely checks for availability.
     *   
     *   Compilers are expected to produce function headers that check for
     *   the maximum amount of stack space needed locally in the function
     *   on entry, which allows us to check once at the start of the
     *   function for available stack space, relieving us of the need to
     *   check for available space in every push operation.
     *   
     *   Returns true if the required amount of space is available, false
     *   if not.  
     */
    int check_space(size_t slots) const
        { return (get_depth() + slots <= max_depth_); }

    /*
     *   Release the reserve.  Debuggers can use this to allow manual
     *   recovery from stack overflows, by making some extra stack
     *   temporarily available for the debugger's use in handling the
     *   overflow.  This releases the reserve, if available, that was
     *   specified when the stack was allocated.  Returns true if reserve
     *   space is available for release, false if not.  
     */
    int release_reserve()
    {
        /* if the reserve is already in use, we can't release it again */
        if (reserve_in_use_)
            return FALSE;

        /* add the reserve space to the maximum stack depth */
        max_depth_ += reserve_depth_;

        /* note that the reserve has been released */
        reserve_in_use_ = TRUE;

        /* indicate that we successfully released the reserve */
        return TRUE;
    }

    /*
     *   Recover the reserve.  If the debugger releases the reserve to handle
     *   a stack overflow, it can call this once the situation has been dealt
     *   with to take the reserve back out of play, so that the debugger can
     *   deal with any future overflow in the same manner.  
     */
    void recover_reserve()
    {
        /* if the reserve is in use, put it back in reserve */
        if (reserve_in_use_)
        {
            /* remove the reserve from the stack */
            max_depth_ -= reserve_depth_;

            /* mark the reserve as available again */
            reserve_in_use_ = FALSE;
        }
    }
        

private:
    /* the array of value holders making up the stack */
    vm_val_t *arr_;

    /* 
     *   next available stack slot - the stack pointer starts out pointing
     *   at arr_[0], and is incremented after storing each element 
     */
    vm_val_t *sp_;

    /* maximum depth that the stack is capable of holding */
    size_t max_depth_;

    /* extra reserve space */
    size_t reserve_depth_;

    /* flag: the reserve has been released for VM use */
    int reserve_in_use_;
};

#endif /* VMSTACK_H */

