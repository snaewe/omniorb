// -*- Mode: C++; -*-
//                          Package   : omniNames
// log.cc                   Author    : Tristan Richardson (tjr)
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
#include <errno.h>
#include <time.h>
#include <iostream.h>
#include <fcntl.h>
#if defined(__VMS) && __VMS_VER < 70000000
#include <omniVMS/unlink.hxx>
#include <omniVms/utsname.hxx>
#endif
#include <NamingContext_i.h>
#include <ObjectBinding.h>
#include <log.h>
#include <iomanip.h>

#ifdef __WIN32__

#include <io.h>
#include <winbase.h>

#define stat(x,y) _stat(x,y)
#define unlink(x) _unlink(x)

#else

#include <unistd.h>
#include <sys/utsname.h>

#endif

#if defined(__nextstep__)
#include <libc.h>
#include <sys/param.h>
#endif

#if defined(__nextstep__)
#include <sys/param.h>
#endif

#ifndef O_SYNC
#ifdef  O_FSYNC              // FreeBSD 3.2 does not have O_SYNC???
#define O_SYNC O_FSYNC
#endif
#endif

#ifdef _NO_STRDUP


// we have no strdup
static char *
strdup (char* str)
{
  char *newstr;

  newstr = (char *) malloc (strlen (str) + 1);
  if (newstr)
    strcpy (newstr, str);
  return newstr;
}
#endif  // _NO_STRDUP

extern void usage();


//
// This class can be used to generate timestamps.  The t() method normally
// returns a timestamp string, but if the same timestamp (to the nearest
// second) would be returned as last time then an empty string is returned
// instead.
//

class timestamp {
  char str[29];
public:
  timestamp(void) {
    str[0] = '\n';
    str[1] = str[28] = '\0';
  }
  char* t(void) {
    time_t t = time(NULL);
    char *p = ctime(&t);
    if (strncmp(p, &str[1], 24) == 0) {
      return &str[28];
    }
    strncpy(&str[1], p, 24);
    str[25] = ':';
    str[26] = '\n';
    str[27] = '\n';
    return str;
  }
};

timestamp ts;


//
// Constructor for class omniNameslog.  There will normally be only one
// instance of this class.  Unfortunately the initialisation of the class
// cannot be completed in the constructor - the rest is done in the init()
// member function.  This is because the main program cannot give us
// pointers to the ORB and the BOA until it has worked out which port to
// listen on, but it normally finds out the port from the log file.  So in
// this constructor we initialise as much as we can, find out the port from
// the log file if there is one and return.  Then the main program
// initialises the ORB and the BOA and calls init().
//

omniNameslog::omniNameslog(int& p,char* logdir) : port(p)
{
  startingUp = 1;
  checkpointNeeded = 1;
  line = 1;

#ifdef __WIN32__
  struct _stat sb;
#else
  struct stat sb;
#endif

  if (!logdir && (logdir = getenv(LOGDIR_ENV_VAR)) == NULL)
    logdir = strdup(DEFAULT_LOGDIR);

#if !defined(__WIN32__) && !defined(__VMS)
  if (logdir[0] != '/') {
    cerr << ts.t() << "Error: " << LOGDIR_ENV_VAR << " (" << logdir
	 << ") is not an absolute path name." << endl;
    exit(1);
  }

  if (logdir[strlen(logdir)-1] == '/') {
    logdir[strlen(logdir)-1] = '\0';		// strip trailing '/'
  }

#ifndef _USE_GETHOSTNAME
  struct utsname un;
  if (uname(&un) < 0) {
    cerr << ts.t() << "Error: cannot get the name of this host." << endl;
	
    exit(1);
  }

  char* logname = new char[strlen(logdir) + strlen("/omninames-")
			   + strlen(un.nodename) + 1];
  sprintf(logname, "%s/omninames-%s", logdir, un.nodename);
#else

  // Apparently on some AIX versions, MAXHOSTNAMELEN is too small (32) to
  // reflect the true size a hostname can be. Check and fix the value.
#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 256
#elif   MAXHOSTNAMELEN < 64
#undef  MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 256
#endif

  char hostname[MAXHOSTNAMELEN+1];

  if (gethostname(hostname, MAXHOSTNAMELEN) < 0) {
    cerr << ts.t() << "Error: cannot get the name of this host." << endl;
	
    exit(1);
  }
  char* logname = new char[strlen(logdir) + strlen("/omninames-")
			   + strlen(hostname) + 1];
  sprintf(logname, "%s/omninames-%s", logdir, hostname);
#endif // _USE_GETHOSTNAME
#elif defined(__WIN32__)

  // Get host name:

  DWORD machineName_len = MAX_COMPUTERNAME_LENGTH+1;
  char* machineName = new char[machineName_len];
  if (!GetComputerName((LPTSTR) machineName, &machineName_len)) {
    cerr << ts.t() << "Error: cannot get the name of this host." << endl;
	
    exit(1);
  }

  char* logname = new char[strlen(logdir) + strlen("\\omninames-")
			   + strlen(machineName) + 1];
  sprintf(logname, "%s\\omninames-%s", logdir, machineName);
  
  delete[] machineName;
#else
  char last(
    logdir[strlen(logdir)-1]
  );
  if (last != ':' && last != ']') {
    cerr << ts.t() << "Error: " << LOGDIR_ENV_VAR << " (" << logdir
         << ") is not a directory name." << endl;
    exit(1);
  }

//  if (logdir[strlen(logdir)-1] == '/') {
//    logdir[strlen(logdir)-1] = '\0';          // strip trailing '/'
//  }

  struct utsname un;
  if (uname(&un) < 0) {
    cerr << ts.t() << "Error: cannot get the name of this host." << endl;
        
    exit(1);
  }

  char* logname = new char[strlen(logdir) + strlen("/omninames-")
                           + strlen(un.nodename) + 1];
  sprintf(logname, "%somninames-%s", logdir, un.nodename);
#endif

#ifndef __VMS
  active = new char[strlen(logname)+strlen(".log")+1];
  sprintf(active,"%s.log",logname);
  backup = new char[strlen(logname)+strlen(".bak")+1];
  sprintf(backup,"%s.bak",logname);
  checkpt = new char[strlen(logname)+strlen(".ckp")+1];
  sprintf(checkpt,"%s.ckp",logname);
#else
  // specify latest version:
  active = new char[strlen(logname)+strlen(".log;")+1];
  sprintf(active,"%s.log;",logname);
  backup = new char[strlen(logname)+strlen(".bak;")+1];
  sprintf(backup,"%s.bak;",logname);
  checkpt = new char[strlen(logname)+strlen(".ckp;")+1];
  sprintf(checkpt,"%s.ckp;",logname);
#endif

  delete [] logname;

  if (port != 0) {

    //
    // Starting for the first time - make sure log file doesn't exist, and
    // for safety, that there is no backup file either.
    //

    firstTime = 1;

    if (stat(active,&sb) == 0) {
      cerr << ts.t() << "Error: log file '" << active
	   << "' exists.  Can't use -start option." << endl;
      exit(1);
    }
    if (stat(backup,&sb) == 0) {
      cerr << ts.t() << "Error: backup file '" << backup
	   << "' exists.  Can't use -start option." << endl;
      exit(1);
    }

  } else {

    //
    // Restart - get port info from log file.
    //

    firstTime = 0;

#ifdef __WIN32__
    ifstream initf(active,ios::in | ios::nocreate);
#else
    ifstream initf(active);
#endif
    if (!initf) {
      cerr << ts.t() << "Error: cannot open log file '" << active << "'."
	   << endl;
      if (stat(backup,&sb) == 0) {
	cerr << "Info: backup file '" << backup << "' exists." << endl
	     << "This must be removed if you want to use the -start option."
	     << endl;
      }
      usage();
    }

    try {
      getPort(initf);

    } catch (IOError&) {

      cerr << ts.t() << "Error: reading log file '" << active << "' failed: "
	   << flush;
      perror("");
      initf.close();
      exit(1);

    } catch (ParseError&) {

      cerr << ts.t() << "Error: parse error in log file '" << active
	   << "' at line " << line << "." << endl;
      initf.close();
      exit(1);
    }

    p = port;

    initf.close();
  }
}


void
omniNameslog::init(CORBA::ORB_ptr the_orb, PortableServer::POA_ptr the_poa)
{
  orb = the_orb;
  poa = the_poa;

  if (firstTime) {

    //
    // starting for the first time - create an initial log file with the
    // port specification and the root context.
    //

    cerr << ts.t() << "Starting omniNames for the first time." << endl;

    try {
#if !defined(__SUNPRO_CC) || __SUNPRO_CC < 0x500
#  ifdef __WIN32__
      int fd = _open(active, O_WRONLY | O_CREAT | O_TRUNC, _S_IWRITE);
#  else
      int fd = open(active, O_WRONLY | O_CREAT | O_TRUNC | O_SYNC, 0666);
#  endif

      if (fd < 0)
	throw IOError();
      logf.attach(fd);
#else
      logf.open(active,ios::out|ios::trunc,0666);
      if (!logf)
	throw IOError();
#endif
      putPort(port, logf);

      {
	CORBA::Object_var ref = poa->create_reference(
				    CosNaming::NamingContext::_PD_repoId);
	PortableServer::ObjectId_var refid = poa->reference_to_id(ref);

	putCreate(refid, logf);
      }

      logf.close();
      if (!logf)
	throw IOError();

// a bug in sparcworks C++ means that the fd doesn't get closed.
#if defined(__sunos__) && defined(__SUNPRO_CC) && __SUNPRO_CC < 0x500
      if (close(fd) < 0)
	throw IOError();
#endif

    } catch (IOError& ex) {
      cerr << ts.t() << "Error: cannot create initial log file '" << active
	   << "': " << endl;
      perror("");
      cerr << "\nYou can set the environment variable " << LOGDIR_ENV_VAR
	   << " to specify the\ndirectory where the log files are kept.\n"
	   << endl;
      logf.close();
      unlink(active);
      exit(1);
    }

    cerr << ts.t() << "Wrote initial log file." << endl;
  }

  // claim the global lock as a writer (ie exclusive).

  NamingContext_i::lock.writerIn();

  ifstream initf(active);

  if (!initf) {
    cerr << ts.t() << "Error: cannot open log file '" << active << "'."
	 << endl;
    exit(1);
  }

  try {
    line = 1;

    while (initf && (initf.peek() != EOF)) {

      char* cmd;

      getNonfinalString(cmd, initf);

      if (strcmp(cmd, "port") == 0) {
	while (initf && (initf.get() != '\n'));	// ignore rest of line
	line++;
      } else if (strcmp(cmd, "create") == 0) {
	getCreate(initf);
      } else if (strcmp(cmd, "destroy") == 0) {
	getDestroy(initf);
      } else if (strcmp(cmd, "bind") == 0) {
	getBind(initf);
      } else if (strcmp(cmd, "unbind") == 0) {
	getUnbind(initf);
      } else {
	cerr << ts.t() << "Error: unknown command '" << cmd
	     << "' in log file '" << active << "'." << endl;
	throw ParseError();
      }

      delete [] cmd;
    }

    initf.close();

  } catch (IOError&) {

    cerr << ts.t() << "Error: reading log file '" << active << "' failed: "
	 << flush;
    perror("");
    initf.close();
    exit(1);

  } catch (ParseError&) {

    cerr << ts.t() << "Error: parse error in log file '" << active
	 << "' at line " << line << "." << endl;
    initf.close();
    exit(1);
  }


  cerr << ts.t() << "Read log file successfully." << endl;

  CosNaming::NamingContext_ptr rootContext
    = NamingContext_i::headContext->_this();

  char* p = orb->object_to_string(rootContext);
  cerr << ts.t() << "Root context is " << p << endl;
  // Now use the backdoor to tell the bootstrap agent in this
  // address space to return this root context in response to
  // CORBA::InitialReferences::get("NameService");
  _omni_set_NameService(rootContext);
  delete p;

  CORBA::release(rootContext);	// dispose of the object reference

#if !defined(__SUNPRO_CC) || __SUNPRO_CC < 0x500
#  ifdef __WIN32__
  int fd = _open(active, O_WRONLY | O_APPEND);
#  else
  int fd = open(active, O_WRONLY | O_APPEND | O_SYNC);
#  endif

  if (fd < 0) {
    cerr << ts.t() << "Error: cannot open log file '" << active
	 << "' for writing." << endl;
    exit(1);
  }
  logf.attach(fd);
#else
  logf.open(active,ios::out|ios::app,0666);
  if (!logf) {
    cerr << ts.t() << "Error: cannot open log file '" << active
	 << "' for writing." << endl;
    exit(1);
  }
#endif

  startingUp = 0;

  NamingContext_i::lock.writerOut();

  // remove checkpoint. This will protect us from trouble if the previous
  // incarnation crashed during checkpointing and at the
  // time when both active and checkpoint are linked to the same file.

  unlink(checkpt);
}


void
omniNameslog::create(const PortableServer::ObjectId& id)
{
  if (!startingUp) {
    try {
      putCreate(id, logf);
    } catch (IOError& ex) {
      cerr << ts.t() << flush;
      perror("I/O error writing log file");
      throw CORBA::PERSIST_STORE();
    }
    checkpointNeeded = 1;
  }
}

void
omniNameslog::destroy(CosNaming::NamingContext_ptr nc)
{
  if (!startingUp) {
    try {
      putDestroy(nc, logf);
    } catch (IOError& ex) {
      cerr << ts.t() << flush;
      perror("I/O error writing log file");
      throw CORBA::PERSIST_STORE();
    }
    checkpointNeeded = 1;
  }
}

void
omniNameslog::bind(CosNaming::NamingContext_ptr nc, const CosNaming::Name& n,
	  CORBA::Object_ptr obj, CosNaming::BindingType t)
{
  if (!startingUp) {
    try {
      putBind(nc, n, obj, t, logf);
    } catch (IOError& ex) {
      cerr << ts.t() << flush;
      perror("I/O error writing log file");
      throw CORBA::PERSIST_STORE();
    }
    checkpointNeeded = 1;
  }
}

void
omniNameslog::unbind(CosNaming::NamingContext_ptr nc, const CosNaming::Name& n)
{
  if (!startingUp) {
    try {
      putUnbind(nc, n, logf);
    } catch (IOError& ex) {
      cerr << ts.t() << flush;
      perror("I/O error writing log file");
      throw CORBA::PERSIST_STORE();
    }
    checkpointNeeded = 1;
  }
}


void
omniNameslog::checkpoint(void)
{
  if (!checkpointNeeded) {
    // cerr << ts.t() << "No checkpoint needed." << endl;
    return;
  }

  cerr << ts.t() << "Checkpointing Phase 1: Prepare." << endl;

  //
  // Get the global lock as a reader.  This means clients will still be able
  // to do "resolve" and "list" operations, but anyone who tries to alter
  // the state in any way will block until we've finished.
  //

  NamingContext_i::lock.readerIn();

  ofstream ckpf;
  int fd = -1;

  try {

#if !defined(__SUNPRO_CC) || __SUNPRO_CC < 0x500
#  ifdef __WIN32__
    fd = _open(checkpt, O_WRONLY | O_CREAT | O_TRUNC, _S_IWRITE);
#  else
    fd = open(checkpt, O_WRONLY | O_CREAT | O_TRUNC | O_SYNC, 0666);
#  endif

    if (fd < 0) {
      cerr << ts.t() << "Error: cannot open checkpoint file '"
	   << checkpt << "' for writing." << endl;
      throw IOError();
    }

    ckpf.attach(fd);
#else
    ckpf.open(checkpt,ios::out|ios::trunc,0666);
    if (!ckpf) {
      cerr << ts.t() << "Error: cannot open checkpoint file '"
	   << checkpt << "' for writing." << endl;
      throw IOError();
    }
#endif
    putPort(port, ckpf);

    NamingContext_i* nci;

    for (nci = NamingContext_i::headContext; nci; nci = nci->next) {
      PortableServer::ObjectId_var id = poa->servant_to_id(nci);
      putCreate(id, ckpf);
    }

    for (nci = NamingContext_i::headContext; nci; nci = nci->next) {
      for (ObjectBinding* ob = nci->headBinding; ob; ob = ob->next) {
	putBind(nci->_this(), ob->binding.binding_name, ob->object,
		ob->binding.binding_type, ckpf);
      }
    }

    ckpf.close();
    if (!ckpf)
      throw IOError();

// a bug in sparcworks C++ means that the fd doesn't get closed.
#if defined(__sunos__) && defined(__SUNPRO_CC) && __SUNPRO_CC < 0x500
    if (close(fd) < 0)
      throw IOError();
#endif

  } catch (IOError& ex) {
    cerr << ts.t() << flush;
    perror("I/O error writing checkpoint file");
    cerr << "Abandoning checkpoint" << endl;
    ckpf.close();
// a bug in sparcworks C++ means that the fd doesn't get closed.
#if defined(__sunos__) && defined(__SUNPRO_CC) && __SUNPRO_CC < 0x500
    close(fd);
#endif
    NamingContext_i::lock.readerOut();
    unlink(checkpt);
    return;
  }


  //
  // Now commit the checkpoint to become the active log.
  //

  cerr << ts.t() << "Checkpointing Phase 2: Commit." << endl;

// a bug in sparcworks C++ means that the fd doesn't get closed.
#if defined(__sunos__) && defined(__SUNPRO_CC) && __SUNPRO_CC < 0x500
  close(logf.rdbuf()->fd());
#endif

  logf.close();

  unlink(backup);

#if defined(__WIN32__)
  if (!CopyFile(active,backup,TRUE)) {
#elif defined(__VMS)
  if (rename(active, backup) < 0) {
#else
  if (link(active,backup) < 0) {
#endif
    // Failure here leaves old active and checkpoint file.
    cerr << ts.t() << "Error: failed to link backup file '" << backup
	 << "' to old log file '" << active << "'." << endl;
    exit(1);
  }

#ifndef __VMS
  if (unlink(active) < 0) {
    // Failure here leaves active and backup pointing to the same (old) file.
    cerr << ts.t() << "Error: failed to unlink old log file '" << active
	 << "'." << endl;
    exit(1);
  }
#endif

#if defined(__WIN32__)
  if (!CopyFile(checkpt,active,TRUE)) {
#elif defined(__VMS)
  if (rename(checkpt,active) < 0) {
#else
  if (link(checkpt,active) < 0) {
#endif
    // Failure here leaves no active but backup points to the old file.
    cerr << ts.t() << "Error: failed to link log file '" << active
	 << "' to checkpoint file '" << checkpt << "'." << endl;
    exit(1);
  }

#ifndef __VMS
  if (unlink(checkpt) < 0) {
    // Failure here leaves active and checkpoint pointing to the same file.
    cerr << ts.t() << "Error: failed to unlink checkpoint file '" << checkpt
	 << "'." << endl;
    exit(1);
  }
#endif

#if !defined(__SUNPRO_CC) || __SUNPRO_CC < 0x500
#  ifdef __WIN32__
  fd = _open(active, O_WRONLY | O_APPEND);
#  else
  fd = open(active, O_WRONLY | O_APPEND | O_SYNC);
#  endif

  if (fd < 0) {
    cerr << ts.t() << "Error: cannot open new log file '" << active
	 << "' for writing." << endl;
    exit(1);
  }
  logf.attach(fd);
#else
  logf.open(active,ios::out|ios::app,0666);
  if (!logf) {
    cerr << ts.t() << "Error: cannot open log file '" << active
	 << "' for writing." << endl;
    exit(1);
  }
#endif

  NamingContext_i::lock.readerOut();

  cerr << ts.t() << "Checkpointing completed." << endl;

  checkpointNeeded = 0;
}


void
omniNameslog::putPort(int p, ostream& file)
{
  file << "port " << p << '\n' << flush;
  if (!file) throw IOError();
}


//
// getPort is different from the other get... functions in that the "command"
// ("port") hasn't been read yet, and also we cannot use any CORBA stuff
// since ORB_init may not have been called yet.
//

void
omniNameslog::getPort(istream& file)
{
  char* str;

  getNonfinalString(str, file);

  if (strcmp(str, "port") != 0) {
    cerr << ts.t() << "Error: log file doesn't start with \"port\"." << endl;
    throw ParseError();
  }

  delete [] str;

  getFinalString(str, file);

  port = atoi(str);

  delete [] str;

  if (port == 0) {
    cerr << ts.t() << "Error: invalid port specified in log file." << endl;
    throw ParseError();
  }
}


void
omniNameslog::putCreate(const PortableServer::ObjectId& id, ostream& file)
{
  file << "create ";
  putKey(id, file);
  file << '\n' << flush;
  if (!file) throw IOError();
}


void
omniNameslog::getCreate(istream& file)
{
  //
  // Argument to "create" is the object key of the naming context.
  //

  PortableServer::ObjectId id;
  getKey(id, file);
  NamingContext_i* rc = new NamingContext_i(poa, id, this);
}


void
omniNameslog::putDestroy(CosNaming::NamingContext_ptr nc, ostream& file)
{
  file << "destroy ";
  CORBA::String_var s = orb->object_to_string(nc);
  putString(s, file);
  file << '\n' << flush;
  if (!file) throw IOError();
}


void
omniNameslog::getDestroy(istream& file)
{
  //
  // Argument to "destroy" is NamingContext IOR.
  //

  char* str;
  getFinalString(str, file);
  CORBA::Object_var o;
  try {
    o = orb->string_to_object(str);
  } catch (...) {
    cerr << ts.t() << "getDestroy: invalid IOR." << endl;
    delete [] str;
    throw ParseError();
  }

  delete [] str;

  CosNaming::NamingContext_var nc = CosNaming::NamingContext::_narrow(o);
  if (CORBA::is_nil(nc)) {
    cerr << ts.t() << "getDestroy: IOR not a NamingContext." << endl;
    throw ParseError();
  }

  nc->destroy();
}


void
omniNameslog::putBind(CosNaming::NamingContext_ptr nc, const CosNaming::Name& n,
	     CORBA::Object_ptr obj, CosNaming::BindingType t, ostream& file)
{
  file << "bind ";
  CORBA::String_var s = orb->object_to_string(nc);
  putString(s, file);
  file << ' ';
  putString(n[0].id, file);
  file << ' ';
  putString(n[0].kind, file);
  if (t == CosNaming::nobject)
    file << " nobject ";
  else
    file << " ncontext ";
  s = orb->object_to_string(obj);
  putString(s, file);
  file << '\n' << flush;
  if (!file) throw IOError();
}


void
omniNameslog::getBind(istream& file)
{
  //
  // First arg is NamingContext IOR.
  //

  char* str;
  getNonfinalString(str, file);
  CORBA::Object_var o;
  try {
    o = orb->string_to_object(str);
  } catch (...) {
    cerr << ts.t() << "getBind: invalid IOR." << endl;
    delete [] str;
    throw ParseError();
  }
  delete [] str;

  CosNaming::NamingContext_var nc = CosNaming::NamingContext::_narrow(o);
  if (CORBA::is_nil(nc)) {
    cerr << ts.t() << "getBind: IOR not a NamingContext." << endl;
    throw ParseError();
  }

  //
  // 2nd & 3rd args are name id and kind.
  //

  CosNaming::Name name(1);
  name.length(1);

  getNonfinalString(str, file);
  name[0].id = str;

  getNonfinalString(str, file);
  name[0].kind = str;

  //
  // 4th arg is binding type.
  //

  char* bindingType;
  getNonfinalString(bindingType, file);

  //
  // 5th arg is object IOR.
  //

  getFinalString(str, file);
  try {
    o = orb->string_to_object(str);
  } catch (...) {
    cerr << ts.t() << "getDestroy: invalid IOR." << endl;
    delete [] str;
    throw ParseError();
  }
  delete [] str;

  if (strcmp(bindingType, "ncontext") == 0) {

    CosNaming::NamingContext_var nc2
      = CosNaming::NamingContext::_narrow(o);
    if (CORBA::is_nil(nc2)) {
      cerr << ts.t() << "bind: IOR not a NamingContext." << endl;
      throw ParseError();
    }
    nc->rebind_context(name, nc2);

  } else {

    nc->rebind(name, o);

  }

  delete [] bindingType;
}



void
omniNameslog::putUnbind(CosNaming::NamingContext_ptr nc, const CosNaming::Name& n,
	       ostream& file)
{
  file << "unbind ";
  CORBA::String_var s = orb->object_to_string(nc);
  putString(s, file);
  file << ' ';
  putString(n[0].id, file);
  file << ' ';
  putString(n[0].kind, file);
  file << '\n' << flush;
  if (!file) throw IOError();
}


void
omniNameslog::getUnbind(istream& file)
{
  //
  // First arg is NamingContext IOR.
  //

  char* str;
  getNonfinalString(str, file);
  CORBA::Object_var o;
  try {
    o = orb->string_to_object(str);
  } catch (...) {
    cerr << ts.t() << "getUnbind: invalid IOR." << endl;
    delete [] str;
    throw ParseError();
  }
  delete [] str;

  CosNaming::NamingContext_var nc = CosNaming::NamingContext::_narrow(o);
  if (CORBA::is_nil(nc)) {
    cerr << ts.t() << "getUnbind: IOR not a NamingContext." << endl;
    throw ParseError();
  }

  //
  // 2nd & 3rd args are name id and kind.
  //

  CosNaming::Name name(1);
  name.length(1);

  getNonfinalString(str, file);
  name[0].id = str;

  getFinalString(str, file);
  name[0].kind = str;

  nc->unbind(name);
}


void
omniNameslog::putKey(const PortableServer::ObjectId& id, ostream& file)
{
  file << hex;
  for (int i = 0; i < id.length(); i++) {
#if !defined(__SUNPRO_CC) || __SUNPRO_CC < 0x500
    file << setfill('0') << setw(2) << (int)id[i];
#else
    // Eventually, use the following for all standard C++ compilers
    file << std::setfill('0') << std::setw(2) << (int)id[i];
#endif
  }
  file << dec;
}


void
omniNameslog::getKey(PortableServer::ObjectId& id, istream& file)
{
  char* str;
  getFinalString(str, file);

  int l = strlen(str) / 2;
  id.length(l);
  char* p = str;
  for (int i = 0; i < l; i++) {
    int n;
    sscanf(p,"%02x",&n);
    id[i] = n;
    p += 2;
  }
  delete [] str;
}


void
omniNameslog::putString(const char* str, ostream& file)
{
  for (int i = strlen(str); i > 0; i--, str++) {
    switch (*str) {
      case '\\':
	file.put('\\');
	file.put('\\');
	break;
      case ' ':
	file.put('\\');
	file.put(' ');
	break;
      case '\n':
	file.put('\\');
	file.put('n');
	break;
      case '\t':
	file.put('\\');
	file.put('t');
	break;
      case '\r':
	file.put('\\');
	file.put('r');
	break;
      default:
	file.put(*str);
	break;
    }
  }
}


void
omniNameslog::getFinalString(char*& buf, istream& file)
{
  if (getString(buf, file) != '\n')
    throw ParseError();
  line++;
}

void
omniNameslog::getNonfinalString(char*& buf, istream& file)
{
  if (getString(buf, file) != ' ')
    throw ParseError();
}

int
omniNameslog::getString(char*& buf, istream& file)
{
  int bufsz = 512;
  buf = new char[bufsz];

  char* p = buf;

  int backslash = 0;

  while (1) {

    char c;

    if (!file.get(c)) {	// I/O problem or EOF
      delete [] buf;
      if (!file.eof())
	throw IOError();
      throw ParseError();
    }

    if (backslash) {

      backslash = 0;

      switch (c) {
      case '\\':
	*p++ = '\\';
	break;
      case ' ':
	*p++ = ' ';
	break;
      case 'n':
	*p++ = '\n';
	break;
      case 't':
	*p++ = '\t';
	break;
      case 'r':
	*p++ = '\r';
	break;
      default:
	cerr << ts.t() << "Unknown character following '\\' in log file"
	     << endl;
      }

    } else {

      switch (c) {
      case '\\':
	backslash = 1;
	break;
      case ' ':
      case '\n':
	*p = '\0';
	return (int)c;
	break;
      default:
	*p++ = c;
      }
    }

    if (p == (buf + bufsz)) {

      // buffer is too small

      char *obuf = buf;
      buf = new char[bufsz+bufsz];
      memcpy(buf, obuf, bufsz);
      delete [] obuf;
      p = buf + bufsz;
      bufsz += bufsz;
    }
  }
}
