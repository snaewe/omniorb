// -*- Mode: C++; -*-
//                            Package   : omniORB2
// tcParseEngine.h            Created on: 21/10/97
//                            Author    : Eoin Carroll (ewc)
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
//      Header file for TypeCode parser


/* $Log$
/* Revision 1.1  1998/01/27 15:47:18  ewc
/* Initial revision
/*
 */

#ifndef __tcParseEngine_h__
#define __tcParseEngine_h__

#include <omniORB2/CORBA.h>

class tcParseEngine
{
 public:
  tcParseEngine(CORBA::TypeCode_ptr tc, MemBufferedStream* inBuf) : 
    pd_mInBuf(inBuf), pd_nInBuf((NetBufferedStream*) 0), pd_bufType(0),
    pd_tck(tc->pd_tck), pd_tmpParam(tc->pd_param,1), pd_reAlign(0)  { }

  tcParseEngine(CORBA::TypeCode_ptr tc, NetBufferedStream* inBuf) :  
    pd_mInBuf((MemBufferedStream*) 0), pd_nInBuf(inBuf), pd_bufType(1),
    pd_tck(tc->pd_tck), pd_tmpParam(tc->pd_param,1), pd_reAlign(0)   { }

  virtual ~tcParseEngine() { }

  CORBA::ULong parse(MemBufferedStream& outBuf, CORBA::Boolean reAlign = 0);

  static CORBA::Char* getUnionLabel(CORBA::TCKind tck, MemBufferedStream& buf,
				    CORBA::ULong& discrimLen);

  static CORBA::Char* getUnionLabel(CORBA::TCKind tck, NetBufferedStream& buf,
				    CORBA::ULong& discrimLen);
  protected:
  void parseTC(MemBufferedStream& outBuf) {
    if (pd_bufType) parseTCn(outBuf);
    else parseTCm(outBuf);
  }

  void parseTCn(MemBufferedStream& outBuf);
  void parseTCm(MemBufferedStream& outBuf);

  void parseAny(MemBufferedStream& outBuf);
  void parsePrincipal(MemBufferedStream& outBuf);
  void parseObjectRef(MemBufferedStream& outBuf);
  void parseStructExcept(MemBufferedStream& outBuf);
  void parseUnion(MemBufferedStream& outBuf);
  void parseEnum(MemBufferedStream& outBuf);
  void parseString(MemBufferedStream& outBuf);
  void parseSeqArray(MemBufferedStream& outBuf);
  void parseAlias(MemBufferedStream& outBuf);

#if !defined(NO_FLOAT)  
  void parseDouble(MemBufferedStream& outBuf);
#endif

  CORBA::Boolean skipInitialStrings();
  CORBA::Boolean setByteOrder();

  inline void setTCKind(CORBA::ULong utck) {
    if (utck > (CORBA::ULong) CORBA::tk_except) {
      if (utck == (CORBA::ULong) 0xffffffff && omniORB::traceLevel >= 10) 
	cerr << "\nIndirection TypeCodes not yet handled." << endl;
      throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
    }
    pd_tck = (CORBA::TCKind) utck;
  }

  inline void setTCPos(CORBA::ULong pos) {
    if (pos > (CORBA::ULong) pd_tmpParam.size())
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
    pd_tmpParam.rewind_in_mkr();
    pd_tmpParam.skip(pos);
  }
  

  inline void unsetByteOrder(CORBA::Boolean bOrder) {
    if (pd_tmpParam.byteOrder() != bOrder) pd_tmpParam.byteOrder(bOrder);
  }


 private:
  MemBufferedStream* pd_mInBuf;
  NetBufferedStream* pd_nInBuf;

  CORBA::Boolean pd_bufType;

  CORBA::TCKind pd_tck;
  MemBufferedStream pd_tmpParam;
  CORBA::Boolean pd_reAlign;

  tcParseEngine();
};

#endif

