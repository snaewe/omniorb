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
  Revision 1.1.2.7  2004/02/11 15:44:53  dgrisby
  Hook to set SSL verify mode. Thanks Matthew Wood.

  Revision 1.1.2.6  2003/04/25 15:53:33  dgrisby
  Work around OpenSSL crypt() clash.

  Revision 1.1.2.5  2002/09/05 14:29:01  dgrisby
  Link force mechanism wasn't working with gcc.

  Revision 1.1.2.4  2002/02/25 11:17:11  dpg1
  Use tracedmutexes everywhere.

  Revision 1.1.2.3  2001/09/14 11:10:35  sll
  Do the right dllimport for win32.

  Revision 1.1.2.2  2001/09/13 15:36:00  sll
  Provide hooks to openssl for thread safety.
  Switched to select v2 or v3 methods but accept only v3 or tls v1 protocol.
  Added extra method set_supported_versions.

  Revision 1.1.2.1  2001/06/11 18:11:07  sll
  *** empty log message ***

*/

#ifndef __SSLCONTEXT_H__
#define __SSLCONTEXT_H__

#include <omniORB4/linkHacks.h>

OMNI_FORCE_LINK(omnisslTP);


#ifdef _core_attr
# error "A local CPP macro _core_attr has already been defined."
#endif

#if defined(_OMNIORB_SSL_LIBRARY)
#     define _core_attr
#else
#     define _core_attr _OMNIORB_NTDLL_IMPORT
#endif

#define crypt _openssl_broken_crypt
#include <openssl/ssl.h>
#undef crypt

OMNI_NAMESPACE_BEGIN(omni)
  class omni_sslTransport_initialiser;
OMNI_NAMESPACE_END(omni)

class sslContext {
 public:
  sslContext(const char* cafile, const char* keyfile, const char* password);

  SSL_CTX* get_SSL_CTX() const { return pd_ctx; }
  
  // These three parameters must be set or else the default way to
  // initialise a sslContext singleton will not be used.
  static _core_attr const char* certificate_authority_file; // In PEM format
  static _core_attr const char* key_file;                   // In PEM format
  static _core_attr const char* key_file_password;

  static _core_attr sslContext* singleton;

  virtual ~sslContext();

 protected:
  virtual SSL_METHOD* set_method(); 
  // Default to return SSLv23_method().

  virtual void set_supported_versions(); 
  // Default to SSL_CTX_set_options(ssL_ctx, SSL_OP_NO_SSLv2); That is
  // only accept SSL version 3 or TLS version 1.

  virtual void set_CA();
  // Default to read the certificates of the Certificate Authorities in the 
  // file named by the static member certificate_authority_file.

  virtual void set_certificate();
  // Default to read the certificate of this server from the file named
  // by the static member key_file. 

  virtual void set_cipher();
  // Default to call OpenSSL_add_all_algorithms().

  virtual void set_privatekey();
  // Default to read the private key of this server from the file named
  // by the statci member key_file. Notice that this file also contains
  // the server's certificate.

  virtual void seed_PRNG();
  // On systems that does not provide a /dev/urandom, default to provide
  // a seed for the PRNG using process ID and time of date. This is not
  // a very good seed cryptographically. Secure applications should definitely
  // override this method to provide a better seed.

  virtual void set_DH();

  virtual void set_ephemeralRSA();

  virtual int set_verify_mode();
  // Set the SSL verify mode.
  // Defaults to return SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT.

  sslContext();

  friend class _OMNI_NS(omni_sslTransport_initialiser);
 private:

  void thread_setup();
  void thread_cleanup();

  virtual void internal_initialise();

  const char* 	    pd_cafile;
  const char* 	    pd_keyfile;
  const char* 	    pd_password;
  SSL_CTX*    	    pd_ctx;
  omni_tracedmutex* pd_locks;
};

#endif // __SSLCONTEXT_H__
