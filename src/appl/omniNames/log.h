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

#include <omniORB2/CORBA.h>
#include <fstream.h>

#ifndef LOGDIR_ENV_VAR
#define LOGDIR_ENV_VAR "OMNINAMES_LOGDIR"
#endif

class log {

  CORBA::ORB_ptr orb;
  CORBA::BOA_ptr boa;
  char *active;
  char *backup;
  char *checkpt;
  ofstream logf;

  int port;

  int startingUp;	// true while reading log file initially.
  int firstTime;	// true if -start option was given.
  int checkpointNeeded;	// true if changes have been made since last checkpoint

  int line;		// current line number when reading log file initially.

  //
  // functions to write to a file
  //

  void putPort(int port, ostream& file);

  void putCreate(const omniORB::objectKey& key, ostream& file);

  void putDestroy(CosNaming::NamingContext_ptr nc, ostream& file);

  void putBind(CosNaming::NamingContext_ptr nc,
	       const CosNaming::Name& n, CORBA::Object_ptr obj,
	       CosNaming::BindingType t, ostream& file);

  void putUnbind(CosNaming::NamingContext_ptr nc, const CosNaming::Name& n,
		 ostream& file);

  void putKey(const omniORB::objectKey& key, ostream& file);

  void putString(const char* str, ostream& file);

  //
  // functions to read from a file
  //

  void getPort(istream& file);

  void getCreate(istream& file);

  void getDestroy(istream& file);

  void getBind(istream& file);

  void getUnbind(istream& file);

  void getKey(omniORB::objectKey& k, istream& file);

  void getFinalString(char*& buf, istream& file);

  void getNonfinalString(char*& buf, istream& file);

  int getString(char*& buf, istream& file);

public:

  class IOError {};
  class ParseError {};

  log(int& port,char* logdir=0);

  void init(CORBA::ORB_ptr o, CORBA::BOA_ptr b);

  void create(const omniORB::objectKey& key);
  void destroy(CosNaming::NamingContext_ptr nc);
  void bind(CosNaming::NamingContext_ptr nc,
	    const CosNaming::Name& n, CORBA::Object_ptr obj,
	    CosNaming::BindingType t);
  void unbind(CosNaming::NamingContext_ptr nc, const CosNaming::Name& n);

  void checkpoint(void);

};

#endif
