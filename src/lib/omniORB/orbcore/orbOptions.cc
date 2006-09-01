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
  Revision 1.1.2.15  2006/09/01 13:28:41  dgrisby
  Pass over unknown configuration options in config file / registry
  with only a warning.

  Revision 1.1.2.14  2005/09/29 11:31:51  dgrisby
  For loop scoping problem.

  Revision 1.1.2.13  2005/09/19 14:23:56  dgrisby
  New traceFile configuration parameter.

  Revision 1.1.2.12  2005/09/08 14:26:17  dgrisby
  New -ORBconfigFile command line argument.

  Revision 1.1.2.11  2005/09/05 14:31:08  dgrisby
  SSL transport extensions from Jon Biggar; SSL command line options.

  Revision 1.1.2.10  2004/03/30 13:15:28  dgrisby
  Remove spurious empty lines in config dump.

  Revision 1.1.2.9  2004/02/06 16:17:44  dgrisby
  Properly handle large giopMaxMsgSize settings.

  Revision 1.1.2.8  2003/11/05 13:00:33  dgrisby
  Properly set sequence length in dumpSpecified.

  Revision 1.1.2.7  2003/07/26 22:52:22  dgrisby
  Avoid spurious gcc warnings when sizeof pointer > sizeof int.

  Revision 1.1.2.6  2002/03/18 15:13:09  dpg1
  Fix bug with old-style ORBInitRef in config file; look for
  -ORBtraceLevel arg before anything else; update Windows registry
  key. Correct error message.

  Revision 1.1.2.5  2001/10/29 17:44:59  dpg1
  Missing loop increment.

  Revision 1.1.2.4  2001/10/19 11:06:45  dpg1
  Principal support for GIOP 1.0. Correct some spelling mistakes.

  Revision 1.1.2.3  2001/08/21 11:03:38  sll
  Environment variables to set the configuration parameters must now
  prefix with "ORB". For instance, environment variable ORBtraceLevel
  corresponds to parameter traceLevel.
  orbOptions handlers are now told where an option comes from. This
  is necessary to process DefaultInitRef and InitRef correctly.

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
#include <errno.h>


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
    (*i)->handler_->visit((*i)->value_,(*i)->source_);
  }
}

////////////////////////////////////////////////////////////////////////
void
orbOptions::addOption(const char* key,
		      const char* value,
		      orbOptions::Source source) throw (orbOptions::Unknown,
							orbOptions::BadParam) {

  if (!pd_handlers_sorted) sortHandlers();

  orbOptions::Handler* handler = findHandler(key);
  if (handler) {
    pd_values.push_back(new HandlerValuePair(handler,value,source));
  }
  else {
    switch (source) {
    case fromFile:
    case fromEnvironment:
    case fromRegistry:
      if (omniORB::trace(2)) {
	omniORB::logger log;
	log << "Warning: ignoring unknown configuration option '"
	    << key << "'.\n";
      }
      break;
    default:
      throw orbOptions::Unknown(key,value);
    }
  }
}

////////////////////////////////////////////////////////////////////////
void
orbOptions::addOptions(const char* options[][2]) throw (orbOptions::Unknown,
							orbOptions::BadParam) {

  for (int i=0; options[i][0]; i++) {
    addOption(options[i][0],options[i][1],fromArray);
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

	addOption(k,argv[idx+1],fromArgv);
	move_args(argc,argv,idx,2);
      }
      else {
	addOption(k,0,fromArgv);
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
orbOptions::getTraceLevel(int argc, char** argv)
  throw (orbOptions::Unknown,orbOptions::BadParam) {

  int i;
  for (i=0; i<argc; i++) {
    if (!strcmp(argv[i], "-ORBtraceLevel")) {
      if (i+1 == argc) {
	throw orbOptions::BadParam("traceLevel", "<missing>",
				   "Expected parameter missing");
      }
      CORBA::ULong v;
      if (!orbOptions::getULong(argv[i+1], v))
	throw orbOptions::BadParam("traceLevel", argv[i+1],
				   orbOptions::expect_ulong_msg);
      omniORB::traceLevel = v;
      if (v >= 10)
	omniORB::traceExceptions = 1;

      break;
    }
  }
  
  for (i=0; i<argc; i++) {
    if (!strcmp(argv[i], "-ORBtraceFile")) {
      if (i+1 == argc) {
	throw orbOptions::BadParam("traceFile", "<missing>",
				   "Expected parameter missing");
      }
      omniORB::setLogFilename(argv[i+1]);
      break;
    }
  }
}


////////////////////////////////////////////////////////////////////////
const char*
orbOptions::getConfigFileName(int argc, char** argv, const char* fname)
  throw (orbOptions::Unknown,orbOptions::BadParam) {

  for (int i=0; i<argc; i++) {
    if (!strcmp(argv[i], "-ORBconfigFile")) {
      if (i+1 == argc) {
	throw orbOptions::BadParam("configFile", "<missing>",
				   "Expected parameter missing");
      }
      return argv[i+1];
    }
  }
  return fname;
}


////////////////////////////////////////////////////////////////////////
void
orbOptions::importFromEnv() throw (orbOptions::Unknown,orbOptions::BadParam) {
  
  omnivector<orbOptions::Handler*>::const_iterator i = pd_handlers.begin();
  omnivector<orbOptions::Handler*>::const_iterator last = pd_handlers.end();

  for (; i != last; i++) {
    CORBA::String_var envkey;
    envkey = CORBA::string_alloc(strlen((*i)->key())+3);
    sprintf(envkey,"ORB%s",(*i)->key());
    const char* value = getenv(envkey);
    if (value && strlen(value)) addOption((*i)->key(),value,fromEnvironment);
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
  result->length(j);  // some options may be obsoleted. They are those
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
  result->length(j);  // some options may be obsoleted. They are those
                      // with no usage strings. We have to adjust the length
                      // to cater for their omission.
  return result._retn();
}

////////////////////////////////////////////////////////////////////////
orbOptions::sequenceString*
orbOptions::dumpSpecified() const {

  sequenceString_var result(new sequenceString(pd_values.size()));
  result->length(pd_values.size());

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

  sequenceString_var result(new sequenceString());

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

  unsigned long v;
  v = strtoul(value,0,10);
  if (v == ULONG_MAX && errno == ERANGE) return 0;
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
  sprintf(kv,"%s = %lu",key,(unsigned long)value);

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
const char* orbOptions::expect_ulong_msg = "Invalid value, expect n >= 0";
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


