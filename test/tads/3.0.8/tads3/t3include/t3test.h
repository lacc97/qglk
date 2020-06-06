#charset "us-ascii"

/* 
 *   Copyright (c) 1999, 2002 Michael J. Roberts
 *   
 *   This file is part of TADS 3 
 */

/*
 *   T3 intrinsic function set definition 
 */

#ifndef T3TEST_H
#define T3TEST_H

/* 
 *   define the T3 Test system interface 
 */
intrinsic 't3vmTEST/010000'
{
    t3test_get_obj_id(obj);
    t3test_get_obj_gc_state(id);
    t3test_get_charcode(c);
}

#endif /* T3TEST_H */
