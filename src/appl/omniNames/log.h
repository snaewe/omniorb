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

  int startingUp;

  int firstTime;

  //
  // functions to write to a file
  //

  void putString(const char* str, ostream& file);

  void putKey(const omniORB::objectKey& key, ostream& file);

  void putPort(int port, ostream& file);

  void putCreate(const omniORB::objectKey& key, ostream& file);

  void putDestroy(CosNaming::NamingContext_ptr nc, ostream& file);

  void putBind(CosNaming::NamingContext_ptr nc,
	       const CosNaming::Name& n, CORBA::Object_ptr obj,
	       CosNaming::BindingType t, ostream& file);

  void putUnbind(CosNaming::NamingContext_ptr nc, const CosNaming::Name& n,
		 ostream& file);

  //
  // functions to read from a file
  //

  int getString(char*& buf, istream& file);

  int getKey(omniORB::objectKey& k, istream& file);

  int getCreate(istream& file);

  int getDestroy(istream& file);

  int getBind(istream& file);

  int getUnbind(istream& file);

public:

  class IOError {};

  log(int& port);

  void redo(CORBA::ORB_ptr o, CORBA::BOA_ptr b);

  void create(const omniORB::objectKey& key);
  void destroy(CosNaming::NamingContext_ptr nc);
  void bind(CosNaming::NamingContext_ptr nc,
	    const CosNaming::Name& n, CORBA::Object_ptr obj,
	    CosNaming::BindingType t);
  void unbind(CosNaming::NamingContext_ptr nc, const CosNaming::Name& n);

  void checkpoint(void);

};

#endif
