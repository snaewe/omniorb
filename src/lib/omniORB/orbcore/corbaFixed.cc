// -*- Mode: C++; -*-
//                            Package   : omniORB
// corbaFixed.cc              Created on: 07/02/2001
//                            Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2001 AT&T Laboratories Cambridge
//
//    This file is part of the omniORB library
//
//    The omniORB library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Library General Public
//    License as published by the Free Software Foundation; either
//    version 2 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Library General Public License for more details.
//
//    You should have received a copy of the GNU Library General Public
//    License along with this library; if not, write to the Free
//    Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//    02111-1307, USA
//
//
// Description:
//
//    Implementation of the fixed point type

// $Log$
// Revision 1.1.2.5  2001/08/17 13:47:32  dpg1
// Small bug fixes.
//
// Revision 1.1.2.4  2001/08/03 17:41:18  sll
// System exception minor code overhaul. When a system exeception is raised,
// a meaning minor code is provided.
//
// Revision 1.1.2.3  2001/06/13 20:12:32  sll
// Minor updates to make the ORB compiles with MSVC++.
//
// Revision 1.1.2.2  2001/04/09 15:18:46  dpg1
// Tweak fixed point to make life easier for omniORBpy.
//
// Revision 1.1.2.1  2001/03/13 10:32:10  dpg1
// Fixed point support.
//

#include <omniORB4/CORBA.h>
#include <exceptiondefs.h>

#include <stdio.h>
#include <string.h>
#include <ctype.h>

OMNI_USING_NAMESPACE(omni)

CORBA::Fixed::Fixed(int val) :
  pd_digits(0), pd_scale(0), pd_negative(0), pd_idl_digits(0), pd_idl_scale(0)
{
  if (val < 0) {
    pd_negative = 1;
    val = - val;
  }
  while (val) {
    pd_val[pd_digits++] = val % 10;
    val /= 10;
  }
  memset(pd_val + pd_digits, 0, OMNI_FIXED_DIGITS - pd_digits);
}

CORBA::Fixed::Fixed(unsigned val) :
  pd_digits(0), pd_scale(0), pd_negative(0), pd_idl_digits(0), pd_idl_scale(0)
{
  while (val) {
    pd_val[pd_digits++] = val % 10;
    val /= 10;
  }
  memset(pd_val + pd_digits, 0, OMNI_FIXED_DIGITS - pd_digits);
}


#ifndef OMNI_LONG_IS_INT

CORBA::Fixed::Fixed(CORBA::Long val) :
  pd_digits(0), pd_scale(0), pd_negative(0), pd_idl_digits(0), pd_idl_scale(0)
{
  if (val < 0) {
    pd_negative = 1;
    val = - val;
  }
  while (val) {
    pd_val[pd_digits++] = val % 10;
    val /= 10;
  }
  memset(pd_val + pd_digits, 0, OMNI_FIXED_DIGITS - pd_digits);
}

CORBA::Fixed::Fixed(CORBA::ULong val) :
  pd_digits(0), pd_scale(0), pd_negative(0), pd_idl_digits(0), pd_idl_scale(0)
{
  while (val) {
    pd_val[pd_digits++] = val % 10;
    val /= 10;
  }
  memset(pd_val + pd_digits, 0, OMNI_FIXED_DIGITS - pd_digits);
}

#endif

#ifdef HAS_LongLong

CORBA::Fixed::Fixed(CORBA::LongLong val) :
  pd_digits(0), pd_scale(0), pd_negative(0), pd_idl_digits(0), pd_idl_scale(0)
{
  if (val < 0) {
    pd_negative = 1;
    val = - val;
  }
  while (val) {
    pd_val[pd_digits++] = val % 10;
    val /= 10;
  }
  memset(pd_val + pd_digits, 0, OMNI_FIXED_DIGITS - pd_digits);
}

CORBA::Fixed::Fixed(CORBA::ULongLong val) :
  pd_digits(0), pd_scale(0), pd_negative(0), pd_idl_digits(0), pd_idl_scale(0)
{
  while (val) {
    pd_val[pd_digits++] = val % 10;
    val /= 10;
  }
  memset(pd_val + pd_digits, 0, OMNI_FIXED_DIGITS - pd_digits);
}

#endif

#ifndef NO_FLOAT

CORBA::Fixed::Fixed(CORBA::Double val) :
  pd_idl_digits(0), pd_idl_scale(0)
{
  if (val > 1e32 || val < -1e32) {
    // Too big
    OMNIORB_THROW(DATA_CONVERSION, DATA_CONVERSION_RangeError,
		  CORBA::COMPLETED_NO);
  }
  char buffer[80];
  int len = sprintf(buffer, "%.31f", val);
  OMNIORB_ASSERT(len < 79);

  NP_fromString(buffer);
}

#endif


#ifdef HAS_LongDouble

# error "LongDouble not supported yet."

#endif


CORBA::Fixed::Fixed(const Fixed& val) :
  pd_digits    (val.pd_digits),
  pd_scale     (val.pd_scale),
  pd_negative  (val.pd_negative),
  pd_idl_digits(0),
  pd_idl_scale (0)
{
  memcpy(pd_val, val.pd_val, OMNI_FIXED_DIGITS);
}

CORBA::Fixed::Fixed(const char* val) :
  pd_idl_digits(0), pd_idl_scale(0)
{
  NP_fromString(val);
}

CORBA::Fixed::Fixed(const CORBA::Octet* val,
		    CORBA::UShort       digits,
		    CORBA::UShort       scale,
		    CORBA::Boolean      negative)
  : pd_digits(digits), pd_scale(scale), pd_negative(negative),
    pd_idl_digits(0), pd_idl_scale(0)
{
  OMNIORB_ASSERT(digits <= OMNI_FIXED_DIGITS);
  OMNIORB_ASSERT(scale  <= digits);

  if (digits == 0) pd_negative = 0;

  memcpy(pd_val, val, digits);
  memset(pd_val + digits, 0, OMNI_FIXED_DIGITS - digits);
}


CORBA::Fixed::~Fixed()
{
}


#ifdef HAS_LongLong
CORBA::Fixed::operator CORBA::LongLong() const
{
  CORBA::LongLong r = 0, s;

  for (int i = pd_digits - 1; i >= pd_scale; --i) {
    s = r * 10 + pd_val[i];
    if (s < r) {
      // Overflow
      OMNIORB_THROW(DATA_CONVERSION, DATA_CONVERSION_RangeError, 
		    CORBA::COMPLETED_NO);
    }
    r = s;
  }
  if (pd_negative)
    r = -r;

  return r;
}

#else

CORBA::Fixed::operator CORBA::Long() const
{
  CORBA::Long r = 0, s;

  for (int i = pd_digits - 1; i >= pd_scale; --i) {
    s = r * 10 + pd_val[i];
    if (s < r) {
      // Overflow
      OMNIORB_THROW(DATA_CONVERSION, DATA_CONVERSION_RangeError,
		    CORBA::COMPLETED_NO);
    }
    r = s;
  }
  if (pd_negative)
    r = -r;

  return r;
}

#endif


#ifndef NO_FLOAT

#  ifdef HAS_LongDouble

#    error "LongDouble not supported yet."

#  else

CORBA::Fixed::operator CORBA::Double() const
{
  CORBA::Double r = 0, s = 0;
  int i;

  // Digits before decimal point
  for (i = pd_digits - 1; i >= pd_scale; --i) {
    r = r * 10 + pd_val[i];
  }

  // Digits after decimal point
  for (i=0; i < pd_scale; ++i) {
    s = (s + pd_val[i]) / 10;
  }

  return r + s;
}

#  endif
#endif


CORBA::Fixed
CORBA::Fixed::round(CORBA::UShort scale) const
{
  if (scale >= pd_scale)
    return *this;

  int cut = pd_scale - scale;

  if (pd_val[cut - 1] >= 5) {
    // Round up
    CORBA::Octet work[OMNI_FIXED_DIGITS];
    memcpy(work, pd_val, OMNI_FIXED_DIGITS);

    int i = cut;
    for (i = cut; i < OMNI_FIXED_DIGITS; ++i) {
      if (++work[cut] <= 9) break;
      work[cut] = 0;
    }
    if (i == OMNI_FIXED_DIGITS) {
      // Overflow
      OMNIORB_THROW(DATA_CONVERSION, DATA_CONVERSION_RangeError,
		    CORBA::COMPLETED_NO);
    }
    return CORBA::Fixed(work + cut, pd_digits - cut, scale, pd_negative);
  }
  else
    return CORBA::Fixed(pd_val + cut, pd_digits - cut, scale, pd_negative);
}


CORBA::Fixed
CORBA::Fixed::truncate(CORBA::UShort scale) const
{
  if (scale >= pd_scale)
    return *this;

  int cut      = pd_scale - scale;
  int newscale = scale;

  while (pd_val[cut] == 0 && newscale > 0) {
    ++cut;
    --newscale;
  }

  return CORBA::Fixed(pd_val + cut, pd_digits - cut, newscale, pd_negative);
}


CORBA::Fixed&
CORBA::Fixed::operator=(const Fixed& val)
{
  pd_digits   = val.pd_digits;
  pd_scale    = val.pd_scale;
  pd_negative = val.pd_negative;

  memcpy(pd_val, val.pd_val, OMNI_FIXED_DIGITS);

  PR_checkLimits();

  return *this;
}

CORBA::Fixed&
CORBA::Fixed::operator+=(const Fixed& val)
{
  *this = *this + val;
  return *this;
}

CORBA::Fixed&
CORBA::Fixed::operator-=(const Fixed& val)
{
  *this = *this - val;
  return *this;
}

CORBA::Fixed&
CORBA::Fixed::operator*=(const Fixed& val)
{
  *this = *this * val;
  return *this;
}

CORBA::Fixed&
CORBA::Fixed::operator/=(const Fixed& val)
{
  *this = *this / val;
  return *this;
}

CORBA::Fixed&
CORBA::Fixed::operator++()
{
  *this = *this + CORBA::Fixed(1);
  return *this;
}

CORBA::Fixed
CORBA::Fixed::operator++(int)
{
  CORBA::Fixed r(*this);
  *this = *this + CORBA::Fixed(1);
  return r;
}

CORBA::Fixed&
CORBA::Fixed::operator--()
{
  *this = *this - CORBA::Fixed(1);
  return *this;
}

CORBA::Fixed
CORBA::Fixed::operator--(int)
{
  CORBA::Fixed r(*this);
  *this = *this - CORBA::Fixed(1);
  return r;
}

CORBA::Fixed
CORBA::Fixed::operator+() const
{
  return *this;
}

CORBA::Fixed
CORBA::Fixed::operator-() const
{
  if (pd_digits == 0)
    return *this;

  CORBA::Fixed r(*this);
  r.pd_negative = !pd_negative;
  return r;
}

CORBA::Boolean
CORBA::Fixed::operator!() const
{
  return pd_digits == 0;
}

char*
CORBA::Fixed::NP_asString() const
{
  int len = pd_digits + 1;
  if (pd_negative)           ++len; // for '-'
  if (pd_digits == pd_scale) ++len; // for '0'
  if (pd_scale > 0)          ++len; // for '.'

  char* r = CORBA::string_alloc(len);
  int i = 0, j;

  if (pd_negative)           r[i++] = '-';
  if (pd_digits == pd_scale) r[i++] = '0';

  for (j=pd_digits; j; ) {
    if (j-- == pd_scale)
      r[i++] = '.';
    r[i++] = pd_val[j] + '0';
  }
  r[i] = '\0';
  return r;
}

void
CORBA::Fixed::NP_fromString(const char* s, CORBA::Boolean ignore_end)
{
  // Skip leading white space
  while (isspace(*s)) ++s;

  // Sign
  if (*s == '-') {
    pd_negative = 1;
    ++s;
  }
  else if (*s == '+') {
    pd_negative = 0;
    ++s;
  }
  else
    pd_negative = 0;

  // Check there are some digits
  if (!((*s >= '0' && *s <= '9') || *s == '.')) {
    OMNIORB_THROW(DATA_CONVERSION,
		  DATA_CONVERSION_BadInput, CORBA::COMPLETED_NO);
  }

  // Skip leading zeros:
  while (*s == '0') ++s;

  int i, j, unscale = -1;

  // Count digits
  for (i=0, pd_digits=0; (s[i] >= '0' && s[i] <= '9') || s[i] == '.'; ++i) {
    if (s[i] == '.') {
      if (unscale != -1) {
	// Already seen a decimal point
	OMNIORB_THROW(DATA_CONVERSION,
		      DATA_CONVERSION_BadInput, CORBA::COMPLETED_NO);
      }
      unscale = pd_digits;
    }
    else
      ++pd_digits;
  }
  if (unscale == -1)
    unscale = pd_digits;

  pd_scale = pd_digits - unscale;

  // Check there is no trailing garbage
  if (!ignore_end) {
    j = i;
    if (s[i] == 'd' || s[i] == 'D') ++j;

    while (s[j]) {
      if (!isspace(s[j])) {
	OMNIORB_THROW(DATA_CONVERSION,
		      DATA_CONVERSION_BadInput, CORBA::COMPLETED_NO);
      }
      ++j;
    }
  }

  --i; // i is now the index of the last digit

  // Truncate if too many digits
  while (pd_digits > OMNI_FIXED_DIGITS && pd_scale > 0) {
    --i; --pd_digits; --pd_scale;
  }

  // Back-up over trailing zeros
  if (pd_scale > 0) {
    while (s[i] == '0') {
      --i; --pd_digits; --pd_scale;
    }
  }

  if (pd_digits > OMNI_FIXED_DIGITS) {
    OMNIORB_THROW(DATA_CONVERSION,
		  DATA_CONVERSION_BadInput, CORBA::COMPLETED_NO);
  }

  // Scan back through the string, setting the value least
  // siginificant digit first.
  for (j=0; j < pd_digits; ++j, --i) {
    if (s[i] == '.') --i;
    pd_val[j] = s[i] - '0';
  }
  // Clear any remaining digits
  memset(pd_val + j, 0, OMNI_FIXED_DIGITS - j);

  // Make sure zero is always positive
  if (pd_digits == 0) pd_negative = 0;

  PR_checkLimits();
}


void
CORBA::Fixed::PR_setLimits(UShort idl_digits, UShort idl_scale)
{
  OMNIORB_ASSERT(idl_digits <= 31);
  OMNIORB_ASSERT(idl_scale  <= idl_digits);

  pd_idl_digits = idl_digits;
  pd_idl_scale  = idl_scale;
  PR_checkLimits();
}

void
CORBA::Fixed::PR_checkLimits()
{
  if (pd_idl_digits == 0) return;

  if (pd_scale > pd_idl_scale)
    *this = truncate(pd_idl_scale);

  if (pd_digits - pd_scale > pd_idl_digits - pd_idl_scale)
    OMNIORB_THROW(DATA_CONVERSION,
		  DATA_CONVERSION_RangeError, CORBA::COMPLETED_NO);
}

void
CORBA::Fixed::PR_changeScale(CORBA::UShort new_scale)
{
  if (new_scale > pd_digits)
    pd_digits = new_scale;

  pd_scale = new_scale;
}


//
// Arithmetic
//

static int
absCmp(const CORBA::Fixed& a, const CORBA::Fixed& b)
{
  int c;
  c = (a.fixed_digits()-a.fixed_scale()) - (b.fixed_digits()-b.fixed_scale());
  if (c) return c;

  int ai, bi;
  ai = a.fixed_digits() - 1;
  bi = b.fixed_digits() - 1;

  while (ai >= 0 && bi >= 0) {
    c = a.PR_val()[ai] - b.PR_val()[bi];
    if (c) return c;
    --ai; --bi;
  }
  if (ai > 0) return  1;
  if (bi > 0) return -1;
  return 0;
}


static CORBA::Fixed
realAdd(const CORBA::Fixed& a, const CORBA::Fixed& b, CORBA::Boolean negative)
{
  int scale, v, carry = 0, ai = 0, bi = 0, wi = 0;
  CORBA::Octet work[OMNI_FIXED_DIGITS * 2];

  if (a.fixed_scale() > b.fixed_scale()) {
    scale = a.fixed_scale();

    while (ai < a.fixed_scale() - b.fixed_scale())
      work[wi++] = a.PR_val()[ai++];
  }
  else if (b.fixed_scale() > a.fixed_scale()) {
    scale = b.fixed_scale();

    while (bi < b.fixed_scale() - a.fixed_scale())
      work[wi++] = b.PR_val()[bi++];
  }
  else
    scale = a.fixed_scale();

  while (ai < a.fixed_digits() && bi < b.fixed_digits()) {
    v = a.PR_val()[ai++] + b.PR_val()[bi++] + carry;
    if (v > 9) {
      carry = 1; v -= 10;
    }
    else carry = 0;
    work[wi++] = v;
  }
  while (ai < a.fixed_digits()) {
    v = a.PR_val()[ai++] + carry;
    if (v > 9) {
      carry = 1; v -= 10;
    }
    else carry = 0;
    work[wi++] = v;
  }
  while (bi < b.fixed_digits()) {
    v = b.PR_val()[bi++] + carry;
    if (v > 9) {
      carry = 1; v -= 10;
    }
    else carry = 0;
    work[wi++] = v;
  }
  if (carry) {
    work[wi++] = carry;
  }

  CORBA::Octet* wp = work;
  int digits = wi;

  // Truncate or complain if too many digits
  if (digits > OMNI_FIXED_DIGITS) {
    if (digits - scale <= OMNI_FIXED_DIGITS) {
      int chop  = digits - OMNI_FIXED_DIGITS;
      wp       += chop;
      scale    -= chop;
      digits    = OMNI_FIXED_DIGITS;
    }
    else {
      OMNIORB_THROW(DATA_CONVERSION,
		    DATA_CONVERSION_RangeError, CORBA::COMPLETED_NO);
    }
  }

  // Strip trailing zeros
  while (scale > 0 && *wp == 0) {
    ++wp; --scale; --digits;
  }
  return CORBA::Fixed(wp, digits, scale, negative);
}

static CORBA::Fixed
realSub(const CORBA::Fixed& a, const CORBA::Fixed& b, CORBA::Boolean negative)
{
  int scale, v, carry = 0, ai = 0, bi = 0, wi = 0;
  CORBA::Octet work[OMNI_FIXED_DIGITS * 2];

  if (a.fixed_scale() > b.fixed_scale()) {
    scale = a.fixed_scale();

    while (ai < a.fixed_scale() - b.fixed_scale())
      work[wi++] = a.PR_val()[ai++];
  }
  else if (b.fixed_scale() > a.fixed_scale()) {
    scale = b.fixed_scale();

    while (bi < b.fixed_scale() - a.fixed_scale()) {
      work[wi++] = 10 - b.PR_val()[bi++] + carry;
      carry = -1;
    }
  }
  else
    scale = a.fixed_scale();

  while (ai < a.fixed_digits() && bi < b.fixed_digits()) {
    v = a.PR_val()[ai++] - b.PR_val()[bi++] + carry;
    if (v < 0) {
      carry = -1; v += 10;
    }
    else carry = 0;
    work[wi++] = v;
  }
  while (ai < a.fixed_digits()) {
    v = a.PR_val()[ai++] + carry;
    if (v < 0) {
      carry = -1; v += 10;
    }
    else carry = 0;
    work[wi++] = v;
  }
  OMNIORB_ASSERT(bi = b.fixed_digits());
  OMNIORB_ASSERT(carry == 0);

  int digits = wi;
  CORBA::Octet* wp = work;

  // Strip leading zeros
  while (work[digits-1] == 0 && digits > scale)
    --digits;

  // Truncate or complain if too many digits
  if (digits > OMNI_FIXED_DIGITS) {
    OMNIORB_ASSERT(digits - scale <= OMNI_FIXED_DIGITS);

    int chop  = digits - OMNI_FIXED_DIGITS;
    wp       += chop;
    scale    -= chop;
    digits    = OMNI_FIXED_DIGITS;
  }

  // Strip trailing zeros
  while (scale > 0 && *wp == 0) {
    ++wp; --scale; --digits;
  }

  return CORBA::Fixed(wp, digits, scale, negative);
}


static CORBA::Fixed
realMul(const CORBA::Fixed& a, const CORBA::Fixed& b, CORBA::Boolean negative)
{
  int ai, bi, wi, digits, scale, v, ad, bd, carry = 0;
  CORBA::Octet work[OMNI_FIXED_DIGITS * 2];

  memset(work, 0, OMNI_FIXED_DIGITS * 2);

  scale = a.fixed_scale() + b.fixed_scale();

  for (ai=0, wi=0; ai < a.fixed_digits(); ++ai) {
    ad = a.PR_val()[ai];
    if (ad == 0) continue;

    for (bi=0; bi < b.fixed_digits(); ++bi) {
      bd = b.PR_val()[bi];
      wi       = ai + bi;
      v        = work[wi] + ad * bd + carry;
      carry    = v / 10;
      work[wi] = v % 10;
    }
    while (carry) {
      ++wi;
      v        = work[wi] + carry;
      carry    = v / 10;
      work[wi] = v % 10;
    }
  }
  digits = wi+1;
  if (scale > digits) digits = scale;

  // Truncate or complain if too many digits
  CORBA::Octet* wp = work;
  if (digits > OMNI_FIXED_DIGITS) {
    if (digits - scale <= OMNI_FIXED_DIGITS) {
      int chop  = digits - OMNI_FIXED_DIGITS;
      wp       += chop;
      scale    -= chop;
      digits    = OMNI_FIXED_DIGITS;
    }
    else {
      OMNIORB_THROW(DATA_CONVERSION,
		    DATA_CONVERSION_RangeError, CORBA::COMPLETED_NO);
    }
  }

  // Strip trailing zeros
  while (scale > 0 && *wp == 0) {
    ++wp; --scale; --digits;
  }

  return CORBA::Fixed(wp, digits, scale, negative);
}


static int
divCmp(const CORBA::Octet* av, int ad, const CORBA::Octet* bv, int bd, int pos)
{
  int c, ai, bi;

  for (ai = ad-1; ai > pos; --ai) {
    if (av[ai])
      return 1;
  }
  ai = pos;
  bi = bd - 1;
  OMNIORB_ASSERT(ai >= bi);
  while (bi >= 0) {
    c = av[ai] - bv[bi];
    if (c) return c;
    --ai; --bi;
  }
  return 0;
}


static int
divDigit(CORBA::Octet* av, int ad, const CORBA::Octet* bv, int bd, int pos)
{
  int ai, bi, carry, v, count = 0;
  while (divCmp(av, ad, bv, bd, pos) >= 0) {
    ++count;
    carry = 0;

    ai = pos - bd + 1;
    bi = 0;
    while (bi < bd) {
      v = av[ai] - bv[bi] + carry;
      if (v < 0) {
	carry = -1;
	v += 10;
      }
      else
	carry = 0;
      av[ai] = v;
      ++ai;
      ++bi;
    }
    while (ai < ad) {
      v = av[ai] + carry;
      if (v < 0) {
	carry = -1;
	v += 10;
      }
      else
	carry = 0;
      av[ai] = v;
      ++ai;
    }
  }
  OMNIORB_ASSERT(count < 10);
  return count;
}


static CORBA::Fixed
realDiv(const CORBA::Fixed& a, const CORBA::Fixed& b, CORBA::Boolean negative)
{
  int i, ai, bi, wi, ri, digits, scale, unscale, v, ad, bd, carry = 0;

  // This division algorithm basically does classic long division. The
  // numerator, a, is loaded into the top digits of "running". The
  // divisor, b, is then repeatedly subtracted from the top digits of
  // "running" until it can no longer be subtracted without becoming
  // negative. The count of subtractions forms the top digit of the
  // result. Then the next digit is found by shifting the divisor down
  // one digit, and repeating, and so on.
  //
  // The ugliness all comes because we need to figure out where to put
  // the decimal point. It would be easy if it wasn't for the fact
  // that values with scale > digits are not permitted. This means
  // that if the result is going to be of that form, some initial zero
  // digits have to be included.

  CORBA::Octet work   [OMNI_FIXED_DIGITS * 2];
  CORBA::Octet running[OMNI_FIXED_DIGITS * 2];

  memset(work,    0, OMNI_FIXED_DIGITS * 2);
  memset(running, 0, OMNI_FIXED_DIGITS * 2);

  // Skip all leading zeros in a
  ad = a.fixed_digits();
  while (a.PR_val()[ad - 1] == 0) --ad;

  // Set most siginificant digits of running to a's digits
  ri = OMNI_FIXED_DIGITS * 2 - 1;
  ai = ad - 1;
  while (ai >= 0) {
    running[ri] = a.PR_val()[ai];
    --ri;
    --ai;
  }

  // Skip all leading zeros in b
  bd = b.fixed_digits();
  while (b.PR_val()[bd - 1] == 0) --bd;

  // unscale = number of digits to left of decimal point in answer
  unscale = (ad - a.fixed_scale()) - (bd - b.fixed_scale()) + 1;
  digits  = 0;

  // Set some initial zero digits to prevent scale > digits
  if (unscale < 0)
    digits  = -unscale;

  ri = OMNI_FIXED_DIGITS * 2 - 1;
  wi = ri - digits;

  // Iterate to work out the result digits
  while (digits < OMNI_FIXED_DIGITS) {

    // Finish if running is zero
    for (i=0; i < OMNI_FIXED_DIGITS*2 && running[i] == 0; ++i);
    if (i == OMNI_FIXED_DIGITS * 2)
      break;

    work[wi] = divDigit(running, OMNI_FIXED_DIGITS * 2, b.PR_val(), bd, ri);

    if (digits || work[wi])
      ++digits;

    --wi; --ri;
  }
  wi = OMNI_FIXED_DIGITS * 2 - 1;

  // Skip an initial zero if we weren't expecting one
  if (unscale >= 0) {
    while (work[wi] == 0) {
      --wi; --unscale;
    }
  }
  else
    unscale = 0;

  // Complain if too many digits before decimal point
  if (unscale > OMNI_FIXED_DIGITS) {
    OMNIORB_THROW(DATA_CONVERSION,
		  DATA_CONVERSION_RangeError, CORBA::COMPLETED_NO);
  }

  // Deal with numbers with trailing zeros before the decimal point
  if (digits < unscale)
    digits = unscale;

  CORBA::Octet* wp = &work[wi - digits + 1];

  // Figure out scale
  scale = digits - unscale;
  if (digits < scale)
    digits = scale;

  // Strip trailing zeros
  while (scale > 0 && *wp == 0) {
    ++wp; --scale; --digits;
  }

  return CORBA::Fixed(wp, digits, scale, negative);
}


//
// Arithmetic operators
//

CORBA::Fixed
operator+(const CORBA::Fixed& a, const CORBA::Fixed& b)
{
  if (a.PR_negative() == b.PR_negative())
    return realAdd(a, b, a.PR_negative());

  int cmp = absCmp(a, b);

  if (cmp == 0)     // a == b
    return CORBA::Fixed();
  else if (cmp > 0) // a > b
    return realSub(a, b, a.PR_negative());
  else
    return realSub(b, a, b.PR_negative());
}

CORBA::Fixed
operator-(const CORBA::Fixed& a, const CORBA::Fixed& b)
{
  if (a.PR_negative() != b.PR_negative())
    return realAdd(a, b, a.PR_negative());

  int cmp = absCmp(a, b);

  if (cmp == 0)     // a == b
    return CORBA::Fixed();
  else if (cmp > 0) // a > b
    return realSub(a, b, a.PR_negative());
  else
    return realSub(b, a, !a.PR_negative());
}

CORBA::Fixed
operator*(const CORBA::Fixed& a, const CORBA::Fixed& b)
{
  if (a.fixed_digits() == 0 || b.fixed_digits() == 0)
    return CORBA::Fixed();

  if (a.PR_negative() == b.PR_negative())
    return realMul(a, b, 0);
  else
    return realMul(a, b, 1);
}

CORBA::Fixed
operator/(const CORBA::Fixed& a, const CORBA::Fixed& b)
{
  if (b.fixed_digits() == 0) {
    int fixed_point_divide_by_zero = 0;
    OMNIORB_USER_CHECK(fixed_point_divide_by_zero);
  }

  if (a.fixed_digits() == 0)
    return CORBA::Fixed();

  if (a.PR_negative() == b.PR_negative())
    return realDiv(a, b, 0);
  else
    return realDiv(a, b, 1);
}


//
// Comparison operators
//

int
CORBA::Fixed::NP_cmp(const Fixed& a, const Fixed& b)
{
  int c;

  if (a.PR_negative()) {
    if (b.PR_negative())
      c = absCmp(b, a);
    else
      return -1;
  }
  else {
    if (b.PR_negative())
      return 1;
    else
      c = absCmp(a, b);
  }
  if (c < 0) return -1;
  if (c > 0) return  1;
  return 0;
}


CORBA::Boolean
operator>(const CORBA::Fixed& a, const CORBA::Fixed& b)
{
  return CORBA::Fixed::NP_cmp(a, b) > 0;
}

CORBA::Boolean
operator<(const CORBA::Fixed& a, const CORBA::Fixed& b)
{
  return CORBA::Fixed::NP_cmp(a, b) < 0;
}

CORBA::Boolean
operator>=(const CORBA::Fixed& a, const CORBA::Fixed& b)
{
  return CORBA::Fixed::NP_cmp(a, b) >= 0;
}

CORBA::Boolean
operator<=(const CORBA::Fixed& a, const CORBA::Fixed& b)
{
  return CORBA::Fixed::NP_cmp(a, b) <= 0;
}

CORBA::Boolean
operator==(const CORBA::Fixed& a, const CORBA::Fixed& b)
{
  return CORBA::Fixed::NP_cmp(a, b) == 0;
}

CORBA::Boolean
operator!=(const CORBA::Fixed& a, const CORBA::Fixed& b)
{
  return CORBA::Fixed::NP_cmp(a, b) != 0;
}


//
// Marshalling operators
//

void
CORBA::Fixed::operator>>=(cdrStream& s) const
{
  OMNIORB_ASSERT(pd_idl_digits);
  OMNIORB_ASSERT(pd_digits <= pd_idl_digits);
  OMNIORB_ASSERT(pd_scale  <= pd_idl_scale);

  CORBA::Octet buffer[16];

  int digits_to_write = pd_idl_digits;

  // Pad with extra zero digit if even no. of digits
  if ((digits_to_write % 2) == 0) ++digits_to_write;

  int unscale_to_write = digits_to_write - pd_idl_scale;
  int unscale          = pd_digits       - pd_scale;

  int bi, vi;

  // Write any leading zeros
  for (bi=0; bi < unscale_to_write - unscale; bi+=2) {
    buffer[bi/2] = 0;
  }
  bi = unscale_to_write - unscale;
  vi = pd_digits - 1;

  // Write digits
  for (; vi >= 0; --vi, ++bi) {
    if (bi % 2)
      buffer[bi/2] |= pd_val[vi];
    else
      buffer[bi/2]  = pd_val[vi] << 4;
  }

  // Write trailing zeros
  if (bi % 2) ++bi;

  for (; bi < digits_to_write; bi+=2) {
    buffer[bi/2] = 0;
  }

  // Sign indicator
  buffer[digits_to_write / 2] |= pd_negative ? 0xD : 0xC;

  // Marshal the buffer
  s.put_octet_array(buffer, digits_to_write / 2 + 1);
}


void
CORBA::Fixed::operator<<=(cdrStream& s)
{
  OMNIORB_ASSERT(pd_idl_digits);

  CORBA::Octet buffer[16];

  int digits_to_read = pd_idl_digits;
  if ((digits_to_read % 2) == 0) ++digits_to_read;

  pd_digits = digits_to_read;
  pd_scale  = pd_idl_scale;

  // Read the data
  s.get_octet_array(buffer, digits_to_read / 2 + 1);

  CORBA::Octet d;

  // Sign indicator
  int vi, di, bi = digits_to_read;

  d = buffer[bi/2] & 0xF;

  if (d == 0xC)
    pd_negative = 0;
  else if (d == 0xD)
    pd_negative = 1;
  else
    OMNIORB_THROW(MARSHAL, MARSHAL_InvalidFixedValue, 
		  (CORBA::CompletionStatus)s.completion());

  --bi;

  di = vi = 0;

  // Skip trailing zeros
  while (di < pd_idl_scale) {
    if (bi % 2)
      d = buffer[bi/2] & 0xF;
    else
      d = buffer[bi/2] >> 4;

    if (d != 0) break;

    --bi;
    ++di;
    --pd_digits;
    --pd_scale;
  }

  // Read and check the digits
  for (vi = 0; di < digits_to_read; ++di, ++vi, --bi) {

    if (bi % 2)
      d = buffer[bi/2] & 0xF;
    else
      d = buffer[bi/2] >> 4;

    if (d > 0x9)
      OMNIORB_THROW(MARSHAL, MARSHAL_InvalidFixedValue, 
		    (CORBA::CompletionStatus)s.completion());

    pd_val[vi] = d;
  }
  OMNIORB_ASSERT(vi == pd_digits);

  // Strip off leading zeros
  --vi;
  while (pd_val[vi] == 0) --vi;

  pd_digits = vi + 1;

  // Fill rest of val with zeros
  memset(pd_val + pd_digits, 0, OMNI_FIXED_DIGITS - pd_digits);

  // Sanity check
  if (pd_digits - pd_scale == pd_idl_digits - pd_idl_scale + 1) {
    // This happens if idl_digits is even, and the sending ORB put a
    // non-zero half-octet as the most-siginificant digit.
    OMNIORB_THROW(MARSHAL, MARSHAL_InvalidFixedValue, 
		  (CORBA::CompletionStatus)s.completion());
  }

  OMNIORB_ASSERT(pd_digits <= pd_idl_digits);
  OMNIORB_ASSERT(pd_scale  <= pd_idl_scale);
}
