// -*- Mode: C++; -*-

// This file includes the four relevant .hh files and shorthands
// for the 4 module names and the elts of the modules.

// The shortdhands allow us to avoid the use of M::x to name elt
// x of module M (we use M_x instead).  This is needed to deal with
// compilers that do not support namespaces. 

#ifndef _COS_NOTIFY_SHORTHANDS_H_
#define _COS_NOTIFY_SHORTHANDS_H_

#ifndef COS_USES_BOA
#include "COS/TimeBase.hh"
#include "COS/CosNotification.hh"
#include "COS/CosNotifyComm.hh"
#include "COS/CosNotifyChannelAdmin.hh"
#include "COS/CosNotifyFilter.hh"
#else
#include "COS/BOA/TimeBase.hh"
#include "COS/BOA/CosNotification.hh"
#include "COS/BOA/CosNotifyComm.hh"
#include "COS/BOA/CosNotifyChannelAdmin.hh"
#include "COS/BOA/CosNotifyFilter.hh"
#endif

#define CosN   CosNotification
#define CosNC  CosNotifyComm
#define CosNA  CosNotifyChannelAdmin
#define CosNF  CosNotifyFilter

// XXX TODO: need typedefs and defines for
// XXX       compilers that do not support M::x syntax

typedef CosN::AdminProperties			CosN_AdminProperties;
typedef CosN::AdminPropertiesAdmin		CosN_AdminPropertiesAdmin;
#define CosN_AnyOrder				CosN::AnyOrder
#define CosN_BAD_PROPERTY			CosN::BAD_PROPERTY
#define CosN_BAD_TYPE				CosN::BAD_TYPE
#define CosN_BAD_VALUE				CosN::BAD_VALUE
#define CosN_BestEffort				CosN::BestEffort
#define CosN_ConnectionReliability		CosN::ConnectionReliability
#define CosN_DeadlineOrder			CosN::DeadlineOrder
#define CosN_DefaultPriority			CosN::DefaultPriority
#define CosN_DiscardPolicy			CosN::DiscardPolicy
typedef CosN::EventBatch			CosN_EventBatch;
#define CosN_EventReliability			CosN::EventReliability
typedef CosN::EventType				CosN_EventType;
typedef CosN::EventTypeSeq			CosN_EventTypeSeq;
#define CosN_FifoOrder				CosN::FifoOrder
#define CosN_HighestPriority			CosN::HighestPriority
#define CosN_LifoOrder				CosN::LifoOrder
#define CosN_LowestPriority			CosN::LowestPriority
#define CosN_MaxEventsPerConsumer		CosN::MaxEventsPerConsumer
#define CosN_MaximumBatchSize			CosN::MaximumBatchSize
#define CosN_NamedPropertyRangeSeq		CosN::NamedPropertyRangeSeq
#define CosN_OrderPolicy			CosN::OrderPolicy
#define CosN_PacingInterval			CosN::PacingInterval
#define CosN_Persistent				CosN::Persistent
#define CosN_Priority				CosN::Priority
#define CosN_PriorityOrder			CosN::PriorityOrder
typedef CosN::Property				CosN_Property;
typedef CosN::PropertyError			CosN_PropertyError;
typedef CosN::PropertyErrorSeq			CosN_PropertyErrorSeq;
typedef CosN::PropertyRange			CosN_PropertyRange;
typedef CosN::PropertySeq			CosN_PropertySeq;
typedef CosN::QoSAdmin				CosN_QoSAdmin;
typedef CosN::QoSError_code			CosN_QoSError_code;
typedef CosN::QoSProperties			CosN_QoSProperties;
#define CosN_RejectNewEvents			CosN::RejectNewEvents
#define CosN_StartTimeSupported			CosN::StartTimeSupported
#define CosN_StopTimeSupported			CosN::StopTimeSupported
typedef CosN::StructuredEvent			CosN_StructuredEvent;
#define CosN_Timeout				CosN::Timeout
#define CosN_UNAVAILABLE_PROPERTY		CosN::UNAVAILABLE_PROPERTY
#define CosN_UNAVAILABLE_VALUE			CosN::UNAVAILABLE_VALUE
#define CosN_UNSUPPORTED_PROPERTY		CosN::UNSUPPORTED_PROPERTY
#define CosN_UNSUPPORTED_VALUE			CosN::UNSUPPORTED_VALUE
typedef CosN::UnsupportedAdmin			CosN_UnsupportedAdmin;
typedef CosN::UnsupportedQoS			CosN_UnsupportedQoS;

#define CosNA_ALL_NOW_UPDATES_OFF		CosNA::ALL_NOW_UPDATES_OFF
#define CosNA_ALL_NOW_UPDATES_ON		CosNA::ALL_NOW_UPDATES_ON
#define CosNA_AND_OP				CosNA::AND_OP
#define CosNA_ANY_EVENT				CosNA::ANY_EVENT
typedef CosNA::AdminID				CosNA_AdminID;
typedef CosNA::AdminIDSeq			CosNA_AdminIDSeq;
typedef CosNA::AdminLimit			CosNA_AdminLimit;
typedef CosNA::AdminLimitExceeded		CosNA_AdminLimitExceeded;
typedef CosNA::AdminNotFound			CosNA_AdminNotFound;
typedef CosNA::ChannelID			CosNA_ChannelID;
typedef CosNA::ChannelIDSeq			CosNA_ChannelIDSeq;
typedef CosNA::ChannelNotFound			CosNA_ChannelNotFound;
typedef CosNA::ClientType			CosNA_ClientType;
typedef CosNA::ConnectionAlreadyActive		CosNA_ConnectionAlreadyActive;
typedef CosNA::ConnectionAlreadyInactive	CosNA_ConnectionAlreadyInactive;
typedef CosNA::ConsumerAdmin			CosNA_ConsumerAdmin;
typedef CosNA::ConsumerAdmin_ptr		CosNA_ConsumerAdmin_ptr;
typedef CosNA::ConsumerAdmin_var		CosNA_ConsumerAdmin_var;
typedef CosNA::EventChannel			CosNA_EventChannel;
typedef CosNA::EventChannelFactory		CosNA_EventChannelFactory;
typedef CosNA::EventChannelFactory_ptr		CosNA_EventChannelFactory_ptr;
typedef CosNA::EventChannelFactory_var		CosNA_EventChannelFactory_var;
typedef CosNA::EventChannel_ptr			CosNA_EventChannel_ptr;
typedef CosNA::EventChannel_var			CosNA_EventChannel_var;
typedef CosNA::InterFilterGroupOperator		CosNA_InterFilterGroupOperator;
#define CosNA_NONE_NOW_UPDATES_OFF		CosNA::NONE_NOW_UPDATES_OFF
#define CosNA_NONE_NOW_UPDATES_ON		CosNA::NONE_NOW_UPDATES_ON
typedef CosNA::NotConnected			CosNA_NotConnected;
#define CosNA_OR_OP				CosNA::OR_OP
typedef CosNA::ObtainInfoMode			CosNA_ObtainInfoMode;
#define CosNA_PULL_ANY				CosNA::PULL_ANY
#define CosNA_PULL_SEQUENCE			CosNA::PULL_SEQUENCE
#define CosNA_PULL_STRUCTURED			CosNA::PULL_STRUCTURED
#define CosNA_PUSH_ANY				CosNA::PUSH_ANY
#define CosNA_PUSH_SEQUENCE			CosNA::PUSH_SEQUENCE
#define CosNA_PUSH_STRUCTURED			CosNA::PUSH_STRUCTURED
#define CosNA_PUSH_TYPED     			CosNA::PUSH_TYPED
#define CosNA_PULL_TYPED     			CosNA::PULL_TYPED
typedef CosNA::ProxyConsumer			CosNA_ProxyConsumer;
typedef CosNA::ProxyConsumer_ptr		CosNA_ProxyConsumer_ptr;
typedef CosNA::ProxyConsumer_var		CosNA_ProxyConsumer_var;
typedef CosNA::ProxyID				CosNA_ProxyID;
typedef CosNA::ProxyIDSeq			CosNA_ProxyIDSeq;
typedef CosNA::ProxyNotFound			CosNA_ProxyNotFound;
typedef CosNA::ProxyPullConsumer		CosNA_ProxyPullConsumer;
typedef CosNA::ProxyPullSupplier		CosNA_ProxyPullSupplier;
typedef CosNA::ProxyPushConsumer		CosNA_ProxyPushConsumer;
typedef CosNA::ProxyPushConsumer_var		CosNA_ProxyPushConsumer_var;
typedef CosNA::ProxyPushSupplier		CosNA_ProxyPushSupplier;
typedef CosNA::ProxyPushSupplier_var		CosNA_ProxyPushSupplier_var;
typedef CosNA::ProxySupplier			CosNA_ProxySupplier;
typedef CosNA::ProxySupplier_ptr		CosNA_ProxySupplier_ptr;
typedef CosNA::ProxySupplier_var		CosNA_ProxySupplier_var;
typedef CosNA::ProxyType			CosNA_ProxyType;
#define CosNA_SEQUENCE_EVENT			CosNA::SEQUENCE_EVENT
#define CosNA_STRUCTURED_EVENT			CosNA::STRUCTURED_EVENT
typedef CosNA::SequenceProxyPullConsumer	CosNA_SequenceProxyPullConsumer;
typedef CosNA::SequenceProxyPullConsumer_var	CosNA_SequenceProxyPullConsumer_var;
typedef CosNA::SequenceProxyPullSupplier	CosNA_SequenceProxyPullSupplier;
typedef CosNA::SequenceProxyPullSupplier_var	CosNA_SequenceProxyPullSupplier_var;
typedef CosNA::SequenceProxyPushConsumer	CosNA_SequenceProxyPushConsumer;
typedef CosNA::SequenceProxyPushConsumer_ptr	CosNA_SequenceProxyPushConsumer_ptr;
typedef CosNA::SequenceProxyPushConsumer_var	CosNA_SequenceProxyPushConsumer_var;
typedef CosNA::SequenceProxyPushSupplier	CosNA_SequenceProxyPushSupplier;
typedef CosNA::SequenceProxyPushSupplier_var	CosNA_SequenceProxyPushSupplier_var;
typedef CosNA::StructuredProxyPullConsumer	CosNA_StructuredProxyPullConsumer;
typedef CosNA::StructuredProxyPullConsumer_var	CosNA_StructuredProxyPullConsumer_var;
typedef CosNA::StructuredProxyPullSupplier	CosNA_StructuredProxyPullSupplier;
typedef CosNA::StructuredProxyPullSupplier_var	CosNA_StructuredProxyPullSupplier_var;
typedef CosNA::StructuredProxyPushConsumer	CosNA_StructuredProxyPushConsumer;
typedef CosNA::StructuredProxyPushConsumer_ptr	CosNA_StructuredProxyPushConsumer_ptr;
typedef CosNA::StructuredProxyPushConsumer_var	CosNA_StructuredProxyPushConsumer_var;
typedef CosNA::StructuredProxyPushSupplier	CosNA_StructuredProxyPushSupplier;
typedef CosNA::StructuredProxyPushSupplier_var	CosNA_StructuredProxyPushSupplier_var;
typedef CosNA::SupplierAdmin			CosNA_SupplierAdmin;
typedef CosNA::SupplierAdmin_ptr		CosNA_SupplierAdmin_ptr;
typedef CosNA::SupplierAdmin_var		CosNA_SupplierAdmin_var;

typedef CosNC::InvalidEventType			CosNC_InvalidEventType;
typedef CosNC::NotifyPublish			CosNC_NotifyPublish;
typedef CosNC::NotifySubscribe			CosNC_NotifySubscribe;
typedef CosNC::NotifySubscribe_ptr		CosNC_NotifySubscribe_ptr;
typedef CosNC::SequencePullConsumer		CosNC_SequencePullConsumer;
typedef CosNC::SequencePullConsumer_ptr		CosNC_SequencePullConsumer_ptr;
typedef CosNC::SequencePullConsumer_var		CosNC_SequencePullConsumer_var;
typedef CosNC::SequencePullSupplier		CosNC_SequencePullSupplier;
typedef CosNC::SequencePullSupplier_ptr		CosNC_SequencePullSupplier_ptr;
typedef CosNC::SequencePullSupplier_var		CosNC_SequencePullSupplier_var;
typedef CosNC::SequencePushConsumer		CosNC_SequencePushConsumer;
typedef CosNC::SequencePushConsumer_ptr		CosNC_SequencePushConsumer_ptr;
typedef CosNC::SequencePushConsumer_var		CosNC_SequencePushConsumer_var;
typedef CosNC::SequencePushSupplier		CosNC_SequencePushSupplier;
typedef CosNC::SequencePushSupplier_ptr		CosNC_SequencePushSupplier_ptr;
typedef CosNC::SequencePushSupplier_var		CosNC_SequencePushSupplier_var;
typedef CosNC::StructuredPullConsumer		CosNC_StructuredPullConsumer;
typedef CosNC::StructuredPullConsumer_ptr	CosNC_StructuredPullConsumer_ptr;
typedef CosNC::StructuredPullConsumer_var	CosNC_StructuredPullConsumer_var;
typedef CosNC::StructuredPullSupplier		CosNC_StructuredPullSupplier;
typedef CosNC::StructuredPullSupplier_ptr	CosNC_StructuredPullSupplier_ptr;
typedef CosNC::StructuredPullSupplier_var	CosNC_StructuredPullSupplier_var;
typedef CosNC::StructuredPushConsumer		CosNC_StructuredPushConsumer;
typedef CosNC::StructuredPushConsumer_ptr	CosNC_StructuredPushConsumer_ptr;
typedef CosNC::StructuredPushConsumer_var	CosNC_StructuredPushConsumer_var;
typedef CosNC::StructuredPushSupplier		CosNC_StructuredPushSupplier;
typedef CosNC::StructuredPushSupplier_ptr		CosNC_StructuredPushSupplier_ptr;
typedef CosNC::StructuredPushSupplier_var	CosNC_StructuredPushSupplier_var;

typedef CosNF::CallbackID			CosNF_CallbackID;
typedef CosNF::CallbackIDSeq			CosNF_CallbackIDSeq;
typedef CosNF::ConstraintExp			CosNF_ConstraintExp;
typedef CosNF::ConstraintExpSeq			CosNF_ConstraintExpSeq;
typedef CosNF::ConstraintID			CosNF_ConstraintID;
typedef CosNF::ConstraintIDSeq			CosNF_ConstraintIDSeq;
typedef CosNF::ConstraintInfoSeq		CosNF_ConstraintInfoSeq;
typedef CosNF::ConstraintInfoSeq_var		CosNF_ConstraintInfoSeq_var;
typedef CosNF::ConstraintNotFound		CosNF_ConstraintNotFound;
typedef CosNF::Filter				CosNF_Filter;
typedef CosNF::FilterAdmin			CosNF_FilterAdmin;
typedef CosNF::FilterFactory			CosNF_FilterFactory;
typedef CosNF::FilterFactory_ptr		CosNF_FilterFactory_ptr;
typedef CosNF::FilterFactory_var		CosNF_FilterFactory_var;
typedef CosNF::FilterID				CosNF_FilterID;
typedef CosNF::FilterIDSeq			CosNF_FilterIDSeq;
typedef CosNF::FilterNotFound			CosNF_FilterNotFound;
typedef CosNF::Filter_ptr			CosNF_Filter_ptr;
typedef CosNF::Filter_var			CosNF_Filter_var;
typedef CosNF::InvalidConstraint		CosNF_InvalidConstraint;
typedef CosNF::InvalidGrammar			CosNF_InvalidGrammar;
typedef CosNF::MappingConstraintInfoSeq		CosNF_MappingConstraintInfoSeq;
typedef CosNF::MappingConstraintPairSeq		CosNF_MappingConstraintPairSeq;
typedef CosNF::MappingFilter			CosNF_MappingFilter;
typedef CosNF::MappingFilter_ptr		CosNF_MappingFilter_ptr;

#endif

