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

import config, util

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
        self._scopedName = scopedName
        self._prefix = ""
        self._suffix = ""


    def __cmp__(self, other):
        if not isinstance(other, Name): return 1
        return cmp(self.fullyQualify(), other.fullyQualify())


    def simple(self, cxx = 1):
        """simple(id.Name, cxx boolean): string
           Returns the 'simple' part of the name with the scope removed"""
        sn = self._scopedName
        if cxx: sn = self._map_cxx()
        sn = self._apply_presuffix(sn)
        sname = sn[-1]
        
        return sname

    def scope(self):
        """scope(id.Name): string list
           Returns the scope of the name"""
        return self._scopedName[:-1]

    def _apply_presuffix(self, scopedName):
        # internal function to apply the prefix and suffix to the IDL name
        scope = scopedName[:-1]
        name = scopedName[-1]
        return scope + [ self._prefix + name + self._suffix ]

    def prefix(self, prefix):
        """prefix(id.Name): id.Name
           Add a prefix to the name
               ie ::A::B::C -> ::A::B::_objref_C"""
        new = Name(self._scopedName)
        new._prefix = prefix + self._prefix
        new._suffix = self._suffix
        return new

    def suffix(self, suffix):
        """suffix(id.Name): id.Name
           Add a suffix to the name
               ie ::A::B::C -> ::A::B::C_ptr"""
        new = Name(self._scopedName)
        new._prefix = self._prefix
        new._suffix = self._suffix + suffix
        return new

    def _map_cxx(self):
        # _map_cxx(id.Name): string list
        # Maps an IDL name into a cxx one with proper escaping

        return map(mapID, self._scopedName)
    
    def fullyQualify(self, cxx = 1):
        """fullyQualify(id.Name, cxx boolean, include_root boolean): string
           Returns a fully qualified C++ name (initial root :: optional)"""
        if cxx: sn = self._map_cxx()
        else:   sn = self._scopedName

        sn = self._apply_presuffix(sn)

        return string.join(sn, "::")

    def fullName(self):
        """fullName(id.Name): string list
           Returns the fully scopedname (with prefixes and suffixes as
           appropriate) as a string component list"""
        return self._apply_presuffix(self._scopedName)

    def relName(self, environment):
        """relName(id.Name, id.Environment): string list
           Returns a list of the name components essential to making
           the name unambiguous"""
        rscope = idlutil.relativeScope(environment.scope(), self._scopedName)
        if rscope and rscope[0] is None:
            return None
        else:
            return rscope

    def unambiguous(self, environment, cxx = 1):
        """unambiguous(id.Name, id.Environment option, cxx boolean): string
           Returns the unambiguous C++ name for a particular environment"""

        if environment is None:
            return self.fullyQualify(cxx)
        
        relName = self.relName(environment)
        if relName == None:
            # must fully qualify from the root
            return "::" + self.fullyQualify(cxx = cxx)

        if cxx: relName = map(mapID, relName)
        relName = self._apply_presuffix(relName)
        return string.join(relName, "::")


    def guard(self):
        """guard(id.Name): string
           Produces a flattened guard string suitable for an #ifdef"""
        def escapeChars(text):
            # escape all escapes
            text = re.sub(r"_", "__", text)
            return re.sub(r"\W", "_", text)
        scopedName = map(escapeChars, self._scopedName[:])

        return string.join(self._apply_presuffix(scopedName), "_m")

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
        return string.join(self._scopedName, "/")

    def __str__(self):
        return string.join(self._scopedName, "/")

class Environment:
    """An environment encapsulates the naming environment at a point
       in the AST. The environment consists of both a scope and a set
       of already defined names."""
    def __init__(self, scope=None, parent=None):
        if scope is None:
            self._scope = []
        else:
            self._scope = scope

        self._parent   = parent
        self._children = {}

    def copy(self):
        """copy(id.Environment): id.Environment
           Copy constructor"""
        return Environment(self._scope[:])

    def enter(self, scope_name, interface = None):
        """enter(id.Environment, scope_name string, interface option)
           Creates and returns a new environment for the new scope"""

        try:
            new = self._children[scope_name]
        except KeyError:
            new = Environment(self._scope + [scope_name], self)
            self._children[scope_name]= new

        return new

    def leave(self):
        """leave(id.Environment): id.Environment
           Returns an environment representing the outer scope. Defined
           names are kept"""
        return self._parent

    def scope(self):
        """scope(id.Environment): scope string list
           Returns the current scope"""
        return self._scope

    def __str__(self):
        text = "Environment(%s)" % repr(self._scope)


# Hash of AST nodes -> environments
# (facilitates multiple passes of the AST, by precaching naming info)
_environments = None

# List of predefined names
_predefined_names =  [ ["CORBA", "Object"] ]

def predefine_decl(decl):
    _predefined_names.append(decl.scopedName())


# Could annotate the tree instead?
def lookup(node):
    """lookup : AST node -> Environment"""
    try:
        return _environments[node]
    except KeyError:
        try:
            nname = node.scopedName()
        except:
            nname = repr(node)

        util.fatalError("Failed to find environment corresponding to node (%s)"
                        % nname)

def addNode(node, env):
    _environments[node] = env


class WalkTree(idlvisitor.AstVisitor):
    """Walks over the AST once building the hash of
       AST nodes -> Environments"""
    def _enter(self, name):
        self._env = self._env.enter(name)

    def _leave(self):
        self._env = self._env.leave()

    def _cache(self, node):
        if _environments.has_key(node):
            util.fatalError("Unexpected inconsistency: encountered node " +\
                             "more than once while walking the tree")
        _environments[node] = self._env
            

    def __init__(self):
        global _environments
        _environments = {}

        self._env = Environment()

        
    # Tree walking functions
    def visitAST(self, node):
        for n in node.declarations():
            n.accept(self)

        self._cache(node)

    def visitDeclRepoId(self, node):
        self._cache(node)

    def visitModule(self, node):
        name = node.identifier()
        
        self._enter(name)
        for n in node.definitions():
            n.accept(self)
        self._leave()

        self._cache(node)

    def visitInterface(self, node):
        name = node.identifier()
        
        self._enter(name)
        for n in node.declarations():
            n.accept(self)
        self._leave()

        self._cache(node)
        
    def visitForward(self, node):
        self._cache(node)
        
    def visitConst(self, node):
        self._cache(node)
        
    def visitDeclarator(self, node):
        self._cache(node)
        
    def visitTypedef(self, node):
        if node.constrType():
            node.aliasType().decl().accept(self)

        for d in node.declarators():
            d.accept(self)

        self._cache(node)

    def visitMember(self, node):
        if node.constrType():
            node.memberType().decl().accept(self)
            
        for d in node.declarators():
            d.accept(self)

        self._cache(node)
        
    def visitStruct(self, node):
        name = node.identifier()

        self._enter(name)
        for n in node.members():
            n.accept(self)
        self._leave()

        self._cache(node)
        
    def visitException(self, node):
        name = node.identifier()

        self._enter(name)
        for n in node.members():
            n.accept(self)
        self._leave()

        self._cache(node)
        
    def visitUnion(self, node):
        name = node.identifier()

        self._enter(name)
        # deal with constructed switch type
        if node.constrType():
            node.switchType().decl().accept(self)

        # deal with constructed member types
        for n in node.cases():
            if n.constrType():
                n.caseType().decl().accept(self)

        self._leave()
        self._cache(node)

    def visitCaseLabel(self, node):
        pass

    def visitUnionCase(self, node):
        pass

    def visitEnumerator(self, node):
        pass

    def visitEnum(self, node):
        self._cache(node)

    def visitValue(self, node):
        name = node.identifier()
        
        self._enter(name)
        for n in node.declarations():
            n.accept(self)
        self._leave()

        self._cache(node)

        from omniidl_be.cxx import value
        value.getValueType(node)

    def visitValueAbs(self, node):
        name = node.identifier()
        
        self._enter(name)
        for n in node.declarations():
            n.accept(self)
        self._leave()

        self._cache(node)

        from omniidl_be.cxx import value
        value.getValueType(node)

    def visitValueBox(self, node):
        if node.constrType():
            node.boxedType().decl().accept(self)

        name = node.identifier()

        self._cache(node)

        from omniidl_be.cxx import value
        value.getValueType(node)
