//
// default.h
//

#ifndef __default_h__
#define __default_h__

// Local directory where the trader keeps its logs

#ifndef DEFAULT_LOGNAME
#define DEFAULT_LOGNAME "/project/omni/var/omniNames"
#endif

// Minimum idle period before the trader takes a checkpoint (15 mins)

#ifndef DEFAULT_IDLE_TIME_BTW_CHKPT
#define DEFAULT_IDLE_TIME_BTW_CHKPT  (15*60)
#endif

#endif
