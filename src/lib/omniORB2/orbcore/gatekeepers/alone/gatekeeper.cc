// -*- Mode: C++; -*-
//                            Package   : omniORB
// gatekeeper.cc              Created on: 
//                            Author    : 
//
//    Copyright (C) 1999 AT&T Laboratories Cambridge
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

#ifdef __WIN32__

# include <windows.h>

#else

# include <stdio.h>
# include <stdarg.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <netdb.h>

#include <unistd.h>

#endif // __WIN32__

extern "C" void exit(int) ;

#include <omniORB2/CORBA.h>
#include <libcWrapper.h>
#include <gatekeeper.h>

#if defined(NEED_GETHOSTNAME_PROTOTYPE)
extern "C" int gethostname(char *name, int namelen);
#endif

CORBA::Boolean external_allowed = 0 ;
char *hosts_deny_table = (char*) "";
char *hosts_allow_table = (char*) "";
char * & gateKeeper::denyFile = hosts_deny_table ;
char * & gateKeeper::allowFile = hosts_allow_table ;


#define LOGMESSAGE(level,prefix,message) do {\
   if (omniORB::trace(level)) {\
     omniORB::logger log("gatekeeper " ## prefix ## ": ");\
	log << message ## "\n";\
   }\
} while (0)


//////////////////////////////////////////////////////////////////////////
const char *
gateKeeper::version() {
  return (const char *) "single host version";
}

//////////////////////////////////////////////////////////////////////////
CORBA::Boolean 
gateKeeper::checkConnect( _tcpStrand *s) {
  static CORBA::Boolean gServerResolved = 0 ;
  static struct sockaddr_in gServerAddr ;
  static int	gServerAddrSize = sizeof(gServerAddr) ;
  static char gServerName[256] = "" ;
  static char gServerDotted[256] = "" ;

  struct sockaddr_in clientAddr ;
  char clientDotted[256] = "" ;
  char clientName[256] = "" ;

  struct hostent * host ;
  unsigned int handle = s->handle() ;

  // ********************************
  // if no security, accepts connect.
  // ********************************
  if ( external_allowed )
    return 1 ;

  // ********************************
  // retrieve the client name
  // ********************************
#if (defined(__GLIBC__) && __GLIBC__ >= 2)
    // GNU C library uses socklen_t * instead of int* in getpeername().
    // This is suppose to be compatible with the upcoming POSIX standard.
    socklen_t clientAddrSize;
#elif defined(__aix__) || defined(__VMS) || defined(__SINIX__) || defined(__uw7__)
    size_t clientAddrSize;
# else
    int clientAddrSize;
# endif
  clientAddrSize = sizeof(clientAddr) ;

  if ( getpeername ( handle, 
		     (struct sockaddr*) (void *) & clientAddr, 
		     & clientAddrSize ) ) {
    LOGMESSAGE(1,"","Failed to authenticate peer name: access denied.");
    return 0 ;
  }
  host = gethostbyaddr ( (const char *) & clientAddr.sin_addr, 
			 sizeof(clientAddr.sin_addr), 
			 clientAddr.sin_family ) ;
  if ( ! host ) {
    LOGMESSAGE(1,"","Failed to retrieve client host infos: access denied." ) ;
    return 0 ;
  }
  strcpy ( clientName, host->h_name ) ;
  strcpy ( clientDotted, 
	   inet_ntoa ( *(struct in_addr *) host->h_addr_list[0] ) ) ;


  // ********************************
  // retrieve the official local name
  // ********************************
  if ( ! gServerResolved ) {
    if ( gethostname ( gServerName, sizeof(gServerName) ) ) {
      LOGMESSAGE(1,"","Failed to retrieve our host name: access denied") ;
      return 0 ;
    }

    LibcWrapper::hostent_var h;
    int rc;

    if (LibcWrapper::gethostbyname(gServerName,h,rc) < 0) {
      LOGMESSAGE(1,"","Failed to retrieve our host infos: access denied") ;
      return 0 ;
    }
    strcpy ( gServerName, h.hostent()->h_name ) ;
    strcpy ( gServerDotted,
	     inet_ntoa ( *(struct in_addr *) h.hostent()->h_addr_list[0] ) ) ;
    gServerResolved = 1 ;
  }


  // ********************************
  // compare the addresses
  // ********************************
  if ( strcmp ( gServerDotted, clientDotted ) ) {
    if (omniORB::trace(1)) {
      omniORB::logger log("gatekeeper : ");
      log << gServerName << " access denied to " 
	  << clientName << "(" << clientDotted << ")\n";
    }
    return 0 ;
  }

  return 1;
}


