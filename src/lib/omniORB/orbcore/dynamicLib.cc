// -*- Mode: C++; -*-
//                            Package   : omniORB
// dynamicLib.cc              Created on: 15/9/99
//                            Author    : David Riddoch (djr)
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
//    Stubs for dynamic library 'hook' functions.
//

/*
  $Log$
  Revision 1.2.2.7  2001/08/21 11:02:14  sll
  orbOptions handlers are now told where an option comes from. This
  is necessary to process DefaultInitRef and InitRef correctly.

  Revision 1.2.2.6  2001/08/17 17:12:36  sll
  Modularise ORB configuration parameters.

  Revision 1.2.2.5  2001/08/15 10:31:23  dpg1
  Minor tweaks and fixes.

  Revision 1.2.2.4  2001/08/03 17:41:20  sll
  System exception minor code overhaul. When a system exeception is raised,
  a meaning minor code is provided.

  Revision 1.2.2.3  2001/04/18 18:18:08  sll
  Big checkin with the brand new internal APIs.

  Revision 1.2.2.2  2000/09/27 17:59:38  sll
  Updated to use the new cdrStream abstraction.

  Revision 1.2.2.1  2000/07/17 10:35:53  sll
  Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.

  Revision 1.3  2000/07/13 15:25:58  dpg1
  Merge from omni3_develop for 3.0 release.

  Revision 1.1.2.4  2000/06/22 10:40:14  dpg1
  exception.h renamed to exceptiondefs.h to avoid name clash on some
  platforms.

  Revision 1.1.2.3  1999/10/14 16:22:08  djr
  Implemented logging when system exceptions are thrown.

  Revision 1.1.2.2  1999/09/27 11:01:11  djr
  Modifications to logging.

  Revision 1.1.2.1  1999/09/22 14:26:48  djr
  Major rewrite of orbcore to support POA.

*/

#include <omniORB4/CORBA.h>
#include <dynamicLib.h>
#include <exceptiondefs.h>
#include <initialiser.h>
#include <orbOptions.h>
#include <orbParameters.h>

////////////////////////////////////////////////////////////////////////////
//             Configuration options                                      //
////////////////////////////////////////////////////////////////////////////

CORBA::Boolean  omniORB::omniORB_27_CompatibleAnyExtraction = 0;
// In omniORB pre-2.8.0 versions, the CORBA::Any extraction operator for
//   1. unbounded string operator>>=(char*&)
//   2. bounded string   operator>>=(to_string)
//   3. object reference operator>>=(A_ptr&) for interface A
// Returns a copy of the value. The caller must free the returned
// value later.
//
// With 2.8.0 and later, the semantics becomes non-copy, i.e. the Any
// still own the storage of the returned value.
// This would cause problem in programs that is written to use the
// pre-2.8.0 semantics. To make it easier for the transition,
// set omniORB_27_CompatibleAnyExtraction to 1.
// This would revert the semantics to the pre-2.8.0 versions.
//
// Valid values = 0 or 1

OMNI_NAMESPACE_BEGIN(omni)

CORBA::Boolean  orbParameters::tcAliasExpand          = 0;
// This flag is used to indicate whether TypeCodes associated with anys
// should have aliases removed. This functionality is included because
// some ORBs will not recognise an Any containing a TypeCode containing
// aliases to be the same as the actual type contained in the Any. Note
// that omniORB will always remove top-level aliases, but will not remove
// aliases from TypeCodes that are members of other TypeCodes (e.g.
// TypeCodes for members of structs etc.), unless tcAliasExpand is set to 1.
// There is a performance penalty when inserting into an Any if 
// tcAliasExpand is set to 1. The default value is 0 (i.e. aliases of
// member TypeCodes are not expanded). Note that aliases won't be expanded
// when one of the non-type-safe methods of inserting into an Any is
// used (i.e. when the replace() member function or non - type-safe Any
// constructor is used. )
//
//  Valid values = 0 or 1

CORBA::Boolean  orbParameters::diiThrowsSysExceptions = 0;
// If the value of this variable is 1 then the Dynamic Invacation Interface
// functions (Request::invoke, send_oneway, send_deferred, get_response,
// poll_response) will throw system exceptions as appropriate. Otherwise 
// the exception will be stored in the Environment pseudo object associated
// with the Request. By default system exceptions are passed through the 
// Environment object.
//
// Valid values = 0 or 1


CORBA::Boolean  orbParameters::useTypeCodeIndirections = 1;
// If true (the default), typecode indirectional will be used. Set
// this to false to disable that. Setting this to false might be
// useful to interoperate with another ORB implementation that cannot
// handle indirectional properly.
//
// Valid values = 0 or 1

CORBA::Boolean  orbParameters::acceptMisalignedTcIndirections = 0;
// If true, try to fix a mis-aligned indirection in a typecode. This
// could be used to work around some versions of Visibroker's Java ORB.
//
// Valid values = 0 or 1


////////////////////////////////////////////////////////////////////////////
static void init();
static void deinit();
static void marshal_context(cdrStream&, CORBA::Context_ptr cxtx,
			    const char*const* which, int how_many);
static void lookup_id_lcfn(omniCallDescriptor* cd, omniServant* svnt);


static omniDynamicLib orbcore_ops = {
  init,
  deinit,
  marshal_context,
  lookup_id_lcfn
};

omniDynamicLib* omniDynamicLib::ops = &orbcore_ops;
omniDynamicLib* omniDynamicLib::hook = 0;


static void
init()
{
  omniORB::logs(2, "Information: the omniDynamic library is not linked.");
}


static void
deinit()
{
}

static void
marshal_context(cdrStream& s, CORBA::Context_ptr cxtx,
		const char*const* which, int how_many)
{
  omniORB::logs(1, "Attempt to marshal context, but omniDynamic library"
		" is not linked!");
  OMNIORB_THROW(NO_IMPLEMENT,NO_IMPLEMENT_Unsupported, 
		(CORBA::CompletionStatus)s.completion());
}


static void
lookup_id_lcfn(omniCallDescriptor* cd, omniServant* svnt)
{
  // Can't have a local call to a repository if the dynamic
  // library is not linked ...
  OMNIORB_ASSERT(0);
}

/////////////////////////////////////////////////////////////////////////////
//            Handlers for Configuration Options                           //
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
class tcAliasExpandHandler : public orbOptions::Handler {
public:

  tcAliasExpandHandler() : 
    orbOptions::Handler("tcAliasExpand",
			"tcAliasExpand = 0 or 1",
			1,
			"-ORBtcAliasExpand < 0 | 1 >") {}


  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam) {

    CORBA::Boolean v;
    if (!orbOptions::getBoolean(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_boolean_msg);
    }
    orbParameters::tcAliasExpand = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVBoolean(key(),orbParameters::tcAliasExpand,
			     result);
  }
};

static tcAliasExpandHandler tcAliasExpandHandler_;

/////////////////////////////////////////////////////////////////////////////
class diiThrowsSysExceptionsHandler : public orbOptions::Handler {
public:

  diiThrowsSysExceptionsHandler() : 
    orbOptions::Handler("diiThrowsSysExceptions",
			"diiThrowsSysExceptions = 0 or 1",
			1,
			"-ORBdiiThrowsSysExceptions < 0 | 1 >") {}


  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam) {

    CORBA::Boolean v;
    if (!orbOptions::getBoolean(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_boolean_msg);
    }
    orbParameters::diiThrowsSysExceptions = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVBoolean(key(),orbParameters::diiThrowsSysExceptions,
			     result);
  }
};

static diiThrowsSysExceptionsHandler diiThrowsSysExceptionsHandler_;

/////////////////////////////////////////////////////////////////////////////
class omniORB_27_CompatibleAnyExtractionHandler : public orbOptions::Handler {
public:

  omniORB_27_CompatibleAnyExtractionHandler() : 
    orbOptions::Handler("omniORB_27_CompatibleAnyExtraction",
			"omniORB_27_CompatibleAnyExtraction = 0 or 1",
			1,
			"-ORBomniORB_27_CompatibleAnyExtraction < 0 | 1 >") {}


  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam) {

    CORBA::Boolean v;
    if (!orbOptions::getBoolean(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_boolean_msg);
    }
    omniORB::omniORB_27_CompatibleAnyExtraction = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVBoolean(key(),omniORB::omniORB_27_CompatibleAnyExtraction,
			     result);
  }
};

static omniORB_27_CompatibleAnyExtractionHandler omniORB_27_CompatibleAnyExtractionHandler_;

/////////////////////////////////////////////////////////////////////////////
class useTypeCodeIndirectionsHandler : public orbOptions::Handler {
public:

  useTypeCodeIndirectionsHandler() : 
    orbOptions::Handler("useTypeCodeIndirections",
			"useTypeCodeIndirections = 0 or 1",
			1,
			"-ORBuseTypeCodeIndirections < 0 | 1 >") {}


  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam) {

    CORBA::Boolean v;
    if (!orbOptions::getBoolean(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_boolean_msg);
    }
    orbParameters::useTypeCodeIndirections = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVBoolean(key(),orbParameters::useTypeCodeIndirections,
			     result);
  }
};

static useTypeCodeIndirectionsHandler useTypeCodeIndirectionsHandler_;

/////////////////////////////////////////////////////////////////////////////
class acceptMisalignedTcIndirectionsHandler : public orbOptions::Handler {
public:

  acceptMisalignedTcIndirectionsHandler() : 
    orbOptions::Handler("acceptMisalignedTcIndirections",
			"acceptMisalignedTcIndirections = 0 or 1",
			1,
			"-ORBacceptMisalignedTcIndirections < 0 | 1 >") {}


  void visit(const char* value,orbOptions::Source) throw (orbOptions::BadParam) {

    CORBA::Boolean v;
    if (!orbOptions::getBoolean(value,v)) {
      throw orbOptions::BadParam(key(),value,
				 orbOptions::expect_boolean_msg);
    }
    orbParameters::acceptMisalignedTcIndirections = v;
  }

  void dump(orbOptions::sequenceString& result) {
    orbOptions::addKVBoolean(key(),orbParameters::acceptMisalignedTcIndirections,
			     result);
  }
};

static acceptMisalignedTcIndirectionsHandler acceptMisalignedTcIndirectionsHandler_;

/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////

class omni_dynamiclib_initialiser : public omniInitialiser {
public:

  omni_dynamiclib_initialiser() {
    orbOptions::singleton().registerHandler(tcAliasExpandHandler_);
    orbOptions::singleton().registerHandler(diiThrowsSysExceptionsHandler_);
    orbOptions::singleton().registerHandler(omniORB_27_CompatibleAnyExtractionHandler_);
    orbOptions::singleton().registerHandler(useTypeCodeIndirectionsHandler_);
    orbOptions::singleton().registerHandler(acceptMisalignedTcIndirectionsHandler_);
  }

  void attach() {
    if( omniDynamicLib::hook )
      omniDynamicLib::ops = omniDynamicLib::hook;
    omniDynamicLib::ops->init();
  }
  void detach() {
    omniDynamicLib::ops->deinit();
  }
};


static omni_dynamiclib_initialiser initialiser;

omniInitialiser& omni_dynamiclib_initialiser_ = initialiser;

OMNI_NAMESPACE_END(omni)
