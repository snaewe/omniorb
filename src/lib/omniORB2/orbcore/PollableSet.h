#include <omniORB3/CORBA.h>

// Problems:
//  1. Can only poll, with lots of RPCs
//  2. Adding to a set requires lots of _is_equivalent => lots of RPCs

class PollableSet_polling: public POA_CORBA::PollableSet{
 protected:
  // store the set as a sequence of object references
  CORBA::PollableSet::set_T pd_set;

  CORBA::ULong pd_number_left;
  omni_mutex pd_state_lock;

 public:
  PollableSet_polling();
  ~PollableSet_polling();

  void add_pollable(CORBA::Pollable_ptr potential);

  void remove(CORBA::Pollable_ptr potential);
  
  CORBA::UShort number_left();

  CORBA::Pollable_ptr poll(CORBA::ULong timeout);

  CORBA::DIIPollable_ptr create_dii_pollable();

};
