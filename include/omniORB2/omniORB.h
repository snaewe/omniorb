// -*- Mode: C++; -*-
//                            Package   : omniORB2
// omniORB.h                  Created on: 6/2/96
//                            Author    : Sai Lai Lo (sll)
//
// Copyright (C) Olivetti Research Limited, 1996
//
// Description:
//        This is the public API of omniORB2's extension to CORBA.
//      The API is intended to be used in application code.

/*
  $Log$
  Revision 1.4  1997/04/23 13:56:41  sll
  Added strictIIOP flag.

 * Revision 1.3  1997/03/26  17:38:42  ewc
 * Runtime converted to Win32 DLL
 *
 * Revision 1.2  1997/03/11  19:56:43  sll
 * keyToOctetSequence and octetSequenceToKey should be static. Fixed.
 *
 * Revision 1.1  1997/03/10  11:41:57  sll
 * Initial revision
 *
 */

#ifndef __OMNIORB_H__
#define __OMNIORB_H__

class _OMNIORB2_NTDLL_ omniORB {

public:

  ///////////////////////////////////////////////////////////////////////
  // Tracing level                                                     //
  //     level 0 - turn off all tracing and informational messages     //
  //     level 1 - informational messages only                         //
  //     level 2 - configuration information                           //
  //     level 5 - the above plus report server thread creation and    //
  //               communication socket shutdown                       //
  //     level 10 - the above plus execution trace messages            //
  //     ...                                                           //
  static _CORBA_ULong   traceLevel;                                    //
  //                                                                   //
  //     This value can be changed at runtime either by command-line   //
  //     option: -ORBtraceLevel <n>, or by direct assignment to this   //
  //     variable.                                                     //
  ///////////////////////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////////////////////
  // strictIIOP flag                                                   //
  //   Enable vigorous check on incoming IIOP messages                 //
  //                                                                   //
  // In some (sloppy) IIOP implementation, the message size value in   //
  // the header can be larger than the actual body size, i.e. there is //
  // garbage at the end. As the spec does not say the message size     //
  // must match the body size exactly, this is not a clear violation   //
  // of the spec.                                                      //
  //                                                                   //
  // If this flag is non-zero, the incoming message is expected to     //
  // be well behaved. Any messages that have garbage at the end will   //
  // be rejected.
  //
  // The default value of this flag is zero and the ORB would silently //
  // skip the unread part. The problem with this behaviour is that the //
  // header message size may actually be garbage value, caused by a bug//
  // in the sender's code. Th receiving thread may forever block on    //
  // the strand as it tries to read more data from it. In this case the//
  // sender won't send anymore as it thinks it has marshalled in all   //
  // the data.							       //
  static _CORBA_Boolean   strictIIOP;                                  //
  //                                                                   //
  //     This value can be changed at runtime either by command-line   //
  //     option: -ORBstrictIIOP <0|1>, or by direct assignment to this //
  //     variable.                                                     //
  ///////////////////////////////////////////////////////////////////////


  ////////////////////////////////////////////////////////////////////////
  // objectKey is a data type that uniquely identify each object        //
  //           implementation in the same address space. Its actual     //
  //           implmentation is not public. The data type should only   //
  //           be processed in an application by the following utility  //
  //           functions.                                               //
  //                                                                    //
  //                                                                    //
  typedef omniObjectKey objectKey;                                      //
  //                                                                    //
  //  size of the hash table used by hash().                            //
  static const unsigned int hash_table_size;                            //
  //                                                                    //
  //  hash()                                                            //
  //    return the hash value of this key. The return value             //
  //    is between 0 - (hash_table_size - 1);                           //
  static int hash(objectKey& k);                                        //
  //                                                                    //
  // generateNewKey()                                                   //
  //   generate a new key. The key is guaranteed to be temporally       //
  //   unique. On OSs that provide unique process IDs, e.g. unices,     //
  //   the key is guaranteed to be unique among all keys ever generated //
  //   on the same machine.                                             //
  static void generateNewKey(objectKey &k);                             //
  //                                                                    //
  // Return a fixed key value that always hash to 0.                    //
  static objectKey nullkey();                                           //
  //                                                                    //
  // Return non-zero if the keys are the same                           //
  friend int operator==(const objectKey &k1,const objectKey &k2);       //
  //                                                                    //
  // Return non-zero if the keys are different                          //
  friend int operator!=(const objectKey &k1,const objectKey &k2);       //
  //                                                                    //
  //                                                                    //
  typedef _CORBA_Unbounded_Sequence_Octet seqOctets;                    //
  // Convert a key to a sequence of octets.                             //
  static seqOctets* keyToOctetSequence(const objectKey &k1);            //
  //                                                                    //
  // Convert a sequence of octets back to an object key.                //
  // This function may throw a CORBA::MARSHAL exception if the sequence //
  // is not an object key.                                              //
  static objectKey octetSequenceToKey(const seqOctets& seq);             //
  ////////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////////
  // MaxMessageSize()                                                   //
  //                                                                    //
  // returns the ORB-wide limit on the size of GIOP message (excluding  //
  // the header).                                                       //
  //                                                                    //
  static size_t MaxMessageSize();                                       //
  ////////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////////
  // class fatalException                                               //
  //                                                                    //
  // This exception is thrown if a bug inside the omniORB2 runtime is   //
  // detected. The exact location in the source where the exception is  //
  // thrown is indicated by file() and line().                          //
  //                                                                    //
  class fatalException {                                                //
  public:                                                               //
    fatalException(const char *file,int line,const char *errmsg) {      //
      pd_file = file;                                                   //
      pd_line = line;                                                   //
      pd_errmsg = errmsg;                                               //
    }                                                                   //
    ~fatalException() {}                                                //
    const char *file() const { return pd_file; }                        //
    int line() const { return pd_line; }                                //
    const char *errmsg() const { return pd_errmsg; }                    //
  private:                                                              //
    const char *pd_file;                                                //
    int         pd_line;                                                //
    const char *pd_errmsg;                                              //
                                                                        //
    fatalException();                                                   //
  };                                                                    //
  ////////////////////////////////////////////////////////////////////////

  friend class omni;
private:
    static objectKey seed;
};

#endif // __OMNIORB_H__
