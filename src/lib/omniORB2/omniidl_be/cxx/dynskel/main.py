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

from omniidl.be.cxx.dynskel import bdesc

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

    stream.out("""\
static void
@private_prefix@_tcParser_setObjectPtr_@guard_name@(tcObjrefDesc *_desc, CORBA::Object_ptr _ptr)
{
  @fqname@_ptr _p = @fqname@::_narrow(_ptr);
  @fqname@_ptr* pp = (@fqname@_ptr*)_desc->opq_objref;
  if (_desc->opq_release && !CORBA::is_nil(*pp)) CORBA::release(*pp);
  *pp = _p;
  CORBA::release(_ptr);
}

static CORBA::Object_ptr
@private_prefix@_tcParser_getObjectPtr_@guard_name@(tcObjrefDesc *_desc)
{
  return (CORBA::Object_ptr) *((@fqname@_ptr*)_desc->opq_objref);
}

void @private_prefix@_buildDesc_c@guard_name@(tcDescriptor& _desc, const @objref_member@& _data)
{
  _desc.p_objref.opq_objref = (void*) &_data._ptr;
  _desc.p_objref.opq_release = _data.pd_rel;
  _desc.p_objref.setObjectPtr = @private_prefix@_tcParser_setObjectPtr_@guard_name@;
  _desc.p_objref.getObjectPtr = @private_prefix@_tcParser_getObjectPtr_@guard_name@;
}


void @private_prefix@_delete_@guard_name@(void* _data) {
  CORBA::release((@fqname@_ptr) _data);
}

void operator<<=(CORBA::Any& _a, @fqname@_ptr _s) {
  tcDescriptor tcd;
  @objref_member@ tmp(_s,0);
  @private_prefix@_buildDesc_c@guard_name@(tcd, tmp);
  _a.PR_packFrom(@tc_name@, &tcd);
}

void operator<<=(CORBA::Any& _a, @fqname@_ptr* _sp) {
  _a <<= *_sp;
  CORBA::release(*_sp);
  *_sp = @fqname@::_nil();
}

CORBA::Boolean operator>>=(const CORBA::Any& _a, @fqname@_ptr& _s) {
  @fqname@_ptr sp = (@fqname@_ptr) _a.PR_getCachedData();
  if (sp == 0) {
    tcDescriptor tcd;
    @objref_member@ tmp;
    @private_prefix@_buildDesc_c@guard_name@(tcd, tmp);
    if( _a.PR_unpackTo(@tc_name@, &tcd) ) {
      if (!omniORB::omniORB_27_CompatibleAnyExtraction) {
        ((CORBA::Any*)&_a)->PR_setCachedData((void*)tmp._ptr,@private_prefix@_delete_@guard_name@);
      }
      _s = tmp._ptr;
      tmp._ptr = @fqname@::_nil(); return 1;
    } else {
      _s = @fqname@::_nil(); return 0;
    }
  }
  else {
    CORBA::TypeCode_var tc = _a.type();
    if (tc->equivalent(@tc_name@)) {
    _s = sp; return 1;
    }
    else {
    _s = @fqname@::_nil(); return 0;
    }
  }
}
""", guard_name = guard_name, fqname = fqname, objref_member = objref_member,
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
            stream.out("""\
void @private_prefix@_delete_@guard_name@(void* _data) {
  @fqname@_slice* _0RL_t = (@fqname@_slice*) _data;
  @fqname@_free(_0RL_t);
}
""", fqname = fqname, guard_name = guard_name,
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
                
            stream.out("""\
void operator<<=(CORBA::Any& _a, const @fqname@_forany& _s) {
  @fqname@_slice* @private_prefix@_s = _s.NP_getSlice();
  tcDescriptor @private_prefix@_tcdesc;
  @private_prefix@_buildDesc@decl_cname@(@private_prefix@_tcdesc, (const @dtype@(*)@tail_dims@)(@dtype@(*)@tail_dims@)(@private_prefix@_s));
  _a.PR_packFrom(@tcname@, &@private_prefix@_tcdesc);
  if( _s.NP_nocopy() ) {
    delete[] @private_prefix@_s;
  }
}
""",
                       fqname = fqname,
                       decl_cname = decl_cname,
                       type = alias_tyname,
                       dtype = argtype,
                       tail_dims = tail_dims,
                       private_prefix = config.privatePrefix(),
                       tcname = tc_name)

            stream.out("""\
CORBA::Boolean operator>>=(const CORBA::Any& _a, @fqname@_forany& _s) {
  @fqname@_slice* @private_prefix@_s = (@fqname@_slice*) _a.PR_getCachedData();
  if( !@private_prefix@_s ) {
    @private_prefix@_s = @fqname@_alloc();
    tcDescriptor @private_prefix@_tcdesc;
    @private_prefix@_buildDesc@decl_cname@(@private_prefix@_tcdesc, (const @dtype@(*)@tail_dims@)(@dtype@(*)@tail_dims@)(@private_prefix@_s));
    if( !_a.PR_unpackTo(@tcname@, &@private_prefix@_tcdesc) ) {
      delete[] @private_prefix@_s;
      _s = 0;
      return 0;
    }
    ((CORBA::Any*)&_a)->PR_setCachedData(@private_prefix@_s, @private_prefix@_delete_@guard_name@);
  } else {
    CORBA::TypeCode_var @private_prefix@_tc = _a.type();
    if( !@private_prefix@_tc->equivalent(@tcname@) ) {
      _s = 0;
      return 0;
    }
  }
  _s = @private_prefix@_s;
  return 1;
}
""",
                       fqname = fqname,
                       decl_cname = decl_cname,
                       type = alias_tyname,
                       dtype = argtype,
                       tail_dims = tail_dims,
                       tcname = tc_name,
                       private_prefix = config.privatePrefix(),
                       guard_name = guard_name)
        

        # --- sequences
        if not(is_array_declarator) and tyutil.isSequence(aliasType):
            if first_declarator:
                stream.out(str(bdesc.sequence(deref_aliasType)))
            stream.out("""\
void operator <<= (CORBA::Any& a, const @fqname@& s)
{
  tcDescriptor tcdesc;
  @private_prefix@_buildDesc@decl_cname@(tcdesc, s);
  a.PR_packFrom(@tcname@, &tcdesc);
}

void @private_prefix@_seq_delete_@guard_name@(void* data)
{
  delete (@fqname@*)data;
}

CORBA::Boolean operator >>= (const CORBA::Any& a, @fqname@*& s_out)
{
  return a >>= (const @fqname@*&) s_out;
}

CORBA::Boolean operator >>= (const CORBA::Any& a, const @fqname@*& s_out)
{
  s_out = 0;
  @fqname@* stmp = (@fqname@*) a.PR_getCachedData();
  if( stmp == 0 ) {
    tcDescriptor tcdesc;
    stmp = new @fqname@;
    @private_prefix@_buildDesc@decl_cname@(tcdesc, *stmp);
    if( a.PR_unpackTo(@tcname@, &tcdesc)) {
      ((CORBA::Any*)&a)->PR_setCachedData((void*)stmp, @private_prefix@_seq_delete_@guard_name@);
      s_out = stmp;
      return 1;
    } else {
      delete (@fqname@ *)stmp;
      return 0;
    }
  } else {
    CORBA::TypeCode_var tctmp = a.type();
    if( tctmp->equivalent(@tcname@) ) {
      s_out = stmp;
      return 1;
    } else {
      return 0;
    }
  }
}

""",
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

    stream.out("""\
void @private_prefix@_buildDesc_c@guard_name@(tcDescriptor& _desc, const @fqname@& _data)
{
  _desc.p_enum = (CORBA::ULong*)&_data;
}

void operator<<=(CORBA::Any& _a, @fqname@ _s)
{
  tcDescriptor @private_prefix@_tcd;
  @private_prefix@_buildDesc_c@guard_name@(@private_prefix@_tcd, _s);
  _a.PR_packFrom(@private_prefix@_tc_@guard_name@, &@private_prefix@_tcd);
}

CORBA::Boolean operator>>=(const CORBA::Any& _a, @fqname@& _s)
{
  tcDescriptor @private_prefix@_tcd;
  @private_prefix@_buildDesc_c@guard_name@(@private_prefix@_tcd, _s);
  return _a.PR_unpackTo(@private_prefix@_tc_@guard_name@, &@private_prefix@_tcd);
}
""",
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

    stream.out("""\
void @private_prefix@_delete_@guard_name@(void* _data) {
  @fqname@* @private_prefix@_t = (@fqname@*) _data;
  delete @private_prefix@_t;
}

@member_desc@

void operator<<=(CORBA::Any& _a, const @fqname@& _s) {
  tcDescriptor @private_prefix@_tcdesc;
  @private_prefix@_buildDesc_c@guard_name@(@private_prefix@_tcdesc, _s);
  _a.PR_packFrom(@private_prefix@_tc_@guard_name@, &@private_prefix@_tcdesc);
}
 
void operator<<=(CORBA::Any& _a, @fqname@* _sp) {
  tcDescriptor @private_prefix@_tcdesc;
  @private_prefix@_buildDesc_c@guard_name@(@private_prefix@_tcdesc, *_sp);
  _a.PR_packFrom(@private_prefix@_tc_@guard_name@, &@private_prefix@_tcdesc);
  delete _sp;
}

CORBA::Boolean operator>>=(const CORBA::Any& _a, @fqname@*& _sp) {
  return _a >>= (const @fqname@*&) _sp;
}

CORBA::Boolean operator>>=(const CORBA::Any& _a, const @fqname@*& _sp) {
  _sp = (@fqname@ *) _a.PR_getCachedData();
  if (_sp == 0) {
    tcDescriptor @private_prefix@_tcdesc;
    _sp = new @fqname@;
    @private_prefix@_buildDesc_c@guard_name@(@private_prefix@_tcdesc, *_sp);
    if (_a.PR_unpackTo(@private_prefix@_tc_@guard_name@, &@private_prefix@_tcdesc)) {
      ((CORBA::Any *)&_a)->PR_setCachedData((void*)_sp, @private_prefix@_delete_@guard_name@);
      return 1;
    } else {
      delete (@fqname@ *)_sp; _sp = 0;
      return 0;
    }
  } else {
    CORBA::TypeCode_var @private_prefix@_tctmp = _a.type();
    if (@private_prefix@_tctmp->equivalent(@private_prefix@_tc_@guard_name@)) return 1;
    _sp = 0;
    return 0;
  }
}""", fqname = fqname, guard_name = guard_name, cases = str(cases),
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

    allCaseValues = tyutil.allCaseValues(node)
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
            stream.out("""\
#ifndef @private_prefix@_buildDesc_c@bound@string
#define @private_prefix@_buildDesc_c@bound@string @private_prefix@_buildDesc_cstring
#endif""", bound = str(caseType.bound()),
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
        
    stream.out("""\
class @private_prefix@_tcParser_unionhelper_@guard_name@ {
public:
  static void getDiscriminator(tcUnionDesc* _desc, tcDescriptor& _newdesc, CORBA::PR_unionDiscriminator& _discrim) {
    @fqname@* _u = (@fqname@*)_desc->opq_union;
    @private_prefix@_buildDesc@discrim_cname@(_newdesc, _u->pd__d);
    _discrim = (CORBA::PR_unionDiscriminator)_u->pd__d;
  }

  static void setDiscriminator(tcUnionDesc* _desc, CORBA::PR_unionDiscriminator _discrim, int _is_default) {
    @fqname@* _u = (@fqname@*)_desc->opq_union;
    _u->pd__d = (@discrim_type@)_discrim;
    _u->pd__default = _is_default;
  }

  static CORBA::Boolean getValueDesc(tcUnionDesc* _desc, tcDescriptor& _newdesc) {
    @fqname@* _u = (@fqname@*)_desc->opq_union;
    @switch@
    return 1;
  }
};""",
               guard_name = guard_name,
               discrim_cname = discrim_cname,
               discrim_type = discrim_type,
               switch = str(switch),
               private_prefix = config.privatePrefix(),
               fqname = fqname)

    

    stream.out("""\
void @private_prefix@_buildDesc_c@guard_name@(tcDescriptor& _desc, const @fqname@& _data)
{
  _desc.p_union.getDiscriminator = @private_prefix@_tcParser_unionhelper_@guard_name@::getDiscriminator;
  _desc.p_union.setDiscriminator = @private_prefix@_tcParser_unionhelper_@guard_name@::setDiscriminator;
  _desc.p_union.getValueDesc = @private_prefix@_tcParser_unionhelper_@guard_name@::getValueDesc;
  _desc.p_union.opq_union = (void*)&_data;
}
""", guard_name = guard_name, fqname = fqname,
               private_prefix = config.privatePrefix())

    stream.out("""\
void @private_prefix@_delete_@guard_name@(void* _data)
{
  @fqname@* @private_prefix@_t = (@fqname@*) _data;
  delete @private_prefix@_t;
}
""", guard_name = guard_name, fqname = fqname,
               private_prefix = config.privatePrefix())

    stream.out("""\
void operator<<=(CORBA::Any& _a, const @fqname@& _s)
{
  tcDescriptor @private_prefix@_tcdesc;
  @private_prefix@_buildDesc_c@guard_name@(@private_prefix@_tcdesc, _s);
  _a.PR_packFrom(@private_prefix@_tc_@guard_name@, &@private_prefix@_tcdesc);
}
""", guard_name = guard_name, fqname = fqname,
               private_prefix = config.privatePrefix())    

    stream.out("""\
CORBA::Boolean operator>>=(const CORBA::Any& _a, @fqname@*& _sp) {
  return _a >>= (const @fqname@*&) _sp;
}
""", fqname = fqname)

    stream.out("""\
CORBA::Boolean operator>>=(const CORBA::Any& _a, const @fqname@*& _sp) {
  _sp = (@fqname@ *) _a.PR_getCachedData();
  if (_sp == 0) {
    tcDescriptor @private_prefix@_tcdesc;
    _sp = new @fqname@;
    @private_prefix@_buildDesc_c@guard_name@(@private_prefix@_tcdesc, *_sp);
    if( _a.PR_unpackTo(@private_prefix@_tc_@guard_name@, &@private_prefix@_tcdesc) ) {
      ((CORBA::Any*)&_a)->PR_setCachedData((void*)_sp, @private_prefix@_delete_@guard_name@);
      return 1;
    } else {
      delete ( @fqname@*)_sp;
      _sp = 0;
      return 0;
    }
  } else {
    CORBA::TypeCode_var @private_prefix@_tctmp = _a.type();
    if (@private_prefix@_tctmp->equivalent(@private_prefix@_tc_@guard_name@)) return 1;
    _sp = 0;
    return 0;
  }
}
""", fqname = fqname, guard_name = guard_name,
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

    stream.out("""\
void @private_prefix@_delete_@guard_name@(void* _data) {
  @fqname@* @private_prefix@_t = (@fqname@*) _data;
  delete @private_prefix@_t;
}
""", guard_name = guard_name, fqname = fqname,
               private_prefix = config.privatePrefix())

    stream.out("""\
void operator<<=(CORBA::Any& _a, const @fqname@& _s) {
  tcDescriptor _0RL_tcdesc;
  @private_prefix@_buildDesc_c@guard_name@(@private_prefix@_tcdesc, _s);
  _a.PR_packFrom(@private_prefix@_tc_@guard_name@, &@private_prefix@_tcdesc);
}

void operator<<=(CORBA::Any& _a, const @fqname@* _sp) {
  tcDescriptor @private_prefix@_tcdesc;
  @private_prefix@_buildDesc_c@guard_name@(@private_prefix@_tcdesc, *_sp);
  _a.PR_packFrom(@private_prefix@_tc_@guard_name@, &@private_prefix@_tcdesc);
  delete (@fqname@ *)_sp;
}

CORBA::Boolean operator>>=(const CORBA::Any& _a,const @fqname@*& _sp) {
  _sp = (@fqname@ *) _a.PR_getCachedData();
  if (_sp == 0) {
    tcDescriptor @private_prefix@_tcdesc;
    _sp = new @fqname@;
    @private_prefix@_buildDesc_c@guard_name@(@private_prefix@_tcdesc, *_sp);
    if (_a.PR_unpackTo(@private_prefix@_tc_@guard_name@, &@private_prefix@_tcdesc)) {
      ((CORBA::Any *)&_a)->PR_setCachedData((void*)_sp, @private_prefix@_delete_@guard_name@);
      return 1;
    } else {
      delete (@fqname@ *)_sp;_sp = 0;
      return 0;
    }
  } else {
    CORBA::TypeCode_var @private_prefix@_tctmp = _a.type();
    if (@private_prefix@_tctmp->equivalent(@private_prefix@_tc_@guard_name@)) return 1;
    delete (@fqname@ *)_sp;_sp = 0;
    return 0;
  }
}
""", fqname = fqname, guard_name = guard_name,
               private_prefix = config.privatePrefix())

    stream.out("""\
static void @private_prefix@_insertToAny__c@guard_name@(CORBA::Any& a,const CORBA::Exception& e) {
  const @fqname@ & _ex = (const @fqname@ &) e;
  operator<<=(a,_ex);
}

static void @private_prefix@_insertToAnyNCP__c@guard_name@ (CORBA::Any& a,const CORBA::Exception* e) {
  const @fqname@ * _ex = (const @fqname@ *) e;
  operator<<=(a,_ex);
}

class @private_prefix@_insertToAny_Singleton__c@guard_name@ {
public:
  @private_prefix@_insertToAny_Singleton__c@guard_name@() {
    @fqname@::insertToAnyFn = @private_prefix@_insertToAny__c@guard_name@;
    @fqname@::insertToAnyFnNCP = @private_prefix@_insertToAnyNCP__c@guard_name@;
  }
};
static @private_prefix@_insertToAny_Singleton__c@guard_name@ @private_prefix@_insertToAny_Singleton__c@guard_name@_;
""", fqname = fqname, guard_name = guard_name,
               private_prefix = config.privatePrefix())
    


    bdesc.finishingNode()



