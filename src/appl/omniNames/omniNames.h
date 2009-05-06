// -*- Mode: C++; -*-
//                          Package   : omniNames
// omniNames.h              Author    : Duncan Grisby
//
//    Copyright (C) 2007 Apasphere Ltd
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

#ifndef _omniNames_h_
#define _omniNames_h_

#include "log.h"

class omniNames {
public:  

  omniNames(int            port,
	    const char*    logdir,
	    const char*    errlog,
	    CORBA::Boolean ignoreport,
	    CORBA::Boolean nohostname,
	    CORBA::Boolean always,
	    int argc, char** argv);
  // Initialise ORB and prepare redo log. If port is 0, expects to
  // find an existing log file.

  ~omniNames();
  // Close files and destroy ORB.

  void run();
  // Run the naming service until stop() is called. Blocks the calling
  // thread.

  void stop();
  // Stop the thread blocked in run().

  CORBA::Boolean waitForStart(int timeout=0);
  // Block until the thread in run() is running. If timeout is
  // non-zero, it is a timeout in seconds. Returns true if started
  // successfully; false if timed out.

#ifdef __WIN32__
  static int installService(int            port,
			    const char*    logdir,
			    const char*    errlog,
			    CORBA::Boolean ignoreport,
			    CORBA::Boolean nohostname,
			    CORBA::Boolean manual,
			    int argc, char** argv);
  // Install Windows service

  static int removeService();
  // Remove Windows service

  static int runService(int port, const char* logdir, const char* errlog,
			CORBA::Boolean ignoreport, CORBA::Boolean nohostname,
			int a_argc, char** a_argv);
  // Entry-point for service execution

#endif

private:
  CORBA::ORB_var orb_;
  omniNameslog*  log_;

  omni_mutex     mu_;
  omni_condition cond_;
  CORBA::Boolean stop_;
  CORBA::Boolean running_;

#ifdef __WIN32__
  std::ofstream   errstream_;
  std::streambuf* cerrbuf_;
#endif
};


#endif // _omniNames_h_
