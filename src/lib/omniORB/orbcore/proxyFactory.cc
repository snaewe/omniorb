// -*- Mode: C++; -*-
//                            Package   : omniORB
// proxyFactory.cc            Created on: 24/2/99
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 1996, 1997 Olivetti & Oracle Research Laboratory
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
//    Implementation of proxyObjectFactory.
//

/*
  $Log$
  Revision 1.2.2.3  2000/11/09 12:27:59  dpg1
  Huge merge from omni3_develop, plus full long long from omni3_1_develop.

  Revision 1.2.2.2  2000/09/27 17:58:56  sll
  Changed include/omniORB3 to include/omniORB4

  Revision 1.2.2.1  2000/07/17 10:35:58  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.3  2000/07/13 15:25:55  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.1.2.1  1999/09/22 14:27:05  djr
  Major rewrite of orbcore to support POA.

*/

#include <omniORB4/CORBA.h>

#ifdef HAS_pch
#pragma hdrstop
#endif

#include <omniORB4/proxyFactory.h>


static proxyObjectFactory** ofl = 0;
static int                  ofl_size = 0;
static int                  ofl_len = 0;


proxyObjectFactory::~proxyObjectFactory()  {}


proxyObjectFactory::proxyObjectFactory(const char* repoId)
{
  // These factories are constructed statically in the stubs, thus
  // there should be no possiblilty of concurrency.

  OMNIORB_ASSERT(repoId);

  pd_repoId = CORBA::string_dup(repoId);

  if( !ofl ) {
    ofl_size = 5;
    ofl = new proxyObjectFactory* [ofl_size];
    ofl_len = 0;
  }

  if( ofl_len == ofl_size ) {
    int new_ofl_size = ofl_size * 2;
    proxyObjectFactory** new_ofl = new proxyObjectFactory* [new_ofl_size];
    for( int i = 0; i < ofl_size; i++ )  new_ofl[i] = ofl[i];
    delete[] ofl;
    ofl = new_ofl;
    ofl_size = new_ofl_size;
  }

  // Binary search to determine the insertion point.

  int bottom = 0;
  int top = ofl_len;

  while( bottom < top ) {

    int middle = (bottom + top) / 2;

    int cmp = strcmp(repoId, ofl[middle]->pd_repoId);

    if( cmp < 0 )       top = middle;
    else if( cmp > 0 )  bottom = middle + 1;
    else {
      ofl[middle] = this;
      if( omniORB::trace(15) )
	  omniORB::logf("Replaced proxyObjectFactory for %s.", repoId);
      return;
    }
  }

  OMNIORB_ASSERT(top == bottom);

  for( int i = ofl_len; i > bottom; i-- )
    ofl[i] = ofl[i - 1];

  ofl[bottom] = this;
  ofl_len++;
}


void
proxyObjectFactory::shutdown()
{
  ofl_size = 0;
  ofl_len = 0;
  delete[] ofl;
}


proxyObjectFactory*
proxyObjectFactory::lookup(const char* repoId)
{
  // Factories should all be registered before the ORB is initialised,
  // so at this point the list is read-only. Concurrent accesses are
  // safe, except that the list is deleted when the ORB is shutdown.
  // There is a very small possibility that we will segfault below,
  // but that can only happen if the application is creating an object
  // reference at the same time as they are shutting down the ORB.

  OMNIORB_ASSERT(repoId);

  // Binary search to find the factory.

  int bottom = 0;
  int top = ofl_len;

  while( bottom < top ) {

    int middle = (bottom + top) / 2;

    int cmp = strcmp(repoId, ofl[middle]->pd_repoId);

    if( cmp < 0 )       top = middle;
    else if( cmp > 0 )  bottom = middle + 1;
    else                return ofl[middle];
  }

  return 0;
}