#ifndef _log_h_
#define _log_h_

#include <omniORB2/CORBA.h>
#include <fstream.h>

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

  log(int& port);

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
