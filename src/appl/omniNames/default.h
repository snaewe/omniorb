//
// default.h
//

#ifndef __default_h__
#define __default_h__

// Local directory where the trader keeps its logs

#ifndef DEFAULT_LOGDIR

#ifdef __NT__
// Replace with the omniORB2 directory created by the install:
#define DEFAULT_LOGDIR "C:\\TEMP"
#else
#define DEFAULT_LOGDIR "/project/omni/var"
#endif

#endif

// Minimum idle period before the trader takes a checkpoint (15 mins)

#ifndef DEFAULT_IDLE_TIME_BTW_CHKPT
#define DEFAULT_IDLE_TIME_BTW_CHKPT  (15*60)
#endif

#endif
