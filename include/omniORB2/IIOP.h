// -*- Mode: C++; -*-
//                            Package   : omniORB2
// IIOP.h                     Created on: 8/2/96
//                            Author    : Sai Lai Lo (sll)
//
// Copyright (C) Olivetti Research Limited, 1996
//
// Description:
//       C++ mapping of the OMG IIOP module
//       Reference: CORBA V2.0 12.8.2
//	
//	

/*
  $Log$
  Revision 1.2  1997/03/09 12:18:43  sll
  Minor cleanup.

  Revision 1.1  1997/01/08 17:28:30  sll
  Initial revision

 */

#ifndef __IIOP_H__
#define __IIOP_H__

class IIOP {
  public:
 
  struct Version { 
    _CORBA_Char major;
    _CORBA_Char minor; 
  };
 
  // Current IIOP Protocol version
  static const _CORBA_Char current_major;
  static const _CORBA_Char current_minor;
 
  struct ProfileBody {
    Version                 iiop_version;
    _CORBA_Char*            host;
    _CORBA_UShort           port;
    _CORBA_Unbounded_Sequence_Octet  object_key;
  };

  // omniORB2 private functions
  static void profileToEncapStream(ProfileBody &,_CORBA_Unbounded_Sequence_Octet &);
  static void EncapStreamToProfile(const _CORBA_Unbounded_Sequence_Octet &,ProfileBody &);
};


#endif // __IIOP_H__
