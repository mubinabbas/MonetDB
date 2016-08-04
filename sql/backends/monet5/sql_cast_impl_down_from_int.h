/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0.  If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright 1997 - July 2008 CWI, August 2008 - 2016 MonetDB B.V.
 */

/* ! ENSURE THAT THESE LOCAL MACROS ARE UNDEFINED AT THE END OF THIS FILE ! */

/* stringify token */
#define _STRNG_(s) #s
#define STRNG(t) _STRNG_(t)

/* concatenate two, three or four tokens */
#define CONCAT_2(a,b)     a##b
#define CONCAT_3(a,b,c)   a##b##c
#define CONCAT_4(a,b,c,d) a##b##c##d

#define NIL(t)       CONCAT_2(t,_nil)
#define TPE(t)       CONCAT_2(TYPE_,t)
#define GDKmin(t)    CONCAT_3(GDK_,t,_min)
#define GDKmax(t)    CONCAT_3(GDK_,t,_max)
#define FUN(a,b,c,d) CONCAT_4(a,b,c,d)


str
FUN(,TP1,_dec2_,TP2) (TP2 *res, const int *s1, const TP1 *v)
{
	int scale = *s1;
	lng val = *v, h = 0;

	/* shortcut nil */
	if (*v == NIL(TP1)) {
		*res = NIL(TP2);
		return (MAL_SUCCEED);
	}

	if (scale)
		val = (val + h * scales[scale - 1]) / scales[scale];
	/* see if the number fits in the data type */
	if (val > (lng) GDKmin(TP2) && val <= GDKmax(TP2)
		) {
		*res = (TP2) val;
		return MAL_SUCCEED;
	} else {
		throw(SQL, "convert", "22003!value (" LLFMT ") exceeds limits of type "STRNG(TP2), val);
	}
}

str
FUN(,TP1,_dec2dec_,TP2) (TP2 *res, const int *S1, const TP1 *v, const int *d2, const int *S2)
{
	int p = *d2, inlen = 1;
	lng val = *v, cpyval = val, h = (val < 0) ? -5 : 5;
	int s1 = *S1, s2 = *S2;

	/* shortcut nil */
	if (*v == NIL(TP1)) {
		*res = NIL(TP2);
		return (MAL_SUCCEED);
	}

	/* count the number of digits in the input */
	while (cpyval /= 10)
		inlen++;
	/* rounding is allowed */
	inlen += (s2 - s1);
	if (p && inlen > p) {
		throw(SQL, STRNG(FUN(,TP1,_2_,TP2)), "22003!too many digits (%d > %d)", inlen, p);
	}

	if (s2 > s1)
		val *= scales[s2 - s1];
	else if (s2 != s1)
		val = (val + h * scales[s1 - s2 - 1]) / scales[s1 - s2];

	/* see if the number fits in the data type */
	if (val > (lng) GDKmin(TP2) && val <= GDKmax(TP2)
		) {
		*res = (TP2) val;
		return MAL_SUCCEED;
	} else {
		throw(SQL, "convert", "22003!value (" LLFMT ") exceeds limits of type "STRNG(TP2), val);
	}
}

str
FUN(,TP1,_num2dec_,TP2) (TP2 *res, const TP1 *v, const int *d2, const int *s2)
{
	int zero = 0;
	return FUN(,TP1,_dec2dec_,TP2)(res, &zero, v, d2, s2);
}

str
FUN(bat,TP1,_dec2_,TP2) (bat *res, const int *s1, const bat *bid)
{
	BAT *b, *bn;
	TP1 *p, *q;
	char *msg = NULL;
	int scale = *s1;
	TP2 *o;
	TP1 val;

	if ((b = BATdescriptor(*bid)) == NULL) {
		throw(SQL, "batcalc."STRNG(FUN(,TP1,_dec2_,TP2)), "Cannot access descriptor");
	}
	bn = COLnew(b->hseqbase, TPE(TP2), BATcount(b), TRANSIENT);
	if (bn == NULL) {
		BBPunfix(b->batCacheid);
		throw(SQL, "sql."STRNG(FUN(dec,TP1,_2_,TP2)), MAL_MALLOC_FAIL);
	}
	o = (TP2 *) Tloc(bn, 0);
	p = (TP1 *) Tloc(b, 0);
	q = (TP1 *) Tloc(b, BUNlast(b));
	bn->tnonil = 1;
	if (b->tnonil) {
		for (; p < q; p++, o++) {
			if (scale)
				val = (TP1) ((*p + (*p < 0 ? -5 : 5) * scales[scale - 1]) / scales[scale]);
			else
				val = (TP1) (*p);
			/* see if the number fits in the data type */
			if (val > (lng) GDKmin(TP2) && val <= GDKmax(TP2)
				)
				*o = (TP2) val;
			else {
				BBPunfix(b->batCacheid);
				BBPunfix(bn->batCacheid);
				throw(SQL, "convert", "22003!value (" LLFMT ") exceeds limits of type "STRNG(TP2), (lng) val);
			}
		}
	} else {
		for (; p < q; p++, o++) {
			if (*p == NIL(TP1)) {
				*o = NIL(TP2);
				bn->tnonil = FALSE;
			} else {
				if (scale)
					val = (TP1) ((*p + (*p < 0 ? -5 : 5) * scales[scale - 1]) / scales[scale]);
				else
					val = (TP1) (*p);
				/* see if the number fits in the data type */
				if (val > (lng) GDKmin(TP2)
				    && val <= GDKmax(TP2)
					)
					*o = (TP2) val;
				else {
					BBPunfix(b->batCacheid);
					BBPunfix(bn->batCacheid);
					throw(SQL, "convert", "22003!value (" LLFMT ") exceeds limits of type "STRNG(TP2), (lng) val);
				}
			}
		}
	}
	BATsetcount(bn, BATcount(b));
	bn->tsorted = 0;
	bn->trevsorted = 0;
	BATkey(bn, FALSE);

	BBPkeepref(*res = bn->batCacheid);
	BBPunfix(b->batCacheid);
	return msg;
}

str
FUN(bat,TP1,_dec2dec_,TP2) (bat *res, const int *S1, const bat *bid, const int *d2, const int *S2)
{
	BAT *b, *dst;
	BATiter bi;
	BUN p, q;
	char *msg = NULL;

	if ((b = BATdescriptor(*bid)) == NULL) {
		throw(SQL, "batcalc."STRNG(FUN(,TP1,_dec2dec_,TP2)), "Cannot access descriptor");
	}
	bi = bat_iterator(b);
	dst = COLnew(b->hseqbase, TPE(TP2), BATcount(b), TRANSIENT);
	if (dst == NULL) {
		BBPunfix(b->batCacheid);
		throw(SQL, "sql."STRNG(FUN(,TP1,_dec2dec_,TP2)), MAL_MALLOC_FAIL);
	}
	BATloop(b, p, q) {
		TP1 *v = (TP1 *) BUNtail(bi, p);
		TP2 r;
		msg = FUN(,TP1,_dec2dec_,TP2)(&r, S1, v, d2, S2);
		if (msg) {
			BBPunfix(dst->batCacheid);
			BBPunfix(b->batCacheid);
			return msg;
		}
		BUNappend(dst, &r, FALSE);
	}
	BBPkeepref(*res = dst->batCacheid);
	BBPunfix(b->batCacheid);
	return msg;
}

str
FUN(bat,TP1,_num2dec_,TP2) (bat *res, const bat *bid, const int *d2, const int *s2)
{
	BAT *b, *dst;
	BATiter bi;
	BUN p, q;
	char *msg = NULL;

	if ((b = BATdescriptor(*bid)) == NULL) {
		throw(SQL, "batcalc."STRNG(FUN(,TP1,_num2dec_,TP2)), "Cannot access descriptor");
	}
	bi = bat_iterator(b);
	dst = COLnew(b->hseqbase, TPE(TP2), BATcount(b), TRANSIENT);
	if (dst == NULL) {
		BBPunfix(b->batCacheid);
		throw(SQL, "sql."STRNG(FUN(,TP1,_num2dec_,TP2)), MAL_MALLOC_FAIL);
	}
	BATloop(b, p, q) {
		TP1 *v = (TP1 *) BUNtail(bi, p);
		TP2 r;
		msg = FUN(,TP1,_num2dec_,TP2)(&r, v, d2, s2);
		if (msg) {
			BBPunfix(dst->batCacheid);
			BBPunfix(b->batCacheid);
			return msg;
		}
		BUNappend(dst, &r, FALSE);
	}
	BBPkeepref(*res = dst->batCacheid);
	BBPunfix(b->batCacheid);
	return msg;
}


/* undo local defines */
#undef FUN
#undef NIL
#undef TPE
#undef GDKmin
#undef GDKmax
#undef CONCAT_2
#undef CONCAT_3
#undef CONCAT_4
#undef STRNG
#undef _STRNG_

