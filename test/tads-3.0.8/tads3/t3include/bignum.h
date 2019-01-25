#charset "us-ascii"

/* 
 *   Copyright (c) 2000, 2002 Michael J. Roberts
 *   
 *   This file is part of TADS 3.  
 */

#ifndef _BIGNUM_H_
#define _BIGNUM_H_

/* include our base class definition */
#include "systype.h"

/*
 *   'bignumber' metaclass 
 */
intrinsic class BigNumber 'bignumber/030000': Object
{
    /* format to a string */
    formatString(maxDigits, flags?,
                 wholePlaces?, fracDigits?, expDigits?, leadFiller?);

    /* 
     *   compare for equality after rounding to the smaller of my
     *   precision and num's precision 
     */
    equalRound(num);

    /* 
     *   returns an integer giving the number of digits of precision that
     *   this number stores 
     */
    getPrecision();

    /* 
     *   Return a new number, with the same value as this number but with
     *   the given number of decimal digits of precision.  If the new
     *   precision is higher than the old precision, this will increase
     *   the precision to the requested new size and add trailing zeroes
     *   to the value.  If the new precision is lower than the old
     *   precision, we'll round the number for the reduced precision.  
     */
    setPrecision(digits);

    /* get the fractional part */
    getFraction();

    /* get the whole part (truncates the fraction - doesn't round) */
    getWhole();

    /* 
     *   round to the given number of digits after the decimal point; if
     *   the value is zero, round to integer; if the value is negative,
     *   round to the given number of places before the decimal point 
     */
    roundToDecimal(places);

    /* return the absolute value */
    getAbs();

    /* least integer greater than or equal to this number */
    getCeil();

    /* greatest integer less than or equal to this number */
    getFloor();

    /* get the base-10 scale of the number */
    getScale();

    /* 
     *   scale by 10^x - if x is positive, this multiplies the number by
     *   ten the given number of times; if x is negative, this divides the
     *   number by ten the given number of times 
     */
    scaleTen(x);

    /* negate - invert the sign of the number */
    negate();

    /* 
     *   copySignFrom - combine the absolute value of self with the sign
     *   of x 
     */
    copySignFrom(x);

    /* determine if the value is negative */
    isNegative();

    /* 
     *   Calculate the integer quotient and the remainder; returns a list
     *   whose first element is the integer quotient (a BigNumber
     *   containing an integer value), and whose second element is the
     *   remainder (the value R such that dividend = quotient*x + R).
     *   
     *   Note that the quotient returned will not necessarily have the
     *   same value as the whole part of dividing self by x with the '/'
     *   operator, because this division handles rounding differently.  In
     *   particular, the '/' operator will perform the appropriate
     *   rounding on the quotient if the quotient has insufficient
     *   precision to represent the exact result.  This routine, in
     *   contrast, does NOT round the quotient, but merely truncates any
     *   trailing digits that cannot be represented in the result's
     *   precision.  The reason for this difference is that it ensures
     *   that the relation (dividend=quotient*x+remainder) holds, which
     *   would not always be the case if the quotient were rounded up.
     *   
     *   Note also that the remainder will not necessarily be less than
     *   the divisor.  If the quotient cannot be exactly represented
     *   (which occurs if the precision of the quotient is smaller than
     *   its scale), the remainder will be the correct value so that the
     *   relationship above holds, rather than the unique remainder that
     *   is smaller than the divisor.  In all cases where there is
     *   sufficient precision to represent the quotient exactly (to the
     *   units digit only, since the quotient returned from this method
     *   will always be an integer), the remainder will satisfy the
     *   relationship AND will be the unique remainder with absolute value
     *   less than the divisor.  
     */
    divideBy(x);

    /* 
     *   calculate and return the trigonometric sine of the value (taken
     *   as a radian value) 
     */
    sine();

    /* 
     *   calculate and return the trigonometric cosine of the value (taken
     *   as a radian value) 
     */
    cosine();

    /* 
     *   calculate and return the trigonometric tangent of the value
     *   (taken as a radian value) 
     */
    tangent();

    /* 
     *   interpreting this number as a number of degrees, convert the
     *   value to radians and return the result 
     */
    degreesToRadians();

    /* 
     *   interpreting this number as a number of radians, convert the
     *   value to degrees and return the result 
     */
    radiansToDegrees();

    /* 
     *   Calculate and return the arcsine (in radians) of the value.  Note
     *   that the value must be between -1 and +1 inclusive, since sine()
     *   never has a value outside of this range. 
     */
    arcsine();

    /* 
     *   Calculate and return the arccosine (in radians).  The value must
     *   be between -1 and +1 inclusive. 
     */
    arccosine();

    /* calculate and return the arctangent (in radians) */
    arctangent();

    /* calculate the square root and return the result */
    sqrt();

    /* 
     *   calculate the natural logarithm of this number and return the
     *   result 
     */
    logE();

    /* 
     *   raise e (the base of the natural logarithm) to the power of this
     *   value and return the result 
     */
    expE();

    /* calculate the base-10 logarithm of the number and return the result */
    log10();

    /* 
     *   raise this number to the power of the argument and return the
     *   result 
     */
    raiseToPower(x);

    /* calculate the hyperbolic sine, cosine, and tangent */
    sinh();
    cosh();
    tanh();

    /* get the value of pi to a given precision */
    getPi(digits);

    /* get the value of e to a given precision */
    getE(digits);
}

/*
 *   flags for formatString 
 */

/* always show a sign, even if positive */
#define BignumSign          0x0001

/* always show in exponential format */
#define BignumExp           0x0002

/* always show a sign in the exponent, even if positive */
#define BignumExpSign      0x0004

/* 
 *   show a zero before the decimal point - this is only relevant in
 *   non-exponential format when the number is between -1 and +1 
 */
#define BignumLeadingZero  0x0008

/* always show a decimal point */
#define BignumPoint         0x0010

/* insert commas to denote thousands, millions, etc */
#define BignumCommas        0x0020

/* show a leading space if the number is positive */
#define BignumPosSpace     0x0040

/* 
 *   use European-style formatting: use a comma instead of a period for
 *   the decimal point, and use periods instead of commas to set off
 *   thousands, millions, etc 
 */
#define BignumEuroStyle     0x0080

#endif /* _BIGNUM_H_ */

