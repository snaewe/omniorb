# -*- python -*-
#                           Package   : omniidl
# main.py                   Created on: 1999/11/12
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
#   Produce the main dynamic skeleton definitions

# $Id$
# $Log$
# Revision 1.12  2000/01/20 18:26:44  djs
# Moved large C++ output strings into an external template file
#
# Revision 1.11  2000/01/19 11:23:48  djs
# *** empty log message ***
#
# Revision 1.10  2000/01/17 17:06:56  djs
# Better handling of recursive and constructed types
#
# Revision 1.9  2000/01/13 15:56:35  djs
# Factored out private identifier prefix rather than hard coding it all through
# the code.
#
# Revision 1.8  2000/01/13 14:16:24  djs
# Properly clears state between processing separate IDL input files
#
# Revision 1.7  2000/01/11 12:02:38  djs
# More tidying up
#
# Revision 1.6  2000/01/11 11:33:55  djs
# Tidied up
#
# Revision 1.5  2000/01/07 20:31:24  djs
# Regression tests in CVSROOT/testsuite now pass for
#   * no backend arguments
#   * tie templates
#   * flattened tie templates
#   * TypeCode and Any generation
#
# Revision 1.4  1999/12/24 18:16:39  djs
# Array handling and TypeCode building fixes (esp. across multiple files)
#
# Revision 1.3  1999/12/10 18:26:36  djs
# Moved most #ifdef buildDesc code into a separate module
# General tidying up
#
# Revision 1.2  1999/12/09 20:40:14  djs
# TypeCode and Any generation option performs identically to old compiler for
# all current test fragments.
#
# Revision 1.1  1999/11/12 17:18:07  djs
# Skeleton of dynamic skeleton code :)
#

"""Produce the main dynamic skeleton definitions"""
# similar to o2be_root::produce_dynskel in the old C++ BE

import string

from omniidl import idlast, idltype, idlutil
from omniidl.be.cxx import tyutil, util, name, config
from omniidl.be.cxx.skel import mangler
from omniidl.be.cxx.dynskel import bdesc, template

import main

self = main

def __init__(stream):
    self.stream = stream
    self.__names = {}
    self.__override = 0

    bdesc.__init__()

    return self

def defineName(name):
    self.__names[name] = 1

def alreadyDefined(name):
    return self.__names.has_key(name)

# ------------------------------------
# Control arrives here

def visitAST(node):
    for n in node.declarations():
        n.accept(self)

def visitModule(node):
    # consider reopening modules spanning files here
    if not(node.mainFile()):
        return
    
    for n in node.definitions():
        n.accept(self)

def visitInterface(node):
    if not(node.mainFile()) and not(self.__override):
        return

    bdesc.startingNode(node)

    for n in node.declarations():
        n.accept(self)

    scopedName = node.scopedName()
    env = name.Environment()
    fqname = env.nameToString(scopedName)
    guard_name = tyutil.guardName(scopedName)
    scopedName = map(tyutil.mapID, scopedName)

    objref_name = name.prefixName(scopedName, "_objref_")
    tc_name = name.prefixName(scopedName, "_tc_")
    helper_name = name.suffixName(scopedName, "_Helper")

    objref_member = "_CORBA_ObjRef_Member<" + objref_name + ", " +\
                    helper_name + ">"

    stream.out(template.interface,
               guard_name = guard_name,
               fqname = fqname, objref_member = objref_member,
               tc_name = tc_name, private_prefix = config.privatePrefix())

    bdesc.finishingNode()


def visitTypedef(node):
    if not(node.mainFile()) and not(self.__override):
        return

    bdesc.startingNode(node)
    
    aliasType = node.aliasType()
    deref_aliasType = tyutil.deref(aliasType)
    type_dims = tyutil.typeDims(aliasType)
    env = name.Environment()

    if node.constrType():
        aliasType.decl().accept(self)

    alias_cname = mangler.canonTypeName(aliasType)
    alias_tyname = env.principalID(aliasType)
    deref_alias_tyname = env.principalID(deref_aliasType)
    if tyutil.isObjRef(deref_aliasType):
        alias_tyname = tyutil.objRefTemplate(aliasType, "Member", env)
        deref_alias_tyname = tyutil.objRefTemplate(deref_aliasType, "Member", env)
    elif tyutil.isString(deref_aliasType):
        alias_tyname = "CORBA::String_member"


    bdesc.setStreamEnv(stream, env)

    # The old backend does something funny with output order
    # this helps recreate it
    first_is_array_decl = node.declarators()[0].sizes() != []
    if not(first_is_array_decl):
        if type_dims != []:
            node.accept(bdesc)
        
    for declarator in node.declarators():
        first_declarator = declarator == node.declarators()[0]
        
        decl_dims = declarator.sizes()
        full_dims = decl_dims + type_dims
        is_array = full_dims != []
        is_array_declarator = decl_dims != []
        scopedName = declarator.scopedName()
        fqname = env.nameToString(scopedName)
        tc_name = name.prefixName(scopedName, "_tc_")
        guard_name = tyutil.guardName(scopedName)

        decl_cname = mangler.canonTypeName(aliasType, declarator)

        if is_array_declarator:
            stream.out(template.typedef_array_decl_delete,
                       fqname = fqname, guard_name = guard_name,
                       private_prefix = config.privatePrefix())

            if first_declarator:
                node.accept(bdesc)
                pass
            stream.out(str(bdesc.array(aliasType, declarator)))

            dims_str   = map(str, full_dims)
            dims_index = map(lambda x:"[" + x + "]", dims_str)
            dims_tail_index = dims_index[1:]
            tail_dims = string.join(dims_tail_index, "")

            argtype = deref_alias_tyname
            if tyutil.isSequence(deref_aliasType):
                argtype = tyutil.sequenceTemplate(deref_aliasType, env)
                
            stream.out(template.typedef_array_decl_oper,
                       fqname = fqname,
                       decl_cname = decl_cname,
                       type = alias_tyname,
                       dtype = argtype,
                       tail_dims = tail_dims,
                       private_prefix = config.privatePrefix(),
                       tcname = tc_name,
                       guard_name = guard_name)

        # --- sequences
        if not(is_array_declarator) and tyutil.isSequence(aliasType):
            if first_declarator:
                stream.out(str(bdesc.sequence(deref_aliasType)))
            stream.out(template.typedef_sequence_oper,
                       fqname = fqname,
                       tcname = tc_name,
                       decl_cname = decl_cname,
                       private_prefix = config.privatePrefix(),
                       guard_name = guard_name)

    bdesc.finishingNode()


def visitEnum(node):
    if not(node.mainFile()) and not(self.__override):
        return

    bdesc.startingNode(node)
    
    scopedName = node.scopedName()
    guard_name = tyutil.guardName(scopedName)
    env = name.Environment()
    fqname = env.nameToString(scopedName)

    stream.out(template.enum,
               guard_name = guard_name,
               private_prefix = config.privatePrefix(),
               fqname = fqname)

    bdesc.finishingNode()

   
def visitStruct(node):
    if not(node.mainFile()) and not(self.__override):
        return

    bdesc.startingNode(node)

    scopedName = node.scopedName()
    guard_name = tyutil.guardName(scopedName)
    env = name.Environment()
    fqname = env.nameToString(scopedName)

    num_members = 0
    cases = util.StringStream()
    member_code = util.StringStream()

    for m in node.members():
        m.accept(self)

    member_desc = bdesc.member(node)    

    stream.out(template.struct,
               fqname = fqname, guard_name = guard_name, cases = str(cases),
               member_desc = str(member_desc),
               private_prefix = config.privatePrefix(),
               num_members = str(num_members))    

    bdesc.finishingNode()

    
def visitUnion(node):
    if not(node.mainFile()) and not(self.__override):
        return

    bdesc.startingNode(node)
    
    scopedName = node.scopedName()
    guard_name = tyutil.guardName(scopedName)
    env = name.Environment()
    fqname = env.nameToString(scopedName)
    switchType = node.switchType()
    deref_switchType = tyutil.deref(switchType)
    discrim_cname = mangler.canonTypeName(switchType)
    discrim_type = env.principalID(deref_switchType)

    allCaseValues = tyutil.allCases(node)
    isExhaustive = tyutil.exhaustiveMatch(switchType, allCaseValues)

    default_case = None
    for c in node.cases():
        for l in c.labels():
            if l.default():
                default_case = c
                break

    # constructed types
    if node.constrType():
        node.switchType().decl().accept(self)
    for n in node.cases():
        if n.constrType():
            n.caseType().decl().accept(self)
            
    switch = util.StringStream()
    if default_case:
        default_decl = default_case.declarator()
        default_type = default_case.caseType()
        default_dims = tyutil.typeDims(default_type)
        decl_dims = default_decl.sizes()
        full_dims = decl_dims + default_dims
    
        default_is_array = full_dims != []
        mem_cname = mangler.canonTypeName(default_type, default_decl)
        mem_name = tyutil.mapID(tyutil.name(default_decl.scopedName()))
        thing = "_u->pd_" + mem_name
        if default_is_array:
            thing = bdesc.docast(default_type, default_decl, thing)
        
        switch.out("""\
    if( _u->pd__default ) {
      @private_prefix@_buildDesc@mem_cname@(_newdesc, @thing@);
    } else {""",
                   mem_cname = mem_cname,
                   private_prefix = config.privatePrefix(),
                   thing = thing)
        switch.inc_indent()

    # handle the main cases
    switch.out("""\
      switch( _u->pd__d ) {""")


    for c in node.cases():
        caseType = c.caseType()
        declarator = c.declarator()
        deref_caseType = tyutil.deref(caseType)
        type_cname = mangler.canonTypeName(caseType, declarator)
        type_name = env.principalID(caseType)
        deref_type_name = env.principalID(deref_caseType)
        mem_name = tyutil.mapID(tyutil.name(c.declarator().scopedName()))
        case_dims = tyutil.typeDims(caseType)
        full_dims = declarator.sizes() + case_dims
        
        is_array = full_dims != []
        is_array_declarator = declarator.sizes() != []
        union_member = "_u->pd_" + mem_name
        cast = union_member
        if is_array:
            cast = bdesc.docast(caseType, declarator, cast)

        # handle cases which are themselves anonymous array
        # or sequence declarators
        if tyutil.isSequence(caseType):
            stream.out(str(bdesc.sequence(caseType)))
        # handle uses of sequences through typedefs but where the
        # actual declaration is in anothe file
        elif tyutil.isSequence(deref_caseType) and \
             not(caseType.decl().mainFile()):
            stream.out(str(bdesc.sequence(deref_caseType)))
        if is_array_declarator:
            stream.out(str(bdesc.array(caseType, declarator)))
        if tyutil.isObjRef(caseType):
            stream.out(str(bdesc.interface(caseType)))
        # FIXME: unify common code with bdesc/member#
        if tyutil.isStruct(caseType) or \
           tyutil.isUnion(caseType)  or \
           tyutil.isEnum(caseType):
            # only if not defined in this file
            if not(caseType.decl().mainFile()):
                stream.out(str(bdesc.external(caseType)))

        if tyutil.isString(caseType) and caseType.bound() != 0:
            stream.out(template.bdesc_string,
                       n = str(caseType.bound()),
                       private_prefix = config.privatePrefix())

            
        for l in c.labels():
            if l.default():
                continue
            # FIXME: same problem occurs in header/defs and skel/main and dynskel/bdesc
            if tyutil.isChar(switchType) and l.value() == '\0':
                label = "0000"
            else:
                label = tyutil.valueString(switchType, l.value(), env)
            switch.out("""\
      case @label@:
        @private_prefix@_buildDesc@type_cname@(_newdesc, @cast@);
        break;""", label = label, type_cname = type_cname, cast = cast,
                       private_prefix = config.privatePrefix())
            switch.dec_indent()

    if not(isExhaustive):
        switch.out("""\
        default: return 0;""")
    switch.dec_indent()
    switch.out("""\
      }""")
    if default_case:
        switch.out("""\
    }""")

    # FIXME: see above
    if tyutil.isStruct(switchType) or \
       tyutil.isUnion(switchType)  or \
       tyutil.isEnum(switchType):
        if not(switchType.decl().mainFile()):
            stream.out(str(bdesc.external(switchType)))
        
        sw_scopedName = switchType.decl().scopedName()
        sw_guard_name = tyutil.guardName(sw_scopedName)
        sw_fqname = env.nameToString(sw_scopedName)
        fn_name = config.privatePrefix() + "_buildDesc_c" + sw_guard_name
        
    stream.out(template.union_tcParser,
               guard_name = guard_name,
               discrim_cname = discrim_cname,
               discrim_type = discrim_type,
               switch = str(switch),
               private_prefix = config.privatePrefix(),
               fqname = fqname)

    
    stream.out(template.union,
               guard_name = guard_name, fqname = fqname,
               private_prefix = config.privatePrefix())
               

    bdesc.finishingNode()


def visitForward(node):
    pass
def visitConst(node):
    pass
def visitDeclarator(node):
    pass
def visitMember(node):
    memberType = node.memberType()
    if node.constrType():
        memberType.decl().accept(self)
        
def visitException(node):
    if not(node.mainFile()) and not(self.__override):
        return

    bdesc.startingNode(node)
    
    scopedName = node.scopedName()
    guard_name = tyutil.guardName(scopedName)
    env = name.Environment()
    fqname = env.nameToString(scopedName)

    for m in node.members():
        m.accept(self)

        memberType = m.memberType()

    stream.out(str(bdesc.member(node, modify_for_exception = 1)))

    stream.out(template.exception,
               guard_name = guard_name, fqname = fqname,
               private_prefix = config.privatePrefix())


    bdesc.finishingNode()



