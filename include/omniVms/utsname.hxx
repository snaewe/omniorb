#ifndef utsname_hxx
#define utsname_hxx

#if defined(__VMS) && __VMS_VER < 70000000

//  provide uname functionality (note: only nodename is returned) for OpenVMS
//  systems older than 7.0

#include <errno.h>
#include <descrip.h>
#include <lib$routines.h>
#include <syidef.h>

struct utsname {
    char nodename[1024+1];   /*  DECnet node name                    */
};

static int uname (utsname* name) {

    const int nodeNameCode(SYI$_NODENAME);

    dsc$descriptor_s dx;
    dx.dsc$w_length = 1024;
    dx.dsc$b_dtype = DSC$K_DTYPE_T;
    dx.dsc$b_class = DSC$K_CLASS_S;
    dx.dsc$a_pointer = name->nodename;

    unsigned short length(0);

    vaxc$errno = lib$getsyi(
	&nodeNameCode,		// item-code
	0,			// resultant-value (ignored)
	&dx,			// resultant-string
	&length,		// resultant-length
	0,			// cluster-system-id
	0			// node-name
    );
    name->nodename[length]=0;

    int result = (vaxc$errno & 1) ? 0 : -1;
    if (result == -1) {
	errno = EVMSERR;
    }
    return result;
}

#else

#include <sys/utsname.h>

#endif

#endif
