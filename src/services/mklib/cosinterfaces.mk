##############################################################################
# N.B. CosNaming omitted -- its stubs/skels are already in omniORB libraries.
# The .hh file can be found at <omni_top>/include/<omniORBX>/CosNaming.hh
##############################################################################

##############################################################################
# Summary of Dependencies:
# 
#    if you use                 precede with
# --------------    -------------------------------------------
# COS_SEC_INT       TIMEBASE_INT
# COS_EVT_INT       TIMEBASE_INT
# COS_NFY_INT       TIMEBASE_INT, COS_EVT_INT
# COS_COL_INT       TIMEBASE_INT, COS_EVT_INT
# COS_LIC_INT       TIMEBASE_INT, COS_EVT_INT, COS_PRP_INT
# COS_LIF_INT       COS_REL_INT
# COS_EXT_INT       COS_REL_INT, COS_LIF_INT
# 
##############################################################################

# The following should be a valid ordering on all the interfaces.
# One could copy + edit this to produce a desired COS_INTERFACES,
# keeping in mind the dependencies above.
#
# COS_TRX_INT and COS_SEC_INT are left out because:
# * Transactions requires:  CORBA::Current, CORBA::Environent
#        -- currently missing from omniORB's CORBA.h
# * Security requires: CORBA::Current,
#                       CORBA::ServiceOption, CORBA::ServiceDetailType
#        -- currently missing from omniORB's CORBA.h

COS_ALL_INT = \
  $(TIMEBASE_INT) \
  $(COS_REL_INT) \
  $(COS_PRP_INT) \
  $(COS_TRD_INT) \
  $(COS_PER_INT) \
  $(COS_QRY_INT) \
  $(COS_EVT_INT) \
  $(COS_NFY_INT) \
  $(COS_COL_INT) \
  $(COS_LIC_INT) \
  $(COS_LIF_INT) \
  $(COS_EXT_INT)

# omitted:
#  $(COS_SEC_INT) \
#  $(COS_TRX_INT) \

# TimeBase is used be many interfaces
TIMEBASE_INT = \
  TimeBase

# Interfaces for CosTime*/CosEvent*
#   DEPENDS_ON: TIMEBASE_INT
COS_EVT_INT = \
  CosTime \
  CosEventComm \
  CosEventChannelAdmin \
  CosTypedEventComm \
  CosTypedEventChannelAdmin \
  CosTimerEvent

# Interfaces for Notification
#   DEPENDS_ON: TIMEBASE_INT, COS_EVT_INT
COS_NFY_INT = \
  CosNotification \
  CosNotifyComm \
  CosNotifyFilter \
  CosNotifyChannelAdmin \
  CosTypedNotifyComm \
  CosTypedNotifyChannelAdmin

# Interfaces for Relationships
COS_REL_INT = \
  CosObjectIdentity \
  CosRelationships \
  CosGraphs \
  CosContainment \
  CosReference

# Interfaces for Transactions and Concurrency
COS_TRX_INT = \
  CosTransactions \
  CosTSPortability \
  CosConcurrencyControl

# Interfaces for PropertyService
COS_PRP_INT = \
  CosPropertyService

# Interfaces for LicensingManager
#   DEPENDS_ON: CosEventComm       (=> TIMEBASE_INT, COS_EVT_INT)
#   DEPENDS_ON: CosPropertyService (=> COS_PRP_INT)
COS_LIC_INT = \
  CosLicensingManager

# Interfaces for Collection
#   DEPENDS_ON: CosEventComm (in TIMEBASE_INT, COS_EVT_INT)
COS_COL_INT = \
  CosCollection

# Interfaces for Security
#   DEPENDS_ON: TIMEBASE_INT
COS_SEC_INT = \
  DCE_CIOPSecurity \
  Security \
  SecurityLevel1 \
  SecurityLevel2 \
  SecurityAdmin \
  SecurityReplaceable \
  NRService \
  SECIOP \
  SSLIOP

# Interfaces for Externalization
#  DEPENDS_ON:  CosLifeCycle (COS_LIF_INT)
#  DEPENDS_ON:  several interfaces in COS_REL_INT
COS_EXT_INT = \
  CosStream \
  CosExternalization \
  CosExternalizationContainment \
  CosExternalizationReference

# Interfaces for LifeCycle
#  DEPENDS_ON:  CosNaming
#  DEPENDS_ON:  several interfaces in COS_REL_INT
COS_LIF_INT = \
  CosLifeCycle \
  CosCompoundLifeCycle \
  CosLifeCycleReference \
  CosLifeCycleContainment \
  LifeCycleService

# Interfaces for Trading
COS_TRD_INT = \
  CosTrading \
  CosTradingDynamic \
  CosTradingRepos

# Interfaces for Persistence
COS_PER_INT = \
  CosPersistencePID \
  CosPersistencePDS \
  CosPersistencePDS_DA \
  CosPersistencePO \
  CosPersistencePOM \
  CosPersistenceDDO \
  CosPersistenceDS_CLI

# Interfaces for Query
COS_QRY_INT = \
  CosQueryCollection \
  CosQuery

