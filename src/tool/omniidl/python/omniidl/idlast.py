# -*- python -*-
#                           Package   : omniidl
# idlast.py                 Created on: 1999/10/27
#			    Author    : Duncan Grisby (dpg1)
#
#    Copyright (C) 1999 AT&T Laboratories Cambridge
#
#  This file is part of omniidl.
#
#  omniidl is free software; you can redistribute it and/or modify it
#  under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
#  02111-1307, USA.
#
# Description:
#   
#   Python definitions for abstract syntax tree classes

# $Id$
# $Log$
# Revision 1.6  1999/11/02 10:01:46  dpg1
# Minor fixes.
#
# Revision 1.5  1999/11/01 20:19:55  dpg1
# Support for union switch types declared inside the switch statement.
#
# Revision 1.4  1999/11/01 16:39:01  dpg1
# Small fixes and cosmetic changes.
#
# Revision 1.3  1999/11/01 10:05:01  dpg1
# New file attribute to AST.
#
# Revision 1.2  1999/10/29 18:19:39  dpg1
# Clean up
#
# Revision 1.1  1999/10/29 15:47:08  dpg1
# First revision.
#

import idlutil
import idlvisitor

class AST:
    def __init__(self, file, declarations):
        self.__file         = file
        self.__declarations = declarations
        #print "AST init:", declarations

    def file(self):            return self.__file
    def declarations(self):    return self.__declarations
    def accept(self, visitor): visitor.visitAST(self)


# Base declaration
class Decl:
    def __init__(self, file, line, mainFile, pragmas):
        self.__file     = file
        self.__line     = line
        self.__mainFile = mainFile
        self.__pragmas  = pragmas

    # File name of declaration
    def file(self):     return self.__file

    # Line number of declaration
    def line(self):     return self.__line

    # True if this declaration came from the main IDL file
    def mainFile(self): return self.__mainFile

    # List of unknown pragmas, as strings, attached to this decl
    def pragmas(self):  return self.__pragmas


# Mixin class for Decls which have a Repository Id
class DeclRepoId :
    def __init__(self, identifier, scopedName, repoId):
        self.__identifier = identifier
        self.__scopedName = scopedName
        self.__repoId     = repoId

    # Name as a string
    def identifier(self): return self.__identifier

    # Fully scoped name as a list of strings
    def scopedName(self): return self.__scopedName

    # Repository Id string
    def repoId(self):     return self.__repoId


# Module
class Module (Decl, DeclRepoId):
    def __init__(self, file, line, mainFile, pragmas,
                 identifier, scopedName, repoId,
                 definitions):

        Decl.__init__(self, file, line, mainFile, pragmas)
        DeclRepoId.__init__(self, identifier, scopedName, repoId)

        self.__definitions  = definitions
        self._continuations = []
        #print line, "Module init:", identifier, definitions

    def accept(self, visitor): visitor.visitModule(self)

    # List containing contents of module:
    def definitions(self):   return self.__definitions

    # List containing continuations of this module
    def continuations(self): return self._continuations


# Interface
class Interface (Decl, DeclRepoId):
    def __init__(self, file, line, mainFile, pragmas,
                 identifier, scopedName, repoId,
                 abstract, inherits):

        Decl.__init__(self, file, line, mainFile, pragmas)
        DeclRepoId.__init__(self, identifier, scopedName, repoId)

        self.__abstract     = abstract
        self.__inherits     = inherits
        self.__contents     = []
        self.__declarations = []
        self.__callables    = []
        #print line, "Interface init:", identifier, inherits

    def _setContents(self, contents):
        self.__contents     = contents
        self.__declarations = filter(lambda c: not (isinstance(c, Attribute) or
                                                    isinstance(c, Operation)),
                                     contents)
        self.__callables    = filter(lambda c: (isinstance(c, Attribute) or
                                                isinstance(c, Operation)),
                                     contents)

    def accept(self, visitor): visitor.visitInterface(self)

    # Abstract?
    def abstract(self):     return self.__abstract

    # List of Interface objects from which this one derives
    def inherits(self):     return self.__inherits

    # List of complete contents
    def contents(self):     return self.__contents

    # List of declarations of types, constants, exceptions
    def declarations(self): return self.__declarations

    # List of operations and attributes
    def callables(self):    return self.__callables


# Forward-declared interface.
class Forward (Decl, DeclRepoId):
    def __init__(self, file, line, mainFile, pragmas,
                 identifier, scopedName, repoId,
                 abstract):

        Decl.__init__(self, file, line, mainFile, pragmas)
        DeclRepoId.__init__(self, identifier, scopedName, repoId)

        self.__abstract   = abstract
        #print line, "Forward init:", identifier

    def accept(self, visitor): visitor.visitForward(self)

    # Abstract?
    def abstract(self):     return self.__abstract


# Constant
class Const (Decl, DeclRepoId):
    def __init__(self, file, line, mainFile, pragmas,
                 identifier, scopedName, repoId,
                 constType, constKind, value):

        Decl.__init__(self, file, line, mainFile, pragmas)
        DeclRepoId.__init__(self, identifier, scopedName, repoId)

        self.__constType = constType
        self.__constKind = constKind
        self.__value     = value
        #print line, "Const init:", constType, identifier, value

    def accept(self, visitor): visitor.visitConst(self)

    # IdlType object
    def constType(self):  return self.__constType

    # Kind of const with aliases stripped
    def constKind(self):  return self.__constKind

    # Value is either a number or an Enumerator object
    def value(self):      return self.__value


# Declarator used in typedefs, struct and members, etc.

class Declarator (Decl, DeclRepoId):
    def __init__(self, file, line, mainFile, pragmas,
                 identifier, scopedName, repoId,
                 sizes):

        Decl.__init__(self, file, line, mainFile, pragmas)
        DeclRepoId.__init__(self, identifier, scopedName, repoId)

        self.__sizes = sizes
        self.__alias = None

    def _setAlias(self, alias): self.__alias = alias

    def accept(self, visitor): visitor.visitDeclarator(self)

    # List of array sizes, or None if this is a simple declarator
    def sizes(self): return self.__sizes

    # Typedef object if this is a typedef declarator, None if not
    def alias(self): return self.__alias


class Typedef (Decl):
    def __init__(self, file, line, mainFile, pragmas,
                 aliasType, constrType, declarators):

        Decl.__init__(self, file, line, mainFile, pragmas)

        self.__aliasType   = aliasType
        self.__constrType  = constrType
        self.__declarators = declarators
        #print line, "Typedef init:", aliasType

    def accept(self, visitor): visitor.visitTypedef(self)

    # Type this is an alias to
    def aliasType(self):   return self.__aliasType

    # True if the alias type was constructed within this typedef
    def constrType(self):  return self.__constrType

    # List of Declarators
    def declarators(self): return self.__declarators


# Struct / exception member
class Member (Decl):
    def __init__(self, file, line, mainFile, pragmas,
                 memberType, constrType, declarators):

        Decl.__init__(self, file, line, mainFile, pragmas)

        self.__memberType  = memberType
        self.__constrType  = constrType
        self.__declarators = declarators
        #print line, "Member init:", memberType

    def accept(self, visitor): visitor.visitMember(self)

    # Type of this member
    def memberType(self):   return self.__memberType

    # True if the member type was constructed within this member
    def constrType(self):  return self.__constrType

    # List of Declarators
    def declarators(self): return self.__declarators


class Struct (Decl, DeclRepoId):
    def __init__(self, file, line, mainFile, pragmas,
                 identifier, scopedName, repoId,
                 recursive):

        Decl.__init__(self, file, line, mainFile, pragmas)
        DeclRepoId.__init__(self, identifier, scopedName, repoId)

        self.__recursive = recursive
        #print line, "Struct init:", identifier

    def _setMembers(self, members):
        self.__members = members

    def accept(self, visitor): visitor.visitStruct(self)

    # Members of the struct
    def members(self):    return self.__members

    # True if the struct is recursive
    def recursive(self):  return self.__recursive


class Exception (Decl, DeclRepoId):
    def __init__(self, file, line, mainFile, pragmas,
                 identifier, scopedName, repoId,
                 members):

        Decl.__init__(self, file, line, mainFile, pragmas)
        DeclRepoId.__init__(self, identifier, scopedName, repoId)

        self.__members    = members
        #print line, "Exception init:", identifier, members

    def accept(self, visitor): visitor.visitException(self)

    # Members of the exception
    def members(self):    return self.__members


class CaseLabel (Decl):
    def __init__(self, file, line, mainFile, pragmas,
                 default, value, labelKind):

        Decl.__init__(self, file, line, mainFile, pragmas)

        self.__default   = default
        self.__value     = value
        self.__labelKind = labelKind

    def accept(self, visitor): visitor.visitCaseLabel(self)

    # True if this is the default label
    def default(self): return self.__default

    # Label value. If default, this value is a value used by none of
    # the other labels.
    def value(self): return self.__value

    # TypeCode kind for label
    def labelKind(self): return self.__labelKind


class UnionCase (Decl):
    def __init__(self, file, line, mainFile, pragmas,
                 labels, caseType, constrType, declarator):

        Decl.__init__(self, file, line, mainFile, pragmas)

        self.__labels     = labels
        self.__caseType   = caseType
        self.__constrType = constrType
        self.__declarator = declarator
        #print line, "UnionCase init"

    def accept(self, visitor): visitor.visitUnionCase(self)

    def labels(self):     return self.__labels
    def caseType(self):   return self.__caseType
    def constrType(self): return self.__constrType
    def declarator(self): return self.__declarator


class Union (Decl, DeclRepoId):
    def __init__(self, file, line, mainFile, pragmas,
                 identifier, scopedName, repoId,
                 switchType, constrType, recursive):

        Decl.__init__(self, file, line, mainFile, pragmas)
        DeclRepoId.__init__(self, identifier, scopedName, repoId)

        self.__switchType = switchType
        self.__constrType = constrType
        self.__recursive  = recursive
        #print line, "Union init:", identifier

    def _setCases(self, cases):
        self.__cases = cases

    def accept(self, visitor): visitor.visitUnion(self)

    def switchType(self): return self.__switchType

    # True if the switch type is declared inside the switch!
    def constrType(self): return self.__constrType
    def cases(self):      return self.__cases

    # True if the union is recursive
    def recursive(self):  return self.__recursive


class Enumerator (Decl, DeclRepoId):
    def __init__(self, file, line, mainFile, pragmas,
                 identifier, scopedName, repoId):

        Decl.__init__(self, file, line, mainFile, pragmas)
        DeclRepoId.__init__(self, identifier, scopedName, repoId)
        #print line, "Enumerator:", identifier

    def accept(self, visitor): visitor.visitEnumerator(self)


class Enum (Decl, DeclRepoId):
    def __init__(self, file, line, mainFile, pragmas,
                 identifier, scopedName, repoId,
                 enumerators):

        Decl.__init__(self, file, line, mainFile, pragmas)
        DeclRepoId.__init__(self, identifier, scopedName, repoId)

        self.__enumerators = enumerators
        #print line, "Enum: ", identifier

    def accept(self, visitor): visitor.visitEnum(self)

    def enumerators(self): return self.__enumerators


class Attribute (Decl):
    def __init__(self, file, line, mainFile, pragmas,
                 readonly, attrType, identifiers):

        Decl.__init__(self, file, line, mainFile, pragmas)

        self.__readonly = readonly
        self.__attrType = attrType
        self.__identifiers = identifiers
        #print line, "Attribute init:", readonly, identifiers

    def accept(self, visitor): visitor.visitAttribute(self)

    def readonly(self):    return self.__readonly
    def attrType(self):    return self.__attrType
    def identifiers(self): return self.__identifiers


class Parameter (Decl):
    def __init__(self, file, line, mainFile, pragmas,
                 direction, paramType, identifier):

        Decl.__init__(self, file, line, mainFile, pragmas)

        self.__direction  = direction
        self.__is_in      = (direction == 0 or direction == 2)
        self.__is_out     = (direction == 1 or direction == 2)
        self.__paramType  = paramType
        self.__identifier = identifier
        #print line, "Parameter init:", identifier

    def accept(self, visitor): visitor.visitParameter(self)

    def direction(self):  return self.__direction
    def is_in(self):      return self.__is_in
    def is_out(self):     return self.__is_out
    def paramType(self):  return self.__paramType
    def identifier(self): return self.__identifier


class Operation (Decl):
    def __init__(self, file, line, mainFile, pragmas,
                 oneway, returnType, identifier,
                 parameters, raises, contexts):

        Decl.__init__(self, file, line, mainFile, pragmas)

        self.__oneway     = oneway
        self.__returnType = returnType
        self.__identifier = identifier
        self.__parameters = parameters
        self.__raises     = raises
        self.__contexts   = contexts
        #print line, "Operation init:", identifier, raises, contexts

    def accept(self, visitor): visitor.visitOperation(self)

    def oneway(self):     return self.__oneway
    def returnType(self): return self.__returnType
    def identifier(self): return self.__identifier
    def parameters(self): return self.__parameters
    def raises(self):     return self.__raises
    def contexts(self):   return self.__contexts


# Map of Decl objects, indexed by stringified scoped name, and
# functions to access it

declMap = {}

def registerDecl(scopedName, decl):
    sname = idlutil.slashName(scopedName)
    if declMap.has_key(sname):
#        print "registerDecl asked to re-register:", sname

        if isinstance(decl, Interface) and \
           isinstance(declMap[sname], Forward):
#            print "It's OK, it's resolving a forward interface."
            declMap[sname]._definition = decl
            declMap[sname] = decl

        elif isinstance(decl, Forward) and \
             isinstance(declMap[sname], Forward):
#            print "It's OK, it's a repeat forward declaration."
            pass

        elif isinstance(decl, Module) and \
             isinstance(declMap[sname], Module):
#            print "It's OK, it's a continued module."
            declMap[sname]._continuations.append(decl)

        return
    declMap[sname] = decl

def findDecl(scopedName):
    sname = idlutil.slashName(scopedName)
    if not declMap.has_key(sname):
        print "WARNING! findDecl() couldn't find:", sname
        return scopedName
    return declMap[sname]

# Declarations of non-basic `built-in' types

CORBAObject = Interface("<built in>", 0, 0, [],
                        "Object", ["CORBA", "Object"],
                        "IDL:omg.org/CORBA/Object:1.0",
                        0, [])
registerDecl(["CORBA", "Object"], CORBAObject)

CORBAModule = Module("<built in>", 0, 0, [], "CORBA", ["CORBA"],
                     "IDL:omg.org/CORBA:1.0", [CORBAObject])
registerDecl(["CORBA"], CORBAModule)
