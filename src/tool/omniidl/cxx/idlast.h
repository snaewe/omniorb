// -*- c++ -*-
//                          Package   : omniidl
// idlast.h                 Created on: 1999/10/07
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
//   Definitions for abstract syntax tree classes

// $Id$
// $Log$
// Revision 1.10.2.1  2000/07/17 10:36:02  sll
// Merged from omni3_develop the diff between omni3_0_0_pre3 and omni3_0_0.
//
// Revision 1.11  2000/07/13 15:25:54  dpg1
// Merge from omni3_develop for 3.0 release.
//
// Revision 1.8.2.4  2000/06/08 14:36:19  dpg1
// Comments and pragmas are now objects rather than plain strings, so
// they can have file,line associated with them.
//
// Revision 1.8.2.3  2000/06/05 18:13:27  dpg1
// Comments can be attached to subsequent declarations (with -K). Better
// idea of most recent decl in operation declarations
//
// Revision 1.8.2.2  2000/03/07 10:36:38  dpg1
// More sensible idea of the "most recent" declaration.
//
// Revision 1.8.2.1  2000/03/06 15:03:48  dpg1
// Minor bug fixes to omniidl. New -nf and -k flags.
//
// Revision 1.8  2000/02/04 12:17:09  dpg1
// Support for VMS.
//
// Revision 1.7  1999/11/17 17:17:00  dpg1
// Changes to remove static initialisation of objects.
//
// Revision 1.6  1999/11/02 17:07:27  dpg1
// Changes to compile on Solaris.
//
// Revision 1.5  1999/11/01 20:19:56  dpg1
// Support for union switch types declared inside the switch statement.
//
// Revision 1.4  1999/11/01 10:05:01  dpg1
// New file attribute to AST.
//
// Revision 1.3  1999/10/29 15:43:02  dpg1
// Code to detect recursive structs and unions.
//
// Revision 1.2  1999/10/29 10:00:43  dpg1
// Added code to find a value for the default case in a union.
//
// Revision 1.1  1999/10/27 14:05:59  dpg1
// *** empty log message ***
//

#ifndef _idlast_h_
#define _idlast_h_

#include <idlutil.h>
#include <idltype.h>
#include <idlexpr.h>
#include <idlscope.h>
#include <idlvisitor.h>

#include <stdio.h>

class Decl;

// Pragma class stores a list of pragmas:
class Pragma {
public:
  Pragma(const char* pragmaText, const char* file, int line)
    : pragmaText_(idl_strdup(pragmaText)),
      file_(idl_strdup(file)), line_(line), next_(0) {}

  ~Pragma() {
    delete [] pragmaText_;
    delete [] file_;
    if (next_) delete next_;
  }

  const char* pragmaText() const { return pragmaText_; }
  const char* file()       const { return file_; }
  int         line()       const { return line_; }
  Pragma*     next()       const { return next_; }

  static void add(const char* pragmaText, const char* file, int line);

private:
  char*   pragmaText_;
  char*   file_;
  int     line_;
  Pragma* next_;

  friend class AST;
  friend class Decl;
};

// Comment class stores a list of comment strings:
class Comment {
public:
  Comment(const char* commentText, const char* file, int line)
    : commentText_(idl_strdup(commentText)),
      file_(idl_strdup(file)), line_(line), next_(0) {
    mostRecent_ = this;
  }

  ~Comment() {
    delete [] commentText_;
    delete [] file_;
    if (next_) delete next_;
  }

  const char* commentText() const { return commentText_; }
  const char* file()        const { return file_; }
  int         line()        const { return line_; }
  Comment*    next()        const { return next_; }

  static void add   (const char* commentText, const char* file, int line);
  static void append(const char* commentText);
  static void clear() { mostRecent_ = 0; }

  static Comment* grabSaved();
  // Return any saved comments, and clear the saved comment list

private:
  char*           commentText_;
  char*           file_;
  int             line_;
  Comment*        next_;
  static Comment* mostRecent_;
  static Comment* saved_;

  friend class AST;
  friend class Decl;
};



// AST class represents the whole IDL definition
class AST {
public:
  AST();
  ~AST();
  static AST*           tree();
  static _CORBA_Boolean process(FILE* f, const char* name);
  static void           clear();

  Decl*       declarations()              { return declarations_; }
  const char* file()                      { return file_; }
  Pragma*     pragmas()                   { return pragmas_; }
  Comment*    comments()                  { return comments_; }

  void        accept(AstVisitor& visitor) { visitor.visitAST(this); }

  void        setFile(const char* f);
  void        addPragma(const char* pragmaText, const char* file, int line);
  void        addComment(const char* commentText, const char* file, int line);

private:
  void        setDeclarations(Decl* d);

  Decl*       declarations_;
  char*       file_;
  static AST* tree_;
  Pragma*     pragmas_;
  Pragma*     lastPragma_;
  Comment*    comments_;
  Comment*    lastComment_;
  friend int  yyparse();
};


// Base declaration abstract class
class Decl {
public:
  // Declaration kinds
  enum Kind {
    D_MODULE, D_INTERFACE, D_FORWARD, D_CONST, D_DECLARATOR,
    D_TYPEDEF, D_MEMBER, D_STRUCT, D_EXCEPTION, D_CASELABEL,
    D_UNIONCASE, D_UNION, D_ENUMERATOR, D_ENUM, D_ATTRIBUTE,
    D_PARAMETER, D_OPERATION, D_NATIVE, D_STATEMEMBER,
    D_FACTORY, D_VALUEFORWARD, D_VALUEBOX, D_VALUEABS, D_VALUE
  };

  Decl(Kind kind, const char* file, int line, _CORBA_Boolean mainFile);
  virtual ~Decl();

  // Declaration kind
  Kind                kind()         const { return kind_; }
  virtual const char* kindAsString() const = 0;

  // Query interface
  const char*       file()       const { return file_; }
  int               line()       const { return line_; }
  _CORBA_Boolean    mainFile()   const { return mainFile_; }
  const Scope*      inScope()    const { return inScope_; }
  const Pragma*     pragmas()    const { return pragmas_; }
  const Comment*    comments()   const { return comments_; }

  // Linked list
  Decl* next() { return next_; }

  void append(Decl* d) {
    if (d) {
      last_->next_ = d;
      last_        = d;
    }
  }

  // Find a decl given a name. Does not mark the name used.
  static Decl* scopedNameToDecl(const char* file, int line,
				const ScopedName* sn);

  static Decl* mostRecent() { return mostRecent_; }
  static void  clear()      { mostRecent_ = 0; }

  // Visitor pattern accept(). The visitor is responsible for
  // recursively visiting children if it needs to
  virtual void accept(AstVisitor& visitor) = 0;

  void addPragma(const char* pragmaText, const char* file, int line);
  void addComment(const char* commentText, const char* file, int line);

private:
  Kind              kind_;
  char*             file_;
  int               line_;
  _CORBA_Boolean    mainFile_;
  const Scope*      inScope_;
  Pragma*           pragmas_;
  Pragma*           lastPragma_;
  Comment*          comments_;
  Comment*          lastComment_;

protected:
  static Decl*      mostRecent_;

  Decl* next_;
  Decl* last_;
};


// Mixin class for Decls with a RepoId
class DeclRepoId {
public:
  DeclRepoId(const char* identifier);
  ~DeclRepoId();

  // eidentifier() returns the identifier with _ escapes intact
  const char*       identifier()  const { return identifier_; }
  const char*       eidentifier() const { return eidentifier_; }
  const ScopedName* scopedName()  const { return scopedName_; }
  const char*       repoId()      const { return repoId_; }
  const char*       prefix()      const { return prefix_; }

  void setRepoId(const char* repoId, const char* file, int line);
  void setVersion(_CORBA_Short maj, _CORBA_Short min,
		  const char* file, int line);

  // Static set functions taking a Decl as an argument
  static void setRepoId(Decl* d, const char* repoId,
			const char* file, int line);
  static void setVersion(Decl* d, _CORBA_Short maj, _CORBA_Short min,
			 const char* file, int line);

  _CORBA_Boolean repoIdSet() const { return set_; }
  const char*    rifile()    const { return rifile_; }
  int            riline()    const { return riline_; }
  _CORBA_Short   rimaj()     const { return maj_; }
  _CORBA_Short   rimin()     const { return min_; }

private:
  void genRepoId();

  char*          identifier_;
  char*          eidentifier_;
  ScopedName*    scopedName_;
  char*          repoId_;
  char*          prefix_; // Prefix in force at time of declaration
  _CORBA_Boolean set_;    // True if repoId or version has been manually set
  char*          rifile_; // File where repoId or version was set
  int            riline_; // Line where repoId or version was set
  _CORBA_Short   maj_;
  _CORBA_Short   min_;
};


// Module
class Module : public Decl, public DeclRepoId {
public:
  Module(const char* file, int line, _CORBA_Boolean mainFile,
	 const char* identifier);

  virtual ~Module();

  const char* kindAsString() const { return "module"; }

  // Query interface
  Decl* definitions() const { return definitions_; }
  
  void accept(AstVisitor& visitor) { visitor.visitModule(this); }

  void finishConstruction(Decl* defs);

private:
  Decl*  definitions_;
};


// List of inherited interfaces
class InheritSpec {
public:
  InheritSpec(const ScopedName* sn, const char* file, int line);

  ~InheritSpec() { if (next_) delete next_; }

  // The ScopedName used in an inheritance specification may be a
  // typedef. In that case, decl() returns the Typedef declarator
  // object and interface() returns the actual Interface object.
  // Otherwise, both functions return the same Interface pointer.

  Interface*   interface() const { return interface_; }
  Decl*        decl()      const { return decl_; }
  const Scope* scope()     const { return scope_; }
  InheritSpec* next()      const { return next_; }

  void append(InheritSpec* is, const char* file, int line);

private:
  Interface*   interface_;
  Decl*        decl_;
  const Scope* scope_;

protected:
  InheritSpec* next_;
};



// Interface
class Interface : public Decl, public DeclRepoId {
public:
  Interface(const char* file, int line, _CORBA_Boolean mainFile,
	    const char* identifier, _CORBA_Boolean abstract,
	    InheritSpec* inherits);

  virtual ~Interface();

  const char* kindAsString() const { return "interface"; }

  // Queries
  _CORBA_Boolean abstract() const { return abstract_; }
  InheritSpec*   inherits() const { return inherits_; }
  Decl*          contents() const { return contents_; }
  Scope*         scope()    const { return scope_;    }
  IdlType*       thisType() const { return thisType_; }

  void accept(AstVisitor& visitor) { visitor.visitInterface(this); }

  void finishConstruction(Decl* decls);

private:
  _CORBA_Boolean abstract_;
  InheritSpec*   inherits_;
  Decl*          contents_;
  Scope*         scope_;
  IdlType*       thisType_;
};


// Forward-declared interface
class Forward : public Decl, public DeclRepoId {
public:
  Forward(const char* file, int line, _CORBA_Boolean mainFile,
	  const char* identifier, _CORBA_Boolean abstract);

  virtual ~Forward();

  const char* kindAsString() const { return "forward interface"; }

  // Query interface
  _CORBA_Boolean abstract()   const { return abstract_; }
  Interface*     definition() const;
  _CORBA_Boolean isFirst()    const { return !firstForward_; }
  IdlType*       thisType()   const { return thisType_; }

  void accept(AstVisitor& visitor) { visitor.visitForward(this); }

  void setDefinition(Interface* defn);

private:
  _CORBA_Boolean abstract_;
  Interface*     definition_;
  Forward*       firstForward_;
  IdlType*       thisType_;
};


// Constant
class Const : public Decl, public DeclRepoId {
public:
  Const(const char* file, int line, _CORBA_Boolean mainFile,
	IdlType* constType, const char* identifier, IdlExpr* expr);

  virtual ~Const();

  const char* kindAsString() const { return "constant"; }

  // Queries
  IdlType*      constType() const { return constType_; }
  IdlType::Kind constKind() const { return constKind_; }

  _CORBA_Short        constAsShort()      const;
  _CORBA_Long         constAsLong()       const;
  _CORBA_UShort       constAsUShort()     const;
  _CORBA_ULong        constAsULong()      const;
  _CORBA_Float        constAsFloat()      const;
  _CORBA_Double       constAsDouble()     const;
  _CORBA_Boolean      constAsBoolean()    const;
  _CORBA_Char         constAsChar()       const;
  _CORBA_Octet        constAsOctet()      const;
  const char*         constAsString()     const;
#ifdef HAS_LongLong
  _CORBA_LongLong     constAsLongLong()   const;
  _CORBA_ULongLong    constAsULongLong()  const;
#endif
#ifdef HAS_LongDouble
  _CORBA_LongDouble   constAsLongDouble() const;
#endif
  _CORBA_WChar        constAsWChar()      const;
  const _CORBA_WChar* constAsWString()    const;
  _CORBA_Fixed        constAsFixed()      const;
  Enumerator*         constAsEnumerator() const;

  void accept(AstVisitor& visitor) { visitor.visitConst(this); }

private:
  IdlType*       constType_;
  _CORBA_Boolean delType_;
  IdlType::Kind  constKind_;
  union {
    _CORBA_Short        short_;
    _CORBA_Long         long_;
    _CORBA_UShort       ushort_;
    _CORBA_ULong        ulong_;
#ifndef __VMS
    _CORBA_Float        float_;
    _CORBA_Double       double_;
#else
    float               float_;
    double              double_;
#endif
    _CORBA_Boolean      boolean_;
    _CORBA_Char         char_;
    _CORBA_Octet        octet_;
    char*               string_;
#ifdef HAS_LongLong
    _CORBA_LongLong     longlong_;
    _CORBA_ULongLong    ulonglong_;
#endif
#ifdef HAS_LongDouble
    _CORBA_LongDouble   longdouble_;
#endif
    _CORBA_WChar        wchar_;
    _CORBA_WChar*       wstring_;
    _CORBA_Fixed        fixed_;
    Enumerator*         enumerator_;
  } v_;
};


// Typedef

class ArraySize {
public:
  ArraySize(int size) : size_(size), next_(0), last_(0) {}

  ~ArraySize() { if (next_) delete next_; }

  int size()        const { return size_; }
  ArraySize* next() const { return next_; }

  void append(ArraySize* as) {
    if (last_) last_->next_ = as;
    else       next_ = as;
    last_ = as;
  }

private:
  int size_;

protected:
  ArraySize* next_;
  ArraySize* last_;
};


class Typedef;

class Declarator : public Decl, public DeclRepoId {
public:
  Declarator(const char* file, int line, _CORBA_Boolean mainFile,
	     const char* identifier, ArraySize* sizes);

  virtual ~Declarator();

  const char* kindAsString() const;

  // Queries
  ArraySize* sizes()    const { return sizes_; }
				// Null if a simple declarator

  // Only for typedef declarators
  IdlType*   thisType() const { return thisType_; }
  Typedef*   alias()    const { return alias_; } 

  void accept(AstVisitor& visitor) { visitor.visitDeclarator(this); }

  void setAlias(Typedef* td);

private:
  ArraySize*  sizes_;
  IdlType*    thisType_;
  Typedef*    alias_;
};


class Typedef : public Decl {
public:
  Typedef(const char* file, int line, _CORBA_Boolean mainFile,
	  IdlType* aliasType, _CORBA_Boolean constrType,
	  Declarator* declarators);

  virtual ~Typedef();

  const char* kindAsString() const { return "typedef"; }

  // Queries
  IdlType*       aliasType()   const { return aliasType_; }
  _CORBA_Boolean constrType()  const { return constrType_; }
  Declarator*    declarators() const { return declarators_; }

  void accept(AstVisitor& visitor) { visitor.visitTypedef(this); }

private:
  IdlType*       aliasType_;
  _CORBA_Boolean delType_;
  _CORBA_Boolean constrType_;
  Declarator*    declarators_;
};


// Struct member
class Member : public Decl {
public:
  Member(const char* file, int line, _CORBA_Boolean mainFile,
	 IdlType* memberType, _CORBA_Boolean constrType,
	 Declarator* declarators);
  virtual ~Member();

  const char* kindAsString() const { return "member"; }

  // Queries
  IdlType*       memberType()  const { return memberType_; }
  _CORBA_Boolean constrType()  const { return constrType_; }
  Declarator*    declarators() const { return declarators_; }

  void accept(AstVisitor& visitor) { visitor.visitMember(this); }

private:
  IdlType*       memberType_;
  _CORBA_Boolean delType_;
  _CORBA_Boolean constrType_;
  Declarator*    declarators_;
};



// Struct
class Struct : public Decl, public DeclRepoId {
public:
  Struct(const char* file, int line, _CORBA_Boolean mainFile,
	 const char* identifier);
  virtual ~Struct();

  const char* kindAsString() const { return "struct"; }

  // Queries
  Member*        members()   const { return members_; }
  IdlType*       thisType()  const { return thisType_; }
  _CORBA_Boolean recursive() const { return recursive_; }
  _CORBA_Boolean finished()  const { return finished_; }

  void accept(AstVisitor& visitor) { visitor.visitStruct(this); }

  void finishConstruction(Member* members);

  void setRecursive() { recursive_ = 1; }

private:
  Member*        members_;
  IdlType*       thisType_;
  _CORBA_Boolean recursive_;
  _CORBA_Boolean finished_;
};


// Exception
class Exception : public Decl, public DeclRepoId {
public:
  Exception(const char* file, int line, _CORBA_Boolean mainFile,
	    const char* identifier);
  virtual ~Exception();

  const char* kindAsString() const { return "exception"; }

  // Queries
  Member*        members()  const { return members_; }

  void accept(AstVisitor& visitor) { visitor.visitException(this); }

  void finishConstruction(Member* members);

private:
  Member*        members_;
};


// Union case label
class CaseLabel : public Decl {
public:
  CaseLabel(const char* file, int line, _CORBA_Boolean mainFile,
	    IdlExpr* value);
  virtual ~CaseLabel();

  const char* kindAsString() const { return "case label"; }

  _CORBA_Short        labelAsShort()      const;
  _CORBA_Long         labelAsLong()       const;
  _CORBA_UShort       labelAsUShort()     const;
  _CORBA_ULong        labelAsULong()      const;
  _CORBA_Boolean      labelAsBoolean()    const;
  _CORBA_Char         labelAsChar()       const;
#ifdef HAS_LongLong
  _CORBA_LongLong     labelAsLongLong()   const;
  _CORBA_ULongLong    labelAsULongLong()  const;
#endif
  _CORBA_WChar        labelAsWChar()      const;
  Enumerator*         labelAsEnumerator() const;

  inline _CORBA_Boolean isDefault() const { return isDefault_; }
  IdlType::Kind         labelKind() const { return labelKind_; }

  void accept(AstVisitor& visitor) { visitor.visitCaseLabel(this); }

  void setType(IdlType* type);
  void setDefaultShort     (_CORBA_Short     v) { v_.short_      = v; }
  void setDefaultLong      (_CORBA_Long      v) { v_.long_       = v; }
  void setDefaultUShort    (_CORBA_UShort    v) { v_.ushort_     = v; }
  void setDefaultULong     (_CORBA_ULong     v) { v_.ulong_      = v; }
  void setDefaultBoolean   (_CORBA_Boolean   v) { v_.boolean_    = v; }
  void setDefaultChar      (_CORBA_Char      v) { v_.char_       = v; }
#ifdef HAS_LongLong
  void setDefaultLongLong  (_CORBA_LongLong  v) { v_.longlong_   = v; }
  void setDefaultULongLong (_CORBA_ULongLong v) { v_.ulonglong_  = v; }
#endif
  void setDefaultWChar     (_CORBA_WChar     v) { v_.wchar_      = v; }
  void setDefaultEnumerator(Enumerator*      v) { v_.enumerator_ = v; }

private:
  IdlExpr*       value_;
  _CORBA_Boolean isDefault_;
  IdlType::Kind  labelKind_;
  union {
    _CORBA_Short        short_;
    _CORBA_Long         long_;
    _CORBA_UShort       ushort_;
    _CORBA_ULong        ulong_;
    _CORBA_Boolean      boolean_;
    _CORBA_Char         char_;
#ifdef HAS_LongLong
    _CORBA_LongLong     longlong_;
    _CORBA_ULongLong    ulonglong_;
#endif
    _CORBA_WChar        wchar_;
    Enumerator*         enumerator_;
  } v_;
};


// Union case
class UnionCase : public Decl {
public:
  UnionCase(const char* file, int line, _CORBA_Boolean mainFile,
	    IdlType* caseType, _CORBA_Boolean constrType,
	    Declarator* declarator);
  virtual ~UnionCase();

  const char* kindAsString() const { return "case"; }

  // Queries
  CaseLabel*     labels()     const { return labels_; }
  IdlType*       caseType()   const { return caseType_; }
  _CORBA_Boolean constrType() const { return constrType_; }
  Declarator*    declarator() const { return declarator_; }

  void accept(AstVisitor& visitor) { visitor.visitUnionCase(this); }

  void finishConstruction(CaseLabel* labels);

private:
  CaseLabel*     labels_;
  IdlType*       caseType_;
  _CORBA_Boolean delType_;
  _CORBA_Boolean constrType_;
  Declarator*    declarator_;
};


// Union
class Union : public Decl, public DeclRepoId {
public:
  Union(const char* file, int line, _CORBA_Boolean mainFile,
	const char* identifier);
  virtual ~Union();

  const char* kindAsString() const { return "union"; }

  // Queries
  IdlType*       switchType() const { return switchType_; }
  _CORBA_Boolean constrType() const { return constrType_; }
  UnionCase*     cases()      const { return cases_; }
  IdlType*       thisType()   const { return thisType_; }
  _CORBA_Boolean recursive()  const { return recursive_; }
  _CORBA_Boolean finished()   const { return finished_; }

  void accept(AstVisitor& visitor) { visitor.visitUnion(this); }

  void finishConstruction(IdlType* switchType, _CORBA_Boolean constrType,
			  UnionCase* cases);
  void setRecursive() { recursive_ = 1; }

private:
  IdlType*       switchType_;
  _CORBA_Boolean constrType_;
  UnionCase*     cases_;
  IdlType*       thisType_;
  _CORBA_Boolean recursive_;
  _CORBA_Boolean finished_;
};


// Enumerator
class Enum;

class Enumerator : public Decl, public DeclRepoId {
public:
  Enumerator(const char* file, int line, _CORBA_Boolean mainFile,
	     const char* identifier);
  virtual ~Enumerator();

  const char* kindAsString() const { return "enumerator"; }

  // Queries
  Enum*       container()  const { return container_; }

  void accept(AstVisitor& visitor) { visitor.visitEnumerator(this); }

  void finishConstruction(Enum* container);

private:
  const char* identifier_;
  Enum*       container_;
};


// Enum
class Enum : public Decl, public DeclRepoId {
public:
  Enum(const char* file, int line, _CORBA_Boolean mainFile,
       const char* identifier);
  virtual ~Enum();

  const char* kindAsString() const { return "enum"; }

  // Queries
  Enumerator* enumerators() const { return enumerators_; }
  IdlType*    thisType()    const { return thisType_; }

  void accept(AstVisitor& visitor) { visitor.visitEnum(this); }

  void finishConstruction(Enumerator* enumerators);

private:
  Enumerator* enumerators_;
  IdlType*    thisType_;
};



// Attribute
class Attribute : public Decl {
public:
  Attribute(const char* file, int line, _CORBA_Boolean mainFile,
	    _CORBA_Boolean readonly, IdlType* attrType,
	    Declarator* declarators);
  virtual ~Attribute();

  const char* kindAsString() const { return "attribute"; }

  // Queries
  _CORBA_Boolean readonly()    const { return readonly_; }
  IdlType*       attrType()    const { return attrType_; }
  Declarator*    declarators() const { return declarators_; }
				// All declarators must be simple

  void accept(AstVisitor& visitor) { visitor.visitAttribute(this); }

private:
  _CORBA_Boolean readonly_;
  IdlType*       attrType_;
  _CORBA_Boolean delType_;
  Declarator*    declarators_;
};


// Parameter
class Parameter : public Decl {
public:
  Parameter(const char* file, int line, _CORBA_Boolean mainFile,
	    int direction, IdlType* paramType, const char* identifier);
  virtual ~Parameter();

  const char* kindAsString() const { return "parameter"; }

  // Queries
  int         direction()  const { return direction_; }
				// 0: in, 1: out, 2: inout
  IdlType*    paramType()  const { return paramType_; }
  const char* identifier() const { return identifier_; }

  void accept(AstVisitor& visitor) { visitor.visitParameter(this); }

private:
  int            direction_;
  IdlType*       paramType_;
  _CORBA_Boolean delType_;
  char*          identifier_;
};


// List of exceptions
class RaisesSpec {
public:
  RaisesSpec(const ScopedName* sn, const char* file, int line);
  ~RaisesSpec();

  Exception*  exception() const { return exception_; }
  RaisesSpec* next()      const { return next_; }

  void append(RaisesSpec* rs) {
    if (rs) {
      last_->next_ = rs;
      last_ = rs;
    }
  }
private:
  Exception*  exception_;

protected:
  RaisesSpec* next_;
  RaisesSpec* last_;
};

// List of contexts
class ContextSpec {
public:
  ContextSpec(const char* c, const char* file, int line);
  ~ContextSpec();

  const char*  context() const { return context_; }
  ContextSpec* next()    const { return next_; }

  void append(ContextSpec* rs) {
    last_->next_ = rs;
    last_ = rs;
  }

private:
  char*  context_;

protected:
  ContextSpec* next_;
  ContextSpec* last_;
};


// Operation
class Operation : public Decl {
public:
  Operation(const char* file, int line, _CORBA_Boolean mainFile,
	    _CORBA_Boolean oneway, IdlType* return_type,
	    const char* identifier);
  virtual ~Operation();

  const char* kindAsString() const { return "operation"; }

  // Queries
  _CORBA_Boolean oneway()     const { return oneway_; }
  IdlType*       returnType() const { return returnType_; }
  const char*    identifier() const { return identifier_; }
  Parameter*     parameters() const { return parameters_; }
  RaisesSpec*    raises()     const { return raises_; }
  ContextSpec*   contexts()   const { return contexts_; }

  void accept(AstVisitor& visitor) { visitor.visitOperation(this); }

  void closeParens();
  void finishConstruction(Parameter* parameters, RaisesSpec* raises,
			  ContextSpec* contexts);

private:
  _CORBA_Boolean oneway_;
  IdlType*       returnType_;
  _CORBA_Boolean delType_;
  char*          identifier_;
  Parameter*     parameters_;
  RaisesSpec*    raises_;
  ContextSpec*   contexts_;
};


// Native
class Native : public Decl, public DeclRepoId {
public:
  Native(const char* file, int line, _CORBA_Boolean mainFile,
	 const char* identifier);
  virtual ~Native();

  const char* kindAsString() const { return "native"; }

  void accept(AstVisitor& visitor) { visitor.visitNative(this); }
};


// Things for valuetype

class StateMember : public Decl {
public:
  StateMember(const char* file, int line, _CORBA_Boolean mainFile,
	      int memberAccess, IdlType* memberType,
	      _CORBA_Boolean constrType, Declarator* declarators);
  virtual ~StateMember();

  const char* kindAsString() const { return "state member"; }

  // Queries
  int            memberAccess() const { return memberAccess_; }
				// 0: public, 1: private
  IdlType*       memberType()   const { return memberType_; }
  _CORBA_Boolean constrType()   const { return constrType_; }
  Declarator*    declarators()  const { return declarators_; }

  void accept(AstVisitor& visitor) { visitor.visitStateMember(this); }

private:
  int            memberAccess_;
  IdlType*       memberType_;
  _CORBA_Boolean delType_;
  _CORBA_Boolean constrType_;
  Declarator*    declarators_;
};

class Factory : public Decl {
public:
  Factory(const char* file, int line, _CORBA_Boolean mainFile,
	  const char* identifier);
  ~Factory();

  const char* kindAsString() const { return "initializer"; }

  // Queries
  const char* identifier() const { return identifier_; }
  Parameter*  parameters() const { return parameters_; }

  void accept(AstVisitor& visitor) { visitor.visitFactory(this); }

  void closeParens();
  void finishConstruction(Parameter* parameters);

private:
  char*       identifier_;
  Parameter*  parameters_;
};


// Base class for all the multifarious value types
class ValueBase : public Decl, public DeclRepoId {
public:
  ValueBase(Decl::Kind k, const char* file, int line, _CORBA_Boolean mainFile,
	    const char* identifier);
  virtual ~ValueBase();
};


// Forward declared value
class ValueForward : public ValueBase {
public:
  ValueForward(const char* file, int line, _CORBA_Boolean mainFile,
	       _CORBA_Boolean abstract, const char* identifier);
  virtual ~ValueForward();

  const char* kindAsString() const { return "forward value"; }

  // Queries
  _CORBA_Boolean abstract()   const { return abstract_; }
  ValueBase*     definition() const;
  _CORBA_Boolean isFirst()    const { return !firstForward_; }
  IdlType*       thisType()   const { return thisType_; }

  void accept(AstVisitor& visitor) { visitor.visitValueForward(this); }

  void setDefinition(ValueBase* defn);

private:
  _CORBA_Boolean abstract_;
  ValueBase*     definition_;
  ValueForward*  firstForward_;
  IdlType*       thisType_;
};


class ValueBox : public ValueBase {
public:
  ValueBox(const char* file, int line, _CORBA_Boolean mainFile,
	   const char* identifier, IdlType* boxedType,
	   _CORBA_Boolean constrType);
  virtual ~ValueBox();

  const char* kindAsString() const { return "value box"; }

  // Queries
  IdlType*       boxedType()  const { return boxedType_; }
  _CORBA_Boolean constrType() const { return constrType_; }
  IdlType*       thisType()   const { return thisType_; }

  void accept(AstVisitor& visitor) { visitor.visitValueBox(this); }

private:
  IdlType*       boxedType_;
  _CORBA_Boolean constrType_;
  IdlType*       thisType_;
};


class ValueInheritSpec {
public:
  ValueInheritSpec(ScopedName* sn, const char* file, int line);

  virtual ~ValueInheritSpec() { if (next_) delete next_; }

  ValueBase*        value()       const { return value_; }
  Decl*             decl()        const { return decl_; }
  const Scope*      scope()       const { return scope_; }
  ValueInheritSpec* next()        const { return next_; }
  _CORBA_Boolean    truncatable() const { return truncatable_; }

  void append(ValueInheritSpec* is, const char* file, int line);
  void setTruncatable() { truncatable_ = 1; }

private:
  ValueBase*   value_;
  Decl*        decl_;
  const Scope* scope_;

protected:
  ValueInheritSpec* next_;
  _CORBA_Boolean    truncatable_;
};


class ValueInheritSupportSpec {
public:
  ValueInheritSupportSpec(ValueInheritSpec* inherits,
			  InheritSpec*      supports) :
    inherits_(inherits), supports_(supports) {}

  ~ValueInheritSupportSpec() {}

  ValueInheritSpec* inherits() const { return inherits_; }
  InheritSpec*      supports() const { return supports_; }

private:
  ValueInheritSpec* inherits_;
  InheritSpec*      supports_;
};


class ValueAbs : public ValueBase {
public:
  ValueAbs(const char* file, int line, _CORBA_Boolean mainFile,
	   const char* identifier, ValueInheritSpec* inherits,
	   InheritSpec* supports);
  virtual ~ValueAbs();

  const char* kindAsString() const { return "abstract valuetype"; }

  // Queries
  ValueInheritSpec* inherits() const { return inherits_; }
  InheritSpec*      supports() const { return supports_; }
  Decl*             contents() const { return contents_; }
  Scope*            scope()    const { return scope_;    }
  IdlType*          thisType() const { return thisType_; }

  void accept(AstVisitor& visitor) { visitor.visitValueAbs(this); }

  void finishConstruction(Decl* contents);

private:
  ValueInheritSpec* inherits_;
  InheritSpec*      supports_;
  Decl*             contents_;
  Scope*            scope_;
  IdlType*          thisType_;
};


class Value : public ValueBase {
public:
  Value(const char* file, int line, _CORBA_Boolean mainFile,
	_CORBA_Boolean custom, const char* identifier,
	ValueInheritSpec* inherits, InheritSpec* supports);
  virtual ~Value();

  const char* kindAsString() const { return "valuetype"; }

  // Queries
  _CORBA_Boolean    custom()   const { return custom_; }
  ValueInheritSpec* inherits() const { return inherits_; }
  InheritSpec*      supports() const { return supports_; }
  Decl*             contents() const { return contents_; }
  Scope*            scope()    const { return scope_;    }
  IdlType*          thisType() const { return thisType_; }

  void accept(AstVisitor& visitor) { visitor.visitValue(this); }

  void finishConstruction(Decl* contents);

private:
  _CORBA_Boolean    custom_;
  ValueInheritSpec* inherits_;
  InheritSpec*      supports_;
  Decl*             contents_;
  Scope*            scope_;
  IdlType*          thisType_;
};


#endif // _idlast_h_
