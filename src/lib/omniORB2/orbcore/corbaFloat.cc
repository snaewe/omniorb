// -*- Mode: C++; -*-
//                            Package   : omniORB2
// corbaFloat.cc              Created on: 23/04/98
//                            Author    :
//
//    Copyright (C) 1996-1999 AT&T Laboratories Cambridge
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
//	Float and Double conversion operators for OpenVMS.
//	This file is specific to OpenVMS.
//

/* 
   $Log$
   Revision 1.5.2.1  2000/07/17 10:35:51  sll
   Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

   Revision 1.6  2000/07/13 15:25:58  dpg1
   Merge from omni3_develop for 3.0 release.

   Revision 1.3  1999/03/11 16:25:52  djr
   Updated copyright notice

   Revision 1.2  1998/08/14 13:44:04  sll
   Added pragma hdrstop to control pre-compile header if the compiler feature
   is available.

   Revision 1.1  1998/08/11 16:39:41  sll
   Initial revision

*/

#include <omniORB2/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#if defined(__VMS) && !__IEEE_FLOAT

#include <cvtdef.h>
extern "C" unsigned int cvt$convert_float(...);

void _CORBA_Float::cvt_(float f) {
  int status(
    cvt$convert_float(
      &f,
      CVT$K_VAX_F,
      &pd_f,
      CVT$K_IEEE_S,
      CVT$M_ROUND_TO_NEAREST
    )
  );
  assert(status &1);
}

float _CORBA_Float::cvt_() const {
  float f;
  int status(
    cvt$convert_float(
      &pd_f,
      CVT$K_IEEE_S,
      &f,
      CVT$K_VAX_F,
      CVT$M_ROUND_TO_NEAREST
    )
  );
  assert(status & 1);
  return f;
}

#if __D_FLOAT
    enum {NativeDoubleFmt = CVT$K_VAX_D};
#elif __G_FLOAT
    enum {NativeDoubleFmt = CVT$K_VAX_G};
#else
#error "This module must be compiled with /Float=D or G."
#endif  // D vs G float

void _CORBA_Double::cvt_(double d) {
  int status(
    cvt$convert_float(
      &d,
      NativeDoubleFmt,
      &pd_d,
      CVT$K_IEEE_T,
      CVT$M_ROUND_TO_NEAREST
    )
  );
  assert(status &1);
}

double _CORBA_Double::cvt_() const {
  double d;
  int status(
    cvt$convert_float(
      &pd_d,
      CVT$K_IEEE_T,
      &d,
      NativeDoubleFmt,
      CVT$M_ROUND_TO_NEAREST
    )
  );
  assert(status & 1);
  return d;
}

#elif defined(__VMS) && __IEEE_FLOAT

void _CORBA_Float::cvt_(float f) {
  memcpy(&pd_f, &f, sizeof(pd_f));
}

float _CORBA_Float::cvt_() const {
  float f;
  memcpy(&f, &pd_f, sizeof(f));
  return f;
}

void _CORBA_Double::cvt_(double d) {
  memcpy(&pd_d, &d, sizeof(pd_d));
}

double _CORBA_Double::cvt_() const {
  double d;
  memcpy(&d, &pd_d, sizeof(d));
  return d;
}

#endif

_CORBA_Float::_CORBA_Float() {
  cvt_(0.0);
}

_CORBA_Float::_CORBA_Float(float f) {
  cvt_(f);
}

_CORBA_Float::operator float() const {
  return cvt_();
}

_CORBA_Double::_CORBA_Double() {
  cvt_(0.0);
}

_CORBA_Double::_CORBA_Double(double d) {
  cvt_(d);
}

_CORBA_Double::operator double() const {
  return cvt_();
}
