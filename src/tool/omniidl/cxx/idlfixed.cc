// -*- c++ -*-
//                          Package   : omniidl
// idlfixed.h               Created on: 2001/01/31
//			    Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 2001 AT&T Laboratories Cambridge
//
//  This file is part of omniidl.
//
//  omniidl is free software; you can redistribute it and/or modify it
//  under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.
//
// Description:
//   
//   Implementation of fixed point type

/*
  $Log$
  Revision 1.1.2.1  2001/03/13 10:32:12  dpg1
  Fixed point support.

*/

#include <idlfixed.h>
#include <idlutil.h>
#include <idlerr.h>

#include <string.h>


IDL_Fixed::IDL_Fixed() :
  digits_(0), scale_(0), negative_(0)
{
  memset(val_, 0, OMNI_FIXED_DIGITS);
}


IDL_Fixed::IDL_Fixed(const IDL_Fixed& f) :
  digits_(f.digits_), scale_(f.scale_), negative_(f.negative_)
{
  memcpy(val_, f.val_, OMNI_FIXED_DIGITS);
}

IDL_Fixed::IDL_Fixed(const char* s, const char* file, int line)
{
  // Sign (Actually redundant for omniidl. Here for completeness.)
  if (s[0] == '-') {
    negative_ = 1;
    ++s;
  }
  else if (s[0] == '+') {
    negative_ = 0;
    ++s;
  }
  else
    negative_ = 0;

  // Check there are some digits
  assert(*s != '\0' && *s != 'd' && *s != 'D');

  // Skip leading zeros:
  while (*s == '0') ++s;

  int i, j, unscale = -1;

  // Count digits
  for (i=0, digits_=0; (s[i] >= '0' && s[i] <= '9') || s[i] == '.'; ++i) {
    if (s[i] == '.') {
      assert(unscale == -1);
      unscale = digits_;
    }
    else
      ++digits_;
  }
  if (unscale == -1)
    unscale = digits_;

  scale_ = digits_ - unscale;

  // Check there is no trailing garbage
  if (s[i] == 'd' || s[i] == 'D')
    assert(s[i+1] == '\0');
  else
    assert(s[i] == '\0');

  --i; // i is now the index of the last digit

  // Truncate if too many digits
  while (digits_ > OMNI_FIXED_DIGITS && scale_ > 0) {
    --i; --digits_; --scale_;
  }

  // Back-up over trailing zeros
  if (scale_ > 0) {
    while (s[i] == '0') {
      --i; --digits_; --scale_;
    }
  }

  if (digits_ > OMNI_FIXED_DIGITS) {
    if (file) {
      IdlError(file, line, "Fixed point constant has too many digits");
    }
    *this = IDL_Fixed("1");
    return;
  }

  for (j=0; j < digits_; ++j, --i) {
    if (s[i] == '.') --i;
    val_[j] = s[i] - '0';
  }
  for (; j < OMNI_FIXED_DIGITS; ++j)
    val_[j] = 0;

  if (digits_ == 0) negative_ = 0;
}

IDL_Fixed::IDL_Fixed(const IDL_Octet* val, IDL_UShort digits,
		     IDL_UShort scale, IDL_Boolean negative)
  : digits_(digits), scale_(scale), negative_(negative)
{
  assert(digits <= OMNI_FIXED_DIGITS);
  assert(scale  <= digits);

  memcpy(val_, val, digits);
  memset(val_ + digits, 0, OMNI_FIXED_DIGITS - digits);
}


IDL_Fixed::~IDL_Fixed()
{
}


IDL_Fixed
IDL_Fixed::truncate(IDL_UShort scale)
{
  if (scale >= scale_)
    return *this;

  int cut = scale_ - scale;
  return IDL_Fixed(val_ + cut, digits_ - cut, scale, negative_);
}



IDL_Fixed&
IDL_Fixed::operator=(const IDL_Fixed& f)
{
  digits_   = f.digits_;
  scale_    = f.scale_;
  negative_ = f.negative_;
  memcpy(val_, f.val_, OMNI_FIXED_DIGITS);
  return *this;
}

IDL_Fixed
IDL_Fixed::operator-() const
{
  if (digits_ == 0) return *this;

  IDL_Fixed r(*this);
  r.negative_ = !r.negative_;
  return r;
}


char*
IDL_Fixed::asString() const
{
  int len = digits_ + 1;
  if (negative_)         ++len; // for '-'
  if (digits_ == scale_) ++len; // for '0'
  if (scale_ > 0)        ++len; // for '.'

  char* r = new char[len];
  int i = 0, j;

  if (negative_)         r[i++] = '-';
  if (digits_ == scale_) r[i++] = '0';

  for (j=digits_; j; ) {
    if (j-- == scale_)
      r[i++] = '.';
    r[i++] = val_[j] + '0';
  }
  r[i] = '\0';
  return r;
}




// Functions which do the real arithmetic


static int
absCmp(const IDL_Fixed& a, const IDL_Fixed& b)
{
  int c;
  c = (a.fixed_digits()-a.fixed_scale()) - (b.fixed_digits()-b.fixed_scale());
  if (c) return c;

  int ai, bi;
  ai = a.fixed_digits() - 1;
  bi = b.fixed_digits() - 1;

  while (ai >= 0 && bi >= 0) {
    c = a.val()[ai] - b.val()[bi];
    if (c) return c;
    --ai; --bi;
  }
  if (ai > 0) return  1;
  if (bi > 0) return -1;
  return 0;
}


static IDL_Fixed
realAdd(const IDL_Fixed& a, const IDL_Fixed& b, IDL_Boolean negative)
{
  int scale, v, carry = 0, ai = 0, bi = 0, wi = 0;
  IDL_Octet work[OMNI_FIXED_DIGITS * 2];

  if (a.fixed_scale() > b.fixed_scale()) {
    scale = a.fixed_scale();

    while (ai < a.fixed_scale() - b.fixed_scale())
      work[wi++] = a.val()[ai++];
  }
  else if (b.fixed_scale() > a.fixed_scale()) {
    scale = b.fixed_scale();

    while (bi < b.fixed_scale() - a.fixed_scale())
      work[wi++] = b.val()[bi++];
  }
  else
    scale = a.fixed_scale();

  while (ai < a.fixed_digits() && bi < b.fixed_digits()) {
    v = a.val()[ai++] + b.val()[bi++] + carry;
    if (v > 9) {
      carry = 1; v -= 10;
    }
    else carry = 0;
    work[wi++] = v;
  }
  while (ai < a.fixed_digits()) {
    v = a.val()[ai++] + carry;
    if (v > 9) {
      carry = 1; v -= 10;
    }
    else carry = 0;
    work[wi++] = v;
  }
  while (bi < b.fixed_digits()) {
    v = b.val()[bi++] + carry;
    if (v > 9) {
      carry = 1; v -= 10;
    }
    else carry = 0;
    work[wi++] = v;
  }
  if (carry) {
    work[wi++] = carry;
  }

  IDL_Octet* wp = work;
  int digits = wi;

  // Truncate or complain if too many digits
  if (digits > OMNI_FIXED_DIGITS) {
    if (digits - scale <= OMNI_FIXED_DIGITS) {
      int chop  = digits - OMNI_FIXED_DIGITS;
      wp       += chop;
      scale    -= chop;
      digits    = OMNI_FIXED_DIGITS;
    }
    else
      throw IDL_Fixed::Overflow();
  }

  // Strip trailing zeros
  while (scale > 0 && *wp == 0) {
    ++wp; --scale; --digits;
  }
  return IDL_Fixed(wp, digits, scale, negative);
}

static IDL_Fixed
realSub(const IDL_Fixed& a, const IDL_Fixed& b, IDL_Boolean negative)
{
  int scale, v, carry = 0, ai = 0, bi = 0, wi = 0;
  IDL_Octet work[OMNI_FIXED_DIGITS * 2];

  if (a.fixed_scale() > b.fixed_scale()) {
    scale = a.fixed_scale();

    while (ai < a.fixed_scale() - b.fixed_scale())
      work[wi++] = a.val()[ai++];
  }
  else if (b.fixed_scale() > a.fixed_scale()) {
    scale = b.fixed_scale();

    while (bi < b.fixed_scale() - a.fixed_scale()) {
      work[wi++] = 10 - b.val()[bi++] + carry;
      carry = -1;
    }
  }
  else
    scale = a.fixed_scale();

  while (ai < a.fixed_digits() && bi < b.fixed_digits()) {
    v = a.val()[ai++] - b.val()[bi++] + carry;
    if (v < 0) {
      carry = -1; v += 10;
    }
    else carry = 0;
    work[wi++] = v;
  }
  while (ai < a.fixed_digits()) {
    v = a.val()[ai++] + carry;
    if (v < 0) {
      carry = -1; v += 10;
    }
    else carry = 0;
    work[wi++] = v;
  }
  assert(bi = b.fixed_digits());
  assert(carry == 0);

  int digits = wi;
  IDL_Octet* wp = work;

  // Strip leading zeros
  while (work[digits-1] == 0 && digits > scale)
    --digits;

  // Truncate or complain if too many digits
  if (digits > OMNI_FIXED_DIGITS) {
    assert(digits - scale <= OMNI_FIXED_DIGITS);

    int chop  = digits - OMNI_FIXED_DIGITS;
    wp       += chop;
    scale    -= chop;
    digits    = OMNI_FIXED_DIGITS;
  }

  // Strip trailing zeros
  while (scale > 0 && *wp == 0) {
    ++wp; --scale; --digits;
  }

  return IDL_Fixed(wp, digits, scale, negative);
}


static IDL_Fixed
realMul(const IDL_Fixed& a, const IDL_Fixed& b, IDL_Boolean negative)
{
  int ai, bi, wi, digits, scale, v, ad, bd, carry = 0;
  IDL_Octet work[OMNI_FIXED_DIGITS * 2];

  memset(work, 0, OMNI_FIXED_DIGITS * 2);

  scale = a.fixed_scale() + b.fixed_scale();

  for (ai=0, wi=0; ai < a.fixed_digits(); ++ai) {
    ad = a.val()[ai];
    if (ad == 0) continue;

    for (bi=0; bi < b.fixed_digits(); ++bi) {
      bd = b.val()[bi];
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
  IDL_Octet* wp = work;
  if (digits > OMNI_FIXED_DIGITS) {
    if (digits - scale <= OMNI_FIXED_DIGITS) {
      int chop  = digits - OMNI_FIXED_DIGITS;
      wp       += chop;
      scale    -= chop;
      digits    = OMNI_FIXED_DIGITS;
    }
    else
      throw IDL_Fixed::Overflow();
  }

  // Strip trailing zeros
  while (scale > 0 && *wp == 0) {
    ++wp; --scale; --digits;
  }

  return IDL_Fixed(wp, digits, scale, negative);
}


static int
divCmp(const IDL_Octet* av, int ad, const IDL_Octet* bv, int bd, int pos)
{
  int c, ai, bi;

  for (ai = ad-1; ai > pos; --ai) {
    if (av[ai])
      return 1;
  }
  ai = pos;
  bi = bd - 1;
  assert(ai >= bi);
  while (bi >= 0) {
    c = av[ai] - bv[bi];
    if (c) return c;
    --ai; --bi;
  }
  return 0;
}


static int
divDigit(IDL_Octet* av, int ad, const IDL_Octet* bv, int bd, int pos)
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
  assert(count < 10);
  return count;
}


static IDL_Fixed
realDiv(const IDL_Fixed& a, const IDL_Fixed& b, IDL_Boolean negative)
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

  IDL_Octet work   [OMNI_FIXED_DIGITS * 2];
  IDL_Octet running[OMNI_FIXED_DIGITS * 2];

  memset(work,    0, OMNI_FIXED_DIGITS * 2);
  memset(running, 0, OMNI_FIXED_DIGITS * 2);

  // Skip all leading zeros in a
  ad = a.fixed_digits();
  while (a.val()[ad - 1] == 0) --ad;

  // Set most siginificant digits of running to a's digits
  ri = OMNI_FIXED_DIGITS * 2 - 1;
  ai = ad - 1;
  while (ai >= 0) {
    running[ri] = a.val()[ai];
    --ri;
    --ai;
  }

  // Skip all leading zeros in b
  bd = b.fixed_digits();
  while (b.val()[bd - 1] == 0) --bd;

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

    work[wi] = divDigit(running, OMNI_FIXED_DIGITS * 2, b.val(), bd, ri);

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
  if (unscale > OMNI_FIXED_DIGITS)
    throw IDL_Fixed::Overflow();

  // Deal with numbers with trailing zeros before the decimal point
  if (digits < unscale)
    digits = unscale;

  IDL_Octet* wp = &work[wi - digits + 1];

  // Figure out scale
  scale = digits - unscale;
  if (digits < scale)
    digits = scale;

  // Strip trailing zeros
  while (scale > 0 && *wp == 0) {
    ++wp; --scale; --digits;
  }

  return IDL_Fixed(wp, digits, scale, negative);
}


//
// Operators
//

IDL_Fixed
operator+(const IDL_Fixed& a, const IDL_Fixed& b)
{
  if (a.negative() == b.negative())
    return realAdd(a, b, a.negative());

  int cmp = absCmp(a, b);

  if (cmp == 0)     // a == b
    return IDL_Fixed();
  else if (cmp > 0) // a > b
    return realSub(a, b, a.negative());
  else
    return realSub(b, a, b.negative());
}

IDL_Fixed
operator-(const IDL_Fixed& a, const IDL_Fixed& b)
{
  if (a.negative() != b.negative())
    return realAdd(a, b, a.negative());

  int cmp = absCmp(a, b);

  if (cmp == 0)     // a == b
    return IDL_Fixed();
  else if (cmp > 0) // a > b
    return realSub(a, b, a.negative());
  else
    return realSub(b, a, !a.negative());
}

IDL_Fixed
operator*(const IDL_Fixed& a, const IDL_Fixed& b)
{
  if (a.fixed_digits() == 0 || b.fixed_digits() == 0)
    return IDL_Fixed();

  if (a.negative() == b.negative())
    return realMul(a, b, 0);
  else
    return realMul(a, b, 1);
}

IDL_Fixed
operator/(const IDL_Fixed& a, const IDL_Fixed& b)
{
  if (b.fixed_digits() == 0)
    throw IDL_Fixed::DivideByZero();

  if (a.fixed_digits() == 0)
    return IDL_Fixed();

  if (a.negative() == b.negative())
    return realDiv(a, b, 0);
  else
    return realDiv(a, b, 1);
}

