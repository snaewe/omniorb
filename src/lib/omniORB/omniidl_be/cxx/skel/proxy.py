# -*- python -*-
#                           Package   : omniidl
# proxy.py                  Created on: 1999/11/24
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
#   Produce operation and attribute proxy call descriptors

# $Id$
# $Log$
# Revision 1.1  1999/11/24 18:00:48  djs
# Started new module to handle proxy call descriptors
#

"""Produce operation and attribute proxy call descriptors"""

import string

from omniidl import idlutil, idltype, idlast

from omniidl.be.cxx import util, tyutil, skutil



import proxy
self = proxy

def __init__(environment, stream):
    self.__environment = environment
    self.__stream = stream
    return self


proxy_class_template = """\
// Proxy call descriptor class. Mangled signature:
//  @signature@
class @call_descriptor@
  : public omniCallDescriptor
{
public:
  inline @call_descriptor@(@ctor_args@):
     @inherits_list@ {}

  @marshal_arguments_decl@
  @unmarshal_arguments_decl@
  
  @result_member_function@
  @member_data@
  @result_member_data@
};
"""

unmarshal_template = """\
void @call_descriptor@::unmarshalReturnedValues(GIOP_C& giop_client)
{
  @unmarshal_block@
}
"""

alignment_template = """\
CORBA::ULong @call_descriptor@::alignedSize(CORBA::ULong msgsize)
{
  @size_calculation@
  return msgsize;
}
"""

marshal_template = """\
void @call_descriptor@::marshalArguments(GIOP_C& giop_client)
{
  @marshal_block@
}
"""

def argmapping(type):
    assert isinstance(type, idltype.Type)

    deref_type = tyutil.deref(type)
    type_dims = tyutil.typeDims(type)
    is_array = type_dims != []
    is_variable = tyutil.isVariableType(deref_type)

    type_name = environment.principalID(type, fully_scope = 1)

    if is_array:
        if is_variable:
            return ["const " + type_name + "_slice*",
                               type_name + "_slice*",
                               type_name + "_slice*",
                               type_name + "_slice*"]
        else:
            return ["const " + type_name + "_slice*",
                               type_name + "_slice*&",
                               type_name + "_slice*",
                               type_name + "_slice*"]

    if tyutil.isObjRef(deref_type):
        return [type_name + "_ptr",
                type_name + "_ptr&",
                type_name + "_ptr&",
                type_name + "_ptr"]

    if tyutil.isString(deref_type):
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
    if isinstance(deref_type, idltype.Base):
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


def operation(operation, seed):
    return_type = operation.returnType()
    deref_return_type = tyutil.deref(return_type)
    has_return_value = not(tyutil.isVoid(deref_return_type))
    identifier = tyutil.mapID(operation.identifier())

    signature = mangler.produce_operation_signature(operation)

    # if a descriptor already exists for this signature, we don't
    # need to generate a whole new proxy
    try:
        descriptor = mangler.operation_descriptor_name(operation)
        need_proxy = 0
    except KeyError:
        mangler.generate_descriptors(operation, seed)
        descriptor = mangler.operation_descriptor_name(operation)
        need_proxy = 1

    if not(need_proxy):
        return

    # build up the constructor argument list, the initialisation
    # list and the data members list
    ctor_args = ["LocalCallFn lcfn", "const char* op", "size_t oplen",
                 "_CORBA_Boolean oneway"]
    init_list = []
    member_list = []

    need_to_marshal = 0
    need_to_unmarshal = 0

    n = -1
    for parameter in operation.parameters():
        n = n + 1
        param_type = parameter.paramType()
        direction = parameter.direction()

        type_mapping = argmapping(param_type)[direction]
        
        ctor_args.append(type_mapping + " a_" + str(n))
        init_list.append(type_mapping + " arg_" + str(n) + "(a_" + str(n) +")")
        member_list.append(type_mapping + " arg_" + str(n))

        if parameter.is_in():  need_to_marshal = 1
        if parameter.is_out(): need_to_unmarshal = 1


    ctor_args_string = string.join(ctor_args, ", ")
    inherits_list_string = string.join(
        ["omniCallDescriptor(lcfn, op, oplen, oneway)"] + init_list, ", ")
    member_list_string = string.join(member_list, ", ")

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
        virtual void marshalArguments(GIOP_C&);"""
    if need_to_unmarshal:
        unmarshal_arguments_decl = """\
        virtual void unmarshalReturnedValues(GIOP_C&);"""
    
    # Write the proxy class definition
    stream.out(proxy_class_template,
               signature = signature,
               call_descriptor = descriptor,
               ctor_args = ctor_args_string,
               inherits_list = inherits_list_string,
               marshal_arguments_decl = marshal_arguments_decl,
               unmarshal_arguments_decl = unmarshal_arguments_decl,
               result_member_function = result_member_function,
               member_data = member_list_string,
               result_member_data = result_member_data)

    # -------------------------------------------------------------
    
    # consider marshalling data
    if need_to_marshal:
        size_calculation = util.StringStream()
        marshal_block = util.StringStream()
        n = -1
        for parameter in operation.parameters():
            n = n + 1
            if parameter.is_in():
                param_type = parameter.paramType()
                name = "arg_" + str(n)
                size_calculation.out(skutil.sizeCalculation(
                    environment, param_type, None, "msgsize", name,
                    fixme = 1, fully_scope = 1))
                skutil.marshall(marshal_block, environment, param_type,
                                None, name, "giop_client",
                                fully_scope = 1)
                
        # write the alignment function
        stream.out(alignment_template,
                   size_calculation = str(size_calculation))
        # write the marshal function
        stream.out(marshal_template,
                   marshal_block = str(marshal_block))

    # -------------------------------------------------------------
    
    # consider unmarshalling data
    if need_to_unmarshal:
        pass
            
