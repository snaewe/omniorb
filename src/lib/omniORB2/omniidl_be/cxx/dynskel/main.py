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
# Revision 1.12.2.5  2000/04/05 10:58:36  djs
# Missing function declaration when a union has a switch type declared in
# another file
#
# Revision 1.12.2.4  2000/03/24 22:30:18  djs
# Major code restructuring:
#   Control flow is more recursive and obvious
#   Properly distinguishes between forward declarations and externs
#   Only outputs definitions once
#   Lots of assertions to check all is well
#
# Revision 1.12.2.3  2000/03/20 11:48:16  djs
# Better handling of unions whose switch types are declared externally
#
# Revision 1.12.2.2  2000/03/15 20:49:18  djs
# Problem with typedefs to sequences or array declarators defined externally
# and used within a local struct or union.
# Refactoring of this code is now required....
#
# Revision 1.12.2.1  2000/02/14 18:34:56  dpg1
# New omniidl merged in.
#
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
from omniidl_be.cxx import tyutil, util, name, config
from omniidl_be.cxx.skel import mangler
from omniidl_be.cxx.dynskel import template

import main

self = main

def __init__(stream):
    self.stream = stream
    self.__symbols = {}
    initSymbols()
    self.__nodes = []
    self.__override = 0

    return self

# ------------------------------------
# Keep track of the symbols defined and check that we don't reference
# any undefined ones (would cause compile/ link error)

# Add entries for all the built in types. These functions are defined
# in src/include/omniORB3/tcDescriptor.h

# Note in particular that omniidl3 would add in externs for CORBA::Object
# but not for any others of these. Seems slightly inconsistent.
def initSymbols():
    for name in [ "_0RL_buildDesc_cboolean",
                  "_0RL_buildDesc_coctet",
                  "_0RL_buildDesc_cchar",
                  "_0RL_buildDesc_cshort",
                  "_0RL_buildDesc_cunsigned_pshort",
                  "_0RL_buildDesc_clong",
                  "_0RL_buildDesc_cunsigned_plong",
                  "_0RL_buildDesc_cfloat",
                  "_0RL_buildDesc_cdouble",
                  "_0RL_buildDesc_cany",
                  "_0RL_buildDesc_cstring",
                  "_0RL_buildDesc_cCORBA_mObject",
                  "_0RL_buildDesc_cTypeCode" ]:
        defineSymbol(name)

def defineSymbol(name):
    self.__symbols[name] = 1

def defineSymbols(symlist):
    for symbol in symlist:
        defineSymbol(symbol)

def isDefined(name):
    return self.__symbols.has_key(name)

def assertDefined(symlist):
    for symbol in symlist:
        if not(isDefined(symbol)):
            raise RuntimeError("Symbol (" + symbol + ") should have been " +\
                               "defined at this point in the output")

# ------------------------------------
# Keep track of our position in the AST to enable us to spot recursive
# AST structures
def startingNode(node):
    self.__nodes.append(node)
def finishingNode():
    assert (self.__nodes != [])
    self.__nodes = self.__nodes[0:len(self.__nodes)-1]
def isRecursive(node):
    return node in self.__nodes

# ------------------------------------
# Control arrives here

def visitAST(node):
    for n in node.declarations():
        n.accept(self)

# ------------------------------------

def visitModule(node):
    # consider reopening modules spanning files here?
    if not(node.mainFile()):
        return
    
    for n in node.definitions():
        n.accept(self)

# -----------------------------------

def visitInterface(node):
    if not(node.mainFile()) and not(self.__override):
        return

    startingNode(node)

    for n in node.declarations():
        n.accept(self)

    scopedName = node.scopedName()
    global_env = name.Environment()
    fqname = global_env.nameToString(scopedName)
    guard_name = tyutil.guardName(scopedName)
    scopedName = map(tyutil.mapID, scopedName)

    objref_name = name.prefixName(scopedName, "_objref_")
    tc_name = name.prefixName(scopedName, "_tc_")
    helper_name = name.suffixName(scopedName, "_Helper")

    objref_member = "_CORBA_ObjRef_Member<" + objref_name + ", " +\
                    helper_name + ">"
    prefix = config.privatePrefix()

    # <--- Check we have the necessary definitions already output
    required_symbols = [ prefix + "_tcParser_setObjectPtr_" + guard_name,
                         prefix + "_tcParser_getObjectPtr_" + guard_name,
                         prefix + "_buildDesc_c" + guard_name ]
    generated_symbols = required_symbols +\
                        [ prefix + "_delete_" + guard_name ]
    defineSymbols(generated_symbols)
    assertDefined(required_symbols)
    # <---
    
    stream.out(template.interface,
               guard_name = guard_name,
               fqname = fqname, objref_member = objref_member,
               tc_name = tc_name, private_prefix = prefix)

    finishingNode()

# -----------------------------------

def visitEnum(node):
    if not(node.mainFile()) and not(self.__override):
        return

    startingNode(node)
    
    scopedName = node.scopedName()
    guard_name = tyutil.guardName(scopedName)
    global_env = name.Environment()
    fqname = global_env.nameToString(scopedName)
    prefix = config.privatePrefix()

    # <--- Check we have the necessary definitions already output
    required_symbols = []
    generated_symbols = [ prefix + "_buildDesc_c" + guard_name ]
    defineSymbols(generated_symbols)
    assertDefined(required_symbols)
    # <---
    
    stream.out(template.enum,
               guard_name = guard_name,
               private_prefix = config.privatePrefix(),
               fqname = fqname)

    finishingNode()

# -----------------------------------

def docast(type, decl, string):
    assert isinstance(type, idltype.Type)
    global_env = name.Environment()
    dims = tyutil.typeDims(type)
    if decl:
        assert isinstance(decl, idlast.Declarator)
        decl_dims = decl.sizes()
        dims = decl_dims + dims
        
    tail_dims_string = ""
    if dims != []:
        tail_dims = dims[1:]
        tail_dims_string = tyutil.dimsToString(tail_dims)
    
    deref_type = tyutil.deref(type)
    cast_to = global_env.principalID(deref_type)
    if tyutil.isObjRef(deref_type):
        cast_to = tyutil.objRefTemplate(deref_type, "Member", global_env)
    elif tyutil.isSequence(deref_type):
        cast_to = tyutil.sequenceTemplate(deref_type, global_env)
    elif tyutil.isTypeCode(deref_type):
        cast_to = "CORBA::TypeCode_member"
    cast_to = cast_to + "(*)" + tail_dims_string
    return "(const " + cast_to + ")(" + cast_to + ")" +\
           "(" + string + ")"


def prototype(decl, where, member = None):
    scopedName = decl.scopedName()
    guard_name = "_c" + tyutil.guardName(scopedName)
    global_env = name.Environment()
    fqname = global_env.nameToString(scopedName)

    prefix = config.privatePrefix()

    # <---
    generated_symbol = prefix + "_buildDesc" + guard_name
    if isDefined(generated_symbol):
        return
    defineSymbols([generated_symbol])
    # <---

    if member == None:
        member = fqname
    
    stream.out(template.builddesc_extern,
               where = where,
               cname = guard_name,
               private_prefix = prefix,
               name = member)

def external(decl, member = None):
    prototype(decl, "extern", member)
def forward(decl, member = None):
    prototype(decl, "", member)

# We walk over types when we encounter the _use_ of a type in an AST node.
# If a type is referenced then we must output code for it, even if the
# defining node is in another source file.
# (normally we skip declarations make in another file)
def visitBaseType(type):
    pass
def visitWStringType(type):
    pass
def visitFixedType(type):
    pass

def visitDeclaredType(type):
    decl = type.decl()
    if decl.mainFile() and not(isRecursive(decl)) and not(tyutil.isTypedef(type)):
        # types declared in the same file will be handled
        #   unless type is recursive -> forward declaration required
        #   unless type is an alias in this file to something in another file
        return
    if tyutil.isTypedef(type):
        if type.decl().sizes() != []:
            visitArray(type.decl().alias().aliasType(), type.decl())
        else:
            type.decl().alias().aliasType().accept(self)
        return

    mem_name = None
    deref_type = tyutil.deref(type)
    if tyutil.isObjRef(deref_type):
        global_env = name.Environment()
        mem_name = tyutil.objRefTemplate(deref_type, "Member", global_env)
    
    # make an extern/ forward declaration
    if isRecursive(decl):
        forward(decl, mem_name)
        return
    external(deref_type.decl(), mem_name)

def visitStringType(type):
    bound = type.bound()
    if bound != 0:
        prefix = config.privatePrefix()
        n = str(bound)
        # <---
        generated_symbol = prefix + "_buildDesc_c" + n + "string"
        if isDefined(generated_symbol):
            return
        required_symbols  = [ prefix + "_buildDesc_c" + "string" ]
        defineSymbols([ generated_symbol ])
        assertDefined(required_symbols)
        # <---
        stream.out(template.bdesc_string, n = n,
                   private_prefix = prefix)
        

def visitSequenceType(type):
    global_env = name.Environment()
    seqType = type.seqType()
    deref_seqType = tyutil.deref(seqType)

    seqType_cname = mangler.canonTypeName(tyutil.derefKeepDims(seqType))
    sequence_template = tyutil.sequenceTemplate(type, global_env)
    deref_type = tyutil.deref(type)
    memberType_cname = mangler.canonTypeName(deref_type)

    seqType_dims = tyutil.typeDims(seqType)
    is_array = seqType_dims != []

    # check if the sequence is recursive
    is_recursive = isinstance(seqType, idltype.Declared) and \
                   isRecursive(seqType.decl())

    seqType.accept(self)

    # something very strange happens here with strings and casting
    sequence_desc = "((" + sequence_template + "*)_desc->opq_seq)"
    thing = "(*" + sequence_desc + ")[_index]"
    if is_array:
        thing = docast(seqType, None, thing)

    elementDesc = util.StringStream()
    prefix = config.privatePrefix()
    # djr and jnw's "Super-Hacky Optimisation"
    if isinstance(deref_seqType, idltype.Base)   and \
       not(tyutil.isVariableType(deref_seqType)) and \
       not(is_array):
        elementDesc.out(template.sequence_elementDesc_contiguous,
                        sequence = sequence_desc)
    else:
        # <---
        required_symbols = [ prefix + "_buildDesc" + seqType_cname ]
        assertDefined(required_symbols)
        # <---
        elementDesc.out(template.sequence_elementDesc_noncontiguous,
                        private_prefix = prefix,
                        thing_cname = seqType_cname,
                        thing = thing)

    # <---
    cname = memberType_cname
    # this is the #ifdef guard
    generated_symbol = prefix + "_tcParser_buildDesc" + cname
    if isDefined(generated_symbol):
        return
    generated_symbols = [ generated_symbol,
                          prefix + "_buildDesc" + cname,
                          prefix + "_tcParser_setElementCount" + cname,
                          prefix + "_tcParser_getElementCount" + cname,
                          prefix + "_tcParser_getElementDesc" + cname ]
    defineSymbols(generated_symbols)
    # <---
    stream.out(template.anon_sequence,
               cname = memberType_cname, thing_cname = seqType_cname,
               sequence_template = sequence_template,
               elementDesc = str(elementDesc),
               private_prefix = config.privatePrefix())

def canonDims(d):
    canon = map(lambda x:"_a" + str(x), d)
    return string.join(canon, "")


# We want to treat anonymous arrays (inside structures) the same as typedef
# generated ones. However the AST.Declarator node does not have the type
# of the declarator, or a pointer to its declaration _unless_ its a typedef
# alias. So this is outside the normal tree walking pattern.
def visitArray(type, declarator):
    d_dims = declarator.sizes()
    d_scopedName = declarator.scopedName()
    d_cname = mangler.canonTypeName(type, declarator)

    deref_type = tyutil.deref(type)
    deref_kd_type = tyutil.derefKeepDims(type)
    type_dims = tyutil.typeDims(type)
    full_dims = d_dims + type_dims

    type.accept(self)

    # recursively ensure that dependent type info is available
    # (if other typedef dimensions are in a different file)
    if tyutil.isTypedef(deref_kd_type):
        # must be an array with dimensions
        decl = deref_kd_type.decl()
        visitArray(decl.alias().aliasType(), decl)

    global_env = name.Environment()
    fqname = global_env.nameToString(d_scopedName)
    tc_name = name.prefixName(d_scopedName, "_tc_")
    guard_name = tyutil.guardName(d_scopedName)

    # problem with sequences as always
    # Probably should look into derefKeepDims here
    if tyutil.isSequence(deref_type) and tyutil.isTypedef(type):
        alias = type.decl().alias()
        alias_cname = mangler.canonTypeName(alias.aliasType())
    else:
        alias_cname = mangler.canonTypeName(deref_type)
    alias_tyname = global_env.principalID(type)
    deref_kd_alias_tyname = global_env.principalID(deref_kd_type)
    deref_alias_tyname = global_env.principalID(deref_type)

    if tyutil.isObjRef(deref_type):
        objref_name = tyutil.objRefTemplate(deref_type, "Member", global_env)
        deref_alias_tyname = objref_name
        if type_dims == []:
            alias_tyname = objref_name

    # a multidimensional declarator will create several of
    # these functions (dimension by dimension)
    current_dims = []
    index = len(d_dims) - 1

    # if thing being aliased is also an array declarator, we need
    # to add its dimensions too.
    if tyutil.isTypedef(type):
        current_dims = type.decl().sizes()
        alias_cname = mangler.canonTypeName(type.decl().alias().aliasType())

    prev_cname = canonDims(current_dims) + alias_cname
    prefix = config.privatePrefix()
    builddesc_nonarray_str = prefix + "_buildDesc" + alias_cname +\
                             "(_desc, " + prefix + "_tmp);"

    required_symbols = [ prefix + "_buildDesc" + alias_cname ]
    assertDefined(required_symbols)

    def builddesc_array(cname, prefix = prefix):
        return """\
_desc.p_array.getElementDesc = """ + prefix + """_tcParser_getElementDesc""" + cname + """;
_desc.p_array.opq_array = &""" + prefix + """_tmp;"""

    if type_dims == []:
        builddesc_str = builddesc_nonarray_str
    else:
        builddesc_str = builddesc_array(prev_cname)

    element_name = alias_tyname
    
    if tyutil.isString(deref_type):
        if type_dims == []:
            element_name = "CORBA::String_member"
    elif tyutil.isSequence(type):
        element_name = tyutil.sequenceTemplate(type, global_env)
    elif tyutil.isTypeCode(deref_type):
        if type_dims == []:
            element_name = "CORBA::TypeCode_member"

    element_dims = []
    while index >= 0:

        first_iteration = element_dims == []
        
        element_dims = [d_dims[index]] + element_dims
        current_dims = [d_dims[index]] + current_dims
        index = index - 1

        new_cname = canonDims(element_dims[1:]) + prev_cname
        
        builddesc_str = builddesc_array(new_cname)
        # first iteration, check for special case
        if first_iteration:
            if type_dims == []:
                builddesc_str = builddesc_nonarray_str

        element_tail_dims = element_dims[1:]
        dims_index = map(lambda x:"[" + str(x) + "]", element_dims)
        dims_tail_index = dims_index[1:]
        this_cname = canonDims(current_dims) + alias_cname

        generated_symbol = prefix +"_tcParser_getElementDesc"+ this_cname
        if not(isDefined(generated_symbol)):
            defineSymbols([ generated_symbol ])
        
            stream.out(template.getdesc_array,
                       this_cname = this_cname,
                       type = element_name,
                       tail_dims = string.join(dims_tail_index, ""),
                       builddesc = builddesc_str,
                       index_string = string.join(dims_index, ""),
                       private_prefix = prefix)
            
    dims_str   = map(str, full_dims)
    dims_index = map(lambda x:"[" + x + "]", dims_str)
    dims_tail_index = dims_index[1:]
    tail_dims = string.join(dims_tail_index, "")

    argtype = deref_alias_tyname
    if tyutil.isSequence(deref_type):
        argtype = tyutil.sequenceTemplate(deref_type, global_env)
    elif tyutil.isTypeCode(deref_type):
        argtype = "CORBA::TypeCode_member"

    required_symbols = [ prefix + "_tcParser_getElementDesc" + d_cname ]
    generated_symbol = prefix + "_buildDesc" + d_cname
    if not(isDefined(generated_symbol)):
        defineSymbols([ generated_symbol ])
        assertDefined(required_symbols)
    
        stream.out(template.builddesc_array,
                   decl_cname = d_cname,
                   tail_dims = tail_dims,
                   dtype = argtype,
                   type = alias_tyname,
                   private_prefix = prefix)


# <-- Code to deal with normal members
def visitMembers(node, stream, fqname, guard_name, prefix, static = ""):
        
    # total up the number of members (enums are a scoping special case)
    num_members = 0
    for m in node.members():
        memberType = m.memberType()
        if tyutil.isEnum(memberType) and m.constrType():
            num_members = num_members + \
                          len(memberType.decl().enumerators())

        for d in m.declarators():
            num_members = num_members + 1

    # deal with types 
    for m in node.members():
        memberType = m.memberType()
        memberType.accept(self)
        
        # needs to deal with array declarators....
        for d in m.declarators():
            d_sizes = d.sizes()
            is_array_declarator = d_sizes != []
            if is_array_declarator:
                visitArray(memberType, d)

    # build the case expression
    cases = util.StringStream()
    index = 0
    for m in node.members():
        memberType = m.memberType()
        deref_memberType = tyutil.derefKeepDims(memberType)
        member_dims = tyutil.typeDims(memberType)
        for d in m.declarators():
            d_scopedName = d.scopedName()
            d_name = tyutil.mapID(tyutil.name(d_scopedName))
            d_cname = mangler.canonTypeName(deref_memberType, d)
            d_dims = d.sizes()
            full_dims = d_dims + member_dims
            is_array = full_dims != []
            is_array_declarator = d_dims != []
            
            thing = "((" + fqname + "*)_desc->opq_struct)->" +\
                    d_name
            if is_array:
                thing = docast(memberType, d, thing)

            assertDefined([ prefix + "_buildDesc" + d_cname ])
            cases.out("""\
case @n@:
  @private_prefix@_buildDesc@cname@(_newdesc, @thing@);
  return 1;""",
                      n = str(index), cname = d_cname,
                      private_prefix = prefix,
                      thing = thing)
            index = index + 1

    stream.out("""\
@static@ CORBA::Boolean
@private_prefix@_tcParser_getMemberDesc_@guard_name@(tcStructDesc *_desc, CORBA::ULong _index, tcDescriptor &_newdesc){
  switch (_index) {
  @cases@
  default:
    return 0;
  };
}
@static@ CORBA::ULong
""", cases = str(cases), private_prefix = prefix, static = static,
               guard_name = guard_name)
       
    defineSymbols([ prefix + "_tcParser_getMemberDesc_" + guard_name ])

    # <---
    required_symbols = [ prefix + "_tcParser_getMemberDesc_" + guard_name ]
    generated_symbols = [ prefix + "_tcParser_getMemberCount_" +\
                          guard_name,
                          prefix + "_buildDesc_c" + guard_name]
    assertDefined(required_symbols)
    defineSymbols(generated_symbols)
    # <---
                             
    stream.out(template.builddesc_member,
               guard_name = guard_name,
               fqname = fqname,
               num_members = str(num_members),
               private_prefix = prefix,
               cases = str(cases))
    
    return



def visitStruct(node):
    if not(node.mainFile()) and not(self.__override):
        return

    startingNode(node)

    scopedName = node.scopedName()
    guard_name = tyutil.guardName(scopedName)
    global_env = name.Environment()
    fqname = global_env.nameToString(scopedName)
    prefix = config.privatePrefix()
    
    # if it's recursive, stick in a forward declaration
    if (node.recursive()):
        stream.out("// forward declaration because struct is recursive")
        forward(node)

    # output code for constructed members (eg nested structs)
    for m in node.members():
        memberType = m.memberType()
        if m.constrType():
            memberType.decl().accept(self)

    def member_desc(node = node, stream = stream, fqname = fqname,
                    guard_name = guard_name, prefix = prefix):
        visitMembers(node, stream, fqname, guard_name, prefix,
                     static = "static")
        
    stream.out(template.struct,
               fqname = fqname, guard_name = guard_name,
               member_desc = member_desc,
               private_prefix = prefix)

    # <---
    required_symbols = [ prefix + "_buildDesc_c" + guard_name ]
    generated_symbols = [ prefix + "_delete_" + guard_name ]
    assertDefined(required_symbols)
    defineSymbols(generated_symbols)
    # <---
    
    finishingNode()

    
def visitTypedef(node):
    if not(node.mainFile()) and not(self.__override):
        return

    startingNode(node)
    
    aliasType = node.aliasType()
    deref_aliasType = tyutil.deref(aliasType)
    type_dims = tyutil.typeDims(aliasType)
    global_env = name.Environment()
    prefix = config.privatePrefix()

    if node.constrType():
        aliasType.decl().accept(self)

    # we don't need to recurse on the aliased type here because
    # the code generated is only used from the code generated from
    # and array declarator.
    # visitArray() would take care of it for us.
    # Don't do this:
    #   aliasType.accept(self)

    alias_cname = mangler.canonTypeName(aliasType)
    alias_tyname = global_env.principalID(aliasType)
    deref_alias_tyname = global_env.principalID(deref_aliasType)
    if tyutil.isObjRef(deref_aliasType):
        alias_tyname = tyutil.objRefTemplate(aliasType, "Member",
                                             global_env)
        deref_alias_tyname = tyutil.objRefTemplate(deref_aliasType, "Member",
                                                   global_env)
    elif tyutil.isString(deref_aliasType):
        alias_tyname = "CORBA::String_member"

    for declarator in node.declarators():
        first_declarator = declarator == node.declarators()[0]
        
        decl_dims = declarator.sizes()
        full_dims = decl_dims + type_dims
        is_array = full_dims != []
        is_array_declarator = decl_dims != []
        scopedName = declarator.scopedName()
        fqname = global_env.nameToString(scopedName)
        tc_name = name.prefixName(scopedName, "_tc_")
        guard_name = tyutil.guardName(scopedName)

        decl_cname = mangler.canonTypeName(aliasType, declarator)

        if is_array_declarator:
            defineSymbols([ prefix + "_delete_" + guard_name ])
            stream.out(template.typedef_array_decl_delete,
                       fqname = fqname, guard_name = guard_name,
                       private_prefix = prefix)

            visitArray(aliasType, declarator)

            dims_str   = map(str, full_dims)
            dims_index = map(lambda x:"[" + x + "]", dims_str)
            dims_tail_index = dims_index[1:]
            tail_dims = string.join(dims_tail_index, "")

            argtype = deref_alias_tyname
            if tyutil.isSequence(deref_aliasType):
                argtype = tyutil.sequenceTemplate(deref_aliasType, global_env)
                
            assertDefined([ prefix + "_buildDesc" + decl_cname ])
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
                deref_aliasType.accept(self)
            stream.out(template.typedef_sequence_oper,
                       fqname = fqname,
                       tcname = tc_name,
                       decl_cname = decl_cname,
                       private_prefix = config.privatePrefix(),
                       guard_name = guard_name)

    finishingNode()


def visitUnion(node):
    if not(node.mainFile()) and not(self.__override):
        return

    startingNode(node)
    
    scopedName = node.scopedName()
    guard_name = tyutil.guardName(scopedName)
    global_env = name.Environment()
    fqname = global_env.nameToString(scopedName)
    switchType = node.switchType()
    deref_switchType = tyutil.deref(switchType)
    discrim_cname = mangler.canonTypeName(switchType)
    discrim_type = global_env.principalID(deref_switchType)

    allCaseValues = tyutil.allCases(node)
    isExhaustive = tyutil.exhaustiveMatch(switchType, allCaseValues)

    prefix = config.privatePrefix()

    # grab the default case if it exists
    default_case = None
    for c in node.cases():
        for l in c.labels():
            if l.default():
                default_case = c
                break

    # if it's recursive, stick in a forward declaration
    if (node.recursive()):
        stream.out("// forward declaration because union is recursive")
        forward(node)

    # this may need an extern
    node.switchType().accept(self)
    
    # constructed types
    if node.constrType():
        node.switchType().decl().accept(self)
    for n in node.cases():
        if n.constrType():
            n.caseType().decl().accept(self)

    required_symbols = []
            
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
            thing = docast(default_type, default_decl, thing)

        required_symbols.append(prefix + "_buildDesc" + mem_cname)
        
        switch.out("""\
    if( _u->pd__default ) {
      @private_prefix@_buildDesc@mem_cname@(_newdesc, @thing@);
    } else {""",
                   mem_cname = mem_cname,
                   private_prefix = prefix,
                   thing = thing)
        switch.inc_indent()

    # handle the main cases
    switch.out("""\
      switch( _u->pd__d ) {""")

    # deal with types
    for c in node.cases():
        caseType = c.caseType()
        caseType.accept(self)
        
        declarator = c.declarator()
        d_sizes = declarator.sizes()
        is_array_declarator = d_sizes != []
        if is_array_declarator:
            visitArray(caseType, declarator)
                
        deref_caseType = tyutil.deref(caseType)
        type_cname = mangler.canonTypeName(caseType, declarator)
        type_name = global_env.principalID(caseType)
        deref_type_name = global_env.principalID(deref_caseType)
        mem_name = tyutil.mapID(tyutil.name(c.declarator().scopedName()))
        case_dims = tyutil.typeDims(caseType)
        full_dims = d_sizes + case_dims
        
        is_array = full_dims != []
        is_array_declarator = declarator.sizes() != []
        union_member = "_u->pd_" + mem_name
        cast = union_member
        if is_array:
            cast = docast(caseType, declarator, cast)
            
        for l in c.labels():
            if l.default():
                continue
            # FIXME: same problem occurs in header/defs and
            # skel/main and dynskel/bdesc
            if tyutil.isChar(switchType) and l.value() == '\0':
                label = "0000"
            else:
                label = tyutil.valueString(switchType, l.value(), global_env)
            required_symbols.append(prefix + "_buildDesc" + type_cname)
            switch.out("""\
      case @label@:
        @private_prefix@_buildDesc@type_cname@(_newdesc, @cast@);
        break;""", label = label, type_cname = type_cname, cast = cast,
                       private_prefix = prefix)
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

    assertDefined(required_symbols) # needed by the switch

    stream.out(template.union_tcParser,
               guard_name = guard_name,
               discrim_cname = discrim_cname,
               discrim_type = discrim_type,
               switch = str(switch),
               private_prefix = prefix,
               fqname = fqname)

    generated_symbols = [ prefix + "_buildDesc_c" + guard_name,
                          prefix + "_delete_" + guard_name ]
    defineSymbols(generated_symbols)
    
    stream.out(template.union,
               guard_name = guard_name, fqname = fqname,
               private_prefix = prefix)
               

    finishingNode()


def visitForward(node):
    if not(node.mainFile()) and not(self.__override):
        return
    scopedName = node.scopedName()
    guard_name = tyutil.guardName(scopedName)
    global_env = name.Environment()
    prefix = config.privatePrefix()

    symbol = prefix + "_buildDesc_c" + guard_name
    if not(isDefined(symbol)):
        stream.out("// forward declaration of interface")
        interface_type = idltype.Declared(node, node.scopedName(),
                                          idltype.tk_objref)
        mem_name = tyutil.objRefTemplate(interface_type, "Member", global_env)

        forward(node, mem_name)


def visitConst(node):
    pass
def visitDeclarator(node):
    pass
def visitMember(node):
    pass

def visitException(node):
    if not(node.mainFile()) and not(self.__override):
        return

    startingNode(node)
    
    scopedName = node.scopedName()
    guard_name = tyutil.guardName(scopedName)
    global_env = name.Environment()
    fqname = global_env.nameToString(scopedName)
    prefix = config.privatePrefix()

    for m in node.members():
        memberType = m.memberType()
        if m.constrType():
            memberType.decl().accept(self)


    visitMembers(node, stream, fqname, guard_name, prefix)
    
    required_symbols = [ prefix + "_buildDesc_c" + guard_name ]
    generated_symbols = [ prefix + "_delete_" + guard_name ]
    assertDefined(required_symbols)
    defineSymbols(generated_symbols)

    stream.out(template.exception,
               guard_name = guard_name, fqname = fqname,
               private_prefix = config.privatePrefix())


    finishingNode()



