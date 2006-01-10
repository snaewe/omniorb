# -*- python -*-
#                           Package   : omniidl
# defs.py                   Created on: 1999/11/2
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
#   Produce the main header definitions for the C++ backend

# $Id$
# $Log$
# Revision 1.36.2.11  2006/01/10 12:24:04  dgrisby
# Merge from omni4_0_develop pre 4.0.7 release.
#
# Revision 1.36.2.10  2005/11/09 12:22:17  dgrisby
# Local interfaces support.
#
# Revision 1.36.2.9  2005/08/16 13:51:21  dgrisby
# Problems with valuetype / abstract interface C++ mapping.
#
# Revision 1.36.2.8  2005/01/06 23:10:03  dgrisby
# Big merge from omni4_0_develop.
#
# Revision 1.36.2.7  2004/10/13 17:58:23  dgrisby
# Abstract interfaces support; values support interfaces; value bug fixes.
#
# Revision 1.36.2.6  2004/07/23 10:29:59  dgrisby
# Completely new, much simpler Any implementation.
#
# Revision 1.36.2.5  2004/07/04 23:53:38  dgrisby
# More ValueType TypeCode and Any support.
#
# Revision 1.36.2.4  2004/02/16 10:10:31  dgrisby
# More valuetype, including value boxes. C++ mapping updates.
#
# Revision 1.36.2.3  2003/11/06 11:56:56  dgrisby
# Yet more valuetype. Plain valuetype and abstract valuetype are now working.
#
# Revision 1.36.2.2  2003/10/23 11:25:55  dgrisby
# More valuetype support.
#
# Revision 1.36.2.1  2003/03/23 21:02:38  dgrisby
# Start of omniORB 4.1.x development branch.
#
# Revision 1.33.2.14  2001/10/29 17:42:39  dpg1
# Support forward-declared structs/unions, ORB::create_recursive_tc().
#
# Revision 1.33.2.13  2001/10/18 12:45:28  dpg1
# IDL compiler tweaks.
#
# Revision 1.33.2.12  2001/08/17 13:45:55  dpg1
# C++ mapping fixes.
#
# Revision 1.33.2.11  2001/08/15 10:29:53  dpg1
# Update DSI to use Current, inProcessIdentity.
#
# Revision 1.33.2.10  2001/07/31 19:25:11  sll
#  Array _var should be separated into fixed and variable size ones.
#
# Revision 1.33.2.9  2001/07/26 11:27:26  dpg1
# Typo in C++ back-end broke wstring constants.
#
# Revision 1.33.2.8  2001/06/08 17:12:16  dpg1
# Merge all the bug fixes from omni3_develop.
#
# Revision 1.33.2.7  2001/03/13 10:32:08  dpg1
# Fixed point support.
#
# Revision 1.33.2.6  2000/11/20 14:43:25  sll
# Added support for wchar and wstring.
#
# Revision 1.33.2.5  2000/11/10 15:40:52  dpg1
# Missed an update in yesterday's merge.
#
# Revision 1.33.2.4  2000/11/07 18:29:56  sll
# Choose a default constant not too large to avoid g++ complaining.
#
# Revision 1.33.2.3  2000/11/03 19:19:46  sll
# visitTypedef() just rely on types.base() to give the right type name for
# object reference, instead of adding in a "_ptr" suffix.
#
# Revision 1.33.2.2  2000/10/12 15:37:50  sll
# Updated from omni3_1_develop.
#
# Revision 1.34.2.3  2000/08/21 11:35:15  djs
# Lots of tidying
#
# Revision 1.34.2.2  2000/08/04 17:10:29  dpg1
# Long long support
#
# Revision 1.34.2.1  2000/08/02 10:52:02  dpg1
# New omni3_1_develop branch, merged from omni3_develop.
#
# Revision 1.34  2000/07/13 15:26:00  dpg1
# Merge from omni3_develop for 3.0 release.
#
# Revision 1.31.2.19  2000/08/03 21:27:39  djs
# Typo in -Wbvirtual_objref code caused incorrect operation signatures to be
# generated in the _objref class of an interface.
#
# Revision 1.31.2.18  2000/07/26 15:29:08  djs
# Missing typedef and forward when generating BOA skeletons
#
# Revision 1.31.2.17  2000/07/18 15:34:25  djs
# Added -Wbvirtual_objref option to make attribute and operation _objref
# methods virtual
#
# Revision 1.31.2.16  2000/07/12 17:16:11  djs
# Minor bugfix to option -Wbsplice-modules
#
# Revision 1.31.2.15  2000/06/27 16:15:10  sll
# New classes: _CORBA_String_element, _CORBA_ObjRef_Element,
# _CORBA_ObjRef_tcDesc_arg to support assignment to an element of a
# sequence of string and a sequence of object reference.
#
# Revision 1.31.2.14  2000/06/26 16:23:57  djs
# Better handling of #include'd files (via new commandline options)
# Refactoring of configuration state mechanism.
#
# Revision 1.31.2.13  2000/06/19 18:19:49  djs
# Implemented union discriminant setting function _d(_value) with checks for
# illegal uses (setting to a label corresponding to a non-current member and
# setting before initialisation)
#
# Revision 1.31.2.12  2000/06/16 08:36:40  djs
# For a union with a boolean discriminant and an implied default, sets a
# better default discriminator in _default()
#
# Revision 1.31.2.11  2000/06/05 13:03:55  djs
# Removed union member name clash (x & pd_x, pd__default, pd__d)
# Removed name clash when a sequence is called "pd_seq"
# Nested union within union fix
# Actually generates BOA non-flattened tie templates
#
# Revision 1.31.2.10  2000/05/31 18:02:56  djs
# Better output indenting (and preprocessor directives now correctly output at
# the beginning of lines)
#
# Revision 1.31.2.9  2000/05/30 15:59:23  djs
# Removed inheritance ambiguity in generated BOA _sk_ and POA_ classes
#
# Revision 1.31.2.8  2000/05/18 15:57:32  djs
# Added missing T* data constructor for bounded sequence types
#
# Revision 1.31.2.7  2000/05/04 14:35:02  djs
# Added new flag splice-modules which causes all continuations to be output
# as one lump. Default is now to output them in pieces following the IDL.
#
# Revision 1.31.2.6  2000/04/26 18:22:28  djs
# Rewrote type mapping code (now in types.py)
# Rewrote identifier handling code (now in id.py)
#
# Revision 1.31.2.5  2000/04/05 10:57:37  djs
# Minor source tidying (removing commented out blocks)
#
# Revision 1.31.2.4  2000/03/24 17:18:48  djs
# Missed _ from typedefs to objrefs
#
# Revision 1.31.2.3  2000/03/24 16:24:22  djs
# Produced 2 invalid typedefs in the case of a typedef to an object reference
# (it transformed a " " into a "_" by mistake)
#
# Revision 1.31.2.2  2000/03/20 11:50:18  djs
# Removed excess buffering- output templates have code attached which is
# lazily evaluated when required.
#
# Revision 1.31.2.1  2000/02/14 18:34:55  dpg1
# New omniidl merged in.
#
# Revision 1.31  2000/01/19 11:23:27  djs
# Moved most C++ code to template file
#
# Revision 1.30  2000/01/18 18:05:52  djs
# Extracted most C++ from header/defs and put in a template file.
# General refactoring.
#
# Revision 1.29  2000/01/17 17:01:49  djs
# Many fixes including:
#   Module continuations
#   Constructed types
#   Union discriminator handling
#
# Revision 1.28  2000/01/14 17:38:27  djs
# Preliminary support for unions with multiple case labels
#
# Revision 1.27  2000/01/13 17:02:00  djs
# Added support for operation contexts.
#
# Revision 1.26  2000/01/13 15:56:38  djs
# Factored out private identifier prefix rather than hard coding it all through
# the code.
#
# Revision 1.25  2000/01/13 14:16:29  djs
# Properly clears state between processing separate IDL input files
#
# Revision 1.24  2000/01/12 17:48:31  djs
# Added option to create BOA compatible skeletons (same as -BBOA in omniidl3)
#
# Revision 1.23  2000/01/11 14:13:15  djs
# Updated array mapping to include NAME_copy(to, from) as per 2.3 spec
#
# Revision 1.22  2000/01/11 11:34:27  djs
# Added support for fragment generation (-F) mode
#
# Revision 1.21  2000/01/10 16:13:13  djs
# Removed a chunk of redundant code.
#
# Revision 1.20  2000/01/10 15:38:55  djs
# Better name and scope handling.
#
# Revision 1.19  2000/01/07 20:31:27  djs
# Regression tests in CVSROOT/testsuite now pass for
#   * no backend arguments
#   * tie templates
#   * flattened tie templates
#   * TypeCode and Any generation
#
# Revision 1.18  1999/12/26 16:43:06  djs
# Enum scope fix
# Handling default case of char discriminated switch fixed
#
# Revision 1.17  1999/12/25 21:47:18  djs
# Better TypeCode support
#
# Revision 1.16  1999/12/24 18:14:29  djs
# Fixed handling of #include'd .idl files
#
# Revision 1.15  1999/12/16 16:10:46  djs
# TypeCode fixes
#
# Revision 1.14  1999/12/15 12:13:16  djs
# Multiple forward declarations of interface fix
#
# Revision 1.13  1999/12/14 17:38:22  djs
# Fixed anonymous sequences of sequences bug
#
# Revision 1.12  1999/12/14 11:52:30  djs
# Support for CORBA::TypeCode and CORBA::Any
#
# Revision 1.11  1999/12/01 17:03:15  djs
# Added support for Typecodes and Anys
#
# Revision 1.10  1999/11/23 18:46:34  djs
# Constant fixes
# Moved exception constructor argument generator into a more central place
#
# Revision 1.9  1999/11/19 20:08:09  djs
# Removed references to a non-existant utility function
#
# Revision 1.8  1999/11/15 19:12:45  djs
# Tidied up sequence template handling
# Moved useful functions into separate utility module
#
# Revision 1.7  1999/11/12 17:18:39  djs
# Lots of header generation bugfixes
#
# Revision 1.6  1999/11/10 20:19:43  djs
# Array struct element fix
# Union sequence element fix
#
# Revision 1.5  1999/11/08 19:29:03  djs
# Rewrite of sequence template code
# Fixed lots of typedef problems
#
# Revision 1.4  1999/11/04 19:05:08  djs
# Finished moving code from tmp_omniidl. Regression tests ok.
#
# Revision 1.3  1999/11/03 18:18:12  djs
# Struct and Exception fixes
#
# Revision 1.2  1999/11/03 17:35:11  djs
# Brought more of the old tmp_omniidl code into the new tree
#
# Revision 1.1  1999/11/03 11:09:56  djs
# General module renaming
#

"""Produce the main header definitions"""
# similar to o2be_root::produce_hdr_defs in the old C++ BE

import string

from omniidl import idlast, idltype, idlutil
from omniidl_be.cxx import output, config, id, types, iface, cxx, ast, util
from omniidl_be.cxx import value
from omniidl_be.cxx.header import template

import defs

self = defs

def __init__(stream):
    defs.stream = stream
    # Need to keep track of how deep within the AST we are.
    # In a recursive procedure these would be extra arguments,
    # but the visitor pattern necessitates them being global.
    self.__insideInterface = 0
    self.__insideModule    = 0
    self.__insideClass     = 0

    # A repository id entry in this hash indicates that an interface
    # has been declared- therefore any more AST forward nodes for this
    # interface are ignored.
    self.__interfaces = {}

    # When we first encounter a module, we sometimes deal with all the
    # continuations straight away. Therefore when we reencounter a
    # continuation later, we don't duplicate the definitions.
    self.__completedModules = {}

    return defs

# Returns the prefix required inside a const declaration (it depends on
# exactly what the declaration is nested inside)
def const_qualifier(insideModule=None, insideClass=None):
    if insideModule is None:
        insideModule = __insideModule
        insideClass  = __insideClass

    if not insideModule and not insideClass:
        return "_CORBA_GLOBAL_VAR"
    elif insideClass:
        return "static"
    else:
        return "_CORBA_MODULE_VAR"

# Same logic for function qualifiers
def func_qualifier():
    return const_qualifier(__insideModule, __insideClass)

# Inline functions are subtly different
def inline_qualifier():
    if not __insideModule and not __insideClass:
        return "inline"
    elif __insideClass:
        return "static inline"
    else:
        return "_CORBA_MODULE_INLINE"


#
# Control arrives here
#
def visitAST(node):
    self.__insideInterface  = 0
    self.__insideModule     = 0
    self.__insideClass      = 0
    self.__interfaces       = {}
    self.__completedModules = {}
    for n in node.declarations():
        if ast.shouldGenerateCodeForDecl(n):
            n.accept(self)

def visitModule(node):
    # Ensure we only output the definitions once.
    # In particular, when the splice-modules flag is set and this is
    # a reopened module, the node will be marked as completed already.
    if self.__completedModules.has_key(node):
        return
    self.__completedModules[node] = 1
    
    ident = node.identifier()
    cxx_id = id.mapID(ident)

    if not config.state['Fragment']:
        stream.out(template.module_begin, name = cxx_id)
        stream.inc_indent()

    # push self.__insideModule, true
    insideModule = self.__insideModule
    self.__insideModule = 1

    for n in node.definitions():
        n.accept(self)

    # deal with continuations (only if the splice-modules flag is set)
    if config.state['Splice Modules']:
        for c in node.continuations():
            for n in c.definitions():
                n.accept(self)
            self.__completedModules[c] = 1

    # pop self.__insideModule
    self.__insideModule = insideModule
    
    if not config.state['Fragment']:
        stream.dec_indent()
        stream.out(template.module_end, name = cxx_id)

        

def visitInterface(node):
    # It's legal to have a forward interface declaration after
    # the actual interface definition. Make sure we ignore these.
    self.__interfaces[node.repoId()] = 1

    name = node.identifier()
    cxx_name = id.mapID(name)

    outer_environment = id.lookup(node)
    environment = outer_environment.enter(name)

    # push self.__insideInterface, true
    # push self.__insideClass, true
    insideInterface = self.__insideInterface
    self.__insideInterface = 1
    insideClass = self.__insideClass
    self.__insideClass = 1

    # make the necessary forward references, typedefs and define
    # the _Helper class
    I = iface.Interface(node)

    I_Helper = iface.instance("I_Helper")(I)
    I_Helper.hh(stream)

    # recursively take care of other IDL declared within this
    # scope (evaluate function later- lazy eval though 'thunking')
    def Other_IDL(node = node):
        for n in node.declarations():
            n.accept(self)

    # Output the this interface's corresponding class
    Ibase = iface.instance("I")(I,Other_IDL)
    Ibase.hh(stream)

    if not node.local():
        _objref_I = iface.instance("_objref_I")(I)
        _objref_I.hh(stream)

        _pof_I = iface.instance("_pof_I")(I)
        _pof_I.hh(stream)

        # Skeleton class
        _impl_I = iface.instance("_impl_I")(I)
        _impl_I.hh(stream)

        # Generate BOA compatible skeletons?
        if config.state['BOA Skeletons']:
            _sk_I = iface.instance("_sk_I")(I)
            _sk_I.hh(stream)

    # pop self.__insideInterface
    # pop self.__insideClass
    self.__insideInterface = insideInterface
    self.__insideClass = insideClass

    # Typecode and Any
    if config.state['Typecode']:
        qualifier = const_qualifier(self.__insideModule, self.__insideClass)
        stream.out(template.typecode,
                   qualifier = qualifier,
                   name = cxx_name)
        
    return
    

def visitForward(node):
    # Note it's legal to have multiple forward declarations
    # of the same name. So ignore the duplicates.
    if self.__interfaces.has_key(node.repoId()):
        return
    self.__interfaces[node.repoId()] = 1

    environment = id.lookup(node)
    scope = environment.scope()
    cxx_id = id.mapID(node.identifier())
    name = id.Name(node.scopedName())
    guard = name.guard()

    # Potentially forward declare BOA skeleton class
    class_sk = ""
    if config.state['BOA Skeletons']:
        class_sk = "class _sk_" + name.unambiguous(environment) + ";"

    # output the definition
    if node.abstract():
        stream.out(template.abstract_interface_Helper,
                   guard = guard,
                   class_sk_name = class_sk,
                   name = name.unambiguous(environment))
    elif node.local():
        stream.out(template.local_interface_Helper,
                   guard = guard,
                   class_sk_name = class_sk,
                   name = name.unambiguous(environment))
    else:
        stream.out(template.interface_Helper,
                   guard = guard,
                   class_sk_name = class_sk,
                   name = name.unambiguous(environment))
        
def visitConst(node):
    environment = id.lookup(node)
    scope = environment.scope()

    constType = types.Type(node.constType())
    d_constType = constType.deref()
    if d_constType.string():
        type_string = "char *"
    elif d_constType.wstring():
        type_string = "CORBA::WChar *"
    elif d_constType.fixed():
        type_string = constType.member()
    else:
        type_string = d_constType.member()
        # should this be .base?

    cxx_name = id.mapID(node.identifier())

    value = d_constType.literal(node.value(), environment)

    representedByInteger = d_constType.representable_by_int()

    # depends on whether we are inside a class / in global scope
    # etc
    # should be rationalised with tyutil.const_qualifier
    if self.__insideClass:
        if representedByInteger:
            stream.out(template.const_inclass_isinteger,
                       type = type_string, name = cxx_name, val = value)
        else:
            stream.out(template.const_inclass_notinteger,
                       type = type_string, name = cxx_name)
    else:
        where = "GLOBAL"
        if self.__insideModule:
            where = "MODULE"
        if representedByInteger:
            stream.out(template.const_outsideclass_isinteger,
                       where = where,
                       type = type_string,
                       name = cxx_name,
                       val = value)
        else:
            stream.out(template.const_outsideclass_notinteger,
                       where = where,
                       type = type_string,
                       name = cxx_name)


def visitTypedef(node):
    environment = id.lookup(node)
    scope = environment.scope()
    
    is_global_scope = not (self.__insideModule or self.__insideInterface)
    
    aliasType = types.Type(node.aliasType())
    aliasTypeID = aliasType.member(environment)

    # is _this_ type a constructed type?
    if node.constrType():
        node.aliasType().decl().accept(self)
    
    d_type = aliasType.deref()
    derefTypeID = d_type.base(environment)

    basicReferencedTypeID = aliasType.member(environment)

    # each one is handled independently
    for d in node.declarators():
        
        # derivedName is the new typedef'd name
        # alias_dims is a list of dimensions of the type being aliased

        derivedName = id.mapID(d.identifier())
        
        alias_dims = aliasType.dims()

        # array_declarator indicates whether this is a simple (non-array)
        # declarator or not
        array_declarator = d.sizes() != []

        # Typecode and Any
        if config.state['Typecode']:
            qualifier = const_qualifier(self.__insideModule,self.__insideClass)
            stream.out(template.typecode,
                       qualifier = qualifier,
                       name = derivedName)
                    
        # is it a simple alias (ie not an array at this level)?
        if not array_declarator:
            # not an array declarator but a simple declarator to an array
            if aliasType.array():
                # simple alias to an array should alias all the
                # array handling functions, but we don't need to duplicate
                # array looping code since we can just call the functions
                # for the base type
                stream.out(template.typedef_simple_to_array,
                           base = basicReferencedTypeID,
                           derived = derivedName,
                           qualifier = func_qualifier(),
                           inline_qualifier = inline_qualifier())
                           
            # Non-array of string
            elif d_type.string():
                stream.out(template.typedef_simple_string,
                           name = derivedName)
            elif d_type.wstring():
                stream.out(template.typedef_simple_wstring,
                           name = derivedName)
            elif d_type.typecode():
                stream.out(template.typedef_simple_typecode,
                           name = derivedName)
            elif d_type.any():
                stream.out(template.typedef_simple_any,
                           name = derivedName)

            elif d_type.fixed():
                stream.out(template.typedef_simple_fixed,
                           name = derivedName,
                           digits = d_type.type().digits(),
                           scale = d_type.type().scale())

            # Non-array of basic type
            elif isinstance(d_type.type(), idltype.Base):
                stream.out(template.typedef_simple_basic,
                           base = basicReferencedTypeID,
                           derived = derivedName)

            # a typedef to a struct or union, or a typedef to a
            # typedef to a sequence
            elif d_type.struct() or d_type.structforward() or \
                 d_type.union() or d_type.unionforward() or \
                 (d_type.sequence() and aliasType.typedef()):
                
                stream.out(template.typedef_simple_constructed,
                           base = basicReferencedTypeID,
                           name = derivedName)
                    
            # Non-array of object reference
            elif d_type.interface():
                derefTypeID = string.replace(derefTypeID,"_ptr","")
                # Note that the base name is fully flattened
                is_CORBA_Object = d_type.type().scopedName() ==\
                                  ["CORBA", "Object"]
                impl_base = ""
                objref_base = ""
                sk_base = ""
                if not is_CORBA_Object:
                    scopedName = d_type.type().decl().scopedName()
                    name = id.Name(scopedName)
                    impl_scopedName = name.prefix("_impl_")
                    objref_scopedName = name.prefix("_objref_")
                    sk_scopedName = name.prefix("_sk_")
                    impl_name = impl_scopedName.unambiguous(environment)
                    objref_name = objref_scopedName.unambiguous(environment)
                    sk_name = sk_scopedName.unambiguous(environment)

                    impl_base =   "typedef " + impl_name   + " _impl_"   +\
                                   derivedName + ";"
                    objref_base = "typedef " + objref_name + " _objref_" +\
                                   derivedName + ";"
                    sk_base =     "typedef " + sk_name     + " _sk_"     +\
                                   derivedName + ";"

                stream.out(template.typedef_simple_objref,
                           base = derefTypeID,
                           name = derivedName,
                           impl_base = impl_base,
                           objref_base = objref_base)
                if config.state['BOA Skeletons']:
                    stream.out(sk_base)
                               
                    
            # Non-array of enum
            elif d_type.enum():
                stream.out(template.typedef_simple_basic,
                           base = basicReferencedTypeID,
                           derived = derivedName)

            # Non-array of sequence
            elif d_type.sequence():
                seqType = types.Type(d_type.type().seqType())
                d_seqType = seqType.deref()
                bounded = d_type.type().bound()
                
                templateName = d_type.sequenceTemplate(environment)
                
                if d_seqType.structforward() or d_seqType.unionforward():
                    # Sequence of forward-declared struct or union.
                    # We cannot use the normal sequence templates
                    # since they have inline methods that require the
                    # full definition of the member type. We use
                    # templates with abstract virtual functions
                    # instead.

                    element = element_ptr = seqType.base(environment)

                    def bounds(bounded = bounded,
                               derivedName = derivedName,
                               derived = templateName,
                               element = element):
                        if bounded:
                            ct = template.sequence_bounded_ctors
                        else:
                            ct = template.sequence_unbounded_ctors
                        stream.out(ct, name = derivedName, element=element,
                                   bound=bounded, derived=derived)

                    stream.out(template.sequence_forward_type,
                               name = derivedName,
                               derived = templateName,
                               element = element,
                               bounds = bounds)

                else:
                    # Normal case using a template class.

                    if seqType.array():
                        element = "*** INVALID"
                        element_ptr = seqType.base(environment)
                    else:
                        if d_seqType.string():
                            element = "_CORBA_String_element"
                            element_ptr = "char*"
                        elif d_seqType.wstring():
                            element = "_CORBA_WString_element"
                            element_ptr = "CORBA::WChar*"
                        elif d_seqType.interface():
                            element = seqType.base(environment)
                            element_ptr = element
                        elif d_seqType.value() or d_seqType.valuebox():
                            element = seqType.base(environment)
                            element_ptr = element + "*"
                        # only if an anonymous sequence
                        elif seqType.sequence():
                            element = d_seqType.sequenceTemplate(environment)
                            element_ptr = element
                        elif d_seqType.typecode():
                            element = "CORBA::TypeCode_member"
                            element_ptr = element
                        else:
                            element = seqType.base(environment)
                            element_ptr = element

                    # enums are a special case
                    # from o2be_sequence.cc:795:
                    # ----
                    # gcc requires that the marshalling operators for the
                    # element be declared before the sequence template is
                    # typedef'd. This is a problem for enums, as the
                    # marshalling operators are not yet defined (and are
                    # not part of the type itself).
                    # ----
                    # Note that the fully dereferenced name is used
                    friend = "friend"
                    if is_global_scope:
                        friend = ""

                    if d_seqType.enum() and not seqType.array():
                        stream.out(template.typedef_enum_oper_friend,
                                   element = d_seqType.base(environment),
                                   friend = friend)

                    # derivedName is the new type identifier
                    # element is the name of the basic element type
                    # templateName contains the template instantiation

                    def bounds(bounded = bounded, derivedName = derivedName,
                               element_ptr = element_ptr,
                               templateName = templateName):
                        if bounded:
                            ctor_template = template.sequence_bounded_ctors
                        else:
                            ctor_template = template.sequence_unbounded_ctors
                        stream.out(ctor_template,
                                   name = derivedName,
                                   element = element_ptr,
                                   derived = templateName)

                    # output the main sequence definition
                    stream.out(template.sequence_type,
                               name = derivedName,
                               derived = templateName,
                               bounds = bounds)
                

                # start building the _var and _out types
                element_reference = "*** INVALID"
                if not aliasType.array():
                    if d_seqType.string():
                        # special case alert
                        element_reference = element
                    elif d_seqType.wstring():
                        # special case alert
                        element_reference = element
                    elif d_seqType.interface():
                        element_reference = d_seqType.objRefTemplate("Element",
                                                                     environment)
                    elif d_seqType.value() or d_seqType.valuebox():
                        element_reference = d_seqType.valueTemplate("Element",
                                                                    environment)
                    # only if an anonymous sequence
                    elif seqType.sequence():
                        element_reference = d_seqType.sequenceTemplate(environment) + "&"
                    else:
                        element_reference = element + "&"

                def subscript_operator_var(stream = stream,
                                           is_array = seqType.array(),
                                           element_ptr = element_ptr,
                                           element_ref = element_reference):

                    if is_array:
                        stream.out(template.sequence_var_array_subscript,
                                   element = element_ptr)
                    else:
                        stream.out(template.sequence_var_subscript,
                                   element = element_ref)

                def subscript_operator_out(stream = stream,
                                           is_array = seqType.array(),
                                           element_ptr = element_ptr,
                                           element_ref = element_reference):
                    if is_array:
                        stream.out(template.sequence_out_array_subscript,
                                   element = element_ptr)
                    else:
                        stream.out(template.sequence_out_subscript,
                                   element = element_ref)
                    
                # write the _var class definition
                stream.out(template.sequence_var,
                           name = derivedName,
                           subscript_operator = subscript_operator_var)

                # write the _out class definition
                stream.out(template.sequence_out,
                           name = derivedName,
                           subscript_operator = subscript_operator_out)

            else:
                util.fatalError("Inexhaustive Case Match")


        # ----------------------------------------------------------------
        # declarator is an array typedef declarator
        elif array_declarator:

            all_dims = d.sizes() + alias_dims
            dimsString = cxx.dimsToString(d.sizes())
            taildims = cxx.dimsToString(d.sizes()[1:])
            
            typestring = aliasType.member(environment)

            # build the _dup loop
            def dup_loop(stream = stream, all_dims = all_dims):
                loop = cxx.For(stream, all_dims)
                stream.out("\n_data@index@ = _s@index@;\n",
                           index = loop.index())
                loop.end()

            # build the _copy loop
            def copy_loop(stream = stream, all_dims = all_dims):
                loop = cxx.For(stream, all_dims)
                stream.out("\n_to@index@ = _from@index@;\n",
                           index = loop.index())
                loop.end()

            stream.out(template.typedef_array,
                       name = derivedName,
                       type = typestring,
                       dims = dimsString,
                       taildims = taildims,
                       firstdim = repr(all_dims[0]),
                       dup_loop = dup_loop,
                       copy_loop = copy_loop,
                       qualifier = func_qualifier(),
                       inline_qualifier = inline_qualifier())

            # output the _copyHelper class
            if types.variableDecl(node):
                stream.out(template.typedef_array_copyHelper,
                           var_or_fix = "Variable",
                           name = derivedName)
                stream.out(template.typedef_array_variable_out_type,
                           name = derivedName)
            else:
                stream.out(template.typedef_array_copyHelper,
                           var_or_fix = "Fix",
                           name = derivedName)
                stream.out(template.typedef_array_fix_out_type,
                           name = derivedName)
               
     

def visitMember(node):
    memberType = node.memberType()
    if node.constrType():
        # if the type was declared here, it must be an instance
        # of idltype.Declared!
        assert isinstance(memberType, idltype.Declared)
        memberType.decl().accept(self)


def visitStruct(node):
    name = node.identifier()
    cxx_name = id.mapID(name)

    outer_environment = id.lookup(node)
    environment = outer_environment.enter(name)
    scope = environment.scope()
    
    insideClass = self.__insideClass
    self.__insideClass = 1
            
    # Deal with types constructed here
    def Other_IDL(stream = stream, node = node, environment = environment):
        for m in node.members():
            if m.constrType():
                m.memberType().decl().accept(self)
            
    # Deal with the actual struct members
    def members(stream = stream, node = node, environment = environment):
        for m in node.members():
            memberType = types.Type(m.memberType())

            memtype = memberType.member(environment)

            for d in m.declarators():
                ident = d.identifier()

                cxx_id = id.mapID(ident)

                decl_dims = d.sizes()
                is_array_declarator = decl_dims != []

                # non-arrays of direct sequences are done via a typedef
                if not is_array_declarator and memberType.sequence():
                    stream.out(template.struct_nonarray_sequence,
                               memtype = memtype,
                               cxx_id = cxx_id)
                else:
                    dims_string = cxx.dimsToString(decl_dims)
                    if is_array_declarator:
                        stream.out(template.struct_array_declarator,
                                   memtype = memtype,
                                   cxx_id = cxx_id,
                                   dims = dims_string,
                                   tail_dims = cxx.dimsToString(d.sizes()[1:]),
                                   prefix = config.state['Private Prefix'])

                    stream.out(template.struct_normal_member,
                               memtype = memtype,
                               cxx_id = cxx_id,
                               dims = dims_string)
            
    # Output the structure itself
    if types.variableDecl(node):
        stream.out(template.struct,
                   name = cxx_name,
                   fix_or_var = "Variable",
                   Other_IDL = Other_IDL,
                   members = members)
        stream.out(template.struct_variable_out_type,
                   name = cxx_name)
    else:
        stream.out(template.struct,
                   name = cxx_name,
                   fix_or_var = "Fix",
                   Other_IDL = Other_IDL,
                   members = members)
        stream.out(template.struct_fix_out_type,
                   name = cxx_name)

    
    self.__insideClass = insideClass

    # TypeCode and Any
    if config.state['Typecode']:
        # structs in C++ are classes with different default privacy policies
        qualifier = const_qualifier(self.__insideModule, self.__insideClass)
        stream.out(template.typecode,
                   qualifier = qualifier,
                   name = cxx_name)


def visitStructForward(node):
    cxx_name = id.mapID(node.identifier())
    stream.out(template.struct_forward, name = cxx_name)


def visitException(node):
    exname = node.identifier()

    cxx_exname = id.mapID(exname)

    outer_environment = id.lookup(node)
    environment = outer_environment.enter(exname)
    scope = environment.scope()
    
    insideClass = self.__insideClass
    self.__insideClass = 1

    # if the exception has no members, inline some no-ops
    no_members = (node.members() == [])

    # other types constructed within this one
    def Other_IDL(stream = stream, node = node):
        for m in node.members():
            if m.constrType():
                m.memberType().decl().accept(self)

    # deal with the exceptions members
    def members(stream = stream, node = node, environment = environment):
        for m in node.members():
            memberType = types.Type(m.memberType())

            for d in m.declarators():
                decl_dims = d.sizes()
                full_dims = decl_dims + memberType.dims()
                is_array = full_dims != []
                is_array_declarator = decl_dims != []
                
                memtype = memberType.member(environment)
                ident = d.identifier()

                cxx_id = id.mapID(ident)

                dims_string = cxx.dimsToString(decl_dims)
                
                if is_array_declarator:
                    stream.out(template.exception_array_declarator,
                               memtype = memtype,
                               cxx_id = cxx_id,
                               dims = dims_string,
                               tail_dims = cxx.dimsToString(d.sizes()[1:]),
                               private_prefix = config.state['Private Prefix'])

                stream.out(template.exception_member,
                           memtype = memtype,
                           cxx_id = cxx_id,
                           dims = dims_string)

    # deal with ctor args
    ctor_args = []
    for m in node.members():
        memberType = types.Type(m.memberType())
        d_memberType = memberType.deref()
        for d in m.declarators():
            decl_dims = d.sizes()
            is_array_declarator = decl_dims != []
            ctor_arg_type = memberType.op(types.IN, environment)
            # sequences are not passed by reference here
            if d_memberType.sequence():
                if memberType.typedef():
                    ctor_arg_type = "const " + id.Name(memberType.type().decl().scopedName()).unambiguous(environment)
                else:
                    ctor_arg_type = "const " + memberType.sequenceTemplate(environment)
            elif d_memberType.typecode():
                ctor_arg_type = "CORBA::TypeCode_ptr"
                
            ident = d.identifier()

            cxx_id = id.mapID(ident)

            if is_array_declarator:
                ctor_arg_type = "const " + config.state['Private Prefix'] +\
                                "_" + cxx_id
            ctor_args.append(ctor_arg_type + " i_" + cxx_id)

       
    ctor = ""
    if ctor_args != []:
        ctor = cxx_exname + "(" + string.join(ctor_args, ", ") + ");"
            
    if no_members:
        inline = "inline "
        body = "{ }"
        alignedSize = ""
    else:
        inline = ""
        body = ";"
        alignedSize = "size_t _NP_alignedSize(size_t) const;"

    # output the main exception declaration
    stream.out(template.exception,
               name = cxx_exname,
               Other_IDL = Other_IDL,
               members = members,
               constructor = ctor,
               alignedSize = alignedSize,
               inline = inline,
               body = body)
               
    self.__insideClass = insideClass

    # Typecode and Any
    if config.state['Typecode']:
        qualifier = const_qualifier(self.__insideModule, self.__insideClass)
        stream.out(template.typecode,
                   qualifier = qualifier,
                   name = cxx_exname)
    


def visitUnion(node):
    ident = node.identifier()

    cxx_id = id.mapID(ident)
    outer_environment = id.lookup(node)
    environment = outer_environment.enter(ident)
    scope = environment.scope()
    
    insideClass = self.__insideClass
    self.__insideClass = 1
    
    switchType = types.Type(node.switchType())
    d_switchType = switchType.deref()

    ####################################################################
    # in the case where there is no default case and an implicit default
    # member, choose a discriminator value to set. Note that attempting
    # to access the data is undefined
    def chooseArbitraryDefault(switchType = switchType,
                               values = ast.allCaseLabelValues(node),
                               environment = environment):
        
        # dereference the switch_type (ie if CASE <scoped_name>)
        switchType = switchType.deref()

        # for integer types, find the lowest unused number
        def min_unused(start, used = values):
            x = start
            while x in used:
                x = x + 1
            return x

        kind = switchType.type().kind()
        if switchType.integer():
            (low, high) = ast.integer_type_ranges[kind]
            s = switchType.literal(min_unused(low+1))
            return s

        # for other types, first compute the set of all legal values
        # (sets are all fairly small)
        elif kind == idltype.tk_char:
            all = map(chr, range(0, 255))
        elif kind == idltype.tk_boolean:
            all = [0, 1]
        elif kind == idltype.tk_enum:
            all = switchType.type().decl().enumerators()
        else:
            util.fatalError("Failed to generate a default union " +\
                            "discriminator value")
            
        # remove all those values which are already in use to leave
        # a set of possible unused values
        possibles = util.minus(all, values)
        # return the first one for simplicity
        return switchType.literal(possibles[0], environment)
    #
    ###############################################################
    

    # does the IDL union have any default case?
    # It'll be handy to know which case is the default one later-
    # so add a new attribute to mark it
    ast.markDefaultCase(node)
    hasDefault = ast.defaultCase(node) != None
        
    # CORBA 2.3 C++ Mapping 1-34
    # "A union has an implicit default member if it does not have
    # a default case and not all permissible values of the union
    # discriminant are listed"
    exhaustive = ast.exhaustiveMatch(switchType, ast.allCaseLabelValues(node))
    implicitDefault = not hasDefault and not exhaustive

    if types.variableDecl(node):
        fixed = "Variable"
    else:
        fixed = "Fix"

    def Other_IDL(stream = stream, node = node):
        # deal with constructed switch type
        if node.constrType():
            node.switchType().decl().accept(self)
        
        # deal with children defined in this scope
        for n in node.cases():
            if n.constrType():
                n.caseType().decl().accept(self)

    
    # create the default constructor body
    def default_constructor(stream = stream,
                            implicitDefault = implicitDefault,
                            hasDefault = hasDefault,
                            choose = chooseArbitraryDefault):
        if implicitDefault:
            stream.out(template.union_constructor_implicit)
        elif hasDefault:
            stream.out(template.union_constructor_default,
                       default = choose())
        return

    def ctor_cases(stream = stream, node = node, switchType = switchType,
                   environment = environment, exhaustive = exhaustive):
        for c in node.cases():
            for l in c.labels():
                if l.default(): continue
                
                discrimvalue = switchType.literal(l.value(), environment)
                name = id.mapID(c.declarator().identifier())
                stream.out(template.union_ctor_case,
                           discrimvalue = discrimvalue,
                           name = name)
        # Booleans are a special case (isn't everything?)
        booleanWrap = switchType.boolean() and exhaustive
        if booleanWrap:
            stream.out(template.union_ctor_bool_default)
        else:
            stream.out(template.union_ctor_default)
        return

    # create the copy constructor and the assignment operator
    # bodies
    def copy_constructor(stream = stream, exhaustive = exhaustive,
                         node = node, ctor_cases = ctor_cases):
        if not exhaustive:
            # grab the default case
            default = ""
            for c in node.cases():
                if c.isDefault:
                    case_id = c.declarator().identifier()
                    cxx_case_id = id.mapID(case_id)
                    default = cxx_case_id + "(_value._pd_" + cxx_case_id + ");"


            stream.out(template.union_ctor_nonexhaustive,
                       default = default,
                       cases = ctor_cases)
        else:
            stream.out(template.union_ctor_exhaustive,
                       cases = ctor_cases)
        return
        
    # do we need an implicit _default function?
    def implicit_default(stream = stream, choose = chooseArbitraryDefault,
                         implicitDefault = implicitDefault):
        if implicitDefault:
    
            stream.out(template.union_implicit_default,
                       arbitraryDefault = choose())
        return

    # The body of the union _d(_value) function generated here
    def _d_fn(stream = stream, node = node, switchType = switchType,
              implicitDefault = implicitDefault,
              environment = environment):

        # The plan:
        #  * Check the _pd__initialised flag is set, else throw BAD_PARAM
        #  * Check for the simple case where _value == _pd__d and return
        #  * Have a nested switch, the outer switch is keyed on the current
        #    discriminator value and the inner one is the requested new value
        #
        # Possibilities:
        #  * Could perform some code minimisation eg for the case
        #      union foo switch(boolean){
        #         case TRUE:
        #         case FALSE:
        #           T bar;
        #      };
        #    This is equivalent to a single default: case and no switch is
        #    required.
        
        # Make sure we don't output a switch with no cases (if there is a
        # one-to-one mapping of labels to cases)
        need_switch = 0

        # Need to fill in a default case only if the union has none itself
        outer_has_default = 0

        cases = output.StringStream()

        # keep track of which cases have been done
        cases_done = []


        # Produce a set of "case <foo>: goto fail;" for every label
        # except those in an exception list
        def fail_all_but(exceptions, node = node, cases = cases,
                         switchType = switchType, environment = environment):
            for c in node.cases():
                for l in c.labels():
                    if l not in exceptions:
                        cases.out("case @label@: goto fail;",
                                  label = switchType.literal(l.value(),
                                                             environment))
                        

        # switch (currently active case){
        #
        outer_has_default = 0 # only mention default: once
        for c in node.cases():

            need_switch = 1

            # If the currently active case has only one non-default label,
            # then the only legal action is to set it to its current value.
            # We've already checked for this in an if (...) statement before
            # here.
            if len(c.labels()) == 1 and not c.labels()[0].default():
                cases.out("case @label@: goto fail;",
                          label = switchType.literal(c.labels()[0].value(),
                                                     environment))
                continue

            # output one C++ case label for each IDL case label
            # case 1:
            # case 2:
            # default:

            this_case_is_default = 0
            for l in c.labels():
                if l.default():
                    this_case_is_default = 1
                    outer_has_default = 1
                    cases.out("default:")
                    continue

                cases.out("case @label@:",
                          label = switchType.literal(l.value(), environment))

            # switch (case to switch to){
            #
            cases.inc_indent()
            cases.out("switch (_value){\n")
            cases.inc_indent()
            inner_has_default = 0


            # If we currently are in the default case, fail all attempts
            # to switch cases.
            if this_case_is_default:
                fail_all_but(c.labels())
                cases.out("default: _pd__d = _value; return;")
                cases.dec_indent()
                cases.out("}\n")
                cases.dec_indent()
                continue
                
            # This is not the default case, all possibilities have associated
            # UnionCaseLabels
            for l in c.labels():
                cases.out("case @label@: _pd__d = @label@; return;",
                          label = switchType.literal(l.value(), environment))

            
            cases.out("default: goto fail;")
            cases.dec_indent()
            cases.out("}\n")
            cases.dec_indent()
            
        if not outer_has_default and not implicitDefault:
            cases.out("default: goto fail;")

        # handle situation where have an implicit default member
        # (ie no actual case, but a legal set of discriminator values)
        # (assumes that the current discriminator is set to one of the
        # defaults)
        if implicitDefault:
            need_switch = 1
            cases.out("default:")
            cases.out("switch (_value){")
            cases.inc_indent()
            # again, make sure we aren't currently in the default state
            # and trying to set the discriminator to a non-default state
            fail_all_but([])

            cases.out("default: _pd__d = _value; return;")

            cases.dec_indent()
            cases.out("}")
                      

        # output the code here
        switch = output.StringStream()
        if need_switch:
            switch.out("switch (_pd__d){\n  @cases@\n};", cases = cases)
        stream.out(template.union_d_fn_body, switch = switch)
            

    # get and set functions for each case:
    def members(stream = stream, node = node, environment = environment,
                choose = chooseArbitraryDefault, switchType = switchType):
        for c in node.cases():
            # Following the typedef chain will deliver the base type of
            # the alias. Whether or not it is an array is stored in an
            # ast.Typedef node.
            caseType = types.Type(c.caseType())
            d_caseType = caseType.deref()

            # the mangled name of the member
            decl = c.declarator()
            decl_dims = decl.sizes()

            full_dims = decl_dims + caseType.dims()
            
            is_array = full_dims != []
            is_array_declarator = decl_dims != []
            alias_array = caseType.dims() != []

            member = id.mapID(decl.identifier())
            
            memtype = caseType.member(environment)
            
            # CORBA 2.3 C++ language mapping (June, 1999) 1-34:
            # ... Setting the union value through a modifier function
            # automatically sets the discriminant and may release the
            # storage associated with the previous value ... If a
            # modifier for a union member with multiple legal
            # discriminant values is used to set the value of the
            # discriminant, the union implementation is free to set
            # the discriminant to any one of the legal values for that
            # member. The actual discriminant value chose under these
            # circumstances is implementation-dependent. ...
            
            # Do we pick the first element (seems obvious)?
            # Or pick another one, hoping to trip-up people who write
            # non-compliant code and make an incorrect assumption?
            
            labels = c.labels()
            if labels != []:
                non_default_labels = filter(lambda x:not x.default(), labels)
                if non_default_labels == []:
                    # only one label and it's the default
                    label = labels[0]
                    discrimvalue = choose()
                elif len(non_default_labels) > 1:
                    # oooh, we have a choice. Let's pick the second one.
                    # no-one will be expecting that
                    label = non_default_labels[1]
                else:
                    # just the one interesting label
                    label = non_default_labels[0]

                if label.default():
                    discrimvalue = choose()
                else:
                    discrimvalue = switchType.literal(label.value(),
                                                      environment)

                # only different when array declarator
                const_type_str = memtype
                
                # anonymous arrays are handled slightly differently
                if is_array_declarator:
                    prefix = config.state['Private Prefix']
                    stream.out(template.union_array_declarator,
                               prefix = prefix,
                               memtype = memtype,
                               name = member,
                               dims = cxx.dimsToString(decl.sizes()),
                               tail_dims = cxx.dimsToString(decl.sizes()[1:]))
                    const_type_str = prefix + "_" + member
                    memtype = "_" + member
             
                if is_array:
                    # build the loop
                    def loop(stream = stream, full_dims = full_dims,
                             member = member):
                        loop = cxx.For(stream, full_dims)
                        index = loop.index()
                        stream.out("\n_pd_" + member + index + " = _value" +\
                                   index + ";\n")
                        loop.end()
                        return
                    
                    stream.out(template.union_array,
                               memtype = memtype,
                               const_type = const_type_str,
                               name = member,
                               isDefault = str(c.isDefault),
                               discrimvalue = discrimvalue,
                               loop = loop)
                    
                elif d_caseType.any():
                    # note type != CORBA::Any when its an alias...
                    stream.out(template.union_any,
                               type = memtype,
                               name = member,
                               isDefault = str(c.isDefault),
                               discrimvalue = discrimvalue)

                elif d_caseType.typecode():
                    stream.out(template.union_typecode,
                               name = member,
                               isDefault = str(c.isDefault),
                               discrimvalue = discrimvalue)
                                    
                elif isinstance(d_caseType.type(), idltype.Base) or \
                     d_caseType.enum():
                    # basic type
                    stream.out(template.union_basic,
                               type = memtype,
                               name = member,
                               isDefault = str(c.isDefault),
                               discrimvalue = discrimvalue)
                    
                elif d_caseType.string():
                    stream.out(template.union_string,
                               name = member,
                               isDefault = str(c.isDefault),
                               discrimvalue = discrimvalue)

                elif d_caseType.wstring():
                    stream.out(template.union_wstring,
                               name = member,
                               isDefault = str(c.isDefault),
                               discrimvalue = discrimvalue)

                elif d_caseType.interface():
                    scopedName = d_caseType.type().decl().scopedName()

                    name = id.Name(scopedName)
                    ptr_name = name.suffix("_ptr").unambiguous(environment)
                    Helper_name = name.suffix("_Helper").unambiguous(
                        environment)
                    var_name = name.suffix("_var").unambiguous(environment)

                    stream.out(template.union_objref,
                               member = member,
                               memtype = memtype,
                               ptr_name = ptr_name,
                               var_name = var_name,
                               Helper_name = Helper_name,
                               isDefault = str(c.isDefault),
                               discrimvalue = discrimvalue)

                elif caseType.typedef() or d_caseType.struct() or \
                     d_caseType.union() or d_caseType.fixed():
                    stream.out(template.union_constructed,
                               type = memtype,
                               name = member,
                               isDefault = str(c.isDefault),
                               discrimvalue = discrimvalue)

                elif d_caseType.sequence():
                    sequence_template = d_caseType.sequenceTemplate(environment)
                    stream.out(template.union_sequence,
                               sequence_template = sequence_template,
                               member = member,
                               isDefault = str(c.isDefault),
                               discrimvalue = discrimvalue)

                elif d_caseType.value() or d_caseType.valuebox():
                    scopedName = d_caseType.type().decl().scopedName()
                    name = id.Name(scopedName)
                    type = name.unambiguous(environment)

                    stream.out(template.union_value,
                               member=member,
                               type=type,
                               isDefault = str(c.isDefault),
                               discrimvalue = discrimvalue)
                    pass

                else:
                    util.fatalError("Unknown union case type encountered")
        return

    # declare the instance of the discriminator and
    # the actual data members (shock, horror)
    # FIXME: there is some interesting behaviour in
    # o2be_union::produce_hdr which I should examine more
    # carefully
    inside = output.StringStream()
    outside = output.StringStream()
    used_inside = 0
    used_outside = 0
    for c in node.cases():

        # find the dereferenced type of the member if its an alias
        caseType = types.Type(c.caseType())
        d_caseType = caseType.deref()

        decl = c.declarator()
        decl_dims = decl.sizes()

        full_dims = caseType.dims() + decl_dims
        
        is_array = full_dims != []
        is_array_declarator = decl_dims != []
        member_name = id.mapID(c.declarator().identifier())

        type_str = caseType.member(environment)

        # non-array sequences have had their template typedef'd somewhere
        if not is_array_declarator and caseType.sequence():
            type_str = "_" + member_name + "_seq"
        
        dims_str = cxx.dimsToString(decl_dims)

        # Decide what does inside and outside the union {} itself
        # Note: floats in unions are special cases
        if (d_caseType.float() or d_caseType.double()) and \
           not is_array:
            inside.out(template.union_noproxy_float,
                       type = type_str, name = member_name,
                       dims = dims_str)
            outside.out(template.union_proxy_float,
                       type = type_str, name = member_name,
                       dims = dims_str)
            used_inside = used_outside = 1
        else:
            if is_array and d_caseType.struct() and \
               not caseType.variable():
                this_stream = inside
                used_inside = 1
            else:
                if d_caseType.type().kind() in \
                   [ idltype.tk_struct,
                     idltype.tk_union,
                     idltype.tk_except,
                     idltype.tk_string,
                     idltype.tk_wstring,
                     idltype.tk_sequence,
                     idltype.tk_any,
                     idltype.tk_TypeCode,
                     idltype.tk_objref,
                     idltype.tk_value,
                     idltype.tk_value_box,
                     idltype.tk_abstract_interface,
                     idltype.tk_local_interface ]:
                                                 
                    this_stream = outside
                    used_outside = 1
                else:
                    this_stream = inside
                    used_inside = 1

            this_stream.out(template.union_member,
                            type = type_str,
                            name = member_name,
                            dims = dims_str)

    discrimtype = d_switchType.base(environment)
    
    if used_inside:
        _union = output.StringStream()
        _union.out(template.union_union, members = str(inside))
        inside = _union

    # write out the union class
    stream.out(template.union,
               unionname = cxx_id,
               fixed = fixed,
               Other_IDL = Other_IDL,
               default_constructor = default_constructor,
               copy_constructor = copy_constructor,
               discrimtype = discrimtype,
               _d_body = _d_fn,
               implicit_default = implicit_default,
               members = members,
               union = str(inside),
               outsideUnion = str(outside))

    if types.variableDecl(node):
        stream.out(template.union_variable_out_type,
                   unionname = cxx_id)
    else:
        stream.out(template.union_fix_out_type,
                   unionname = cxx_id)

    self.__insideClass = insideClass

    # TypeCode and Any
    if config.state['Typecode']:
        qualifier = const_qualifier(self.__insideModule, self.__insideClass)
        stream.out(template.typecode,
                   qualifier = qualifier,
                   name = cxx_id)

    return


def visitUnionForward(node):
    cxx_name = id.mapID(node.identifier())
    stream.out(template.union_forward, name = cxx_name)


def visitEnum(node):
    name = id.mapID(node.identifier())

    enumerators = node.enumerators()
    memberlist = map(lambda x: id.Name(x.scopedName()).simple(), enumerators)
    stream.out(template.enum,
               name = name,
               memberlist = string.join(memberlist, ", "))

    # TypeCode and Any
    if config.state['Typecode']:
        qualifier = const_qualifier(self.__insideModule, self.__insideClass)
        stream.out(template.typecode,
                   qualifier = qualifier, name = name)
    
    return

def visitValue(node):
    v = value.getValueType(node)
    v.module_decls(stream, self)

def visitValueAbs(node):
    v = value.getValueType(node)
    v.module_decls(stream, self)

def visitValueForward(node):
    v = value.getValueType(node)
    v.module_decls(stream, self)

def visitValueBox(node):
    v = value.getValueType(node)
    v.module_decls(stream, self)
