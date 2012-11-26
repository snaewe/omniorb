// -*- Mode: C++; -*-
//                            Package   : omniORB
// omniZIOP.cc                Created on: 2012/10/02
//                            Author    : Duncan Grisby (dgrisby)
//
//    Copyright (C) 2012 Apasphere Ltd.
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
// Description:
//    ZIOP support

#ifndef _omniZIOP_h_
#define _omniZIOP_h_

#include <omniORB4/CORBA.h>

#ifndef OMNIORB_ENABLE_ZIOP
#  error "omniORB was not configured with ZIOP support."
#endif

#include <omniORB4/ziopStubs.h>


//
// Proprietary omniZIOP module
//

_CORBA_MODULE omniZIOP
_CORBA_MODULE_BEG

  //
  // Policy creation functions
  //
  // These are equivalent to using orb->create_policy(), but do not
  // require use of Any.

  ZIOP::CompressionEnablingPolicy_ptr
  create_compression_enabling_policy(CORBA::Boolean compression_enabled);

  ZIOP::CompressionIdLevelListPolicy_ptr
  create_compression_id_level_list_policy(const Compression::CompressorIdLevelList& compressor_ids);

  ZIOP::CompressionLowValuePolicy_ptr
  create_compression_low_value_policy(CORBA::ULong low_value);

  ZIOP::CompressionMinRatioPolicy_ptr
  create_compression_min_ratio_policy(Compression::CompressionRatio ratio);


  //
  // Global policies

  void
  setGlobalPolicies(const CORBA::PolicyList& policies);
  // Sets global policies that apply to all POAs and all object
  // references, equivalent to separately setting the policies in
  // calls to the POAs and calling _set_policy_overrides() on all
  // object references.
  //
  // There is no need to include a CompressionEnablingPolicy with
  // value true, since calling this implicitly enables compression.
  // If other policies are not set, default values are used, so to
  // enable ZIOP with default settings, simply call setGlobalPolicies
  // with empty policies.

  CORBA::Object_ptr
  setServerPolicies(CORBA::Object_ptr obj, const CORBA::PolicyList& policies);
  // Returns a new object reference equivalent to obj, but with
  // specified compression policies, as if the server-side ORB had set
  // those policies. This is useful to enable ZIOP for an object
  // reference constructed from a corbaloc URI.
  //
  // ZIOP is only valid for GIOP 1.2 (and later), so the object
  // reference must be GIOP 1.2. For a corbaloc URI, specify it as
  // 
  //   corbaloc::1.2@some.host.name/key
  //
  // Beware that this may cause the client to make ZIOP calls to a
  // server that does not support ZIOP!


_CORBA_MODULE_END

#undef _ziop_attr

#endif // _omniZIOP_h_
