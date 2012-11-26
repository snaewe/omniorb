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
//    ZIOP stubs

#ifndef _omni_ziop_stubs_h_
#define _omni_ziop_stubs_h_

//
// The fun of Windows DLL linkage...
//

#ifdef _ziop_attr
#  error "A local CPP macro _ziop_attr has already been defined."
#endif

#ifdef _OMNIORB_ZIOP_LIBRARY
#  define _ziop_attr
#else
#  define _ziop_attr _OMNIORB_NTDLL_IMPORT
#  ifndef USE_core_stub_in_nt_dll
#    define USE_core_stub_in_nt_dll
#    define USE_core_stub_in_nt_dll_was_set
#  endif
#endif

#include <omniORB4/compression.hh>


//
// Definitions from standard ZIOP module
//

_CORBA_MODULE ZIOP
_CORBA_MODULE_BEG

# include <omniORB4/ziop_defs.hh>

# define OMNIORB_DECLARE_ZIOP_POLICY_CLASS(name, type, valret, valmethod) \
  class name; \
  typedef name* name##_ptr; \
  \
  class name : public CORBA::Policy  \
  {  \
  public:  \
    inline name(const name##Value& value_) :  \
      CORBA::Policy(type), pd_value(value_) {}  \
    inline name() {}  \
    virtual ~name();  \
    \
    virtual CORBA::Policy_ptr copy();  \
    virtual valret valmethod(); \
    \
    virtual void* _ptrToObjRef(const char* repoId);  \
    \
    static name##_ptr _duplicate(name##_ptr p);  \
    static name##_ptr _narrow(CORBA::Object_ptr p);  \
    static name##_ptr _nil();  \
    \
    static _ziop_attr const char* _PD_repoId;   \
    \
  private:  \
    name##Value pd_value;  \
  }; \
  \
  typedef _CORBA_PseudoObj_Var<name> name##_var

  OMNIORB_DECLARE_ZIOP_POLICY_CLASS(CompressionEnablingPolicy,
                                    COMPRESSION_ENABLING_POLICY_ID,
                                    CompressionEnablingPolicyValue,
                                    compression_enabled);

  OMNIORB_DECLARE_ZIOP_POLICY_CLASS(CompressionIdLevelListPolicy,
                                    COMPRESSOR_ID_LEVEL_LIST_POLICY_ID,
                                    CompressionIdLevelListPolicyValue*,
                                    compressor_ids);

  OMNIORB_DECLARE_ZIOP_POLICY_CLASS(CompressionLowValuePolicy,
                                    COMPRESSION_LOW_VALUE_POLICY_ID,
                                    CompressionLowValuePolicyValue,
                                    low_value);

  OMNIORB_DECLARE_ZIOP_POLICY_CLASS(CompressionMinRatioPolicy,
                                    COMPRESSION_MIN_RATIO_POLICY_ID,
                                    CompressionMinRatioPolicyValue,
                                    ratio);

# undef OMNIORB_DECLARE_ZIOP_POLICY_CLASS

_CORBA_MODULE_END


#ifdef USE_core_stub_in_nt_dll_was_set
#  undef USE_core_stub_in_nt_dll
#endif

#undef _ziop_attr

#endif // _omni_ziop_stubs_h_
