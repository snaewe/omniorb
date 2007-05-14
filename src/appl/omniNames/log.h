// -*- Mode: C++; -*-
//                          Package   : omniNames
// log.h                    Author    : Tristan Richardson (tjr)
//
//    Copyright (C) 1997-1999 AT&T Laboratories Cambridge
//
//  This file is part of omniNames.
//
//  omniNames is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
//  USA.
//

#ifndef _log_h_
#define _log_h_

#include <omniORB4/CORBA.h>

#ifdef HAVE_STD
#  include <fstream>
   using namespace std;
#else
#  include <fstream.h>
#endif

#ifndef LOGDIR_ENV_VAR
#define LOGDIR_ENV_VAR "OMNINAMES_LOGDIR"
#endif

class omniNameslog {

  CORBA::ORB_ptr orb;
  PortableServer::POA_ptr poa;
  PortableServer::POA_ptr ins_poa;
  char *active;
  char *backup;
  char *checkpt;
  ofstream logf;

  int port;
  PortableServer::ObjectId persistentId;

  int startingUp;	// true while reading log file initially.
  int firstTime;	// true if -start option was given.
  int checkpointNeeded;	// true if changes have been made since last checkpoint

  int line;		// current line number when reading log file initially.

  //
  // functions to write to a file
  //

  void putPort(int port, ostream& file);

  void putPersistent(const PortableServer::ObjectId& id, ostream& file);

  void putCreate(const PortableServer::ObjectId& id, ostream& file);

  void putDestroy(CosNaming::NamingContext_ptr nc, ostream& file);

  void putBind(CosNaming::NamingContext_ptr nc,
	       const CosNaming::Name& n, CORBA::Object_ptr obj,
	       CosNaming::BindingType t, ostream& file);

  void putUnbind(CosNaming::NamingContext_ptr nc, const CosNaming::Name& n,
		 ostream& file);

  void putKey(const PortableServer::ObjectId& id, ostream& file);

  void putString(const char* str, ostream& file);

  //
  // functions to read from a file
  //

  void getPort(istream& file);

  void getPersistent(istream& file);

  void getCreate(istream& file);

  void getDestroy(istream& file);

  void getBind(istream& file);

  void getUnbind(istream& file);

  void getKey(PortableServer::ObjectId& id, istream& file);

  void getFinalString(char*& buf, istream& file);

  void getNonfinalString(char*& buf, istream& file);

  int getString(char*& buf, istream& file);

public:

  class IOError {};
  class ParseError {};

  omniNameslog(int& port, char* logdir, int nohostname);

  void init(CORBA::ORB_ptr o,
	    PortableServer::POA_ptr p,
	    PortableServer::POA_ptr ip);

  void create(const PortableServer::ObjectId& id);
  void destroy(CosNaming::NamingContext_ptr nc);
  void bind(CosNaming::NamingContext_ptr nc,
	    const CosNaming::Name& n, CORBA::Object_ptr obj,
	    CosNaming::BindingType t);
  void unbind(CosNaming::NamingContext_ptr nc, const CosNaming::Name& n);

  void checkpoint(void);

};

#endif
