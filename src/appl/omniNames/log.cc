#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <iostream.h>
#include "NamingContext_i.h"
#include "ObjectBinding.h"
#include "log.h"
#include "iomanip.h"
#include "default.h"

#ifdef DEBUG_LOG
#define DB(x) x
#else
#define DB(x)
#endif


typedef char ts[27];

static char *
timestamp(ts& v)
{
  time_t t = time(NULL);
  char *p = ctime(&t);
  strncpy(v,p,24);
  v[24] = ':';
  v[25] = ' ';
  v[26] = '\0';
  return(v);
}


log::log(int& p) : port(p)
{
  ts t;

  DB(cerr << "log::log called" << endl);
  
  char *logname;

  if ((logname = getenv("OMNINAMES_LOGNAME")) == NULL)
    logname = DEFAULT_LOGNAME;
  
  // logname must start with '/', i.e. an absolute path name
  if (logname[0] != '/') {
    cerr << timestamp(t) << "Error: OMNINAMES_LOGNAME (" << logname
	 << ") is not an absolute path name." << endl;
    exit(1);
  }

  // claim the global lock as a writer (ie exclusive).

  NamingContext_i::lock.writerIn();

  startingUp = 1;

  active = new char[strlen(logname)+strlen(".log")+1];
  sprintf(active,"%s.log",logname);
  backup = new char[strlen(logname)+strlen(".bak")+1];
  sprintf(backup,"%s.bak",logname);
  checkpt = new char[strlen(logname)+strlen(".ckp")+1];
  sprintf(checkpt,"%s.ckp",logname);

  if (port != 0) {

    //
    // starting for the first time - make sure log file doesn't exist
    //

    firstTime = 1;

    struct stat sb;

    if (stat(active,&sb) == 0) {
      cerr << timestamp(t) << "Error: log file '" << active
	   << "' exists.  Can't use -start option." << endl;
      exit(1);
    }
    if (stat(backup,&sb) == 0) {
      cerr << timestamp(t) << "Error: backup file '" << backup
	   << "' exists.  Can't use -start option." << endl;
      exit(1);
    }
    if (stat(checkpt,&sb) == 0) {
      cerr << timestamp(t) << "Error: checkpoint file '" << checkpt
	   << "' exists.  Can't use -start option." << endl;
      exit(1);
    }

  } else {

    //
    // restart - get port info from log file
    //

    firstTime = 0;

    ifstream initf(active);

    if (!initf) {
      cerr << timestamp(t) << "Warning: Can't open log file '" << active
	   << "', trying '"<< backup << "' instead." << endl;
      initf.close();
      initf.open(backup, ios::in);
      if (!initf) {
	cerr << timestamp(t) << "Error: Can't open backup file '"
	     << backup << "' either." << endl;
	cerr << timestamp(t)
	     << "Use -start option to start omniNames for the first time."
	     << endl;
	exit(1);
      }

      cerr << timestamp(t) << "Succeeded in opening the backup file." << endl;
    }

    char* str;

    if (!getString(str, initf)) {
      cerr << timestamp(t) << "Error reading log file" << endl;
      exit(1);
    }

    if (strcmp(str, "port") != 0) {
      cerr << timestamp(t)
	   << "Error: log file doesn't start with \"port\" command." << endl;
      exit(1);
    }

    delete [] str;

    if (!getString(str, initf)) {
      cerr << timestamp(t) << "Error reading log file" << endl;
      exit(1);
    }

    port = atoi(str);

    if (port == 0) {
      cerr << timestamp(t)
	   << "Error: invalid port specified in log file." << endl;
      exit(1);
    } 

    p = port;

    delete [] str;

    initf.close();

    // remove checkpoint. This will protect us from trouble if the previous
    // incarnation crashed during checkpointing and at the
    // time when both active and checkpoint are linked to the same file.
    (void) unlink(checkpt);
  }
}


void
log::redo(CORBA::ORB_ptr o, CORBA::BOA_ptr b)
{
  ts t;

  DB(cerr << "log::redo called" << endl);

  orb = o;
  boa = b;

  if (firstTime) {

    cerr << timestamp(t) << "Starting omniNames for the first time." << endl;

    logf.open(active, ios::out);
    if (!logf) {
      cerr << timestamp(t) << "Error: couldn't open log file '" << active
	   << "' for writing" << endl;
      exit(1);
    }

    putPort(port, logf);

    omniORB::objectKey k;
    omniORB::generateNewKey(k);

    putCreate(k, logf);

    logf.close();

    cerr << timestamp(t) << "Wrote initial log file." << endl;
  }

  ifstream initf(active);

  if (!initf) {
    initf.close();
    initf.open(backup, ios::in);
    if (!initf) {
      cerr << timestamp(t) << "Error: Can't open log file '" << active
	   << "' or backup file '" << backup << "'." << endl;
      cerr << timestamp(t)
	   << "Use -start option to start omniNames for the first time."
	   << endl;
      exit(1);
    }
  }

  while (initf) {

    char* cmd;

    if (!getString(cmd, initf)) break;

    if (strcmp(cmd, "port") == 0) {

      while (initf && (initf.get() != '\n'));	// ignore rest of line

    } else if (strcmp(cmd, "create") == 0) {

      if (!getCreate(initf)) break;

    } else if (strcmp(cmd, "destroy") == 0) {

      if (!getDestroy(initf)) break;

    } else if (strcmp(cmd, "bind") == 0) {

      if (!getBind(initf)) break;

    } else if (strcmp(cmd, "unbind") == 0) {

      if (!getUnbind(initf)) break;

    } else {

      cerr << timestamp(t) << "Warning: unknown command '" << cmd
	   << "' in log file - ignoring" << endl;

      while (initf && (initf.get() != '\n'));	// ignore rest of line
    }

    delete [] cmd;
  }

  initf.close();

  cerr << timestamp(t) << "Read log file successfully" << endl;

  CosNaming::NamingContext_ptr rootContext
    = NamingContext_i::headContext->_this();

  char* p = orb->object_to_string(rootContext);
  cerr << timestamp(t) << "Root context is " << p << endl;

  CORBA::release(rootContext);	// dispose of the object reference

  logf.open(active, ios::out | ios::ate);
  if (!logf) {
    cerr << "couldn't open log file '" << active << "' for writing" << endl;
    exit(1);
  }

  startingUp = 0;
  NamingContext_i::lock.writerOut();
}


void
log::create(const omniORB::objectKey& key)
{
  if (!startingUp) putCreate(key, logf);
}

void
log::destroy(CosNaming::NamingContext_ptr nc)
{
  if (!startingUp) putDestroy(nc, logf);
}

void
log::bind(CosNaming::NamingContext_ptr nc, const CosNaming::Name& n,
	  CORBA::Object_ptr obj, CosNaming::BindingType t)
{
  if (!startingUp) putBind(nc, n, obj, t, logf);
}

void
log::unbind(CosNaming::NamingContext_ptr nc, const CosNaming::Name& n)
{
  if (!startingUp) putUnbind(nc, n, logf);
}


void
log::checkpoint(void)
{
  ts t;
  
  cerr << timestamp(t) << "Checkpointing: phase 1. Prepare." << endl;

  ofstream ckpf(checkpt,ios::out|ios::trunc);

  if (!ckpf) {
    cerr << timestamp(t) << "Fatal: Can't open checkpoint file '"
	 << checkpt << "' for writing." << endl;
    throw IOError();
  }

  putPort(port, ckpf);

  NamingContext_i::lock.readerIn();

  for (NamingContext_i* nci = NamingContext_i::headContext;
       nci;
       nci = nci->next) {
    putCreate(nci->_key(), ckpf);
  }

  for (nci = NamingContext_i::headContext; nci; nci = nci->next) {
    for (ObjectBinding* ob = nci->headBinding; ob; ob = ob->next) {
      putBind(nci->_this(), ob->binding.binding_name, ob->object,
	      ob->binding.binding_type, ckpf);
    }
  }

  NamingContext_i::lock.readerOut();

  ckpf.close();

  // Now commit the checkpoint to become the active log.

  cerr << timestamp(t) << "Checkpointing: phase 2. Commit." << endl;

  logf.close();             // Re-open log file after the checkpoint
                            // is committed.

  if (unlink(backup) < 0) {
    struct stat sb;

    // Don't make a fuss if the backup file doesn't exist
    if (stat(backup,&sb) == 0 || errno != ENOENT) {
      DB(cerr << timestamp(t) << "DB: exception(IOError)@"
	 <<__FILE__ << ":" << __LINE__ << endl);
      throw IOError();
    }
  }

  // fail here will leave behind checkpoint
  if (link(active,backup) < 0) {
    DB(cerr << timestamp(t) << "DB: exception(IOError)@"
       <<__FILE__ << ":" << __LINE__ << endl);
    throw IOError();
  }

  // fail here will leave behind active and backup points to the same file
  if (unlink(active) < 0) {
    DB(cerr << timestamp(t) << "DB: exception(IOError)@"
       <<__FILE__ << ":" << __LINE__ << endl);
    throw IOError();
  }

  // fail here will leave behind no active but backup will point to the
  // previous active
  if (link(checkpt,active) < 0) {
    DB(cerr << timestamp(t) << "DB: exception(IOError)@"
       <<__FILE__ << ":" << __LINE__ << endl);
    throw IOError();
  }

  // fail here will leave behind active and checkpoint point to the same
  // file.
  if (unlink(checkpt) < 0) {
    DB(cerr << timestamp(t) << "DB: exception(IOError)@"
       <<__FILE__ << ":" << __LINE__ << endl);
    throw IOError();
  }

  logf.open(active,ios::out|ios::ate);
  if (!logf) {
    cerr << timestamp(t) << "Fatal: can't re-open log file for writing "
	 << "after checkpoint has been taken." << endl;
    throw IOError();
  }

  cerr << timestamp(t) << "Checkpointing: completed." << endl;
}

void
log::putPort(int p, ostream& file)
{
  file << "port " << p << '\n' << flush;
}

void
log::putCreate(const omniORB::objectKey& key, ostream& file)
{
  file << "create ";
  putKey(key, file);
  file << '\n' << flush;
}

void
log::putDestroy(CosNaming::NamingContext_ptr nc, ostream& file)
{
  file << "destroy ";
  CORBA::String_var s = orb->object_to_string(nc);
  putString(s, file);
  file << '\n' << flush;
}

void
log::putBind(CosNaming::NamingContext_ptr nc, const CosNaming::Name& n,
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
}

void
log::putUnbind(CosNaming::NamingContext_ptr nc, const CosNaming::Name& n,
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
}


void
log::putString(const char* str, ostream& file)
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
log::putKey(const omniORB::objectKey& key, ostream& file)
{
  omniORB::seqOctets* os = omniORB::keyToOctetSequence(key);
  file << hex;
  for (int i = 0; i < os->length(); i++) {
    file << setfill('0') << setw(2) << (int)(*os)[i];
  }
  file << dec;
}


int
log::getString(char*& buf, istream& file)
{
  ts t;

  int bufsz = 512;
  buf = new char[bufsz];

  char* p = buf;

  int backslash = 0;

  while (1) {

    char c;

    if (!file.get(c)) {	// I/O problem or premature EOF
      if (p != buf) {
	*p = '\0';
	cerr << timestamp(t) << "Incomplete record in log file - ignoring"
	     << endl;
	cerr << timestamp(t) << "Info: truncated record dump:\n"
	     << buf << endl;
      }
      delete [] buf;
      return 0;
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
	cerr << timestamp(t)
	     << "Unknown character following '\\' in log file" << endl;
      }

    } else {

      switch (c) {
      case '\\':
	backslash = 1;
	break;
      case ' ':
      case '\n':
	*p = '\0';
	return 1;
	break;
      default:
	*p++ = c;
      }
    }

    if (p >= (buf + bufsz)) {

      // buffer is too small

      char *obuf = buf;
      buf = new char[bufsz+bufsz];
      memcpy(buf, obuf, bufsz);
      delete [] obuf;
      bufsz += bufsz;
    }
  }
}


int
log::getKey(omniORB::objectKey& k, istream& file)
{
  char* str;
  if (!getString(str, file)) return 0;

  int l = strlen(str) / 2;
  omniORB::seqOctets os(l);
  os.length(l);
  char* p = str;
  for (int i = 0; i < l; i++) {
    int n;
    sscanf(p,"%02x",&n);
    os[i] = n;
    p += 2;
  }
  k = omniORB::octetSequenceToKey(os);  
  delete [] str;
  return 1;
}


int
log::getCreate(istream& file)
{
  //
  // Argument to "create" is the object key of the naming context.
  //

  omniORB::objectKey k;

  if (!getKey(k, file)) return 0;

  NamingContext_i* rc = new NamingContext_i(boa, k, this);

  return 1;
}


int
log::getDestroy(istream& file)
{
  ts t;

  //
  // Argument to "destroy" is NamingContext IOR.
  //

  char* str;
  if (!getString(str, file)) return 0;
  CORBA::Object_var o = orb->string_to_object(str);
  delete [] str;

  CosNaming::NamingContext_var nc = CosNaming::NamingContext::_narrow(o);
  if (CORBA::is_nil(nc)) {
    cerr << timestamp(t) << "getDestroy: IOR not a NamingContext." << endl;
    return 0;
  }

  nc->destroy();

  return 1;
}


int
log::getBind(istream& file)
{
  ts t;

  //
  // First arg is NamingContext IOR.
  //

  char* str;
  if (!getString(str, file)) return 0;
  CORBA::Object_var o = orb->string_to_object(str);
  delete [] str;

  CosNaming::NamingContext_var nc = CosNaming::NamingContext::_narrow(o);
  if (CORBA::is_nil(nc)) {
    cerr << timestamp(t) << "getBind: IOR not a NamingContext." << endl;
    return 0;
  }

  //
  // 2nd & 3rd args are name id and kind.
  //

  CosNaming::Name name(1);
  name.length(1);

  if (!getString(str, file)) return 0;
  name[0].id = str;

  if (!getString(str, file)) return 0;
  name[0].kind = str;

  //
  // 4th arg is binding type.
  //

  char* bindingType;
  if (!getString(bindingType, file)) return 0;

  //
  // 5th arg is object IOR.
  //

  if (!getString(str, file)) return 0;
  o = orb->string_to_object(str);
  delete [] str;

  if (strcmp(bindingType, "ncontext") == 0) {

    CosNaming::NamingContext_var nc2
      = CosNaming::NamingContext::_narrow(o);
    if (CORBA::is_nil(nc2)) {
      cerr << timestamp(t) << "bind: IOR not a NamingContext." << endl;
      return 0;
    }
    nc->bind_context(name, nc2);

  } else {

    nc->bind(name, o);

  }

  delete [] bindingType;

  return 1;
}


int
log::getUnbind(istream& file)
{
  ts t;

  //
  // First arg is NamingContext IOR.
  //

  char* str;
  if (!getString(str, file)) return 0;
  CORBA::Object_var o = orb->string_to_object(str);
  delete [] str;

  CosNaming::NamingContext_var nc = CosNaming::NamingContext::_narrow(o);
  if (CORBA::is_nil(nc)) {
    cerr << timestamp(t) << "getUnbind: IOR not a NamingContext." << endl;
    return 0;
  }

  //
  // 2nd & 3rd args are name id and kind.
  //

  CosNaming::Name name(1);
  name.length(1);

  if (!getString(str, file)) return 0;
  name[0].id = str;

  if (!getString(str, file)) return 0;
  name[0].kind = str;

  nc->unbind(name);

  return 1;
}
