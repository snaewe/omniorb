# -*- python -*-
#                           Package   : omniidl
# id.py                     Created on: 2000/4/6
#			    Author    : David Scott (djs)
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
#   Environment handling, identifier scoping, naming

from omniidl import idlvisitor, idlutil

import id, config, util

import string, re

# CORBA2.3 P1-166 1.43 C++ Keywords
#
reservedWords = [
    "and", "and_eq", "asm", "auto", "bitand", "bitor", "bool", "break",
    "case", "catch", "char", "class", "compl", "const", "const_cast",
    "continue", "default", "delete", "do", "double", "dynamic_cast",
    "else", "enum", "explicit", "export", "extern", "false", "float",
    "for", "friend", "goto", "if", "inline", "int", "long", "mutable",
    "namespace", "new", "not", "not_eq", "operator", "or", "or_eq",
    "private", "protected", "public", "register", "reinterpret_cast",
    "return", "short", "signed", "sizeof", "static", "static_cast",
    "struct", "switch", "template", "this", "throw", "true", "try",
    "typedef", "typeid", "typename", "union", "unsigned", "using",
    "virtual", "void", "volatile", "wchar_t", "while", "xor", "xor_eq" ]

rwdict = {}
for w in reservedWords:
    rwdict[w] = None

def mapID(identifier):
    """id.mapID(identifier string): string
       Returns the identifier with an escape if it is a C++ keyword"""
    if rwdict.has_key(identifier):
        return config.state['Reserved Prefix'] + identifier
    return identifier

class Name:
    """Encapsulates an IDL identifier and facilitates its mapping into
       C++"""
    def __init__(self, scopedName):
        """Create a Name object from a fully scoped IDL name"""
        # stores the IDL name with prefix and suffix separately
        self.__scopedName = scopedName
        self.__prefix = ""
        self.__suffix = ""


    def __cmp__(self, other):
        if not isinstance(other, Name): return 1
        return self.fullyQualify() == other.fullyQualify()


    def simple(self, cxx = 1):
        """simple(id.Name, cxx boolean): string
           Returns the 'simple' part of the name with the scope removed"""
        sn = self.__scopedName
        if cxx: sn = self.__map_cxx()
        sn = self.__apply_presuffix(sn)
        sname = sn[-1]
        
        return sname

    def scope(self):
        """scope(id.Name): string list
           Returns the scope of the name"""
        return self.__scopedName[:-1]

    def __apply_presuffix(self, scopedName):
        # internal function to apply the prefix and suffix to the IDL name
        scope = scopedName[:-1]
        name = scopedName[-1]
        return scope + [ self.__prefix + name + self.__suffix ]

    def prefix(self, prefix):
        """prefix(id.Name): id.Name
           Add a prefix to the name
               ie ::A::B::C -> ::A::B::_objref_C"""
        new = Name(self.__scopedName)
        new.__prefix = prefix + self.__prefix
        new.__suffix = self.__suffix
        return new

    def suffix(self, suffix):
        """suffix(id.Name): id.Name
           Add a suffix to the name
               ie ::A::B::C -> ::A::B::C_ptr"""
        new = Name(self.__scopedName)
        new.__prefix = self.__prefix
        new.__suffix = self.__suffix + suffix
        return new

    def __map_cxx(self):
        # __map_cxx(id.Name): string list
        # Maps an IDL name into a cxx one with proper escaping

        return map(mapID, self.__scopedName)
    
    def fullyQualify(self, cxx = 1):
        """fullyQualify(id.Name, cxx boolean, include_root boolean): string
           Returns a fully qualified C++ name (initial root :: optional)"""
        if cxx: sn = self.__map_cxx()
        else:   sn = self.__scopedName

        sn = self.__apply_presuffix(sn)

        return string.join(sn, "::")

    def fullName(self):
        """fullName(id.Name): string list
           Returns the fully scopedname (with prefixes and suffixes as
           appropriate) as a string component list"""
        return self.__apply_presuffix(self.__scopedName)

    def relName(self, environment):
        """relName(id.Name, id.Environment): string list
           Returns a list of the name components essential to making
           the name unambiguous"""
        rscope = idlutil.relativeScope(environment._Environment__scope,
                                       self.__scopedName)
        if rscope and rscope[0] is None:
            return None
        else:
            return rscope

    def unambiguous(self, environment, cxx = 1):
        """unambiguous(id.Name, id.Environment option, cxx boolean): string
           Returns the unambiguous C++ name for a particular environment"""

        if environment == None:
            return self.fullyQualify(cxx)
        
        relName = self.relName(environment)
        if relName == None:
            # must fully qualify from the root
            return "::" + self.fullyQualify(cxx = cxx)

        if cxx: relName = map(mapID, relName)
        relName = self.__apply_presuffix(relName)
        return string.join(relName, "::")


    def guard(self):
        """guard(id.Name): string
           Produces a flattened guard string suitable for an #ifdef"""
        def escapeChars(text):
            # escape all escapes
            text = re.sub(r"_", "__", text)
            return re.sub(r"\W", "_", text)
        scopedName = map(escapeChars, self.__scopedName[:])

        return string.join(self.__apply_presuffix(scopedName), "_m")

    # comment copied from src/tool/omniidl2/omniORB2_be/o2be_interface.cc:

    # MSVC {4.2,5.0} cannot deal with a call to a virtual member
    # of a base class using the member function's fully/partially
    # scoped name. Have to use the alias for the base class in the
    # global scope to refer to the virtual member function instead.
    #
    # We scan all the base interfaces to see if any of them has to
    # be referred to by their fully/partially qualified names. If
    # that is necessary, we generate a typedef to define an alias for
    # this base interface. This alias is used in the stub generated below
    #
    # FIXME: is this a solution to the OMNI_BASE_CTOR stuff below?
    #
    def flatName(self):
        return string.join(self.fullName(), "_")
    
    def needFlatName(self, environment):
        # does the name have scope :: qualifiers
        relName = self.relName(environment)
        #if not relName: return 0
            
        return len(relName) > 1


    def hash(self):
        """hash(id.Name): string
           Returns a hashable unique key for this object"""
        return string.join(self.__scopedName, "/")

    def __str__(self):
        return string.join(self.__scopedName, "/")

class Environment:
    """An environment encapsulates the naming environment at a point
       in the AST. The environment consists of both a scope and a set
       of already defined names."""
    def __init__(self):
        self.__scope = []
        self.__names = {}

    def copy(self):
        """copy(id.Environment): id.Environment
           Copy constructor"""
        new = Environment()
        new.__scope = self.__scope[:]
        new.__names = self.__names.copy()
        return new

    def enter(self, scope_name, interface = None):
        """enter(id.Environment, scope_name string, interface option)
           Creates and returns a new environment for the new scope"""
        new = Environment()
        new.__scope = self.__scope + [ scope_name ]
        new.__names = self.__names.copy()
        # if the scope is an interface, special scoping rules apply
        # (because of interface inheritance)
        if interface:
            assert isinstance(interface, idlast.Interface)
            new.__interface = interface
            
        return new

    def leave(self):
        """leave(id.Environment): id.Environment
           Returns an environment representing the outer scope. Defined
           names are kept"""
        new = Environment()
        new.__scope = self.__scope[:-1]
        new.__names = self.__names.copy()
        return new

    def addName(self, scopedName, allow_already_exists = 0):
        """addName(id.Environment, scopedName, allow_already_exists boolean)
           Adds a new scopedName to the current environment. Will throw
           a fatal exception if it already exists (unless the
           allow_already_exists flag is set to 1"""
        name = Name(scopedName)
        hash = name.hash()
        if self.__names.has_key(hash) and not allow_already_exists:
            util.fatalError("Attempt to redefine existing name: " + str(name))
        self.__names[hash] = name

    def lookup(self, relName):
        """lookup(id.Environment, relName): id.Name option
           Searches for a relative scoped name in the same manner as C++,
           returning the Name object it refers to, or None if there is
           nothing to find"""
        scope = self.__scope[:]
        while 1:
            name = Name(scope + relName)
            if self.__names.has_key(name.hash()):
                return self.__names[name.hash()]
            if scope == []:
                return None
            del scope[-1]

    def scope(self):
        """scope(id.Environment): scope string list
           Returns the current scope"""
        return self.__scope

    def __str__(self):
        text = "Environment {\n"
        text = text + "\tscope = " + repr(self.__scope) + "\n"
        text = text + "\tnames = "
        names = []
        for hashkey in self.__names.keys():
            names.append(str(self.__names[hashkey]))
        text = text + repr(names) + "\n"
        text = text + "}"
        return text


# Hash of AST nodes -> environments
# (facilitates multiple passes of the AST, by precaching naming info)
id._environments = None

# List of predefined names
id._predefined_names =  [ ["CORBA", "Object"] ]

def predefine_decl(decl):
    id._predefined_names.append(decl.scopedName())


# Could annotate the tree instead?
def lookup(node):
    """lookup : AST node -> Environment"""
    try:
        return id._environments[node]
    except KeyError:
        util.fatalError("Failed to find environment corresponding to node" +\
                        " (= " + repr(node.scopedName()) + ")")

class WalkTree(idlvisitor.AstVisitor):
    """Walks over the AST once building the hash of
       AST nodes -> Environments"""
    def __enter(self, name):
        self.__env = self.__env.enter(name)

    def __leave(self):
        self.__env = self.__env.leave()

    def __add(self, name, allow_already_exists = 0):
        new = self.__env.copy()
        scopedName = self.__env.scope() + [ name ]
        new.addName(scopedName, allow_already_exists)
        self.__env = new
        return

    def __cache(self, node):
        if id._environments.has_key(node):
            util.fatalError("Unexpected inconsistency: encountered node " +\
                             "more than once while walking the tree")
        id._environments[node] = self.__env
            

    def __init__(self):
        id._environments = {}
        # Names which are predefined
        self.__env = Environment()

        for name in id._predefined_names:
            self.__env.addName(name)

        
    # Tree walking functions
    def visitAST(self, node):
        for n in node.declarations():
            n.accept(self)

        self.__cache(node)

    def visitDeclRepoId(self, node):
        name = node.identifier()
        self.__add(name)

        self.__cache(node)
            
    def visitModule(self, node):
        name = node.identifier()
        # already exists => reopening module
        self.__add(name, allow_already_exists = 1)
        
        self.__enter(name)
        for n in node.definitions():
            n.accept(self)
        self.__leave()

        self.__cache(node)
                

    def visitInterface(self, node):
        name = node.identifier()
        self.__add(name, allow_already_exists = 1)
        
        self.__enter(name)
        for n in node.declarations():
            n.accept(self)
        self.__leave()

        self.__cache(node)
        
    def visitForward(self, node):
        name = node.identifier()
        self.__add(name, allow_already_exists = 1)

        self.__cache(node)
        
    def visitConst(self, node):
        name = Name(node.scopedName()).simple()
        self.__add(name)

        self.__cache(node)
        
    def visitDeclarator(self, node):
        name = Name(node.scopedName()).simple()
        self.__add(name)

        self.__cache(node)
        
    def visitTypedef(self, node):
        if node.constrType():
            node.aliasType().decl().accept(self)

        for d in node.declarators():
            d.accept(self)

        self.__cache(node)
                
    def visitMember(self, node):
        if node.constrType():
            node.memberType().decl().accept(self)
            
        for d in node.declarators():
            d.accept(self)

        self.__cache(node)
        
    def visitStruct(self, node):
        name = node.identifier()
        self.__add(name)

        self.__enter(name)
        for n in node.members():
            n.accept(self)
        self.__leave()

        self.__cache(node)
        
    def visitException(self, node):
        name = node.identifier()
        self.__add(name)

        self.__enter(name)
        for n in node.members():
            n.accept(self)
        self.__leave()

        self.__cache(node)
        
    def visitUnion(self, node):
        name = node.identifier()
        self.__add(name)

        self.__enter(name)
        # deal with constructed switch type
        if node.constrType():
            node.switchType().decl().accept(self)

        # deal with constructed member types
        for n in node.cases():
            if n.constrType():
                n.caseType().decl().accept(self)

        self.__leave()

        self.__cache(node)

    def visitCaseLabel(self, node):
        pass
    def visitUnionCase(self, node):
        pass
    def visitEnumerator(self, node):
        pass
    def visitEnum(self, node):
        name = node.identifier()
        self.__add(name)

        for value in node.enumerators():
            self.__add(value.identifier())
        
        self.__cache(node)

    def visitValue(self, node):
        name = node.identifier()
        self.__add(name, allow_already_exists = 1)
        
        self.__enter(name)
        for n in node.declarations():
            n.accept(self)
        self.__leave()

        self.__cache(node)

    def visitValueAbs(self, node):
        name = node.identifier()
        self.__add(name, allow_already_exists = 1)
        
        self.__enter(name)
        for n in node.declarations():
            n.accept(self)
        self.__leave()

        self.__cache(node)
