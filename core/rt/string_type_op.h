#pragma once

#include "../os/predefines.h"

#pragma warning(disable:4146)

// code adapted based on modp_numtoa.h/.c

/**
 * \file
 *
 * <pre>
 * Copyright &copy; 2007, Nick Galbreath -- nickg [at] modp [dot] com
 * All rights reserved.
 * http://code.google.com/p/stringencoders/
 * Released under the bsd license.
 * </pre>
 *
 * This defines signed/unsigned integer, and 'double' to char buffer
 * converters.  The standard way of doing this is with "sprintf", however
 * these functions are
 *   * guarenteed maximum size output
 *   * 5-20x faster!
 *   * core-dump safe
 *
 *
 */

namespace rt
{

struct string_ops
{

FORCEINL static void strreverse(char* begin, char* end)
{
    char aux;
    while(end > begin)
        aux = *end, *end-- = *begin, *begin++ = aux;
}

INLFUNC static int itoa(INT value, char* str)
{
    char* wstr=str;
    // Take care of sign
    unsigned int uvalue = (value < 0) ? -value : value;
    // Conversion. Number is reversed.
    do *wstr++ = (char)(48 + (uvalue % 10)); while(uvalue /= 10);
    if (value < 0) *wstr++ = '-';
    *wstr='\0';

    // Reverse string
    strreverse(str,wstr-1);
	return (int)(wstr - str);
}

INLFUNC static int itoa(UINT value, char* str)
{
    char* wstr=str;
    // Conversion. Number is reversed.
    do *wstr++ = (char)(48 + (value % 10)); while (value /= 10);
    *wstr='\0';
    // Reverse string
    strreverse(str, wstr-1);
	return (int)(wstr - str);
}

INLFUNC static int itoa(LONGLONG value, char* str)
{
    char* wstr=str;
    ULONGLONG uvalue = (LONGLONG)((value < 0) ? -value : value);

    // Conversion. Number is reversed.
    do *wstr++ = (char)(48 + (uvalue % 10)); while(uvalue /= 10);
    if (value < 0) *wstr++ = '-';
    *wstr='\0';

    // Reverse string
    strreverse(str,wstr-1);
	return (int)(wstr - str);
}

INLFUNC static int itoa(ULONGLONG value, char* str)
{
    char* wstr=str;
    // Conversion. Number is reversed.
    do *wstr++ = (char)(48 + (value % 10)); while (value /= 10);
    *wstr='\0';
    // Reverse string
    strreverse(str, wstr-1);
	return (int)(wstr - str);
}

template<typename T>
INLFUNC static int ftoa(T value, char* str, int prec = 2)
{
	static const T pow10[] = 
	{1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000};
	
	/* Hacky test for NaN
     * under -fast-math this won't work, but then you also won't
     * have correct nan values anyways.  The alternative is
     * to link with libmath (bad) or hack IEEE double bits (bad)
     */
    if (! (value == value)) {
        str[0] = 'n'; str[1] = 'a'; str[2] = 'n'; str[3] = '\0';
        return 3;
    }
    /* if input is larger than thres_max, revert to exponential */
    const T thres_max = (T)(0x7FFFFFFF);

    T diff = 0;
    char* wstr = str;

    if (prec < 0) {
        prec = 0;
    } else if (prec > 9) {
        /* precision of >= 10 can lead to overflow errors */
        prec = 9;
    }


    /* we'll work in positive values and deal with the
       negative sign issue later */
    int neg = 0;
    if (value < 0) {
        neg = 1;
        value = -value;
    }


    int whole = (int) value;
    T tmp = (value - whole) * pow10[prec];
    UINT frac = (UINT)(tmp);
    diff = tmp - frac;

    if (diff > 0.5) {
        ++frac;
        /* handle rollover, e.g.  case 0.99 with prec 1 is 1.0  */
        if (frac >= pow10[prec]) {
            frac = 0;
            ++whole;
        }
    } else if (diff == 0.5 && ((frac == 0) || (frac & 1))) {
        /* if halfway, round up if odd, OR
           if last digit is 0.  That last part is strange */
        ++frac;
    }

    /* for very large numbers switch back to native sprintf for exponentials.
       anyone want to write code to replace this? */
    /*
      normal printf behavior is to print EVERY whole number digit
      which can be 100s of characters overflowing your buffers == bad
    */
    if (value > thres_max) {
		char fmt[5] = "%.2e";
		fmt[2] = prec + '0';
        return sprintf(str, fmt, (double)(neg ? -value : value));
    }

    if (prec == 0) {
        diff = value - whole;
        if (diff > 0.5) {
            /* greater than 0.5, round up, e.g. 1.6 -> 2 */
            ++whole;
        } else if (diff == 0.5 && (whole & 1)) {
            /* exactly 0.5 and ODD, then round up */
            /* 1.5 -> 2, but 2.5 -> 2 */
            ++whole;
        }
    } else {
        int count = prec;
        // now do fractional part, as an unsigned number
        do {
            --count;
            *wstr++ = (char)(48 + (frac % 10));
        } while (frac /= 10);
        // add extra 0s
        while (count-- > 0) *wstr++ = '0';
        // add decimal
        *wstr++ = '.';
    }

    // do whole part
    // Take care of sign
    // Conversion. Number is reversed.
    do *wstr++ = (char)(48 + (whole % 10)); while (whole /= 10);
    if (neg) {
        *wstr++ = '-';
    }
    *wstr='\0';
    strreverse(str, wstr-1);
	return (int)(wstr-str);
}


};

} // namespace rt


#pragma warning(default:4146)