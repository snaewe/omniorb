#ifndef unlink_hxx
#define unlink_hxx

#if defined(__VMS) && __VMS_VER < 70000000

//  provide unlink() functionality for VMS versions prior to 7.0

#include <errno.h>
#include <descrip.h>
#include <lib$routines.h>

static int unlink(char const* fileName) {

     dsc$descriptor_s dx;
     dx.dsc$w_length = strlen(fileName);
     dx.dsc$b_dtype = DSC$K_DTYPE_T;
     dx.dsc$b_class = DSC$K_CLASS_S;
     dx.dsc$a_pointer = (char*)fileName;	// VMS descriptors are not
						// const correct!
     vaxc$errno = lib$delete_file(&dx);
     int result = (vaxc$errno & 1) ? 0 : -1;
     if (result == -1) {
	errno = EVMSERR;
     }
     return result;

}
#endif	// VMS before 7.0

#endif
