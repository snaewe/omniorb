// -*- Mode: C++; -*-
//                            Package   : omniORB2
// relStream.h                Created on: 30/9/97
//                            Author    : Sai Lai Lo (sll)
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
//	*** PROPRIETORY INTERFACE ***
// 

/*
 $Log$
 Revision 1.2.6.3  2000/03/27 17:30:40  sll
 Changed access control to ctor.

 Revision 1.2.6.2  1999/09/25 17:00:20  sll
 Merged changes from omni2_8_develop branch.

 Revision 1.2.4.1  1999/09/21 20:37:16  sll
 -Simplified the scavenger code and the mechanism in which connections
  are shutdown. Now only one scavenger thread scans both incoming
  and outgoing connections. A separate thread do the actual shutdown.
 -omniORB::scanGranularity() now takes only one argument as there is
  only one scan period parameter instead of 2.
 -Trace messages in various modules have been updated to use the logger
  class.
 -ORBscanGranularity replaces -ORBscanOutgoingPeriod and
                                -ORBscanIncomingPeriod.

 Revision 1.2.6.1  1999/09/15 20:25:43  sll
 Make sure that receive never receive more than max_receive_buffer_size().

 Revision 1.2  1999/03/11 16:25:55  djr
 Updated copyright notice

 Revision 1.1  1997/12/09 18:43:16  sll
 Initial revision

*/

#ifndef __RELSTREAM_H__
#define __RELSTREAM_H__

class reliableStreamStrand : public Strand {

  // Do not use public virtual inheritance here or in the classes derived
  // from here.
  // The class derived from this one may throw an exception in its ctor
  // when a new connection cannot be opened. If we use public virtual here,
  // we would go right into a bug in MS VC++ 4.{1,2} and 5.0. The effect of
  // the bug is that the dtor of Strand would be called more than once and
  // all but the last one would be called with the *wrong* this pointer.

public:

  reliableStreamStrand(size_t buffer_size, Rope* r);

protected:
  virtual ~reliableStreamStrand();

public:
  size_t MaxMTU() const;
  Strand::sbuf receive(size_t size,CORBA::Boolean exactly,int align,
		       CORBA::Boolean startMTU=0);
  void giveback_received(size_t leftover);
  size_t max_receive_buffer_size();
  void receive_and_copy(Strand::sbuf b,CORBA::Boolean startMTU=0);
  void skip(size_t size,CORBA::Boolean startMTU=0);
  sbuf reserve_and_startMTU(size_t size, CORBA::Boolean exactly,
			    int align,CORBA::Boolean transmit=0,
			    CORBA::Boolean at_most_once=0);
  Strand::sbuf reserve(size_t size,CORBA::Boolean exactly,int align,
		       CORBA::Boolean transmit=0,CORBA::Boolean endMTU=0);
  void giveback_reserved(size_t leftover,CORBA::Boolean transmit=0,
			 CORBA::Boolean endMTU=0);
  size_t max_reserve_buffer_size();
  void reserve_and_copy(Strand::sbuf b,CORBA::Boolean transmit=0,
			CORBA::Boolean endMTU=0);

  virtual void real_shutdown() = 0;
  virtual void ll_send(void* buf,size_t sz) = 0;
  virtual size_t ll_recv(void* buf,size_t sz) = 0;

private:
  void transmit();
  void fetch(CORBA::ULong max=0);

  void    *pd_tx_buffer;
  void    *pd_tx_begin;
  void    *pd_tx_end;
  void    *pd_tx_reserved_end;

  void    *pd_rx_buffer;
  void    *pd_rx_begin;
  void    *pd_rx_end;
  void    *pd_rx_received_end;

  const    size_t pd_buffer_size;

  reliableStreamStrand();
};


#endif // __RELSTREAM_H__
