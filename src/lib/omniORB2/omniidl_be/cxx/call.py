# -*- python -*-
#                           Package   : omniidl
# call.py                   Created on: 2000/08/03
#			    Author    : David Scott (djs)
#
#    Copyright (C) 2000 AT&T Laboratories Cambridge
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
#   Produce local callback functions
# 

# $Id$
# $Log$
# Revision 1.1.2.2  2000/09/14 16:03:01  djs
# Remodularised C++ descriptor name generator
# Bug in listing all inherited interfaces if one is a forward
# repoID munging function now handles #pragma ID in bootstrap.idl
# Naming environments generating code now copes with new IDL AST types
# Modified type utility functions
# Minor tidying
#
# Revision 1.1.2.1  2000/08/21 11:34:32  djs
# Lots of omniidl/C++ backend changes
#

"""Produce local callback functions"""

from omniidl import idlast, idltype
from omniidl_be.cxx import types, id, util, skutil, output, cxx, ast, descriptor
from omniidl_be.cxx.skel import mangler, template

import string

import call
self = call

# Callable- represents the notion of a callable entity (eg operation or
# attribute accessor method). Note that a read/write attribute is really
# two such entities paired together.
#
#  .interface():      get the associated Interface object
#  .operation_name(): get the IIOP operation name (eg get_colour)
#  .method_name():    get the C++ mapped method name (eg colour)
#  .returnType():     idltype
#  .parameters():     idlast.Parameter
#  .oneway():         boolean
#  .contexts():
#  .signature():      string representing the IDL signature
#  .method(use_out):  C++ mapping
#
class Callable:
    def __init__(self, interface, operation_name, method_name, returnType,
                 parameters, oneway = 0, raises = [], contexts = []):
        self.__interface = interface
        self.__operation_name = operation_name
        self.__method_name = method_name
        self.__returnType = returnType
        self.__parameters = parameters
        self.__oneway = oneway
        self.__raises = raises
        self.__contexts = contexts
        self.__signature = mangler.produce_signature(returnType, parameters,
                                                     raises)

    def interface(self): return self.__interface
    def operation_name(self): return self.__operation_name
    def method_name(self): return self.__method_name
    def returnType(self): return self.__returnType
    def parameters(self): return self.__parameters
    def oneway(self): return self.__oneway
    def raises(self): return self.__raises
    def contexts(self): return self.__contexts
    def signature(self): return self.__signature

# Utility functions to build Callables #################################
#
def operation(interface, operation):
    assert isinstance(operation, idlast.Operation)
    return Callable(interface,
                    operation.identifier(),
                    id.mapID(operation.identifier()),
                    operation.returnType(),
                    operation.parameters(),
                    operation.oneway(),
                    operation.raises(),
                    operation.contexts())

def read_attributes(interface, attribute):
    assert isinstance(attribute, idlast.Attribute)
    callables = []
    for identifier in attribute.identifiers():
        callables.append(Callable(interface,
                                  "_get_" + identifier,
                                  id.mapID(identifier),
                                  attribute.attrType(),
                                  [], 0, [], []))
    return callables

def write_attributes(interface, attribute):
    assert isinstance(attribute, idlast.Attribute)
    voidType = idltype.Base(idltype.tk_void)
    callables = []
    param = idlast.Parameter(attribute.file(), attribute.line(),
                             attribute.mainFile(), [], [],
                             0, attribute.attrType(), "_v")
    for identifier in attribute.identifiers():
        callables.append(Callable(interface,
                                  "_set_" + identifier,
                                  id.mapID(identifier),
                                  voidType, [param], 0, [], []))
    return callables
                                  

# FIXME: these should be centralised ################################
# Create appropriate argument type instances
#  -out- (includes return) types have their storage
#        created here (eg by _var mapping)
#  -in-, -inout- types have had their storage
#        allocated by the caller
def argument_instance(type, environment):
    assert isinstance(type, types.Type)
    
    d_type = type.deref()
    dims = type.dims()
    
    is_variable = d_type.variable()
    is_array = dims != []

    # in, out, inout (corresponds to idltype.Parameter.direction()
    mapping = ["", "", ""]

    # all strings are CORBA::String_var
    if d_type.string() and not(is_array):
        mapping = ["CORBA::String_var", "CORBA::String_var",
                   "CORBA::String_var"]
        return mapping

    # all object references and typecodes are _var types
    if d_type.objref() and not(is_array):
        name = d_type.base(environment)
        mapping = [name + "_var", name + "_var", name + "_var"]

        return mapping

                
    if d_type.typecode() and not(is_array):
        name = "CORBA::TypeCode"
        mapping = [name + "_var", name + "_var", name + "_var"]
        return mapping

    # typedefs aren't dereferenced
    if type.typedef():
        name = type.base(environment)
        mapping[0] = name
        # out types have storage allocated here
        if is_variable:
            mapping[1] = name + "_var"
        else:
            mapping[1] = name
        mapping[2] = name
        
        return mapping

    name = type.base(environment)
    mapping = [name, name, name]
    if is_variable:
        mapping[1] = name + "_var"

    return mapping


# Given an argument type and a name, returns the thing actually
# passed to the method call.
def method_argument(type, name, environment):
    assert isinstance(type, types.Type)
    d_type = type.deref()
    type_dims = type.dims()
    is_array = type_dims != []
    is_variable = d_type.variable()
    
    # in, out, inout (corresponds to idltype.Parameter.direction()
    mapping = ["", "", ""]

    # arrays
    if is_array and is_variable:
        mapping = [name, name + ".out()", name]
        return mapping

    # strings and object references and typecodes
    if d_type.string() or d_type.objref() or d_type.typecode():
        mapping = [name + ".in()", name + ".out()", name + ".inout()"]
        return mapping

    is_variable = d_type.variable()
    mapping = [name, name, name]
    if is_variable:
        mapping[1] = name + ".out()"

    return mapping
        

# Given an argument type and a name, indicates whether the thing
# is passed by a pointer (involves .operator->() etc)
def is_pointer(type):
    assert isinstance(type, types.Type)
    d_type = type.deref()

    # strings are a special case
    if d_type.string():
        return 0

    is_variable = d_type.variable()
    if is_variable:
        return 1

    return 0

def argmapping(type):
    assert isinstance(type, types.Type)

    d_type = type.deref()
    type_dims = type.dims()
    is_variable = type.variable()
    is_array = type_dims != []

    type_name = type.base()
    d_type_name = d_type.base()

    if is_array:
        if is_variable:
            return ["const " + type_name + "_slice*",
                               type_name + "_slice*&",
                               type_name + "_slice*",
                               type_name + "_slice*"]
        else:
            return ["const " + type_name + "_slice*",
                               type_name + "_slice*",
                               type_name + "_slice*",
                               type_name + "_slice*"]

    if d_type.objref():
        return [d_type_name + "_ptr",
                d_type_name + "_ptr&",
                d_type_name + "_ptr&",
                d_type_name + "_ptr"]
    
    if d_type.typecode():
        return ["CORBA::TypeCode_ptr",
                "CORBA::TypeCode_ptr&",
                "CORBA::TypeCode_ptr&",
                "CORBA::TypeCode_ptr"]        

    if d_type.string():
        return ["const char*",
                "char *&",
                "char *&",
                "char *"]

    if is_variable:
        return ["const " + type_name + "&",
                           type_name + "*&",
                           type_name + "&",
                           type_name + "*"]
    
    # basic types are all representable by an integer so
    # passing by value is as cheap as by reference
    if isinstance(d_type.type(), idltype.Base) or \
       d_type.enum():
        return [type_name,
                type_name + "&",
                type_name + "&",
                type_name]

    # else it is cheaper to pass the in arguments by
    # constant reference
    return ["const " + type_name + "&",
                       type_name + "&",
                       type_name + "&",
                       type_name]

   
#####################################################################


def impl_dispatch_method(stream, callable, environment):
    assert isinstance(stream, output.Stream)
    assert isinstance(callable, Callable)
    assert isinstance(environment, id.Environment)

    idl_operation_name = callable.operation_name()
    operation_name = callable.method_name()

    return_type = types.Type(callable.returnType())
    d_return_type = return_type.deref()
    return_type_dims = return_type.dims()
    has_return_value = not(return_type.void())

    # all arguments are prefixed, return value is called "return"
    prefix = "arg_"
    
    # create the operation arguments and demarshal them
    # if values are supplied by the caller (ie in or inout)
    get_arguments = output.StringStream()

    # names of arguments to pass to the method
    argument_list = []

    # declare the result type if necessary and call the method
    decl_result = output.StringStream()

    # compute the message size for alignment handling
    size_calc_arguments = output.StringStream()
    size_calc_results = output.StringStream()

    # marshal the result (if any)
    put_results = output.StringStream()

    # marshal the out arguments (if any)
    put_arguments = output.StringStream()

    for argument in callable.parameters():
        argument_name = id.mapID(argument.identifier())
        argument_prefixed_name = prefix + argument_name
        direction = argument.direction()
        argument_type = types.Type(argument.paramType())
        argument_type_name = argument_type.base(environment)
        argument_is_variable = argument_type.variable()
        argument_dims = argument_type.dims()
        is_array = argument_dims != []
        d_argument_type = argument_type.deref()
        d_dims_type = argument_type.deref(keep_dims = 1)
        d_dims_name = d_dims_type.base(environment)
                                              
        argument_type_names = argument_instance(argument_type, environment)
        # declare the argument
        get_arguments.out("""\
@argument_type_name@ @argument_name@;
""",
                          argument_type_name = argument_type_names[direction],
                          argument_name = argument_prefixed_name)
        # consider the need to demarshal it
        if argument.is_in():
            skutil.unmarshall(get_arguments, environment, argument_type, None,
                              prefix + argument_name,
                              from_where = "giop_s",
                              can_throw_marshall = 1,
                              string_via_member = 1)

        marshal_name = argument_prefixed_name
        align_name = argument_prefixed_name
        if is_array:
            argument_slice_name = "((" + d_dims_name + "_slice*)" +\
                                  argument_prefixed_name + ")"
        else:
            argument_slice_name = "((" + argument_type_name + "_slice*)" +\
                                  argument_prefixed_name + ")"
        argument_operator_name = "(" + argument_prefixed_name +\
                                 ".operator->())"

        is_pntr = is_pointer(argument_type)
        arg_is_pntr = 0
        if is_array and direction == 1: # only out
            # fixed structures don't do slices?
            if (d_argument_type.struct() or d_argument_type.union()) and \
               not(argument_is_variable):
                pass
            # TypeCodes and Anys are variable base types
            elif not(isinstance(d_argument_type.type(), idltype.Base) and \
                     not(d_argument_type.any())) and \
                 not(d_argument_type.enum()):
                marshal_name = argument_slice_name
                align_name = argument_slice_name

        # and consider the need to marshal it again
        if argument.is_out():
            if is_pntr and direction == 1 and not(is_array):
                marshal_name = argument_operator_name
                align_name = argument_operator_name
                arg_is_pntr = 1
                if not(d_argument_type.objref()) and \
                   not(d_argument_type.typecode()):
                    marshal_name = "*" + marshal_name
            if is_pntr and direction == 2 and d_argument_type.typecode():
                arg_is_pntr = 1

            skutil.marshall(put_arguments, environment, argument_type,
                            None, marshal_name, "giop_s")
            size_calc_arguments.out(
                skutil.sizeCalculation(environment, argument_type,
                                       None, "msgsize",
                                       align_name,
                                       is_pointer = arg_is_pntr,
                                       fixme = 1))
        # build the argument list
        argument_list.append(method_argument(argument_type,
                                             argument_prefixed_name,
                                             environment)[direction])
        
    # by default there is no result to assign method return value to    
    result_assignment = ""
    
    if has_return_value:
        return_is_array = return_type_dims != []
        return_is_variable = return_type.variable()
        result_mapping = argument_instance(return_type, environment)[1]
        return_is_pointer = is_pointer(return_type) and not(return_is_array)
        dims_return_type = return_type.deref(keep_dims = 1)
        return_type_name = return_type.base(environment)

        # something very strange happening with array typedefs
        if return_is_array:
            return_type = dims_return_type
            return_type_name = dims_return_type.base(environment)
        
        # exception- arrays of fixed types use the _var mapping
        if not(return_is_variable) and return_is_array:
            result_mapping = result_mapping + "_var"
            
        decl_result.out("""\
@result_type@ result;
""", result_type = result_mapping)

        marshal_name = "result"
        align_name = "result"
        if return_is_array:
            # TypeCodes and Anys are variable base types (see above)
            if not(isinstance(d_return_type.type(), idltype.Base) and \
                   not(d_return_type.any())) and not(d_return_type.enum()):
                marshal_name = "((" + return_type_name + "_slice*)result)"
            align_name = marshal_name
        elif return_is_pointer:
            align_name = "(result.operator->())"
            marshal_name = align_name
            if not(d_return_type.objref()) and not(d_return_type.typecode()):
                marshal_name = "*" + align_name
        # needs to be counted in the message size calculation
        size_calc_results.out(
            skutil.sizeCalculation(environment, return_type,
                                   None, "msgsize", align_name,
                                   is_pointer = return_is_pointer,
                                   fixme = 1))
        skutil.marshall(put_results, environment, return_type, None,
                        marshal_name, "giop_s")
        result_assignment = "result = "


    raises = callable.raises()
    raises_sorted = skutil.sort_exceptions(raises)
    has_user_exceptions = raises != []
    # need to declare user exceptions
    exceptions = output.StringStream()
    try_ = output.StringStream()
    catch = output.StringStream()
    if has_user_exceptions:
        # old compiler seems to order repoIDs by exception definition
        # no need to duplicate that behaviour here
        repoIDs = map(lambda x: "\"" + x.repoId() + "\"", raises_sorted)
        exceptions.out(template.interface_operation_exn,
                       repoID_list = string.join(repoIDs, ",\n"),
                       n = str(len(repoIDs)))
        
        try_.out(template.interface_operation_try)
        catch.out(template.interface_operation_catch_start)
        for exception in raises:
            ex_scopedName = id.Name(exception.scopedName())
            catch.out(template.interface_operation_catch_exn,
                      exname = ex_scopedName.fullyQualify())
            
        catch.out(template.interface_operation_catch_end)

    # handle "context"s
    get_context = output.StringStream()
    if callable.contexts() != []:
        
        get_context.out(template.interface_operation_context)
        argument_list.append("_ctxt")

    # main block of code goes here
    stream.out(template.interface_operation_dispatch,
               operation_name = operation_name,
               idl_operation_name = idl_operation_name,
               exception_decls = exceptions,
               get_arguments = get_arguments,
               get_context = get_context,
               decl_result = decl_result,
               try_ = str(try_),
               argument_list = string.join(argument_list, ", "),
               catch = str(catch),
               result_assignment = result_assignment,
               size_calculation_results = size_calc_results,
               size_calculation_arguments = size_calc_arguments,
               put_results = put_results,
               put_arguments = put_arguments)

# Predefined proxy call descriptors
prefix = "omniStdCallDesc::"
system_descriptors = \
      { "void":                      prefix + "void_call",
        "_cCORBA_mObject_i_cstring": prefix + "_cCORBA_mObject_i_cstring" }
self.proxy_call_descriptors = system_descriptors.copy()
        
def proxy_call_descriptor(callable, stream):
    return_type = types.Type(callable.returnType())
    d_return_type = return_type.deref()
    return_type_name = return_type.base()
    return_dims = return_type.dims()
    return_is_array = return_dims != []
    has_return_value = not(d_return_type.void())
    has_arguments = callable.parameters() != []
    has_exceptions = callable.raises() != []

    identifier = callable.method_name()

    # signature is a text string form of the complete operation signature
    signature = callable.signature()
    # we only need one descriptor for each _signature_ (not operation)
    if self.proxy_call_descriptors.has_key(signature):
        return self.proxy_call_descriptors[signature]

    desc = descriptor.call_descriptor(signature)
    self.proxy_call_descriptors[signature] = desc

    # if no arguments and no return value and no exceptions then
    # use the void_call predefined one in the runtime
    if not(has_return_value) and not(has_arguments) and \
       not(has_exceptions):
        assert(0)
        return
    

    # build up the constructor argument list, the initialisation
    # list and the data members list
    ctor_args = ["LocalCallFn lcfn", "const char* op", "size_t oplen",
                 "_CORBA_Boolean oneway"]
    init_list = []
    member_list = []
    member_list_string = ""

    need_to_marshal = 0
    need_to_unmarshal = 0

    n = -1
    for parameter in callable.parameters():
        n = n + 1
        param_type = types.Type(parameter.paramType())
        direction = parameter.direction()

        type_mapping = argmapping(param_type)[direction]
        
        ctor_args.append(type_mapping + " a_" + str(n))
        init_list.append("arg_" + str(n) + "(a_" + str(n) +")")
        member = type_mapping + " arg_" + str(n)
        member_list.append(member + ";")

        if parameter.is_in():  need_to_marshal = 1
        if parameter.is_out(): need_to_unmarshal = 1


    ctor_args_string = string.join(ctor_args, ", ")
    inherits_list_string = string.join(
        ["omniCallDescriptor(lcfn, op, oplen, oneway)"] + init_list, ",\n")
    member_list_string = string.join(member_list, "\n")

    # consider the return value if present
    result_member_function = ""
    result_member_data = ""
    if has_return_value:
        return_type_name = argmapping(return_type)[3]
        result_member_function = "inline " + return_type_name +\
                                 " result() { return pd_result; }"
                                                       
        result_member_data = return_type_name + " pd_result;"
        need_to_unmarshal = 1


    # declare the marshalling and unmarshalling functions
    marshal_arguments_decl = ""
    unmarshal_arguments_decl = ""
    if need_to_marshal:
        marshal_arguments_decl = """\
virtual CORBA::ULong alignedSize(CORBA::ULong size_in);
virtual void marshalArguments(GIOP_C&);
"""
    if need_to_unmarshal:
        unmarshal_arguments_decl = """\
virtual void unmarshalReturnedValues(GIOP_C&);
"""

    # are there any user exceptions
    user_exceptions_decl = ""
    raises = callable.raises()
    has_user_exceptions = raises != []
    if has_user_exceptions:
        user_exceptions_decl = \
"virtual void userException(GIOP_C&, const char*);\n"
    
    # Write the proxy class definition
    stream.out(template.interface_proxy_class,
               signature = signature,
               call_descriptor = desc,
               ctor_args = ctor_args_string,
               inherits_list = inherits_list_string,
               marshal_arguments_decl = marshal_arguments_decl,
               user_exceptions_decl = user_exceptions_decl,
               unmarshal_arguments_decl = unmarshal_arguments_decl,
               result_member_function = result_member_function,
               member_data = member_list_string,
               result_member_data = result_member_data)

    # -------------------------------------------------------------
    
    # consider marshalling data
    if need_to_marshal:
        size_calculation = output.StringStream()
        marshal_block = output.StringStream()
        n = -1
        for parameter in callable.parameters():
            n = n + 1
            if parameter.is_in():
                param_type = types.Type(parameter.paramType())
                d_param_type = param_type.deref()
                is_pointer = 0
                if d_param_type.typecode():
                    is_pointer = 1
                name = "arg_" + str(n)
                size_calculation.out(skutil.sizeCalculation(
                    None, param_type, None, "msgsize", name,
                    fixme = 1, is_pointer = is_pointer))
                skutil.marshall(marshal_block, None, param_type,
                                None, name, "giop_client")
                
        # write the alignment function
        stream.out(template.interface_proxy_alignment,
                   call_descriptor = desc,
                   size_calculation = size_calculation)
        # write the marshal function
        stream.out(template.interface_proxy_marshal,
                   call_descriptor = desc,
                   marshal_block = marshal_block)

    # -------------------------------------------------------------
    
    # consider unmarshalling data
    if need_to_unmarshal:
        # split into 3 sections:
        #  pre_decls for predeclarations of temporaries etc
        #  unmarshal_block for the actual unmarshalling code
        #  post_assign for assignment of data from the
        #    temporaries
        # This could be rationalised, but it's the way the old BE
        # seems to do things.
        pre_decls = output.StringStream()
        unmarshal_block = output.StringStream()
        post_assign = output.StringStream()

        def unmar_via_temp(type, name, temp, temp_type_name, init_value,
                           direction, dereference = 0,
                           pre_decls = pre_decls,
                           unmarshal_block = unmarshal_block,
                           post_assign = post_assign,
                           environment = None):
            dims = type.dims()
            is_array = dims != []
            d_type = type.deref()
            is_variable = d_type.variable()
            is_out = direction == 1
            is_inout = direction == 2
            assert is_out or is_inout

            d_dims_type = type.deref(keep_dims = 1)

            if d_type.objref() and not(is_array):
                helper_name = temp_type_name + "_Helper"
                temp_type_name = temp_type_name + "_ptr"
            elif d_type.typecode() and not(is_array):
                if direction == 2:
                    temp_type_name = temp_type_name + "_var"
                else:
                    temp_type_name = temp_type_name + "_ptr"

            if is_out:
                pre_decls.out("""\
@temp_type_name@ @temp@ = @init_value@;
""", temp_type_name = temp_type_name,
                              temp = temp, init_value = init_value)
            elif is_inout:
                unmarshal_block.out("""\
@temp_type_name@ @temp@;
""", temp_type_name = temp_type_name,
                                    temp = temp)

            # This bit is almost certainly wrong. It seems that when an
            # argument is a typedef to an array type, the non-derefed
            # type is used in the typedec but a dereferenced slice is used
            # elsewhere. Why?
            if is_array:
                temp_type_name = d_dims_type.base() + "_slice*"
                
            if dereference:
                deref_name = "*" + temp
            # 'Normal' base types don't have slices. TypeCodes and Anys are
            # variable base types though.
            elif is_array and not(isinstance(d_type.type(), idltype.Base) and \
                                  not(d_type.typecode())   and \
                                  not(d_type.any())):
                deref_name = "((" + temp_type_name + ") " + temp + ")"
            else:
                deref_name = temp

                
            skutil.unmarshall(unmarshal_block, None, type, None,
                              deref_name, can_throw_marshall = 1,
                              from_where = "giop_client",
                              string_via_member = 1)

            if is_inout and not(is_array):
                if d_type.string():
                    unmarshal_block.out("""\
CORBA::string_free(@name@);
@name@ = @temp@;
""", name = name, temp = temp)
                elif d_type.objref():
                    unmarshal_block.out("""\
@helper_name@::release(@name@);
@name@ = @temp@;
""", helper_name = helper_name,
                                        name = name, temp = temp)
                elif d_type.typecode():
                    unmarshal_block.out("""\
CORBA::release(@name@);
@name@ = @temp@._retn();
""", name = name, temp = temp)
                

            if is_out:
                post_assign.out("""\
@name@ = @temp@;
""", name = name, temp = temp)

        # handle the return value, if one exists. This could be
        # simplified- surely having a return type is semantically
        # identical to having a similarly named out argument?
        if has_return_value:
            name = "pd_result"
            d_dims_return_type = return_type.deref(keep_dims = 1)
            # Something strange is happening here.
            # Need to investigate the array mapping further at a later stage
            # hopefully to remove this oddity
            if return_is_array:
                return_type_base = d_dims_return_type.base()
            else:
                return_type_base = return_type.base()
            return_is_variable = return_type.variable()
            # we need to allocate storage if the return is an
            # array
            if return_is_array:
                if return_is_variable                or \
                   d_return_type.struct() or d_return_type.union():
                    name = "((" + return_type_base + "_slice*) pd_result)"
                unmarshal_block.out("""\
pd_result = @type@_alloc();
""", type = return_type_base)
            elif return_is_variable and d_return_type.type().kind() in \
                 [idltype.tk_struct, idltype.tk_union,
                  idltype.tk_sequence, idltype.tk_any ]:
                unmarshal_block.out("""\
pd_result = new @type@;
""", type = return_type_base)
                name = "*" + name

            skutil.unmarshall(unmarshal_block, None, return_type, None,
                              name, can_throw_marshall = 1,
                              from_where = "giop_client",
                              string_via_member = 1)
        
        n = -1
        for parameter in callable.parameters():
            n = n + 1
            direction = parameter.direction()
            # don't need to worry about in arguments!
            if direction == 0:
                continue
            
            arg_name = "arg_" + str(n)
            temp_name = "tmp_" + str(n)
            param_type = types.Type(parameter.paramType())
            d_param_type = param_type.deref()
            d_dims_param_type = param_type.deref(keep_dims = 1)
            param_type_name = param_type.base()
            d_dims_name = d_dims_param_type.base()
            dims = param_type.dims()
            is_array = dims != []
            is_out = direction == 1
            in_inout = direction == 2
            is_variable = d_param_type.variable()

            # for variable structs and unions, we need to *name
            dereference = 0
            # do we unmarshal via a temporary?
            # we need to setup the temporary variables
            temp_init_value = None
            if (isinstance(d_param_type.type(), idltype.Base) and \
                not d_param_type.any() and not d_param_type.typecode()) or \
                d_param_type.enum():
                via_tmp = 0
            elif is_array:
                if is_variable and is_out:
                    via_tmp = 1
                    # the temporary variable is an array slice
                    temp_type_name = param_type_name + "_slice*"
                    temp_init_value = d_dims_name + "_alloc()"
                else:
                    via_tmp = 0
            elif is_variable:
                if d_param_type.string():
                    via_tmp = 1
                    temp_type_name = "char*"
                    temp_init_value = "0"
                elif d_param_type.objref():
                    via_tmp = 1
                    temp_type_name = d_dims_name
                    temp_init_value = "0"
                elif d_param_type.typecode():
                    via_tmp = 1
                    temp_type_name = "CORBA::TypeCode"
                    temp_init_value = "CORBA::TypeCode::_nil()"
                elif d_param_type.type().kind() in \
                     [idltype.tk_struct, idltype.tk_union,
                      idltype.tk_sequence, idltype.tk_any]:
                    if is_out:
                        via_tmp = 1
                        temp_type_name = param_type_name + "*"
                        dereference = 1
                        temp_init_value = " new " + param_type_name
                    else:
                        via_tmp = 0
            else:
                via_tmp = 0
                    
            # either unmarshal via the temporary or not...
            if via_tmp:
                unmar_via_temp(param_type, arg_name, temp_name,
                               temp_type_name, temp_init_value,
                               direction, dereference)
            else:
                skutil.unmarshall(unmarshal_block, None, param_type, None,
                                  arg_name, can_throw_marshall = 1,
                                  from_where = "giop_client",
                                  string_via_member = 1)

        # write the unmarshal function
        stream.out(template.interface_proxy_unmarshal,
                   call_descriptor = desc,
                   pre_decls = pre_decls,
                   unmarshal_block = unmarshal_block,
                   post_assign = post_assign)


    # -------------------------------------------------------------
    
    # consider user exceptions
    if has_user_exceptions:
        block = output.StringStream()
        first_one = 1

        for exception in raises:
            scopedName = exception.scopedName()
            repoID = scopedName + ["_PD_repoId"]
            repoID_str = id.Name(repoID).fullyQualify()
            exname = id.Name(scopedName).fullyQualify()
            if first_one:
                switch = "if"
                first_one = 0
            else:
                switch = "else if"
            block.out("""\
@switch@( strcmp(repoId, @repoID_str@) == 0 ) {
  @exname@ _ex;
  _ex <<= giop_client;
  giop_client.RequestCompleted();
  throw _ex;
}
""", switch = switch, repoID_str = repoID_str, exname = exname)

        # write the user exception template
        stream.out(template.interface_proxy_exn,
                   call_descriptor = desc,
                   exception_block = str(block))

    return desc

# unique to (callable, node)

self.local_callbacks = {}

def local_callback_function(stream, name, callable):
    assert isinstance(stream, output.Stream)
    assert isinstance(name, id.Name)
    assert isinstance(callable, Callable)

    key = name.hash() + "/" + callable.signature()
    if self.local_callbacks.has_key(key):
        return local_callbacks[key]

    impl_name = name.prefix("_impl_")

    signature = callable.signature()
    call_descriptor = self.proxy_call_descriptors[signature]
    local_call_descriptor = descriptor.local_callback_fn(name,
             callable.operation_name(), signature)
    
    parameters = callable.parameters()
    
    impl_args = map(lambda x: "tcd->arg_" + str(x),
                    range(0, len(callable.parameters())))

    if callable.contexts() != []:
        impl_args.append("cd->context_info()->context")

    returnType = types.Type(callable.returnType())
    result_type = returnType.op(types.RET)
    result_string = ""
    if not(returnType.void()):
        result_string = "tcd->pd_result = "

    # If we have no return value and no arguments at all then we don't
    # need to fetch the call descriptor. This suppresses a warning in gcc
    # about an unused variable.
    get_cd = ""
    if not(returnType.void()) or (impl_args != []):
        get_cd = call_descriptor + "* tcd = (" + call_descriptor + "*) cd;"

    stream.out(template.interface_callback,
               local_call_descriptor = local_call_descriptor,
               get_call_descriptor = get_cd,
               impl_fqname = impl_name.fullyQualify(),
               name = name.fullyQualify(),
               cxx_operation_name = callable.method_name(),
               operation_arguments = string.join(impl_args, ", "),
               result = result_string)

    self.local_callbacks[key] = local_call_descriptor
    return local_call_descriptor

