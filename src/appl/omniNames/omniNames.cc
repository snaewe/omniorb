#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream.h>
#include <omnithread.h>
#include "NamingContext_i.h"
#include "default.h"

static void
usage()
{
  cerr << "usage: omniNames [-start <port>] [<omniORB2-options>...]" << endl;
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
  // If we have a "-start" option, get the given port number.
  //

  int port = 0;

  if ((argc > 1) && (strcmp(argv[1], "-start") == 0)) {
    if (argc < 3) usage();
    port = atoi(argv[2]);
    removeArgs(argc, argv, 1, 2);
  }


  //
  // Set up an instance of class log.  This also gives us back the port
  // number from the log file if "-start" wasn't specified.
  //

  log l(port);


  //
  // Add a fake command line option to tell the BOA which port to use.
  //

  insertArgs(argc, argv, 1, 2);
  argv[1] = strdup("-BOAiiop_port");
  argv[2] = new char[16];
  sprintf(argv[2], "%d", port);


  //
  // Initialize the ORB and the object adaptor.
  //

  CORBA::ORB_ptr orb = CORBA::ORB_init(argc,argv,"omniORB2");
  CORBA::BOA_ptr boa = orb->BOA_init(argc,argv,"omniORB2_BOA");

  boa->impl_is_ready(0,1);


  //
  // Read the log file and set up all the naming contexts described in it.
  //

  l.init(orb, boa);


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
    c.timed_wait(s,n);
  }
  m.unlock();

  return 0;
}
