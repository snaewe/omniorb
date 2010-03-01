// -*- Mode: C++; -*-
//                            Package   : omniORB
// sslContext.cc              Created on: 29 May 2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2003-2009 Apasphere Ltd
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
  Revision 1.1.4.6  2009/05/06 16:14:51  dgrisby
  Update lots of copyright notices.

  Revision 1.1.4.5  2008/02/14 13:50:03  dgrisby
  Initialise openssl only if necessary. Thanks Teemu Torma.

  Revision 1.1.4.4  2005/09/05 17:12:20  dgrisby
  Merge again. Mainly SSL transport changes.

  Revision 1.1.4.3  2005/03/30 23:35:59  dgrisby
  Another merge from omni4_0_develop.

  Revision 1.1.4.2  2005/01/06 23:10:53  dgrisby
  Big merge from omni4_0_develop.

  Revision 1.1.4.1  2003/03/23 21:01:59  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.12  2002/12/19 12:23:02  dgrisby
  Don't set SSL verify depth to 1.

  Revision 1.1.2.11  2002/12/19 11:49:33  dgrisby
  Vladimir Panov's SSL fixes.

  Revision 1.1.2.10  2002/04/16 12:44:27  dpg1
  Fix SSL accept bug, clean up logging.

  Revision 1.1.2.9  2002/02/25 11:17:14  dpg1
  Use tracedmutexes everywhere.

  Revision 1.1.2.8  2002/02/11 17:10:18  dpg1
  Cast result of pthread_self().

  Revision 1.1.2.7  2001/09/13 16:45:03  sll
  Changed thread id callback function for the openssl library.
  Only provide one for non-win32 platform and use pthread_self() directly.

  Revision 1.1.2.6  2001/09/13 15:36:01  sll
  Provide hooks to openssl for thread safety.
  Switched to select v2 or v3 methods but accept only v3 or tls v1 protocol.
  Added extra method set_supported_versions.

  Revision 1.1.2.5  2001/09/13 15:22:12  sll
  Correct test macro for WIN32.

  Revision 1.1.2.4  2001/08/03 17:41:25  sll
  System exception minor code overhaul. When a system exeception is raised,
  a meaning minor code is provided.

  Revision 1.1.2.3  2001/07/26 16:37:21  dpg1
  Make sure static initialisers always run.

  Revision 1.1.2.2  2001/06/20 18:53:34  sll
  Rearrange the declaration of for-loop index variable to work with old and
  standard C++.

  Revision 1.1.2.1  2001/06/11 18:11:06  sll
  *** empty log message ***

*/

#include <omniORB4/CORBA.h>

#include <stdlib.h>
#ifndef __WIN32__
#include <unistd.h>
#else
#include <process.h>
#endif
#include <sys/stat.h>
#include <omniORB4/minorCode.h>
#include <omniORB4/sslContext.h>
#include <exceptiondefs.h>
#include <ssl/sslTransportImpl.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <omniORB4/linkHacks.h>

OMNI_EXPORT_LINK_FORCE_SYMBOL(sslContext);

OMNI_USING_NAMESPACE(omni)

static void report_error();

const char* sslContext::certificate_authority_file = 0;
const char* sslContext::key_file = 0;
const char* sslContext::key_file_password = 0;
int         sslContext::verify_mode = (SSL_VERIFY_PEER |
				       SSL_VERIFY_FAIL_IF_NO_PEER_CERT);

sslContext* sslContext::singleton = 0;


/////////////////////////////////////////////////////////////////////////
sslContext::sslContext(const char* cafile,
		       const char* keyfile,
		       const char* password) :
  pd_cafile(cafile), pd_keyfile(keyfile), pd_password(password), pd_ctx(0),
  pd_locks(0), pd_ssl_owner(0) {}


/////////////////////////////////////////////////////////////////////////
sslContext::sslContext() :
  pd_cafile(0), pd_keyfile(0), pd_password(0), pd_ctx(0),
  pd_locks(0), pd_ssl_owner(0) {
}

/////////////////////////////////////////////////////////////////////////
void
sslContext::internal_initialise() {

  if (pd_ctx) return;

  // Assume we own the ssl if no locking callback yet.
  pd_ssl_owner = CRYPTO_get_locking_callback() == 0;

  if (pd_ssl_owner) {
    SSL_library_init();
    set_cipher();
    SSL_load_error_strings();
  }

  pd_ctx = SSL_CTX_new(set_method());
  if (!pd_ctx) {
    report_error();
    OMNIORB_THROW(INITIALIZE,INITIALIZE_TransportError,
		  CORBA::COMPLETED_NO);
  }

  static const unsigned char session_id_context[] = "omniORB";
  size_t session_id_len =
    (sizeof(session_id_context) >= SSL_MAX_SSL_SESSION_ID_LENGTH ?
     SSL_MAX_SSL_SESSION_ID_LENGTH : sizeof(session_id_context));

  if (SSL_CTX_set_session_id_context(pd_ctx,
				     session_id_context, session_id_len) != 1) {
    report_error();
    OMNIORB_THROW(INITIALIZE,INITIALIZE_TransportError,
		  CORBA::COMPLETED_NO);
  }

  set_supported_versions();
  seed_PRNG();
  set_certificate();
  set_privatekey();
  set_CA();
  set_DH();
  set_ephemeralRSA();
  // Allow the user to overwrite the SSL verification types.
  SSL_CTX_set_verify(pd_ctx,set_verify_mode(),NULL);
  if (pd_ssl_owner)
    thread_setup();
}

/////////////////////////////////////////////////////////////////////////
sslContext::~sslContext() {
  if (pd_ctx) {
    SSL_CTX_free(pd_ctx);
  }
  if (pd_ssl_owner)
    thread_cleanup();
}

/////////////////////////////////////////////////////////////////////////
SSL_METHOD*
sslContext::set_method() {
  return OMNI_CONST_CAST(SSL_METHOD*, SSLv23_method());
}

/////////////////////////////////////////////////////////////////////////
void
sslContext::set_supported_versions() {
  SSL_CTX_set_options(pd_ctx, SSL_OP_NO_SSLv2);
}

/////////////////////////////////////////////////////////////////////////
void
sslContext::set_CA() {

  {
    struct stat buf;
    if (!pd_cafile || stat(pd_cafile,&buf) < 0) {
      if (omniORB::trace(1)) {
	omniORB::logger log;
	log << "Error: sslContext CA file is not set "
	    << "or cannot be found\n";
      }
      OMNIORB_THROW(INITIALIZE,INITIALIZE_TransportError,
		    CORBA::COMPLETED_NO);
    }
  }

  if (!(SSL_CTX_load_verify_locations(pd_ctx,pd_cafile,0))) {
    report_error();
    OMNIORB_THROW(INITIALIZE,INITIALIZE_TransportError,CORBA::COMPLETED_NO);
  }

  // We no longer set the verify depth to 1, to use the default of 9.
  //  SSL_CTX_set_verify_depth(pd_ctx,1);

}

/////////////////////////////////////////////////////////////////////////
void
sslContext::set_certificate() {
  {
    struct stat buf;
    if (!pd_keyfile || stat(pd_keyfile,&buf) < 0) {
      if (omniORB::trace(5)) {
	omniORB::logger log;
	log << "sslContext certificate file is not set "
	    << "or cannot be found\n";
      }
      return;
    }
  }

  if(!(SSL_CTX_use_certificate_file(pd_ctx,pd_keyfile,SSL_FILETYPE_PEM))) {
    report_error();
    OMNIORB_THROW(INITIALIZE,INITIALIZE_TransportError,CORBA::COMPLETED_NO);
  }
}

/////////////////////////////////////////////////////////////////////////
void
sslContext::set_cipher() {
  OpenSSL_add_all_algorithms();
}

/////////////////////////////////////////////////////////////////////////
static const char* ssl_password = 0;

extern "C"
int sslContext_password_cb (char *buf,int num,int,void *) {
  int size = strlen(ssl_password);
  if (num < size+1) return 0;
  strcpy(buf,ssl_password);
  return size;
}

/////////////////////////////////////////////////////////////////////////
void
sslContext::set_privatekey() {

  if (!pd_password) {
    if (omniORB::trace(5)) {
      omniORB::logger log;
      log << "sslContext private key is not set\n";
    }
    return;
  }

  ssl_password = pd_password;
  SSL_CTX_set_default_passwd_cb(pd_ctx,sslContext_password_cb);
  if(!(SSL_CTX_use_PrivateKey_file(pd_ctx,pd_keyfile,SSL_FILETYPE_PEM))) {
    report_error();
    OMNIORB_THROW(INITIALIZE,INITIALIZE_TransportError,CORBA::COMPLETED_NO);
  }
}

/////////////////////////////////////////////////////////////////////////
void
sslContext::seed_PRNG() {
  // Seed the PRNG if it has not been done
  if (!RAND_status()) {

    // This is not necessary on systems with /dev/urandom. Otherwise, the
    // application is strongly adviced to seed the PRNG using one of the
    // seeding functions: RAND_seed(), RAND_add(), RAND_event() or
    // RAND_screen().
    // What we do here is a last resort and does not necessarily give a very
    // good seed!

    int* data = new int[256];

#if ! defined(__WIN32__)
    srand(getuid() + getpid());
#else
    srand(_getpid());
#endif
    int i;
    for(i = 0 ; i < 128 ; ++i)
      data[i] = rand();

    unsigned long abs_sec, abs_nsec;
    omni_thread::get_time(&abs_sec,&abs_nsec);
    srand(abs_sec + abs_nsec);
    for(i = 128 ; i < 256 ; ++i)
      data[i] = rand();

    RAND_seed((unsigned char *)data, (256 * (sizeof(int))));

    if (omniORB::trace(1)) {
      omniORB::logger log;
      log << "SSL: the pseudo random number generator has not been seeded.\n"
	  << "A seed is generated but it is not consided to be of crypto strength.\n"
	  << "The application should call one of the OpenSSL seed functions,\n"
	  << "e.g. RAND_event() to initialise the PRNG before calling sslTransportImpl::initialise().\n";
    }
  }
}

/////////////////////////////////////////////////////////////////////////
void
sslContext::set_DH() {

  DH* dh = DH_new();
  if(dh == 0) {
    OMNIORB_THROW(INITIALIZE,INITIALIZE_TransportError,CORBA::COMPLETED_NO);
  }

  unsigned char dh512_p[] = {
    0xDA,0x58,0x3C,0x16,0xD9,0x85,0x22,0x89,0xD0,0xE4,0xAF,0x75,
    0x6F,0x4C,0xCA,0x92,0xDD,0x4B,0xE5,0x33,0xB8,0x04,0xFB,0x0F,
    0xED,0x94,0xEF,0x9C,0x8A,0x44,0x03,0xED,0x57,0x46,0x50,0xD3,
    0x69,0x99,0xDB,0x29,0xD7,0x76,0x27,0x6B,0xA2,0xD3,0xD4,0x12,
    0xE2,0x18,0xF4,0xDD,0x1E,0x08,0x4C,0xF6,0xD8,0x00,0x3E,0x7C,
    0x47,0x74,0xE8,0x33
  };

  unsigned char dh512_g[] = {
    0x02
  };

  dh->p = BN_bin2bn(dh512_p, sizeof(dh512_p), 0);
  dh->g = BN_bin2bn(dh512_g, sizeof(dh512_g), 0);
  if( !dh->p || !dh->g) {
    OMNIORB_THROW(INITIALIZE,INITIALIZE_TransportError,CORBA::COMPLETED_NO);
  }

  SSL_CTX_set_tmp_dh(pd_ctx, dh);
  DH_free(dh);
}

/////////////////////////////////////////////////////////////////////////
void
sslContext::set_ephemeralRSA() {

  RSA *rsa;

  rsa = RSA_generate_key(512,RSA_F4,NULL,NULL);

  if (!SSL_CTX_set_tmp_rsa(pd_ctx,rsa)) {
    OMNIORB_THROW(INITIALIZE,INITIALIZE_TransportError,CORBA::COMPLETED_NO);
  }
  RSA_free(rsa);
}


/////////////////////////////////////////////////////////////////////////
int
sslContext::set_verify_mode() {
  return sslContext::verify_mode;
}


/////////////////////////////////////////////////////////////////////////
static omni_tracedmutex *openssl_locks = 0;

extern "C" 
void sslContext_locking_callback(int mode, int type, const char *,int) { 
  
  if (mode & CRYPTO_LOCK) {
    openssl_locks[type].lock();
  }
  else {
    OMNIORB_ASSERT(mode & CRYPTO_UNLOCK);
    openssl_locks[type].unlock();
  }
}

/////////////////////////////////////////////////////////////////////////
#ifndef __WIN32__
extern "C"
unsigned long sslContext_thread_id(void) {
  unsigned long id = (unsigned long)pthread_self();
  return id;
}
#endif

/////////////////////////////////////////////////////////////////////////
void
sslContext::thread_setup() {
  pd_locks = new omni_tracedmutex[CRYPTO_num_locks()];
  openssl_locks = pd_locks;
  CRYPTO_set_locking_callback(sslContext_locking_callback);
#ifndef __WIN32__
  CRYPTO_set_id_callback(sslContext_thread_id);
#endif
}

/////////////////////////////////////////////////////////////////////////
void
sslContext::thread_cleanup() {
  CRYPTO_set_locking_callback(NULL);
#ifndef __WIN32__
  CRYPTO_set_id_callback(NULL);
#endif
  if (pd_locks) {
    delete [] pd_locks;
    openssl_locks = 0;
  }
}

/////////////////////////////////////////////////////////////////////////
static void report_error() {

  if (omniORB::trace(1)) {
    char buf[128];
    ERR_error_string_n(ERR_get_error(),buf,128);
    omniORB::logger log;
    log << "sslContext.cc : " << (const char*) buf << "\n";
  }
}

