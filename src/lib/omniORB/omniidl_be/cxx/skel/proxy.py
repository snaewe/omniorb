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
# Revision 1.18  2000/07/13 15:25:59  dpg1
# Merge from omni3_develop for 3.0 release.
#
# Revision 1.15.2.6  2000/05/31 18:03:40  djs
# Better output indenting (and preprocessor directives now correctly output at
# the beginning of lines)
# Calling an exception "e" resulted in a name clash (and resultant C++
# compile failure)
#
# Revision 1.15.2.5  2000/04/26 18:22:56  djs
# Rewrote type mapping code (now in types.py)
# Rewrote identifier handling code (now in id.py)
# Removed superfluous externs in front of function definitions
#
# Revision 1.15.2.4  2000/04/05 10:58:03  djs
# Scoping problem with generated proxies for attributes (not operations)
#
# Revision 1.15.2.3  2000/03/20 11:50:27  djs
# Removed excess buffering- output templates have code attached which is
# lazily evaluated when required.
#
# Revision 1.15.2.2  2000/02/16 16:30:04  djs
# Fix to proxy call descriptor code- failed to handle special case of
#   Object method(in string x)
#
# Revision 1.15.2.1  2000/02/14 18:34:53  dpg1
# New omniidl merged in.
#
# Revision 1.15  2000/01/19 17:05:16  djs
# Modified to use an externally stored C++ output template.
#
# Revision 1.14  2000/01/11 12:02:46  djs
# More tidying up
#
# Revision 1.13  2000/01/10 18:42:22  djs
# Removed redundant code, tidied up.
#
# Revision 1.12  2000/01/07 20:31:33  djs
# Regression tests in CVSROOT/testsuite now pass for
#   * no backend arguments
#   * tie templates
#   * flattened tie templates
#   * TypeCode and Any generation
#
# Revision 1.11  1999/12/25 21:47:19  djs
# Better TypeCode support
#
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
from omniidl_be.cxx import util, tyutil, skutil, id, types
from omniidl_be.cxx.skel import mangler, template


import proxy
self = proxy

def __init__(environment, stream):
    self.__stream = stream
    return self


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


def operation(operation):
    stream = self.__stream
    return_type = types.Type(operation.returnType())
    d_return_type = return_type.deref()
    return_type_name = return_type.base()
    return_dims = return_type.dims()
    return_is_array = return_dims != []
    has_return_value = not(d_return_type.void())
    has_arguments = operation.parameters() != []
    has_exceptions = operation.raises() != []

    identifier = id.mapID(operation.identifier())

    signature = mangler.produce_operation_signature(operation)

    # if a descriptor already exists for this signature, we don't
    # need to generate a whole new proxy
    try:
        descriptor = mangler.operation_descriptor_name(operation)
        need_proxy = 0
    except KeyError:
        mangler.generate_descriptor(signature)
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
    raises = operation.raises()
    has_user_exceptions = raises != []
    if has_user_exceptions:
        user_exceptions_decl = \
"virtual void userException(GIOP_C&, const char*);\n"
    
    # Write the proxy class definition
    stream.out(template.interface_proxy_class,
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
                param_type = types.Type(parameter.paramType())
                d_param_type = param_type.deref()
                #deref_param_type = tyutil.deref(param_type)
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
                   call_descriptor = descriptor,
                   size_calculation = str(size_calculation))
        # write the marshal function
        stream.out(template.interface_proxy_marshal,
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
        for parameter in operation.parameters():
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
                   call_descriptor = descriptor,
                   exception_block = str(block))
        

        
def attribute(attribute):
    assert isinstance(attribute, idlast.Attribute)

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
        mangler.generate_descriptor(read_signature)
    try:
        # check for a write proxy
        mangler.attribute_write_descriptor_name(attribute)
        need_write_proxy = 0
    except KeyError:
        mangler.generate_descriptor(write_signature)
        
    if not(need_read_proxy or need_write_proxy):
        return
    
    read_desc = mangler.attribute_read_descriptor_name(attribute)
    write_desc = mangler.attribute_write_descriptor_name(attribute)

    attrType = types.Type(attribute.attrType())
    attr_dims = attrType.dims()
    is_array = attr_dims != []   
    d_attrType = attrType.deref()
    attrType_name = attrType.base()

    return_type = attrType.op(types.RET)

    in_type = attrType.op(types.IN)
    if is_array:
        in_type = in_type + "_slice*"

    is_pointer = 0
    if d_attrType.typecode():
        is_pointer = 1

    size = skutil.sizeCalculation(None, attrType, None ,
                                  "msgsize",
                                  "arg_0", fixme = 1,
                                  is_pointer = is_pointer)
    marshal_arg = util.StringStream()
    skutil.marshall(marshal_arg, None, attrType, None,
                    "arg_0", "giop_client")


    if is_array:
        s = util.StringStream()
        s.out("pd_result = @name@_alloc();", name = attrType_name)
        # basic types don't have slices
        if types.basic_map.has_key(d_attrType.type().kind()):
            result_string = "pd_result"
        else:
            result_string = "((" + attrType_name + "_slice*)" + "pd_result)"

        skutil.unmarshall(s, None, attrType, None, result_string, 1,
                          "giop_client")
        unmarshal_ret = str(s)
            
    elif d_attrType.string():
        unmarshal_ret = skutil.unmarshal_string_via_temporary("pd_result",
                                                              "giop_client")
    elif d_attrType.objref():
        if d_attrType.type().scopedName() == ["CORBA", "Object"]:
            objref_helper = "CORBA::Object_Helper"
        else:
            objref_helper = attrType_name + "_Helper"
        unmarshal_ret = "\
pd_result = " + objref_helper + "::unmarshalObjRef(giop_client);\n"
    elif d_attrType.typecode():
        unmarshal_ret = "\
pd_result = CORBA::TypeCode::unmarshalTypeCode(giop_client);\n"
    elif d_attrType.variable():

        unmarshal_ret = "\
pd_result = new " + attrType_name + ";\n" + "\
*pd_result <<= giop_client;\n"
    else:
        unmarshal_ret = """\
pd_result <<= giop_client;
"""

    # -------------------------------------------------------------

    ctor_args = "LocalCallFn lcfn, const char* op, " +\
                "size_t oplen, _CORBA_Boolean oneway"
    inherits_list = "omniCallDescriptor(lcfn, op, oplen, oneway)"
    if need_read_proxy:
        # write the read class template
        unmarshal_decl = "virtual void unmarshalReturnedValues(GIOP_C&);\n"
        result_mem_fn = "inline " + return_type + " result() { return pd_result; }\n"
        result_mem_data = return_type + " pd_result;\n"
        stream.out(template.interface_proxy_class,
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
        stream.out(template.interface_proxy_unmarshal,
                   call_descriptor = read_desc,
                   pre_decls = "",
                   unmarshal_block = unmarshal_ret,
                   post_assign = "")

    # -------------------------------------------------------------

    if need_write_proxy:
        # write the write class template
        ctor_args = ctor_args + ", " + in_type + " a_0"
        inherits_list = inherits_list + ",\n" + "arg_0(a_0)"
        marshal_decl = "virtual CORBA::ULong alignedSize(CORBA::ULong);\n" +\
                       "virtual void marshalArguments(GIOP_C&);\n"
        member_data = in_type + " arg_0;\n"
        stream.out(template.interface_proxy_class,
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
        stream.out(template.interface_proxy_alignment,
                   call_descriptor = write_desc,
                   size_calculation = size)

        # -------------------------------------------------------------
        
        # write the write marshal template
        stream.out(template.interface_proxy_marshal,
                   call_descriptor = write_desc,
                   marshal_block = str(marshal_arg))


