// -*- Mode: C++; -*-
//                            Package   : omniORB
// omniORBcompat.h            Created on: 16/08/2001
//                            Author    : Sai Lai Lo (sll)
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
//      The following API is only for omniORB 2.x compatibility. Do
//      not use for new code.

#ifndef __OMNIORBCOMPAT_H__
#define __OMNIORBCOMPAT_H__

#ifndef _INCLUDE_OMNIORBCOMPAT_
#error "omniORBcompat.h must be included from omniORB.h"
#endif

  ////////////////////////////////////////////////////////////////////////
  //                                                                    //
  // This section is only for omniORB 2.x compatibility. Do not use for //
  // new code.                                                          //
  //                                                                    //
  // objectKey is a data type that uniquely identify each object        //
  //           implementation in the same address space. Its actual     //
  //           implmentation is not public. The data type should only   //
  //           be processed in an application by the following utility  //
  //           functions.                                               //
  //                                                                    //
  //                                                                    //
  typedef omniOrbBoaKey objectKey;                                      //
  //                                                                    //
  // generateNewKey()                                                   //
  //   generate a new key. The key is guaranteed to be temporally       //
  //   unique. On OSs that provide unique process IDs, e.g. unices,     //
  //   the key is guaranteed to be unique among all keys ever generated //
  //   on the same machine.                                             //
  _CORBA_MODULE_FN void generateNewKey(objectKey &k);                   //
  //                                                                    //
  // Return a fixed key value that always hash to 0.                    //
  _CORBA_MODULE_FN objectKey nullkey();                                 //
  //                                                                    //
  // Return non-zero if the keys are the same                           //
  _CORBA_MODULE_OP int operator==(const objectKey &k1,                  //
                                      const objectKey &k2);             //
  //                                                                    //
  // Return non-zero if the keys are different                          //
  _CORBA_MODULE_OP int operator!=(const objectKey &k1,                  //
                                      const objectKey &k2);             //
  //                                                                    //
  //                                                                    //
  typedef _CORBA_Unbounded_Sequence_Octet seqOctets;                    //
  // Convert a key to a sequence of octets.                             //
  _CORBA_MODULE_FN seqOctets* keyToOctetSequence(const objectKey &k1);  //
  //                                                                    //
  // Convert a sequence of octets back to an object key.                //
  // This function may throw a CORBA::MARSHAL exception if the sequence //
  // is not an object key.                                              //
  _CORBA_MODULE_FN objectKey octetSequenceToKey(const seqOctets& seq);  //
  ////////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////////
  //                                                                    //
  // This section is only for omniORB 2.x compatibility. Do not use for //
  // new code.                                                          //
  //                                                                    //
  // An application can register a handler for loading objects          //
  // dynamically. The handler should have the signature:                //
  //                                                                    //
  //          omniORB::loader::mapKeyToObject_t                         //
  //                                                                    //
  // When the ORB cannot locate the target object in this address space,//
  // it calls the handler with the object key of the target.            //
  // The handler is expected to instantiate the object, either in       //
  // this address space or in another address space, and returns the    //
  // object reference to the newly instantiated object. The ORB will    //
  // then reply with a LOCATION_FORWARD message to instruct the client  //
  // to retry using the object reference returned by the handler.       //
  // When the handler returns, the ORB assumes ownership of the         //
  // returned value. It will call CORBA::release() on the returned      //
  // value when it has finished with it.                                //
  //                                                                    //
  // The handler may be called concurrently by multi-threads. Hence it  //
  // must be thread-safe.                                               //
  //                                                                    //
  // If the handler cannot load the target object, it should return     //
  // CORBA::Object::_nil(). The object will be treated as non-existing. //
  //                                                                    //
  // The application registers the handler with the ORB at runtime      //
  // using omniORB::loader::set(). This function is not thread-safe.    //
  // Calling this function again will replace the old handler with      //
  // the new one.                                                       //
  //                                                                    //
  class loader {                                                        //
  public:                                                               //
    typedef CORBA::Object_ptr (*mapKeyToObject_t) (                     //
                                       const objectKey& key);           //
                                                                        //
    static void set(mapKeyToObject_t NewKeyToObject);                   //
  };                                                                    //
  ////////////////////////////////////////////////////////////////////////

#endif // __OMNIORBCOMPAT_H__
