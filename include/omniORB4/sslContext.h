// -*- Mode: C++; -*-
//                            Package   : omniORB
// sslContext.h               Created on: 29 May 2001
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
//	*** PROPRIETORY INTERFACE ***
// 

/*
  $Log$
  Revision 1.1.2.1  2001/06/11 18:11:07  sll
  *** empty log message ***

*/

#ifndef __SSLCONTEXT_H__
#define __SSLCONTEXT_H__

#include <openssl/ssl.h>


OMNI_NAMESPACE_BEGIN(omni)
  class omni_sslTransport_initialiser;
OMNI_NAMESPACE_END(omni)

class sslContext {
 public:
  sslContext(const char* cafile, const char* keyfile, const char* password);

  SSL_CTX* get_SSL_CTX() const { return pd_ctx; }
  
  // These three parameters must be set or else the default way to
  // initialise a sslContext singleton will not be used.
  static const char* certificate_authority_file;
  static const char* key_file;
  static const char* key_file_password;

  static sslContext* singleton;

 protected:
  virtual SSL_METHOD* set_method();
  virtual void set_CA();
  virtual void set_certificate();
  virtual void set_cipher();
  virtual void set_privatekey();
  virtual void seed_PRNG();
  virtual void set_DH();
  virtual void set_ephemeralRSA();

  sslContext();

  friend class _OMNI_NS(omni_sslTransport_initialiser);
 private:

  virtual void internal_initialise();

  const char* pd_cafile;
  const char* pd_keyfile;
  const char* pd_password;
  SSL_CTX*  pd_ctx;
  
};

#endif // __SSLCONTEXT_H__
