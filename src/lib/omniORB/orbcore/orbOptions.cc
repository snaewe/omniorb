// -*- Mode: C++; -*-
//                            Package   : omniORB
// orbOptions.cc              Created on: 13/8/2001
//                            Author    : Sai Lai Lo (sll)
//
//    Copyright (C) 2001 AT&T Laboratories Cambridge
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
  Revision 1.1.2.2  2001/08/20 08:19:23  sll
  Read the new ORB configuration file format. Can still read old format.
  Can also set configuration parameters from environment variables.

  Revision 1.1.2.1  2001/08/17 17:12:41  sll
  Modularise ORB configuration parameters.

*/

#include <omniORB4/CORBA.h>
#include <orbOptions.h>
#include <initialiser.h>
#include <stdio.h>
#include <stdlib.h>


OMNI_NAMESPACE_BEGIN(omni)

////////////////////////////////////////////////////////////////////////
orbOptions::Handler* 
orbOptions::findHandler(const char* k) {

  //  if (!pd_handlers_sorted) sortHandlers();

  omnivector<orbOptions::Handler*>::iterator i = pd_handlers.begin();
  omnivector<orbOptions::Handler*>::iterator last = pd_handlers.end();
  
  for (; i != last; i++) {
    if (strcmp((*i)->key(),k) == 0)
      return (*i);
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////
void
orbOptions::sortHandlers() {
  // Won't it be nice to just use stl qsort? It is tempting to just
  // forget about old C++ compiler and use stl. Until the time has come
  // here is a little bit of code to sort the handlers in alphabetical
  // order of their key(). The algorithm is shell sort.

  int n = pd_handlers.size();
  for (int gap=n/2; gap > 0; gap=gap/2 ) {
    for (int i=gap; i < n ; i++)
      for (int j =i-gap; j>=0; j=j-gap) {
	if (strcmp( (pd_handlers[j])->key(),
		    (pd_handlers[j+gap])->key() ) > 0) {
	  Handler* temp = pd_handlers[j];
	  pd_handlers[j] = pd_handlers[j+gap];
	  pd_handlers[j+gap] = temp;
	}
      }
  }
  pd_handlers_sorted = 1;
}

////////////////////////////////////////////////////////////////////////
orbOptions::orbOptions() : pd_handlers_sorted(0) {}

////////////////////////////////////////////////////////////////////////
orbOptions::~orbOptions() {
  reset();
}

////////////////////////////////////////////////////////////////////////
void
orbOptions::registerHandler(orbOptions::Handler& h) {

  OMNIORB_ASSERT(findHandler(h.key()) == 0);
  pd_handlers.push_back(&h);
  pd_handlers_sorted = 0;
}

////////////////////////////////////////////////////////////////////////
void
orbOptions::reset() {
  omnivector<HandlerValuePair*>::iterator i = pd_values.begin();
  omnivector<HandlerValuePair*>::iterator last = pd_values.end();

  for (; i != last; i++) {
    delete (*i);
  }
  pd_values.erase(pd_values.begin(),last);
}

////////////////////////////////////////////////////////////////////////
void
orbOptions::visit() throw(orbOptions::BadParam) {

  omnivector<HandlerValuePair*>::iterator i = pd_values.begin();
  omnivector<HandlerValuePair*>::iterator last = pd_values.end();
  
  for (; i != last; i++) {
    (*i)->handler_->visit((*i)->value_);
  }
}

////////////////////////////////////////////////////////////////////////
void
orbOptions::addOption(const char* key,
		      const char* value) throw (orbOptions::Unknown,
						orbOptions::BadParam) {

  if (!pd_handlers_sorted) sortHandlers();

  orbOptions::Handler* handler = findHandler(key);
  if (!handler) throw orbOptions::Unknown(key,value);
  pd_values.push_back(new HandlerValuePair(handler,value));
}

////////////////////////////////////////////////////////////////////////
void
orbOptions::addOptions(const char* options[][2]) throw (orbOptions::Unknown,
							orbOptions::BadParam) {

  int i = 0;
  while (options[i][0]) {
    addOption(options[i][0],options[i][1]);
  }
}

////////////////////////////////////////////////////////////////////////
void
orbOptions::move_args(int& argc,char **argv,int idx,int nargs)
{
  if ((idx+nargs) <= argc) {
    for (int i=idx+nargs; i < argc; i++) {
      argv[i-nargs] = argv[i];
    }
    argc -= nargs;
  }
}

////////////////////////////////////////////////////////////////////////
void
orbOptions::extractInitOptions(int& argc,char** argv) 
  throw (orbOptions::Unknown,orbOptions::BadParam) {

  if (!pd_handlers_sorted) sortHandlers();

  omnivector<orbOptions::Handler*>::iterator i = pd_handlers.begin();
  omnivector<orbOptions::Handler*>::iterator last = pd_handlers.end();

  for (; i != last; i++) {

    if (!(*i)->argvYes()) continue;

    const char* k = (*i)->key();
    int idx = 0;
    while (idx < argc) {

      // -ORBxxxxxxx ?
      if (strlen(argv[idx]) < 4 ||
	  !(argv[idx][0] == '-' && argv[idx][1] == 'O' &&
	    argv[idx][2] == 'R' && argv[idx][3] == 'B'    )) {
	
	idx++;
	continue;
      }

      if (strcmp(argv[idx]+4,k) != 0) {
	idx++;
	continue;
      }

      if (!(*i)->argvHasNoValue()) {

	if ((idx+1) >= argc) {
	  throw orbOptions::BadParam(k,"<missing>",
				     "Expected parameter missing");
	}

	addOption(k,argv[idx+1]);
	move_args(argc,argv,idx,2);
      }
      else {
	addOption(k,0);
	move_args(argc,argv,idx,1);
      }
    }
  }

  // Now any -ORB option left are not supported
  {
    int idx = 0;
    while (idx < argc) {
      if ( strlen(argv[idx]) > 4 &&
	   (argv[idx][0] == '-' && argv[idx][1] == 'O' &&
	    argv[idx][2] == 'R' && argv[idx][3] == 'B'    ) ) {
	
	throw orbOptions::Unknown(argv[idx],"");
      }
      idx++;
    }
  }
  
}

////////////////////////////////////////////////////////////////////////
void
orbOptions::importFromEnv() {
  
  omnivector<orbOptions::Handler*>::const_iterator i = pd_handlers.begin();
  omnivector<orbOptions::Handler*>::const_iterator last = pd_handlers.end();

  for (; i != last; i++) {
    const char* value = getenv((*i)->key());
    if (value && strlen(value)) addOption((*i)->key(),value);
  }
}

////////////////////////////////////////////////////////////////////////
orbOptions::sequenceString*
orbOptions::usage() const {

  if (!pd_handlers_sorted) ((orbOptions*)this)->sortHandlers();

  sequenceString_var result(new sequenceString(pd_handlers.size()));

  result->length(pd_handlers.size());
  
  omnivector<orbOptions::Handler*>::const_iterator i = pd_handlers.begin();
  omnivector<orbOptions::Handler*>::const_iterator last = pd_handlers.end();

  int j = 0;
  for (; i != last; i++) {
    if ((*i)->usage()) {
      result[j] = (*i)->usage();
      j++;
    }
  }
  result->length(j);  // some options may be obsoluted. They are those
                      // with no usage strings. We have to adjust the length
                      // to cater for their omission.
  return result._retn();
}

////////////////////////////////////////////////////////////////////////
orbOptions::sequenceString*
orbOptions::usageArgv() const {

  if (!pd_handlers_sorted) ((orbOptions*)this)->sortHandlers();

  sequenceString_var result(new sequenceString(pd_handlers.size()));

  result->length(pd_handlers.size());
  
  omnivector<orbOptions::Handler*>::const_iterator i = pd_handlers.begin();
  omnivector<orbOptions::Handler*>::const_iterator last = pd_handlers.end();

  int j = 0;
  for (; i != last; i++) {
    if ((*i)->usageArgv()) {
      result[j] = (*i)->usageArgv();
      j++;
    }
  }
  result->length(j);  // some options may be obsoluted. They are those
                      // with no usage strings. We have to adjust the length
                      // to cater for their omission.
  return result._retn();
}

////////////////////////////////////////////////////////////////////////
orbOptions::sequenceString*
orbOptions::dumpSpecified() const {

  sequenceString_var result(new sequenceString(pd_values.size()));
  
  omnivector<HandlerValuePair*>::const_iterator i = pd_values.begin();
  omnivector<HandlerValuePair*>::const_iterator last = pd_values.end();

  int j = 0;
  for (; i != last; i++,j++) {
    CORBA::String_var kv;
    CORBA::ULong l = strlen((*i)->handler_->key()) + strlen((*i)->value_) + 3;
    kv = CORBA::string_alloc(l);
    sprintf(kv,"%s = %s",
	    (*i)->handler_->key(),(const char*)((*i)->value_));
    result[j] = kv._retn();
  }
  return result._retn();

}

////////////////////////////////////////////////////////////////////////
orbOptions::sequenceString*
orbOptions::dumpCurrentSet() const {

  if (!pd_handlers_sorted) ((orbOptions*)this)->sortHandlers();

  sequenceString_var result(new sequenceString(pd_handlers.size()));

  omnivector<orbOptions::Handler*>::const_iterator i = pd_handlers.begin();
  omnivector<orbOptions::Handler*>::const_iterator last = pd_handlers.end();

  for (; i != last; i++) {
    (*i)->dump(result.inout());
  }
  return result._retn();
  
}


////////////////////////////////////////////////////////////////////////
orbOptions&
orbOptions::singleton() {
  static orbOptions* singleton_ = 0;
  if (!singleton_) {
    singleton_ = new orbOptions();
  }
  return *singleton_;
}


////////////////////////////////////////////////////////////////////////
CORBA::Boolean
orbOptions::getBoolean(const char* value, CORBA::Boolean& result) {

  long v;
  v = strtol(value,0,10);
  if (v != 0 && v != 1) return 0;
  result = v;
  return 1;
}

////////////////////////////////////////////////////////////////////////
CORBA::Boolean
orbOptions::getULong(const char* value, CORBA::ULong& result) {

  long v;
  v = strtol(value,0,10);
  if (v == LONG_MIN || v == LONG_MAX) return 0;
  result = v;
  return 1;
}

////////////////////////////////////////////////////////////////////////
void
orbOptions::addKVBoolean(const char* key, CORBA::Boolean value,
			 orbOptions::sequenceString& result) {

  CORBA::String_var kv;
  CORBA::ULong l;

  l = strlen(key) + 4;
  kv = CORBA::string_alloc(l);
  sprintf(kv,"%s = %s",key,(value ? "1" : "0"));

  l = result.length();
  result.length(l+1);
  result[l] = kv._retn();
}

////////////////////////////////////////////////////////////////////////
void
orbOptions::addKVULong(const char* key, CORBA::ULong value,
			 orbOptions::sequenceString& result) {

  CORBA::String_var kv;
  CORBA::ULong l;

  l = strlen(key) + 16;
  kv = CORBA::string_alloc(l);
  sprintf(kv,"%s = %lu",key,value);

  l = result.length();
  result.length(l+1);
  result[l] = kv._retn();
}

////////////////////////////////////////////////////////////////////////
void
orbOptions::addKVString(const char* key, const char* value,
			 orbOptions::sequenceString& result) {

  CORBA::String_var kv;
  CORBA::ULong l;

  l = strlen(key) + strlen(value) + 3;
  kv = CORBA::string_alloc(l);
  sprintf(kv,"%s = %s",key,value);

  l = result.length();
  result.length(l+1);
  result[l] = kv._retn();
}

////////////////////////////////////////////////////////////////////////
const char* orbOptions::expect_boolean_msg = "Invalid value, expect 0 or 1";
const char* orbOptions::expect_non_zero_ulong_msg = "Invalid value, expect n > 0";
const char* orbOptions::expect_greater_than_zero_ulong_msg = "Invalid value, expect n >= 1";

/////////////////////////////////////////////////////////////////////////////
//            Module initialiser                                           //
/////////////////////////////////////////////////////////////////////////////
//
// No need to register this initialiser to ORB_init.
//
class omni_orbOptions_initialiser : public omniInitialiser {
public:

  omni_orbOptions_initialiser() {
    orbOptions& s = orbOptions::singleton();
    s.reset();
  }
  virtual ~omni_orbOptions_initialiser() {
    orbOptions* s = &orbOptions::singleton();
    delete s;
  }

  void attach() { }
  void detach() { }
};


static omni_orbOptions_initialiser initialiser;

omniInitialiser& omni_orbOptions_initialiser_ = initialiser;

OMNI_NAMESPACE_END(omni)


