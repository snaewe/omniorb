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
#   Produce the main skeleton definitions

# $Id$
# $Log$
# Revision 1.27  2000/02/01 09:26:49  djs
# Tracking fixes in old compiler: powerpc-aix scoped identifier workarounds
#
# Revision 1.26  2000/01/19 17:05:15  djs
# Modified to use an externally stored C++ output template.
#
# Revision 1.25  2000/01/19 11:21:52  djs
# *** empty log message ***
#
# Revision 1.24  2000/01/17 17:05:38  djs
# Marshalling and constructed types fixes
#
# Revision 1.23  2000/01/14 15:57:20  djs
# Added MSVC workaround for interface inheritance
#
# Revision 1.22  2000/01/13 17:02:05  djs
# Added support for operation contexts.
#
# Revision 1.21  2000/01/13 15:56:43  djs
# Factored out private identifier prefix rather than hard coding it all through
# the code.
#
# Revision 1.20  2000/01/13 14:16:34  djs
# Properly clears state between processing separate IDL input files
#
# Revision 1.19  2000/01/12 17:48:34  djs
# Added option to create BOA compatible skeletons (same as -BBOA in omniidl3)
#
# Revision 1.18  2000/01/11 14:13:23  djs
# Updated array mapping to include NAME_copy(to, from) as per 2.3 spec
#
# Revision 1.17  2000/01/11 12:02:45  djs
# More tidying up
#
# Revision 1.16  2000/01/10 18:42:22  djs
# Removed redundant code, tidied up.
#
# Revision 1.15  2000/01/10 15:39:48  djs
# Better name and scope handling.
#
# Revision 1.14  2000/01/07 20:31:33  djs
# Regression tests in CVSROOT/testsuite now pass for
#   * no backend arguments
#   * tie templates
#   * flattened tie templates
#   * TypeCode and Any generation
#
# Revision 1.13  1999/12/25 21:47:18  djs
# Better TypeCode support
#
# Revision 1.12  1999/12/24 18:18:32  djs
# #include bug fixed
#
# Revision 1.11  1999/12/16 16:11:21  djs
# Now uses transitive closure of inherits relation where appropriate
#
# Revision 1.10  1999/12/14 11:53:22  djs
# Support for CORBA::TypeCode and CORBA::Any
# Exception member bugfix
#
# Revision 1.9  1999/12/09 20:40:57  djs
# Bugfixes and integration with dynskel/ code
#
# Revision 1.8  1999/11/29 19:27:05  djs
# Code tidied and moved around. Some redundant code eliminated.
#
# Revision 1.7  1999/11/29 15:27:53  djs
# Minor bugfixes
#
# Revision 1.6  1999/11/26 18:51:13  djs
# Now uses proxy module for most proxy generation
#
# Revision 1.5  1999/11/23 18:48:25  djs
# Bugfixes, more interface operations and attributes code
#
# Revision 1.4  1999/11/19 20:12:03  djs
# Generates skeletons for interface operations and attributes
#
# Revision 1.3  1999/11/17 20:37:23  djs
# Code for call descriptors and proxies
#
# Revision 1.2  1999/11/15 19:13:38  djs
# Union skeletons working
#
# Revision 1.1  1999/11/12 17:18:58  djs
# Struct skeleton code added
#

"""Produce the main skeleton definitions"""
# similar to o2be_root::produce_skel in the old C++ BE

import string

from omniidl import idlast, idltype, idlutil
from omniidl.be.cxx import tyutil, util, name, env, config, skutil
from omniidl.be.cxx.skel import mangler, dispatch, proxy
from omniidl.be.cxx.skel import template

import main
self = main


def __init__(stream):
    self.stream = stream
    self.__globalScope = name.globalScope()
    self.__insideInterface = 0
    self.__insideModule = 0
    return self

# ------------------------------------
# Control arrives here

def visitAST(node):
    for n in node.declarations():
        # check what happens with reopened modules
        if n.mainFile():
            n.accept(self)

def visitModule(node):
    insideModule = self.__insideModule
    self.__insideModule = 1
    for n in node.definitions():
        n.accept(self)

    self.__insideModule = insideModule


def visitInterface(node):
    id = node.identifier()
    cxx_id = tyutil.mapID(id)
    
    outer_environment = env.lookup(node)
    environment = outer_environment.enterScope(id)

    insideInterface = self.__insideInterface
    self.__insideInterface = 1

    # produce skeletons for types declared here
    for n in node.declarations():
        n.accept(self)

    self.__insideInterface = insideInterface

    scopedName = map(tyutil.mapID, node.scopedName())
    fqname = string.join(scopedName, "::")
    id = environment.nameToString(scopedName)

    # we need to generate several useful classes for object
    # references
    objref_scopedName = tyutil.scope(scopedName) + \
                        ["_objref_" + tyutil.name(scopedName)]
    
    objref_fqname = string.join(map(tyutil.mapID, objref_scopedName), "::")
    objref_name = environment.nameToString(environment.relName(objref_scopedName))

    impl_scopedName = tyutil.scope(scopedName) + \
                      ["_impl_" + tyutil.name(scopedName)]
    impl_fqname = string.join(map(tyutil.mapID, impl_scopedName), "::")
    impl_name = environment.nameToString(environment.relName(impl_scopedName))

    sk_scopedName = tyutil.scope(scopedName) + \
                    ["_sk_" + tyutil.name(scopedName)]
    sk_fqname = string.join(map(tyutil.mapID, sk_scopedName), "::")
    sk_name = environment.nameToString(environment.relName(sk_scopedName))


    # build the helper class methods
    stream.out(template.interface_Helper,
               name = id)

    # the class itself
    stream.out(template.interface_class,
               name = id,
               objref_name = objref_name,
               repoID = tyutil.mapRepoID(node.repoId()))

    # gather information for possible interface inheritance
    # (needs to use the transitive closure of inheritance)
    all_inherits = tyutil.allInherits(node)
    inherits_str = ""
    inherited_repoIDs = ""
    for i in all_inherits:
        inherits_scopedName = map(tyutil.mapID, i.scopedName())
        inherits_name = string.join(inherits_scopedName, "::")

        inherited_repoIDs = inherited_repoIDs + "\
        if( !strcmp(id, " + inherits_name + "::_PD_repoId) )\n\
          return (" + inherits_name + "_ptr) this;\n"

    for i in node.inherits():
        inherits_scopedName = map(tyutil.mapID, i.scopedName())        
        inherits_objref_scopedName =  tyutil.scope(inherits_scopedName) + \
                                     ["_objref_" + \
                                      tyutil.name(inherits_scopedName)]
        inherits_objref_relName = environment.relName(inherits_objref_scopedName)
        inherits_objref_name = environment.nameToString(inherits_objref_relName)

        this_inherits_str = inherits_objref_name + "(mdri, p, id, lid),\n"
        
        # powerpc-aix workaround
        inherits_scope_prefix = tyutil.scope(inherits_objref_scopedName)
        if inherits_scope_prefix != []:
            inherits_scope_prefix = string.join(inherits_scope_prefix, "::") + "::"
            this_inherits_str = "OMNIORB_BASE_CTOR(" + inherits_scope_prefix + ")" + \
                                this_inherits_str
        
        inherits_str = inherits_str + this_inherits_str
        


        

    # generate the _objref_ methods
    stream.out(template.interface_objref,
               name = id, fq_objref_name = objref_fqname,
               objref_name = objref_name, inherits_str = inherits_str,
               inherited_repoIDs = inherited_repoIDs)

    # deal with callables (attributes and interfaces)
    callables = node.callables()
    attributes = filter(lambda x:isinstance(x, idlast.Attribute), callables)
    operations = filter(lambda x:isinstance(x, idlast.Operation), callables)
    scopedName = node.scopedName()

    # every callable has an associated call descriptor proxy.
    Proxy = proxy.__init__(environment, stream)

    # ------------------------------------
    # Operations
    for operation in operations:
        operationName = operation.identifier()
        cxx_operationName = tyutil.mapID(operationName)
        
        seed = scopedName + [operation.identifier()]

        # try the all new proxy code!
        Proxy.operation(operation, seed)
        descriptor = mangler.operation_descriptor_name(operation)

        parameters = operation.parameters()
        parameters_ID  = map(lambda x:tyutil.mapID(x.identifier()), parameters)
        
        parameter_argmapping = []

        returnType = operation.returnType()
        result_type = tyutil.operationArgumentType(returnType,
                                                   self.__globalScope,
                                                   virtualFn = 0)[0]
        has_return_value = not(tyutil.isVoid(returnType))

        # compute the argument mapping for the operation parameters
        for parameter in parameters:
            paramType = parameter.paramType()
            deref_paramType = tyutil.deref(paramType)
            param_dims = tyutil.typeDims(paramType)
            is_array = param_dims != []
            paramType_name = self.__globalScope.principalID(paramType)
            
            optypes = tyutil.operationArgumentType(paramType,
                                                   outer_environment)
            # optypes[0] is return [1] is in [2] is out [3] is inout
            parameter_argmapping.append(optypes[parameter.direction() + 1])

        # deal with possible "context"s
        if operation.contexts() != []:
            # pinch the unique name first (!)
            context_descriptor = mangler.generate_unique_name(
                mangler.CTX_DESC_PREFIX)
            contexts = util.StringStream()
            for context in operation.contexts():
                contexts.out("\"" + context + "\",")
            contexts.out("0")
            stream.out(template.interface_context_array,
                       context_descriptor = context_descriptor,
                       contexts = str(contexts))

        # static call back function
        local_call_descriptor = mangler.generate_unique_name(
            mangler.LCALL_DESC_PREFIX)
        impl_args = map(lambda x: "tcd->arg_" + str(x),
                        range(0, len(parameters)))

        if operation.contexts() != []:
            impl_args.append("cd->context_info()->context")

        result_string = ""
        if has_return_value:
            result_string = "tcd->pd_result = "
        stream.out(template.interface_callback,
                   local_call_descriptor = local_call_descriptor,
                   call_descriptor = descriptor,
                   impl_name = impl_name,
                   impl_fqname = impl_fqname,
                   name = id,
                   cxx_operation_name = cxx_operationName,
                   operation_arguments = string.join(impl_args, ", "),
                   result = result_string)

        # objref::operation name
        objref_args = util.zip(parameter_argmapping, parameters_ID)
        objref_args = map(lambda (x,y): x + " " + y, objref_args)
        call_desc_args = [local_call_descriptor, "\"" + operationName + "\"",
                          str(len(operationName) + 1)]
        if operation.oneway():
            call_desc_args.append("1/*oneway*/")
        else:
            call_desc_args.append("0")

        call_desc_args = call_desc_args + parameters_ID

        return_string = ""
        if has_return_value:
            return_string = "return _call_desc.result();"

        context = util.StringStream()
        if operation.contexts() != []:
            objref_args.append("CORBA::Context_ptr _ctxt")
            context.out("""\
  omniCallDescriptor::ContextInfo _ctxt_info(_ctxt, @context_descriptor@, @n@);
  _call_desc.set_context_info(&_ctxt_info);""",
                        context_descriptor = context_descriptor,
                        n = str(len(operation.contexts())))


        stream.out(template.interface_operation,
                   result_type = result_type,
                   objref_fqname = objref_fqname,
                   operation_name = cxx_operationName,
                   arguments = string.join(objref_args, ", "),
                   call_descriptor = descriptor,
                   call_desc_args = string.join(call_desc_args, ", "),
                   context = str(context),
                   return_string = return_string)
                   
    # ------------------------------------
    # Attributes
    for attribute in attributes:
        seed = scopedName + [attribute.identifiers()[0]]

        Proxy.attribute(attribute, seed)

        read = mangler.attribute_read_descriptor_name(attribute)
        write = mangler.attribute_write_descriptor_name(attribute)
        attrType = attribute.attrType()
        attr_dims = tyutil.typeDims(attrType)
        is_array = attr_dims != []
        
        deref_attrType = tyutil.deref(attrType)
        attrType_name = environment.principalID(attrType)

        # we need the type with and without its full scope
        attrTypes = tyutil.operationArgumentType(attrType,
                                                 self.__globalScope, 0)
        scoped_attrTypes = tyutil.operationArgumentType(attrType,
                                                        outer_environment,
                                                        0)
        return_type = attrTypes[0]

        # FIXME: change to the old compiler might make this always use
        # fully scoped names
        if is_array:
            scoped_in_type = attrTypes[1]+"_slice*"
        else:
            scoped_in_type = attrTypes[1]


        for ident in attribute.identifiers():
            attrib_name = ident
            cxx_attrib_name = tyutil.mapID(attrib_name)

            get_attrib_name = "_get_" + attrib_name
            local_call_descriptor = mangler.generate_unique_name(
                mangler.LCALL_DESC_PREFIX)

            # generate the callback
            stream.out(template.interface_callback,
                       local_call_descriptor = local_call_descriptor,
                       call_descriptor = read,
                       impl_fqname = impl_fqname,
                       name = id,
                       result = "tcd->pd_result = ",
                       cxx_operation_name = cxx_attrib_name,
                       operation_arguments = "")
            # generate the objref_method
            call_desc_args = local_call_descriptor + ", \"" +\
                             get_attrib_name + "\", " +\
                             str(len(get_attrib_name) + 1) + ", 0"
            stream.out(template.interface_operation,
                       result_type = return_type,
                       objref_fqname = objref_fqname,
                       operation_name = cxx_attrib_name,
                       arguments = "",
                       call_descriptor = read,
                       call_desc_args = call_desc_args,
                       context = "",
                       return_string = "return _call_desc.result();")

            if not(attribute.readonly()):
                # make another one of these
                local_call_descriptor = mangler.generate_unique_name(
                    mangler.LCALL_DESC_PREFIX)
                set_attrib_name = "_set_" + attrib_name
                # generate the callback
                stream.out(template.interface_callback,
                           local_call_descriptor = local_call_descriptor,
                           call_descriptor = write,
                           impl_fqname = impl_fqname,
                           name = id,
                           result = "",
                           cxx_operation_name = cxx_attrib_name,
                           operation_arguments = "tcd->arg_0")

                # generate the objref_method
                call_desc_args = local_call_descriptor + ", \"" + \
                                 set_attrib_name + "\", " + \
                                 str(len(set_attrib_name) + 1) + ", 0, arg_0"

                stream.out(template.interface_operation,
                           result_type = "void",
                           objref_fqname = objref_fqname,
                           operation_name = cxx_attrib_name,
                           arguments = scoped_in_type + " arg_0",
                           call_descriptor = write,
                           call_desc_args = call_desc_args,
                           context = "",
                           return_string = "")

    # _pof_ class
    pof_scopedName = map(tyutil.mapID, tyutil.scope(scopedName)) + \
                      ["_pof_" + tyutil.mapID(tyutil.name(scopedName))]
    pof_name = string.join(pof_scopedName, "::")
    u_name = tyutil.mapID(tyutil.name(scopedName))

    # build the inheritance list
    inherits_repoIDs = util.StringStream()
    for i in all_inherits:
        ancestor = string.join(map(tyutil.mapID, i.scopedName()), "::")
        inherits_repoIDs.out(template.interface_pof_repoID,
                             inherited = ancestor)
    stream.out(template.interface_pof,
               pof_name = pof_name,
               objref_fqname = objref_fqname,
               name = id,
               uname = u_name,
               Other_repoIDs = str(inherits_repoIDs),
               idname = mangler.produce_idname(scopedName))

    # comment copied from src/tool/omniidl2/omniORB2_be/o2be_interface.cc:

    # MSVC {4.2,5.0} cannot deal with a call to a virtual member
    # of a base class using the member function's fully/partially
    # scoped name. Have to use the alias for the base class in the
    # global scope to refer to the virtual member function instead.
    #
    # We scan all the base interfaces to see if any of them has to
    # be referred to by their fully/partially qualified names. If
    # that is necessary, we generate a typedef to define an alias for
    # this base interface. This alias is used in the stub generated below
    for i in node.inherits():
       i_scopedName = i.scopedName()
       relName = environment.relName(i_scopedName)
       # does this name have scope :: qualifiers?
       if len(relName) > 1:
          i_guard_name = tyutil.guardName(i_scopedName)
          i_fqname = self.__globalScope.nameToString(i_scopedName)
          i_flat_fqname = string.join(i_scopedName, "_")
          i_impl_scopedName = tyutil.scope(i_scopedName) + \
                              ["_impl_" + tyutil.name(i_scopedName)]
          i_impl_fqname = self.__globalScope.nameToString(i_impl_scopedName)
          i_impl_flat_fqname = string.join(i_impl_scopedName, "_")

          stream.out(template.interface_ALIAS,
                     guard_name = i_guard_name,
                     fqname = i_fqname,
                     flat_fqname = i_flat_fqname,
                     impl_fqname = i_impl_fqname,
                     impl_flat_fqname = i_impl_flat_fqname)
          
    # _impl_ class (contains the callable dispatch code)

    # dispatch operations and attributes from this class
    this_dispatch = util.StringStream()
    dispatcher = dispatch.__init__(environment, this_dispatch)
    for callable in node.callables():
        # This isn't quite as neat as it could be
        if isinstance(callable, idlast.Operation):
            identifiers = [callable.identifier()]
        else:
            identifiers = callable.identifiers()

        # separate case for each callable thing
        for ident in identifiers:
            id_name = tyutil.mapID(ident)
            if isinstance(callable, idlast.Operation):
                dispatcher.operation(callable)

            elif isinstance(callable, idlast.Attribute):
                dispatcher.attribute_read(callable, ident)

                if not(callable.readonly()):
                    dispatcher.attribute_write(callable, ident)

    # dispatch operations and attributes inherited from base classes
    inherited_dispatch = util.StringStream()
    for i in node.inherits():
       inherited_name = tyutil.mapID(tyutil.name(i.scopedName()))
       impl_inherits = "_impl_" + inherited_name
       # The MSVC workaround might be needed here again
       i_scopedName = i.scopedName()
       relName = environment.relName(i_scopedName)
       # does this name have scope :: qualifiers?
       if len(relName) > 1:
          i_impl_scopedName = tyutil.scope(i_scopedName) + \
                              ["_impl_" + tyutil.name(i_scopedName)]
          impl_inherits = string.join(i_impl_scopedName, "_")
          
       inherited_dispatch.out(template.interface_impl_inherit_dispatch,
                              impl_inherited_name = impl_inherits)

    Other_repoIDs = util.StringStream()
    # it's a bit odd that it will fully qualify the current class name
    # but it will strip off as much scope as possible for the inherited
    # ones
    for i in all_inherits:
       i_scopedName = i.scopedName()
       i_rel_scopedName = environment.relName(i_scopedName)
       i_impl_name = name.prefixName(i_rel_scopedName, "_impl_")
       inherited_name = environment.nameToString(i_rel_scopedName)
       
       Other_repoIDs.out(template.interface_impl_repoID,
                  inherited_name = inherited_name,
                  impl_inherited_name = i_impl_name)


    # Output the _impl_ class
    stream.out(template.interface_impl,
               impl_fqname = impl_fqname,
               uname = u_name,
               this_dispatch = str(this_dispatch),
               inherited_dispatch = str(inherited_dispatch),
               impl_name = impl_name,
               Other_repoIDs = str(Other_repoIDs),
               name = id)
               
    
    # BOA compatible skeletons
    if config.BOAFlag():
        stream.out(template.interface_sk,
                   sk_fqname = sk_fqname,
                   sk_name = sk_name)

    #leave()

def visitTypedef(node):
    environment = env.lookup(node)
    is_global_scope = not(self.__insideModule or self.__insideInterface)

    aliasType = node.aliasType()
    alias_dims = tyutil.typeDims(aliasType)

    if node.constrType():
        aliasType.decl().accept(self)

    fq_aliased = environment.principalID(aliasType)

    for d in node.declarators():
        # record in the environment
        environment.add(tyutil.name(d.scopedName()))
        
        decl_dims = d.sizes()
        decl_dims_str = tyutil.dimsToString(decl_dims)
        decl_first_dim_str = ""
        if decl_dims != []:
            decl_first_dim_str = tyutil.dimsToString([decl_dims[0]])
        
        full_dims = decl_dims + alias_dims
        is_array = full_dims != []
        is_array_declarator = decl_dims != []
        
        fq_derived = idlutil.ccolonName(map(tyutil.mapID, d.scopedName()))

        if is_global_scope and is_array_declarator:
            # build _dup and _copy loops
            dup_loop = util.StringStream()
            copy_loop = util.StringStream()
            index = util.start_loop(dup_loop, full_dims,
                                    iter_type = "unsigned int")
            dup_loop.out("_data" + index + " = _s" + index + ";")
            util.finish_loop(dup_loop, full_dims)
            index = util.start_loop(copy_loop, full_dims,
                                        iter_type = "unsigned int")
            copy_loop.out("_to" + index + " = _from" + index + ";")
            util.finish_loop(copy_loop, full_dims)

            stream.out(template.typedef_global_array_declarator,
                       fq_derived = fq_derived,
                       decl_dims_str = decl_dims_str,
                       decl_first_dim_str = decl_first_dim_str,
                       dup_loop = str(dup_loop),
                       copy_loop = str(copy_loop))

        elif is_global_scope and is_array:
            stream.out(template.typedef_global_simple_array,
                       fq_derived = fq_derived,
                       fq_aliased = fq_aliased)
            
    
    pass

def visitEnum(node):
    return

def visitMember(node):
    memberType = node.memberType()
    if node.constrType():
        # if the type was declared here, it must be an instance
        # of idltype.Declared!
        assert isinstance(memberType, idltype.Declared)
        memberType.decl().accept(self)
        
def visitStruct(node):

    outer_environment = env.lookup(node)
    
    name = map(tyutil.mapID, node.scopedName())
    name = string.join(name, "::")

    size_calculation = "omni::align_to(_msgsize, omni::ALIGN_4) + 4"

    marshall = util.StringStream()
    Mem_unmarshall = util.StringStream()
    Net_unmarshall = util.StringStream()
    msgsize = util.StringStream()


    for n in node.members():
        n.accept(self)
        memberType = n.memberType()
        type_dims = tyutil.typeDims(memberType)
        
        for d in n.declarators():
            decl_dims = d.sizes()
            full_dims = decl_dims + type_dims
            is_array = full_dims != []
            # marshall and unmarshall the struct members
            member_name = tyutil.mapID(tyutil.name(d.scopedName()))

            skutil.marshall_struct_union(marshall, outer_environment,
                                         memberType, d, member_name)
            skutil.unmarshall_struct_union(Mem_unmarshall, outer_environment,
                                           memberType, d, member_name, 0)
            skutil.unmarshall_struct_union(Net_unmarshall, outer_environment,
                                           memberType, d, member_name, 1)
            # computation of aligned size
            size = skutil.sizeCalculation(outer_environment, memberType, d,
                                          "_msgsize", member_name)
            msgsize.out(size)
            
            
            
    stream.out(template.struct,
               name = name,
               size_calculation = str(msgsize),
               marshall_code = str(marshall),
               mem_unmarshall_code = str(Mem_unmarshall),
               net_unmarshall_code = str(Net_unmarshall))

    stream.reset_indent()
    
def visitUnion(node):
    outer_environment = env.lookup(node)
    environment = outer_environment.enterScope(node.identifier())
    
    name = map(tyutil.mapID, node.scopedName())
    name = string.join(name, "::")

    switchType = node.switchType()
    deref_switchType = tyutil.deref(switchType)

    exhaustive = tyutil.exhaustiveMatch(switchType,
                                        tyutil.allCases(node))
    defaultCase = tyutil.getDefaultCaseAndMark(node)
    if defaultCase:
        defaultLabel = tyutil.getDefaultLabel(defaultCase)
        defaultMember = tyutil.name(map(tyutil.mapID,
                                        defaultCase.declarator().scopedName()))
        
    hasDefault = defaultCase != None

    # Booleans are a special case (isn't everything?)
    booleanWrap = tyutil.isBoolean(switchType) and exhaustive


    # deal with types constructed here
    if node.constrType():
        node.switchType().decl().accept(self)
    for n in node.cases():
        if n.constrType():
            n.caseType().decl().accept(self)

    # --------------------------------------------------------------
    # union::_NP_alignedSize(size_t initialoffset) const
    #
    discriminator_size_calc = skutil.sizeCalculation(environment,
                                                     switchType,
                                                     None,
                                                     "_msgsize", "")

    # build the switch case in the alignedSize method
    # build the cases...
    cases = util.StringStream()
    for c in node.cases():
        caseType = c.caseType()
        deref_caseType = tyutil.deref(caseType)
        decl = c.declarator()
        decl_name =  tyutil.name(map(tyutil.mapID, decl.scopedName()))
        for l in c.labels():
            # default case was already taken care of
            if not(l.default()):
                 discrim_value = tyutil.valueString(switchType, l.value(),
                                                    environment)
                 # FIXME: stupid special case. An explicit discriminator
                 # value of \0 -> 0000 whereas an implicit one (valueString)
                 # \0 -> '\000'
                 if tyutil.isChar(switchType) and l.value() == '\0':
                    discrim_value = "0000"
                    
                 cases.out("case " + str(discrim_value) + ":")
                 size_calc = skutil.sizeCalculation(environment, caseType,
                                                    decl, "_msgsize",
                                                    "pd_" + decl_name)
                 cases.out(size_calc)
                 cases.out("break;")
    if booleanWrap:
        cases.out(template.union_default_bool)
    else:
        cases.out(template.union_default)

    # build the switch itself
    switch = util.StringStream()
    if not(exhaustive):
        size_calc = ""
        if hasDefault:
            caseType = defaultCase.caseType()
            decl = defaultCase.declarator()
            size_calc = skutil.sizeCalculation(environment, caseType,
                                               decl, "_msgsize",
                                               "pd_" + defaultMember)
        switch.out(template.union_align_nonexhaustive,
                   size_calc = size_calc,
                   cases = str(cases))
    else:
        switch.out(template.union_align_exhaustive,
                   cases = str(cases))

    # output the alignedSize method
    stream.out(template.union,
               name = name,
               discriminator_size_calc = discriminator_size_calc,
               switch = str(switch))
    

    # --------------------------------------------------------------
    # union::operator{>>, <<}= ({Net, Mem}BufferedStream& _n) [const]
    #
    # FIXME: I thought the CORBA::MARSHAL exception thrown when
    # unmarshalling an array of strings was skipped when unmarshalling
    # from a MemBufferedStream (it is for a struct, but not for a union)
    # (This is probably due to a string-inconsistency with the old compiler
    # and can be sorted out later)

    # marshal/ unmarshal individual cases
    marshal_cases = util.StringStream()
    unmarshal_cases = util.StringStream()
    for c in node.cases():
        caseType = c.caseType()
        decl = c.declarator()
        decl_name = tyutil.name(map(tyutil.mapID, decl.scopedName()))
        isDefault = defaultCase == c
        
        for l in c.labels():
            value = l.value()
            # FIXME: stupid special case. An explicit discriminator
            # value of \0 -> 0000 whereas an implicit one (valueString)
            # \0 -> '\000'
            discrim_value = tyutil.valueString(switchType, value,
                                               environment)
            if tyutil.isChar(switchType) and value == '\0':
                        discrim_value = "0000"

            if l.default():
                unmarshal_cases.out("default:")
            else:
                unmarshal_cases.out("case " + discrim_value + ":")

                marshal_cases.out("case " + discrim_value + ":")
                skutil.marshall_struct_union(marshal_cases, environment,
                                             caseType, decl, "pd_" + decl_name)
                marshal_cases.out("break;")

            unmarshal_cases.out("pd__default = " + str(isDefault) + ";")
            skutil.unmarshall_struct_union(unmarshal_cases, environment,
                                           caseType, decl, "pd_" + decl_name,
                                           can_throw_marshall = 1)
            unmarshal_cases.out("break;")

    if not(hasDefault) and not(exhaustive):
        unmarshal_cases.out("default: pd__default = 1; break;")
        
            
    if booleanWrap:
        marshal_cases.out(template.union_default_bool)
    else:
        marshal_cases.out(template.union_default)


    marshal = util.StringStream()
    if not(exhaustive):
        default = util.StringStream()
        if hasDefault:
            caseType = defaultCase.caseType()
            decl = defaultCase.declarator()
            decl_name =  tyutil.name(map(tyutil.mapID, decl.scopedName()))
            skutil.marshall_struct_union(default, environment, caseType,
                                         decl, "pd_" + decl_name)
        marshal.out(template.union_operators_nonexhaustive,
                    default = str(default),
                    cases = str(marshal_cases))
    else:
        marshal.out(template.union_operators_exhaustive,
                    cases = str(marshal_cases))

    # write the operators
    stream.out(template.union_operators,
               name = name,
               marshal_cases = str(marshal),
               unmarshal_cases = str(unmarshal_cases))
                
        
    return
    
    
def visitForward(node):
    return

def visitConst(node):
    environment = env.lookup(node)
    
    constType = node.constType()
    deref_constType = tyutil.deref(constType)
    
    if tyutil.isString(deref_constType):
        type_string = "char *"
    else:
        type_string = environment.principalID(deref_constType)

    scopedName = node.scopedName()
    scopedName = map(tyutil.mapID, scopedName)
    name = idlutil.ccolonName(scopedName)
    value = tyutil.valueString(deref_constType, node.value(), environment)
    
    init_in_def = tyutil.const_init_in_def(deref_constType)

    
    if init_in_def:
        if self.__insideInterface:
            stream.out("""\
const @type@ @name@ _init_in_cldef_( = @value@ );""",
                       type = type_string, name = name, value = value)
        else:
            stream.out("""\
_init_in_def_( const @type@ @name@ = @value@; )""",
                       type = type_string, name = name, value = value)
        return

    # not init_in_def
    if self.__insideModule and not(self.__insideInterface):
        scopedName = node.scopedName()
        scopedName = map(tyutil.mapID, scopedName)
        scope_str = idlutil.ccolonName(tyutil.scope(scopedName))
        name_str = tyutil.name(scopedName)
        stream.out(template.const_namespace,
                   type = type_string, scope = scope_str, name = name_str,
                   scopedName = name, value = value)
        
    else:
        stream.out("""\
const @type@ @name@ = @value@;""",
                   type = type_string, name = name, value = value)
        

    
    pass
def visitDeclarator(node):
    pass
def visitException(node):
    name = tyutil.name(node.scopedName())
    cxx_name = tyutil.mapID(name)
    
    outer_environment = env.lookup(node)
    environment = outer_environment.enterScope(name)
    
    scoped_name = environment.nameToString(node.scopedName())
    name = tyutil.mapID(tyutil.name(node.scopedName()))
    repoID = tyutil.mapRepoID(node.repoId())

    # build the default ctor, copy ctor, assignment operator
    copy_ctor_body = util.StringStream()
    default_ctor_body = util.StringStream()
    default_ctor_args = []
    assign_op_body = util.StringStream()
    has_default_ctor = 0

    for m in node.members():
        has_default_ctor = 1
        memberType = m.memberType()
        if m.constrType():
            memberType.decl().accept(self)
        deref_memberType = tyutil.deref(memberType)
        memberType_name = environment.principalID(memberType)
        memberType_fqname = self.__globalScope.principalID(memberType)
        type_dims = tyutil.typeDims(memberType)
        for d in m.declarators():
            decl_name = tyutil.mapID(tyutil.name(d.scopedName()))
            
            decl_dims = d.sizes()
            full_dims = decl_dims + type_dims
            is_array = full_dims != []
            is_array_declarator = decl_dims != []

            if is_array_declarator:
                # we use the internal typedef'ed type if the member is an array
                # declarator
                memberType_name_arg = "const " + config.privatePrefix() + "_" + \
                                      decl_name
            else:
                # we normally use the utility function for this purpose
                memberType_name_arg = tyutil.makeConstructorArgumentType(
                    memberType, environment, d)
                
            index = ""

            if is_array:
                index = util.block_begin_loop(copy_ctor_body, full_dims,
                                              iter_type = "unsigned int")
                index = util.block_begin_loop(default_ctor_body, full_dims,
                                              iter_type = "unsigned int")
                index = util.block_begin_loop(assign_op_body, full_dims,
                                              iter_type = "unsigned int")
                
            copy_ctor_body.out("""\
@member_name@@index@ = _s.@member_name@@index@;""", member_name = decl_name,
                               index = index)

            if tyutil.isObjRef(deref_memberType) and not(is_array):
                # these are special resources which need to be explicitly
                # duplicated (but not if an array?)
                default_ctor_body.out("""\
@member_type_name@_Helper::duplicate(_@member_name@@index@);""",
                                      member_type_name = memberType_fqname,
                                      member_name = decl_name,
                                      index = index)
            
            default_ctor_args.append(memberType_name_arg + " _" + decl_name)
            default_ctor_body.out("""\
@member_name@@index@ = _@member_name@@index@;""", member_name = decl_name,
                                  index = index)

            assign_op_body.out("""\
@member_name@@index@ = _s.@member_name@@index@;""", member_name = decl_name,
                               index = index)
            
            if is_array:
                util.block_end_loop(copy_ctor_body, full_dims)
                util.block_end_loop(default_ctor_body, full_dims)
                util.block_end_loop(assign_op_body, full_dims)
        
          
        
    default_ctor = util.StringStream()
    if has_default_ctor:
        default_ctor.out(template.exception_default_ctor,
                         scoped_name = scoped_name,
                         name = name,
                         ctor_args = string.join(default_ctor_args, ", "),
                         default_ctor_body = str(default_ctor_body))

    # write the main chunk
    stream.out(template.exception,
               scoped_name = scoped_name,
               name = name,
               copy_ctor_body = str(copy_ctor_body),
               default_ctor = str(default_ctor),
               ctor_args = string.join(default_ctor_args, ", "),
               default_ctor_body = str(default_ctor_body),
               repoID = repoID,
               assign_op_body = str(assign_op_body))
    

    # deal with alignment, marshalling and demarshalling
    needs_marshalling = node.members() != []
    aligned_size = util.StringStream()
    mem_marshal = util.StringStream()
    net_marshal = util.StringStream()
    mem_unmarshal = util.StringStream()
    net_unmarshal = util.StringStream()
    
    for m in node.members():
        memberType = m.memberType()
        deref_memberType = tyutil.deref(memberType)
        for d in m.declarators():
            decl_name = tyutil.mapID(tyutil.name(d.scopedName()))
            is_array_declarator = d.sizes() != []
            
            if tyutil.isString(memberType) and not(is_array_declarator):
                tmp = skutil.unmarshal_string_via_temporary(decl_name, "_n")
                mem_unmarshal.out(tmp)
                net_unmarshal.out(tmp)
            # TypeCodes seem to be other exceptions
            elif tyutil.isTypeCode(deref_memberType):
                skutil.unmarshall_struct_union(mem_unmarshal, environment,
                                               memberType, d, decl_name, 0,
                                               "_n")
                skutil.unmarshall_struct_union(net_unmarshal, environment,
                                               memberType, d, decl_name, 0,
                                               "_n")
                
            else:
                skutil.unmarshall(mem_unmarshal, environment,
                                  memberType, d, decl_name, 0, "_n",
                                  string_via_member = 1)
                skutil.unmarshall(net_unmarshal, environment,
                                  memberType, d, decl_name, 1, "_n",
                                  string_via_member = 1)

            if tyutil.isTypeCode(deref_memberType):
                skutil.marshall_struct_union(mem_marshal, environment,
                                             memberType, d, decl_name, "_n")
                skutil.marshall_struct_union(net_marshal, environment,
                                             memberType, d, decl_name, "_n")
            else:
                exception = "BAD_PARAM"
                if is_array_declarator:
                    exception = "MARSHAL"
                skutil.marshall(mem_marshal, environment,
                                memberType, d, decl_name, "_n",
                                exception = exception)
                skutil.marshall(net_marshal, environment,
                                memberType, d, decl_name, "_n",
                                exception = exception)

            aligned_size.out(skutil.sizeCalculation(environment, memberType,
                                                    d, "_msgsize", decl_name,
                                                    fixme = 1))

    if needs_marshalling:
        stream.out(template.exception_operators,
                   scoped_name = scoped_name,
                   aligned_size = str(aligned_size),
                   net_marshal = str(net_marshal),
                   mem_marshal = str(mem_marshal),
                   net_unmarshal = str(net_unmarshal),
                   mem_unmarshal = str(mem_unmarshal))


    return
            
            
                                           
