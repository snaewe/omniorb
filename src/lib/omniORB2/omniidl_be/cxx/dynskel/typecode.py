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

def __init__(stream):
    self.stream = stream
    self.__needs_ms_workaround = 1
    self.__immediatelyInsideModule = 0
    self.__put_it_here = None
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
    regex = re.compile(r"IDL:(.+):(.+)")
    match = regex.match(id)
    the_name = match.group(1)
    ver = match.group(2)
    # extract the name 
    elements = re.split(r"/", the_name)
    mapped_elements = []
    for element in elements:
        mapped_elements.append(tyutil.mapID(element))
    # put it all back together again
    return "IDL:" + string.join(mapped_elements, "/") + ":" + ver

# Places TypeCode symbol in appropriate namespace
def external_linkage(decl, mangled_name = ""):
    assert isinstance(decl, idlast.DeclRepoId)

    if self.__put_it_here == None:
        where = stream
    else:
        where = self.__put_it_here

    guard_name = tyutil.guardName(decl.scopedName())
    scopedName = map(tyutil.mapID, decl.scopedName())
    scope = tyutil.scope(scopedName)
    tc_name = name.prefixName(scopedName, "_tc_")
    tc_unscoped_name = "_tc_" + tyutil.name(scopedName)

    if mangled_name == "":
        mangled_name = "_0RL_tc_" + guard_name

    global_scope = len(scope) == 0

    # Needs the workaround if directly inside a module

    if not(self.__immediatelyInsideModule):
        where.out("""\
const CORBA::TypeCode_ptr @tc_name@ = @mangled_name@;
""",
                   tc_name = tc_name, mangled_name = mangled_name)
        return

    stream.out("""\
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
        idltype.tk_octet:   "octet"
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
        repoID = type.decl().repoId()
        if config.EMULATE_BUGS():
            repoID = mapRepoID(repoID)
        iname = tyutil.mapID(tyutil.name(type.decl().scopedName()))
        return prefix + "interface_tc(\"" + repoID + "\", " +\
                   "\"" + iname + "\")"

    guard_name = tyutil.guardName(type.scopedName())

    return "_0RL_tc_" + guard_name
        


def visitModule(node):
    insideModule = self.__immediatelyInsideModule
    self.__immediatelyInsideModule = 1
    for n in node.definitions():
        n.accept(self)
    self.__immediatelyInsideModule = insideModule

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
    mangled_name = tyutil.guardName(scopedName)

    if len(members) > 0:
        struct.out("""\
static CORBA::PR_structMember _0RL_structmember_@mangled_name@[] = {
  @members@
};""", members = string.join(array, ",\n"), mangled_name = mangled_name)

    return struct

def numMembers(node):
    members = node.members()
    num = 0
    for m in members:
        num = num + len(m.declarators())

    return num
            
def visitStruct_structMember(node):

    def mkTypeCode(node):
        scopedName = node.scopedName()
        mangled_name = tyutil.guardName(scopedName)
        num = numMembers(node)
        repoID = node.repoId()
        if config.EMULATE_BUGS():
            repoID = mapRepoID(repoID)
            struct_name = tyutil.mapID(tyutil.name(scopedName))
            stream.out("""\
static CORBA::TypeCode_ptr _0RL_tc_@mangled_name@ = CORBA::TypeCode::PR_struct_tc("@repoID@", "@name@", _0RL_structmember_@mangled_name@, @n@);""",
                       mangled_name = mangled_name,
                       name = struct_name, n = str(num),
                       repoID = repoID)

    # Filter the children who are nested structures
    nested_struct = filter(lambda x: x.constrType() and \
                           tyutil.isStruct(x.memberType()),
                           node.members())
    
    # Build the StructMember for the nested struct children
    for child in nested_struct:
        child_node = child.memberType().decl()
        visitStruct_structMember(child_node)

    # Build the StructMember for this structure
    stream.out(str(buildMembersStructure(node)))
    mkTypeCode(node)

def visitStruct_linkage(node):
    # Filter the children who are nested structures
    nested_struct = filter(lambda x: x.constrType() and \
                           tyutil.isStruct(x.memberType()),
                           node.members())
    insideModule = self.__immediatelyInsideModule
    self.__immediatelyInsideModule = 0
    # Sort out the external linkage for the nested struct children
    for child in nested_struct:
        child_node = child.memberType().decl()
        visitStruct_linkage(child_node)
    self.__immediatelyInsideModule = insideModule

    # Sort out our own external linkage
    external_linkage(node)
    
def visitStruct(node):

    visitStruct_structMember(node)
    visitStruct_linkage(node)
    return

    def mkTypeCode(node):
        scopedName = node.scopedName()
        mangled_name = tyutil.guardName(scopedName)
        num = numMembers(node)
        repoID = node.repoId()
        if config.EMULATE_BUGS():
            repoID = mapRepoID(repoID)
            struct_name = tyutil.mapID(tyutil.name(scopedName))
            stream.out("""\
static CORBA::TypeCode_ptr _0RL_tc_@mangled_name@ = CORBA::TypeCode::PR_struct_tc("@repoID@", "@name@", _0RL_structmember_@mangled_name@, @n@);""",
                       mangled_name = mangled_name,
                       name = struct_name, n = str(num),
                       repoID = repoID)

    # Filter the children who are nested structures
    nested_struct = filter(lambda x: x.constrType() and \
                           isinstance(x, idlast.Struct),
                           node.members())
    
    # Build the StructMember for the nested struct children
    for child in nested_struct:
        stream.out(str(buildMembersStructure(child)))
        mkTypeCode(child)
        
    # Build the StructMember for this structure
    stream.out(str(buildMembersStructure(node)))
    mkTypeCode(node)

    # Sort out the external linkage for the nested struct children
    for child in nested_struct:
        external_linkage(child)

    # Sort out our own external linkage
    external_linkage(node)
    
    return
    
    needs_ms_workaround = self.__needs_ms_workaround
    # bit of a hack to store the linkage stuff for later
    # just to mimic the old backend
    if self.__needs_ms_workaround == 1:
        # first level of nesting
        self.__put_it_here = util.StringStream()
        
    self.__needs_ms_workaround = 0
    for m in node.members():
        # deal with nested definitions
        m.accept(self)

    num = numMembers(node)
            
    self.__needs_ms_workaround = needs_ms_workaround

    scopedName = node.scopedName()
    mangled_name = tyutil.guardName(scopedName)
    stream.out(str(buildMembersStructure(node)))
    
    repoID = node.repoId()
    if config.EMULATE_BUGS():
        repoID = mapRepoID(repoID)
    struct_name = tyutil.mapID(tyutil.name(scopedName))
    stream.out("""\
static CORBA::TypeCode_ptr _0RL_tc_@mangled_name@ = CORBA::TypeCode::PR_struct_tc("@repoID@", "@name@", _0RL_structmember_@mangled_name@, @n@);""",
               mangled_name = mangled_name, name = struct_name, n = str(num),
               repoID = repoID)

    if self.__needs_ms_workaround == 1:
        # first level of nesting
        stream.out(str(self.__put_it_here))
        self.__put_it_here = None
    

    external_linkage(node)

    

def visitUnion(node):
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
    mangled_name = tyutil.guardName(scopedName)

    discrim_tc = mkTypeCode(switchType)
    repoID = node.repoId()
    if config.EMULATE_BUGS():
        repoID = mapRepoID(repoID)

    union_name = tyutil.mapID(tyutil.name(node.scopedName()))

    default_str = ""
    if hasDefault != None:
        default_str = ", " + str(hasDefault)

    stream.out("""\
static CORBA::PR_unionMember _0RL_unionMember_@mangled_name@[] = {
  @members@
};
static CORBA::TypeCode_ptr _0RL_tc_@mangled_name@ = CORBA::TypeCode::PR_union_tc("@repoID@", "@name@", @discrim_tc@, _0RL_unionMember_@mangled_name@, @cases@@default_str@);""", mangled_name = mangled_name, repoID = repoID, discrim_tc = discrim_tc,
               name = union_name,
               cases = str(numcases), labels = str(numlabels),
               default_str = default_str,
               members = string.join(array, ",\n"))

    external_linkage(node)


def visitMember(node):
    if node.constrType():
        node.memberType().decl().accept(self)

def visitEnum(node):
    scopedName = node.scopedName()
    mangled_name = tyutil.guardName(scopedName)
    enumerators = node.enumerators()

    names = []
    for enumerator in enumerators:
        names.append("\"" + tyutil.name(enumerator.scopedName()) + "\"")

    enum_name = tyutil.name(scopedName)
    
    repoID = node.repoId()
    if config.EMULATE_BUGS():
        repoID = mapRepoID(repoID)

    tc_name = name.prefixName(scopedName, "_tc_")

    stream.out("""\
static const char* _0RL_enumMember_@mangled_name@[] = { @elements@ };
static CORBA::TypeCode_ptr _0RL_tc_@mangled_name@ = CORBA::TypeCode::PR_enum_tc("@repoID@", "@name@", _0RL_enumMember_@mangled_name@, @numcases@);""",
               mangled_name = mangled_name,
               elements = string.join(names, ", "),
               repoID = repoID,
               name = enum_name,
               numcases = str(len(names)))

    external_linkage(node)

def visitInterface(node):

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

    aliasType = node.aliasType()

    prefix = "CORBA::TypeCode::PR_"
    
    for declarator in node.declarators():
        repoID = declarator.repoId()
        if config.EMULATE_BUGS():
            repoID = mapRepoID(repoID)
        typecode = mkTypeCode(aliasType, declarator)
        
        scopedName = declarator.scopedName()
        typedef_name = tyutil.mapID(tyutil.name(scopedName))
        mangled_name = "_0RL_tc_" + tyutil.guardName(scopedName)

        stream.out("""\
static CORBA::TypeCode_ptr @mangled_name@ = CORBA::TypeCode::PR_alias_tc("@repoID@", "@name@", @typecode@);

""", mangled_name = mangled_name, repoID = repoID,
                   name = typedef_name, typecode = typecode)

        external_linkage(declarator)

        
def visitForward(node):
    pass
def visitConst(node):
    pass
def visitDeclarator(node):
    pass

def visitException(node):
    # output the structure of members
    stream.out(str(buildMembersStructure(node)))
    
    scopedName = node.scopedName()
    mangled_name = tyutil.guardName(scopedName)
    tc_name = name.prefixName(scopedName, "_tc_")

    num = numMembers(node)

    repoID = node.repoId()
    if config.EMULATE_BUGS():
        repoID = mapRepoID(repoID)
    ex_name = tyutil.mapID(tyutil.name(scopedName))
    struct_member = "_0RL_structmember_" + mangled_name
    if num == 0:
        struct_member = "(CORBA::PR_structMember*) 0"
    stream.out("""\
static CORBA::TypeCode_ptr _0RL_tc_@mangled_name@ = CORBA::TypeCode::PR_exception_tc("@repoID@", "@name@", @struct_member@, @n@);""",
               mangled_name = mangled_name, name = ex_name, n = str(num),
               struct_member = struct_member,
               repoID = repoID)

    external_linkage(node)
    



