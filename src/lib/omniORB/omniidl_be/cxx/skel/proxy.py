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

def unmarshal(type, name, from_where, to, can_throw_marshal, fully_scope = 0):
    assert isinstance(type, idltype.Type)
    environment = self.__environment
    deref_type = tyutil.deref(type)
    deref_type_name = environment.principalID(deref_type, fully_scope)
    type_dims = tyutil.typeDims(type)
    # for a type with dimensions [10][20][30]....[n][n+1]
    # we iterate over the last dimension, so we need to use
    # [10][20][30]....[n]
    dims_string = tyutil.dimsToString(type_dims[0:-1])

    is_array = type_dims != []
    num_elements = reduce(lambda x,y:x*y, type_dims, 1)

    type_name = environment.principalID(type, fully_scope)

    if is_array:
        # BASIC things
        # octets, chars and booleans are handled via
        # get_char_array
        if tyutil.isOctet(deref_type) or \
           tyutil.isChar(deref_type)  or \
           tyutil.isBoolean(deref_type):
            to.out("""\
@where@.get_char_array((_CORBA_Char*) ((@type@*) @name@@dims@), @num@);""",
                       where = from_where, type = deref_type_name,
                       name = name, dims = dims_string,
                       num = str(num_elements))
            return
        # other basic types are handled via a CdrStreamHelper
        array_helper_suffix = {
            idltype.tk_short:  "Short",
            idltype.tk_long:   "Long",
            idltype.tk_ushort: "UShort",
            idltype.tk_ulong:  "ULong",
            idltype.tk_float:  "Float",
            idltype.tk_double: "Double",
            idltype.tk_enum:   "ULong",
            }
        if array_helper_suffix.has_key(deref_type.kind()):
            typecast = "((" + deref_type_name + "*) " + name + dims_string + ")"
            # use the most dereferenced type
            if tyutil.isEnum(deref_type):
                typecast = "(_CORBA_ULong*) " + typecast
            to.out("""\
CdrStreamHelper_unmarshalArray@suffix@(@where@,@typecast@, @num@);""",
                   suffix = array_helper_suffix[deref_type.kind()],
                   where = from_where, typecast = typecast,
                   num = str(num_elements))
            return
        # not sure how to handle other basic types
        if isinstance(deref_type, idltype.Base):
            raise "Don't know how to marshall type: " + repr(deref_type) +\
              "(kind = " + str(deref_type.kind()) + ") array"

    # superfluous bracketting
    if is_array:
        to.out("{")
        to.inc_indent()
        
    # some uniformity here
    istring = skutil.start_loop(to, type_dims)

    element_name = name + istring
    
    if tyutil.isString(deref_type):
        if not(is_array):
            # go via temporary. why?
            to.out("""\
{
  CORBA::String_member _0RL_str_tmp;
  _0RL_str_tmp <<=  @where@;
  @name@ = _0RL_str_tmp._ptr;
  _0RL_str_tmp._ptr = 0;
}""", where = from_where, name = element_name)
        else:
            to.out("""\
  CORBA::ULong _len;
  _len <<= @where@;
  if (!_len) {
    if (omniORB::traceLevel > 1)
      _CORBA_null_string_ptr(1);
    _len = 1;
  }""", where = from_where)            
            if can_throw_marshal:
                to.out("""\
  else if ( @where@.RdMessageUnRead() < _len)
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);""",
                           where = from_where)
                to.dec_indent()
            to.out("""\
  if (!(char*)(@name@ = CORBA::string_alloc(_len-1)))
    throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
  if (_len > 1)
    @where@.get_char_array((CORBA::Char *)((char *)@name@),_len);
  else
    *((CORBA::Char*)((char*) @name@)) <<= @where@ ;""",
                   where = from_where, name = element_name)
            to.dec_indent()            


    elif tyutil.isObjRef(deref_type):
        base_type_name = environment.principalID(deref_type, fully_scope)
        to.out("""\
  @name@ = @type@_Helper::unmarshalObjRef(@where@);""",
                   name = element_name, type = base_type_name,
                   where = from_where)

    else:
        to.out("""\
  @name@ <<= @where@;""", where = from_where,
                   name = element_name)

    skutil.finish_loop(to, type_dims)

    if is_array:
        to.dec_indent()
        to.out("}")

def argmapping(type):
    assert isinstance(type, idltype.Type)
    environment = self.__environment

    deref_type = tyutil.deref(type)
    type_dims = tyutil.typeDims(type)
    is_array = type_dims != []
    is_variable = tyutil.isVariableType(deref_type)

    type_name = environment.principalID(type, fully_scope = 1)

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
                           post_assign = post_assign):
            dims = tyutil.typeDims(type)
            is_array = dims != []
            deref_type = tyutil.deref(type)
            is_variable = tyutil.isVariableType(deref_type)
            is_out = direction == 1
            is_inout = direction == 2
            assert is_out or is_inout

            if tyutil.isObjRef(deref_type) and not(is_array):
                helper_name = temp_type_name + "_Helper"
                temp_type_name = temp_type_name + "_ptr"

            if is_out:
                pre_decls.out("""\
@temp_type_name@ @temp@ = @init_value@;""", temp_type_name = temp_type_name,
                              temp = temp, init_value = init_value)
            elif is_inout:
                unmarshal_block.out("""\
@temp_type_name@ @temp@;""", temp_type_name = temp_type_name,
                                    temp = temp)
            if dereference:
                deref_name = "*" + temp
            elif is_array and not(isinstance(deref_type, idltype.Base)):
                deref_name = "((" + temp_type_name + ") " + temp + ")"
            else:
                deref_name = temp
            unmarshal(type, deref_name, "giop_client", unmarshal_block,
                      can_throw_marshal = 1,
                      fully_scope = 1)

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
                

            if is_out:
                post_assign.out("""\
@name@ = @temp@;""", name = name, temp = temp)

        # handle the return value, if one exists. This could be
        # simplified- surely having a return type is semantically
        # identical to having a similarly named out argument?
        if has_return_value:
            name = "pd_result"
            return_type_base = environment.principalID(return_type,
                                                       fully_scope = 1)
            return_is_variable = tyutil.isVariableType(return_type)
            # we need to allocate storage if the return is an
            # array
            if return_is_array:
                if return_is_variable or tyutil.isStruct(deref_return_type):
                    name = "((" + return_type_base + "_slice*) pd_result)"
                unmarshal_block.out("""\
pd_result = @type@_alloc();""", type = return_type_base)
            elif return_is_variable and (tyutil.isStruct(deref_return_type) or \
                                         tyutil.isUnion(deref_return_type)  or \
                                         tyutil.isSequence(deref_return_type)):
                unmarshal_block.out("""\
pd_result = new @type@;""", type = return_type_base)
                name = "*" + name

            unmarshal(return_type, name, "giop_client",
                      unmarshal_block, can_throw_marshal = 1,
                      fully_scope = 1)
                
        
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
            param_type_name = environment.principalID(param_type,
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
            if isinstance(deref_param_type, idltype.Base) or \
               tyutil.isEnum(deref_param_type):
                via_tmp = 0
            elif is_array:
                if is_variable and is_out:
                    via_tmp = 1
                    # the temporary variable is an array slice
                    temp_type_name = param_type_name + "_slice*"
                    temp_init_value = param_type_name + "_alloc()"
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
                elif tyutil.isStruct(deref_param_type) or \
                     tyutil.isUnion(deref_param_type)  or \
                     tyutil.isSequence(deref_param_type):
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
                unmarshal(param_type, arg_name, "giop_client",
                          unmarshal_block,
                          can_throw_marshal = 1,
                          fully_scope = 1)
            

        # write the unmarshal function
        stream.out(unmarshal_template,
                   call_descriptor = descriptor,
                   pre_decls = str(pre_decls),
                   unmarshal_block = str(unmarshal_block),
                   post_assign = str(post_assign))


                    

