// -*- Mode: C++; -*-

#ifndef _COS_NAMING_I_H_
#define _COS_NAMING_I_H_

#include "corba_wrappers.h"

// ------------------------------------------------------------- //
// Naming Service Wrapper: the following class provides a simple //
// interface for accessing a CORBA Naming Service.               //
//                                                               //
// Since the Naming Service supports hierarchical naming spaces, //
// the string representation of such names uses a char separator //
// to brake the string into components.                          //
// ------------------------------------------------------------- //

class CosNaming_i {
public:
  CosNaming_i(char separator='.');
  ~CosNaming_i();

  // Associate a name with an object. If the name is complex (i.e., it
  // specifies a naming hierarchy), its components must exist already.
  // If 'force' is not 0, this name will overwrite any past name which
  // may exist already.  On error, system or naming service exceptions
  // are thrown

  void bind(const char* objName, CORBA::Object_ptr objRef, int force=1);

  // Remove an existing association from the naming service. On error,
  // system or naming service exceptions are thrown

  void unbind(const char* objName);

  // Locate object reference given a name.  On error, system or naming
  // service exceptions are thrown

  CORBA::Object_ptr lookup(const char* objName);

private:
  CosNaming::NamingContext_var _rtcontext;
  char                         _separator;

  void initialize();
  void makeName(const char* objName, CosNaming::Name& name) const;
};

#endif
