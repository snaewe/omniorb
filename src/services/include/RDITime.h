#ifndef _RDI_TIME_VALUE_H_
#define _RDI_TIME_VALUE_H_

#include <iostream.h>
#include <sys/time.h>
#include "RDIDefs.h"

// ------------------------------------------------------------------- //
// The following class supports absolute and relative time.  Increment //
// and decrement operations are provided  for constructing time points //
// starting from an absolute time.                                     //
//                                                                     //
// The format of the 'tspec' argument used in several members is:      //
//     time_spec: time_info [date_info]                                //
//     time_info: noon | now | midnight | HH:MM:SS:mm [am|pm]          //
//     date_info: day [month] [year] | mm/dd/yyyy                      //
//     day:       today | tomorrow | lastday                           //
//     month:     January - December | Jan - Dec                       //
//     year:      next | number                                        //
//                                                                     //
// The format of the 'ispec' argument used in 'increment' is:          //
//     incr_spec: [[+|-] number time_info]+                            //
//     time_info: year[s]   | month[s]  | week[s] | day[s] | hour[s] | //
//                min[ute[s]] | sec[ond[s]] | m[illi]sec[ond[s]]       //
//                                                                     //
// The format of the 'pspec' argument used in 'make_period' is:        //
//     perd_spec: HH:MM:SS:mm                                          //
// ------------------------------------------------------------------- //

class RDI_TimeValue {
public:
  RDI_TimeValue(long secs=0, long usecs=0)
		{ _tms.tv_sec=secs; _tms.tv_usec=usecs; }
  RDI_TimeValue(const RDI_TimeValue& tv) 
		{ _tms.tv_sec=tv._tms.tv_sec; _tms.tv_usec=tv._tms.tv_usec; }
  RDI_TimeValue(const struct timeval& tv)  
		{ _tms.tv_sec=tv.tv_sec; _tms.tv_usec=tv.tv_usec; }

  RDI_TimeValue& operator = (const RDI_TimeValue& tv)
		{ _tms.tv_sec  = tv._tms.tv_sec; 
		  _tms.tv_usec = tv._tms.tv_usec; return *this; }
  RDI_TimeValue& operator = (const struct timeval& tv)
		{ _tms.tv_sec  = tv.tv_sec;
		  _tms.tv_usec = tv.tv_usec; return *this; }

  // Create absolute time with respect to 'base' (null => current time).
  // If absolute time is a past time, the next valid time is constructed
  // when 'nvalid' is true. On error, the error is stored in 'ebuff', if
  // not NULL and RDI_TimeValue::null is returned

  static RDI_TimeValue make_time(const char* tspec, bool nvalid=false,
                                 char* ebuff=0, RDI_TimeValue base=null);

  // Increment time by 'ispec'.  On error, 'tval' is not modified, error
  // is stored in 'ebuff', if not NULL and -1 is returned (0 on success)

  static int increment(RDI_TimeValue& tval, const char* ispec, char* ebuff=0);

  // Construct a time period.  On error, RDI_TimeValue::null is returned

  static RDI_TimeValue make_period(const char* pspec);

  inline RDI_TimeValue& operator+=(const RDI_TimeValue& v);
  inline RDI_TimeValue& operator-=(const RDI_TimeValue& v);
  friend RDI_TimeValue  operator+ (const RDI_TimeValue& l,
				   const RDI_TimeValue& r)
		{ RDI_TimeValue v(l); v += r; return v; }

  inline friend int operator< (const RDI_TimeValue& l, const RDI_TimeValue& r);
  inline friend int operator> (const RDI_TimeValue& l, const RDI_TimeValue& r);
  inline friend int operator==(const RDI_TimeValue& l, const RDI_TimeValue& r);
  inline friend int operator!=(const RDI_TimeValue& l, const RDI_TimeValue& r);

  friend const RDI_TimeValue& max(const RDI_TimeValue& l,const RDI_TimeValue& r)
    		{ return (l > r) ? l : r; }
  friend const RDI_TimeValue& min(const RDI_TimeValue& l,const RDI_TimeValue& r)
    		{ return (l > r) ? r : l; }

  // Compute the absolute difference between two time points

  static RDI_TimeValue delta(const RDI_TimeValue& l, const RDI_TimeValue& r);

  // Compute the current time (seconds and microseconds since 01/01/1970)

  inline static RDI_TimeValue timeofday(void);

  inline bool expired(void) const;

  operator struct timeval  (void) const { return _tms;  }
  operator struct timeval* (void) const { return (struct timeval *) &_tms; }

  long secds(void) const                { return _tms.tv_sec;  }
  long usecs(void) const                { return _tms.tv_usec; }

  // Construct a string representation of the current time -- space of
  // 'buffer' should be be at least 24 chars

  void convert2string(char* buffer) const;

  friend ostream& operator << (ostream& out, const RDI_TimeValue& tv);

  static const RDI_TimeValue null;
private:
  struct timeval _tms;

  static char * parse_time(const char* tinfo, int& hours, int& mins, 
			   int& secs, int& msecs, char* ebuff=0);
};

// --------------------------------------------------------- //

inline RDI_TimeValue& RDI_TimeValue::operator += (const RDI_TimeValue& v)
{ _tms.tv_sec += v._tms.tv_sec; _tms.tv_usec += v._tms.tv_usec;
  _tms.tv_sec += _tms.tv_usec / 1000000; 
  _tms.tv_usec = _tms.tv_usec % 1000000; return *this; 
}

inline RDI_TimeValue& RDI_TimeValue::operator -= (const RDI_TimeValue& v) 
{
  if ( *this < v) 
	return *this; 	// do nothing if result would be negative time
  _tms.tv_sec -= v._tms.tv_sec;
  if (_tms.tv_usec < v._tms.tv_usec) {
    	_tms.tv_sec  -= 1;
	_tms.tv_usec += 1000000;
  }
  _tms.tv_usec -= v._tms.tv_usec;
  _tms.tv_sec  += _tms.tv_usec / 1000000; 
  _tms.tv_usec  = _tms.tv_usec % 1000000; return *this;
}

inline int operator <  (const RDI_TimeValue& lt, const RDI_TimeValue& rt)
{ return ( (lt._tms.tv_sec < rt._tms.tv_sec) ||
           	((lt._tms.tv_sec == rt._tms.tv_sec) && 
			(lt._tms.tv_usec < rt._tms.tv_usec)) ) ? 1 : 0; 
}

inline int operator >  (const RDI_TimeValue& lt, const RDI_TimeValue& rt)
{ return ( (lt._tms.tv_sec > rt._tms.tv_sec) || 
           	((lt._tms.tv_sec == rt._tms.tv_sec) && 
			(lt._tms.tv_usec > rt._tms.tv_usec)) ) ? 1 : 0; 
}

inline int operator == (const RDI_TimeValue& lt, const RDI_TimeValue& rt)
{ return ( (lt._tms.tv_sec == rt._tms.tv_sec) && 
		(lt._tms.tv_usec == rt._tms.tv_usec) ) ? 1 : 0; 
}

inline int operator != (const RDI_TimeValue& lt, const RDI_TimeValue& rt)
{ return ( (lt._tms.tv_sec != rt._tms.tv_sec) || 
		(lt._tms.tv_usec != rt._tms.tv_usec) ) ? 1 : 0; 
}

inline RDI_TimeValue RDI_TimeValue::timeofday()
{ RDI_TimeValue t; (void) gettimeofday(&t._tms, 0); return t; }

inline bool RDI_TimeValue::expired() const
{ RDI_TimeValue t = timeofday(); return (t < *this) ? false : true; }

#endif
