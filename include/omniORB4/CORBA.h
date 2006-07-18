// -*- Mode: C++; -*-
//                            Package   : omniORB
// CORBA.h                    Created on: 30/1/96
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 1996-1999 AT&T Laboratories Cambridge
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
//    A complete set of C++ definitions for the CORBA module.
//

/*
 $Log$
 Revision 1.5.2.10  2006/07/18 16:21:24  dgrisby
 New experimental connection management extension; ORB core support
 for it.

 Revision 1.5.2.9  2005/11/09 12:22:18  dgrisby
 Local interfaces support.

 Revision 1.5.2.8  2005/01/17 14:33:22  dgrisby
 Standard StringValue and WStringValue typedefs.

 Revision 1.5.2.7  2004/10/13 17:58:18  dgrisby
 Abstract interfaces support; values support interfaces; value bug fixes.

 Revision 1.5.2.6  2004/07/23 10:29:56  dgrisby
 Completely new, much simpler Any implementation.

 Revision 1.5.2.5  2004/04/02 13:26:25  dgrisby
 Start refactoring TypeCode to support value TypeCodes, start of
 abstract interfaces support.

 Revision 1.5.2.4  2004/02/16 10:10:28  dgrisby
 More valuetype, including value boxes. C++ mapping updates.

 Revision 1.5.2.3  2003/10/23 11:25:54  dgrisby
 More valuetype support.

 Revision 1.5.2.2  2003/09/26 16:12:53  dgrisby
 Start of valuetype support.

 Revision 1.5.2.1  2003/03/23 21:04:26  dgrisby
 Start of omniORB 4.1.x development branch.

 Revision 1.2.2.22  2002/01/09 11:35:20  dpg1
 Remove separate omniAsyncInvoker library to save library overhead.

 Revision 1.2.2.21  2001/11/08 16:33:49  dpg1
 Local servant POA shortcut policy.

 Revision 1.2.2.20  2001/10/17 16:43:59  dpg1
 Update DynAny to CORBA 2.5 spec, const Any exception extraction.

 Revision 1.2.2.19  2001/08/17 13:39:43  dpg1
 Split CORBA.h into separate bits.

 Revision 1.2.2.18  2001/08/15 10:19:29  dpg1
 _name and _rep_id methods in CORBA::Exception.

 Revision 1.2.2.17  2001/08/03 17:45:39  sll
 Added minorCode.h

 Revision 1.2.2.16  2001/07/31 16:04:07  sll
 Added ORB::create_policy() and associated types and operators.

 Revision 1.2.2.15  2001/06/18 20:30:51  sll
 Only define 1 conversion operator from T_var to T* if the compiler is
 gcc. Previously, this is only done for gcc 2.7.2. It seems that gcc 3.0
 requires this to be the case. This is the default for all versions of
 gcc.

 Revision 1.2.2.14  2001/06/08 17:12:07  dpg1
 Merge all the bug fixes from omni3_develop.

 Revision 1.2.2.13  2001/04/18 17:50:45  sll
 Big checkin with the brand new internal APIs.
 Scoped where appropriate with the omni namespace.

 Revision 1.2.2.12  2001/04/09 15:18:46  dpg1
 Tweak fixed point to make life easier for omniORBpy.

 Revision 1.2.2.11  2001/03/13 10:32:04  dpg1
 Fixed point support.

 Revision 1.2.2.10  2001/01/08 12:35:41  dpg1
 _duplicate in CORBA::Object::_narrow

 Revision 1.2.2.9  2000/11/20 14:40:03  sll
 Added TypeCode::PR_wstring_tc(CORBA::ULong bound).

 Revision 1.2.2.8  2000/11/17 19:09:36  dpg1
 Support codeset conversion in any.

 Revision 1.2.2.7  2000/11/15 17:03:14  sll
 Moved include codeSets.h to omniInternal.h

 Revision 1.2.2.6  2000/11/09 12:27:48  dpg1
 Huge merge from omni3_develop, plus full long long from omni3_1_develop.

 Revision 1.2.2.5  2000/11/07 18:18:47  sll
 Added external guard in CORBA.h to ensure that any idl that include
 CosNaming.idl will compile as expected.

 Revision 1.2.2.4  2000/11/03 18:58:46  sll
 Unbounded sequence of octet got a new type name.

 Revision 1.2.2.3  2000/10/27 15:42:02  dpg1
 Initial code set conversion support. Not yet enabled or fully tested.

 Revision 1.2.2.2  2000/09/27 17:19:12  sll
 Updated to use the new cdrStream abstraction.
 Replace include/omniORB3 with include/omniORB4.

 Revision 1.2.2.1  2000/07/17 10:35:32  sll
 Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

 Revision 1.3  2000/07/13 15:26:06  dpg1
 Merge from omni3_develop for 3.0 release.

 Revision 1.1.2.9  2000/07/10 13:06:44  dpg1
 Initialisation of Any insertion functions was missing from system
 exceptions.

 Revision 1.1.2.8  2000/06/27 16:15:07  sll
 New classes: _CORBA_String_element, _CORBA_ObjRef_Element,
 _CORBA_ObjRef_tcDesc_arg to support assignment to an element of a
 sequence of string and a sequence of object reference.

 Revision 1.1.2.7  2000/06/02 14:15:17  dpg1
 SystemException constructors now public so exceptions caught by base
 class can be rethrown

 Revision 1.1.2.6  2000/03/03 14:29:15  djr
 Improvement to BOA skeletons (less generated code).

 Revision 1.1.2.5  1999/10/21 11:04:59  djr
 Added _core_attr to declarations of _PD_repoId in interfaces.

 Revision 1.1.2.4  1999/10/18 17:28:17  djr
 Fixes for building MSVC dlls.

 Revision 1.1.2.3  1999/10/18 11:27:36  djr
 Centralised list of system exceptions.

 Revision 1.1.2.2  1999/10/16 13:22:51  djr
 Changes to support compiling on MSVC.

 Revision 1.1.2.1  1999/09/24 09:51:36  djr
 Moved from omniORB2 + some new files.

 Revision 1.46  1999/08/30 18:45:58  sll
 Made #include ir stubs conditional on ENABLE_CLIENT_IR_SUPPORT.
 Application code has to define ENABLE_CLIENT_IR_SUPPORT in order to act
 as a client for an interface repository.

 Revision 1.45  1999/08/30 17:40:22  sll
 Removed use of _T and _T_var.

 Revision 1.44  1999/08/24 12:36:44  djr
 PR_structMember now uses 'const char*'.

 Revision 1.43  1999/08/16 19:33:24  sll
 New method NP_destroy() in class CORBA::ORB.

 Revision 1.42  1999/08/15 13:51:33  sll
 Define ImplmentationDef as an empty class to keep some compilers happy.

 Revision 1.41  1999/06/27 16:44:42  sll
 enclose Any extraction operator for string with test for macro
 _NO_ANY_STRING_EXTRACTION_. Define this macro as compiler option would
 remove the operators from the declaration. This make it easier to
 find the code in the source where the operators are used. Hence make it
 easier to fix old code which uses the pre-2.8.0 semantics.

 Revision 1.40  1999/06/25 13:54:17  sll
 Removed Any::operator>>=(Object_ptr&) const.

 Revision 1.39  1999/06/22 15:04:29  sll
 Put back pre CORBA-2.3 operator>>=(Any&a) in Any_var.

 Revision 1.38  1999/06/18 20:32:59  sll
 Updated to CORBA 2.3 mapping.

 Revision 1.37  1999/06/03 17:10:32  sll
 Added T_out types and updated T_var types to CORBA 2.2

 Revision 1.36  1999/06/02 16:07:44  sll
 Enabled IR client support for all platforms. Previously, IR client is
 not available with compilers that do not support namespace.

 Revision 1.35  1999/05/26 12:21:35  sll
 Use ENABLE_CLIENT_IR_SUPPORT alone to enable IR support.

 Revision 1.34  1999/05/25 13:37:44  sll
 Added missing CORBA 2.1 definitions.
 Added pd_magic field, PR_is_valid() static member functions to pseudo
 objects so that at runtime static member functions of these objects
 can check if the argument pointer is valid.

 Revision 1.33  1999/04/21 15:23:51  djr
 CORBA::ORB::ObjectIdList corrected to use new string sequence class.

 Revision 1.32  1999/04/21 13:41:10  djr
 Added marshalling methods to CORBA::Context.
 String types are now defined in stringtypes.h, and typedef inside the
 CORBA module.

 Revision 1.31  1999/02/18 15:23:29  djr
 New type CORBA::Request_member. Corrected CORBA::ORB::RequestSeq to use
 this. CORBA::ORB::get_next_response now has Request_out parameter.

 Revision 1.30  1999/01/11 16:39:51  djr
 Added guard to prevent attempt to include corbaidl.hh from ir.hh. This
 fails as it is in omniORB2/corbaidl.hh. It doesn't need to be included
 from ir.hh, since it has already been included into CORBA.h.

 Revision 1.29  1999/01/07 18:14:11  djr
 Changes to support
  - New implementation of TypeCode and Any
  - New implementation of DynAny
  - DII and DSI
  - Other minor changes.

 Revision 1.28  1998/08/25 18:55:33  sll
 Added dummy marshalling functions in DynAny_member to keep gcc-2.7.2 happy.

 Revision 1.27  1998/08/21 19:26:48  sll
 New private function _omni_set_NameService.

 Revision 1.26  1998/08/19 15:59:00  sll
 All <<= and >>= operators are now defined in the global namespace.
 In particular, the operator>>= and <<= for DefinitionKind have been
 moved out of the namespace CORBA. This change should have no effect
 on platforms which do not support C++ namespace. On platforms with
 C++ namespace, the new code is expected to work whether or not the
 compiler supports koenig lookup rule.

 Revision 1.25  1998/08/15 15:07:08  sll
 *** empty log message ***

 Revision 1.24  1998/08/15 14:22:04  sll
 Remove inline member implementations in _nil_TypeCode and IRObject.

 Revision 1.23  1998/08/05 18:10:39  sll
 Added DynAny.

 Revision 1.22  1998/04/18 10:07:28  sll
 Renamed __e with _0RL_e in operator<<=() of DefinitionKind.

 Revision 1.21  1998/04/08 13:56:47  sll
 Minor change to the ctor of _nil_TypeCode to help some compiler to find
 the right TypeCode ctor.

 Revision 1.20  1998/04/07 19:55:53  sll
 Updated to use namespace if available.
 Moved inline functions of Any and TypeCode out of this header file.

 * Revision 1.19  1998/02/20  14:44:44  ewc
 * Changed to compile with aCC on HPUX
 *
 * Revision 1.18  1998/02/03  16:47:09  ewc
 * Updated some interfaces.
 *
 * Revision 1.17  1998/01/27  16:02:34  ewc
 * Added TypeCode and type Any
 *
 Revision 1.16  1997/12/18 17:37:20  sll
 Added (const char*) type casting to help strcpy().

 Revision 1.15  1997/12/09 20:35:16  sll
 New members BOA::impl_shutdown, BOA::destroy.

 Revision 1.14  1997/08/21 22:20:17  sll
 - String_member copy ctor bug fix.
 - New system exception TRANSACTION_REQUIRED, TRANSACTION_ROLLEDBACK,
   INVALID_TRANSACTION and WRONG_TRANSACTION.
 - Correct ORB_init() signature.
 - CORBA::is_nil(Object_ptr) is now more sympathetic to applications
   treating a nil pointer as a nil object reference.

 * Revision 1.13  1997/05/21  15:01:40  sll
 * Added typedef <type>_ptr <type>Ref;
 *
 * Revision 1.12  1997/05/06  16:04:43  sll
 * Public release.
 *
*/

#ifndef __CORBA_H__
#define __CORBA_H__

#ifndef __CORBA_H_EXTERNAL_GUARD__
# define __CORBA_H_EXTERNAL_GUARD__
#endif

#ifdef _core_attr
# error "A local CPP macro _core_attr has already been defined."
#endif

#ifdef _dyn_attr
# error "A local CPP macro _dyn_attr has already been defined."
#endif

#if defined(_OMNIORB_LIBRARY) && defined(_OMNIORB_DYNAMIC_LIBRARY)
# error "_OMNIORB_LIBRARY and _OMNIORB_DYNAMIC_LIBRARY are both defined."
#endif

#if    defined(_OMNIORB_LIBRARY)
#         define _core_attr
#         define _dyn_attr  _OMNIORB_NTDLL_IMPORT
#elif  defined(_OMNIORB_DYNAMIC_LIBRARY)
#         define _core_attr _OMNIORB_NTDLL_IMPORT
#         define _dyn_attr
#else
#         define _core_attr _OMNIORB_NTDLL_IMPORT
#         define _dyn_attr  _OMNIORB_NTDLL_IMPORT
#endif

#ifndef USE_omniORB_logStream
#define USE_omniORB_logStream
#endif

#include <omniORB4/omniInternal.h>

// Forward declarations.
class omniOrbBoaServant;
class _omni_ValueFactoryManager;
struct _omni_ValueIds;
class cdrAnyMemoryStream;

_CORBA_MODULE CORBA

_CORBA_MODULE_BEG

# define INSIDE_OMNIORB_CORBA_MODULE

  //////////////////////////////////////////////////////////////////////
  ////////////////////////////// Forward decls /////////////////////////
  //////////////////////////////////////////////////////////////////////

  class Object;
  class Object_var;
  class Object_OUT_arg;
  typedef Object* Object_ptr;
  typedef Object_ptr ObjectRef;

  class TypeCode;
  typedef TypeCode* TypeCode_ptr;
  typedef TypeCode_ptr TypeCodeRef;

  class Fixed;
  class Any;
  class ValueBase;

  class AbstractBase;
  typedef AbstractBase* AbstractBase_ptr;

  //////////////////////////////////////////////////////////////////////
  ////////////////////// Definitions ///////////////////////////////////
  //////////////////////////////////////////////////////////////////////

# include "CORBA_primitive_types.h"
# include "CORBA_String.h"
# include "CORBA_Exception.h"
# include "CORBA_Environment.h"
# include "CORBA_NamedValue.h"
# include "CORBA_Context.h"
# include "CORBA_Principal.h"
# include "CORBA_ExceptionList.h"
# include "CORBA_TypeCode_member.h"
# include "CORBA_Object.h"
# include "CORBA_Object_vartypes.h"
# include "CORBA_LocalObject.h"
# include "CORBA_LocalObject_vartypes.h"
# include "CORBA_Any.h"
# include "CORBA_Any_vartypes.h"

  //////////////////////////////////////////////////////////////////////
  ////////////////////// Generated from corbaidl.idl ///////////////////
  //////////////////////////////////////////////////////////////////////

# if defined(_OMNIORB_LIBRARY)
#    undef   _core_attr
#    define  _core_attr  _OMNIORB_NTDLL_IMPORT
# include <omniORB4/corbaidl_defs.hh>
#    undef   _core_attr
#    define  _core_attr
# elif defined(_OMNIORB_DYNAMIC_LIBRARY)
#    undef   _core_attr
#    define  _core_attr
# include <omniORB4/corbaidl_defs.hh>
#    undef   _core_attr
#    define  _core_attr  _OMNIORB_NTDLL_IMPORT
# else
# include <omniORB4/corbaidl_defs.hh>
# endif

  //////////////////////////////////////////////////////////////////////
  ////////////////////// More Definitions //////////////////////////////
  //////////////////////////////////////////////////////////////////////

# include "CORBA_TypeCode.h"
# include "CORBA_Request.h"
# include "CORBA_ServerRequest.h"
# include "CORBA_ValueBase.h"
# include "CORBA_ValueBase_vartypes.h"
# include "CORBA_AbstractBase.h"
# include "CORBA_UnknownUserException.h"
# include "CORBA_Policy.h"
# include "CORBA_Current.h"
# include "CORBA_DomainManager.h"
# include "CORBA_BOA.h"
# include "CORBA_ORB.h"
# include "CORBA_Fixed.h"
# include "CORBA_static_fns.h"
# include "CORBA_vartypes.h"

  //////////////////////////////////////////////////////////////////////
  ////////////////////// Generated from ir.idl /////////////////////////
  //////////////////////////////////////////////////////////////////////

#if defined(ENABLE_CLIENT_IR_SUPPORT)
#if defined(_OMNIORB_LIBRARY)
#    undef   _core_attr
#    define  _core_attr  _OMNIORB_NTDLL_IMPORT
#include <omniORB4/ir_defs.hh>
#    undef   _core_attr
#    define  _core_attr
#elif defined(_OMNIORB_DYNAMIC_LIBRARY)
#    undef   _core_attr
#    define  _core_attr
#include <omniORB4/ir_defs.hh>
#    undef   _core_attr
#    define  _core_attr  _OMNIORB_NTDLL_IMPORT
#else
#include <omniORB4/ir_defs.hh>
#endif
#endif

  //////////////////////////////////////////////////////////////////////
  ////////////////////// Generated from boxes.idl //////////////////////
  //////////////////////////////////////////////////////////////////////

#if defined(_OMNIORB_LIBRARY)
#    undef   _core_attr
#    define  _core_attr  _OMNIORB_NTDLL_IMPORT
#include <omniORB4/boxes_defs.hh>
#    undef   _core_attr
#    define  _core_attr
#elif defined(_OMNIORB_DYNAMIC_LIBRARY)
#    undef   _core_attr
#    define  _core_attr
#include <omniORB4/boxes_defs.hh>
#    undef   _core_attr
#    define  _core_attr  _OMNIORB_NTDLL_IMPORT
#else
#include <omniORB4/boxes_defs.hh>
#endif

#  undef INSIDE_OMNIORB_CORBA_MODULE

_CORBA_MODULE_END


//?? These really want to be renamed and put elsewhere.
extern CORBA::Boolean
_omni_callTransientExceptionHandler(omniObjRef* obj, CORBA::ULong retries,
				    const CORBA::TRANSIENT& ex);
extern CORBA::Boolean
_omni_callCommFailureExceptionHandler(omniObjRef* obj,
				      CORBA::ULong retries,
				      const CORBA::COMM_FAILURE& ex);
extern CORBA::Boolean
_omni_callSystemExceptionHandler(omniObjRef* obj, CORBA::ULong retries,
				 const CORBA::SystemException& ex);


extern void _omni_set_NameService(CORBA::Object_ptr);

#include <omniORB4/omniIOR.h>
#include <omniORB4/omniORB.h>
#include <omniORB4/proxyFactory.h>
#include <omniORB4/valueType.h>

#include <omniORB4/templatedefns.h>
#include <omniORB4/corba_operators.h>
#include <omniORB4/poa.h>
#include <omniORB4/fixed.h>
#include <omniORB4/BiDirPolicy.h>
#include <omniORB4/omniPolicy.h>
#include <omniORB4/dynAny.h>

#include <omniORB4/minorCode.h>

#include <omniORB4/omniAsyncInvoker.h>

#include <omniORB4/corbaidl_operators.hh>

#if defined(ENABLE_CLIENT_IR_SUPPORT)
#  include <omniORB4/ir_operators.hh>
#endif

#include <omniORB4/boxes_operators.hh>


_CORBA_MODULE POA_CORBA
_CORBA_MODULE_BEG

#include <omniORB4/corbaidl_poa.hh>
#if defined(ENABLE_CLIENT_IR_SUPPORT)
#  include <omniORB4/ir_poa.hh>
#endif
#include <omniORB4/boxes_poa.hh>

_CORBA_MODULE_END

#include <omniORB4/boa.h>

#undef _core_attr
#undef _dyn_attr


#if !defined(_OMNIORB_LIBRARY) && !defined(_OMNIORB_DYNAMIC_LIBRARY)
#ifndef USE_core_stub_in_nt_dll
#define USE_core_stub_in_nt_dll
#define USE_core_stub_in_nt_dll_NOT_DEFINED
#endif
#ifndef USE_dyn_stub_in_nt_dll
#define USE_dyn_stub_in_nt_dll
#define USE_dyn_stub_in_nt_dll_NOT_DEFINED
#endif
#include <omniORB4/Naming.hh>
#ifdef  USE_core_stub_in_nt_dll_NOT_DEFINED
#undef  USE_core_stub_in_nt_dll
#undef  USE_core_stub_in_nt_dll_NOT_DEFINED
#endif
#ifdef  USE_dyn_stub_in_nt_dll_NOT_DEFINED
#undef  USE_dyn_stub_in_nt_dll
#undef  USE_dyn_stub_in_nt_dll_NOT_DEFINED
#endif
#endif

// OMG COS IDLs refer to CosNaming IDL as "CosNaming.idl".
// omniORB uses the file name "Naming.idl". Any IDLs that include
// CosNaming.idl will have in their stubs #include "CosNaming.hh".
// Define the external guard for CosNaming to stop include to have
// any effect. This works because the stub generated by omniidl
// put external guards around the include.
#ifndef __CosNaming_hh_EXTERNAL_GUARD__
#define __CosNaming_hh_EXTERNAL_GUARD__
#endif

#ifndef __corbaidl_hh_EXTERNAL_GUARD__
#define __corbaidl_hh_EXTERNAL_GUARD__
#endif
#ifdef ENABLE_CLIENT_IR_SUPPORT
#ifndef __ir_hh_EXTERNAL_GUARD__
#define __ir_hh_EXTERNAL_GUARD__
#endif
#endif
#ifndef __boxes_hh_EXTERNAL_GUARD__
#define __boxes_hh_EXTERNAL_GUARD__
#endif

#endif // __CORBA_H__
