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
from omniidl_be.cxx import tyutil, util, config
from omniidl_be.cxx import id, types
from omniidl_be.cxx.header import template

import defs

self = defs

def __init__(stream):
    defs.stream = stream
    # Need to keep track of how deep within the AST we are
    # in a recursive procedure these would be extra arguments,
    # but the visitor pattern necessitates them being global.
    self.__insideInterface = 0
    self.__insideModule = 0
    self.__insideClass = 0
    # An entry in this hash indicates that an interface has
    # been declared- therefore any more AST forward nodes for
    # this interface are ignored.
    self.__interfaces = {}

    # When we first encounter a module, we deal with all the
    # continuations straight away. Therefore when we reencounter
    # a continuation later, we don't duplicate the definitions.
    self.__completedModules = {}

    return defs

#
# Control arrives here
#
def visitAST(node):
    self.__completedModules = {}
    for n in node.declarations():
        n.accept(self)

def visitModule(node):
    # This may be incorrect wrt reopened modules in multiple
    # files?
    if not(node.mainFile()):
        return

    # In case of continuations, don't output the definitions
    # more than once (by marking the module as done)
    if self.__completedModules.has_key(node):
        return
    self.__completedModules[node] = 1
    
    ident = node.identifier()
    cxx_id = id.mapID(ident)

    if not(config.FragmentFlag()):
        stream.out(template.module_begin, name = cxx_id)
        stream.inc_indent()

    # push self.__insideModule, true
    insideModule = self.__insideModule
    self.__insideModule = 1

    for n in node.definitions():
        n.accept(self)

    # deal with continuations
    for c in node.continuations():
        self.__completedModules[node] = 1
        for n in c.definitions():
            n.accept(self)

    # pop self.__insideModule
    self.__insideModule = insideModule
    
    if not(config.FragmentFlag()):
        stream.dec_indent()
        stream.out(template.module_end, name = cxx_id)

        

def visitInterface(node):
    if not(node.mainFile()):
        return

    # It's legal to have a forward interface declaration after
    # the actual interface definition. Make sure we ignore these.
    self.__interfaces[node] = 1

    name = node.identifier()
    cxx_name = id.mapID(name)

    outer_environment = id.lookup(node)
    environment = outer_environment.enter(name)
    scope = environment.scope()

    # push self.__insideInterface, true
    # push self.__insideClass, true
    insideInterface = self.__insideInterface
    self.__insideInterface = 1
    insideClass = self.__insideClass
    self.__insideClass = 1
    
    # the ifndef guard name contains flattened scope
    guard = id.Name(scope).guard()

    # make the necessary forward references, typedefs and define
    # the _Helper class
    stream.out(template.interface_begin,
               guard = guard,
               name = cxx_name)

    # recursively take care of other IDL declared within this
    # scope (evaluate function later- lazy eval though 'thunking')
    def Other_IDL(node = node, environment = environment):
        for n in node.declarations():
            n.accept(self)

    # Output the this interface's corresponding class
    stream.out(template.interface_type,
               name = cxx_name,
               Other_IDL = Other_IDL)
    
    # build methods corresponding to attributes, operations etc
    attributes = []
    operations = []
    virtual_operations = []
        
    for c in node.callables():
        if isinstance(c, idlast.Attribute):
            attrType = types.Type(c.attrType())
            
            for i in c.identifiers():
                attribname = id.mapID(i)
                attributes.append(attrType.op(types.RET, outer_environment) +\
                                  " " + attribname + "()")
                if not(c.readonly()):
                    attributes.append("void " + attribname + "("             +\
                                      attrType.op(types.IN,outer_environment)+\
                                      ")")
        elif isinstance(c, idlast.Operation):
            params = []
            virtual_params = []
            for p in c.parameters():
                paramType = types.Type(p.paramType())

                tuple = ("", "")
                direction = types.direction(p)
                tuple = (paramType.op(direction, outer_environment, use_out=1),
                         paramType.op(direction, outer_environment, use_out=0))

                argname = id.mapID(p.identifier())
                params.append(tuple[0] + " " + argname)
                virtual_params.append(tuple[1] + " " + argname)

            # deal with possible "context"
            if c.contexts() != []:
                params.append("CORBA::Context_ptr _ctxt")
                virtual_params.append("CORBA::Context_ptr _ctxt")

            return_type = types.Type(c.returnType()).op(types.RET,
                                                        outer_environment)

            opname = id.mapID(c.identifier())
            arguments = string.join(params, ", ")
            virtual_arguments = string.join(virtual_params, ", ")
            operations.append(return_type + " " + opname + \
                              "(" + arguments + ")")
            virtual_operations.append(return_type + " " + opname + \
                                      "(" + virtual_arguments + ")")
        else:
            raise "No code for interface member: " + repr(c)

    attributes_str = string.join(map(lambda x: x + ";\n", attributes ),"")
    operations_str = string.join(map(lambda x: x + ";\n", operations ),"")
        
    virtual_attributes_str = string.join(
        map( lambda x: "virtual " + x + " = 0;\n", attributes ), "")
    virtual_operations_str = string.join(
        map( lambda x: "virtual " + x + " = 0;\n", virtual_operations ), "")

    # deal with inheritance
    objref_inherits = []
    impl_inherits = []
    for i in node.inherits():

        name = id.Name(i.scopedName())
        ident = id.mapID(i.identifier())
        scope = name.scope()

        objref_scopedName = name.prefix("_objref_")
        impl_scopedName   = name.prefix("_impl_")
        objref_string     = objref_scopedName.unambiguous(environment)
        impl_string       = impl_scopedName.unambiguous(environment)

        objref_inherits.append("public virtual " + objref_string)
        impl_inherits.append("public virtual " + impl_string)

    # if already inheriting, the base classes will be present
    # (transitivity of the inherits-from relation)
    if node.inherits() == []:
        objref_inherits = [ "public virtual CORBA::Object, " + \
                            "public virtual omniObjRef" ]
        impl_inherits   = [ "public virtual omniServant" ]
            
    objref_inherits = string.join(objref_inherits, ",\n")
    impl_inherits = string.join(impl_inherits, ", \n")

    # Output the _objref_ class definition
    stream.out(template.interface_objref,
               name = cxx_name,
               inherits = objref_inherits,
               operations = operations_str,
               attributes = attributes_str)

    # Output the _pof_ class definition
    stream.out(template.interface_pof,
               name = cxx_name)

    # Output the _impl_ class definition
    stream.out(template.interface_impl,
               inherits = impl_inherits,
               virtual_operations = virtual_operations_str,
               virtual_attributes = virtual_attributes_str,
               name = cxx_name)

    # Generate BOA compatible skeletons?
    if config.BOAFlag():
        stream.out(template.interface_sk,
                   name = cxx_name)

    # pop self.__insideInterface
    # pop self.__insideClass
    self.__insideInterface = insideInterface
    self.__insideClass = insideClass

    # Typecode and Any
    if config.TypecodeFlag():
        qualifier = tyutil.const_qualifier(self.__insideModule,
                                           self.__insideClass)
        stream.out(template.typecode,
                   qualifier = qualifier,
                   name = cxx_name)
        
    return
    

def visitForward(node):
    if not(node.mainFile()):
        return
    
    # Note it's legal to have multiple forward declarations
    # of the same name. So ignore the duplicates.
    if self.__interfaces.has_key(node):
        return
    self.__interfaces[node] = 1
    
    environment = id.lookup(node)
    scope = environment.scope()
    cxx_id = id.mapID(node.identifier())
    name = id.Name(node.scopedName())
    guard = name.guard()

    # output the definition
    stream.out(template.interface_begin,
               guard = guard,
               name = name.unambiguous(environment))

def visitConst(node):
    if not(node.mainFile()):
        return

    environment = id.lookup(node)
    scope = environment.scope()

    constType = types.Type(node.constType())
    d_constType = constType.deref()
    if d_constType.string():
        type_string = "char *"
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
    if not(node.mainFile()):
        return

    environment = id.lookup(node)
    scope = environment.scope()
    
    is_global_scope = not(self.__insideModule or self.__insideInterface)
    
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
        if config.TypecodeFlag():
            qualifier = tyutil.const_qualifier(self.__insideModule,
                                               self.__insideClass)
            stream.out(template.typecode,
                       qualifier = qualifier,
                       name = derivedName)
                    
        # is it a simple alias (ie not an array at this level)?
        if not(array_declarator):
            # not an array declarator but a simple declarator to an array
            if aliasType.array():
                # simple alias to an array should alias all the
                # array handling functions, but we don't need to duplicate
                # array looping code since we can just call the functions
                # for the base type
                stream.out(template.typedef_simple_to_array,
                           base = basicReferencedTypeID,
                           derived = derivedName)
                # the declaration of the alloc(), dup() and free() methods
                # depend on whether the declaration is in global scope
                if not(is_global_scope):
                    stream.out(template.typedef_simple_to_array_static_fn,
                               base = basicReferencedTypeID,
                               derived = derivedName)
                else:
                    stream.out(template.typedef_simple_to_array_extern,
                               base = basicReferencedTypeID,
                               derived = derivedName)
                           
            # Non-array of string
            elif d_type.string():
                stream.out(template.typedef_simple_string,
                           name = derivedName)
            elif d_type.typecode():
                stream.out(template.typedef_simple_typecode,
                           name = derivedName)
            elif d_type.any():
                stream.out(template.typedef_simple_any,
                           name = derivedName)
            # Non-array of basic type
            elif isinstance(d_type.type(), idltype.Base):

                # typedefs to basic types are always fully qualified?
                # IDL oddity?
                basicReferencedTypeID = aliasType.member()
                stream.out(template.typedef_simple_basic,
                           base = basicReferencedTypeID,
                           derived = derivedName)
            # a typedef to a struct or union, or a typedef to a
            # typedef to a sequence
            elif d_type.struct() or d_type.union() or (d_type.sequence() and \
                 aliasType.typedef()):
                
                stream.out(template.typedef_simple_constructed,
                           base = basicReferencedTypeID,
                           name = derivedName)
                    
            # Non-array of objrect reference
            elif d_type.objref():
                # Note that the base name is fully flattened
                is_CORBA_Object = d_type.type().scopedName() ==\
                                  ["CORBA", "Object"]
                impl_base = ""
                objref_base = ""
                if not(is_CORBA_Object):
                    scopedName = d_type.type().decl().scopedName()
                    name = id.Name(scopedName)
                    impl_scopedName = name.prefix("_impl_")
                    objref_scopedName = name.prefix("_objref_")
                    impl_name = impl_scopedName.unambiguous(environment)
                    objref_name = objref_scopedName.unambiguous(environment)

                    impl_base = "typedef " + impl_name + " _impl_" +\
                                derivedName + ";"
                    objref_base = "typedef " + objref_name + " _objref_" + \
                                  derivedName + ";"

                stream.out(template.typedef_simple_objref,
                           base = derefTypeID,
                           name = derivedName,
                           impl_base = impl_base,
                           objref_base = objref_base)
            # Non-array of user declared types
            elif d_type.kind() in [ idltype.tk_struct, idltype.tk_union,
                                    idltype.tk_except, idltype.tk_enum ]:
                stream.out(template.typedef_simple_basic,
                           base = basicReferencedTypeID,
                           derived = derivedName)
            # Non-array of sequence
            elif d_type.sequence():
                seqType = types.Type(d_type.type().seqType())
                d_seqType = seqType.deref()
                bounded = d_type.type().bound()
                
                seq_dims = seqType.dims()

                templateName = d_type.sequenceTemplate(environment)

                if d_seqType.string():
                    element = "_CORBA_String_member"
                    element_IN = "char *"
                elif d_seqType.objref():
                    element = seqType.base(environment) + "_ptr"
                    element_IN = element
                # only if an anonymous sequence
                elif seqType.sequence():
                    element = d_seqType.sequenceTemplate(environment)
                    element_IN = element
                else:
                    element = seqType.base(environment)
                    element_IN = element
                    
                element_ptr = element_IN
                if d_seqType.string() and not(seqType.array()):
                    element_ptr = "char*"
                elif d_seqType.objref() and not(seqType.array()):
                    element_ptr = seqType.base(environment) + "_ptr"
                # only if an anonymous sequence
                elif seqType.sequence() and not(seqType.array()):
                    element_ptr = element
                elif d_seqType.typecode():
                    element_ptr = "CORBA::TypeCode_member"
                    element = element_ptr
                    element_IN = element_ptr
                else:
                    element_ptr = seqType.base(environment)
                    
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
                    
                if d_seqType.enum() and not(seqType.array()):
                    stream.out(template.typedef_enum_oper_friend,
                               element = d_seqType.base(environment),
                               friend = friend)
                        
                # derivedName is the new type identifier
                # element is the name of the basic element type
                # seq_dims contains dimensions if a sequence of arrays
                # templateName contains the template instantiation

                def bounds(bounded = bounded, derivedName = derivedName,
                           element_ptr = element_ptr,
                           templateName = templateName):
                    if not(bounded):
                        stream.out(template.sequence_type_bounds,
                                   name = derivedName,
                                   element = element_ptr,
                                   derived = templateName)
                        
                # output the main sequence definition
                stream.out(template.sequence_type,
                           name = derivedName,
                           derived = templateName,
                           bounds = bounds)
                

                # start building the _var and _out types
                element_reference = ""
                if not(aliasType.array()):
                    if d_seqType.string():
                        # special case alert
                        element_reference = element
                    elif d_seqType.objref():
                        element_reference = d_seqType.objRefTemplate("Member",
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
            dimsString = tyutil.dimsToString(d.sizes())
            taildims = tyutil.dimsToString(d.sizes()[1:])
            
            typestring = aliasType.member(environment)

            stream.out(template.typedef_array,
                       name = derivedName,
                       type = typestring,
                       dims = dimsString,
                       taildims = taildims)

            # if in global scope we define the functions as extern
            if is_global_scope:
                stream.out(template.typedef_array_extern,
                           name = derivedName)
            else:
                # build the _dup loop
                def dup_loop(stream = stream, all_dims = all_dims):
                    index = util.start_loop(stream, all_dims)
                    stream.out("\n_data@index@ = _s@index@;\n",
                               index = index)
                    util.finish_loop(stream, all_dims)

                # build the _copy loop
                def copy_loop(stream = stream, all_dims = all_dims):
                    index = util.start_loop(stream, all_dims)
                    stream.out("\n_to@index@ = _from@index@;\n", index = index)
                    util.finish_loop(stream, all_dims)

                # output the static functions
                stream.out(template.typedef_array_static,
                           name = derivedName,
                           firstdim = repr(all_dims[0]),
                           dup_loop = dup_loop,
                           copy_loop = copy_loop)                            
            # output the _copyHelper class
            stream.out(template.typedef_array_copyHelper,
                       name = derivedName)
                
     

def visitMember(node):
    if not(node.mainFile()):
        return
    
    memberType = node.memberType()
    if node.constrType():
        # if the type was declared here, it must be an instance
        # of idltype.Declared!
        assert isinstance(memberType, idltype.Declared)
        memberType.decl().accept(self)


def visitStruct(node):
    if not(node.mainFile()):
        return

    name = node.identifier()
    cxx_name = id.mapID(name)

    outer_environment = id.lookup(node)
    environment = outer_environment.enter(name)
    scope = environment.scope()
    
    insideClass = self.__insideClass
    self.__insideClass = 1
            
    type = "Fix"
    if types.variableDecl(node):
        type = "Variable"

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
                if not(is_array_declarator) and memberType.sequence():
                    stream.out(template.struct_nonarray_sequence,
                               memtype = memtype,
                               cxx_id = cxx_id)
                else:
                    stream.out(template.struct_normal_member,
                               memtype = memtype,
                               cxx_id = cxx_id,
                               dims = tyutil.dimsToString(decl_dims))
            
    # Output the structure itself
    stream.out(template.struct,
               name = cxx_name,
               type = type,
               Other_IDL = Other_IDL,
               members = members)
    
    self.__insideClass = insideClass

    # TypeCode and Any
    if config.TypecodeFlag():
        # structs in C++ are classes with different default privacy policies
        qualifier = tyutil.const_qualifier(self.__insideModule,
                                           self.__insideClass)
        stream.out(template.typecode,
                   qualifier = qualifier,
                   name = cxx_name)



def visitException(node):
    if not(node.mainFile()):
        return
    
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

                dims_string = tyutil.dimsToString(decl_dims)
                
                if is_array_declarator:
                    stream.out(template.exception_array_declarator,
                               memtype = memtype,
                               cxx_id = cxx_id,
                               dims = dims_string,
                               private_prefix = config.privatePrefix())

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
                ctor_arg_type = "const " + config.privatePrefix() +\
                                "_" + cxx_id
            ctor_args.append(ctor_arg_type + " i_" + cxx_id)

       
    ctor = ""
    if ctor_args != []:
        ctor = cxx_exname + "(" + string.join(ctor_args, ", ") + ");"
            
    if no_members:
        inline = "inline"
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
    if config.TypecodeFlag():
        qualifier = tyutil.const_qualifier(self.__insideModule,
                                           self.__insideClass)
        stream.out(template.typecode,
                   qualifier = qualifier,
                   name = cxx_exname)
    


def visitUnion(node):
    if not(node.mainFile()):
        return
    
    ident = node.identifier()

    cxx_id = id.mapID(ident)
    outer_environment = id.lookup(node)
    environment = outer_environment.enter(ident)
    scope = environment.scope()
    
    insideClass = self.__insideClass
    self.__insideClass = 1
    
    switchType = types.Type(node.switchType())
    d_switchType = switchType.deref()

    # in the case where there is no default case and an implicit default
    # member, choose a discriminator value to set. Note that attempting
    # to access the data is undefined
    #
    # FIXME: The new AST representation now has
    #   CaseLabel.default() : true if representing the default case
    #   CaseLabel.value()   : if the default case, returns a value
    #                         used by none of the other labels
    # For the time being, keep the independent mechanism because it seems
    # to closely match the old compiler's behaviour.
    # Once this backend is trusted independantly, convert to use the
    # new mechanism?
    def chooseArbitraryDefault(switchType = switchType,
                               allCases = tyutil.allCases(node),
                               environment = environment):
        # dereference the switch_type (ie if CASE <scoped_name>)
        switchType = switchType.deref()
        # get the values from the cases
        values = map(lambda x: x.value(), allCases)

        # for integer types, find the lowest unused number
        def min_unused(start, used = values):
            x = start
            while x in used:
                x = x + 1
            return x
                
        # CASE <integer_type>
        kind = switchType.type().kind()
        if kind == idltype.tk_short:
            short_min = -32767 # - [ 2 ^ (32-1) -1 ]
            return str(min_unused(short_min))
        
        elif kind == idltype.tk_long:
            long_min = -2147483647 # - [ 2 ^ (64-1) -1 ]
            return str(min_unused(long_min))
        
        elif kind in [ idltype.tk_ushort, idltype.tk_longlong,
                       idltype.tk_ulong, idltype.tk_ulonglong ]:
            # unsigned values start at 0
            return str(min_unused(0))
            
        # CASE <char_type>
        elif kind == idltype.tk_char:
            # choose the first one not already used
            possibles = map(chr, range(0, 255))
            difference = util.minus(possibles, values)
            # FIXME: stupid special case. An explicit discriminator
            # value of \0 -> 0000 whereas an implicit one (valueString)
            # \0 -> '\000'
            if difference[0] == '\0':
                return "'\\000'"
            
            return switchType.literal(difference[0])

        # CASE <boolean_type>
        elif kind == idltype.tk_boolean:
            return "0"
        
        # CASE <enum_type>
        elif kind == idltype.tk_enum:
            enums = switchType.type().decl().enumerators()
            # pick the first enum not already in a case
            difference = util.minus(enums, values)
            scopedName = id.Name(difference[0].scopedName())
            # need to be careful of scope
            return scopedName.unambiguous(environment)
        else:
            util.fatalError("Failed to generate a default union " +\
                            "discriminator value")

    # does the IDL union have any default case?
    # It'll be handy to know which case is the default one later-
    # so add a new attribute to mark it
    hasDefault = tyutil.getDefaultCaseAndMark(node) != None
        
    # CORBA 2.3 C++ Mapping 1-34
    # "A union has an implicit default member if it does not have
    # a default case and not all permissible values of the union
    # discriminant are listed"
    exhaustive = tyutil.exhaustiveMatch(switchType, tyutil.allCases(node))
    implicitDefault = not(hasDefault) and not(exhaustive)

    fixed = "Fix"
    if types.variableDecl(node):
        fixed = "Variable"

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
        if not(exhaustive):
            # grab the default case
            default = ""
            for c in node.cases():
                if c.isDefault:
                    case_id = c.declarator().identifier()
                    cxx_case_id = id.mapID(case_id)
                    default = cxx_case_id + "(_value.pd_" + cxx_case_id + ");"


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
                non_default_labels = filter(lambda x:not(x.default()), labels)
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

                # FIXME: stupid special case, see above
                if switchType.char() and label.value() == '\0':
                    discrimvalue = "0000"

                # only different when array declarator
                const_type_str = memtype
                
                # anonymous arrays are handled slightly differently
                if is_array_declarator:
                    prefix = config.privatePrefix()
                    stream.out(template.union_array_declarator,
                               prefix = prefix,
                               memtype = memtype,
                               name = member,
                               dims = tyutil.dimsToString(decl.sizes()),
                               tail_dims = tyutil.dimsToString(decl.sizes()[1:]))
                    const_type_str = prefix + "_" + member
                    memtype = "_" + member
             
                if is_array:
                    # build the loop
                    def loop(stream = stream, full_dims = full_dims,
                             member = member):
                        index = util.start_loop(stream, full_dims)
                        stream.out("\npd_" + member + index + " = _value" +\
                                   index + ";\n")
                        util.finish_loop(stream, full_dims)
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
                elif d_caseType.objref():
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
                     d_caseType.union():
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

                else:
                    util.fatalError("Unknown union case type encountered")
        return

    # Typecode and Any
    def tcParser_unionHelper(stream = stream, node = node):
        if config.TypecodeFlag():
            guard_name = id.Name(node.scopedName()).guard()
            stream.out(template.union_tcParser_friend,
                       name = guard_name,
                       private_prefix = config.privatePrefix())


    # declare the instance of the discriminator and
    # the actual data members (shock, horror)
    # FIXME: there is some interesting behaviour in
    # o2be_union::produce_hdr which I should examine more
    # carefully
    inside = util.StringStream()
    outside = util.StringStream()
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
        if not(is_array_declarator) and caseType.sequence():
            type_str = "_" + member_name + "_seq"
        
        dims_str = tyutil.dimsToString(decl_dims)

        # Decide what does inside and outside the union {} itself
        # Note: floats in unions are special cases
        if (d_caseType.float() or d_caseType.double()) and \
           not(is_array):
            inside.out(template.union_noproxy_float,
                       type = type_str, name = member_name,
                       dims = dims_str)
            outside.out(template.union_proxy_float,
                       type = type_str, name = member_name,
                       dims = dims_str)
            used_inside = used_outside = 1
        else:
            if is_array and d_caseType.struct() and \
               not(caseType.variable()):
                this_stream = inside
                used_inside = 1
            else:
                if d_caseType.type().kind() in \
                   [ idltype.tk_struct, idltype.tk_union,
                     idltype.tk_except, idltype.tk_string,
                     idltype.tk_sequence, idltype.tk_any,
                     idltype.tk_TypeCode, idltype.tk_objref ]:
                                                 
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
        _union = util.StringStream()
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
               implicit_default = implicit_default,
               members = members,
               tcParser_unionHelper = tcParser_unionHelper,
               union = str(inside),
               outsideUnion = str(outside))
               

    self.__insideClass = insideClass

    # TypeCode and Any
    if config.TypecodeFlag():
        qualifier = tyutil.const_qualifier(self.__insideModule,
                                           self.__insideClass)
        stream.out(template.typecode,
                   qualifier = qualifier,
                   name = cxx_id)

    return


def visitEnum(node):
    if not(node.mainFile()):
        return

    name = id.mapID(node.identifier())

    enumerators = node.enumerators()
    memberlist = map(lambda x: id.Name(x.scopedName()).simple(), enumerators)
    stream.out(template.enum,
               name = name,
               memberlist = string.join(memberlist, ", "))

    # TypeCode and Any
    if config.TypecodeFlag():
        insideModule = self.__insideModule
        insideClass = self.__insideClass
        qualifier = tyutil.const_qualifier(insideModule, insideClass)
        stream.out(template.typecode,
                   qualifier = qualifier, name = name)
    
    return
