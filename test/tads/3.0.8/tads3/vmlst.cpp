#ifdef RCSID
static char RCSid[] =
"$Header: d:/cvsroot/tads/tads3/VMLST.CPP,v 1.3 1999/05/17 02:52:28 MJRoberts Exp $";
#endif

/* 
 *   Copyright (c) 1998, 2002 Michael J. Roberts.  All Rights Reserved.
 *   
 *   Please see the accompanying license file, LICENSE.TXT, for information
 *   on using and copying this software.  
 */
/*
Name
  vmlst.cpp - list metaclass
Function
  
Notes
  
Modified
  10/29/98 MJRoberts  - Creation
*/

#include <stdlib.h>
#include <string.h>

#include "t3std.h"
#include "vmmcreg.h"
#include "vmtype.h"
#include "vmlst.h"
#include "vmobj.h"
#include "vmerr.h"
#include "vmerrnum.h"
#include "vmfile.h"
#include "vmpool.h"
#include "vmstack.h"
#include "vmmeta.h"
#include "vmrun.h"
#include "vmbif.h"
#include "vmpredef.h"
#include "vmiter.h"
#include "vmsort.h"


/* ------------------------------------------------------------------------ */
/*
 *   statics 
 */

/* metaclass registration object */
static CVmMetaclassList metaclass_reg_obj;
CVmMetaclass *CVmObjList::metaclass_reg_ = &metaclass_reg_obj;

/* function table */
int (*CVmObjList::func_table_[])(VMG_ vm_val_t *retval,
                                 const vm_val_t *self_val,
                                 const char *lst, uint *argc) =
{
    &CVmObjList::getp_undef,
    &CVmObjList::getp_subset,
    &CVmObjList::getp_map,
    &CVmObjList::getp_len,
    &CVmObjList::getp_sublist,
    &CVmObjList::getp_intersect,
    &CVmObjList::getp_index_of,
    &CVmObjList::getp_car,
    &CVmObjList::getp_cdr,
    &CVmObjList::getp_index_which,
    &CVmObjList::getp_for_each,
    &CVmObjList::getp_val_which,
    &CVmObjList::getp_last_index_of,
    &CVmObjList::getp_last_index_which,
    &CVmObjList::getp_last_val_which,
    &CVmObjList::getp_count_of,
    &CVmObjList::getp_count_which,
    &CVmObjList::getp_get_unique,
    &CVmObjList::getp_append_unique,
    &CVmObjList::getp_append,
    &CVmObjList::getp_sort,
    &CVmObjList::getp_prepend,
    &CVmObjList::getp_insert_at,
    &CVmObjList::getp_remove_element_at,
    &CVmObjList::getp_remove_range,
    &CVmObjList::getp_for_each_assoc
};


/* ------------------------------------------------------------------------ */
/*
 *   Static creation methods.  These routines allocate an object ID and
 *   create a new list object. 
 */

/* create dynamically using stack arguments */
vm_obj_id_t CVmObjList::create_from_stack(VMG_ const uchar **pc_ptr,
                                          uint argc)
{
    vm_obj_id_t id;
    CVmObjList *lst;
    size_t idx;
    
    /* 
     *   create the list - this type of construction is never used for
     *   root set objects
     */
    id = vm_new_id(vmg_ FALSE, TRUE, FALSE);

    /* create a list with one element per argument */
    lst = new (vmg_ id) CVmObjList(vmg_ argc);

    /* add each argument */
    for (idx = 0 ; idx < argc ; ++idx)
    {
        /* retrieve the next element from the stack and add it to the list */
        lst->cons_set_element(idx, G_stk->get(idx));
    }

    /* discard the stack parameters */
    G_stk->discard(argc);

    /* return the new object */
    return id;
}

/* 
 *   create dynamically from the current method's parameters 
 */
vm_obj_id_t CVmObjList::create_from_params(VMG_ uint param_idx, uint cnt)
{
    vm_obj_id_t id;
    CVmObjList *lst;
    size_t idx;

    /* create the new list object as a non-root-set object */
    id = vm_new_id(vmg_ FALSE, TRUE, FALSE);
    lst = new (vmg_ id) CVmObjList(vmg_ cnt);

    /* copy each parameter into the new list */
    for (idx = 0 ; cnt != 0 ; --cnt, ++param_idx, ++idx)
    {
        /* retrieve the next element and add it to the list */
        lst->cons_set_element(idx,
                              G_interpreter->get_param(vmg_ param_idx));
    }

    /* return the new object */
    return id;
}

/* create a list with no initial contents */
vm_obj_id_t CVmObjList::create(VMG_ int in_root_set)
{
    vm_obj_id_t id = vm_new_id(vmg_ in_root_set, TRUE, FALSE);
    new (vmg_ id) CVmObjList();
    return id;
}

/* create a list with a given number of elements */
vm_obj_id_t CVmObjList::create(VMG_ int in_root_set,
                               size_t element_count)
{
    vm_obj_id_t id = vm_new_id(vmg_ in_root_set, TRUE, FALSE);
    new (vmg_ id) CVmObjList(vmg_ element_count);
    return id;
}

/* create a list by copying a constant list */
vm_obj_id_t CVmObjList::create(VMG_ int in_root_set, const char *lst)
{
    vm_obj_id_t id = vm_new_id(vmg_ in_root_set, TRUE, FALSE);
    new (vmg_ id) CVmObjList(vmg_ lst);
    return id;
}

/* ------------------------------------------------------------------------ */
/*
 *   Constructors.  These are called indirectly through our static
 *   creation methods.  
 */

/*
 *   create a list object from a constant list 
 */
CVmObjList::CVmObjList(VMG_ const char *lst)
{
    size_t cnt;
    
    /* get the element count from the original list */
    cnt = vmb_get_len(lst);

    /* allocate space */
    alloc_list(vmg_ cnt);

    /* copy the list's contents */
    memcpy(ext_, lst, calc_alloc(cnt));
}

/*
 *   Create a list with a given number of elements.  This can be used to
 *   construct a list element-by-element. 
 */
CVmObjList::CVmObjList(VMG_ size_t cnt)
{
    /* allocate space */
    alloc_list(vmg_ cnt);

    /* 
     *   Clear the list.  Since the caller is responsible for populating the
     *   list in this version of the constructor, it's possible that GC will
     *   run between now and the time the list is fully populated.  We must
     *   initialize the list to ensure that we don't misinterpret the
     *   contents as valid should we run GC between now and the time the
     *   caller has finished populating the list.  It's adequate to set the
     *   list to all zeroes, since we won't try to interpret the contents as
     *   valid if the type markers are all invalid.  
     */
    memset(ext_ + VMB_LEN, 0, calc_alloc(cnt) - VMB_LEN);
}

/* ------------------------------------------------------------------------ */
/*
 *   allocate space for a list with a given number of elements 
 */
void CVmObjList::alloc_list(VMG_ size_t cnt)
{
    size_t alo;
    
    /* calculate the allocation size */
    alo = calc_alloc(cnt);

    /* 
     *   ensure we're within the limit (NB: this really is 65535 on ALL
     *   PLATFORMS - this is a portable limit imposed by the storage format,
     *   not a platform-specific size limit 
     */
    if (alo > 65535)
    {
        ext_ = 0;
        err_throw(VMERR_LIST_TOO_LONG);
    }

    /* allocate space for the given number of elements */
    ext_ = (char *)G_mem->get_var_heap()->alloc_mem(alo, this);

    /* set the element count */
    vmb_put_len(ext_, cnt);
}

/* ------------------------------------------------------------------------ */
/*
 *   construction: set an element 
 */
void CVmObjList::cons_set_element(size_t idx, const vm_val_t *val)
{
    /* set the element's value */
    vmb_put_dh(get_element_ptr(idx), val);
}

/*
 *   construction: copy a list into our list 
 */
void CVmObjList::cons_copy_elements(size_t idx, const char *orig_list)
{
    /* copy the elements */
    memcpy(get_element_ptr(idx), orig_list + VMB_LEN,
           (vmb_get_len(orig_list) * VMB_DATAHOLDER));
}

/*
 *   construction: copy element data into our list 
 */
void CVmObjList::cons_copy_data(size_t idx, const char *ele_array,
                                size_t ele_count)
{
    /* copy the elements */
    memcpy(get_element_ptr(idx), ele_array, ele_count * VMB_DATAHOLDER);
}


/* ------------------------------------------------------------------------ */
/*
 *   receive notification of deletion 
 */
void CVmObjList::notify_delete(VMG_ int in_root_set)
{
    /* free our extension */
    if (ext_ != 0 && !in_root_set)
        G_mem->get_var_heap()->free_mem(ext_);
}

/* ------------------------------------------------------------------------ */
/*
 *   Set a property.  Lists have no settable properties, so simply signal
 *   an error indicating that the set-prop call is invalid.  
 */
void CVmObjList::set_prop(VMG_ CVmUndo *, vm_obj_id_t,
                          vm_prop_id_t, const vm_val_t *)
{
    err_throw(VMERR_INVALID_SETPROP);
}

/* ------------------------------------------------------------------------ */
/*
 *   Save the object to a file 
 */
void CVmObjList::save_to_file(VMG_ CVmFile *fp)
{
    size_t cnt;

    /* get our element count */
    cnt = vmb_get_len(ext_);

    /* write the count and the elements */
    fp->write_bytes(ext_, calc_alloc(cnt));
}

/*
 *   Restore the object from a file 
 */
void CVmObjList::restore_from_file(VMG_ vm_obj_id_t,
                                   CVmFile *fp, CVmObjFixup *fixups)
{
    size_t cnt;

    /* read the element count */
    cnt = fp->read_uint2();

    /* free any existing extension */
    if (ext_ != 0)
    {
        G_mem->get_var_heap()->free_mem(ext_);
        ext_ = 0;
    }

    /* allocate the space */
    alloc_list(vmg_ cnt);

    /* store our element count */
    vmb_put_len(ext_, cnt);

    /* read the contents, if there are any elements */
    fp->read_bytes(ext_ + VMB_LEN, cnt * VMB_DATAHOLDER);

    /* fix object references */
    fixups->fix_dh_array(vmg_ ext_ + VMB_LEN, cnt);
}

/* ------------------------------------------------------------------------ */
/*
 *   Mark references 
 */
void CVmObjList::mark_refs(VMG_ uint state)
{
    size_t cnt;
    char *p;

    /* get my element count */
    cnt = vmb_get_len(ext_);

    /* mark as referenced each object in our list */
    for (p = get_element_ptr(0) ; cnt != 0 ; --cnt, inc_element_ptr(&p))
    {
        /* 
         *   if this is an object, mark it as referenced, and mark its
         *   references as referenced 
         */
        if (vmb_get_dh_type(p) == VM_OBJ)
            G_obj_table->mark_all_refs(vmb_get_dh_obj(p), state);
    }
}


/* ------------------------------------------------------------------------ */
/*
 *   Add a value to the list.  This yields a new list, with the value
 *   appended to the existing list.  If the value to be appended is itself
 *   a list (constant or object), we'll append each element of that list
 *   to our list (rather than appending a single element containing a
 *   sublist).  
 */
void CVmObjList::add_val(VMG_ vm_val_t *result,
                         vm_obj_id_t self, const vm_val_t *val)
{
    /* 
     *   Use the generic list adder, using my extension as the constant
     *   list.  We store our extension in the general list format required
     *   by the static adder. 
     */
    add_to_list(vmg_ result, self, ext_, val);
}

/*
 *   Static list adder.  This creates a new list object that results from
 *   appending the given value to the given list constant.  This is
 *   defined statically so that this code can be shared for adding to
 *   constant pool lists and adding to CVmObjList objects.
 *   
 *   'lstval' must point to a constant list.  The first two bytes of the
 *   list are stored in portable UINT2 format and give the number of
 *   elements in the list; this is immediately followed by a packed array
 *   of data holders in portable format. 
 *   
 *   Note that we *always* create a new object to hold the result, even if
 *   the new string is identical to the first, so that we consistently
 *   return a distinct reference from the original.  
 */
void CVmObjList::add_to_list(VMG_ vm_val_t *result,
                             vm_obj_id_t self, const char *lstval,
                             const vm_val_t *rhs)
{
    size_t lhs_cnt, rhs_cnt;
    vm_obj_id_t obj;
    CVmObjList *objptr;
    size_t i;

    /* push self and the other list for protection against GC */
    G_stk->push()->set_obj(self);
    G_stk->push(rhs);

    /* get the number of elements in the right-hand side */
    lhs_cnt = vmb_get_len(lstval);

    /* get the number of elements the right-hand side concatenates */
    rhs_cnt = rhs->get_coll_addsub_rhs_ele_cnt(vmg0_);

    /* allocate a new object to hold the new list */
    obj = create(vmg_ FALSE, lhs_cnt + rhs_cnt);
    objptr = (CVmObjList *)vm_objp(vmg_ obj);

    /* copy the first list into the new object's list buffer */
    objptr->cons_copy_elements(0, lstval);

    /* add each value from the right-hand side */
    for (i = 0 ; i < rhs_cnt ; ++i)
    {
        vm_val_t val;

        /* retrieve this element of the rhs (adjusting to a 1-based index) */
        rhs->get_coll_addsub_rhs_ele(vmg_ i + 1, &val);
        
        /* store the element in the new list */
        objptr->cons_set_element(lhs_cnt + i, &val);
    }

    /* set the result to the new list */
    result->set_obj(obj);

    /* discard the GC protection items */
    G_stk->discard(2);
}

/* ------------------------------------------------------------------------ */
/*
 *   Subtract a value from the list.
 */
void CVmObjList::sub_val(VMG_ vm_val_t *result,
                         vm_obj_id_t self, const vm_val_t *val)
{
    vm_val_t self_val;
    
    /* 
     *   Invoke our static list subtraction routine, using our extension
     *   as the constant list.  Our extension is stored in the same format
     *   as a constant list, so we can use the same code to handle
     *   subtraction from a list object as we would for subtraction from a
     *   constant list. 
     */
    self_val.set_obj(self);
    sub_from_list(vmg_ result, &self_val, ext_, val);
}

/*
 *   Subtract a value from a constant list. 
 */
void CVmObjList::sub_from_list(VMG_ vm_val_t *result,
                               const vm_val_t *lstval, const char *lstmem,
                               const vm_val_t *rhs)
{
    size_t lhs_cnt, rhs_cnt;
    vm_obj_id_t obj;
    CVmObjList *objptr;
    size_t i;
    char *dst;
    size_t dst_cnt;

    /* push self and the other list for protection against GC */
    G_stk->push(lstval);
    G_stk->push(rhs);

    /* get the number of elements in the right-hand side */
    lhs_cnt = vmb_get_len(lstmem);

    /* 
     *   allocate a new object to hold the new list, which will be no
     *   bigger than the original left-hand side, since we're doing
     *   nothing but (possibly) taking elements out 
     */
    obj = create(vmg_ FALSE, lhs_cnt);
    objptr = (CVmObjList *)vm_objp(vmg_ obj);

    /* get the number of elements to consider from the right-hand side */
    rhs_cnt = rhs->get_coll_addsub_rhs_ele_cnt(vmg0_);

    /* copy the first list into the new object's list buffer */
    objptr->cons_copy_elements(0, lstmem);

    /* consider each element of the left-hand side */
    for (i = 0, dst = objptr->get_element_ptr(0), dst_cnt = 0 ;
         i < lhs_cnt ; ++i)
    {
        vm_val_t src_val;
        int keep;
        size_t j;

        /* 
         *   if our list is from constant memory, get its address again --
         *   the address could have changed due to swapping if we
         *   traversed into another list 
         */
        VM_IF_SWAPPING_POOL(if (lstval != 0 && lstval->typ == VM_LIST)
            lstmem = G_const_pool->get_ptr(lstval->val.ofs));

        /* get this element */
        vmb_get_dh(get_element_ptr_const(lstmem, i), &src_val);

        /* presume we'll keep it */
        keep = TRUE;

        /* 
         *   scan the right side to see if we can find this value - if we
         *   can, it's to be removed, so we don't want to copy it to the
         *   result list 
         */
        for (j = 0 ; j < rhs_cnt ; ++j)
        {
            vm_val_t rem_val;
            
            /* retrieve this rhs value (using a 1-based index) */
            rhs->get_coll_addsub_rhs_ele(vmg_ j + 1, &rem_val);

            /* if this value matches, we're removing it */
            if (rem_val.equals(vmg_ &src_val))
            {
                /* it's to be removed */
                keep = FALSE;

                /* no need to look any further in the rhs list */
                break;
            }
        }

        /* if we're keeping the value, put it in the result list */
        if (keep)
        {
            /* store it in the result list */
            vmb_put_dh(dst, &src_val);

            /* advance the result pointer */
            inc_element_ptr(&dst);

            /* count it */
            ++dst_cnt;
        }
    }

    /* set the length of the result list */
    objptr->cons_set_len(dst_cnt);

    /* set the result to the new list */
    result->set_obj(obj);

    /* discard the GC protection items */
    G_stk->discard(2);
}

/* ------------------------------------------------------------------------ */
/*
 *   Index the list 
 */
void CVmObjList::index_val(VMG_ vm_val_t *result, vm_obj_id_t /*self*/,
                           const vm_val_t *index_val)
{
    /* 
     *   use the constant list indexing routine, using our extension data
     *   as the list data 
     */
    index_list(vmg_ result, ext_, index_val);
}

/* ------------------------------------------------------------------------ */
/*
 *   Index a constant list 
 */
void CVmObjList::index_list(VMG_ vm_val_t *result, const char *lst,
                            const vm_val_t *index_val)
{
    uint32 idx;
    
    /* get the index value as an integer */
    idx = index_val->num_to_int();

    /* index the list */
    index_list(vmg_ result, lst, idx);
}

/*
 *   Index a constant list by an integer value
 */
void CVmObjList::index_list(VMG_ vm_val_t *result, const char *lst, uint idx)
{
    /* make sure it's in range - 1 to our element count, inclusive */
    if (idx < 1 || idx > vmb_get_len(lst))
        err_throw(VMERR_INDEX_OUT_OF_RANGE);

    /* 
     *   get the indexed element and store it in the result, adjusting the
     *   index to the C-style 0-based range 
     */
    get_element_const(lst, idx - 1, result);
}

/* ------------------------------------------------------------------------ */
/*
 *   Set an element of the list
 */
void CVmObjList::set_index_val(VMG_ vm_val_t *result, vm_obj_id_t self,
                               const vm_val_t *index_val,
                               const vm_val_t *new_val)
{
    /* put the list on the stack to avoid garbage collection */
    G_stk->push()->set_obj(self);
    
    /* 
     *   use the constant list set-index routine, using our extension data
     *   as the list data 
     */
    set_index_list(vmg_ result, ext_, index_val, new_val);

    /* discard the GC protection */
    G_stk->discard();
}

/* ------------------------------------------------------------------------ */
/*
 *   Set an element in a constant list 
 */
void CVmObjList::set_index_list(VMG_ vm_val_t *result, const char *lst,
                                const vm_val_t *index_val,
                                const vm_val_t *new_val)
{
    uint32 idx;
    CVmObjList *obj;

    /* get the index value as an integer */
    idx = index_val->num_to_int();

    /* push the new value for gc protection during the create */
    G_stk->push(new_val);

    /* make sure it's in range - 1 to our element count, inclusive */
    if (idx < 1 || idx > vmb_get_len(lst))
        err_throw(VMERR_INDEX_OUT_OF_RANGE);

    /* create a new list as a copy of this list */
    result->set_obj(create(vmg_ FALSE, lst));

    /* get the new list object */
    obj = (CVmObjList *)vm_objp(vmg_ result->val.obj);

    /* update the element of the new list */
    obj->cons_set_element(idx - 1, new_val);

    /* discard our gc protection */
    G_stk->discard();
}

/* ------------------------------------------------------------------------ */
/*
 *   Check a value for equality 
 */
int CVmObjList::equals(VMG_ vm_obj_id_t self, const vm_val_t *val,
                       int depth) const
{
    /* if it's a reference to myself, we certainly match */
    if (val->typ == VM_OBJ && val->val.obj == self)
        return TRUE;

    /* 
     *   compare via the constant list comparison routine, using our
     *   extension data as the list data 
     */
    return const_equals(vmg_ 0, ext_, val, depth);
}

/* ------------------------------------------------------------------------ */
/*
 *   Constant list comparison routine 
 */
int CVmObjList::const_equals(VMG_ const vm_val_t *lstval, const char *lstmem,
                             const vm_val_t *val, int depth)
{
    const char *lstmem2;
    const vm_val_t *lstval2;
    size_t cnt;
    size_t idx;
    
    /* get the list underlying the other value */
    lstmem2 = val->get_as_list(vmg0_);
    lstval2 = val;

    /* if it doesn't have an underlying list, it doesn't match */
    if (lstmem2 == 0)
        return FALSE;

    /* if the lists don't have the same length, they don't match */
    cnt = vmb_get_len(lstmem);
    if (cnt != vmb_get_len(lstmem2))
        return FALSE;

    /* compare each element in the list */
    for (idx = 0 ; idx < cnt ; ++idx)
    {
        vm_val_t val1;
        vm_val_t val2;

        /* 
         *   if either list comes from constant memory, re-translate its
         *   pointer, in case we did any swapping while traversing the
         *   previous element 
         */
        VM_IF_SWAPPING_POOL(if (lstval != 0 && lstval->typ == VM_LIST)
            lstmem = G_const_pool->get_ptr(lstval->val.ofs));
        VM_IF_SWAPPING_POOL(if (lstval2 != 0 && lstval2->typ == VM_LIST)
            lstmem2 = G_const_pool->get_ptr(lstval2->val.ofs));
        
        /* get the two elements */
        vmb_get_dh(get_element_ptr_const(lstmem, idx), &val1);
        vmb_get_dh(get_element_ptr_const(lstmem2, idx), &val2);

        /* 
         *   If these elements don't match, the lists don't match.  Note that
         *   lists can't contain circular references (by their very nature as
         *   immutable objects), so we don't need to increase the depth. 
         */
        if (!val1.equals(vmg_ &val2, depth))
            return FALSE;
    }

    /* if we got here, we didn't find any differences, so they match */
    return TRUE;
}

/* ------------------------------------------------------------------------ */
/*
 *   Hash value calculation 
 */
uint CVmObjList::calc_hash(VMG_ vm_obj_id_t self, int depth) const
{
    vm_val_t self_val;

    /* set up our 'self' value pointer */
    self_val.set_obj(self);

    /* calculate the value */
    return const_calc_hash(vmg_ &self_val, ext_, depth);
}

/*
 *   Hash value calculation 
 */
uint CVmObjList::const_calc_hash(VMG_ const vm_val_t *self_val,
                                 const char *lst, int depth)
{
    size_t len;
    size_t i;
    uint hash;

    /* get and skip the length prefix */
    len = vmb_get_len(lst);
    
    /* calculate a hash combining the hash of each element in the list */
    for (hash = 0, i = 0 ; i < len ; ++i)
    {
        vm_val_t ele;
        
        /* re-translate in case of swapping */
        VM_IF_SWAPPING_POOL(if (self_val->typ == VM_LIST)
            lst = G_const_pool->get_ptr(self_val->val.ofs));

        /* get this element */
        get_element_const(lst, i, &ele);

        /* 
         *   Compute its hash value and add it into the total.  Note that
         *   even though we're recursively calculating the hash of an
         *   element, we don't need to increase the recursion depth, because
         *   it's impossible for a list to have cycles.
         *   
         *   (It's not possible for a list to have cycles because a list is
         *   always constructed with its contents, and can never be changed.
         *   This means that there's no possibility of storing a reference to
         *   the new list inside the list itself, or inside any other list
         *   the list refers to.  It *is* possible to put the reference to
         *   the new list in a mutable object to which the list refers, but
         *   in such cases, that mutable object will be capable of having
         *   cycles in its references, so it will be responsible for
         *   increasing the depth counter when it recurses.)  
         */
        hash += ele.calc_hash(vmg_ depth);
    }

    /* return the hash value */
    return hash;
}


/* ------------------------------------------------------------------------ */
/*
 *   Find a value in a list 
 */
int CVmObjList::find_in_list(VMG_ const vm_val_t *lst,
                             const vm_val_t *val, size_t *idxp)
{
    const char *lstmem;
    size_t cnt;
    size_t idx;
    
    /* get the list underyling this value */
    lstmem = lst->get_as_list(vmg0_);

    /* get the length of the list */
    cnt = vmb_get_len(lstmem);

    /* scan the list for the value */
    for (idx = 0 ; idx < cnt ; ++idx)
    {
        vm_val_t curval;

        /* 
         *   re-translate the list pointer if it's in constant memory, in
         *   case we did any swapping on the last iteratino 
         */
        VM_IF_SWAPPING_POOL(if (lst->typ == VM_LIST)
            lstmem = G_const_pool->get_ptr(lst->val.ofs));
        
        /* get this list element */
        vmb_get_dh(get_element_ptr_const(lstmem, idx), &curval);

        /* compare this value to the one we're looking for */
        if (curval.equals(vmg_ val))
        {
            /* this is the one - set the return index */
            if (idxp != 0)
                *idxp = idx;

            /* indicate that we found the value */
            return TRUE;
        }
    }

    /* we didn't find the value */
    return FALSE;
}

/*
 *   Find the last match for a value in a list 
 */
int CVmObjList::find_last_in_list(VMG_ const vm_val_t *lst,
                                  const vm_val_t *val, size_t *idxp)
{
    const char *lstmem;
    size_t cnt;
    size_t idx;

    /* get the list underyling this value */
    lstmem = lst->get_as_list(vmg0_);

    /* get the length of the list */
    cnt = vmb_get_len(lstmem);

    /* scan the list for the value */
    for (idx = cnt ; idx != 0 ; --idx)
    {
        vm_val_t curval;

        /* 
         *   re-translate the list pointer if it's in constant memory, in
         *   case we did any swapping on the last iteratino 
         */
        VM_IF_SWAPPING_POOL(if (lst->typ == VM_LIST)
            lstmem = G_const_pool->get_ptr(lst->val.ofs));

        /* get this list element */
        vmb_get_dh(get_element_ptr_const(lstmem, idx), &curval);

        /* compare this value to the one we're looking for */
        if (curval.equals(vmg_ val))
        {
            /* this is the one - set the return index */
            if (idxp != 0)
                *idxp = idx;

            /* indicate that we found the value */
            return TRUE;
        }
    }

    /* we didn't find the value */
    return FALSE;
}

/* ------------------------------------------------------------------------ */
/*
 *   Compute the intersection of two lists.  Returns a new list with the
 *   elements that occur in both lists.  
 */
vm_obj_id_t CVmObjList::intersect(VMG_ const vm_val_t *lst1,
                                  const vm_val_t *lst2)
{
    const char *lstmem1;
    const char *lstmem2;
    size_t cnt1;
    size_t cnt2;
    size_t idx;
    vm_obj_id_t resobj;
    CVmObjList *reslst;
    size_t residx;

    /* get the two list memory blocks */
    lstmem1 = lst1->get_as_list(vmg0_);
    lstmem2 = lst2->get_as_list(vmg0_);

    /* get the lengths of the lists */
    cnt1 = vmb_get_len(lstmem1);
    cnt2 = vmb_get_len(lstmem2);

    /* if the first list is larger than the second, swap them */
    if (cnt1 > cnt2)
    {
        const vm_val_t *tmp;

        /* swap the vm_val_t pointers */
        tmp = lst1;
        lst1 = lst2;
        lst2 = tmp;

        /* 
         *   momentarily forget the larger count and memory pointer, and
         *   copy the smaller count into cnt1 
         */
        cnt1 = cnt2;
        lstmem1 = lstmem2;
    }

    /* 
     *   Allocate our result list.  The result list can't have any more
     *   elements in it than the shorter of the two lists, whose length is
     *   now in cnt1. 
     */
    resobj = create(vmg_ FALSE, cnt1);
    reslst = (CVmObjList *)vm_objp(vmg_ resobj);

    /* we haven't put any elements in the result list yet */
    residx = 0;

    /* 
     *   for each element in the first list, find the element in the
     *   second list 
     */
    for (idx = 0 ; idx < cnt1 ; ++idx)
    {
        vm_val_t curval;
        
        /* re-translate the first list address in case of swapping */
        if (lst1->typ == VM_LIST)
            lstmem1 = lst1->get_as_list(vmg0_);

        /* get this element from the first list */
        vmb_get_dh(get_element_ptr_const(lstmem1, idx), &curval);

        /* find the element in the second list */
        if (find_in_list(vmg_ lst2, &curval, 0))
        {
            /* we found it - copy it into the result list */
            reslst->cons_set_element(residx, &curval);

            /* count the new entry in the result list */
            ++residx;
        }
    }

    /* 
     *   set the actual result length, which might be shorter than the
     *   amount we allocated 
     */
    reslst->cons_set_len(residx);

    /* return the result list */
    return resobj;
}

/* ------------------------------------------------------------------------ */
/*
 *   Uniquify the list; modifies the list in place, so this can only be
 *   used during construction of a new list 
 */
void CVmObjList::cons_uniquify(VMG0_)
{
    size_t cnt;
    size_t src, dst;

    /* get the length of the list */
    cnt = vmb_get_len(ext_);

    /* loop through the list and look for repeated values */
    for (src = dst = 0 ; src < cnt ; ++src)
    {
        size_t idx;
        vm_val_t src_val;
        int found;

        /* 
         *   look for a copy of this source value already in the output
         *   list 
         */
        index_list(vmg_ &src_val, ext_, src + 1);
        for (idx = 0, found = FALSE ; idx < dst ; ++idx)
        {
            vm_val_t idx_val;

            /* get this value */
            index_list(vmg_ &idx_val, ext_, idx + 1);

            /* if it's equal to the current source value, note it */
            if (src_val.equals(vmg_ &idx_val))
            {
                /* note that we found it */
                found = TRUE;

                /* no need to look any further */
                break;
            }
        }

        /* if we didn't find the value, copy it to the output list */
        if (!found)
        {
            /* add it to the output list */
            cons_set_element(dst, &src_val);

            /* count it */
            ++dst;
        }
    }

    /* adjust the size of the result list */
    cons_set_len(dst);
}

/* ------------------------------------------------------------------------ */
/*
 *   Create an iterator 
 */
void CVmObjList::new_iterator(VMG_ vm_val_t *retval,
                              const vm_val_t *self_val)
{
    size_t len;
    
    /* get the number of elements in the list */
    len = vmb_get_len(self_val->get_as_list(vmg0_));

    /* 
     *   Set up a new indexed iterator object.  The first valid index for
     *   a list is always 1, and the last valid index is the same as the
     *   number of elements in the list. 
     */
    retval->set_obj(CVmObjIterIdx::create_for_coll(vmg_ self_val, 1, len));
}

/* ------------------------------------------------------------------------ */
/*
 *   Evaluate a property 
 */
int CVmObjList::get_prop(VMG_ vm_prop_id_t prop, vm_val_t *retval,
                         vm_obj_id_t self, vm_obj_id_t *source_obj,
                         uint *argc)
{
    vm_val_t self_val;
    
    /* use the constant evaluator */
    self_val.set_obj(self);
    if (const_get_prop(vmg_ retval, &self_val, ext_, prop, source_obj, argc))
    {
        *source_obj = metaclass_reg_->get_class_obj(vmg0_);
        return TRUE;
    }

    /* inherit default handling from the base object class */
    return CVmObjCollection::get_prop(vmg_ prop, retval, self,
                                      source_obj, argc);
}

/* ------------------------------------------------------------------------ */
/*
 *   Evaluate a property of a constant list value 
 */
int CVmObjList::const_get_prop(VMG_ vm_val_t *retval,
                               const vm_val_t *self_val, const char *lst,
                               vm_prop_id_t prop, vm_obj_id_t *src_obj,
                               uint *argc)
{
    ushort func_idx;

    /* presume no source object */
    *src_obj = VM_INVALID_OBJ;

    /* translate the property index to an index into our function table */
    func_idx = G_meta_table
               ->prop_to_vector_idx(metaclass_reg_->get_reg_idx(), prop);
    
    /* call the appropriate function */
    if ((*func_table_[func_idx])(vmg_ retval, self_val, lst, argc))
        return TRUE;

    /* 
     *   If this is a constant list (which is indicated by a non-object type
     *   'self'), try inheriting the default object interpretation, passing
     *   the constant list placeholder object for its type information.  
     */
    if (self_val->typ != VM_OBJ)
    {
        /* try going to our base class, CVmCollection */
        if (((CVmObjCollection *)vm_objp(vmg_ G_predef->const_lst_obj))
            ->const_get_coll_prop(vmg_ prop, retval, self_val, src_obj, argc))
            return TRUE;

        /* try going to our next base class, CVmObject */
        if (vm_objp(vmg_ G_predef->const_lst_obj)
            ->CVmObject::get_prop(vmg_ prop, retval, G_predef->const_lst_obj,
                                  src_obj, argc))
            return TRUE;
    }

    /* not handled */
    return FALSE;
}

/* ------------------------------------------------------------------------ */
/*
 *   property evaluator - select a subset through a callback
 */
int CVmObjList::getp_subset(VMG_ vm_val_t *retval, const vm_val_t *self_val,
                            const char *lst, uint *argc)
{
    const vm_val_t *func_val;
    size_t src;
    size_t dst;
    size_t cnt;
    char *new_lst;
    CVmObjList *new_lst_obj;
    static CVmNativeCodeDesc desc(1);
    
    /* check arguments */
    if (get_prop_check_argc(retval, argc, &desc))
        return TRUE;

    /* get the function pointer argument, but leave it on the stack */
    func_val = G_stk->get(0);

    /* push a self-reference while allocating to protect from gc */
    G_stk->push(self_val);

    /* 
     *   Make a copy of our list for the return value.  The result value
     *   will be at most the same size as our current list; since we have
     *   no way of knowing exactly how large it will be, and since we
     *   don't want to run through the selection functions twice, we'll
     *   just allocate at the maximum size and leave it partially unused
     *   if we don't need all of the space.  By making a copy of the input
     *   list, we also can avoid worrying about whether the input list was
     *   a constant, and hence we don't have to worry about the
     *   possibility of constant page swapping.  
     */
    retval->set_obj(create(vmg_ FALSE, lst));

    /* get the return value list data */
    new_lst_obj = (CVmObjList *)vm_objp(vmg_ retval->val.obj);
    new_lst = new_lst_obj->ext_;

    /* get the length of the list */
    cnt = vmb_get_len(new_lst);

    /* 
     *   push a reference to the new list to protect it from the garbage
     *   collector, which could be invoked in the course of executing the
     *   user callback 
     */
    G_stk->push(retval);

    /*
     *   Go through each element of our list, and invoke the callback on
     *   each element.  If the element passes, write it to the current
     *   output location in the list; otherwise, just skip it.
     *   
     *   Note that we're using the same list as source and destination,
     *   which is easy because the list will either shrink or stay the
     *   same - we'll never need to insert new elements.  
     */
    for (src = dst = 0 ; src < cnt ; ++src)
    {
        vm_val_t ele;
        const vm_val_t *val;
        
        /* 
         *   get this element (using a 1-based index), and push it as the
         *   callback's argument 
         */
        index_list(vmg_ &ele, new_lst, src + 1);
        G_stk->push(&ele);

        /* invoke the callback */
        G_interpreter->call_func_ptr(vmg_ func_val, 1, "list.subset", 0);

        /* get the result from R0 */
        val = G_interpreter->get_r0();

        /* 
         *   if the callback returned non-nil and non-zero, include this
         *   element in the result 
         */
        if (val->typ == VM_NIL
            || (val->typ == VM_INT && val->val.intval == 0))
        {
            /* it's nil or zero - don't include it in the result */
        }
        else
        {
            /* include this element in the result */
            new_lst_obj->cons_set_element(dst, &ele);

            /* advance the output index */
            ++dst;
        }
    }

    /* 
     *   set the result list length to the number of elements we actually
     *   copied 
     */
    new_lst_obj->cons_set_len(dst);

    /* discard our gc protection (self, return value) and our arguments */
    G_stk->discard(3);

    /* handled */
    return TRUE;
}


/* ------------------------------------------------------------------------ */
/*
 *   property evaluator - map through a callback
 */
int CVmObjList::getp_map(VMG_ vm_val_t *retval, const vm_val_t *self_val,
                         const char *lst, uint *argc)
{
    const vm_val_t *func_val;
    size_t cnt;
    size_t idx;
    char *new_lst;
    CVmObjList *new_lst_obj;
    static CVmNativeCodeDesc desc(1);

    /* check arguments */
    if (get_prop_check_argc(retval, argc, &desc))
        return TRUE;

    /* get the function pointer argument, but leave it on the stack */
    func_val = G_stk->get(0);

    /* push a self-reference while allocating to protect from gc */
    G_stk->push(self_val);

    /* 
     *   Make a copy of our list for the return value, since the result
     *   value is always the same size as our current list.  By making a
     *   copy of the input list, we also can avoid worrying about whether
     *   the input list was a constant, and hence we don't have to worry
     *   about the possibility of constant page swapping - we'll just
     *   update elements of the copy in-place.  
     */
    retval->set_obj(create(vmg_ FALSE, lst));

    /* get the return value list data */
    new_lst_obj = (CVmObjList *)vm_objp(vmg_ retval->val.obj);
    new_lst = new_lst_obj->ext_;

    /* get the length of the list */
    cnt = vmb_get_len(new_lst);

    /* 
     *   push a reference to the new list to protect it from the garbage
     *   collector, which could be invoked in the course of executing the
     *   user callback 
     */
    G_stk->push(retval);

    /*
     *   Go through each element of our list, and invoke the callback on
     *   each element.  Replace each element with the result of the
     *   callback.  
     */
    for (idx = 0 ; idx < cnt ; ++idx)
    {
        /* 
         *   get this element (using a 1-based index), and push it as the
         *   callback's argument 
         */
        index_and_push(vmg_ new_lst, idx + 1);
        
        /* invoke the callback */
        G_interpreter->call_func_ptr(vmg_ func_val, 1, "list.mapAll", 0);

        /* store the result in the list */
        new_lst_obj->cons_set_element(idx, G_interpreter->get_r0());
    }

    /* discard our gc protection (self, return value) and our arguments */
    G_stk->discard(3);

    /* handled */
    return TRUE;
}

/* ------------------------------------------------------------------------ */
/*
 *   property evaluator - length
 */
int CVmObjList::getp_len(VMG_ vm_val_t *retval, const vm_val_t *self_val,
                         const char *lst, uint *argc)
{
    static CVmNativeCodeDesc desc(0);

    /* check arguments */
    if (get_prop_check_argc(retval, argc, &desc))
        return TRUE;

    /* return the element count */
    retval->set_int(vmb_get_len(lst));

    /* handled */
    return TRUE;
}

/* ------------------------------------------------------------------------ */
/*
 *   property evaluator - sublist
 */
int CVmObjList::getp_sublist(VMG_ vm_val_t *retval, const vm_val_t *self_val,
                             const char *lst, uint *in_argc)
{
    uint argc = (in_argc != 0 ? *in_argc : 0);
    ulong start;
    ulong len;
    vm_obj_id_t obj;
    size_t old_cnt;
    size_t new_cnt;
    static CVmNativeCodeDesc desc(1, 1);

    /* check arguments */
    if (get_prop_check_argc(retval, in_argc, &desc))
        return TRUE;

    /* get the original element count */
    old_cnt = vmb_get_len(lst);

    /* pop the starting index */
    start = CVmBif::pop_long_val(vmg0_);

    /* 
     *   pop the length, if present; if not, use the current element
     *   count, which will ensure that we use all available elements of
     *   the sublist 
     */
    if (argc >= 2)
        len = CVmBif::pop_long_val(vmg0_);
    else
        len = old_cnt;

    /* push the 'self' as protection from GC */
    G_stk->push(self_val);

    /* skip to the first element */
    lst += VMB_LEN;

    /* skip to the desired first element */
    if (start >= 1 && start <= old_cnt)
    {
        /* it's in range - skip to the desired first element */
        lst += (start - 1) * VMB_DATAHOLDER;
        new_cnt = old_cnt - (start - 1);
    }
    else
    {
        /* there's nothing left */
        new_cnt = 0;
    }

    /* 
     *   limit the result to the desired new count, if it's shorter than
     *   what we have left (we obviously can't give them more elements
     *   than we have remaining) 
     */
    if (len < new_cnt)
        new_cnt = (size_t)len;

    /* create the new list */
    obj = create(vmg_ FALSE, new_cnt);

    /* copy the elements */
    ((CVmObjList *)vm_objp(vmg_ obj))->cons_copy_data(0, lst, new_cnt);

    /* return the new object */
    retval->set_obj(obj);

    /* discard GC protection */
    G_stk->discard();

    /* handled */
    return TRUE;
}

/* ------------------------------------------------------------------------ */
/*
 *   property evaluator - intersect
 */
int CVmObjList::getp_intersect(VMG_ vm_val_t *retval,
                               const vm_val_t *self_val,
                               const char *lst, uint *argc)
{
    vm_val_t val2;
    vm_obj_id_t obj;
    static CVmNativeCodeDesc desc(1);

    /* check arguments */
    if (get_prop_check_argc(retval, argc, &desc))
        return TRUE;

    /* get the second list, but leave it on the stack for GC protection */
    val2 = *G_stk->get(0);

    /* put myself on the stack for GC protection as well */
    G_stk->push(self_val);

    /* make sure the other value is indeed a list */
    if (val2.get_as_list(vmg0_) == 0)
        err_throw(VMERR_LIST_VAL_REQD);

    /* compute the intersection */
    obj = intersect(vmg_ self_val, &val2);

    /* discard the argument lists */
    G_stk->discard(2);

    /* return the new object */
    retval->set_obj(obj);

    /* handled */
    return TRUE;
}

/* ------------------------------------------------------------------------ */
/*
 *   property evaluator - indexOf
 */
int CVmObjList::getp_index_of(VMG_ vm_val_t *retval, const vm_val_t *self_val,
                              const char *lst, uint *argc)
{
    vm_val_t subval;
    size_t idx;
    static CVmNativeCodeDesc desc(1);

    /* check arguments */
    if (get_prop_check_argc(retval, argc, &desc))
        return TRUE;

    /* pop the value to find */
    G_stk->pop(&subval);

    /* find the value in the list */
    if (find_in_list(vmg_ self_val, &subval, &idx))
    {
        /* found it - adjust to 1-based index for return */
        retval->set_int(idx + 1);
    }
    else
    {
        /* didn't find it - return nil */
        retval->set_nil();
    }

    /* handled */
    return TRUE;
}

/* ------------------------------------------------------------------------ */
/*
 *   property evaluator - car
 */
int CVmObjList::getp_car(VMG_ vm_val_t *retval, const vm_val_t *self_val,
                         const char *lst, uint *argc)
{
    static CVmNativeCodeDesc desc(0);

    /* check arguments */
    if (get_prop_check_argc(retval, argc, &desc))
        return TRUE;

    /* 
     *   if the list has at least one element, return it; otherwise return
     *   nil 
     */
    if (vmb_get_len(lst) == 0)
    {
        /* no elements - return nil */
        retval->set_nil();
    }
    else
    {
        /* it has at least one element - return the first element */
        vmb_get_dh(lst + VMB_LEN, retval);
    }

    /* handled */
    return TRUE;
}

/* ------------------------------------------------------------------------ */
/*
 *   property evaluator - cdr
 */
int CVmObjList::getp_cdr(VMG_ vm_val_t *retval, const vm_val_t *self_val,
                         const char *lst, uint *argc)
{
    static CVmNativeCodeDesc desc(0);

    /* check arguments */
    if (get_prop_check_argc(retval, argc, &desc))
        return TRUE;

    /* push a self-reference for GC protection */
    G_stk->push(self_val);

    /* 
     *   if the list has no elements, return nil; otherwise, return the
     *   sublist starting with the second element (thus return an empty
     *   list if the original list has only one element) 
     */
    if (vmb_get_len(lst) == 0)
    {
        /* no elements - return nil */
        retval->set_nil();
    }
    else
    {
        vm_obj_id_t obj;
        size_t new_cnt;

        /* reduce the list count by one */
        new_cnt = vmb_get_len(lst) - 1;

        /* skip past the first element */
        lst += VMB_LEN + VMB_DATAHOLDER;

        /* create the new list */
        obj = create(vmg_ FALSE, new_cnt);

        /* copy the elements */
        ((CVmObjList *)vm_objp(vmg_ obj))->cons_copy_data(0, lst, new_cnt);

        /* return the new object */
        retval->set_obj(obj);
    }

    /* discard the stack protection */
    G_stk->discard();

    /* handled */
    return TRUE;
}

/* ------------------------------------------------------------------------ */
/*
 *   property evaluator - indexWhich
 */
int CVmObjList::getp_index_which(VMG_ vm_val_t *retval,
                                 const vm_val_t *self_val,
                                 const char *lst, uint *argc)
{
    /* use the generic index-which routine, stepping forward */
    return gen_index_which(vmg_ retval, self_val, lst, argc, TRUE);
}

/*
 *   general index finder for indexWhich and lastIndexWhich - steps either
 *   forward or backward through the list 
 */
int CVmObjList::gen_index_which(VMG_ vm_val_t *retval,
                                const vm_val_t *self_val,
                                const char *lst, uint *argc,
                                int forward)
{
    const vm_val_t *func_val;
    size_t cnt;
    size_t idx;
    static CVmNativeCodeDesc desc(1);

    /* check arguments */
    if (get_prop_check_argc(retval, argc, &desc))
        return TRUE;

    /* get the function pointer argument, but leave it on the stack */
    func_val = G_stk->get(0);

    /* push a self-reference while allocating to protect from gc */
    G_stk->push(self_val);

    /* get the length of the list */
    cnt = vmb_get_len(lst);

    /* presume that we won't find any element that satisfies the condition */
    retval->set_nil();

    /* 
     *   start at either the first or last index, depending on which way
     *   we're stepping
     */
    idx = (forward ? 1 : cnt);

    /*
     *   Go through each element of our list, and invoke the callback on
     *   the element.  Stop when we reach the first element that returns
     *   true, or when we run out of elements.  
     */
    for (;;)
    {
        /* if we're out of elements, stop now */
        if (forward ? idx > cnt : idx == 0)
            break;

        /* re-translate the list address in case of swapping */
        if (self_val->typ == VM_LIST)
            lst = self_val->get_as_list(vmg0_);

        /* get this element, and push it as the callback's argument */
        index_and_push(vmg_ lst, idx);

        /* invoke the callback */
        G_interpreter->call_func_ptr(vmg_ func_val, 1, "list.indexWhich", 0);

        /* 
         *   if the callback returned true, we've found the element we're
         *   looking for 
         */
        if (G_interpreter->get_r0()->typ == VM_NIL
            || (G_interpreter->get_r0()->typ == VM_INT
                && G_interpreter->get_r0()->val.intval == 0))
        {
            /* nil or zero - this one failed the test, so keep looking */
        }
        else
        {
            /* it passed the test - return its index */
            retval->set_int(idx);

            /* no need to keep searching - we found what we're looking for */
            break;
        }

        /* advance to the next element */
        if (forward)
            ++idx;
        else
            --idx;
    }

    /* discard our gc protection (self) and our arguments */
    G_stk->discard(2);

    /* handled */
    return TRUE;
}

/* ------------------------------------------------------------------------ */
/*
 *   property evaluator - forEach
 */
int CVmObjList::getp_for_each(VMG_ vm_val_t *retval,
                              const vm_val_t *self_val,
                              const char *lst, uint *argc)
{
    /* use the generic forEach/forEachAssoc processor */
    return for_each_gen(vmg_ retval, self_val, lst, argc, FALSE);
}

/*
 *   property evaluator - forEachAssoc 
 */
int CVmObjList::getp_for_each_assoc(VMG_ vm_val_t *retval,
                                    const vm_val_t *self_val,
                                    const char *lst, uint *argc)
{
    /* use the generic forEach/forEachAssoc processor */
    return for_each_gen(vmg_ retval, self_val, lst, argc, TRUE);
}

/*
 *   General forEach processor - combines the functionality of forEach and
 *   forEachAssoc, using a flag to specify whether or not to pass the index
 *   of each element to the callback. 
 */
int CVmObjList::for_each_gen(VMG_ vm_val_t *retval,
                             const vm_val_t *self_val,
                             const char *lst, uint *argc,
                             int send_idx_to_cb)
{
    const vm_val_t *func_val;
    size_t cnt;
    size_t idx;
    static CVmNativeCodeDesc desc(1);

    /* check arguments */
    if (get_prop_check_argc(retval, argc, &desc))
        return TRUE;

    /* get the function pointer argument, but leave it on the stack */
    func_val = G_stk->get(0);

    /* push a self-reference while allocating to protect from gc */
    G_stk->push(self_val);

    /* get the length of the list */
    cnt = vmb_get_len(lst);

    /* no return value */
    retval->set_nil();

    /* invoke the callback on each element */
    for (idx = 1 ; idx <= cnt ; ++idx)
    {
        /* re-translate the list address in case of swapping */
        if (self_val->typ == VM_LIST)
            lst = self_val->get_as_list(vmg0_);

        /* 
         *   get this element (using a 1-based index) and push it as the
         *   callback's argument 
         */
        index_and_push(vmg_ lst, idx);

        /* push the index, if desired */
        if (send_idx_to_cb)
            G_stk->push()->set_int(idx);

        /* invoke the callback */
        G_interpreter->call_func_ptr(vmg_ func_val, send_idx_to_cb ? 2 : 1,
                                     "list.forEach", 0);
    }

    /* discard our gc protection (self) and our arguments */
    G_stk->discard(2);

    /* handled */
    return TRUE;
}

/* ------------------------------------------------------------------------ */
/*
 *   property evaluator - valWhich
 */
int CVmObjList::getp_val_which(VMG_ vm_val_t *retval,
                               const vm_val_t *self_val,
                               const char *lst, uint *argc)
{
    /* get the index of the value using indexWhich */
    getp_index_which(vmg_ retval, self_val, lst, argc);

    /* if the return value is a valid index, get the value at the index */
    if (retval->typ == VM_INT)
    {
        int idx;
        
        /* re-translate the list address in case of swapping */
        if (self_val->typ == VM_LIST)
            lst = self_val->get_as_list(vmg0_);

        /* get the element as the return value */
        idx = (int)retval->val.intval;
        index_list(vmg_ retval, lst, idx);
    }
    
    /* handled */
    return TRUE;
}

/* ------------------------------------------------------------------------ */
/*
 *   property evaluator - lastIndexOf
 */
int CVmObjList::getp_last_index_of(VMG_ vm_val_t *retval,
                                   const vm_val_t *self_val,
                                   const char *lst, uint *argc)
{
    vm_val_t subval;
    size_t idx;
    static CVmNativeCodeDesc desc(1);

    /* check arguments */
    if (get_prop_check_argc(retval, argc, &desc))
        return TRUE;

    /* pop the value to find */
    G_stk->pop(&subval);

    /* find the value in the list */
    if (find_last_in_list(vmg_ self_val, &subval, &idx))
    {
        /* found it - adjust to 1-based index for return */
        retval->set_int(idx + 1);
    }
    else
    {
        /* didn't find it - return nil */
        retval->set_nil();
    }

    /* handled */
    return TRUE;
}

/* ------------------------------------------------------------------------ */
/*
 *   property evaluator - lastIndexWhich
 */
int CVmObjList::getp_last_index_which(VMG_ vm_val_t *retval,
                                      const vm_val_t *self_val,
                                      const char *lst, uint *argc)
{
    /* use the generic index-which routine, stepping backward */
    return gen_index_which(vmg_ retval, self_val, lst, argc, FALSE);
}

/* ------------------------------------------------------------------------ */
/*
 *   property evaluator - lastValWhich
 */
int CVmObjList::getp_last_val_which(VMG_ vm_val_t *retval,
                                    const vm_val_t *self_val,
                                    const char *lst, uint *argc)
{
    /* get the index of the value using lastIndexWhich */
    getp_last_index_which(vmg_ retval, self_val, lst, argc);

    /* if the return value is a valid index, get the value at the index */
    if (retval->typ == VM_INT)
    {
        int idx;
        
        /* re-translate the list address in case of swapping */
        if (self_val->typ == VM_LIST)
            lst = self_val->get_as_list(vmg0_);

        /* get the element as the return value */
        idx = (int)retval->val.intval;
        index_list(vmg_ retval, lst, idx);
    }

    /* handled */
    return TRUE;
}

/* ------------------------------------------------------------------------ */
/*
 *   property evaluator - countOf
 */
int CVmObjList::getp_count_of(VMG_ vm_val_t *retval,
                              const vm_val_t *self_val,
                              const char *lst, uint *argc)
{
    vm_val_t *val;
    size_t idx;
    size_t cnt;
    size_t val_cnt;
    static CVmNativeCodeDesc desc(1);

    /* check arguments */
    if (get_prop_check_argc(retval, argc, &desc))
        return TRUE;

    /* get the value to find, but leave it on the stack for gc protection */
    val = G_stk->get(0);

    /* lave the self value on the stack for gc protection */
    G_stk->push(self_val);

    /* get the number of elements in the list */
    cnt = vmb_get_len(lst);

    /* scan the list and count the elements */
    for (idx = 0, val_cnt = 0 ; idx < cnt ; ++idx)
    {
        vm_val_t ele;
        
        /* re-translate the list address in case of swapping */
        if (self_val->typ == VM_LIST)
            lst = self_val->get_as_list(vmg0_);

        /* get this list element */
        vmb_get_dh(get_element_ptr_const(lst, idx), &ele);

        /* if it's the one we're looking for, count it */
        if (ele.equals(vmg_ val))
            ++val_cnt;
    }

    /* discard our gc protection */
    G_stk->discard(2);

    /* return the count */
    retval->set_int(val_cnt);

    /* handled */
    return TRUE;
}

/* ------------------------------------------------------------------------ */
/*
 *   property evaluator - countWhich
 */
int CVmObjList::getp_count_which(VMG_ vm_val_t *retval,
                                 const vm_val_t *self_val,
                                 const char *lst, uint *argc)
{
    const vm_val_t *func_val;
    size_t cnt;
    size_t idx;
    int val_cnt;
    static CVmNativeCodeDesc desc(1);

    /* check arguments */
    if (get_prop_check_argc(retval, argc, &desc))
        return TRUE;

    /* get the function pointer argument, but leave it on the stack */
    func_val = G_stk->get(0);

    /* push a self-reference while allocating to protect from gc */
    G_stk->push(self_val);

    /* get the length of the list */
    cnt = vmb_get_len(lst);

    /* no return value */
    retval->set_nil();

    /* invoke the callback on each element */
    for (idx = 1, val_cnt = 0 ; idx <= cnt ; ++idx)
    {
        vm_val_t *val;

        /* re-translate the list address in case of swapping */
        if (self_val->typ == VM_LIST)
            lst = self_val->get_as_list(vmg0_);

        /* 
         *   get this element (using a 1-based index), and push it as the
         *   callback's argument 
         */
        index_and_push(vmg_ lst, idx);

        /* invoke the callback */
        G_interpreter->call_func_ptr(vmg_ func_val, 1, "list.forEach", 0);

        /* get the result from R0 */
        val = G_interpreter->get_r0();

        /* if the callback returned non-nil and non-zero, count it */
        if (val->typ == VM_NIL
            || (val->typ == VM_INT && val->val.intval == 0))
        {
            /* it's nil or zero - don't include it in the result */
        }
        else
        {
            /* count it */
            ++val_cnt;
        }
    }

    /* discard our gc protection (self) and our arguments */
    G_stk->discard(2);

    /* return the count */
    retval->set_int(val_cnt);

    /* handled */
    return TRUE;
}

/* ------------------------------------------------------------------------ */
/*
 *   property evaluator - getUnique
 */
int CVmObjList::getp_get_unique(VMG_ vm_val_t *retval,
                                const vm_val_t *self_val,
                                const char *lst, uint *argc)
{
    CVmObjList *new_lst_obj;
    static CVmNativeCodeDesc desc(0);

    /* check arguments */
    if (get_prop_check_argc(retval, argc, &desc))
        return TRUE;

    /* put myself on the stack for GC protection */
    G_stk->push(self_val);

    /* 
     *   Make a copy of our list for the return value, since the result
     *   value will never be larger than the original list.  By making a
     *   copy of the input list, we also can avoid worrying about whether
     *   the input list was a constant, and hence we don't have to worry
     *   about the possibility of constant page swapping - we'll just
     *   update elements of the copy in-place.  
     */
    retval->set_obj(create(vmg_ FALSE, lst));

    /* get the return value list data */
    new_lst_obj = (CVmObjList *)vm_objp(vmg_ retval->val.obj);

    /* push a reference to the new list for gc protection */
    G_stk->push(retval);

    /* uniquify the list */
    new_lst_obj->cons_uniquify(vmg0_);

    /* discard the gc protection */
    G_stk->discard(2);

    /* handled */
    return TRUE;
}

/* ------------------------------------------------------------------------ */
/*
 *   property evaluator - appendUnique 
 */
int CVmObjList::getp_append_unique(VMG_ vm_val_t *retval,
                                   const vm_val_t *self_val,
                                   const char *lst, uint *argc)
{
    vm_val_t val2;
    const char *lst2;
    size_t lst_len;
    size_t lst2_len;
    CVmObjList *new_lst;
    static CVmNativeCodeDesc desc(1);

    /* check arguments */
    if (get_prop_check_argc(retval, argc, &desc))
        return TRUE;

    /* remember the length of my list */
    lst_len = vmb_get_len(lst);

    /* get the second list, but leave it on the stack for GC protection */
    val2 = *G_stk->get(0);

    /* put myself on the stack for GC protection as well */
    G_stk->push(self_val);

    /* make sure the other value is indeed a list */
    if ((lst2 = val2.get_as_list(vmg0_)) == 0)
        err_throw(VMERR_LIST_VAL_REQD);

    /* get the length of the second list */
    lst2_len = vmb_get_len(lst2);

    /*
     *   Create a new list for the return value.  Allocate space for the
     *   current list plus the list to be added - this is an upper bound,
     *   since the actual result list can be shorter 
     */
    retval->set_obj(create(vmg_ FALSE, lst_len + lst2_len));

    /* push a reference to the new list for gc protection */
    G_stk->push(retval);

    /* get the return value list data */
    new_lst = (CVmObjList *)vm_objp(vmg_ retval->val.obj);

    /* 
     *   copy the first list into the result list (including only the data
     *   elements, not the length prefix) 
     */
    lst = self_val->get_as_list(vmg0_);
    new_lst->cons_copy_elements(0, lst);

    /* append the second list to the result list */
    lst2 = val2.get_as_list(vmg0_);
    new_lst->cons_copy_elements(lst_len, lst2);

    /* make the list unique */
    new_lst->cons_uniquify(vmg0_);

    /* discard the gc protection and arguments */
    G_stk->discard(3);

    /* handled */
    return TRUE;
}


/* ------------------------------------------------------------------------ */
/*
 *   General insertion routine - this is used to handle append, prepend, and
 *   insertAt property evaluators.  Inserts elements from the argument list
 *   at the given index, with zero indicating insertion before the first
 *   existing element.  
 */
void CVmObjList::insert_elements(VMG_ vm_val_t *retval,
                                 const vm_val_t *self_val,
                                 const char *lst, uint argc, int idx)
{
    size_t lst_len;
    CVmObjList *new_lst;
    uint i;
    const int stack_temp_cnt = 2;

    /* remember the length of my list */
    lst_len = vmb_get_len(lst);

    /* the index must be in the range 0 to the number of elements */
    if (idx < 0 || (size_t)idx > lst_len)
        err_throw(VMERR_INDEX_OUT_OF_RANGE);

    /* put myself on the stack for GC protection as well */
    G_stk->push(self_val);

    /*
     *   Create a new list for the return value.  Allocate space for the
     *   current list plus one new element for each argument.  
     */
    retval->set_obj(create(vmg_ FALSE, lst_len + argc));

    /* push a reference to the new list for gc protection */
    G_stk->push(retval);

    /* get the return value list data */
    new_lst = (CVmObjList *)vm_objp(vmg_ retval->val.obj);

    /* get the original list data */
    lst = self_val->get_as_list(vmg0_);

    /* 
     *   Copy the first list into the result list (including only the data
     *   elements, not the length prefix).  Copy it in two pieces: first,
     *   copy the elements before the insertion point.  
     */
    if (idx != 0)
        new_lst->cons_copy_data(0, get_element_ptr_const(lst, 0), idx);

    /* second, copy the elements after the insertion point */
    if ((size_t)idx != lst_len)
        new_lst->cons_copy_data(idx + argc, get_element_ptr_const(lst, idx),
                                lst_len - idx);

    /* copy each argument into the proper position in the new list */
    for (i = 0 ; i < argc ; ++i)
    {
        const vm_val_t *argp;

        /* 
         *   get a pointer to this argument value - the arguments are just
         *   after the temporary items we've pushed onto the stack 
         */
        argp = G_stk->get(stack_temp_cnt + i);
        
        /* copy the argument into the list */
        new_lst->cons_set_element((uint)idx + i, argp);
    }

    /* discard the gc protection and arguments */
    G_stk->discard(argc + stack_temp_cnt);
}


/* ------------------------------------------------------------------------ */
/*
 *   property evaluator - append 
 */
int CVmObjList::getp_append(VMG_ vm_val_t *retval,
                            const vm_val_t *self_val,
                            const char *lst, uint *argc)
{
    static CVmNativeCodeDesc desc(1);

    /* check arguments */
    if (get_prop_check_argc(retval, argc, &desc))
        return TRUE;

    /* insert the element (there's just one) at the end of the list */
    insert_elements(vmg_ retval, self_val, lst, 1, vmb_get_len(lst));

    /* handled */
    return TRUE;
}

/* ------------------------------------------------------------------------ */
/*
 *   property evaluator - prepend 
 */
int CVmObjList::getp_prepend(VMG_ vm_val_t *retval,
                             const vm_val_t *self_val,
                             const char *lst, uint *argc)
{
    static CVmNativeCodeDesc desc(1);

    /* check arguments */
    if (get_prop_check_argc(retval, argc, &desc))
        return TRUE;

    /* insert the element (there's just one) at the start of the list */
    insert_elements(vmg_ retval, self_val, lst, 1, 0);

    /* handled */
    return TRUE;
}

/* ------------------------------------------------------------------------ */
/*
 *   property evaluator - insert new elements
 */
int CVmObjList::getp_insert_at(VMG_ vm_val_t *retval,
                               const vm_val_t *self_val,
                               const char *lst, uint *in_argc)
{
    int idx;
    uint argc = (in_argc != 0 ? *in_argc : 0);
    static CVmNativeCodeDesc desc(2, 0, TRUE);
    
    /* check arguments - we need at least two */
    if (get_prop_check_argc(retval, in_argc, &desc))
        return TRUE;

    /* 
     *   Pop the index value - the remaining arguments are the new element
     *   values to be inserted, so leave them on the stack.  Note that we
     *   must adjust the value from the 1-based indexing of our caller to
     *   the zero-based indexing we use internally.  
     */
    idx = CVmBif::pop_int_val(vmg0_) - 1;

    /* 
     *   Insert the element (there's just one) at the start of the list.
     *   Note that we must decrement the argument count we got, since we
     *   already took off the first argument (the index value). 
     */
    insert_elements(vmg_ retval, self_val, lst, argc - 1, idx);

    /* handled */
    return TRUE;
}

/* ------------------------------------------------------------------------ */
/*
 *   General property evaluator for removing a range of elements - this
 *   handles removeElementAt and removeRange. 
 */
void CVmObjList::remove_range(VMG_ vm_val_t *retval,
                              const vm_val_t *self_val,
                              const char *lst, int start_idx, int del_cnt)
{
    size_t lst_len;
    CVmObjList *new_lst;

    /* push myself onto the stack for GC protection */
    G_stk->push(self_val);

    /* get the original list length */
    lst_len = vmb_get_len(lst);
    
    /* 
     *   allocate a new list with space for the original list minus the
     *   elements to be deleted 
     */
    retval->set_obj(create(vmg_ FALSE, lst_len - del_cnt));

    /* push a reference to teh new list for gc protection */
    G_stk->push(retval);

    /* get the return value list data */
    new_lst = (CVmObjList *)vm_objp(vmg_ retval->val.obj);

    /* get the original list data as well */
    lst = self_val->get_as_list(vmg0_);

    /* 
     *   copy elements from the original list up to the first item to be
     *   removed 
     */
    if (start_idx != 0)
        new_lst->cons_copy_data(0, get_element_ptr_const(lst, 0), start_idx);

    /* 
     *   copy elements of the original list following the last item to be
     *   removed 
     */
    if ((size_t)(start_idx + del_cnt) < lst_len)
        new_lst->
            cons_copy_data(start_idx,
                           get_element_ptr_const(lst, start_idx + del_cnt),
                           lst_len - (start_idx + del_cnt));

    /* discard the gc protection */
    G_stk->discard(2);
}

/* ------------------------------------------------------------------------ */
/*
 *   property evaluator - remove the element at the given index
 */
int CVmObjList::getp_remove_element_at(VMG_ vm_val_t *retval,
                                       const vm_val_t *self_val,
                                       const char *lst, uint *argc)
{
    int idx;
    static CVmNativeCodeDesc desc(1);

    /* check arguments */
    if (get_prop_check_argc(retval, argc, &desc))
        return TRUE;

    /* retrieve the index value, and adjust to zero-based indexing */
    idx = CVmBif::pop_int_val(vmg0_) - 1;

    /* make sure it's in range - it must refer to a valid element */
    if (idx < 0 || (size_t)idx >= vmb_get_len(lst))
        err_throw(VMERR_INDEX_OUT_OF_RANGE);

    /* remove one element at the given index */
    remove_range(vmg_ retval, self_val, lst, idx, 1);

    /* handled */
    return TRUE;
}

/* ------------------------------------------------------------------------ */
/*
 *   property evaluator - remove the element at the given index 
 */
int CVmObjList::getp_remove_range(VMG_ vm_val_t *retval,
                                  const vm_val_t *self_val,
                                  const char *lst, uint *argc)
{
    int start_idx;
    int end_idx;
    static CVmNativeCodeDesc desc(2);

    /* check arguments */
    if (get_prop_check_argc(retval, argc, &desc))
        return TRUE;

    /* 
     *   retrieve the starting and ending index values, and adjust to
     *   zero-based indexing 
     */
    start_idx = CVmBif::pop_int_val(vmg0_) - 1;
    end_idx = CVmBif::pop_int_val(vmg0_) - 1;

    /* 
     *   make sure the index values are in range - both must refer to valid
     *   elements, and the ending index must be at least as high as the
     *   starting index 
     */
    if (start_idx < 0 || (size_t)start_idx >= vmb_get_len(lst)
          || end_idx < 0 || (size_t)end_idx >= vmb_get_len(lst)
          || end_idx < start_idx)
        err_throw(VMERR_INDEX_OUT_OF_RANGE);

    /* remove the specified elements */
    remove_range(vmg_ retval, self_val, lst, start_idx,
                 end_idx - start_idx + 1);

    /* handled */
    return TRUE;
}

/* ------------------------------------------------------------------------ */
/*
 *   sorter for list data 
 */
class CVmQSortList: public CVmQSortVal
{
public:
    CVmQSortList()
    {
        lst_ = 0;
    }
    
    /* get an element */
    void get_ele(VMG_ size_t idx, vm_val_t *val)
    {
        vmb_get_dh(get_ele_ptr(idx), val);
    }

    /* set an element */
    void set_ele(VMG_ size_t idx, const vm_val_t *val)
    {
        vmb_put_dh(get_ele_ptr(idx), val);
    }

    /* get an element pointer */
    char *get_ele_ptr(size_t idx)
        { return lst_ + VMB_LEN + (idx * VMB_DATAHOLDER); }

    /* our list data */
    char *lst_;
};

/*
 *   property evaluator - sort
 */
int CVmObjList::getp_sort(VMG_ vm_val_t *retval,
                          const vm_val_t *self_val,
                          const char *lst, uint *in_argc)
{
    size_t lst_len;
    CVmObjList *new_lst;
    uint argc = (in_argc == 0 ? 0 : *in_argc);
    CVmQSortList sorter;    
    static CVmNativeCodeDesc desc(0, 2);

    /* check arguments */
    if (get_prop_check_argc(retval, in_argc, &desc))
        return TRUE;

    /* remember the length of my list */
    lst_len = vmb_get_len(lst);

    /* if we have an 'descending' flag, note it */
    if (argc >= 1)
        sorter.descending_ = (G_stk->get(0)->typ != VM_NIL);

    /* 
     *   if we have a comparison function, note it, but leave it on the
     *   stack for gc protection 
     */
    if (argc >= 2)
        sorter.compare_fn_ = *G_stk->get(1);

    /* put myself on the stack for GC protection as well */
    G_stk->push(self_val);

    /* create a copy of the list as the return value */
    retval->set_obj(create(vmg_ FALSE, lst));

    /* push a reference to the new list for gc protection */
    G_stk->push(retval);

    /* get the return value list data */
    new_lst = (CVmObjList *)vm_objp(vmg_ retval->val.obj);

    /* set the list pointer in the sorter */
    sorter.lst_ = new_lst->ext_;

    /* sort the new list if it has any elements */
    if (lst_len != 0)
        sorter.sort(vmg_ 0, lst_len - 1);

    /* discard the gc protection and arguments */
    G_stk->discard(2 + argc);

    /* handled */
    return TRUE;
}


/* ------------------------------------------------------------------------ */
/*
 *   Constant-pool list object 
 */

/*
 *   create 
 */
vm_obj_id_t CVmObjListConst::create(VMG_ const char *const_ptr)
{
    /* create our new ID */
    vm_obj_id_t id = vm_new_id(vmg_ FALSE, FALSE, FALSE);

    /* create our list object, pointing directly to the constant pool */
    new (vmg_ id) CVmObjListConst(vmg_ const_ptr);

    /* return the new ID */
    return id;
}
