# -*- python -*-
#                           Package   : omniidl
# bdesc.py                  Created on: 1999/12/9
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
#   Produce the #ifdef ...buildDesc... blocks for types

# $Id$
# $Log$
# Revision 1.6  2000/01/11 11:33:54  djs
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
# Revision 1.3  1999/12/16 16:09:30  djs
# TypeCode fixes
# Enum scope special case fixed
#
# Revision 1.2  1999/12/14 17:38:19  djs
# Fixed anonymous sequences of sequences bug
#
# Revision 1.1  1999/12/10 18:26:36  djs
# Moved most #ifdef buildDesc code into a separate module
# General tidying up
#
#

"""Produce the #ifdef ...buildDesc... blocks for types"""

# Even though the old backend wraps each function with #ifdef, define, endif
# guards it will still output functions and extern declarations multiple times.
# In future it might be better (more readable here) to either:
#   * always output everything
#       The #define guards nullify repeats, compiler's dead code elimination
#       should be able to eliminate the unnecessary fns
#   * only output functions as we need them, and add them to an environment
#     to prevent silly repeats.
#
# The second would probably be faster (hashtable lookup overhead, but saves
# a lot of extra computation and output) and would result in more readable
# generated code.


import string

from omniidl import idlast, idltype, idlutil

from omniidl.be.cxx import tyutil, util, name, config

from omniidl.be.cxx.skel import mangler

import bdesc

self = bdesc

self.__buildDesc = {}


# Used to limit recursion on typedef chains when they point to external
# declarations. Only recurse if haven't seen that declarator node before.
# If the declarator is defined in the same file, it has necessarily already
# been fully considered.
self.__seenDeclarators = {}

# We need to be able to detect recursive types so keep track of the current
# node here
self.__currentNode = None

def __init__(s, env):
    self.stream = s
    self.__environment = env

def canonDims(d):
    canon = map(lambda x:"_a" + str(x), d)
    return string.join(canon, "")


def write_array_desc(where, aliasType, dims):
    assert isinstance(aliasType, idltype.Type)

    env = name.Environment()

    type_dims = tyutil.typeDims(aliasType)
    type_is_array = type_dims != []

    full_dims = dims + type_dims
    tail_dims = tyutil.dimsToString(full_dims[1:])

    cname = mangler.canonTypeName(deref_aliasType)





    where.out("""\
#ifndef __0RL_tcParser_getElementDesc@this_cname@__
#define __0RL_tcParser_getElementDesc@this_cname@__
static CORBA::Boolean
_0RL_tcParser_getElementDesc@this_cname@(tcArrayDesc* _adesc, CORBA::ULong _index, tcDescriptor &_desc)
{
  @type@ (&_0RL_tmp)@tail_dims@ = (*((@type@(*)@index_string@)_adesc->opq_array))[_index];
  @builddesc@
  return 1;
}
#endif
""",
                   this_cname = this_cname,
                   type = element_name,
                   tail_dims = string.join(dims_tail_index, ""),
                   builddesc = builddesc_str,
                   index_string = string.join(dims_index, ""))

    # need to build the tcParser_getElementDesc function
    # If we amount to a single dimensional array, then we do
    # an array index

    
    if tyutil.isSequence(deref_aliasType) and \
       tyutil.isTypedef(aliasType):
        alias = aliasType.decl().alias()
        cname = mangler.canonTypeName(alias.aliasType())

    elif tyutil.isTypedef(aliasType):
        cname = mangler.canonTypeName(aliasType.decl().alias().aliasType())


    if tyutil.isObjRef(deref_aliasType):
        # this could easily now be broken:
        objref_name = tyutil.objRefTemplate(deref_aliasType, "Member", env)
        argtype = objref_name

    argtype = env.principalID(deref_aliasType)

    if tyutil.isSequence(deref_aliasType):
        argtype = tyutil.sequenceTemplate(deref_aliasType, env)
    elif tyutil.isTypeCode(deref_aliasType):
        argtype = "CORBA::TypeCode_member"

    where.out("""\
#ifndef __0RL_tcParser_buildDesc@decl_cname@__
#define __0RL_tcParser_buildDesc@decl_cname@__
static void
_0RL_buildDesc@cname@(tcDescriptor& _desc, const @dtype@(*_data)@tail_dims@)
{
  _desc.p_array.getElementDesc = _0RL_tcParser_getElementDesc@cname@;
  _desc.p_array.opq_array = (void*) _data;
}
#endif
""",
             cname = cname,
             tail_dims = tail_dims,
             dtype = argtype,
             type = alias_tyname)



def bstring(type):
    assert tyutil.isString(type)
    bound = type.bound()
    desc = util.StringStream()
    if bound != 0:
        desc.out("""\
#ifndef _0RL_buildDesc_c@n@string
#define _0RL_buildDesc_c@n@string _0RL_buildDesc_cstring
#endif""", n = str(bound))
    return desc
    

# Creates the #ifdefs for an array type. Also handles anonymous
# arrays via the declarator argument.
def array(aliasType, declarator = None, scopedName = ""):
    assert isinstance(aliasType, idltype.Type)
    if declarator:
        assert isinstance(declarator, idlast.Declarator)
        # should be an array declarator
        decl_dims = declarator.sizes()
        scopedName = declarator.scopedName()
        assert (decl_dims != [])
    else:
        decl_dims = []

        
    deref_aliasType = tyutil.deref(aliasType)
    type_dims = tyutil.typeDims(aliasType)
    full_dims = decl_dims + type_dims
    env = name.Environment()

    fqname = env.nameToString(scopedName)
    tc_name = name.prefixName(scopedName, "_tc_")
    guard_name = tyutil.guardName(scopedName)
    # problem with sequences as always
    # Probably should look into derefKeepDims here
    if tyutil.isSequence(deref_aliasType) and tyutil.isTypedef(aliasType):
        alias = aliasType.decl().alias()
        alias_cname = mangler.canonTypeName(alias.aliasType())
    else:
        alias_cname = mangler.canonTypeName(deref_aliasType)
    alias_tyname = env.principalID(aliasType)
    deref_keep_dims_alias_tyname = env.principalID(tyutil.derefKeepDims(aliasType))
    deref_alias_tyname = env.principalID(deref_aliasType)
    decl_cname = mangler.canonTypeName(aliasType, declarator)

    desc = util.StringStream()

    if tyutil.isObjRef(deref_aliasType):
        # this could easily now be broken:
        objref_name = tyutil.objRefTemplate(deref_aliasType, "Member", env)
        deref_alias_tyname = objref_name
        # only if a direct typedef to an objref. Musn't duplicate prototype
        if type_dims == []:
            alias_tyname = objref_name
        if type_dims == []:
            desc.out(str(interface(aliasType)))

    # if the thing being aliased is defined in another file we need to build
    # more than the normal set of getElementDesc macros
    defined_outside = isinstance(aliasType, idltype.Declared) and \
                      not(aliasType.decl().mainFile())

    if defined_outside:
        if full_dims != []:
            if tyutil.isTypedef(aliasType):
                # the old backend will only recurse once per AST declaration node
                # (even though each function block has #ifdef,define,endif guards)
                alias_declarator = aliasType.decl()
                if not(self.__seenDeclarators.has_key(alias_declarator)):
                    self.__seenDeclarators[alias_declarator] = 1
                    desc.out(str(array(alias_declarator.alias().aliasType(),
                                       alias_declarator)))
            elif tyutil.isStruct(aliasType) or \
                 tyutil.isUnion(aliasType)  or \
                 tyutil.isEnum(aliasType):
                desc.out(str(external(aliasType)))

    # for some reason, a multidimensional declarator will create
    # several of these functions, slowly adding dimensions
    current_dims = []
    index = len(decl_dims) - 1

    # if thing being aliased is also an array declarator, we need
    # to add its dimensions too.
    if tyutil.isTypedef(aliasType):
        current_dims = tyutil.typeDims(aliasType)
        current_dims = aliasType.decl().sizes()
        alias_cname = mangler.canonTypeName(aliasType.decl().alias().aliasType())
    prev_cname = canonDims(current_dims) + alias_cname
            
    builddesc_flat_str = "_0RL_buildDesc" + alias_cname +\
                         "(_desc, _0RL_tmp);"
    if type_dims == []:
        builddesc_str = builddesc_flat_str
    else:
        builddesc_str = """\
_desc.p_array.getElementDesc = _0RL_tcParser_getElementDesc""" + prev_cname + """;
_desc.p_array.opq_array = &_0RL_tmp;"""

    element_name = alias_tyname
    
    if tyutil.isString(deref_aliasType):
        if type_dims == []:
            element_name = deref_alias_tyname # String_member
    elif tyutil.isSequence(aliasType):
        element_name = tyutil.sequenceTemplate(aliasType, env)
    elif tyutil.isTypeCode(deref_aliasType):
        if type_dims == []:
            element_name = "CORBA::TypeCode_member"

    element_dims = []
    while index >= 0:

        first_iteration = element_dims == []
        
        element_dims = [decl_dims[index]] + element_dims
        current_dims = [decl_dims[index]] + current_dims
        index = index - 1

        new_cname = canonDims(element_dims[1:]) + prev_cname
        builddesc_str = """\
_desc.p_array.getElementDesc = _0RL_tcParser_getElementDesc""" + new_cname + """;
_desc.p_array.opq_array = &_0RL_tmp;"""

        # first iteration, check for special case
        if first_iteration:
            if type_dims == []:
                builddesc_str = builddesc_flat_str

        element_tail_dims = element_dims[1:]
        dims_index = map(lambda x:"[" + str(x) + "]", element_dims)
        dims_tail_index = dims_index[1:]
        this_cname = canonDims(current_dims) + alias_cname
        desc.out("""\
#ifndef __0RL_tcParser_getElementDesc@this_cname@__
#define __0RL_tcParser_getElementDesc@this_cname@__
static CORBA::Boolean
_0RL_tcParser_getElementDesc@this_cname@(tcArrayDesc* _adesc, CORBA::ULong _index, tcDescriptor &_desc)
{
  @type@ (&_0RL_tmp)@tail_dims@ = (*((@type@(*)@index_string@)_adesc->opq_array))[_index];
  @builddesc@
  return 1;
}
#endif
""",
                   this_cname = this_cname,
                   type = element_name,
                   tail_dims = string.join(dims_tail_index, ""),
                   builddesc = builddesc_str,
                   index_string = string.join(dims_index, ""))


    dims_str   = map(str, full_dims)
    dims_index = map(lambda x:"[" + x + "]", dims_str)
    dims_tail_index = dims_index[1:]
    tail_dims = string.join(dims_tail_index, "")

    argtype = deref_alias_tyname
    if tyutil.isSequence(deref_aliasType):
        argtype = tyutil.sequenceTemplate(deref_aliasType, env)
    elif tyutil.isTypeCode(deref_aliasType):
        argtype = "CORBA::TypeCode_member"
        
    desc.out("""\
#ifndef __0RL_tcParser_buildDesc@decl_cname@__
#define __0RL_tcParser_buildDesc@decl_cname@__
static void
_0RL_buildDesc@decl_cname@(tcDescriptor& _desc, const @dtype@(*_data)@tail_dims@)
{
  _desc.p_array.getElementDesc = _0RL_tcParser_getElementDesc@decl_cname@;
  _desc.p_array.opq_array = (void*) _data;
}
#endif
""",
               decl_cname = decl_cname,
               tail_dims = tail_dims,
               dtype = argtype,
               type = alias_tyname)

    return desc


# dont_do_tail is a hack, set to 1 if the type has a corresponding
# array declarator.
def docast(type, decl, string):
    assert isinstance(type, idltype.Type)
    env = name.Environment()
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
    cast_to = env.principalID(deref_type, fully_scope = 1)
    if tyutil.isObjRef(deref_type):
        cast_to = tyutil.objRefTemplate(deref_type, "Member", env)
    elif tyutil.isSequence(deref_type):
        cast_to = tyutil.sequenceTemplate(deref_type, env)
    elif tyutil.isTypeCode(deref_type):
        cast_to = "CORBA::TypeCode_member"
    cast_to = cast_to + "(*)" + tail_dims_string
    return "(const " + cast_to + ")(" + cast_to + ")" +\
           "(" + string + ")"


def interface(type):
    assert isinstance(type, idltype.Type)
    deref_type = tyutil.deref(type)
    desc = util.StringStream()
    if tyutil.isObjRef(deref_type):
        cname = mangler.canonTypeName(tyutil.derefKeepDims(type))
        # have we already made this declaration?
        if not(self.__buildDesc.has_key(cname)):
            self.__buildDesc[cname] = "defined"
            env = name.Environment()
            objref_name = tyutil.objRefTemplate(deref_type, "Member", env)
            desc.out("""\
extern void _0RL_buildDesc@cname@(tcDescriptor &, const @objref@&);""",
                     cname = cname, objref = objref_name)
    return desc


def sequence(type):
    env = name.Environment()
    seqType = type.seqType()
    deref_seqType = tyutil.deref(seqType)
    # FIXME:this is very hacky
    seqType_cname = mangler.canonTypeName(tyutil.derefKeepDims(seqType))
    sequence_template = tyutil.sequenceTemplate(type, env)
    deref_type = tyutil.deref(type)
    memberType_cname = mangler.canonTypeName(deref_type)

    seqType_dims = tyutil.typeDims(seqType)
    is_array = seqType_dims != []


    desc = util.StringStream()

    # if the thing being aliased is defined in another file we need to build
    # more than the normal set of getElementDesc macros
    defined_outside = isinstance(seqType, idltype.Declared) and \
                      not(seqType.decl().mainFile())
    # also if type is recursive
    recursive = isinstance(seqType, idltype.Declared) and \
                seqType.decl() == self.__currentNode

    if defined_outside or recursive:

        if tyutil.isStruct(seqType) or \
           tyutil.isUnion(seqType)  or \
           tyutil.isEnum(seqType):
            desc.out(str(external(seqType)))
        elif tyutil.isTypedef(seqType):            
            # sequence is of some kind of typedef
            seqType_decl = seqType.decl()

            # have we seen this declarator before?
            if not(self.__seenDeclarators.has_key(seqType_decl)):
                self.__seenDeclarators[seqType_decl] = 1
            
                seqType_dims = seqType_decl.sizes() + tyutil.typeDims(seqType)
                deref_seqType = tyutil.deref(seqType)
                # if its an array we need to generate that
                if seqType_dims != []:
                    desc.out(str(array(seqType.decl().alias().aliasType(), seqType_decl)))
                elif tyutil.isSequence(deref_seqType):
                    desc.out(str(sequence(deref_seqType)))
                

    # something very strange happens here with strings and casting
    thing = "(*((" + sequence_template + "*)_desc->opq_seq))[_index]"
    if is_array:
        thing = docast(seqType, None, thing)

    if tyutil.isObjRef(deref_seqType):
        objref_name = tyutil.objRefTemplate(deref_seqType, "Member", env)
        if not(is_array):
            desc.out(str(interface(seqType)))

    elif tyutil.isString(deref_seqType):
        desc.out(str(bstring(deref_seqType)))

    elif tyutil.isSequence(seqType):
        # element is an _anonymous_ sequence
        desc.out(str(sequence(seqType)))
    
    desc.out("""\
#ifndef __0RL_tcParser_buildDesc@cname@__
#define __0RL_tcParser_buildDesc@cname@__
static void
_0RL_tcParser_setElementCount@cname@(tcSequenceDesc* _desc, CORBA::ULong _len)
{
  ((@sequence_template@*)_desc->opq_seq)->length(_len);
}

static CORBA::ULong
_0RL_tcParser_getElementCount@cname@(tcSequenceDesc* _desc)
{
  return ((@sequence_template@*)_desc->opq_seq)->length();
}

static CORBA::Boolean
_0RL_tcParser_getElementDesc@cname@(tcSequenceDesc* _desc, CORBA::ULong _index, tcDescriptor& _newdesc)
{
  _0RL_buildDesc@thing_cname@(_newdesc, @thing@);
  return 1;
}

static void
_0RL_buildDesc@cname@(tcDescriptor &_desc, const @sequence_template@& _data)
{
  _desc.p_sequence.opq_seq = (void*) &_data;
  _desc.p_sequence.setElementCount =
    _0RL_tcParser_setElementCount@cname@;
  _desc.p_sequence.getElementCount =
    _0RL_tcParser_getElementCount@cname@;
  _desc.p_sequence.getElementDesc =
    _0RL_tcParser_getElementDesc@cname@;
  }
#endif
""", cname = memberType_cname, thing_cname = seqType_cname,
             sequence_template = sequence_template,
             thing = thing)
    return desc

# This code appears to be common to both structs and exceptions.
# Differences include some whitespace formatting and
# functions declared as static for structs and not for exceptions?
def member(node, modify_for_exception = 0):
    scopedName = node.scopedName()
    guard_name = tyutil.guardName(scopedName)
    env = name.Environment()
    fqname = env.nameToString(scopedName)
        
    cases = util.StringStream()
    # note the enum special case here
    num_members = 0
    index = 0
    desc = util.StringStream()
    for member in node.members():
        memberType = member.memberType()
        deref_memberType = tyutil.derefKeepDims(memberType)
        memberType_cname = mangler.canonTypeName(deref_memberType)
        deref_memberType_name = env.principalID(tyutil.deref(memberType),
                                                fully_scope = 1)
        member_dims = tyutil.typeDims(memberType)
        is_array = member_dims != []

        # enums have funny scoping rules
        if tyutil.isEnum(memberType) and member.constrType():
            num_members = num_members + len(memberType.decl().enumerators())
        
        if tyutil.isObjRef(deref_memberType):
            m_scopedName = map(tyutil.mapID, memberType.decl().scopedName())
            objref_name = name.prefixName(m_scopedName, "_objref_")
            helper_name = name.suffixName(m_scopedName, "_Helper")
            objref_member = "_CORBA_ObjRef_Member<" + objref_name + ", " +\
                            helper_name + ">"
            if not(is_array):
                desc.out(str(interface(memberType)))
        elif tyutil.isSequence(memberType):
            desc.out(str(sequence(memberType)))
        elif tyutil.isString(memberType):
            desc.out(str(bstring(memberType)))

        # FIXME: unify with  dynskel/main/visitUnion
        elif tyutil.isStruct(memberType) or \
             tyutil.isUnion(memberType)  or \
             tyutil.isEnum(memberType):
            # only if not defined in this file
            if not(memberType.decl().mainFile()):
                desc.out(str(external(memberType)))

        # build the cases
        for d in member.declarators():
            member_scopedName = d.scopedName()
            member_name = tyutil.mapID(tyutil.name(member_scopedName))
            decl_cname = mangler.canonTypeName(deref_memberType, d)
            decl_dims = d.sizes()
            full_dims = decl_dims + member_dims
            is_array = full_dims != []
            is_array_declarator = decl_dims != []

            if is_array_declarator:
                desc.out(str(array(memberType, d)))

            # something strange happening here too
            thing = "((" + fqname + "*)_desc->opq_struct)->" + member_name
            if is_array:
                thing = docast(memberType, d, thing)

            cases.out("""\
case @n@:
  _0RL_buildDesc@cname@(_newdesc, @thing@);
  return 1;""", n = str(index), cname = decl_cname,
                      thing = thing)
            num_members = num_members + 1
            index = index + 1
            

    # IMPROVEME (FIXME)
    if modify_for_exception:
        desc.out("""\
CORBA::Boolean _0RL_tcParser_getMemberDesc_@guard_name@(tcStructDesc *_desc, CORBA::ULong _index, tcDescriptor &_newdesc)
{""", guard_name = guard_name)
    else:
        desc.out("""\
static CORBA::Boolean
_0RL_tcParser_getMemberDesc_@guard_name@(tcStructDesc *_desc, CORBA::ULong _index, tcDescriptor &_newdesc){""", guard_name = guard_name)

    desc.inc_indent()
    desc.out("""\
  switch (_index) {
  @cases@
  default:
    return 0;
  };
}
""", cases = str(cases))
    if modify_for_exception:
        desc.out("""\
CORBA::ULong""")
    else:
        desc.out("""\
static CORBA::ULong""")
    desc.out("""\
_0RL_tcParser_getMemberCount_@guard_name@(tcStructDesc *_desc)
{
  return @num_members@;
}

void _0RL_buildDesc_c@guard_name@(tcDescriptor &_desc, const @fqname@& _data)
{
  _desc.p_struct.getMemberDesc = _0RL_tcParser_getMemberDesc_@guard_name@;
  _desc.p_struct.getMemberCount = _0RL_tcParser_getMemberCount_@guard_name@;
  _desc.p_struct.opq_struct = (void *)&_data;
}

""",
             guard_name = guard_name,
             fqname = fqname,
             num_members = str(num_members),
             cases = str(cases))
    return desc
   

def external(type):
    scopedName = type.decl().scopedName()
    guard_name = tyutil.guardName(scopedName)
    env = name.Environment()
    fqname = env.nameToString(scopedName)
    fn_name = "_0RL_buildDesc_c" + guard_name

    desc = util.StringStream()
    desc.out("""\
extern void _0RL_buildDesc_c@guard_name@(tcDescriptor &, const @fqname@&);""",
                         guard_name = guard_name,
                         fqname = fqname)
    return desc


def visitTypedef(node):
    aliasType = node.aliasType()

    # if the alias points to an "anonymous" sequence type
    # then we need to build it too
    if tyutil.isSequence(aliasType):
        stream.out(str(bdesc.sequence(aliasType)))

    # If the alias points to another typedef which has dimensions,
    # force generation of code for it if it is external.
    # else if the alias points to a declared thing which was declared in
    # another file, add an extern
    elif isinstance(aliasType, idltype.Declared) and \
         not(aliasType.decl().mainFile()):
        if tyutil.isTypedef(aliasType):
            type = aliasType.decl().alias().aliasType()
            decl = aliasType.decl()
            is_array_decl = decl.sizes() != []

            if is_array_decl:
                return
            
            aliasType.decl().alias().accept(self)
        elif tyutil.isStruct(aliasType) or \
             tyutil.isUnion(aliasType)  or \
             tyutil.isEnum(aliasType):
            pass


            
    
