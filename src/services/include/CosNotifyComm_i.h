// -*- Mode: C++; -*-

#ifndef _COS_NOTIFY_COMM_I_H_
#define _COS_NOTIFY_COMM_I_H_

#include "CosNotifyComm.hh"

// ------------------------------------------------------------- //
// Here, we define the implementation of the client skeletons so //
// that a "default" library can be provided to applications.  In //
// most cases, applications will customize these implementations //
// using inheritance.                                            //
// ------------------------------------------------------------- //

typedef CosNotification::EventType       _EventType;
typedef CosNotification::EventTypeSeq    _EventTypeSeq;
typedef CosNotification::StructuredEvent _StructuredEvent;
typedef CosNotification::EventBatch      _EventBatch;

// ------------------------------------------------------------- //

/** The simplest push consumer -- it accepts CORBA::Any events
  *
  */

class PushConsumer_i : 
	public virtual CosNotifyComm::_sk_PushConsumer {
public:
  PushConsumer_i() : num_push(0) {;}

  inline void push(const CORBA::Any & data);
  inline void disconnect_push_consumer();
  inline void offer_change(const _EventTypeSeq& added,
                           const _EventTypeSeq& deled);
protected:
  CORBA::ULong num_push;
};

inline void PushConsumer_i::push(const CORBA::Any & data)
{ cout << "PushConsumer: push() called " << ++num_push << " times" << endl; }

inline void PushConsumer_i::disconnect_push_consumer()
{ cout << "PushConsumer: disconnected" << endl; }

inline void PushConsumer_i::offer_change(const _EventTypeSeq& added,
				         const _EventTypeSeq& deled)
{
  CORBA::ULong indx;
  _EventType   type;

  cout << "PushConsumer: offer_change contains: " << endl;
  for (indx = 0; indx < added.length(); indx++) {
        type = added[indx];
        cout << "\t+ " << type.domain_name << "::" << type.type_name << endl;
  }
  for (indx = 0; indx < deled.length(); indx++) {
        type = deled[indx];
        cout << "\t- " << type.domain_name << "::" << type.type_name << endl;
  }
}

/**
  *
  */

class PullConsumer_i :
	public virtual CosNotifyComm::_sk_PullConsumer {
public:
  PullConsumer_i() {;}

  inline void disconnect_pull_consumer();
  inline void offer_change(const _EventTypeSeq& added,
			   const _EventTypeSeq& deled);
};

inline void PullConsumer_i::disconnect_pull_consumer()
{ cout << "PullConsumer: disconnected" << endl; }

inline void PullConsumer_i::offer_change(const _EventTypeSeq& added,
                                         const _EventTypeSeq& deled)
{
  CORBA::ULong indx;
  _EventType   type;

  cout << "PullConsumer: offer_change contains: " << endl;
  for (indx = 0; indx < added.length(); indx++) {
        type = added[indx];
        cout << "\t+ " << type.domain_name << "::" << type.type_name << endl;
  }
  for (indx = 0; indx < deled.length(); indx++) {
        type = deled[indx];
        cout << "\t- " << type.domain_name << "::" << type.type_name << endl;
  }
}

/**
  *
  */

class PullSupplier_i :
	public virtual CosNotifyComm::_sk_PullSupplier {
public:
  PullSupplier_i() : event_value(0) {;}
 
  inline CORBA::Any* pull();
  inline CORBA::Any* try_pull(CORBA::Boolean& has_event);
  inline void        disconnect_pull_supplier();
  inline void        subscription_change(const _EventTypeSeq& added,
                                  	 const _EventTypeSeq& deled);
protected:
  CORBA::ULong event_value;
};

inline CORBA::Any* PullSupplier_i::pull()
{ 
  CORBA::Any *any = new CORBA::Any();
  *any <<= ++event_value;
  cout << "PullSupplier: pull() called for event " << event_value << endl;
  return any;
}

inline CORBA::Any* PullSupplier_i::try_pull(CORBA::Boolean& has_event)
{
  CORBA::Any *any = new CORBA::Any();
  *any <<= ++event_value;
  cout << "PullSupplier: try_pull() called for event " << event_value << endl;
  has_event = 1;
  return any;
}

inline void PullSupplier_i::disconnect_pull_supplier()
{ cout << "PullSupplier: disconnected" << endl; }

inline void PullSupplier_i::subscription_change(const _EventTypeSeq& added,
					        const _EventTypeSeq& deled)
{
  CORBA::ULong indx;
  _EventType   type;

  cout << "PullSupplier: subscription_change contains: " << endl;
  for (indx = 0; indx < added.length(); indx++) {
        type = added[indx];
        cout << "\t+ " << type.domain_name << "::" << type.type_name << endl;
  }
  for (indx = 0; indx < deled.length(); indx++) {
        type = deled[indx];
        cout << "\t- " << type.domain_name << "::" << type.type_name << endl;
  }
}					

/**
  *
  */

class PushSupplier_i :
	public virtual CosNotifyComm::_sk_PushSupplier {
public:
  PushSupplier_i() {;}

  inline void disconnect_push_supplier();
  inline void subscription_change(const _EventTypeSeq& added,
                                  const _EventTypeSeq& deled);
};

inline void PushSupplier_i::disconnect_push_supplier()
{ cout << "PushSupplier: disconnected" << endl; }

inline void PushSupplier_i::subscription_change(const _EventTypeSeq& added,
					 	const _EventTypeSeq& deled)
{
  CORBA::ULong indx;
  _EventType   type;

  cout << "PushSupplier: subscription_change contains: " << endl;
  for (indx = 0; indx < added.length(); indx++) {
        type = added[indx];
        cout << "\t+ " << type.domain_name << "::" << type.type_name << endl;
  }
  for (indx = 0; indx < deled.length(); indx++) {
        type = deled[indx];
        cout << "\t- " << type.domain_name << "::" << type.type_name << endl;
  }
}

// ------------------------------------------------------------- //

/**
  *
  */

class StructuredPushConsumer_i :
	public virtual CosNotifyComm::_sk_StructuredPushConsumer {
public:
  StructuredPushConsumer_i() : num_push(0) {;}

  inline void push_structured_event(const _StructuredEvent& event);
  inline void disconnect_structured_push_consumer();
  inline void offer_change(const _EventTypeSeq& added,
                           const _EventTypeSeq& deled);
protected:
  CORBA::ULong num_push;
};

inline void StructuredPushConsumer_i::push_structured_event(
				const _StructuredEvent& event)
{ cout << "StructuredPushConsumer: push() called " << 
	   ++num_push << " times" << endl; 
}

inline void StructuredPushConsumer_i::disconnect_structured_push_consumer()
{ cout << "StructuredPushConsumer: disconnected" << endl; }

inline void StructuredPushConsumer_i::offer_change(const _EventTypeSeq& added,
					    	   const _EventTypeSeq& deled)
{
  CORBA::ULong indx;
  _EventType   type;

  cout << "StructuredPushConsumer: offer_change contains: " << endl;
  for (indx = 0; indx < added.length(); indx++) {
        type = added[indx];
        cout << "\t+ " << type.domain_name << "::" << type.type_name << endl;
  }
  for (indx = 0; indx < deled.length(); indx++) {
        type = deled[indx];
        cout << "\t- " << type.domain_name << "::" << type.type_name << endl;
  }
}

/**
  *
  */

class StructuredPullConsumer_i :
	public virtual CosNotifyComm::_sk_StructuredPullConsumer {
public:
  StructuredPullConsumer_i() {;}
  inline void disconnect_structured_pull_consumer();
  inline void offer_change(const _EventTypeSeq& added,
			   const _EventTypeSeq& deled);
};

inline void StructuredPullConsumer_i::disconnect_structured_pull_consumer()
{ cout << "StructuredPullConsumer: disconnected" << endl; }

inline void StructuredPullConsumer_i::offer_change(const _EventTypeSeq& added,
						   const _EventTypeSeq& deled)
{
  CORBA::ULong indx;
  _EventType   type;

  cout << "StructuredPullConsumer: offer_change contains: " << endl;
  for (indx = 0; indx < added.length(); indx++) {
        type = added[indx];
        cout << "\t+ " << type.domain_name << "::" << type.type_name << endl;
  }
  for (indx = 0; indx < deled.length(); indx++) {
        type = deled[indx];
        cout << "\t- " << type.domain_name << "::" << type.type_name << endl;
  }
}

/**
  *
  */

class StructuredPullSupplier_i :
        public virtual CosNotifyComm::_sk_StructuredPullSupplier {
public:
  inline StructuredPullSupplier_i();

  inline _StructuredEvent* pull_structured_event();
  inline _StructuredEvent* try_pull_structured_event(CORBA::Boolean& has_event);
  inline void              disconnect_structured_pull_supplier();
  inline void              subscription_change(const _EventTypeSeq& added,
					       const _EventTypeSeq& deled);
protected:
  CORBA::ULong num_events;
  _EventType   event_type;
};

inline StructuredPullSupplier_i::StructuredPullSupplier_i() : num_events(0)
{ event_type.domain_name = CORBA::string_dup("DummyDomain");
  event_type.type_name   = CORBA::string_dup("DummyType");
}

inline _StructuredEvent* StructuredPullSupplier_i::pull_structured_event()
{
  _StructuredEvent* event = new _StructuredEvent();
  event->header.fixed_header.event_type.domain_name = event_type.domain_name;
  event->header.fixed_header.event_type.type_name   = event_type.type_name;
  event->header.variable_header.length(0);
  event->filterable_data.length(0);
  event->remainder_of_body <<= ++num_events;
  cout << "StructuredPullSupplier_i: pull_structured_event() called" << endl;
  return event;
}

inline _StructuredEvent* StructuredPullSupplier_i::try_pull_structured_event(
					CORBA::Boolean& has_event)
{
  _StructuredEvent* event = new _StructuredEvent();
  event->header.fixed_header.event_type.domain_name = event_type.domain_name; 
  event->header.fixed_header.event_type.type_name   = event_type.type_name;
  event->header.variable_header.length(0);
  event->filterable_data.length(0);
  event->remainder_of_body <<= ++num_events;
  has_event = 1;
  cout << "StructuredPullSupplier_i: try_pull_structured_event() called"<< endl;
  return event;
}

inline void StructuredPullSupplier_i::disconnect_structured_pull_supplier()
{ cout << " StructuredPullSupplier: disconnected" << endl; }

inline void StructuredPullSupplier_i::subscription_change(
					const _EventTypeSeq& added,
					const _EventTypeSeq& deled)
{
  CORBA::ULong indx;
  _EventType   type;

  cout << "StructuredPullSupplier: subscription_change contains: " << endl;
  for (indx = 0; indx < added.length(); indx++) {
        type = added[indx]; 
        cout << "\t+ " << type.domain_name << "::" << type.type_name << endl;
  }
  for (indx = 0; indx < deled.length(); indx++) {
        type = deled[indx];
        cout << "\t- " << type.domain_name << "::" << type.type_name << endl;
  }
}

/**
  *
  */

class StructuredPushSupplier_i :
        public virtual CosNotifyComm::_sk_StructuredPushSupplier {
public:
  StructuredPushSupplier_i()	{;}
  inline void disconnect_structured_push_supplier();
  inline void subscription_change(const _EventTypeSeq& added,
				  const _EventTypeSeq& deled);
};

inline void StructuredPushSupplier_i::disconnect_structured_push_supplier()
{ cout << "StructuredPushSupplier: disconnected" << endl; }

inline void StructuredPushSupplier_i::subscription_change(
					const _EventTypeSeq& added,
					const _EventTypeSeq& deled)
{
  CORBA::ULong indx;
  _EventType   type;

  cout << "StructuredPushSupplier: subscription_change contains: " << endl;
  for (indx = 0; indx < added.length(); indx++) {
        type = added[indx]; 
        cout << "\t+ " << type.domain_name << "::" << type.type_name << endl;
  }
  for (indx = 0; indx < deled.length(); indx++) {
        type = deled[indx];
        cout << "\t- " << type.domain_name << "::" << type.type_name << endl;
  }
}

// ------------------------------------------------------------- //

/**
  *
  */

class SequencePushConsumer_i :
        public virtual CosNotifyComm::_sk_SequencePushConsumer {
public:
  SequencePushConsumer_i() : num_batches(0), num_events(0) {;}

  inline void push_structured_events(const _EventBatch& events);
  inline void disconnect_sequence_push_consumer();
  inline void offer_change(const _EventTypeSeq& added,
			   const _EventTypeSeq& deled);
protected:
  CORBA::ULong num_batches;
  CORBA::ULong num_events;
};

inline void SequencePushConsumer_i::push_structured_events(
					const _EventBatch& events)
{ 
  num_batches += 1;
  num_events  += events.length();
  cout << "SequencePushConsumer: " << num_batches << " batches with " <<
	  num_events << " events so far" << endl;
}

inline void SequencePushConsumer_i::disconnect_sequence_push_consumer()
{ cout << "SequencePushConsumer: disconnected" << endl; }

inline void SequencePushConsumer_i::offer_change(const _EventTypeSeq& added,
					  	 const _EventTypeSeq& deled)
{
  CORBA::ULong indx;
  _EventType   type;

  cout << "SequencePushConsumer: offer_change contains: " << endl;
  for (indx = 0; indx < added.length(); indx++) {
        type = added[indx];
        cout << "\t+ " << type.domain_name << "::" << type.type_name << endl; 
  }
  for (indx = 0; indx < deled.length(); indx++) {
        type = deled[indx];
        cout << "\t- " << type.domain_name << "::" << type.type_name << endl;
  }
}

/**
  *
  */

class SequencePullConsumer_i :
        public virtual CosNotifyComm::_sk_SequencePullConsumer {
public:
  SequencePullConsumer_i() {;}

  inline void disconnect_sequence_pull_consumer();
  inline void offer_change(const _EventTypeSeq& added,
			   const _EventTypeSeq& deled);
};

inline void SequencePullConsumer_i::disconnect_sequence_pull_consumer()
{ cout << "SequencePullConsumer: disconnected" << endl; }

inline void SequencePullConsumer_i::offer_change(const _EventTypeSeq& added,
					  	 const _EventTypeSeq& deled)
{
  CORBA::ULong indx;
  _EventType   type;

  cout << "SequencePullConsumer: offer_change contains: " << endl;
  for (indx = 0; indx < added.length(); indx++) {
        type = added[indx];
        cout << "\t+ " << type.domain_name << "::" << type.type_name << endl;
  }
  for (indx = 0; indx < deled.length(); indx++) {
        type = deled[indx];
        cout << "\t- " << type.domain_name << "::" << type.type_name << endl;
  }
}

/**
  *
  */

class SequencePullSupplier_i :
        public virtual CosNotifyComm::_sk_SequencePullSupplier {
public:
  inline SequencePullSupplier_i();

  inline _EventBatch* pull_structured_events(CORBA::ULong max_number);
  inline _EventBatch* try_pull_structured_events(CORBA::ULong max_number,
						 CORBA::Boolean& has_event);
  inline void         disconnect_sequence_pull_supplier();
  inline void         subscription_change(const _EventTypeSeq& added,
					  const _EventTypeSeq& deled);
protected:
  CORBA::ULong num_events;
  _EventType   event_type;
};

inline SequencePullSupplier_i::SequencePullSupplier_i() : num_events(0)
{ event_type.domain_name = CORBA::string_dup("DummyDomain");
  event_type.type_name   = CORBA::string_dup("DummyType");
}

inline _EventBatch* SequencePullSupplier_i::pull_structured_events(
					CORBA::ULong max_number)
{
  _EventBatch*     batch = new _EventBatch;
  _StructuredEvent event;
  event.header.fixed_header.event_type.domain_name = event_type.domain_name; 
  event.header.fixed_header.event_type.type_name   = event_type.type_name;
  event.header.variable_header.length(0);
  event.filterable_data.length(0);
  event.remainder_of_body <<= ++num_events;
  batch->length(1);
  (*batch)[0] = event;
  cout << "SequencePullSupplier_i: pull_structured_events() called" << endl;
  return batch;
}

inline _EventBatch* SequencePullSupplier_i::try_pull_structured_events(
					CORBA::ULong max_number,
					CORBA::Boolean& has_event)
{
  _EventBatch*     batch = new _EventBatch;
  _StructuredEvent event;
  event.header.fixed_header.event_type.domain_name = event_type.domain_name; 
  event.header.fixed_header.event_type.type_name   = event_type.type_name;
  event.header.variable_header.length(0);
  event.filterable_data.length(0);
  event.remainder_of_body <<= ++num_events;
  batch->length(1);
  (*batch)[0] = event;
  has_event = 1;
  cout << "SequencePullSupplier_i: try_pull_structured_events() called" << endl;
  return batch;
}

inline void SequencePullSupplier_i::disconnect_sequence_pull_supplier()
{ cout << "SequencePullSupplier: disconnected" << endl; }

inline void SequencePullSupplier_i::subscription_change(
					const _EventTypeSeq& added,
					const _EventTypeSeq& deled)
{
  CORBA::ULong indx;
  _EventType   type;

  cout << "SequencePullSupplier: subscription_change contains: " << endl;
  for (indx = 0; indx < added.length(); indx++) {
        type = added[indx]; 
        cout << "\t+ " << type.domain_name << "::" << type.type_name << endl;
  }
  for (indx = 0; indx < deled.length(); indx++) {
        type = deled[indx];
        cout << "\t- " << type.domain_name << "::" << type.type_name << endl;
  }
}

/**
  *
  */

class SequencePushSupplier_i :
        public virtual CosNotifyComm::_sk_SequencePushSupplier {
public:
  SequencePushSupplier_i() {;}

  inline void disconnect_sequence_push_supplier();
  inline void subscription_change(const _EventTypeSeq& added,
				  const _EventTypeSeq& deled);
};

inline void SequencePushSupplier_i::disconnect_sequence_push_supplier()
{ cout << "SequencePushSupplier: disconnected" << endl; }

inline void SequencePushSupplier_i::subscription_change(
				const _EventTypeSeq& added,
				const _EventTypeSeq& deled)
{
  CORBA::ULong indx;
  _EventType   type;

  cout << "SequencePushSupplier: subscription_change contains: " << endl;
  for (indx = 0; indx < added.length(); indx++) {
        type = added[indx]; 
        cout << "\t+ " << type.domain_name << "::" << type.type_name << endl;
  }
  for (indx = 0; indx < deled.length(); indx++) {
        type = deled[indx];
        cout << "\t- " << type.domain_name << "::" << type.type_name << endl;
  }
}

#endif
