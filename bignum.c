BABYL OPTIONS:
Version:5
Append:1

1, recent,,
Received: from ames.arc.nasa.gov (TCP 20031411003) by AI.AI.MIT.EDU 13 Jan 88 12:37:28 EST
Received: Wed, 13 Jan 88 09:35:55 PST by ames.arc.nasa.gov (5.58/1.2)
Received: from gumby.UUCP (gumby) by mips.UUCP (5.52/4.7)
	id AA17640; Wed, 13 Jan 88 09:36:57 PST
Received: by gumby.UUCP (5.52/4.7)
	id AA16329; Wed, 13 Jan 88 09:36:52 PST
Date: Wed, 13 Jan 88 09:36:52 PST
From: mips!earl@ames.arc.nasa.gov (Earl Killian)
Message-Id: <8801131736.AA16329@gumby.UUCP>
To: JAR@AI.AI.MIT.EDU
In-Reply-To: Jonathan A Rees's message of Tue, 12 Jan 88 19:16:34 EST <310502.880112.JAR@AI.AI.MIT.EDU>
Subject: here is bignum.c

*** EOOH ***
Date: Wed, 13 Jan 88 09:36:52 PST
From: mips!earl at ames.arc.nasa.gov (Earl Killian)
To:   JAR at AI.AI.MIT.EDU
Re:   here is bignum.c

/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */
/* %Q% %M% %I% */

/* bignum -- Multi-word arithmetic package. */

/* Bignums are represented as arrays of ``bigits'' (bignum digits --
   name due to Jon L. White.)  There are abigits (adder bigits), used
   for most operations, and mbigits used for multiply and integer
   divide.  Abigits are words (e.g. 32 bits) and mbigits are halfwords
   (e.g. 16 bits).  Mbigits are half size because double mbigit values
   are required for multiply and divide algorithms.  Abigits are
   stored in little-endian order in the array as words in the
   machine's natural byte sex.  Mbigits are conceptually also stored
   in little-endian order, but on a big-endian machine are actually
   halfword swapped so that they work as abigit arrays.  The M macro
   in bignum.h is used to make make a mbigit reference; it is
   equivalent to the C "*" indirection operator on a little-endian
   machine, and does the flip (via address modification) on a
   big-endian machine.

   Bignums arguments are specified by a pointer and a length.  Sometimes
   the length argument represents the length of several bignum arguments.
   The terse comments preceding the functions give details on the use
   of length arguments.  0 length bignums are not supported (don't try it).
   Lengths are always the number of abigits, even to routines that work
   in terms of mbigits.  The MEND macro is used to do pointer arithmetic
   in terms of abigits on mbigit pointers.

   The loops are written to generate efficient code on the mips
   machine: a pointer to the end of a bignum is usually generated and
   used as an end-test.  Sadly, the reorganizer doesn't always fill
   delay slots the way it should for this stuff.

   I do not claim that this is the right set of primitives; it just
   happens to be the things I needed at the time.
 */

#include <stdio.h>
#include "bignum.h"

/* A:N = 0 */
void
big_zero (n, a)
    register nbigit n;
    register abigit *a;
{
    register abigit *ae = a + n;
    do {
	*a++ = 0;
    } while (a != ae);
}

/* A:N = I */
void
big_int (n, a, i)
    register nbigit n;
    register abigit *a;
    register s_abigit i;
{
    register abigit *ae = a + n;
    *a++ = i;
    i >>= abigit_bits - 1;
    while (a != ae) {
	*a++ = i;
    }
}

/* A:N = B:N */
void
big_copy (n, a, b)
    register nbigit n;
    register abigit *a, *b;
{
    register abigit *ae = a + n;
    do {
	*a++ = *b++;
    } while (a != ae);
}

/* A:N = B:N, reverse copy for handling certain kinds of overlap */
void
big_rcopy (n, a, b)
    register nbigit n;
    register abigit *a, *b;
{
    register abigit *ai = a + n;
    register abigit *bi = b + n;
    do {
	*--ai = *--bi;
    } while (ai != a);
}

/* A:NA = B:NB */
void
big_sextend (na, a, nb, b)
    register nbigit na, nb;
    register abigit *a, *b;
{
    register abigit *ae = a + na;
    register abigit *be = b + nb;
    register abigit bd;
    do {
	bd = *b++;
	*a++ = bd;
    } while (b != be);
    bd = (s_abigit)bd >> (abigit_bits - 1);
    do {
	*a++ = bd;
    } while (a != ae);
}

/* A:NA = B:NB */
void
big_zextend (na, a, nb, b)
    register nbigit na, nb;
    register abigit *a, *b;
{
    register abigit *ae = a + na;
    register abigit *be = b + nb;
    do {
	*a++ = *b++;
    } while (b != be);
    do {
	*a++ = 0;
    } while (a != ae);
}

/* A:N = -B:N */
/* ??Return borrow?? */
void
big_neg (n, a, b)
    register nbigit n;
    register abigit *a, *b;
{
    register abigit *ae = a + n;
    register abigit bd;
    do {
	bd = *b++;
	*a++ = -bd;
	if (a == ae) return;
    } while (bd == 0);
    do {
	*a++ = ~*b++;
    } while (a != ae);
}

/* A:N = abs(B:N) */
/* ??Return borrow?? */
void
big_abs (n, a, b)
    register nbigit n;
    register abigit *a, *b;
{
    register abigit *be = b + n;
    if ((s_abigit)be[-1] >= 0) {
	do {
	    *a++ = *b++;
	} while (b != be);
    }
    else {
	register abigit bd;
	do {
	    bd = *b++;
	    *a++ = -bd;
	    if (b == be) return;
	} while (bd == 0);
	do {
	    *a++ = ~*b++;
	} while (b != be);
    }
}

/* A:N = B:N + C:N, return 0/1 for carry out */
abigit
big_add3 (n, a, b, c)
    register nbigit n;
    register abigit *a, *b, *c;
{
    register abigit *ae, bd, cd;
    ae = a + n;

    /* Sometimes gotos are clearer than contorted whiles... */
loop1:
    /* Add next two bigits with no carry in */
    /* A carry generated if B + C > 2**N-1, i.e. if C > 2**N-1 - B,
       i.e. if ~B < C.  It is important that 2**N-1 - B (i.e. ~B) not
       overflow, which it cannot.  The comparison must be unsigned. */
    if (a == ae) goto ret0;
    bd = *b++;
    cd = *c++;
    *a++ = bd + cd;
    if (!((~bd) < cd)) goto loop1;	/* if no carry out, stay in loop1 */
    goto loop2;				/* if carry out, goto loop2 */

loop2:
    /* Add next two bigits with carry in */
    /* A carry generated if B + C + 1 > 2**N-1, i.e. if C >= 2**N-1 - B,
       i.e. if ~B <= C.  It is important that 2**N-1 - B (i.e. ~B) not
       overflow, which it cannot.  The comparison must be unsigned. */
    if (a == ae) goto ret1;
    bd = *b++;
    cd = *c++;
    *a++ = bd + cd + 1;
    if (!((~bd) <= cd)) goto loop1;	/* if no carry out, goto loop1 */
    goto loop2;				/* if carry out, stay in loop2 */

ret0:
    return 0;
ret1:
    return 1;
}

/* A:N = B:N - C:N, return 0/1 for borrow out */
abigit
big_sub3 (n, a, b, c)
    register nbigit n;
    register abigit *a, *b, *c;
{
    register abigit *ae, bd, cd;
    ae = a + n;

loop1:
    /* Subtract next two bigits with no borrow in */
    /* A borrow is generated from B - C if B < C. */
    if (a == ae) goto ret0;
    bd = *b++;
    cd = *c++;
    *a++ = bd - cd;
    if (!(bd < cd)) goto loop1;		/* if no borrow out, stay in loop1 */
    goto loop2;				/* if borrow out, goto loop2 */

loop2:
    /* Subtract next two bigits with borrow in */
    /* A borrow is generated from B - C - 1 if B <= C. */
    if (a == ae) goto ret1;
    bd = *b++;
    cd = *c++;
    *a++ = bd - cd - 1;
    if (!(bd <= cd)) goto loop1;	/* if no borrow out, goto loop1 */
    goto loop2;				/* if borrow out, stay in loop2 */

ret0:
    return 0;
ret1:
    return 1;
}

/* A:N = A:N + I */
void
big_add2_i (n, a, i)
    register nbigit n;
    register abigit *a, i;
{
    register abigit *ae, ad;
    ae = a + n;
    ad = *a;
    *a++ = ad + i;
    if ((~ad) < i && a != ae) {
	do {
	    ad = *a + 1;
	    *a++ = ad;
	} while (ad == 0 && a != ae);
    }
}

/* A:N = A:N + 1 */
/* ??Return carry?? */
void
big_inc (n, a)
    register nbigit n;
    register abigit *a;
{
    register abigit *ae, ad;
    ae = a + n;
    do {
	ad = *a + 1;
	*a++ = ad;
    } while (ad == 0 && a != ae);
}

/* A:N = A:N - 1 */
/* ??Return borrow?? */
void
big_dec (n, a)
    register nbigit n;
    register abigit *a;
{
    register abigit *ae, ad;
    ae = a + n;
    do {
	ad = *a;
	*a++ = ad - 1;
    } while (ad == 0 && a != ae);
}

/* A:N = lshift(B:N, L), 1 <= L <= 31 */
/* A and B can be identical, but other overlap may not work. */
/* ??Return shifted out bits?? */
/* Probably there should be another routine that supports larger
   shift counts. */
void
big_lshift (n, a, b, l)
    register nbigit n;
    register abigit *a, *b;
    register unsigned l;
{
    register abigit *ai = a + n;
    register abigit *bi = b + n;
    register unsigned r = abigit_bits - l;
    while (--bi, bi != b) {
	*--ai = (bi[0] << l) | (bi[-1] >> r);
    }
    *--ai = bi[0] << l;
}

/* A:N = rshift(B:N, R), 1 <= R <= 31 */
/* A and B can be identical, but other overlap may not work. */
/* ??Return shifted out bits?? */
/* Probably there should be another routine that supports larger
   shift counts. */
void
big_rshift (n, a, b, r)
    register nbigit n;
    register abigit *a, *b;
    register unsigned r;
{
    register abigit *ae = a + n - 1;
    register unsigned l = abigit_bits - r;
    while (a != ae) {
	*a++ = (b[0] >> r) | (b[1] << l);
	b += 1;
    }
    *a = *b >> r;
}

/* A:N = A:N * M, return mbigit carry out */
/* This could be 2x faster if I had access to the full 64-bit result
   generated by the multiply instruction. */
mbigit
big_umul2_i (n, a, m)
    register nbigit n;
    register mbigit *a, m;
{
    register mbigit *ae = MEND(n, a);
    register m2bigit x;
    x = 0;
    do {
	x += M(a) * m;
	M(a++) = x;
	x >>= mbigit_bits;
    } while (a != ae);
    return x;			/* if non-zero then overflow */
}

/* A:N+1 = B:N * M */
/* This could be 2x faster if I had access to the full 64-bit result
   generated by the multiply instruction. */
void
big_umul3_i (n, a, b, m)
    register nbigit n;
    register mbigit *a, *b, m;
{
    register mbigit *ae = MEND(n, a);
    register m2bigit x;
    x = 0;
    do {
	x += M(b++) * m;
	M(a++) = x;
	x >>= mbigit_bits;
    } while (a != ae);
    M(a) = x;
}

/* A:NB+NC = A:NB + B:NB * C:NC */
void
big_smuladd (a, nb, b, nc, c)
    register nbigit nb, nc;
    register abigit *a, *b, *c;
{
    big_umuladd (a, nb, b, nc, c);
    if ((s_abigit)b[nb-1] < 0) {
	big_sub3 (nc, a+nb, a+nb, c);
    }
    if ((s_abigit)c[nc-1] < 0) {
	big_sub3 (nb, a+nc, a+nc, b);
    }
}

/* A:NB+NC = A:NB + B:NB * C:NC */
/* This could be 4x faster if I had access to the full 64-bit result
   generated by the multiply instruction. */
void
big_umuladd (a, nb, b, nc, c)
    register nbigit nb, nc;
    register mbigit *a, *b, *c;
{
    register mbigit *ae, *be, bd, *ce, cd;
    ae = MEND(nb + nc, a);
    be = MEND(nb, b);
    ce = MEND(nc, c);
    do {
	register mbigit cd = M(c++);
	if (cd != 0) {
	    register mbigit *bi = b;
	    register mbigit *ai = a;
	    register m2bigit x = 0;
	    do {
		x += M(bi++) * cd + M(ai);
		M(ai++) = x;
		x >>= mbigit_bits;
	    } while (bi != be);
	    M(ai++) = x;
	}
	a++;
    } while (c != ce);
}

/* A:N = B:N / M, return remainder */
/* If there were a 64/32 bit divide instruction, this would be 2x faster. */
mbigit
big_udiv_i (n, a, b, m)
    register nbigit n;
    register mbigit *a, *b, m;
{
    register mbigit *ai, *bi;
    register m2bigit x;
    ai = MEND(n, a);
    bi = MEND(n, b);
    x = 0;
    do {
	x = (x << mbigit_bits) + M(--bi);
	M(--ai) = x / m;
	x %= m;
    } while (ai != a);
    return x;
}

/* Convert ascii number in radix to bignum. */
void
big_atoi (n, a, s, radix)
    register nbigit n;
    register abigit *a;
    register char *s;
    register unsigned radix;
{
    register unsigned d;
    register unsigned sign;
    big_zero (n, a);

    sign = *s;
    if (sign == '-' || sign == '+') {
	s++;
    }
    while (d = *s++, d != 0) {
	d -= '0';
	if (d > 9) {
	    d -= 'a' - '0';
	    if (d > 25) {
		d -= 'A' - 'a';
		if (d > 25) goto bad_digit;
	    }
	    d += 10;
	}
	if (d >= radix) goto bad_digit;
	big_umul2_i (n, a, radix);
	big_add2_i (n, a, d);
    }
    if (sign == '-') {
	big_neg (n, a, a);
    }
    return;

bad_digit:
    fprintf(stderr, "Illegal digit '%c'.\n", s[-1]);
    exit(1);
}

/* Convert signed bignum A:N to ascii in radix.  Returns pointer into
   caller-supplied buffer. */
char *
big_itoa (buffer, buffer_length, n, a, radix)
    char buffer[];
    register unsigned buffer_length;
    register nbigit n;
    register abigit *a;
    register unsigned radix;
{
    register abigit *ti;
    register char *s;
    register s_abigit sign;
    abigit t[maximum_abigits];

    s = buffer + buffer_length;
    *--s = '\0';

    sign = a[n-1];
    big_abs (n, t, a);

    ti = t + n;
    while (*--ti == 0 && ti != t) {
	n -= 1;
    }

    while (1) {
	register unsigned d = big_udiv_i (n, t, t, radix);
	if (d > 9) {
	    d += 'A' - 10;
	}
	else {
	    d += '0';
	}
	*--s = d;
	if (*ti == 0) {
	    ti--;
	    n -= 1;
	    if (n == 0) break;
	}
    }

    if (sign < 0) {
	*--s = '-';
    }
    return s;
}

/* Unsigned normalized divide done the slow easy way */
/* A:NA = B:NB / C:NB, B:NB = remainder.  B and C are normalized fractions
   (i.e. high bit of high word is 1).  Produces exactly NA words of
   quotient.  One extra word of B is used (i.e. must be NB+1 abigits).
   Returns number of extra iterations to make msb of A 1. */
int
big_ndiv (na, a, nb, b, c)
    register nbigit na, nb;
    register abigit *a, *b, *c;
{
    register unsigned f;
    register int e;

    /* Non-restoring bit-at-time division */
    big_zero (na, a);
    b[nb] = 0;
    c[nb] = 0;
    e = na * abigit_bits;
loop1:
    e -= 1;
    big_lshift (na, a, a, 1);
    big_inc (na, a);
    big_sub3 (nb+1, b, b, c);
    if ((s_abigit)a[na-1] < 0) goto done;
    big_lshift (nb+1, b, b, 1);
    if ((s_abigit)b[nb] >= 0) goto loop1;
    goto loop2;

loop2:
    e -= 1;
    big_lshift (na, a, a, 1);
    big_dec (na, a);
    big_add3 (nb+1, b, b, c);
    if ((s_abigit)a[na-1] < 0) goto done;
    big_lshift (nb+1, b, b, 1);
    if ((s_abigit)b[nb] >= 0) goto loop1;
    goto loop2;

done:
    /* remainder negative, make positive */
    if ((s_abigit)b[nb] < 0) {
	big_add3 (nb+1, b, b, c);
	if ((s_abigit)b[nb] < 0) {
	    fprintf(stderr, "Internal error in big_ndiv.\n");
	}
	big_dec (na, a);	/*** can this unnormalize A? ***/
    }
done1: ;
    return e;
}
