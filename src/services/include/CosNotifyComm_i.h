// -*- Mode: C++; -*-

#ifndef _COS_NOTIFY_COMM_I_H_
#define _COS_NOTIFY_COMM_I_H_

#include <iostream.h>
#include "corba_wrappers.h"
#include "CosNotifyComm.hh"

// ------------------------------------------------------------- //
// Here, we define the implementation of the client skeletons so //
// that a "default" library can be provided to applications.  In //
// most cases, applications will customize these implementations //
// using inheritance.                                            //
// ------------------------------------------------------------- //

typedef CosNotification::EventType       CosN_EventType;
typedef CosNotification::EventTypeSeq    CosN_EventTypeSeq;
typedef CosNotification::StructuredEvent CosN_StructuredEvent;
typedef CosNotification::EventBatch      CosN_EventBatch;

// ---------------- CosNotifyComm::PushConsumer ---------------- //

class PushConsumer_i : 
	WRAPPED_SKELETON_SUPER(CosNotifyComm::, PushConsumer) {
public:
  PushConsumer_i() : num_push(0) {;}

  inline void push(const CORBA::Any & data  WRAPPED_DECLARG );
  inline void disconnect_push_consumer( WRAPPED_DECLARG_VOID );
  inline void offer_change(const CosN_EventTypeSeq& added,
                           const CosN_EventTypeSeq& deled  WRAPPED_DECLARG );
protected:
  CORBA::ULong num_push;
};

inline void PushConsumer_i::push(const CORBA::Any& data WRAPPED_IMPLARG )
{ cout << "PushConsumer: push() called " << ++num_push << " times" << endl; }

inline void PushConsumer_i::disconnect_push_consumer( WRAPPED_IMPLARG_VOID )
{ cout << "PushConsumer: disconnected" << endl; }

inline void PushConsumer_i::offer_change(const CosN_EventTypeSeq& added,
				         const CosN_EventTypeSeq& deled 
					 WRAPPED_IMPLARG )
{
  CORBA::ULong indx;
  CosN_EventType   type;

  cout << "PushConsumer: offer_change contains: " << endl;
  for (indx = 0; indx < added.length(); indx++) {
        type = added[indx];
        cout << "\t+ " << (const char*)type.domain_name << 
		 "::"  << (const char*)type.type_name   << endl;
  }
  for (indx = 0; indx < deled.length(); indx++) {
        type = deled[indx];
        cout << "\t- " << (const char*)type.domain_name << 
	        "::"   << (const char*)type.type_name   << endl;
  }
}

// ---------------- CosNotifyComm::PullConsumer ---------------- //

class PullConsumer_i : 
	WRAPPED_SKELETON_SUPER(CosNotifyComm::, PullConsumer) {
public:
  PullConsumer_i() {;}

  inline void disconnect_pull_consumer(  WRAPPED_DECLARG_VOID );
  inline void offer_change(const CosN_EventTypeSeq& added,
			   const CosN_EventTypeSeq& deled WRAPPED_DECLARG );
};

inline void PullConsumer_i::disconnect_pull_consumer( WRAPPED_IMPLARG_VOID )
{ cout << "PullConsumer: disconnected" << endl; }

inline void PullConsumer_i::offer_change(const CosN_EventTypeSeq& added,
                                         const CosN_EventTypeSeq& deled
				         WRAPPED_IMPLARG )
{
  CORBA::ULong indx;
  CosN_EventType   type;

  cout << "PullConsumer: offer_change contains: " << endl;
  for (indx = 0; indx < added.length(); indx++) {
        type = added[indx];
        cout << "\t+ " << (const char*)type.domain_name << 
		"::"   << (const char*)type.type_name   << endl;
  }
  for (indx = 0; indx < deled.length(); indx++) {
        type = deled[indx];
        cout << "\t- " << (const char*)type.domain_name << 
		"::"   << (const char*)type.type_name   << endl;
  }
}

// ---------------- CosNotifyComm::PullSupplier ---------------- //

class PullSupplier_i : 
	WRAPPED_SKELETON_SUPER(CosNotifyComm::, PullSupplier) {
public:
  PullSupplier_i() : event_value(0) {;}
 
  inline CORBA::Any* pull( WRAPPED_DECLARG_VOID );
  inline CORBA::Any* try_pull(CORBA::Boolean& has_event WRAPPED_DECLARG );
  inline void        disconnect_pull_supplier( WRAPPED_DECLARG_VOID );
  inline void        subscription_change(const CosN_EventTypeSeq& added,
                                  	 const CosN_EventTypeSeq& deled
					 WRAPPED_DECLARG );
protected:
  CORBA::ULong event_value;
};

inline CORBA::Any* PullSupplier_i::pull( WRAPPED_IMPLARG_VOID )
{ 
  CORBA::Any *any = new CORBA::Any();
  *any <<= ++event_value;
  cout << "PullSupplier: pull() called for event " << event_value << endl;
  return any;
}

inline CORBA::Any* PullSupplier_i::try_pull(CORBA::Boolean& has_event
					    WRAPPED_IMPLARG )
{
  CORBA::Any *any = new CORBA::Any();
  *any <<= ++event_value;
  cout << "PullSupplier: try_pull() called for event " << event_value << endl;
  has_event = 1;
  return any;
}

inline void PullSupplier_i::disconnect_pull_supplier( WRAPPED_IMPLARG_VOID )
{ cout << "PullSupplier: disconnected" << endl; }

inline void PullSupplier_i::subscription_change(const CosN_EventTypeSeq& added,
					        const CosN_EventTypeSeq& deled
					        WRAPPED_IMPLARG )
{
  CORBA::ULong indx;
  CosN_EventType   type;

  cout << "PullSupplier: subscription_change contains: " << endl;
  for (indx = 0; indx < added.length(); indx++) {
        type = added[indx];
        cout << "\t+ " << (const char*)type.domain_name << 
		"::"   << (const char*)type.type_name   << endl;
  }
  for (indx = 0; indx < deled.length(); indx++) {
        type = deled[indx];
        cout << "\t- " << (const char*)type.domain_name << 
		"::"   << (const char*)type.type_name   << endl;
  }
}					

// ---------------- CosNotifyComm::PushSupplier ---------------- //

class PushSupplier_i : 
	WRAPPED_SKELETON_SUPER(CosNotifyComm::, PushSupplier) {
public:
  PushSupplier_i() {;}

  inline void disconnect_push_supplier(  WRAPPED_DECLARG_VOID );
  inline void subscription_change(const CosN_EventTypeSeq& added,
                                  const CosN_EventTypeSeq& deled 
				  WRAPPED_DECLARG );
};

inline void PushSupplier_i::disconnect_push_supplier( WRAPPED_IMPLARG_VOID )
{ cout << "PushSupplier: disconnected" << endl; }

inline void PushSupplier_i::subscription_change(const CosN_EventTypeSeq& added,
					 	const CosN_EventTypeSeq& deled
					        WRAPPED_IMPLARG )
{
  CORBA::ULong indx;
  CosN_EventType   type;

  cout << "PushSupplier: subscription_change contains: " << endl;
  for (indx = 0; indx < added.length(); indx++) {
        type = added[indx];
        cout << "\t+ " << (const char*)type.domain_name << 
		"::"   << (const char*)type.type_name   << endl;
  }
  for (indx = 0; indx < deled.length(); indx++) {
        type = deled[indx];
        cout << "\t- " << (const char*)type.domain_name << 
		"::"   << (const char*)type.type_name   << endl;
  }
}

// ----------- CosNotifyComm::StructuredPushConsumer ----------- //

class StructuredPushConsumer_i : 
	WRAPPED_SKELETON_SUPER(CosNotifyComm::, StructuredPushConsumer) {
public:
  StructuredPushConsumer_i() : num_push(0) {;}

  inline void push_structured_event(const CosN_StructuredEvent& event
				    WRAPPED_DECLARG );
  inline void disconnect_structured_push_consumer( WRAPPED_DECLARG_VOID );
  inline void offer_change(const CosN_EventTypeSeq& added,
                           const CosN_EventTypeSeq& deled
			   WRAPPED_DECLARG );
protected:
  CORBA::ULong num_push;
};

inline void StructuredPushConsumer_i::push_structured_event(
		const CosN_StructuredEvent& event WRAPPED_IMPLARG )
{ cout << "StructuredPushConsumer: push() " << ++num_push << " times" << endl; }

inline void StructuredPushConsumer_i::disconnect_structured_push_consumer( 
			WRAPPED_IMPLARG_VOID )
{ cout << "StructuredPushConsumer: disconnected" << endl; }

inline void StructuredPushConsumer_i::offer_change(
				const CosN_EventTypeSeq& added,
			      	const CosN_EventTypeSeq& deled WRAPPED_IMPLARG )
{
  CORBA::ULong indx;
  CosN_EventType   type;

  cout << "StructuredPushConsumer: offer_change contains: " << endl;
  for (indx = 0; indx < added.length(); indx++) {
        type = added[indx];
        cout << "\t+ " << (const char*)type.domain_name << 
		"::"   << (const char*)type.type_name   << endl;
  }
  for (indx = 0; indx < deled.length(); indx++) {
        type = deled[indx];
        cout << "\t- " << (const char*)type.domain_name << 
		"::"   << (const char*)type.type_name   << endl;
  }
}

// ----------- CosNotifyComm::StructuredPullConsumer ----------- //

class StructuredPullConsumer_i : 
	WRAPPED_SKELETON_SUPER(CosNotifyComm::, StructuredPullConsumer) {
public:
  StructuredPullConsumer_i() {;}

  inline void disconnect_structured_pull_consumer(  WRAPPED_DECLARG_VOID );
  inline void offer_change(const CosN_EventTypeSeq& added,
			   const CosN_EventTypeSeq& deled WRAPPED_DECLARG );
};

inline void StructuredPullConsumer_i::disconnect_structured_pull_consumer( 
				WRAPPED_IMPLARG_VOID )
{ cout << "StructuredPullConsumer: disconnected" << endl; }

inline void StructuredPullConsumer_i::offer_change(
				const CosN_EventTypeSeq& added,
				const CosN_EventTypeSeq& deled WRAPPED_IMPLARG )
{
  CORBA::ULong indx;
  CosN_EventType   type;

  cout << "StructuredPullConsumer: offer_change contains: " << endl;
  for (indx = 0; indx < added.length(); indx++) {
        type = added[indx];
        cout << "\t+ " << (const char*)type.domain_name << 
		"::"   << (const char*)type.type_name   << endl;
  }
  for (indx = 0; indx < deled.length(); indx++) {
        type = deled[indx];
        cout << "\t- " << (const char*)type.domain_name << 
		"::"   << (const char*)type.type_name   << endl;
  }
}

// ----------- CosNotifyComm::StructuredPullSupplier ----------- //

class StructuredPullSupplier_i : 
	WRAPPED_SKELETON_SUPER(CosNotifyComm::, StructuredPullSupplier) {
public:
  inline StructuredPullSupplier_i();

  inline CosN_StructuredEvent* pull_structured_event( WRAPPED_DECLARG_VOID );
  inline CosN_StructuredEvent* try_pull_structured_event(
			CORBA::Boolean& has_event WRAPPED_DECLARG );
  inline void disconnect_structured_pull_supplier(  WRAPPED_DECLARG_VOID );
  inline void subscription_change(const CosN_EventTypeSeq& added,
			          const CosN_EventTypeSeq& deled 
				  WRAPPED_DECLARG );
protected:
  CORBA::ULong num_events;
  CosN_EventType   event_type;
};

inline StructuredPullSupplier_i::StructuredPullSupplier_i() : num_events(0)
{ event_type.domain_name = CORBA::string_dup("DummyDomain");
  event_type.type_name   = CORBA::string_dup("DummyType");
}

inline CosN_StructuredEvent* StructuredPullSupplier_i::pull_structured_event( 
				WRAPPED_IMPLARG_VOID )
{
  CosN_StructuredEvent* event = new CosN_StructuredEvent();
  event->header.fixed_header.event_type.domain_name = event_type.domain_name;
  event->header.fixed_header.event_type.type_name   = event_type.type_name;
  event->header.variable_header.length(0);
  event->filterable_data.length(0);
  event->remainder_of_body <<= ++num_events;
  cout << "StructuredPullSupplier_i: pull_structured_event() called" << endl;
  return event;
}

inline CosN_StructuredEvent* 
StructuredPullSupplier_i::try_pull_structured_event(
				CORBA::Boolean& has_event WRAPPED_IMPLARG )
{
  CosN_StructuredEvent* event = new CosN_StructuredEvent();
  event->header.fixed_header.event_type.domain_name = event_type.domain_name; 
  event->header.fixed_header.event_type.type_name   = event_type.type_name;
  event->header.variable_header.length(0);
  event->filterable_data.length(0);
  event->remainder_of_body <<= ++num_events;
  has_event = 1;
  cout << "StructuredPullSupplier_i: try_pull_structured_event() called"<< endl;
  return event;
}

inline void StructuredPullSupplier_i::disconnect_structured_pull_supplier(
				WRAPPED_IMPLARG_VOID )
{ cout << " StructuredPullSupplier: disconnected" << endl; }

inline void StructuredPullSupplier_i::subscription_change(
				const CosN_EventTypeSeq& added,
				const CosN_EventTypeSeq& deled WRAPPED_IMPLARG )
{
  CORBA::ULong indx;
  CosN_EventType   type;

  cout << "StructuredPullSupplier: subscription_change contains: " << endl;
  for (indx = 0; indx < added.length(); indx++) {
        type = added[indx]; 
        cout << "\t+ " << (const char*)type.domain_name << 
		"::"   << (const char*)type.type_name   << endl;
  }
  for (indx = 0; indx < deled.length(); indx++) {
        type = deled[indx];
        cout << "\t- " << (const char*)type.domain_name << 
		"::"   << (const char*)type.type_name   << endl;
  }
}

// ----------- CosNotifyComm::StructuredPushSupplier ----------- //

class StructuredPushSupplier_i : 
	WRAPPED_SKELETON_SUPER(CosNotifyComm::, StructuredPushSupplier) {
public:
  StructuredPushSupplier_i()	{;}

  inline void disconnect_structured_push_supplier(  WRAPPED_DECLARG_VOID );
  inline void subscription_change(const CosN_EventTypeSeq& added,
				  const CosN_EventTypeSeq& deled 
				  WRAPPED_DECLARG );
};

inline void StructuredPushSupplier_i::disconnect_structured_push_supplier( 
				WRAPPED_IMPLARG_VOID )
{ cout << "StructuredPushSupplier: disconnected" << endl; }

inline void StructuredPushSupplier_i::subscription_change(
				const CosN_EventTypeSeq& added,
				const CosN_EventTypeSeq& deled WRAPPED_IMPLARG )
{
  CORBA::ULong indx;
  CosN_EventType   type;

  cout << "StructuredPushSupplier: subscription_change contains: " << endl;
  for (indx = 0; indx < added.length(); indx++) {
        type = added[indx]; 
        cout << "\t+ " << (const char*)type.domain_name << 
		"::"   << (const char*)type.type_name   << endl;
  }
  for (indx = 0; indx < deled.length(); indx++) {
        type = deled[indx];
        cout << "\t- " << (const char*)type.domain_name << 
		"::"   << (const char*)type.type_name   << endl;
  }
}

// ------------ CosNotifyComm::SequencePushConsumer ------------ //

class SequencePushConsumer_i : 
	WRAPPED_SKELETON_SUPER(CosNotifyComm::, SequencePushConsumer) {
public:
  SequencePushConsumer_i() : num_batches(0), num_events(0) {;}

  inline void push_structured_events(const CosN_EventBatch& events 
				     WRAPPED_DECLARG );
  inline void disconnect_sequence_push_consumer( WRAPPED_DECLARG_VOID );
  inline void offer_change(const CosN_EventTypeSeq& added,
			   const CosN_EventTypeSeq& deled WRAPPED_DECLARG );
protected:
  CORBA::ULong num_batches;
  CORBA::ULong num_events;
};

inline void SequencePushConsumer_i::push_structured_events(
				const CosN_EventBatch& events WRAPPED_IMPLARG )
{ 
  num_batches += 1;
  num_events  += events.length();
  cout << "SequencePushConsumer: " << num_batches << " batches with " <<
	  num_events << " events so far" << endl;
}

inline void SequencePushConsumer_i::disconnect_sequence_push_consumer(  
				WRAPPED_IMPLARG_VOID )
{ cout << "SequencePushConsumer: disconnected" << endl; }

inline void SequencePushConsumer_i::offer_change(
				const CosN_EventTypeSeq& added,
			  	const CosN_EventTypeSeq& deled WRAPPED_IMPLARG )
{
  CORBA::ULong indx;
  CosN_EventType   type;

  cout << "SequencePushConsumer: offer_change contains: " << endl;
  for (indx = 0; indx < added.length(); indx++) {
        type = added[indx];
        cout << "\t+ " << (const char*)type.domain_name << 
		"::"   << (const char*)type.type_name   << endl; 
  }
  for (indx = 0; indx < deled.length(); indx++) {
        type = deled[indx];
        cout << "\t- " << (const char*)type.domain_name << 
		"::"   << (const char*)type.type_name   << endl;
  }
}

// ------------ CosNotifyComm::SequencePullConsumer ------------ //

class SequencePullConsumer_i : 
	WRAPPED_SKELETON_SUPER(CosNotifyComm::, SequencePullConsumer) {
public:
  SequencePullConsumer_i() {;}

  inline void disconnect_sequence_pull_consumer(  WRAPPED_DECLARG_VOID );
  inline void offer_change(const CosN_EventTypeSeq& added,
			   const CosN_EventTypeSeq& deled WRAPPED_DECLARG );
};

inline void SequencePullConsumer_i::disconnect_sequence_pull_consumer( 
				WRAPPED_IMPLARG_VOID )
{ cout << "SequencePullConsumer: disconnected" << endl; }

inline void SequencePullConsumer_i::offer_change(const CosN_EventTypeSeq& added,
					  	 const CosN_EventTypeSeq& deled
					         WRAPPED_IMPLARG )
{
  CORBA::ULong indx;
  CosN_EventType   type;

  cout << "SequencePullConsumer: offer_change contains: " << endl;
  for (indx = 0; indx < added.length(); indx++) {
        type = added[indx];
        cout << "\t+ " << (const char*)type.domain_name << 
		"::"   << (const char*)type.type_name   << endl;
  }
  for (indx = 0; indx < deled.length(); indx++) {
        type = deled[indx];
        cout << "\t- " << (const char*)type.domain_name << 
		"::"   << (const char*)type.type_name   << endl;
  }
}

// ------------ CosNotifyComm::SequencePullSupplier ------------ //

class SequencePullSupplier_i : 
	WRAPPED_SKELETON_SUPER(CosNotifyComm::, SequencePullSupplier) {
public:
  inline SequencePullSupplier_i();

  inline CosN_EventBatch* pull_structured_events(CORBA::Long max_number
					         WRAPPED_DECLARG );
  inline CosN_EventBatch* try_pull_structured_events(CORBA::Long max_number,
						     CORBA::Boolean& has_event
						     WRAPPED_DECLARG );
  inline void disconnect_sequence_pull_supplier( WRAPPED_DECLARG_VOID );
  inline void subscription_change(const CosN_EventTypeSeq& added,
				  const CosN_EventTypeSeq& deled 
				  WRAPPED_DECLARG );
protected:
  CORBA::ULong num_events;
  CosN_EventType   event_type;
};

inline SequencePullSupplier_i::SequencePullSupplier_i() : num_events(0)
{ event_type.domain_name = CORBA::string_dup("DummyDomain");
  event_type.type_name   = CORBA::string_dup("DummyType");
}

inline CosN_EventBatch* SequencePullSupplier_i::pull_structured_events(
				CORBA::Long max_number WRAPPED_IMPLARG )
{
  CosN_EventBatch*     batch = new CosN_EventBatch;
  CosN_StructuredEvent event;
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

inline CosN_EventBatch* SequencePullSupplier_i::try_pull_structured_events(
				CORBA::Long max_number,
				CORBA::Boolean& has_event WRAPPED_IMPLARG )
{
  CosN_EventBatch*     batch = new CosN_EventBatch;
  CosN_StructuredEvent event;
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

inline void SequencePullSupplier_i::disconnect_sequence_pull_supplier( 
				WRAPPED_IMPLARG_VOID )
{ cout << "SequencePullSupplier: disconnected" << endl; }

inline void SequencePullSupplier_i::subscription_change(
				const CosN_EventTypeSeq& added,
				const CosN_EventTypeSeq& deled WRAPPED_IMPLARG )
{
  CORBA::ULong indx;
  CosN_EventType   type;

  cout << "SequencePullSupplier: subscription_change contains: " << endl;
  for (indx = 0; indx < added.length(); indx++) {
        type = added[indx]; 
        cout << "\t+ " << (const char*)type.domain_name << 
		"::"   << (const char*)type.type_name   << endl;
  }
  for (indx = 0; indx < deled.length(); indx++) {
        type = deled[indx];
        cout << "\t- " << (const char*)type.domain_name << 
		"::"   << (const char*)type.type_name   << endl;
  }
}

// ------------ CosNotifyComm::SequencePushSupplier ------------ //

class SequencePushSupplier_i : 
	WRAPPED_SKELETON_SUPER(CosNotifyComm::, SequencePushSupplier) {
public:
  SequencePushSupplier_i() {;}

  inline void disconnect_sequence_push_supplier( WRAPPED_DECLARG_VOID );
  inline void subscription_change(const CosN_EventTypeSeq& added,
				  const CosN_EventTypeSeq& deled 
				  WRAPPED_DECLARG );
};

inline void SequencePushSupplier_i::disconnect_sequence_push_supplier(  
				WRAPPED_IMPLARG_VOID )
{ cout << "SequencePushSupplier: disconnected" << endl; }

inline void SequencePushSupplier_i::subscription_change(
				const CosN_EventTypeSeq& added,
				const CosN_EventTypeSeq& deled WRAPPED_IMPLARG )
{
  CORBA::ULong indx;
  CosN_EventType type;

  cout << "SequencePushSupplier: subscription_change contains: " << endl;
  for (indx = 0; indx < added.length(); indx++) {
        type = added[indx]; 
        cout << "\t+ " << (const char*)type.domain_name << 
		"::"   << (const char*)type.type_name   << endl;
  }
  for (indx = 0; indx < deled.length(); indx++) {
        type = deled[indx];
        cout << "\t- " << (const char*)type.domain_name << 
		"::"   << (const char*)type.type_name   << endl;
  }
}

#endif
