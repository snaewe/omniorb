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
/* Revision 1.3  1998/08/05 17:57:09  sll
/* Added comments to document the interface as much as I could.
/*
 * Revision 1.2  1998/04/07 19:38:25  sll
 * Replace cerr with omniORB::log.
 *
 * Revision 1.1  1998/01/27  15:47:18  ewc
 * Initial revision
 *
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
  // Pre-condition:
  //   1) The next element to unmarshal from the input data stream contains
  //      the value of the data type identified by typecode.
  //
  // Post-condition:
  //   1) The value of the data type has been extracted from the input
  //      stream buffer. 
  //
  // Parse the typecode and extract accordingly the value of the type from
  // the input stream and marshal the value into the output stream <outBuf>.
  // Return the current size of data in the output buffer <outBuf>.
  //
  // Note: this function has the side effect of removing data from the
  //       input data stream.
  //       This function can be called repeatedly to extract consecutive
  //       data elements from the input data stream.
  //
  // If reAlign == 1, the value of Any types in the data have to be re-parse
  //            to make sure the alignment is correct in the output buffer.
  // else
  //     do not re-parse the value of Any types.
  //
  // reAlign == 1 is obviously slower but is sure to marshal data with the
  // correct alignment.
  // More info from ewc on the use of the reAlign flag:
  //     It is used when marshalling Anys to GIOP messages. Essentially, 
  //     if the message up to the beginning of the Any is not 8-byte aligned, 
  //     and the Any contains a double (or potentially contains a double - 
  //     i.e. if it's a constructed type), the parser will be called to 
  //     marshal the byte stream stored by the Any in to the message. It
  //     must shift any doubles so that they are 8-byte aligned from 4 
  //     bytes into the Any's byte stream. This means that all of the doubles 
  //     will be 8-byte aligned when copied into the GIOP buffered stream. 
  //     (As far as I remember, the reason this is done in a seperate buffer
  //     is to work out the size without counstructing the message, and 
  //     potentially to cache the value (using a mutable buffered stream) for 
  //     performance improvements in the future (this could lead to quite a 
  //     significant performance improvement).
  

  static CORBA::Char* getUnionLabel(CORBA::TCKind tck, MemBufferedStream& buf,
				    CORBA::ULong& discrimLen);
  // Extract from the input data stream <buf> the union discriminator
  // value.  Return the value in a heap allocated buffer and the size in
  // bytes of the value in discrimLen.
  // The type of the discriminator is given in <tck>.
  //
  // NOTE: this function has the side effect of removing a data element from
  //       the input data stream.


  static CORBA::Char* getUnionLabel(CORBA::TCKind tck, NetBufferedStream& buf,
				    CORBA::ULong& discrimLen);
  // Same as the previous member function except that the input data
  // stream is a NetBufferedStream instead of a MemBufferedStream.

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

  CORBA::Boolean setByteOrder();
  // Pre-condition:
  //    1) The typecode contains a complex parameter list.
  //    2) The next element to unmarshal from pd_tmpParam is the byte
  //       order field of the encapsulation octet sequence that encode
  //       the complex parameter list.
  //
  // Post-condition:
  //    1) The MemBufferedStream pd_tmpParam will unmarshal data from the
  //       buffer stream according to the byte order of the encapsulation
  //       octet sequence.
  //    2) The next element to unmarshal from pd_tmpParam is the complex
  //       parameter list.
  //
  // NOTE: If the pre-conditions are not met, the result is unpredictable.

  CORBA::Boolean skipInitialStrings();
  // Pre-condition:
  //      Same as setByteOrder().
  //
  // Post-condition:
  //    1) The MemBufferedStream pd_tmpParam will unmarshal data from the
  //       buffer stream according to the byte order of the encapsulation
  //       octet sequence.
  //    2) Skip the first two string element in the complex parameter list.
  //
  // Note: If the first two elements of the complex parameter list are not
  //       strings, the result is unpredictable.
  //       See also the note in setByteOrder().


  void setTCKind(CORBA::ULong utck)
  // Pre-condition:
  //    1) The next element to unmarshal from pd_tmpParam is the parameter
  //       list of a typecode. The argument <utck> identify the kind of
  //       this typecode. The next element may or may not be at the begining
  //       of pd_tmpParam.
  //
  // Update the pd_tck member to the value of utck.
  //
  // Post-condition:
  //    1) The next time parseTC() is called, this instance will unmarshal
  //       data from the input buffer according to the kind of utck.
  //
  // NOTE: If the pre-condition is not met, the result is unpredictable.
  {
    if (utck > (CORBA::ULong) CORBA::tk_except) {
      if (utck == (CORBA::ULong) 0xffffffff && omniORB::traceLevel >= 10) {
	omniORB::log << "\nIndirection TypeCodes not yet handled.\n";
	omniORB::log.flush();
      }
      throw CORBA::BAD_TYPECODE(0,CORBA::COMPLETED_NO);
    }
    pd_tck = (CORBA::TCKind) utck;
  }

  inline void setTCPos(CORBA::ULong pos)
  // Set the internal pointer of pd_tmpParam to an offset of <pos> bytes
  // from the beginning of the stream. 
  {
    if (pos > (CORBA::ULong) pd_tmpParam.size())
      throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);
    pd_tmpParam.rewind_in_mkr();
    pd_tmpParam.skip(pos);
  }
  

  inline void unsetByteOrder(CORBA::Boolean bOrder) {
    // Post-condition:
    //    The MemBufferedStream pd_tmpParam will unmarshal data from the
    //    buffer using the new byte order <bOrder>.
    
    if ((CORBA::Boolean)pd_tmpParam.byteOrder() != bOrder) 
      pd_tmpParam.byteOrder((CORBA::Char)bOrder);
  }


 private:

  MemBufferedStream* pd_mInBuf;
  NetBufferedStream* pd_nInBuf;
  // Data buffer where input data come

  CORBA::Boolean pd_bufType;
  // 1 - buffer is a NetBufferedStream, pd_nInBuf points to the buffer
  // 0 - buffer is a MemBufferedStream, pd_mInBuf points to the buffer

  CORBA::TCKind pd_tck;
  // TCKind of the typecode

  MemBufferedStream pd_tmpParam;
  // A MemBufferedStream which shares the internal buffer of the typecode
  // passed as argument to the ctors.
  //
  // pd_tmpParam contains the parameter list of typecode.
  //         1. This may be empty.
  //         2. Contain one simply parameter list.
  //         3. A complex parameter list encoded in an encapsulation
  //            octet sequence.
  //         (ref. CORBA 2.0 table 12-2)


  CORBA::Boolean pd_reAlign;

  tcParseEngine();
};

#endif

