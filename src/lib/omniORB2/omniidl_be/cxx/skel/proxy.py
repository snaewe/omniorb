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
# Revision 1.10  1999/12/16 16:12:33  djs
# Fix for functions with no arguments or return values but that can
# raise exceptions (they should have call descriptors)
#
# Revision 1.9  1999/12/15 12:14:00  djs
# Nolonger produces call descriptor if operation has no arguments and no
# return type
#
# Revision 1.8  1999/12/14 11:53:23  djs
# Support for CORBA::TypeCode and CORBA::Any
# Exception member bugfix
#
# Revision 1.7  1999/12/13 10:50:07  djs
# Treats the two call descriptors associated with an attribute separately,
# since it can happen that one needs to be generated but not the other.
#
# Revision 1.6  1999/12/09 20:40:58  djs
# Bugfixes and integration with dynskel/ code
#
# Revision 1.5  1999/12/01 16:59:27  djs
# Generates code to handle user exceptions being thrown.
#
# Revision 1.4  1999/11/29 19:27:05  djs
# Code tidied and moved around. Some redundant code eliminated.
#
# Revision 1.3  1999/11/29 15:27:28  djs
# Moved proxy call descriptor generation for attributes to this module.
#
# Revision 1.2  1999/11/26 18:50:48  djs
# Module for creating call descriptor proxies
#
# Revision 1.1  1999/11/24 18:00:48  djs
# Started new module to handle proxy call descriptors
#

"""Produce operation and attribute proxy call descriptors"""

import string

from omniidl import idlutil, idltype, idlast

from omniidl.be.cxx import util, tyutil, skutil

from omniidl.be.cxx.skel import mangler



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
  @user_exceptions_decl@
  @result_member_function@
  @member_data@
  @result_member_data@
};
"""

unmarshal_template = """\
void @call_descriptor@::unmarshalReturnedValues(GIOP_C& giop_client)
{
  @pre_decls@
  @unmarshal_block@
  @post_assign@
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

exception_template = """\
void @call_descriptor@::userException(GIOP_C& giop_client, const char* repoId)
{
  @exception_block@
  else {
    giop_client.RequestCompleted(1);
    throw CORBA::MARSHAL(0, CORBA::COMPLETED_MAYBE);
  }
}
"""


def argmapping(type):
    assert isinstance(type, idltype.Type)
    environment = self.__environment

    deref_type = tyutil.deref(type)
    type_dims = tyutil.typeDims(type)
    is_array = type_dims != []
    is_variable = tyutil.isVariableType(deref_type)

    type_name = environment.principalID(type, fully_scope = 1)
    deref_type_name = environment.principalID(deref_type, fully_scope = 1)

    if is_array:
        #if tyutil.isObjRef(deref_type) or \
        #   tyutil.isString(deref_type):
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

    if tyutil.isObjRef(deref_type) or \
       tyutil.isTypeCode(deref_type):
        return [deref_type_name + "_ptr",
                deref_type_name + "_ptr&",
                deref_type_name + "_ptr&",
                deref_type_name + "_ptr"]

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
    if isinstance(deref_type, idltype.Base) or \
       tyutil.isEnum(deref_type):
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
    environment = self.__environment
    stream = self.__stream
    return_type = operation.returnType()
    deref_return_type = tyutil.deref(return_type)
    return_type_name = environment.principalID(return_type, fully_scope = 1)
    return_dims = tyutil.typeDims(return_type)
    return_is_array = return_dims != []
    has_return_value = not(tyutil.isVoid(deref_return_type))
    has_arguments = operation.parameters() != []
    has_exceptions = operation.raises() != []

    identifier = tyutil.mapID(operation.identifier())

    signature = mangler.produce_operation_signature(operation)

    # if a descriptor already exists for this signature, we don't
    # need to generate a whole new proxy
    try:
        descriptor = mangler.operation_descriptor_name(operation)
        need_proxy = 0
    except KeyError:
        mangler.generate_descriptor(seed, signature)
        descriptor = mangler.operation_descriptor_name(operation)
        need_proxy = 1

    if not(need_proxy):
        return

    # if no arguments and no return value and no exceptions then
    # no proxy call descriptor
    if not(has_return_value) and not(has_arguments) and \
       not(has_exceptions):
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
    for parameter in operation.parameters():
        n = n + 1
        param_type = parameter.paramType()
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
        virtual void marshalArguments(GIOP_C&);"""
    if need_to_unmarshal:
        unmarshal_arguments_decl = """\
        virtual void unmarshalReturnedValues(GIOP_C&);"""

    # are there any user exceptions
    user_exceptions_decl = ""
    raises = operation.raises()
    has_user_exceptions = raises != []
    if has_user_exceptions:
        user_exceptions_decl = \
                             "virtual void userException(GIOP_C&, const char*);"
    
    # Write the proxy class definition
    stream.out(proxy_class_template,
               signature = signature,
               call_descriptor = descriptor,
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
        size_calculation = util.StringStream()
        marshal_block = util.StringStream()
        n = -1
        for parameter in operation.parameters():
            n = n + 1
            if parameter.is_in():
                param_type = parameter.paramType()
                is_pointer = 0
                if tyutil.isTypeCode(param_type):
                    is_pointer = 1
                name = "arg_" + str(n)
                size_calculation.out(skutil.sizeCalculation(
                    environment, param_type, None, "msgsize", name,
                    fixme = 1, is_pointer = is_pointer, fully_scope = 1))
                skutil.marshall(marshal_block, environment, param_type,
                                None, name, "giop_client",
                                fully_scope = 1)
                
        # write the alignment function
        stream.out(alignment_template,
                   call_descriptor = descriptor,
                   size_calculation = str(size_calculation))
        # write the marshal function
        stream.out(marshal_template,
                   call_descriptor = descriptor,
                   marshal_block = str(marshal_block))

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
        pre_decls = util.StringStream()
        unmarshal_block = util.StringStream()
        post_assign = util.StringStream()

        def unmar_via_temp(type, name, temp, temp_type_name, init_value,
                           direction, dereference = 0,
                           pre_decls = pre_decls,
                           unmarshal_block = unmarshal_block,
                           post_assign = post_assign,
                           environment = environment):
            dims = tyutil.typeDims(type)
            is_array = dims != []
            deref_type = tyutil.deref(type)
            is_variable = tyutil.isVariableType(deref_type)
            is_out = direction == 1
            is_inout = direction == 2
            assert is_out or is_inout

            deref_dims_type = tyutil.derefKeepDims(type)

            if tyutil.isObjRef(deref_type) and not(is_array):
                helper_name = temp_type_name + "_Helper"
                temp_type_name = temp_type_name + "_ptr"
            elif tyutil.isTypeCode(deref_type) and not(is_array):
                if direction == 2:
                    temp_type_name = temp_type_name + "_var"
                else:
                    temp_type_name = temp_type_name + "_ptr"

            if is_out:
                pre_decls.out("""\
@temp_type_name@ @temp@ = @init_value@;""", temp_type_name = temp_type_name,
                              temp = temp, init_value = init_value)
            elif is_inout:
                unmarshal_block.out("""\
@temp_type_name@ @temp@;""", temp_type_name = temp_type_name,
                                    temp = temp)

            # This bit is almost certainly wrong. It seems that when an
            # argument is a typedef to an array type, the non-derefed
            # type is used in the typedec but a dereferenced slice is used
            # elsewhere. Why?
            if is_array:
                temp_type_name = environment.principalID(deref_dims_type,
                                                         fully_scope = 1) +\
                                                         "_slice*"
                
            if dereference:
                deref_name = "*" + temp
            elif is_array and not(isinstance(deref_type, idltype.Base)):
                deref_name = "((" + temp_type_name + ") " + temp + ")"
            else:
                deref_name = temp

                
            skutil.unmarshall(unmarshal_block, environment, type, None,
                              deref_name, can_throw_marshall = 1,
                              from_where = "giop_client",
                              fully_scope = 1,
                              string_via_member = 1)

            if is_inout and not(is_array):
                if tyutil.isString(deref_type):
                    unmarshal_block.out("""\
CORBA::string_free(@name@);
@name@ = @temp@;""", name = name, temp = temp)
                elif tyutil.isObjRef(deref_type):
                    unmarshal_block.out("""\
@helper_name@::release(@name@);
@name@ = @temp@;""", helper_name = helper_name,
                                        name = name, temp = temp)
                elif tyutil.isTypeCode(deref_type):
                    unmarshal_block.out("""\
CORBA::release(@name@);
@name@ = @temp@._retn();""", name = name, temp = temp)
                

            if is_out:
                post_assign.out("""\
@name@ = @temp@;""", name = name, temp = temp)

        # handle the return value, if one exists. This could be
        # simplified- surely having a return type is semantically
        # identical to having a similarly named out argument?
        if has_return_value:
            name = "pd_result"
            deref_dims_return_type = tyutil.derefKeepDims(return_type)
            # Something strange is happening here.
            # Need to investigate the array mapping further at a later stage
            # hopefully to remove this oddity
            if return_is_array:
                return_type_base = environment.principalID(deref_dims_return_type,
                                                           fully_scope = 1)
            else:
                return_type_base = environment.principalID(return_type,
                                                           fully_scope = 1)
            return_is_variable = tyutil.isVariableType(return_type)
            # we need to allocate storage if the return is an
            # array
            if return_is_array:
                if return_is_variable                or \
                   tyutil.isStruct(deref_return_type) or \
                   tyutil.isUnion(deref_return_type):
                    name = "((" + return_type_base + "_slice*) pd_result)"
                unmarshal_block.out("""\
pd_result = @type@_alloc();""", type = return_type_base)
            elif return_is_variable and (tyutil.isStruct(deref_return_type) or \
                                         tyutil.isUnion(deref_return_type)  or \
                                         tyutil.isSequence(deref_return_type) or \
                                         tyutil.isAny(deref_return_type)):
                unmarshal_block.out("""\
pd_result = new @type@;""", type = return_type_base)
                name = "*" + name

            skutil.unmarshall(unmarshal_block, environment, return_type, None,
                              name, can_throw_marshall = 1,
                              from_where = "giop_client",
                              fully_scope = 1,
                              string_via_member = 1)
        
        n = -1
        for parameter in operation.parameters():
            n = n + 1
            direction = parameter.direction()
            # don't need to worry about in arguments!
            if direction == 0:
                continue
            
            arg_name = "arg_" + str(n)
            temp_name = "tmp_" + str(n)
            param_type = parameter.paramType()
            deref_param_type = tyutil.deref(param_type)
            deref_dims_param_type = tyutil.derefKeepDims(param_type)
            param_type_name = environment.principalID(param_type,
                                                      fully_scope = 1)
            deref_dims_name = environment.principalID(deref_dims_param_type,
                                                      fully_scope = 1)
            dims = tyutil.typeDims(param_type)
            is_array = dims != []
            is_out = direction == 1
            in_inout = direction == 2
            is_variable = tyutil.isVariableType(deref_param_type)

            # for variable structs and unions, we need to *name
            dereference = 0
            # do we unmarshal via a temporary?
            # we need to setup the temporary variables
            temp_init_value = None
            if (isinstance(deref_param_type, idltype.Base) and \
                not tyutil.isAny(deref_param_type)         and \
                not tyutil.isTypeCode(deref_param_type)) or \
               tyutil.isEnum(deref_param_type):
                via_tmp = 0
            elif is_array:
                if is_variable and is_out:
                    via_tmp = 1
                    # the temporary variable is an array slice
                    temp_type_name = param_type_name + "_slice*"
                    temp_init_value = deref_dims_name + "_alloc()"
                else:
                    via_tmp = 0
            elif is_variable:
                if tyutil.isString(deref_param_type):
                    via_tmp = 1
                    temp_type_name = "char*"
                    temp_init_value = "0"
                elif tyutil.isObjRef(deref_param_type):
                    via_tmp = 1
                    temp_type_name = param_type_name
                    temp_init_value = "0"
                elif tyutil.isTypeCode(deref_param_type):
                    via_tmp = 1
                    temp_type_name = param_type_name
                    temp_init_value = "CORBA::TypeCode::_nil()"
                elif tyutil.isStruct(deref_param_type) or \
                     tyutil.isUnion(deref_param_type)  or \
                     tyutil.isSequence(deref_param_type) or \
                     tyutil.isAny(deref_param_type):
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
                skutil.unmarshall(unmarshal_block, environment, param_type, None,
                                  arg_name, can_throw_marshall = 1,
                                  from_where = "giop_client",
                                  fully_scope = 1,
                                  string_via_member = 1)

        # write the unmarshal function
        stream.out(unmarshal_template,
                   call_descriptor = descriptor,
                   pre_decls = str(pre_decls),
                   unmarshal_block = str(unmarshal_block),
                   post_assign = str(post_assign))


    # -------------------------------------------------------------
    
    # consider user exceptions
    if has_user_exceptions:
        block = util.StringStream()
        first_one = 1

        for exception in raises:
            scopedName = exception.scopedName()
            repoID = scopedName + ["_PD_repoId"]
            repoID_str = environment.nameToString(repoID)
            exname = environment.nameToString(scopedName)
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
}""", switch = switch, repoID_str = repoID_str, exname = exname)

        # write the user exception template
        stream.out(exception_template,
                   call_descriptor = descriptor,
                   exception_block = str(block))
        

        
def attribute(attribute, seed):
    assert isinstance(attribute, idlast.Attribute)

    environment = self.__environment
    stream = self.__stream

    read_signature = mangler.produce_read_attribute_signature(attribute)
    write_signature = mangler.produce_write_attribute_signature(attribute)
    need_read_proxy = 1
    need_write_proxy = 1

    # Consider each proxy separately, since it is possible to
    # have operations which have the same signature as one proxy,
    # but not the other.
    try:
        # check for a read proxy
        mangler.attribute_read_descriptor_name(attribute)
        need_read_proxy = 0
    except KeyError:
        mangler.generate_descriptor(seed, read_signature)
    try:
        # check for a write proxy
        mangler.attribute_write_descriptor_name(attribute)
        need_write_proxy = 0
    except KeyError:
        mangler.generate_descriptor(seed, write_signature)
        
    if not(need_read_proxy or need_write_proxy):
        return
    
    read_desc = mangler.attribute_read_descriptor_name(attribute)
    write_desc = mangler.attribute_write_descriptor_name(attribute)

    attrType = attribute.attrType()
    attr_dims = tyutil.typeDims(attrType)
    is_array = attr_dims != []   
    deref_attrType = tyutil.deref(attrType)
    attrType_name = environment.principalID(attrType, 1)

    fully_scoped_attrTypes = tyutil.operationArgumentType(attrType,
                                                          environment, 0,
                                                          fully_scope = 1)
    attrTypes = tyutil.operationArgumentType(attrType, environment, 0,
                                             fully_scope = 0)

    return_type = fully_scoped_attrTypes[0]
    if is_array:
        in_type = attrTypes[1]+"_slice*"
        fully_scoped_in_type = fully_scoped_attrTypes[1]+"_slice*"
    else:
        in_type = attrTypes[1]
        fully_scoped_in_type = fully_scoped_attrTypes[1]

    size = skutil.sizeCalculation(environment, attrType, None ,
                                  "msgsize",
                                  "arg_0", 1, fully_scope = 1)
    marshal_arg = util.StringStream()
    skutil.marshall(marshal_arg, environment, attrType, None,
                    "arg_0", "giop_client", fully_scope = 1)

    if is_array:
        s = util.StringStream()
        s.out("pd_result = @name@_alloc();", name = attrType_name)
        # basic types don't have slices
        if tyutil.ttsMap.has_key(deref_attrType.kind()):
            result_string = "pd_result"
        else:
            result_string = "((" + attrType_name + "_slice*)" + "pd_result)"

        skutil.unmarshall(s, environment, attrType, None, result_string, 1,
                          "giop_client", fully_scope = 1)
        unmarshal_ret = str(s)
            
    elif tyutil.isString(deref_attrType):
        unmarshal_ret = skutil.unmarshal_string_via_temporary("pd_result",
                                                              "giop_client")
    elif tyutil.isObjRef(deref_attrType):
        unmarshal_ret = "\
pd_result = " + attrType_name + "_Helper::unmarshalObjRef(giop_client);"
    elif tyutil.isVariableType(deref_attrType):
        unmarshal_ret = "\
pd_result = new " + attrType_name + ";\n" + "\
*pd_result <<= giop_client;"
    else:
        unmarshal_ret = """\
pd_result <<= giop_client;"""

    # -------------------------------------------------------------

    ctor_args = "LocalCallFn lcfn, const char* op, " +\
                "size_t oplen, _CORBA_Boolean oneway"
    inherits_list = "omniCallDescriptor(lcfn, op, oplen, oneway)"
    if need_read_proxy:
        # write the read class template
        unmarshal_decl = "virtual void unmarshalReturnedValues(GIOP_C&);"
        result_mem_fn = "inline " + return_type + " result() { return pd_result; }"
        result_mem_data = return_type + " pd_result;"
        stream.out(proxy_class_template,
                   signature = read_signature,
                   call_descriptor = read_desc,
                   ctor_args = ctor_args,
                   inherits_list = inherits_list,
                   marshal_arguments_decl = "",
                   user_exceptions_decl = "",
                   unmarshal_arguments_decl = unmarshal_decl,
                   result_member_function = result_mem_fn,
                   member_data = "",
                   result_member_data = result_mem_data)

        # -------------------------------------------------------------
    
        # write the read unmarshalReturned function
        stream.out(unmarshal_template,
                   call_descriptor = read_desc,
                   pre_decls = "",
                   unmarshal_block = unmarshal_ret,
                   post_assign = "")

    # -------------------------------------------------------------

    if need_write_proxy:
        # write the write class template
        ctor_args = ctor_args + ", " + fully_scoped_in_type + " a_0"
        inherits_list = inherits_list + ",\n" + "arg_0(a_0)"
        marshal_decl = "virtual CORBA::ULong alignedSize(CORBA::ULong);\n" +\
                       "virtual void marshalArguments(GIOP_C&);"
        member_data = fully_scoped_in_type + " arg_0;"
        stream.out(proxy_class_template,
                   signature = write_signature,
                   call_descriptor = write_desc,
                   ctor_args = ctor_args,
                   inherits_list = inherits_list,
                   marshal_arguments_decl = marshal_decl,
                   user_exceptions_decl = "",
                   unmarshal_arguments_decl = "",
                   result_member_function = "",
                   member_data = member_data,
                   result_member_data = "") 
        
        # -------------------------------------------------------------
        
        # write the write alignment template
        stream.out(alignment_template,
                   call_descriptor = write_desc,
                   size_calculation = size)

        # -------------------------------------------------------------
        
        # write the write marshal template
        stream.out(marshal_template,
                   call_descriptor = write_desc,
                   marshal_block = str(marshal_arg))
