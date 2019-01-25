/* $Header$ */

/* 
 *   Copyright (c) 2000, 2002 Michael J. Roberts.  All Rights Reserved.
 *   
 *   Please see the accompanying license file, LICENSE.TXT, for information
 *   on using and copying this software.  
 */
/*
Name
  tcprstyp.h - TADS 3 compiler - parser type definitions
Function
  Defines some types for the TADS 3 compiler's parser.  Separated from
  the main parser include file to reduce the amount of the parser that
  has to be included simply for the type definitions.
Notes
  
Modified
  04/09/00 MJRoberts  - Creation
*/

#ifndef TCPRSTYP_H
#define TCPRSTYP_H


/* ------------------------------------------------------------------------ */
/*
 *   Expression evaluation constant value types.  As we evaluate an
 *   expression, we'll attempt to evaluate the constant elements of the
 *   expression, so that the code we generate has any constant expressions
 *   computed at compile-time rather than executed at run-time.
 */
enum tc_constval_type_t
{
    TC_CVT_UNK,                      /* unknown type or non-constant value  */
    TC_CVT_NIL,                                                      /* nil */
    TC_CVT_TRUE,                                                    /* true */
    TC_CVT_INT,                                            /* integer value */
    TC_CVT_SSTR,                              /* single-quoted string value */
    TC_CVT_LIST,                                           /* list constant */
    TC_CVT_OBJ,                                         /* object reference */
    TC_CVT_PROP,                                        /* property pointer */
    TC_CVT_FUNCPTR,                                     /* function pointer */
    TC_CVT_VOCAB_LIST,                       /* vocabulary list placeholder */
    TC_CVT_ANONFUNCPTR,                       /* anonymous function pointer */
    TC_CVT_ENUM,                                              /* enumerator */
    TC_CVT_FLOAT                                   /* floating point number */
};

/* ------------------------------------------------------------------------ */
/*
 *   Symbol types.  These values are stored in symbol export files, so the
 *   order of these entries must not be changed.  If new entries are to be
 *   added, they must be added at the end of the list, and existing
 *   entries must not be deleted (instead, make an existing entry
 *   obsolete, but leave its slot occupied).  
 */
enum tc_symtype_t
{
    /* unknown */
    TC_SYM_UNKNOWN = 0,

    /* function */
    TC_SYM_FUNC,

    /* object */
    TC_SYM_OBJ,

    /* property */
    TC_SYM_PROP,

    /* local variable */
    TC_SYM_LOCAL,

    /* parameter */
    TC_SYM_PARAM,

    /* built-in function */
    TC_SYM_BIF,

    /* external function */
    TC_SYM_EXTFN,

    /* code label */
    TC_SYM_LABEL,

    /* metaclass */
    TC_SYM_METACLASS,

    /* enumerator */
    TC_SYM_ENUM,

    /* 'grammar token' */
    TC_SYM_GRAMTOK
};


#endif /* TCPRSTYP_H */
