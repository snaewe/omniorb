// -*- Mode: C++; -*-
//                            Package   : omniORB
// finalCleanup.h             Created on: 2001/09/12
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
//    Class definition for reference counting compilation units, so
//    things can be cleaned up at the last possible moment.
//

#ifndef __FINALCLEANUP_H__
#define __FINALCLEANUP_H__


class _omniFinalCleanup {
public:
  inline _omniFinalCleanup() {
    count++;
  }
  inline ~_omniFinalCleanup() {
    if (--count == 0)
      cleanup();
  }
  void cleanup();

private:
  static int count;
};

static _omniFinalCleanup _the_omniFinalCleanup;

#endif // __FINALCLEANUP_H__
