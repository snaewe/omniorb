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
// Revision 1.9.2.1  2003/03/23 21:01:44  dgrisby
// Start of omniORB 4.1.x development branch.
//
// Revision 1.5.2.4  2001/10/29 17:42:43  dpg1
// Support forward-declared structs/unions, ORB::create_recursive_tc().
//
// Revision 1.5.2.3  2000/10/27 16:31:10  dpg1
// Clean up of omniidl dependencies and types, from omni3_develop.
//
// Revision 1.5.2.2  2000/10/10 10:18:51  dpg1
// Update omniidl front-end from omni3_develop.
//
// Revision 1.3.2.2  2000/09/19 09:14:26  dpg1
// Scope::Entry::Kind renamed to Scope::Entry::EntryKind to avoid
// problems with over-keen compilers
//
// Revision 1.3.2.1  2000/08/29 15:20:29  dpg1
// New relativeScope() function. New -i flag to enter interactive loop
// after parsing
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

    inline const char* identifier() const { return identifier_; }
    inline Fragment*   next()       const { return next_; };

  protected:
    Fragment* next_;

  private:
    char* identifier_;

    friend class ScopedName;
  };

  ScopedName(const char* identifier, IDL_Boolean absolute);

  // Copy constructors
  ScopedName(const ScopedName* sn);
  ScopedName(const Fragment*   frags, IDL_Boolean absolute);

  ~ScopedName();

  // Return the scope list
  Fragment* scopeList() const { return scopeList_; }

  // Is the name absolute (i.e. ::A::... rather than A::...)
  IDL_Boolean absolute() const { return absolute_; }

  // toString() returns a new string containing the stringified
  // name. The caller is responsible for deleting it. If qualify is
  // true, and the name is absolute, prefix with ::, otherwise do not.
  char* toString(IDL_Boolean qualify=0) const;

  IDL_Boolean equal(const ScopedName* sn) const;

  // Append a new fragment
  void append(const char* identifier);

private:
  Fragment*      scopeList_;
  Fragment*      last_;
  IDL_Boolean    absolute_;
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
  Scope(Scope* parent, Kind k, IDL_Boolean nestedUse,
	const char* file, int line);
  Scope(Scope* parent, const char* identifier,
	Kind k, IDL_Boolean nestedUse,
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
  IDL_Boolean       nestedUse()  const { return nestedUse_; }
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
  // find are given so errors can be reported nicely. If file and line
  // are zero, do not report errors.
  const Entry* findScopedName(const ScopedName* sn,
			      const char* file = 0, int line = 0) const;

  // Find an entry based on a ScopedName, and mark it as used in this
  // scope (and any parent scopes with nestedUse true).
  const Entry* findForUse(const ScopedName* sn, const char* file, int line);

  void addUse(const ScopedName* sn, const char* file, int line);

  // Given source and destination ScopedNames, construct a relative or
  // absolute ScopedName which uniquely identifies the destination
  // from within the scope of the source. Returns 0 if either scoped
  // name does not exist, or is not absolute.
  static ScopedName* relativeScopedName(const ScopedName* from,
					const ScopedName* to);


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

    enum EntryKind {
      E_MODULE,			// Module
      E_DECL,			// Declaration
      E_CALLABLE,		// Operation or attribute
      E_INHERITED,		// Inherited callable
      E_INSTANCE,		// Instance of a type
      E_USE,			// Identifier introduced through use
      E_PARENT			// Name of enclosing scope
    };

    Entry(const Scope* container, EntryKind kind, const char* identifier,
	  Scope* scope, Decl* decl, IdlType* idltype, Entry* inh_from,
	  const char* file, int line);

    ~Entry();

    const Scope*      container()  const { return container_; }
    EntryKind         kind()       const { return kind_; }
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
    EntryKind         kind_;
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
  IDL_Boolean       nestedUse_;
  Entry*            entries_;
  Entry*            last_;
  InheritSpec*      inherited_;
  ValueInheritSpec* valueInherited_;

  static Scope*     global_;
  static Scope*     current_;

  void appendEntry(Entry* e);
  IDL_Boolean keywordClash(const char* identifier,
			   const char* file, int line);
};

#endif // _idlscope_h_
