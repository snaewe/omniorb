// -*- c++ -*-
//                          Package   : omniidl
// idlscope.h               Created on: 1999/10/11
//			    Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 1999 AT&T Laboratories Cambridge
//
//  This file is part of omniidl.
//
//  omniidl is free software; you can redistribute it and/or modify it
//  under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.
//
// Description:
//   
//   Definitions for scope manipulation

// $Id$
// $Log$
// Revision 1.5.2.1  2000/07/17 10:36:04  sll
// Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.
//
// Revision 1.6  2000/07/13 15:25:52  dpg1
// Merge from omni3_develop for 3.0 release.
//
// Revision 1.3  1999/11/02 17:07:25  dpg1
// Changes to compile on Solaris.
//
// Revision 1.2  1999/10/29 10:01:50  dpg1
// Global scope initialisation changed.
//
// Revision 1.1  1999/10/27 14:05:55  dpg1
// *** empty log message ***
//

#ifndef _idlscope_h_
#define _idlscope_h_

#include <idlutil.h>


// Class to represent an absolute or relative scoped name as a list.
class ScopedName {
public:

  class Fragment {
  public:
    // Constructor copies identifier
    Fragment(const char* identifier) :
      next_(0), identifier_(idl_strdup(identifier)) {}

    ~Fragment() {
      delete [] identifier_;
    }

    inline const char* identifier() { return identifier_; }
    inline Fragment*   next()       { return next_; };

  protected:
    Fragment* next_;

  private:
    char* identifier_;

    friend class ScopedName;
  };

  ScopedName(const char* identifier, _CORBA_Boolean absolute);

  // Copy constructor
  ScopedName(const ScopedName* sn);

  ~ScopedName();

  // Return the scope list
  Fragment* scopeList() const { return scopeList_; }

  // Is the name absolute (i.e. ::A::... rather than A::...)
  _CORBA_Boolean absolute() const { return absolute_; }

  // toString() returns a new string containing the stringified
  // name. The caller is responsible for deleting it.
  char*          toString() const;
  _CORBA_Boolean equal(const ScopedName* sn) const;

  // Append a new fragment
  void append(const char* identifier);

private:
  Fragment*      scopeList_;
  Fragment*      last_;
  _CORBA_Boolean absolute_;
};


// Class to represent a scope

class Decl;
class IdlType;
class InheritSpec;
class ValueInheritSpec;

class Scope {
public:

  class Entry;			// Entry in a scope
  class EntryList;		// Linked list of entries

  enum Kind { S_GLOBAL, S_MODULE, S_INTERFACE, S_STRUCT, S_EXCEPTION,
	      S_UNION, S_OPERATION, S_VALUE };

  // Static functions to return the current and global scopes
  static Scope* global()  { return global_; }
  static Scope* current() { return current_; }

  // Static functions to initialise and clear the global and CORBA:: scopes
  static void init();
  static void clear();

  // Functions to create new sub-scopes of the current scope. If
  // newModuleScope() is asked to create a module scope which already
  // exists, it re-opens the existing scope.
  Scope* newModuleScope   (const char* identifier, const char* file, int line);
  Scope* newInterfaceScope(const char* identifier, const char* file, int line);
  Scope* newStructScope   (const char* identifier, const char* file, int line);
  Scope* newExceptionScope(const char* identifier, const char* file, int line);
  Scope* newUnionScope    (const char* identifier, const char* file, int line);
  Scope* newOperationScope(const char* file, int line);
  Scope* newValueScope    (const char* identifier, const char* file, int line);

  // Change the current scope
  static void startScope(Scope* s);
  static void endScope();

  // Create an unnamed or named scope. If the nestedUse flag is true,
  // use of identifiers in child scopes are considered uses within
  // this scope. This is true for scopes created by interfaces and
  // valuetypes, but not those created by modules. If the parent scope
  // has nestedUse true, this scope sets it too.
  Scope(Scope* parent, Kind k, _CORBA_Boolean nestedUse,
	const char* file, int line);
  Scope(Scope* parent, const char* identifier,
	Kind k, _CORBA_Boolean nestedUse,
	const char* file, int line);

  ~Scope();

  // For interfaces, set a list of inherited Scopes. Checks that
  // inheritance has not added any clashing operation or attribute
  // names.
  void setInherited(InheritSpec*      inherited, const char* file, int line);
  void setInherited(ValueInheritSpec* inherited, const char* file, int line);

  // Query interface
  Scope*            parent()     const { return parent_; }
  Kind              kind()       const { return kind_; }
  const char*       identifier() const { return identifier_; }
  const ScopedName* scopedName() const { return scopedName_; }
  _CORBA_Boolean    nestedUse()  const { return nestedUse_; }
  Entry*            entries()    const { return entries_; }

  // Functions to lookup and add entries to the scope, reporting any
  // violations of the IDL scope rules as errors.

  // Find an entry in this scope.
  Entry* find(const char* identifier) const;

  // Find an entry in this scope, ignoring case
  Entry* iFind(const char* identifier) const;

  // Find entries in this scope or inherited scopes. Does not return
  // USE or PARENT entries.
  EntryList* findWithInheritance(const char* identifier) const;
  EntryList* iFindWithInheritance(const char* identifier) const;

  // Find an entry based on a ScopedName. File and line requesting the
  // find are given so errors can be reported nicely.
  const Entry* findScopedName(const ScopedName* sn,
			      const char* file, int line) const;

  // Find an entry based on a ScopedName, and mark it as used in this
  // scope (and any parent scopes with nestedUse true).
  const Entry* findForUse(const ScopedName* sn, const char* file, int line);

  void addUse(const ScopedName* sn, const char* file, int line);


  // The following add functions take identifiers with _ escape
  // characters intact, so they can properly detect clashes with
  // keywords.

  // Add a module, or do nothing if the module was already added
  void addModule(const char* identifier,
		 Scope* scope,
		 Decl* decl,
		 const char* file, int line);

  // Add a declaration
  void addDecl(const char* identifier,
	       Scope*      scope,   // Scope formed by this declaration, if any
	       Decl*       decl,    // Decl object for this declaration
	       IdlType*    idltype, // IdlType for this declaration
	       const char* file, int line);

  // Add an operation or attribute
  void addCallable(const char* identifier, Scope* scope, Decl* decl,
		   const char* file, int line);

  // Add an inherited operation or attribute
  void addInherited(const char* identifier, Scope* scope, Decl* decl,
		    Entry* inh_from, const char* file, int line);

  // Add an instance
  void addInstance(const char* identifier,
		   Decl* decl,	      // Declaration of the instance
		   IdlType* idltype,  // Type of the instance
		   const char* file, int line);

  // Remove an entry. Only used to remove a forward declared interface
  // when the full definition comes along.
  void remEntry(Entry* e);

  class Entry {
  public:

    enum Kind {
      E_MODULE,			// Module
      E_DECL,			// Declaration
      E_CALLABLE,		// Operation or attribute
      E_INHERITED,		// Inherited callable
      E_INSTANCE,		// Instance of a type
      E_USE,			// Identifier introduced through use
      E_PARENT			// Name of enclosing scope
    };

    Entry(const Scope* container, Kind kind, const char* identifier,
	  Scope* scope, Decl* decl, IdlType* idltype, Entry* inh_from,
	  const char* file, int line);

    ~Entry();

    const Scope*      container()  const { return container_; }
    Kind              kind()       const { return kind_; }
    const char*       identifier() const { return identifier_; }
    const ScopedName* scopedName() const { return scopedName_; }
    const char*       file()       const { return file_; }
    int               line()       const { return line_; }

    // Scope, Decl, IdlType, and Entry inherited from, if appropriate,
    // null if not
    Scope*            scope()      const { return scope_; }
    Decl*             decl()       const { return decl_; }
    IdlType*          idltype()    const { return idltype_; }
    Entry*            inh_from()   const { return inh_from_; }

    // Linked list inside Scope
    Entry*            next()       const { return next_; }

  private:
    const Scope*      container_;
    Kind              kind_;
    char*             identifier_;
    ScopedName*       scopedName_;
    Scope*            scope_;
    Decl*             decl_;
    IdlType*          idltype_;
    Entry*            inh_from_;
    char*             file_;
    int               line_;
    Entry*            next_;
    
    friend class Scope;
  };

  class EntryList {
  public:
    EntryList(const Entry* e) : head_(e), next_(0) { last_ = this; }

    ~EntryList() { if (next_) delete next_; }

    const Entry* head() const { return head_; }
    EntryList*   tail() const { return next_; }

    void append(EntryList* el) {
      last_->next_ = el;
      last_        = el->last_;
    }
    void merge(EntryList* ml);

  private:
    const Entry* head_;

  protected:
    EntryList* next_;
    EntryList* last_;
  };

private:
  Scope*            parent_;
  Kind              kind_;
  char*             identifier_;
  ScopedName*       scopedName_;
  _CORBA_Boolean    nestedUse_;
  Entry*            entries_;
  Entry*            last_;
  InheritSpec*      inherited_;
  ValueInheritSpec* valueInherited_;

  static Scope*     global_;
  static Scope*     current_;

  void appendEntry(Entry* e);
  _CORBA_Boolean keywordClash(const char* identifier,
			      const char* file, int line);
};

#endif // _idlscope_h_
