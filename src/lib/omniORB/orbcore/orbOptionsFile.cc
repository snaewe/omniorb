// -*- Mode: C++; -*-
//                            Package   : omniORB
// orbOptionsReg.cc           Created on: 17/8/2001
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
  Revision 1.1.2.1  2001/08/20 08:19:23  sll
  Read the new ORB configuration file format. Can still read old format.
  Can also set configuration parameters from environment variables.

*/

#include <omniORB4/CORBA.h>
#include <orbOptions.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define SUPPORT_OLD_CONFIG_FORMAT 1

OMNI_NAMESPACE_BEGIN(omni)

static
void
invalid_syntax_error(const char* filename, int lineno, 
		     const char* key, const char* value, const char* why) {
  if ( omniORB::trace(1) ) {
    omniORB::logger log;
    log << "Invalid syntax in configuration file " << filename
	<< " line no. " << lineno << " reason: "
	<< why << "\n";
  }
  throw orbOptions::Unknown(key,value);
}

#define LINEBUFSIZE 2048

static CORBA::Boolean parseOldConfigOption(orbOptions& opt, char* line);

void
orbOptions::importFromFile(const char* filename) throw (orbOptions::Unknown,
							orbOptions::BadParam) {

  FILE* file;
  CORBA::String_var line(CORBA::string_alloc(LINEBUFSIZE));
  unsigned int lnum = 0;

  if ( !filename || !strlen(filename) ) return;

  if ( !(file = fopen(filename, "r")) ) {
    if ( omniORB::trace(2) ) {
      omniORB::logger log;
      log << "Configuration file \"" << filename
	  << "\" either does not exist or is not a file. No settings read.\n";
    }
    return;
  }
  else if ( omniORB::trace(2) ) {
    omniORB::logger log;
    log << "Read from configuration file \"" << filename << "\".\n";
  }

  CORBA::String_var key;
  char* value = 0;

  while ( fgets((char*)line, LINEBUFSIZE, file) ) {

    char* p = line;
    lnum++;

    // skip leading white space
    while ( isspace(*p) )
      p++;

    // ignore commented/blank lines
    if ( *p == '#' || *p == '\0' )
      continue;

    char* sep = strchr(line,'=');
    if (!sep) {
#if SUPPORT_OLD_CONFIG_FORMAT
      // Backward compatibility. Detect old format/keywords 
      if (parseOldConfigOption(*this,line)) {
	continue;
      }
      else 
#endif
      {
	invalid_syntax_error(filename,lnum,
			     "<missing>","<missing>","missing '='");
	// not reached here
      }
    }

    if ( p != sep ) {
      // syntax:   key = value
      // extract key
      char* t = p;
      p = sep - 1;
      while ( isspace(*p) )
	p--;
      *(++p) = '\0';
      key = (const char*) t;
    }
    else {
      // syntax:       = value
      // The key stays the same as the last config line.
      if ((const char*)key == 0 || strlen(key) == 0) {
	// The 1st line, no key is set. 
	invalid_syntax_error(filename,lnum,
			     "<missing>","<missing>","key missing");
	// not reached here
      }
    }

    p = sep + 1;
    while ( isspace(*p) )
      p++;

    // extract value
    if ( *p == '\0' ) {
      invalid_syntax_error(filename,lnum,
			   key,"<missing>","value missing");
      // not reached here
    }
    value = p;
    p += strlen(value) - 1;
    while ( isspace(*p) )
      p--;

    while ( *p == '\\' ) {
      int bsz = LINEBUFSIZE - (p - (char*)line);
      if ( bsz && fgets(p,bsz,file) ) {
	lnum++;
	p += strlen(p) - 1;
	while ( isspace(*p) )
	  p--;
      }
      else {
	p--;
      } 
    }

    *(++p) = '\0';
    addOption(key,value);
  }
  fclose(file);
}

#if SUPPORT_OLD_CONFIG_FORMAT
OMNI_NAMESPACE_END(omni)
#include <gatekeeper.h>
OMNI_NAMESPACE_BEGIN(omni)

static
void
oldconfig_warning(const char* key, const char* newkey) {
  static CORBA::Boolean said_warning = 0;

  if (!said_warning && omniORB::trace(1)) {
    omniORB::logger log;
    log << "Warning: the config file is in the old pre-omniORB4 format.\n"
 "omniORB: For the moment this is accepted to maintain backword compatibility.\n"
 "omniORB: Please update to the new config file format ASAP.\n";
    said_warning = 1;
  }
  if (omniORB::trace(1)) {
    omniORB::logger log;
    log << "Warning: translated (" << key << ") to (" << newkey << ")\n";
  }
}

static
CORBA::Boolean
parseOldConfigOption(orbOptions& opt, char* line) {

  char* p = line;

  // skip leading white space
  while ( isspace(*p) )
    p++;

  char* key = p;

  while ( !isspace(*p) && *p != '\0' )
    p++;

  if (*p == '\0')
    return 0;

  *p++ = '\0';

  while ( isspace(*p) )
    p++;

  if (*p == '\0')
    return 0;

  char* value = p;
  
  p += strlen(p) - 1;
  while ( isspace(*p) )
    p--;
  *(++p) = '\0';

  if (strcmp(key,"ORBInitRef") == 0) {
    oldconfig_warning("ORBInitRef","InitRef");
    opt.addOption(key+3,value);
  }
  else if (strcmp(key,"ORBDefaultInitRef") == 0) {
    oldconfig_warning("ORBDefaultInitRef","DefaultInitRef");
    opt.addOption(key+3,value);
  }
  else if (strcmp(key,"NAMESERVICE") == 0) {
    oldconfig_warning("NAMESERVICE","InitRef NameService=");
    const char* format = "NameService=%s";
    CORBA::String_var v(CORBA::string_alloc(strlen(value)+strlen(format)));
    sprintf(v,format,value);
    opt.addOption("InitRef",v);
  }
  else if (strcmp(key,"INTERFACE_REPOSITORY") == 0) {
    oldconfig_warning("INTERFACE_REPOSITORY","InitRef InterfaceRepository=");
    const char* format = "InterfaceRepository=%s";
    CORBA::String_var v(CORBA::string_alloc(strlen(value)+strlen(format)));
    sprintf(v,format,value);
    opt.addOption("InitRef",v);
  }
  else if (strcmp(key,"ORBInitialHost") == 0) {
    oldconfig_warning("ORBInitialHost","bootstrapAgentHostname");
    opt.addOption("bootstrapAgentHostname",value);
  }
  else if (strcmp(key,"ORBInitialPort") == 0) {
    oldconfig_warning("ORBInitialPort","bootstrapAgentPort");
    opt.addOption("bootstrapAgentPort",value);
  }
  else if (strcmp(key,"GATEKEEPER_ALLOWFILE") == 0) {
    oldconfig_warning("GATEKEEPER_ALLOWFILE","accepted");
    gateKeeper::allowFile = CORBA::string_dup(value);
  }
  else if (strcmp(key,"GATEKEEPER_DENYFILE") == 0) {
    oldconfig_warning("GATEKEEPER_DENYFILE","accepted");
    gateKeeper::denyFile = CORBA::string_dup(value);
  }
  else {
    return 0;
  }
  return 1;
}
#endif

OMNI_NAMESPACE_END(omni)
