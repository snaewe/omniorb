// -*- Mode: C++; -*-
//                          Package   : omniNames
// omniNames.cc             Author    : Tristan Richardson (tjr)
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

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <iostream.h>
#include <omnithread.h>
#include <NamingContext_i.h>

#ifdef __WIN32__
#include <io.h>
#else
#include <unistd.h>
#endif

#ifndef O_SYNC
#ifdef  O_FSYNC              // FreeBSD 3.2 does not have O_SYNC???
#define O_SYNC O_FSYNC
#endif
#endif

// Minimum idle period before we take a checkpoint (15 mins)
#define DEFAULT_IDLE_TIME_BTW_CHKPT  (15*60)


PortableServer::POA_var the_poa;
PortableServer::POA_var the_ins_poa;


void
usage()
{
  cerr << "\nusage: omniNames [-start [<port>]]\n"
       <<   "                 [-logdir <directory name>]\n"
       <<   "                 [-errlog <file name>]\n"
       <<   "                 [<omniORB-options>...]" << endl;
  cerr << "\nUse -start option to start omniNames for the first time."
       << endl
       << "With no <port> argument, the standard default of "
       << IIOP::DEFAULT_CORBALOC_PORT << " is used."
       << endl;
  cerr << "\nUse -logdir option to specify the directory where the log/data files are kept.\n";
  cerr << "\nUse -errlog option to specify where standard error output is redirected.\n";
  cerr << "\nYou can also set the environment variable " << LOGDIR_ENV_VAR
       << " to specify the\ndirectory where the log/data files are kept.\n"
       << endl;
  exit(1);
}


static void
removeArgs(int& argc, char** argv, int idx, int nargs)
{
  if ((idx+nargs) > argc) return;
  for (int i = idx+nargs; i < argc; i++) {
    argv[i-nargs] = argv[i];
  }
  argc -= nargs;
}


static void
insertArgs(int& argc, char**& argv, int idx, int nargs)
{
  char** newArgv = new char*[argc+nargs];
  int i;
  for (i = 0; i < idx; i++) {
    newArgv[i] = argv[i];
  }
  for (i = idx; i < argc; i++) {
    newArgv[i+nargs] = argv[i];
  }
  argv = newArgv;
  argc += nargs;
}


//
// main
//

int
main(int argc, char **argv)
{
  //
  // If we have a "-start" option, get the given port number, or use
  // the default.
  //

  int port = 0;
  char* logdir = 0;

  while (argc > 1) {
    if (strcmp(argv[1], "-start") == 0) {
      if (argc < 3 || argv[2][0] == '-') {
	port = IIOP::DEFAULT_CORBALOC_PORT;
	removeArgs(argc, argv, 1, 1);
      }
      else {
	port = atoi(argv[2]);
	removeArgs(argc, argv, 1, 2);
      }
    }
    else if (strcmp(argv[1], "-logdir") == 0) {
      if (argc < 3) usage();
      logdir = argv[2];
      removeArgs(argc, argv, 1, 2);
    }
    else if (strcmp(argv[1], "-errlog") == 0) {
      if (argc < 3) usage();
#ifdef __WIN32__
      int fd = _open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, _S_IWRITE);
      if (fd < 0 || _dup2(fd,2)) {
#else
      int fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC | O_SYNC, 0666);
      if (fd < 0 || dup2(fd,2) < 0) {
#endif
	cerr << "Cannot open error log file: " << argv[2] << endl;
	usage();
      }
      removeArgs(argc, argv, 1, 2);
    }
    else if ((strncmp(argv[1], "-ORB", 4) != 0)) {
      usage();
    }
    else {
      break;
    }
  }

  //
  // Set up an instance of class omniNameslog.  This also gives us back the port
  // number from the log file if "-start" wasn't specified.
  //

  omniNameslog l(port, logdir);


  //
  // Add a fake command line option to tell the POA which port to use.
  //

  insertArgs(argc, argv, 1, 2);
  argv[1] = strdup("-ORBendpoint");
  argv[2] = new char[20];
  sprintf(argv[2], "giop:tcp::%d", port);

  //
  // Initialize the ORB and the object adapter.
  //

  CORBA::ORB_ptr orb;

  try {
    orb = CORBA::ORB_init(argc, argv);
  }
  catch (CORBA::INITIALIZE& ex) {
    cerr << "Failed to initialise the ORB." << endl;
    return 1;
  }

  try {
    CORBA::Object_var poaref = orb->resolve_initial_references("RootPOA");
    PortableServer::POA_var poa = PortableServer::POA::_narrow(poaref);

    PortableServer::POAManager_var pman = poa->the_POAManager();

    CORBA::PolicyList pl(1);
    pl.length(1);
    pl[0] = poa->create_lifespan_policy(PortableServer::PERSISTENT);

    the_poa = poa->create_POA("", pman, pl);
    pman->activate();

    // Get the "magic" interoperable naming service POA
    poaref      = orb->resolve_initial_references("omniINSPOA");
    the_ins_poa = PortableServer::POA::_narrow(poaref);
    pman        = the_ins_poa->the_POAManager();
    pman->activate();
  }
  catch (CORBA::INITIALIZE& ex) {
    cerr << "Failed to initialise the POAs. "
	 << "Is omniNames is already running?" << endl;
    return 1;
  }

  //
  // Read the log file and set up all the naming contexts described in it.
  //

  l.init(orb, the_poa, the_ins_poa);

  //
  // Now this thread has nothing much to do.  Simply take a checkpoint once
  // every so often.
  //

  int idle_time_btw_chkpt;
  char *itbc = getenv("OMNINAMES_ITBC");
  if (itbc == NULL || sscanf(itbc,"%d",&idle_time_btw_chkpt) != 1)
    idle_time_btw_chkpt = DEFAULT_IDLE_TIME_BTW_CHKPT;

  omni_mutex m;
  omni_condition c(&m);

  m.lock();
  while (1) {
    l.checkpoint();
    unsigned long s, n;
    omni_thread::get_time(&s, &n, idle_time_btw_chkpt);
    c.timedwait(s,n);
  }
  m.unlock();

  return 0;
}
