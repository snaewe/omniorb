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

from omniidl.be.cxx import tyutil, util, config, name

import typecode

self = typecode

# name handling code

self.__defined_names = {}
# set to 1 to ignore the defined-in-this-file check.
# The old C++ BE seems to regenerate some symbols. Does it have to?
self.__override = 0

class NameAlreadyDefined:
    def __str__(self):
        return "Name has already been defined in this scope/block/file/section"
    pass


def mangleName(prefix, scopedName):
    mangled = prefix + tyutil.guardName(scopedName)
    if self.__defined_names.has_key(mangled):
        raise NameAlreadyDefined()
    self.__defined_names[mangled] = 1
    return mangled

    
def __init__(stream):
    self.stream = stream
    # declarations are built in two halves, this is to allow us
    # to keep the same order as the old backend. It could be simpler.
    self.tophalf = stream
    self.bottomhalf = stream
    self.__immediatelyInsideModule = 0
    return self

# Control arrives here
#
def visitAST(node):
    for n in node.declarations():
        n.accept(self)

# It appears that the old compiler will map names in repository IDs
# to avoid supposed clashes with C++ keywords, but this is totally
# broken
def mapRepoID(id):
    # extract the naming part of the ID
    regex = re.compile(r"(IDL:)*(.+):(.+)")
    match = regex.match(id)
    first_bit = match.group(1)
    if not(first_bit):
        first_bit = ""
    the_name = match.group(2)
    ver = match.group(3)
    # extract the name 
    elements = re.split(r"/", the_name)
    mapped_elements = []
    for element in elements:
        mapped_elements.append(tyutil.mapID(element))
    # put it all back together again
    return first_bit + string.join(mapped_elements, "/") + ":" + ver

# Places TypeCode symbol in appropriate namespace
def external_linkage(decl, mangled_name = ""):
    assert isinstance(decl, idlast.DeclRepoId)

    if not(decl.mainFile()):
        return

    where = bottomhalf
    scopedName = map(tyutil.mapID, decl.scopedName())
    scope = tyutil.scope(scopedName)
    tc_name = name.prefixName(scopedName, "_tc_")
    tc_unscoped_name = "_tc_" + tyutil.name(scopedName)

    if mangled_name == "":
        mangled_name = "_0RL_tc_" + tyutil.guardName(decl.scopedName())

    global_scope = len(scope) == 0

    # Needs the workaround if directly inside a module

    if not(self.__immediatelyInsideModule):
        where.out("""\
const CORBA::TypeCode_ptr @tc_name@ = @mangled_name@;
""",
                  tc_name = tc_name, mangled_name = mangled_name)
        return

    where.out("""\
#if defined(HAS_Cplusplus_Namespace) && defined(_MSC_VER)
// MSVC++ does not give the constant external linkage otherwise.""")
    if len(scope) > 1:
        flatscope = string.join(scope, "_")
        realscope = string.join(scope, "::")
        where.out("""\
  namespace @flat_scope@ = @real_scope@;""",
                   flat_scope = flatscope, real_scope = realscope)
    else:
        flatscope = scope[0]

    env = name.Environment()
            
    where.out("""\
namespace @scope@ {
  const CORBA::TypeCode_ptr @tc_unscoped_name@ = @mangled_name@;
}
#else
const CORBA::TypeCode_ptr @tc_name@ = @mangled_name@;

#endif
""", scope = flatscope, tc_name = tc_name, mangled_name = mangled_name,
               tc_unscoped_name = tc_unscoped_name)
        

# Gets a TypeCode instance for a type
# Basic types have new typecodes generated, derived types are assumed
# to already exist and a name is passed instead
def mkTypeCode(type, declarator = None):
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

        return pre_str + mkTypeCode(type, None) + post_str

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
        raise "Don't know how to generate TypeCode for Base kind = " +\
              repr(type.kind())

    if isinstance(type, idltype.String):
        return prefix + "string_tc(" + str(type.bound()) + ")"

    if isinstance(type, idltype.WString):
        raise "Don't know how to generate TypeCode for WString"

    if isinstance(type, idltype.Sequence):
        seqType = type.seqType()
        return prefix + "sequence_tc(" + str(type.bound()) + ", " +\
               mkTypeCode(type.seqType()) + ")"

    if isinstance(type, idltype.Fixed):
        raise "Don't know how to generate TypeCode for Fixed"

    assert isinstance(type, idltype.Declared)

    if tyutil.isObjRef(type):
        scopedName = type.decl().scopedName()
        if scopedName == ["CORBA", "Object"]:
            return prefix + "Object_tc()"
        repoID = type.decl().repoId()
        if config.EMULATE_BUGS():
            repoID = mapRepoID(repoID)
        iname = tyutil.mapID(tyutil.name(scopedName))
        return prefix + "interface_tc(\"" + repoID + "\", " +\
                   "\"" + iname + "\")"

    guard_name = tyutil.guardName(type.scopedName())

    return "_0RL_tc_" + guard_name
        


def visitModule(node):
    if not(node.mainFile()):
        return
    
    insideModule = self.__immediatelyInsideModule
    self.__immediatelyInsideModule = 1
    for n in node.definitions():
        n.accept(self)
    self.__immediatelyInsideModule = insideModule

# builds an instance of CORBA::PR_structMember containing pointers
# to all the TypeCodes of the structure members
def buildMembersStructure(node):
    members = node.members()
    array = []

    for m in members:
        memberType = m.memberType()
        for d in m.declarators():
            this_name = tyutil.mapID(tyutil.name(d.scopedName()))
            typecode = mkTypeCode(memberType, d)
            array.append( "{\"" + this_name + "\", " + typecode + "}" )

    struct = util.StringStream()
    scopedName = node.scopedName()
    mangled_name = mangleName("_0RL_structmember_", node.scopedName())

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
            
def visitStruct_structMember(node):

    def mkTypeCode(node):
        scopedName = node.scopedName()
        mangled_name = mangleName("_0RL_tc_", scopedName)
        guard_name = tyutil.guardName(scopedName)
        num = numMembers(node)
        repoID = node.repoId()
        if config.EMULATE_BUGS():
            repoID = mapRepoID(repoID)
            struct_name = tyutil.mapID(tyutil.name(scopedName))
            tophalf.out("""\
static CORBA::TypeCode_ptr @mangled_name@ = CORBA::TypeCode::PR_struct_tc("@repoID@", "@name@", _0RL_structmember_@guard_name@, @n@);""",
                        mangled_name = mangled_name,
                        guard_name = guard_name,
                        name = struct_name, n = str(num),
                        repoID = repoID)

    for child in node.members():
        if child.constrType():
            child.memberType().decl().accept(self)

    tophalf.out(str(buildMembersStructure(node)))
    mkTypeCode(node)
    return


#def visitStruct_linkage(node):
#
#    insideModule = self.__immediatelyInsideModule
#    self.__immediatelyInsideModule = 0
#    for child in node.members():
#        if child.constrType():
#            child.memberType().decl().accept(self)
#    self.__immediatelyInsideModule = insideModule
#
#    external_linkage(node)
#    return
    
def visitStruct(node):
    if not(node.mainFile()):
        return

    # the key here is to redirect the bottom half to a buffer
    # just for now
    oldbottomhalf = self.bottomhalf
    self.bottomhalf = util.StringStream()
    # do the recursion

    insideModule = self.__immediatelyInsideModule
    self.__immediatelyInsideModule = 0    
    visitStruct_structMember(node)
    self.__immediatelyInsideModule = insideModule
    
    external_linkage(node)
    # restore the old bottom half
    oldbottomhalf.out(str(self.bottomhalf))
    self.bottomhalf = oldbottomhalf
    return

    

def visitUnion(node):
    if not(node.mainFile()):
        return
    
    # need to build a static array of node members in a similar fashion
    # to structs
    array = []
    switchType = node.switchType()
    numlabels = 0
    numcases = 0
    hasDefault = None
    
    for c in node.cases():
        numcases = numcases + 1
        decl = c.declarator()
        caseType = c.caseType()
        typecode = mkTypeCode(caseType, decl)
        case_name = tyutil.mapID(tyutil.name(decl.scopedName()))

        env = name.Environment()
        for l in c.labels():
            if l.default():
                label = "0"
                hasDefault = numlabels
            else:
                label = tyutil.valueString(switchType, l.value(), env)
            array.append( "{\"" + case_name + "\", " + typecode + ", " + label + "}")
            numlabels = numlabels + 1

    scopedName = node.scopedName()
    guard_name = tyutil.guardName(scopedName)
    mangled_name = mangleName("_0RL_tc_", scopedName)

    discrim_tc = mkTypeCode(switchType)
    repoID = node.repoId()
    if config.EMULATE_BUGS():
        repoID = mapRepoID(repoID)

    union_name = tyutil.mapID(tyutil.name(node.scopedName()))

    default_str = ""
    if hasDefault != None:
        default_str = ", " + str(hasDefault)

    tophalf.out("""\
static CORBA::PR_unionMember _0RL_unionMember_@guard_name@[] = {
  @members@
};
static CORBA::TypeCode_ptr @mangled_name@ = CORBA::TypeCode::PR_union_tc("@repoID@", "@name@", @discrim_tc@, _0RL_unionMember_@guard_name@, @cases@@default_str@);""", mangled_name = mangled_name, repoID = repoID, discrim_tc = discrim_tc,
                guard_name = guard_name,
                name = union_name,
                cases = str(numcases), labels = str(numlabels),
                default_str = default_str,
                members = string.join(array, ",\n"))
    
    external_linkage(node)


def visitMember(node):
    if node.constrType():
        node.memberType().decl().accept(self)

def visitEnum(node):
    if not(node.mainFile()):
        return
    
    scopedName = node.scopedName()
    guard_name = tyutil.guardName(scopedName)
    mangled_name = mangleName("_0RL_tc_", scopedName)
    enumerators = node.enumerators()

    names = []
    for enumerator in enumerators:
        names.append("\"" + tyutil.name(enumerator.scopedName()) + "\"")

    enum_name = tyutil.name(scopedName)
    
    repoID = node.repoId()
    if config.EMULATE_BUGS():
        repoID = mapRepoID(repoID)

    tc_name = name.prefixName(scopedName, "_tc_")

    tophalf.out("""\
static const char* _0RL_enumMember_@guard_name@[] = { @elements@ };
static CORBA::TypeCode_ptr @mangled_name@ = CORBA::TypeCode::PR_enum_tc("@repoID@", "@name@", _0RL_enumMember_@guard_name@, @numcases@);""",
                guard_name = guard_name,
                mangled_name = mangled_name,
                elements = string.join(names, ", "),
                repoID = repoID,
                name = enum_name,
                numcases = str(len(names)))

    external_linkage(node)

def visitInterface(node):
    if not(node.mainFile()):
        return

    insideModule = self.__immediatelyInsideModule
    self.__immediatelyInsideModule = 0
    for n in node.declarations():
        n.accept(self)
    self.__immediatelyInsideModule = insideModule
    
    repoID = node.repoId()
    if config.EMULATE_BUGS():
        repoID = mapRepoID(repoID)
    iname = tyutil.mapID(tyutil.name(node.scopedName()))
    typecode = "CORBA::TypeCode::PR_interface_tc(\"" + repoID + "\", \"" +\
               iname + "\")"
    external_linkage(node, typecode)


def visitTypedef(node):
    if not(node.mainFile()) and not(self.__override):
        return

    aliasType = node.aliasType()

    prefix = "CORBA::TypeCode::PR_"

    # FIXME: make the base type
    # (this is probably not needed, since the definition will be around
    # either in this file or #included)
    # (we only do this, if the base type's definition is in another file)
    if isinstance(aliasType, idltype.Declared):
        if hasattr(aliasType.decl(), "alias"):
            base_decl = aliasType.decl().alias()
            if not(base_decl.mainFile()):
                override = self.__override
                self.__override = 1
                base_decl.accept(self)
                self.__override = override
    
    
    
    for declarator in node.declarators():
        repoID = declarator.repoId()
        if config.EMULATE_BUGS():
            repoID = mapRepoID(repoID)
        typecode = mkTypeCode(aliasType, declarator)
        
        scopedName = declarator.scopedName()
        typedef_name = tyutil.mapID(tyutil.name(scopedName))
        try:
            mangled_name = mangleName("_0RL_tc_", scopedName)
            tophalf.out("""\
static CORBA::TypeCode_ptr @mangled_name@ = CORBA::TypeCode::PR_alias_tc("@repoID@", "@name@", @typecode@);

""", mangled_name = mangled_name, repoID = repoID,
                   name = typedef_name, typecode = typecode)
        except NameAlreadyDefined:
            pass


        external_linkage(declarator)

        
def visitForward(node):
    pass
def visitConst(node):
    pass
def visitDeclarator(node):
    pass

def visitException(node):
    if not(node.mainFile()):
        return
    
    # output the structure of members
    stream.out(str(buildMembersStructure(node)))
    
    scopedName = node.scopedName()
    guard_name = tyutil.guardName(scopedName)
    mangled_name = mangleName("_0RL_tc_", scopedName)
    tc_name = name.prefixName(scopedName, "_tc_")

    num = numMembers(node)

    repoID = node.repoId()
    if config.EMULATE_BUGS():
        repoID = mapRepoID(repoID)
    ex_name = tyutil.mapID(tyutil.name(scopedName))
    struct_member = "_0RL_structmember_" + guard_name
    if num == 0:
        struct_member = "(CORBA::PR_structMember*) 0"
    tophalf.out("""\
static CORBA::TypeCode_ptr @mangled_name@ = CORBA::TypeCode::PR_exception_tc("@repoID@", "@name@", @struct_member@, @n@);""",
               mangled_name = mangled_name, name = ex_name, n = str(num),
               struct_member = struct_member,
               repoID = repoID)

    external_linkage(node)
    



