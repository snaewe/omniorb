// -*- Mode: C++; -*-
//                            Package   : omniORB2
// omniIOR.h                  Created on: 11/8/99
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2002-2007 Apasphere Ltd
//    Copyright (C) 1999-2000 AT&T Laboratories, Cambridge
//
//    This file is part of the omniORB library
//
//    The omniORB library is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Library General Public
//    License as published by the Free Software Foundation; either
//    version 2 of the License, or (at your option) any later version.
//
//    This library is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Library General Public License for more details.
//
//    You should have received a copy of the GNU Library General Public
//    License along with this library; if not, write to the Free
//    Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//    02111-1307, USA
//
//
// Description:
//	*** PROPRIETORY INTERFACE ***
//

/*
  $Log$
  Revision 1.1.4.5  2009/05/06 16:16:13  dgrisby
  Update lots of copyright notices.

  Revision 1.1.4.4  2007/11/28 12:24:26  dgrisby
  Implement a tiny subset of CSIv2 to permit multiple SSL endpoints in IORs.

  Revision 1.1.4.3  2006/07/18 16:21:24  dgrisby
  New experimental connection management extension; ORB core support
  for it.

  Revision 1.1.4.2  2005/01/06 23:08:09  dgrisby
  Big merge from omni4_0_develop.

  Revision 1.1.4.1  2003/03/23 21:04:14  dgrisby
  Start of omniORB 4.1.x development branch.

  Revision 1.1.2.13  2003/01/14 11:48:15  dgrisby
  Remove warnings from gcc -Wshadow. Thanks Pablo Mejia.

  Revision 1.1.2.12  2002/09/08 21:12:38  dgrisby
  Properly handle IORs with no usable profiles.

  Revision 1.1.2.11  2002/01/02 18:13:43  dpg1
  Platform fixes/additions.

  Revision 1.1.2.10  2001/08/15 10:26:08  dpg1
  New object table behaviour, correct POA semantics.

  Revision 1.1.2.9  2001/08/06 15:49:17  sll
  Added IOP component TAG_OMNIORB_UNIX_TRANS for omniORB specific local
  transport using the unix domain socket.

  Revision 1.1.2.8  2001/07/31 16:10:38  sll
  Added GIOP BiDir support.

  Revision 1.1.2.7  2001/06/13 20:07:25  sll
  Minor update to make the ORB compiles with MSVC++.

  Revision 1.1.2.6  2001/06/11 17:53:23  sll
   The omniIOR ctor used by genior and corbaloc now has the option to
   select whether to call interceptors and what set of interceptors to call.

  Revision 1.1.2.5  2001/05/09 17:00:27  sll
  addr_selected_profile_index() now returns signed long.

  Revision 1.1.2.4  2001/04/18 17:50:44  sll
  Big checkin with the brand new internal APIs.
  Scoped where appropriate with the omni namespace.

  Revision 1.1.2.3  2000/11/15 17:06:49  sll
  Added tcs_c and tcs_w in omniIOR to record what code set convertor to use
  for this IOR.
  Added declarations for handlers of the tagged components used by the ORB.

  Revision 1.1.2.2  2000/10/03 17:37:07  sll
  Changed omniIOR synchronisation mutex from omni::internalLock to its own
  mutex.

  Revision 1.1.2.1  2000/09/27 16:54:08  sll
  *** empty log message ***

*/

#ifndef __OMNIIOR_H__
#define __OMNIIOR_H__

#include <omniORB4/giopEndpoint.h>

OMNI_NAMESPACE_BEGIN(omni)
class Rope;
OMNI_NAMESPACE_END(omni)

class omniIORHints {
public:
  const CORBA::PolicyList* policies;
  inline omniIORHints(const CORBA::PolicyList* p) : policies(p) {}
};


class omniIOR {
public:

  ///////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  // Accessor functions for the 2 components of the IOR
  const char* repositoryID() const {
    return pd_repositoryID;
  }

  const IOP::TaggedProfileList& iopProfiles() const {
    return pd_iopProfiles;
  }

  // index into pd_iopProfiles which has been chosen and decoded
  _CORBA_Long addr_selected_profile_index() const {
    return pd_addr_selected_profile_index;
  }
  void addr_selected_profile_index(_CORBA_Long index_) {
    pd_addr_selected_profile_index = index_;
  }

  // set and get functions for addr_mode.
  // The value of addr_mode determines what AddressingDisposition mode
  // the ORB will use to invoke on the object. The mode is only relevant
  // for GIOP 1.2 upwards.
  GIOP::AddressingDisposition addr_mode() const {
    return pd_addr_mode;
  }

  void addr_mode(GIOP::AddressingDisposition m) {
    pd_addr_mode = m;
  }


  ///////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  // Information encoded in the taggedprofilelist may be decoded
  // into the more accessible forms and stored in an IORInfo.
  //
  class IORInfo;
  IORInfo* getIORInfo() const;

  // Within IORInfo, additional decoded info can be deposited using
  // the IORExtraInfo class. Typically this is used by decodeIOR interceptors
  // to insert extra info into the IOR for later retreival.
  //
  class IORExtraInfo {
  public:
    IORExtraInfo(const IOP::ComponentId cid) : compid(cid) {}
    virtual ~IORExtraInfo() {}
    IOP::ComponentId compid;
  private:
    IORExtraInfo();
    IORExtraInfo(const IORExtraInfo&);
    IORExtraInfo& operator=(const IORExtraInfo&);
  };
  // For each unique ComponentId (e.g., TAG_GROUP) one can add
  // an IORExtraInfo element to the extra_info list.
  // Each extrainfo element should be constructed by deriving from
  // this class so that extra members can be held.

  typedef _CORBA_PseudoValue_Sequence<IORExtraInfo*> IORExtraInfoList;

  class IORInfo {
  public:

    // GIOP version.
    const GIOP::Version& version() const {
      return pd_version;
    }

    void version(const GIOP::Version& ver) {
      pd_version = ver;
    }

    // List of address to use
    _OMNI_NS(giopAddressList)& addresses() {
      return pd_addresses;
    }

    // ORB ID
    _CORBA_ULong orbType() const { return pd_orb_type; }
    void orbType(_CORBA_ULong t) { pd_orb_type = t; }

    // Transmission code set for char and string
    _OMNI_NS(omniCodeSet::TCS_C)* TCS_C() const { return pd_tcs_c; }
    void TCS_C(_OMNI_NS(omniCodeSet::TCS_C)* tcs_c) { pd_tcs_c = tcs_c; }

    // Transmission code set for wchar and wstring
    _OMNI_NS(omniCodeSet::TCS_W)* TCS_W() const { return pd_tcs_w; }
    void TCS_W(_OMNI_NS(omniCodeSet::TCS_W)* tcs_w) { pd_tcs_w = tcs_w; }

    // Extra info list
    IORExtraInfoList& extraInfo() { return pd_extra_info; }

    IORInfo();
    ~IORInfo();

  private:
    GIOP::Version                      pd_version;
    _OMNI_NS(giopAddressList)          pd_addresses;
    _CORBA_ULong                       pd_orb_type;
    _OMNI_NS(omniCodeSet::TCS_C)*      pd_tcs_c;
    _OMNI_NS(omniCodeSet::TCS_W)*      pd_tcs_w;
    IORExtraInfoList                   pd_extra_info;
  };

public:

  ///////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  omniIOR(char* repoId, IOP::TaggedProfileList* iop);
  // Both repoId and iop are consumed by the object.

  omniIOR(char* repoId, IOP::TaggedProfile* iop, _CORBA_ULong niops,
	  _CORBA_ULong selected_profile_index);
  // Both repoId and iop are consumed by the object.

  omniIOR(const char* repoId, const _CORBA_Octet* key, int keysize,
	  const omniIORHints& hints);
  // Create an IOR for a local object with the given key
  //  Must hold <omni::internalLock>.

  enum interceptorOption { NoInterceptor,
			   DefaultInterceptors,
			   AllInterceptors };

  omniIOR(const char* repoId,
	  const _CORBA_Unbounded_Sequence_Octet& key,
	  const IIOP::Address* addrs, _CORBA_ULong naddrs,
	  GIOP::Version ver, interceptorOption call_interceptors,
          const IOP::MultipleComponentProfile* tagged_components = 0);

  ~omniIOR();

  void marshalIORAddressingInfo(cdrStream& s) const;

  // Synchronisation and reference counting:
  //
  // The object is reference counted. Call duplcate() to increment the
  // reference count. Call release() to decrement the reference count.
  //
  // The reference count is protected by the mutex omniIOR::lock.
  //
  omniIOR* duplicate();
  // return a pointer to this object.
  // Atomic and thread safe. Caller must not hold omniIOR::lock.

  void release();
  // If the reference count is 0, delete is called on the object.
  // Atomic and thread safe. Caller must not hold omniIOR::lock

  static _core_attr omni_tracedmutex* lock;


public:
  // ORB internal functions.

  omniIOR* duplicateNoLock();
  // Must hold <omniIOR::lock>. Otherwise same semantics as duplicate().

  void releaseNoLock();
  // Must hold <omniIOR::lock>. Otherwise same semantics as release().

  ///////////////////////////////////////////////////////////////////
  // Create a new taggedcomponent slot in the IIOP profile.
  // The caller can write to the tagged component immediately after this
  // call.
  // Use this call only in the encodeIOR interceptors.
  static IOP::TaggedComponent& newIIOPtaggedComponent(IOP::MultipleComponentProfile&);

  void decodeIOPprofile(const IIOP::ProfileBody&);
  // Decode the information in the argument into IORInfo accessible via
  // getIORInfo().

  // Handlers for each of the tagged component used by the ORB
  static void  unmarshal_TAG_ORB_TYPE(const IOP::TaggedComponent&, omniIOR&);
  static char* dump_TAG_ORB_TYPE(const IOP::TaggedComponent&);

  ////
  static void  unmarshal_TAG_CODE_SETS(const IOP::TaggedComponent&, omniIOR&);
  static char* dump_TAG_CODE_SETS(const IOP::TaggedComponent&);
  static void  add_TAG_CODE_SETS(const _OMNI_NS(CONV_FRAME::CodeSetComponentInfo)&);

  ////
  static void  unmarshal_TAG_ALTERNATE_IIOP_ADDRESS(const IOP::TaggedComponent&,
                                                   omniIOR&);
  static char* dump_TAG_ALTERNATE_IIOP_ADDRESS(const IOP::TaggedComponent&);
  static void  add_TAG_ALTERNATE_IIOP_ADDRESS(const IIOP::Address&);

  ////
  static void  unmarshal_TAG_GROUP(const IOP::TaggedComponent&, omniIOR&);
  static char* dump_TAG_GROUP(const IOP::TaggedComponent&);

  ////
  static void  unmarshal_TAG_SSL_SEC_TRANS(const IOP::TaggedComponent&,
					   omniIOR&);
  static char* dump_TAG_SSL_SEC_TRANS(const IOP::TaggedComponent&);
  static void  add_TAG_SSL_SEC_TRANS(const IIOP::Address&,
				     _CORBA_UShort supports,
				     _CORBA_UShort requires);

  ////
  static void unmarshal_TAG_CSI_SEC_MECH_LIST(const IOP::TaggedComponent&,
					      omniIOR&);
  static char* dump_TAG_CSI_SEC_MECH_LIST(const IOP::TaggedComponent&);


  ////
  static void  unmarshal_TAG_OMNIORB_BIDIR(const IOP::TaggedComponent&,
					   omniIOR&);
  static char* dump_TAG_OMNIORB_BIDIR(const IOP::TaggedComponent&);
  static void  add_TAG_OMNIORB_BIDIR(const char* sendfrom,
				     omniIOR&);

  ////
  static void  unmarshal_TAG_OMNIORB_UNIX_TRANS(const IOP::TaggedComponent&,
						omniIOR&);
  static char* dump_TAG_OMNIORB_UNIX_TRANS(const IOP::TaggedComponent&);
  static void  add_TAG_OMNIORB_UNIX_TRANS(const char* filename);

  ////
  static void  unmarshal_TAG_OMNIORB_PERSISTENT_ID(const IOP::TaggedComponent&,
						   omniIOR&);
  static char* dump_TAG_OMNIORB_PERSISTENT_ID(const IOP::TaggedComponent&);

  ////
  static void  add_IIOP_ADDRESS(const IIOP::Address&);
  // Add this address to the IIOP profile.

private:

  _CORBA_String_member               pd_repositoryID;
  IOP::TaggedProfileList_var 	     pd_iopProfiles;
  _CORBA_Long                        pd_addr_selected_profile_index;
  GIOP::AddressingDisposition        pd_addr_mode;
  IORInfo*                           pd_iorInfo;
  int                                pd_refCount;
  // Protected by <omni::internalLock>

  omniIOR();
  omniIOR(const omniIOR&);
  omniIOR& operator=(const omniIOR&);

};

class omniIOR_var {
public:
  inline omniIOR_var() : pd_ior(0) {}
  inline omniIOR_var(omniIOR* ior) : pd_ior(ior) {}
  inline omniIOR* _retn() { omniIOR* p = pd_ior; pd_ior = 0; return p; }
  inline operator omniIOR* () const { return pd_ior; }
  inline omniIOR* operator->() const { return pd_ior; }
  inline omniIOR_var& operator=(omniIOR* p) {
    if (pd_ior) pd_ior->release();
    pd_ior = p;
    return *this;
  }
  inline ~omniIOR_var() {
    if (pd_ior) pd_ior->release();
  }
private:
  omniIOR* pd_ior;
};


#endif // __OMNIIOR_H__
