#charset "us-ascii"

/* Copyright 2000, 2002 Michael J. Roberts */
/*
 *   TADS 3 library - number handling 
 */

#include "adv3.h"


/* ------------------------------------------------------------------------ */
/*
 *   Spell out an integer number in words.  Returns a string with the
 *   spelled-out number.  
 */
spellInt(val)
{
    return spellIntExt(val, 0);
}

/*
 *   Spell out an integer number in words, but only if it's below the
 *   given threshold.  It's often awkward in prose to spell out large
 *   numbers, but exactly what constitutes a large number depends on
 *   context, so this routine lets the caller specify the threshold.
 *   
 *   If the absolute value of val is less than (not equal to) the
 *   threshold value, we'll return a string with the number spelled out.
 *   If the absolute value is greater than or equal to the threshold
 *   value, we'll return a string representing the number in decimal
 *   digits.  
 */
spellIntBelow(val, threshold)
{
    return spellIntBelowExt(val, threshold, 0, 0);
}

/*
 *   Spell out an integer number in words if it's below a threshold,
 *   using the spellIntXxx flags given in spellFlags to control the
 *   spelled-out format, and using the DigitFormatXxx flags in digitFlags
 *   to control the digit format.  
 */
spellIntBelowExt(val, threshold, spellFlags, digitFlags)
{
    local absval;

    /* compute the absolute value */
    absval = (val < 0 ? -val : val);

    /* check the value to see whether to spell it or write it as digits */
    if (absval < threshold)
    {
        /* it's below the threshold - spell it out in words */
        return spellIntExt(val, spellFlags);
    }
    else
    {
        /* it's not below the threshold - write it as digits */
        return intToDecimal(val, digitFlags);
    }
}

/*
 *   Format a number as a string of decimal digits.  The DigitFormatXxx
 *   flags specify how the number is to be formatted.` 
 */
intToDecimal(val, flags)
{
    local str;
    local sep;

    /* perform the basic conversion */
    str = toString(val);

    /* add group separators as needed */
    if ((flags & DigitFormatGroupComma) != 0)
    {
        /* explicitly use a comma as a separator */
        sep = ',';
    }
    else if ((flags & DigitFormatGroupPeriod) != 0)
    {
        /* explicitly use a period as a separator */
        sep = '.';
    }
    else if ((flags & DigitFormatGroupSep) != 0)
    {
        /* use the current languageGlobals separator */
        sep = languageGlobals.digitGroupSeparator;
    }
    else
    {
        /* no separator */
        sep = nil;
    }

    /* if there's a separator, add it in */
    if (sep != nil)
    {
        local i;
        local len;
        
        /* 
         *   Insert the separator before each group of three digits.
         *   Start at the right end of the string and work left: peel off
         *   the last three digits and insert a comma.  Then, move back
         *   four characters through the string - another three-digit
         *   group, plus the comma we inserted - and repeat.  Keep going
         *   until the amount we'd want to peel off the end is as long or
         *   longer than the entire remaining string. 
         */
        for (i = 3, len = str.length() ; len > i ; i += 4)
        {
            /* insert this comma */
            str = str.substr(1, len - i) + sep + str.substr(len - i + 1);

            /* note the new length */
            len = str.length();
        }
    }

    /* return the result */
    return str;
}

/*
 *   Format a number as a binary string. 
 */
intToBinary(val)
{
    local result;
    local outpos;

    /* if the value is zero, the binary representation is simply '0' */
    if (val == 0)
        return '0';

    /* allocate a vector to store the characters of the result */
    result = new Vector(33).fillValue(nil, 1, 33);

    /* 
     *   Fill up the vector with 1's and 0's, working from the lowest-order
     *   bit to the highest-order bit.  On each iteration, we'll pull out
     *   the low-order bit, add it to the result string, and then shift the
     *   value right one bit so that the next higher-order bit becomes the
     *   low-order bit in the value.  
     */
    for (outpos = 34 ; val != 0 ; val >>= 1)
    {
        /* 
         *   Add the next bit.  Add it at the end of the string so the
         *   final result reads left-to-right, high-to-low.  Note that
         *   Unicode value 0x30 is the digit '0', and Unicode value 0x31 is
         *   the digit '1'.  We build the result as a vector of Unicode
         *   values for efficiency, so that we don't have to repeatedly
         *   allocate partial strings.  
         */
        result[--outpos] = ((val & 1) == 0 ? 0x30 : 0x31);
    }

    /* convert the vector of Unicode characters to a string */
    return makeString(result.toList(outpos, 33 - outpos));
}

/* ------------------------------------------------------------------------ */
/*
 *   Convert an integer number to Roman numerals.  Returns a string with
 *   the Roman numeral format.  This can only accept numbers in the range
 *   1 to 4999; returns nil for anything outside of this range.  
 */
intToRoman(val)
{
    local str;
    local info =
    [
        /* numeral value / corresponding string */
        1000, 'M',
        900,  'CM',
        500,  'D',
        400,  'CD',
        100,  'C',
        90,   'XC',
        50,   'L',
        40,   'XL',
        10,   'X',
        9,    'IX',
        5,    'V',
        4,    'IV',
        1,    'I'
    ];
    local i;

    /* if the value is outside of the legal range, fail immediately */
    if (val < 1 || val > 4999)
        return nil;

    /* 
     *   iterate over the specifiers and apply each one as many times as
     *   possible 
     */
    for (str = '', i = 1 ; val != 0 ; )
    {
        /* 
         *   If we're greater than the current specifier, apply it;
         *   otherwise, move on to the next specifier. 
         */
        if (val >= info[i])
        {
            /* add this specifier's roman numeral into the result */
            str += info[i+1];

            /* subtract the corresponding value */
            val -= info[i];
        }
        else
        {
            /* move to the next specifier */
            i += 2;
        }
    }

    /* return the result */
    return str;
}

