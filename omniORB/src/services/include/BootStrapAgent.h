// -*- Mode: C++; -*-

#ifndef __BOOSTRAP_AGENT_H__
#define __BOOSTRAP_AGENT_H__

#include <stdlib.h>
#include <stdio.h>
#include "bootstrap.hh"
#include "Naming.hh" // not strictly necessary but simplifies life 

//////////////////////////////////////////////////////////////////
// 0: none, 1: simple; 2: full
#ifndef DIAGNOSTIC_LEVEL
#define DIAGNOSTIC_LEVEL 0
#endif

//////////////////////////////////////////////////////////////////

class BootStrapAgent {
 public:
  // use initialreferences bootstrapping mechanism to look for service
  // name id at the given host and port; return null if not found
  static CORBA::Object_ptr get(CORBA::ORB_ptr orb, const char* id, 
			       const char* host, CORBA::UShort port);

  // As above except uses defaults of current hostname and port 9999
  // Defaults can be overridden with environment variables
  //     BOOTSTRAP_AGENT_HOST  and  BOOTSTRAP_AGENT_PORT
  static CORBA::Object_ptr get(CORBA::ORB_ptr orb, const char* id);
};

//////////////////////////////////////////////////////////////////
// inline impl

#if defined(__sunos__) && defined(__sparc__) && __OSVERSION__ == 5
#define NEED_GETHOSTNAME_PROTOTYPE
#endif

// xxx above is not working because of missing defines
#if !defined(__hpux__)
#define NEED_GETHOSTNAME_PROTOTYPE
#endif

#ifdef NEED_GETHOSTNAME_PROTOTYPE
extern "C" int gethostname(char *name, int namelen);
#endif

// use initialreferences bootstrapping mechanism to look for service
// name id at the given host and port; return null if not found

inline CORBA::Object_ptr
BootStrapAgent::get(CORBA::ORB_ptr orb, const char* id) 
{
#if DIAGNOSTIC_LEVEL > 0
  cerr << "BOOTSTRAP_AGENT: bootstrap_agent_getid called\n";
#endif
  CORBA::Object_ptr result = CORBA::Object::_nil();
  char* host;
  if ((host=(char*)getenv("BOOTSTRAP_AGENT_HOST"))==NULL){
    host = (char*)"";
  }
  if (strlen(host)==0) {
    char self[64];
    if (gethostname(&self[0],64) != 0) {
#if DIAGNOSTIC_LEVEL > 0
      cerr << "BOOTSTRAP_AGENT: Cannot get the name of this host\n";
#endif
      return result;
    }
    host = self;
  }

  char* portstr;
  if ((portstr=(char*)getenv("BOOTSTRAP_AGENT_PORT"))==NULL){
    portstr = (char*)"9999";
  }

  unsigned int portnum;
  if ( sscanf(portstr,"%u", &portnum) != 1 || 
       (portnum == 0 || portnum >= 65536)) {
#if DIAGNOSTIC_LEVEL > 0
    cerr << "BOOTSTRAP_AGENT: " << portstr << " not a valid port number\n";
#endif
    return result;
  }

  CORBA::UShort port = (CORBA::UShort)portnum;
  result = get(orb, id, host, port);
  return result;
}


inline CORBA::Object_ptr
BootStrapAgent::get(CORBA::ORB_ptr orb, const char* id, 
		    const char* host, CORBA::UShort port) 
{
  CORBA::Object_ptr result = CORBA::Object::_nil();
#if DIAGNOSTIC_LEVEL > 0
  cerr << "BOOTSTRAP_AGENT: bootstrap_agent_get called\n";
  cerr << "\tid: " << id << " host: " << host << " port: " << (int)port << "\n";
#endif

  try {
    const char* tid = "IDL:omg.org/CORBA/InitialReferences:1.0";
    const char* keystr = "INIT";
    CORBA::ORB::ObjectKey key = new CORBA::ReferenceData;
    key->length(strlen(keystr));
    unsigned int j;
    for (j=0; j<strlen(keystr); j++)
      (*key)[j] = (CORBA::Octet) keystr[j];

    CORBA::String_var s = orb->makeIOR(host, port, key, tid);

#if DIAGNOSTIC_LEVEL > 0
    cerr << "BOOTSTRAP_AGENT: initreferences ior:\n\n";
    cerr << "'" << s << "'" << "\n\n";
    cerr << "BOOTSTRAP_AGENT: calling string_to_object\n";
#endif

    CORBA::Object_var o = orb->string_to_object(s);

#if DIAGNOSTIC_LEVEL > 0
    cerr << "BOOTSTRAP_AGENT: calling _narrow\n";
#endif

    CORBA_InitialReferences_var i = CORBA_InitialReferences::_narrow(o);
    if (CORBA::is_nil(i)) {
#if DIAGNOSTIC_LEVEL > 0
      cerr << "BOOTSTRAP_AGENT: CORBA_InitialReferences::_narrow() failed\n";
#endif
      return result;
    }

    // XXX should we do this check?
    // i->noExistentCheck();

#if DIAGNOSTIC_LEVEL > 0
    cerr << "BOOTSTRAP_AGENT: calling get\n";
#endif

    result = i->get(id);
    if (CORBA::is_nil(result)) {
#if DIAGNOSTIC_LEVEL > 0
      cerr << "BOOTSTRAP_AGENT: i->get(" << id << ") returned null obj ref\n";
#endif
    } else {
#if DIAGNOSTIC_LEVEL > 0
      CORBA::String_var result_ior = orb->object_to_string(result);
      cerr << "BOOTSTRAP_AGENT: i->get(" << id << ") returned ref with IOR:\n";
      cerr << "'" << result_ior << "'" << "\n\n";
#endif
    }
    return result;
  } catch (...) {
#if DIAGNOSTIC_LEVEL > 0
    cerr << "BOOTSTRAP_AGENT: exception in initrefs_bootstrap\n";
#endif
  }
  return result;
}

#endif
