# -*- python -*-
#                           Package   : omniidl
# typecode.py               Created on: 1999/12/2
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
#   Produces the instances of CORBA::TypeCode

# $Id$
# $Log$
# Revision 1.14.2.2  2000/03/09 15:21:48  djs
# Better handling of internal compiler exceptions (eg attempts to use
# wide string types)
#
# Revision 1.14.2.1  2000/02/14 18:34:55  dpg1
# New omniidl merged in.
#
# Revision 1.14  2000/01/20 18:26:45  djs
# Moved large C++ output strings into an external template file
#
# Revision 1.13  2000/01/19 09:35:48  djs
# *** empty log message ***
#
# Revision 1.12  2000/01/17 17:07:29  djs
# Better handling of recursive types
#
# Revision 1.11  2000/01/13 18:16:35  djs
# A little formatting
#
# Revision 1.10  2000/01/13 15:56:35  djs
# Factored out private identifier prefix rather than hard coding it all through
# the code.
#
# Revision 1.9  2000/01/13 14:16:25  djs
# Properly clears state between processing separate IDL input files
#
# Revision 1.8  2000/01/11 11:33:55  djs
# Tidied up
#
# Revision 1.7  2000/01/07 20:31:24  djs
# Regression tests in CVSROOT/testsuite now pass for
#   * no backend arguments
#   * tie templates
#   * flattened tie templates
#   * TypeCode and Any generation
#
# Revision 1.6  1999/12/26 16:41:28  djs
# Fix to output TypeCode information on Enums #included from another IDL
# file. Mimics behaviour of the old BE
#
# Revision 1.5  1999/12/24 18:16:39  djs
# Array handling and TypeCode building fixes (esp. across multiple files)
#
# Revision 1.4  1999/12/16 16:10:05  djs
# Fix to make the output order consistent with the old compiler
#
# Revision 1.3  1999/12/14 11:51:53  djs
# Support for CORBA::TypeCode, CORBA::Any and CORBA::Object
#
# Revision 1.2  1999/12/10 18:26:36  djs
# Moved most #ifdef buildDesc code into a separate module
# General tidying up
#
# Revision 1.1  1999/12/09 20:40:14  djs
# TypeCode and Any generation option performs identically to old compiler for
# all current test fragments.
#

"""Produces the instances of CORBA::TypeCode"""

import string, re

from omniidl import idlast, idltype, idlutil
from omniidl_be.cxx import tyutil, util, config, name
from omniidl_be.cxx.dynskel import tcstring, template

import typecode

self = typecode

# For a given type declaration, creates (private) static instances of
# CORBA::TypeCode_ptr for that type, and any necessary for contained
# constructed types.
# eg
#   IDL:   struct testStruct{
#            char a;
#            foo  b;  // b is defined elsewhere
#          };
# becomes: static CORBA::PR_StructMember _0RL_structmember_testStruct[] = {
#            {"a", CORBA::TypeCode::PR_char_tc()},
#            {"b", _0RL_tc_foo} // defined elsewhere
#          };
#          static CORBA::TypeCode_ptr _0RL_tc_testStruct = .....
#
# Types constructed in the main file have an externally visible symbol
# defined:
#          const CORBA::TypeCode_ptr _tc_testStruct = _0RL_tc_testStruct
#

# ----------------------------------------------------------------------
# Utility functions local to this module start here
# ----------------------------------------------------------------------

class NameAlreadyDefined:
    def __str__(self):
        return "Name has already been defined in this scope/block/file/section"
    pass

# returns true if the name has already been defined, and need not be defined
# again.
def alreadyDefined(mangledname):
    return self.__defined_names.has_key(mangledname)

def defineName(mangledname):
    self.__defined_names[mangledname] = 1

# mangleName("_0RL_tc", ["CORBA", "Object"]) -> "_ORL_tc_CORBA_Object"
def mangleName(prefix, scopedName):
    mangled = prefix + tyutil.guardName(scopedName)
    return mangled

# We need to be able to detect recursive types so keep track of the current
# node here
self.__currentNodes = []

def startingNode(node):
    self.__currentNodes.append(node)
def finishingNode():
    assert(self.__currentNodes != [])
    self.__currentNodes = self.__currentNodes[0:len(self.__currentNodes)-1]
def recursive(node):
    return node in self.__currentNodes
def recursive_Depth(node):
    assert recursive(node)
    outer = self.__currentNodes[:]
    depth = 1

    while(1):
        if outer[-1] == node:
            return depth
        depth = depth + 1
        outer = outer[0:len(outer)-1]


def __init__(stream):
    self.stream = stream
    # declarations are built in two halves, this is to allow us
    # to keep the same order as the old backend. It could be simpler.
    self.tophalf = stream
    self.bottomhalf = stream
    self.__immediatelyInsideModule = 0

    # Hashtable with keys representing names defined. If two structures both
    # have a member of type foo, we should still only define the TypeCode for
    # foo once.
    self.__defined_names = {}

    # Normally when walking over the tree we only consider things defined in
    # the current file. However if we encounter a dependency between something
    # in the current file and something defined elsewhere, we set the override
    # flag and recurse again.
    self.__override = 0
    
    return self

# Places TypeCode symbol in appropriate namespace with a non-static const
# declaration (performs MSVC workaround)
def external_linkage(decl, mangled_name = ""):
    assert isinstance(decl, idlast.DeclRepoId)

    # only needed at all if declaration is in the current file
    if not(decl.mainFile()):
        return

    where = bottomhalf
    scopedName = map(tyutil.mapID, decl.scopedName())
    scope = tyutil.scope(scopedName)
    tc_name = name.prefixName(scopedName, "_tc_")
    tc_unscoped_name = "_tc_" + tyutil.name(scopedName)

    if mangled_name == "":
        mangled_name = mangleName(config.privatePrefix() + "_tc_",
                                  decl.scopedName())

    if alreadyDefined(tc_name):
        return
    defineName(tc_name)

    global_scope = len(scope) == 0

    # Needs the workaround if directly inside a module
    if not(self.__immediatelyInsideModule):
        where.out("""\
const CORBA::TypeCode_ptr @tc_name@ = @mangled_name@;
""",
                  tc_name = tc_name, mangled_name = mangled_name)
        return

    # do we need to make a namespace alias?
    namespace = ""
    if len(scope) > 1:
        flatscope = string.join(scope, "_")
        realscope = string.join(scope, "::")
        namespace = "namespace " + flatscope + " = " + realscope + ";"
    elif scope == []:
        return
    else:
        flatscope = scope[0]

    where.out(template.external_linkage,
              namespace = namespace,
              scope = flatscope,
              tc_name = tc_name,
              mangled_name = mangled_name,
              tc_unscoped_name = tc_unscoped_name)
    

        

# Gets a TypeCode instance for a type
# Basic types have new typecodes generated, derived types are assumed
# to already exist and a name is passed instead
def mkTypeCode(type, declarator = None, node = None):
    assert isinstance(type, idltype.Type)

    prefix = "CORBA::TypeCode::PR_"

    if declarator:
        assert isinstance(declarator, idlast.Declarator)
        dims = declarator.sizes()
        pre_str = ""
        post_str = ""
        for dim in dims:
            pre_str = pre_str + prefix + "array_tc(" + str(dim) + ", "
            post_str = post_str + ")"

        return pre_str + mkTypeCode(type, None, node) + post_str

    basic = {
        idltype.tk_short:   "short",
        idltype.tk_long:    "long",
        idltype.tk_ushort:  "ushort",
        idltype.tk_ulong:   "ulong",
        idltype.tk_float:   "float",
        idltype.tk_double:  "double",
        idltype.tk_boolean: "boolean",
        idltype.tk_char:    "char",
        idltype.tk_octet:   "octet",
        idltype.tk_any:     "any",
        idltype.tk_TypeCode: "TypeCode",
        }
    if basic.has_key(type.kind()):
        return prefix + basic[type.kind()] + "_tc()"

    if isinstance(type, idltype.Base):
        util.fatalError("Internal error generating TypeCode data")
        raise "Don't know how to generate TypeCode for Base kind = " +\
              repr(type.kind())

    if isinstance(type, idltype.String):
        return prefix + "string_tc(" + str(type.bound()) + ")"

    if isinstance(type, idltype.WString):
        util.fatalError("Wide-strings are not supported")
        raise "Don't know how to generate TypeCode for WString"

    if isinstance(type, idltype.Sequence):
        seqType = type.seqType()
        # is the sequence type the same as the current node being defined
        # (ie is it recursive)
        if isinstance(seqType, idltype.Declared) and \
           recursive(seqType.decl()):
            depth = recursive_Depth(seqType.decl())
            return prefix + "recursive_sequence_tc(" + str(type.bound()) +\
                   ", " + str(depth) + ")"
            
        return prefix + "sequence_tc(" + str(type.bound()) + ", " +\
               mkTypeCode(type.seqType()) + ")"

    if isinstance(type, idltype.Fixed):
        util.fatalError("Fixed types are not supported")
        raise "Don't know how to generate TypeCode for Fixed"

    assert isinstance(type, idltype.Declared)

    if tyutil.isObjRef(type):
        scopedName = type.decl().scopedName()
        if scopedName == ["CORBA", "Object"]:
            return prefix + "Object_tc()"
        repoID = type.decl().repoId()
        if config.EMULATE_BUGS():
            repoID = tyutil.mapRepoID(repoID)
        iname = tyutil.mapID(tyutil.name(scopedName))
        return prefix + "interface_tc(\"" + repoID + "\", " +\
                   "\"" + iname + "\")"

    guard_name = tyutil.guardName(type.scopedName())

    return config.privatePrefix() + "_tc_" + guard_name
        
# ---------------------------------------------------------------
# Tree-walking part of module starts here
# ---------------------------------------------------------------

# Control arrives here
#
def visitAST(node):
    self.__completedModules = {}
    for n in node.declarations():
        n.accept(self)


def visitModule(node):
    # no override check required because modules aren't types constructed
    # inside other things :)
    if not(node.mainFile()):
        return

    slash_scopedName = string.join(node.scopedName(), '/')
    if self.__completedModules.has_key(slash_scopedName):
        return
    self.__completedModules[slash_scopedName] = 1
    
    # This has a bearing on making symbols externally visible/ linkable
    insideModule = self.__immediatelyInsideModule
    self.__immediatelyInsideModule = 1
    for n in node.definitions():
        n.accept(self)
    for c in node.continuations():
        slash_scopedName = string.join(c.scopedName(), '/')
        self.__completedModules[slash_scopedName] = 1
        for n in c.definitions():
            n.accept(self)
            
    self.__immediatelyInsideModule = insideModule

# builds an instance of CORBA::PR_structMember containing pointers
# to all the TypeCodes of the structure members
def buildMembersStructure(node):
    struct = util.StringStream()
    mangled_name = mangleName(config.privatePrefix() + \
                              "_structmember_", node.scopedName())
    if alreadyDefined(mangled_name):
        # no need to regenerate
        return struct
    
    defineName(mangled_name)
    
    members = node.members()
    array = []

    for m in members:
        memberType = m.memberType()
        for d in m.declarators():
            this_name = tyutil.mapID(tyutil.name(d.scopedName()))
            typecode = mkTypeCode(memberType, d, node)
            array.append( "{\"" + this_name + "\", " + typecode + "}" )

    if len(members) > 0:
        struct.out("""\
static CORBA::PR_structMember @mangled_name@[] = {
  @members@
};""", members = string.join(array, ",\n"), mangled_name = mangled_name)

    return struct

# Convenience function to total up the number of members, treating
# declarators separately.
def numMembers(node):
    members = node.members()
    num = 0
    for m in members:
        num = num + len(m.declarators())

    return num
            

def visitStruct(node):
    if not(node.mainFile()) and not(self.__override):
        return
    startingNode(node)
    
    # the key here is to redirect the bottom half to a buffer
    # just for now
    oldbottomhalf = self.bottomhalf
    self.bottomhalf = util.StringStream()

    insideModule = self.__immediatelyInsideModule
    self.__immediatelyInsideModule = 0

    # create the static typecodes for constructed types by setting
    # the override flag and recursing
    override = self.__override
    self.__override = 1
    
    for child in node.members():
        memberType = child.memberType()
        #if child.constrType():
        if isinstance(memberType, idltype.Declared):
            memberType.decl().accept(self)

    self.__override = override

    tophalf.out(str(buildMembersStructure(node)))

    scopedName = node.scopedName()
    mangled_name = mangleName(config.privatePrefix() + "_tc_", scopedName)
    if not(alreadyDefined(mangled_name)):
        # only define the name once

        defineName(mangled_name)
    
        structmember_mangled_name = mangleName(config.privatePrefix() + \
                                               "_structmember_", scopedName)
        assert alreadyDefined(structmember_mangled_name), \
               "The name \"" + structmember_mangled_name + \
               "\" should be defined by now"
    
        num = numMembers(node)
        repoID = node.repoId()
        if config.EMULATE_BUGS():
            repoID = tyutil.mapRepoID(repoID)
        struct_name = tyutil.mapID(tyutil.name(scopedName))

        tophalf.out("""\
static CORBA::TypeCode_ptr @mangled_name@ = CORBA::TypeCode::PR_struct_tc("@repoID@", "@name@", @structmember_mangled_name@, @n@);""",
                    mangled_name = mangled_name,
                    structmember_mangled_name = structmember_mangled_name,
                    name = struct_name, n = str(num),
                    repoID = repoID)   


    self.__immediatelyInsideModule = insideModule

    external_linkage(node)
    # restore the old bottom half
    oldbottomhalf.out(str(self.bottomhalf))
    self.bottomhalf = oldbottomhalf
    finishingNode()
    return

    

def visitUnion(node):
    if not(node.mainFile()) and not(self.__override):
        return

    scopedName = node.scopedName()
    mangled_name = mangleName(config.privatePrefix() + "_tc_", scopedName)
    if alreadyDefined(mangled_name):
        return

    startingNode(node)
    
    # the key here is to redirect the bottom half to a buffer
    # just for now
    oldbottomhalf = self.bottomhalf
    self.bottomhalf = util.StringStream()

    
    # need to build a static array of node members in a similar fashion
    # to structs
    array = []
    switchType = node.switchType()
    deref_switchType = tyutil.deref(switchType)
    if isinstance(switchType, idltype.Declared):
        override = self.__override
        self.__override = 1
        switchType.decl().accept(self)
        self.__override = override
        
    numlabels = 0
    numcases = 0
    hasDefault = None
    
    for c in node.cases():
        numcases = numcases + 1
        decl = c.declarator()
        caseType = c.caseType()

        override = self.__override
        self.__override = 1
        if isinstance(caseType, idltype.Declared):
            caseType.decl().accept(self)
        elif isinstance(caseType, idltype.Sequence):
            # anonymous sequence
            seqType = caseType.seqType()
            while isinstance(seqType, idltype.Sequence):
                seqType = seqType.seqType()
            # careful of recursive unions
            if isinstance(seqType, idltype.Declared) and \
               seqType.decl() != node:
                seqType.decl().accept(self)
                
        self.__override = override
        
        typecode = mkTypeCode(caseType, decl, node)
        case_name = tyutil.mapID(tyutil.name(decl.scopedName()))

        env = name.Environment()
        for l in c.labels():
            if l.default():
                label = "0"
                hasDefault = numlabels
            # FIXME: same problem happens in defs/header and skel/main
            elif tyutil.isChar(switchType) and l.value() == '\0':
                label = "0000"
            else:
                label = tyutil.valueString(switchType, l.value(), env)
            array.append( "{\"" + case_name + "\", " + typecode + ", " + label + "}")
            numlabels = numlabels + 1


    discrim_tc = mkTypeCode(deref_switchType)
    repoID = node.repoId()
    if config.EMULATE_BUGS():
        repoID = tyutil.mapRepoID(repoID)

    union_name = tyutil.mapID(tyutil.name(scopedName))
    unionmember_mangled_name = mangleName(config.privatePrefix() + \
                                          "_unionMember_", scopedName)
    
    default_str = ""
    if hasDefault != None:
        default_str = ", " + str(hasDefault)

    tophalf.out("""\
static CORBA::PR_unionMember @unionmember_mangled_name@[] = {
  @members@
};
static CORBA::TypeCode_ptr @mangled_name@ = CORBA::TypeCode::PR_union_tc("@repoID@", "@name@", @discrim_tc@, @unionmember_mangled_name@, @cases@@default_str@);""",
                mangled_name = mangled_name,
                repoID = repoID,
                discrim_tc = discrim_tc,
                unionmember_mangled_name = unionmember_mangled_name,
                name = union_name,
                cases = str(numcases),
                default_str = default_str,
                members = string.join(array, ",\n"))
    
    defineName(unionmember_mangled_name)
    defineName(mangled_name)
    
    external_linkage(node)

    # restore the old bottom half
    oldbottomhalf.out(str(self.bottomhalf))
    self.bottomhalf = oldbottomhalf

    finishingNode()

def visitEnum(node):
    if not(node.mainFile()) and not(self.__override):
        return
    
    scopedName = node.scopedName()
    mangled_name = mangleName(config.privatePrefix() + "_tc_", scopedName)
    if alreadyDefined(mangled_name):
        return
    
    enumerators = node.enumerators()

    names = []
    for enumerator in enumerators:
        names.append("\"" + tyutil.name(enumerator.scopedName()) + "\"")

    enum_name = tyutil.name(scopedName)
    
    repoID = node.repoId()
    if config.EMULATE_BUGS():
        repoID = tyutil.mapRepoID(repoID)

    tc_name = name.prefixName(scopedName, "_tc_")
    enummember_mangled_name = mangleName(config.privatePrefix() + \
                                         "_enumMember_", scopedName)

    tophalf.out("""\
static const char* @enummember_mangled_name@[] = { @elements@ };
static CORBA::TypeCode_ptr @mangled_name@ = CORBA::TypeCode::PR_enum_tc("@repoID@", "@name@", @enummember_mangled_name@, @numcases@);""",
                enummember_mangled_name = enummember_mangled_name,
                mangled_name = mangled_name,
                elements = string.join(names, ", "),
                repoID = repoID,
                name = enum_name,
                numcases = str(len(names)))

    defineName(mangled_name)
    defineName(enummember_mangled_name)

    external_linkage(node)

def visitForward(node):
    pass
    

def visitInterface(node):
    if not(node.mainFile()):
        return
    # interfaces containing members with the type of the interface
    # cause a minor (non fatal) problem with ordering of the outputted
    # declarations. This check only serves to correct this cosmetic flaw
    # and make the output of the new system identical to the old one.
    if hasattr(node, "already_been_here"):
        return
    node.already_been_here = 1

    startingNode(node)

    insideModule = self.__immediatelyInsideModule
    self.__immediatelyInsideModule = 0
    for n in node.declarations():
        n.accept(self)
    self.__immediatelyInsideModule = insideModule
    
    repoID = node.repoId()
    if config.EMULATE_BUGS():
        repoID = tyutil.mapRepoID(repoID)
    iname = tyutil.mapID(tyutil.name(node.scopedName()))
    typecode = "CORBA::TypeCode::PR_interface_tc(\"" + repoID + "\", \"" +\
               iname + "\")"

    node.accept(tcstring)


    external_linkage(node, typecode)
    finishingNode()


def recurse(type):
    deref_type = tyutil.deref(type)
    if isinstance(type, idltype.Declared):
        base_decl = type.decl()
        override = self.__override
        self.__override = 1
        base_decl.accept(self)
        self.__override = override
    elif tyutil.isSequence(deref_type):
        seqType = deref_type.seqType()
        if isinstance(seqType, idltype.Declared):
            base_decl = seqType.decl()

            override = self.__override
            self.__override = 1
            base_decl.accept(self)
            self.__override = override
        elif isinstance(seqType, idltype.Sequence):
            # anonymous sequence
            recurse(seqType.seqType())

            
        

def visitDeclarator(declarator):
    # this must be a typedef declarator
    
    node = declarator.alias()
    aliasType = node.aliasType()

    recurse(aliasType)
    
    scopedName = declarator.scopedName()
    mangled_name = mangleName(config.privatePrefix() + "_tc_", scopedName)
    if alreadyDefined(mangled_name):
        return
    
    repoID = declarator.repoId()
    if config.EMULATE_BUGS():
        repoID = tyutil.mapRepoID(repoID)
    typecode = mkTypeCode(aliasType, declarator)
        
    scopedName = declarator.scopedName()
    typedef_name = tyutil.mapID(tyutil.name(scopedName))
    
    tophalf.out("""\
static CORBA::TypeCode_ptr @mangled_name@ = CORBA::TypeCode::PR_alias_tc("@repoID@", "@name@", @typecode@);

""",
                mangled_name = mangled_name,
                repoID = repoID,
                name = typedef_name,
                typecode = typecode)
    defineName(mangled_name)


    external_linkage(declarator)    

def visitTypedef(node):
    if not(node.mainFile()) and not(self.__override):
        return

    aliasType = node.aliasType()

    prefix = "CORBA::TypeCode::PR_"

    recurse(aliasType)

    
    for declarator in node.declarators():
        declarator.accept(self)

        
def visitConst(node):
    pass


def visitException(node):
    if not(node.mainFile()):
        return

    scopedName = node.scopedName()
    mangled_name = mangleName(config.privatePrefix() + "_tc_", scopedName)
    if alreadyDefined(mangled_name):
        return
    defineName(mangled_name)

    startingNode(node)
    
    # the key here is to redirect the bottom half to a buffer
    # just for now
    oldbottomhalf = self.bottomhalf
    self.bottomhalf = util.StringStream()

    # create the static typecodes for constructed types by setting
    # the override flag and recursing
    override = self.__override
    self.__override = 1

    insideModule = self.__immediatelyInsideModule
    self.__immediatelyInsideModule = 0
    
    for child in node.members():
        memberType = child.memberType()
        if isinstance(memberType, idltype.Declared):
            memberType.decl().accept(self)

    self.__override = override
    self.__immediatelyInsideModule = insideModule

    
    # output the structure of members
    tophalf.out(str(buildMembersStructure(node)))
    
    tc_name = name.prefixName(scopedName, "_tc_")

    num = numMembers(node)

    repoID = node.repoId()
    if config.EMULATE_BUGS():
        repoID = tyutil.mapRepoID(repoID)
    ex_name = tyutil.mapID(tyutil.name(scopedName))
    structmember_mangled_name = mangleName(config.privatePrefix() + \
                                           "_structmember_", scopedName)
    if num == 0:
        structmember_mangled_name = "(CORBA::PR_structMember*) 0"
    tophalf.out("""\
static CORBA::TypeCode_ptr @mangled_name@ = CORBA::TypeCode::PR_exception_tc("@repoID@", "@name@", @structmember_mangled_name@, @n@);""",
                mangled_name = mangled_name,
                name = ex_name, n = str(num),
                structmember_mangled_name = structmember_mangled_name,
                repoID = repoID)

    external_linkage(node)

    # restore the old bottom half
    oldbottomhalf.out(str(self.bottomhalf))
    self.bottomhalf = oldbottomhalf


    finishingNode()
