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
# Revision 1.14.2.3  2000/11/03 19:21:35  sll
# idltype.Declared now takes an extra argument.
#
# Revision 1.14.2.2  2000/10/12 15:37:49  sll
# Updated from omni3_1_develop.
#
# Revision 1.15.2.2  2000/08/21 11:35:06  djs
# Lots of tidying
#
# Revision 1.15.2.1  2000/08/04 17:10:28  dpg1
# Long long support
#
# Revision 1.15  2000/07/13 15:26:00  dpg1
# Merge from omni3_develop for 3.0 release.
#
# Revision 1.12.2.10  2000/06/27 16:15:09  sll
# New classes: _CORBA_String_element, _CORBA_ObjRef_Element,
# _CORBA_ObjRef_tcDesc_arg to support assignment to an element of a
# sequence of string and a sequence of object reference.
#
# Revision 1.12.2.9  2000/06/26 16:23:26  djs
# Refactoring of configuration state mechanism.
#
# Revision 1.12.2.8  2000/06/05 13:03:04  djs
# Removed union member name clash (x & pd_x, pd__default, pd__d)
# Removed name clash when a sequence is called "pd_seq"
#
# Revision 1.12.2.7  2000/05/31 18:02:50  djs
# Better output indenting (and preprocessor directives now correctly output at
# the beginning of lines)
#
# Revision 1.12.2.6  2000/04/26 18:22:20  djs
# Rewrote type mapping code (now in types.py)
# Rewrote identifier handling code (now in id.py)
#
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
from omniidl_be.cxx import ast, cxx, output, util, id, types, config
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
                  "_0RL_buildDesc_cTypeCode",
                  "_0RL_buildDesc_clonglong",
                  "_0RL_buildDesc_cunsigned_plonglong"]:
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

    scopedName = id.Name(node.scopedName())
    fqname = scopedName.fullyQualify()
    guard_name = scopedName.guard()

    objref_name = scopedName.prefix("_objref_").fullyQualify()
    tc_name = scopedName.prefix("_tc_").fullyQualify()
    helper_name = scopedName.suffix("_Helper").fullyQualify()
    interface_type = types.Type(idltype.Declared(node,node.scopedName(),
                                                 idltype.tk_objref,0));
    objref_member = interface_type.objRefTemplate("tcDesc_arg")
    prefix = config.state['Private Prefix']

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
    
    scopedName = id.Name(node.scopedName())
    guard_name = scopedName.guard()
    fqname = scopedName.fullyQualify()
    prefix = config.state['Private Prefix']

    # <--- Check we have the necessary definitions already output
    required_symbols = []
    generated_symbols = [ prefix + "_buildDesc_c" + guard_name ]
    defineSymbols(generated_symbols)
    assertDefined(required_symbols)
    # <---
    
    stream.out(template.enum,
               guard_name = guard_name,
               private_prefix = config.state['Private Prefix'],
               fqname = fqname)

    finishingNode()

# -----------------------------------

def docast(type, decl, string):
    assert isinstance(type, types.Type)
    dims = type.dims()
    if decl:
        assert isinstance(decl, idlast.Declarator)
        decl_dims = decl.sizes()
        dims = decl_dims + dims
        
    tail_dims_string = ""
    if dims != []:
        tail_dims = dims[1:]
        tail_dims_string = cxx.dimsToString(tail_dims)
    
    d_type = type.deref()
    cast_to = d_type.base()
    if d_type.objref():
        cast_to = d_type.objRefTemplate("Member")
    elif d_type.sequence():
        cast_to = d_type.sequenceTemplate()
    elif d_type.typecode():
        cast_to = "CORBA::TypeCode_member"
        
    elif d_type.string():
        cast_to = "CORBA::String_member"
    cast_to = cast_to + "(*)" + tail_dims_string
    return "(const " + cast_to + ")(" + cast_to + ")" +\
           "(" + string + ")"


def prototype(decl, where, member = None):
    scopedName = id.Name(decl.scopedName())
    guard_name = "_c" + scopedName.guard()
    fqname = scopedName.fullyQualify()

    prefix = config.state['Private Prefix']

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
    type = types.Type(type)
    if decl.mainFile() and not(isRecursive(decl)) and not(type.typedef()):
        # types declared in the same file will be handled
        #   unless type is recursive -> forward declaration required
        #   unless type is an alias in this file to something in another file
        return

    if type.typedef():
        if type.type().decl().sizes() != []:
            visitArray(types.Type(type.type().decl().alias().aliasType()),
                       type.type().decl())
        else:
            type.type().decl().alias().aliasType().accept(self)
        return

    mem_name = None
    d_type = type.deref()
    if d_type.objref():
        mem_name = d_type.objRefTemplate("tcDesc_arg")
    
    # make an extern/ forward declaration
    if isRecursive(decl):
        forward(decl, mem_name)
        return
    external(d_type.type().decl(), mem_name)

def visitStringType(type):
    bound = type.bound()
    if bound != 0:
        prefix = config.state['Private Prefix']
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
    seqType = types.Type(type.seqType())
    d_seqType = seqType.deref()

    seqType_cname = mangler.canonTypeName(seqType.deref(keep_dims = 1))
    sequence_template = types.Type(type).sequenceTemplate()
    d_type = types.Type(type).deref()
    memberType_cname = mangler.canonTypeName(d_type)

    seqType_dims = seqType.dims()
    is_array = seqType_dims != []

    # check if the sequence is recursive
    is_recursive = isinstance(seqType.type(), idltype.Declared) and \
                   isRecursive(seqType.type().decl())

    seqType.type().accept(self)

    # something very strange happens here with strings and casting
    sequence_desc = "((" + sequence_template + "*)_desc->opq_seq)"
    thing = "(*" + sequence_desc + ")[_index]"
    if is_array:
        thing = docast(seqType, None, thing)

    elementDesc = output.StringStream()
    prefix = config.state['Private Prefix']
    # djr and jnw's "Super-Hacky Optimisation"
    if isinstance(d_seqType.type(), idltype.Base)   and \
       not(d_seqType.variable()) and \
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
               private_prefix = config.state['Private Prefix'])

def canonDims(d):
    canon = map(lambda x:"_a" + str(x), d)
    return string.join(canon, "")


# We want to treat anonymous arrays (inside structures) the same as typedef
# generated ones. However the AST.Declarator node does not have the type
# of the declarator, or a pointer to its declaration _unless_ its a typedef
# alias. So this is outside the normal tree walking pattern.
def visitArray(type, declarator):
    d_dims = declarator.sizes()
    d_scopedName = id.Name(declarator.scopedName())
    d_cname = mangler.canonTypeName(type, declarator)

    deref_type = type.deref()
    deref_kd_type = type.deref(keep_dims = 1)
    type_dims = type.dims()
    full_dims = d_dims + type_dims

    type.type().accept(self)

    # recursively ensure that dependent type info is available
    # (if other typedef dimensions are in a different file)
    if deref_kd_type.typedef():
        # must be an array with dimensions
        decl = deref_kd_type.type().decl()
        visitArray(types.Type(decl.alias().aliasType()), decl)

    fqname = d_scopedName.fullyQualify()
    tc_name = d_scopedName.prefix("_tc_").fullyQualify()
    guard_name = d_scopedName.guard()

    # problem with sequences as always
    # Probably should look into derefKeepDims here
    if deref_type.sequence() and type.typedef():
        alias = type.type().decl().alias()
        alias_cname = mangler.canonTypeName(types.Type(alias.aliasType()))
    else:
        alias_cname = mangler.canonTypeName(deref_type)
    alias_tyname = type.member()

    if type.typedef():
        alias_tyname = id.Name(type.type().scopedName()).fullyQualify()

    deref_kd_alias_tyname = deref_kd_type.base()
    deref_alias_tyname = deref_type.base()

    if deref_type.objref():
        objref_name = deref_type.objRefTemplate("Member")
        deref_alias_tyname = objref_name
        if type_dims == []:
            alias_tyname = objref_name

    # a multidimensional declarator will create several of
    # these functions (dimension by dimension)
    current_dims = []
    index = len(d_dims) - 1

    # if thing being aliased is also an array declarator, we need
    # to add its dimensions too.
    if type.typedef():
        current_dims = type.type().decl().sizes()
        alias_cname = mangler.canonTypeName(types.Type(type.type().decl().alias().aliasType()))

    prev_cname = canonDims(current_dims) + alias_cname
    prefix = config.state['Private Prefix']
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
    
    if deref_type.string():
        if type_dims == []:
            element_name = "CORBA::String_member"
    elif type.sequence():
        element_name = type.sequenceTemplate()
    elif deref_type.typecode():
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
    if deref_type.sequence():
        argtype = deref_type.sequenceTemplate()
    elif deref_type.typecode():
        argtype = "CORBA::TypeCode_member"
    elif deref_type.string():
        argtype = "CORBA::String_member"

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
        memberType = types.Type(m.memberType())
        if memberType.enum() and m.constrType():
            num_members = num_members + \
                          len(memberType.type().decl().enumerators())

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
                visitArray(types.Type(memberType), d)

    # build the case expression
    cases = output.StringStream()
    index = 0
    for m in node.members():
        memberType = types.Type(m.memberType())
        deref_memberType = memberType.deref(keep_dims = 1)
        member_dims = memberType.dims()
        for d in m.declarators():
            d_scopedName = id.Name(d.scopedName())
            d_name = d_scopedName.simple()
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

    scopedName = id.Name(node.scopedName())
    guard_name = scopedName.guard()
    fqname = scopedName.fullyQualify()
    prefix = config.state['Private Prefix']
    
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
    
    aliasType = types.Type(node.aliasType())
    deref_aliasType = aliasType.deref()
    type_dims = aliasType.dims()
    prefix = config.state['Private Prefix']

    if node.constrType():
        aliasType.type().decl().accept(self)

    # we don't need to recurse on the aliased type here because
    # the code generated is only used from the code generated from
    # and array declarator.
    # visitArray() would take care of it for us.
    # Don't do this:
    #   aliasType.accept(self)

    alias_cname = mangler.canonTypeName(aliasType)
    alias_tyname = aliasType.base()
    deref_alias_tyname = deref_aliasType.base()
    if deref_aliasType.objref():
        alias_tyname = aliasType.objRefTemplate("Member")
        deref_alias_tyname = deref_aliasType.objRefTemplate("Member")
    elif deref_aliasType.string():
        alias_tyname = "CORBA::String_member"

    for declarator in node.declarators():
        first_declarator = declarator == node.declarators()[0]
        
        decl_dims = declarator.sizes()
        full_dims = decl_dims + type_dims
        is_array = full_dims != []
        is_array_declarator = decl_dims != []
        scopedName = id.Name(declarator.scopedName())
        fqname = scopedName.fullyQualify()
        tc_name = scopedName.prefix("_tc_").fullyQualify()
        guard_name = scopedName.guard()

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
            if deref_aliasType.sequence():
                argtype = deref_aliasType.sequenceTemplate()
            if deref_aliasType.string():
                argtype = "CORBA::String_member"
            if deref_aliasType.typecode():
                argtype = "CORBA::TypeCode_member"
                
            assertDefined([ prefix + "_buildDesc" + decl_cname ])
            stream.out(template.typedef_array_decl_oper,
                       fqname = fqname,
                       decl_cname = decl_cname,
                       type = alias_tyname,
                       dtype = argtype,
                       tail_dims = tail_dims,
                       private_prefix = config.state['Private Prefix'],
                       tcname = tc_name,
                       guard_name = guard_name)

        # --- sequences
        if not(is_array_declarator) and aliasType.sequence():
            if first_declarator:
                deref_aliasType.type().accept(self)
            stream.out(template.typedef_sequence_oper,
                       fqname = fqname,
                       tcname = tc_name,
                       decl_cname = decl_cname,
                       private_prefix = config.state['Private Prefix'],
                       guard_name = guard_name)

    finishingNode()


def visitUnion(node):
    if not(node.mainFile()) and not(self.__override):
        return

    startingNode(node)
    
    scopedName = id.Name(node.scopedName())
    guard_name = scopedName.guard()
    fqname = scopedName.fullyQualify()
    switchType = types.Type(node.switchType())
    deref_switchType = switchType.deref()
    discrim_cname = mangler.canonTypeName(switchType)
    discrim_type = deref_switchType.base()

    isExhaustive = ast.exhaustiveMatch(switchType,ast.allCaseLabelValues(node))

    prefix = config.state['Private Prefix']

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
            
    switch = output.StringStream()
    if default_case:
        default_decl = default_case.declarator()
        default_type = types.Type(default_case.caseType())
        default_dims = default_type.dims()
        decl_dims = default_decl.sizes()
        full_dims = decl_dims + default_dims
    
        default_is_array = full_dims != []
        mem_cname = mangler.canonTypeName(default_type, default_decl)
        default_decl_name = id.Name(default_decl.scopedName())
        mem_name = default_decl_name.simple()
        thing = "_u->_pd_" + mem_name
        if default_is_array:
            thing = docast(default_type, default_decl, thing)

        required_symbols.append(prefix + "_buildDesc" + mem_cname)
        
        switch.out("""\
if( _u->_pd__default ) {
  @private_prefix@_buildDesc@mem_cname@(_newdesc, @thing@);
} else {""",
                   mem_cname = mem_cname,
                   private_prefix = prefix,
                   thing = thing)
        switch.inc_indent()

    # handle the main cases
    switch.out("""\
switch( _u->_pd__d ) {""")

    # deal with types
    for c in node.cases():
        caseType = types.Type(c.caseType())
        caseType.type().accept(self)
        
        declarator = c.declarator()
        d_sizes = declarator.sizes()
        is_array_declarator = d_sizes != []
        if is_array_declarator:
            visitArray(caseType, declarator)
                
        deref_caseType = caseType.deref()
        type_cname = mangler.canonTypeName(caseType, declarator)
        type_name = caseType.base()
        deref_type_name = deref_caseType.base()
        mem_name = id.Name(c.declarator().scopedName()).simple()
        case_dims = caseType.dims()
        full_dims = d_sizes + case_dims
        
        is_array = full_dims != []
        is_array_declarator = declarator.sizes() != []
        union_member = "_u->_pd_" + mem_name
        cast = union_member
        if is_array:
            cast = docast(caseType, declarator, cast)
            
        for l in c.labels():
            if l.default():
                continue

            label = switchType.literal(l.value())
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
    scopedName = id.Name(node.scopedName())
    guard_name = scopedName.guard()
    prefix = config.state['Private Prefix']

    symbol = prefix + "_buildDesc_c" + guard_name
    if not(isDefined(symbol)):
        stream.out("// forward declaration of interface")
        interface_type = types.Type(idltype.Declared(node, node.scopedName(),
                                          idltype.tk_objref,0))
        mem_name = interface_type.objRefTemplate("tcDesc_arg")

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
    
    scopedName = id.Name(node.scopedName())
    guard_name = scopedName.guard()
    fqname = scopedName.fullyQualify()
    prefix = config.state['Private Prefix']

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
               private_prefix = config.state['Private Prefix'])


    finishingNode()



