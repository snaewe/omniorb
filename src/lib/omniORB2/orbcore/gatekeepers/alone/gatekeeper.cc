
//-----------------------------------------------------------------------------
#ifdef __WIN32__

#	include <windows.h>

#else

#	include <stdio.h>
#	include <stdarg.h>
#	include <sys/types.h>
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <arpa/inet.h>
#	include <netdb.h>

#	if defined(__sunos__)
#		include <sysent.h>
#	endif //__sunos__

#include <unistd.h>

#endif // __WIN32__
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
extern "C" void exit(int) ;
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
#include <omniORB2/CORBA.h>
#include <gatekeeper.h>
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
CORBA::Boolean external_allowed = 0 ;
char *hosts_deny_table = (char*) "";
char *hosts_allow_table = (char*) "";
char * & gateKeeper::denyFile = hosts_deny_table ;
char * & gateKeeper::allowFile = hosts_allow_table ;
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
static void orb_log ( const char * fmt, ... ) {
//-----------------------------------------------------------------------------
	
#ifdef __WIN32__

/*	va_list al ;
	char tmp[2048] = "";
	char buffer[2048] = "";
	char * crt_src, * crt_tgt ;

	va_start ( al, fmt ) ;
	vsprintf ( buffer, fmt, al ) ;
	crt_src = tmp ;
	crt_tgt = buffer ;
	for ( ; *crt_src; *crt_src++ ) {
		if ( crt_src == '\n' ) {
			crt_tgt[0] == '\n' ; 
			crt_tgt[1] == '\r' ; 
			crt_tgt += 2 ;
			}
		else {
			*crt_tgt = *crt_src ;
			}
		}
	va_end ( al ) ;
	::MessageBox ( NULL, buffer, NULL, MB_ICONSTOP ) ; */
#	warning "WINDOWS CODE IS NOT COMPLETE HERE"
	
#else

	va_list al ;

	va_start ( al, fmt ) ;
	vfprintf ( stderr, fmt, al ) ;
	va_end ( al ) ;
	
#endif
	}


//-----------------------------------------------------------------------------
const char * gateKeeper::version() {
//-----------------------------------------------------------------------------
	return (const char *) "single host version";
	}



//-----------------------------------------------------------------------------
CORBA::Boolean gateKeeper::checkConnect( _tcpStrand *s) {
//-----------------------------------------------------------------------------
	static CORBA::Boolean gServerResolved = 0 ;
	static struct sockaddr_in gServerAddr ;
	static int	gServerAddrSize = sizeof(gServerAddr) ;
	static char gServerName[256] = "" ;
	static char gServerDotted[256] = "" ;

	struct sockaddr_in clientAddr ;
	int clientAddrSize = sizeof(clientAddr) ;
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
	if ( getpeername ( handle, (struct sockaddr*) (void *) & clientAddr, & clientAddrSize ) ) {			
		orb_log ( "Failed to authenticate peer name: access denied...\n" ) ;
		return 0 ;
		}
	host = gethostbyaddr ( (const char *) & clientAddr.sin_addr, sizeof(clientAddr.sin_addr), clientAddr.sin_family ) ;
	if ( ! host ) {
		orb_log ( "Failed to retrieve client host infos: access denied...\n" ) ;
		return 0 ;
		}
	strcpy ( clientName, host->h_name ) ;
	strcpy ( clientDotted, inet_ntoa ( *(struct in_addr *) host->h_addr_list[0] ) ) ;


	// ********************************
	// retrieve the official local name
	// ********************************
	if ( ! gServerResolved ) {
		if ( gethostname ( gServerName, sizeof(gServerName) ) ) {
			orb_log ( "Failed to retrieve our host name: access denied\n" ) ;
			return 0 ;
			}
		host = gethostbyname ( gServerName ) ;
		if ( ! host ) {
			orb_log ( "Failed to retrieve our host infos: access denied...\n" ) ;
			return 0 ;
			}
		strcpy ( gServerName, host->h_name ) ;
		strcpy ( gServerDotted, inet_ntoa ( *(struct in_addr *) host->h_addr_list[0] ) ) ;
		gServerResolved = 1 ;
		}


	// ********************************
	// compare the addresses
	// ********************************
	if ( strcmp ( gServerDotted, clientDotted ) ) {
		orb_log ( "%s: Access denied to %s (%s)\n", gServerName, clientName, clientDotted ) ;
		return 0 ;
		}

	return 1;
	}

//-----------------------------------------------------------------------------
