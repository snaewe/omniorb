#include <omniORB3/CORBA.h>

#include "PollableSet.h"

#if 0
#include <sys/time.h>
#include <unistd.h>
#endif

// Problems:
//  1. Can only poll, with lots of RPCs
//  2. Adding to a set requires lots of _is_equivalent => lots of RPCs

PollableSet_polling::PollableSet_polling(): pd_number_left(0){ }

PollableSet_polling::~PollableSet_polling(){ }

void PollableSet_polling::add_pollable(CORBA::Pollable_ptr potential){
  omni_mutex_lock lock(pd_state_lock);
  
  // check to make sure its not already present
  for (CORBA::ULong x=0;x < pd_set.length();x++){
    CORBA::Pollable_ptr pollable = pd_set[x];
    if (pollable->_is_equivalent(potential)) return;
  }
  CORBA::ULong length = pd_set.length();
  pd_set.length(length + 1);
  pd_set[length] = CORBA::Pollable::_duplicate(potential);
  pd_number_left++;
}

void PollableSet_polling::remove(CORBA::Pollable_ptr potential){
  omni_mutex_lock lock(pd_state_lock);
  
  CORBA::Boolean found_one = 0;
  for (CORBA::ULong x=0;x < pd_set.length();x++)
    if (pd_set[x]->_is_equivalent(potential)){
      found_one = 1;
      CORBA::release(pd_set[x]);
      for (CORBA::ULong y=x+1;y < pd_set.length();y++)
	pd_set[y-1] = pd_set[y];
      pd_set.length(pd_set.length() - 1);
      pd_number_left--;
      return;
    }
  if (!found_one)
    throw CORBA::PollableSet::UnknownPollable();
}
  
CORBA::UShort PollableSet_polling::number_left(){
  CORBA::UShort return_val;
  {
    omni_mutex_lock lock(pd_state_lock);
    return_val = pd_number_left;
  }
  return return_val;
}

CORBA::Pollable_ptr PollableSet_polling::poll(CORBA::ULong timeout){
  if (pd_number_left <= 0) throw CORBA::PollableSet::NoPossiblePollable();
#if 0
  struct timeval starttime;
  gettimeofday(&starttime, NULL);

  while(1){
    struct timeval curtime;
    gettimeofday(&curtime, NULL);
    
    unsigned long time_waiting = (curtime.tv_usec - starttime.tv_usec) / 1000 +
      (curtime.tv_sec - starttime.tv_sec) * 1000;

    if (time_waiting > timeout) throw CORBA::TIMEOUT();

    // scan through each entry and check for events
    // (no thread safety here)
    for (CORBA::ULong x = 0; x < pd_set.length(); x++){
      CORBA::Pollable_ptr pollable = pd_set[x];
      if (pollable->is_ready(0)){
	// remove entry from the set
	for (CORBA::ULong y = x + 1; y < pd_set.length(); y++)
	  pd_set[y-1] = pd_set[y];
	pd_set.length(pd_set.length() - 1);
	pd_number_left--;
	return pollable;
      }
    }
    sleep(1);

  }
#endif
  return CORBA::Pollable::_nil();
}

CORBA::DIIPollable_ptr PollableSet_polling::create_dii_pollable(){
  return CORBA::DIIPollable::_nil();
}
