# -*- python -*-
#                           Package   : omniidl
# skutil.py                 Created on: 1999/11/15
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
#   Skeleton utility functions designed for the C++ backend

# $Id$
# $Log$
# Revision 1.6  1999/11/29 15:26:04  djs
# Marshalling bugfixes.
#
# Revision 1.5  1999/11/26 18:52:06  djs
# Bugfixes and refinements
#
# Revision 1.4  1999/11/23 18:49:26  djs
# Lots of fixes, especially marshalling code
# Added todo list to keep track of issues
#
# Revision 1.3  1999/11/19 20:07:33  djs
# Bugfixes. Added utility functions for operations and attributes
#
# Revision 1.2  1999/11/17 20:37:09  djs
# General util functions
#
# Revision 1.1  1999/11/15 19:10:55  djs
# Added module for utility functions specific to generating skeletons
# Union skeletons working
#

import string

from omniidl import idlutil, idltype, idlast

from omniidl.be.cxx import util, tyutil


# Build a loop iterating over every element of a multidimensional
# thing and return the indexing string
def start_loop(where, dims, prefix = "_i"):
    index = 0
    istring = ""
    for dimension in dims:
        where.out("""\
for (CORBA::ULong @prefix@@n@ = 0;@prefix@@n@ < @dim@;_i@n@++) {""",
                  prefix = prefix,
                  n = str(index),
                  dim = str(dimension))
        where.inc_indent()
        istring = istring + "[" + prefix + str(index) + "]"
        index = index + 1
    return istring

# Finish the loop
def finish_loop(where, dims):
    for dummy in dims:
        where.dec_indent()
        where.out("}")
    

# deprecated
def begin_loop(string, full_dims):
    string.out("{")
    string.inc_indent()
    return start_loop(string, full_dims)

# deprecated
def end_loop(string, full_dims):
    finish_loop(string, full_dims)
            
    string.dec_indent()
    string.out("}")

def marshall_struct_union(string, environment, type, decl, argname, to="_n"):
    assert isinstance(type, idltype.Type)
    if decl:
        assert isinstance(decl, idlast.Declarator)
        dims = decl.sizes()
    else:
        dims = []
    type_dims = tyutil.typeDims(type)
    full_dims = dims + type_dims
    is_array        = full_dims != []

    if not(is_array):
        # marshall the simple way
        string.out("""\
   @name@ >>= @to@;""", name = argname, to = to)
    else:
        # do it the normal way
        marshall(string, environment, type, decl, argname, to)
        

def marshall(string, environment, type, decl, argname, to="_n",
             fully_scope = 0):
    assert isinstance(type, idltype.Type)
    if decl:
        assert isinstance(decl, idlast.Declarator)
        dims = decl.sizes()
    else:
        dims = []
        
    deref_type = tyutil.deref(type)

    type_dims = tyutil.typeDims(type)
    full_dims = dims + type_dims

    # for some reason, a char[10][20][30] x
    # becomes put_char_array(.... x[10][20]...)
    dims_string = tyutil.dimsToString(full_dims[0:-1])

    anonymous_array = dims      != []
    is_array        = full_dims != []
    alias_array     = type_dims != []

    num_elements = reduce(lambda x,y:x*y, full_dims, 1)

    type_name = environment.principalID(deref_type, fully_scope)

    # marshall an array of basic things
    if is_array:
        if tyutil.typeSizeAlignMap.has_key(deref_type.kind()):
            (size, align) = tyutil.typeSizeAlignMap[deref_type.kind()]
            num_bytes = size * num_elements
            if align != 1:
                align_str = ", omni::ALIGN_" + str(align)
            else:
                align_str = ""
                
            string.out("""\
@to@.put_char_array((const _CORBA_Char*) ((const @type@*) @argname@@dims_string@), @size@@align@);""",
                       type = type_name, argname = argname,
                       dims_string = dims_string,
                       size = str(num_bytes), align = align_str, to = to)
            return

    if tyutil.isString(deref_type):
        indexing_string = begin_loop(string, full_dims)
        string.out("""\
    CORBA::ULong _len = (((const char*) @argname@@indexing_string@)? strlen((const char*) @argname@@indexing_string@) + 1 : 1);
    _len >>= @to@;
    if (_len > 1)
      @to@.put_char_array((const CORBA::Char *)((const char*)@argname@@indexing_string@),_len);
    else {
      if ((const char*) @argname@@indexing_string@ == 0 && omniORB::traceLevel > 1)
        _CORBA_null_string_ptr(0);
      CORBA::Char('\\0') >>= @to@;
    }""",
                   argname = argname,
                   indexing_string = indexing_string,
                   to = to)
        end_loop(string, full_dims)
    elif tyutil.isObjRef(deref_type):
        indexing_string = ""
        if is_array:
            indexing_string = begin_loop(string, full_dims) + "._ptr"
        string.out("""\
    @type_name@_Helper::marshalObjRef(@argname@@indexing_string@,@to@);""",
                   type_name = type_name,
                   argname = argname,
                   indexing_string = indexing_string,
                   to = to)
        if is_array:
            end_loop(string, full_dims)
    else:
        if is_array:
            indexing_string = begin_loop(string, full_dims)
        else:
            indexing_string = ""
        string.out("""\
    @argname@@indexing_string@ >>= @to@;""",
                   argname = argname, indexing_string = indexing_string,
                   to = to)
        if is_array:
            end_loop(string, full_dims)
    
    return

def unmarshall_struct_union(string, environment, type, decl, argname,
                            can_throw_marshall, from_where="_n"):
    assert isinstance(type, idltype.Type)
    if decl:
        assert isinstance(decl, idlast.Declarator)
        dims = decl.sizes()
    else:
        dims = []
    type_dims = tyutil.typeDims(type)
    full_dims = dims + type_dims
    is_array        = full_dims != []

    if not(is_array):
        # unmarshall the simple way
        string.out("""\
   @name@ <<= @from_where@;""", name = argname, from_where = from_where)
    else:
        # do it the normal way
        unmarshall(string, environment, type, decl, argname,
                   can_throw_marshall, from_where)


def unmarshall(string, environment, type, decl, argname,
               can_throw_marshall, from_where = "_n",
               fully_scope = 0):
    assert isinstance(type, idltype.Type)
    if decl:
        assert isinstance(decl, idlast.Declarator)
        dims = decl.sizes()
    else:
        dims = []
    
    deref_type = tyutil.deref(type)

    type_dims = tyutil.typeDims(type)
    full_dims = dims + type_dims

    # for some reason, a char[10][20][30] x
    # becomes put_char_array(.... x[10][20]...)
    dims_string = tyutil.dimsToString(full_dims[0:-1])

    anonymous_array = dims      != []
    is_array        = full_dims != []
    alias_array     = type_dims != []

    num_elements = reduce(lambda x,y:x*y, full_dims, 1)

    type_name = environment.principalID(deref_type, fully_scope)
    
    if 0 and not(is_array):
        string.out("""\
@argname@ <<= @from_where@;""", argname = argname, from_where = from_where)
        return

    # unmarshall an array of basic things
    #   octet and chars are handled directly
    if (tyutil.isOctet(deref_type) or tyutil.isChar(deref_type) or \
        tyutil.isBoolean(deref_type)) and is_array:
        string.out("""\
@from_where@.get_char_array((_CORBA_Char*) ((@type@*) @argname@@dims_string@), @num@);""",
                   argname = argname, type = type_name,
                   dims_string = dims_string,
                   num = str(num_elements), from_where = from_where)
        return
    #   other basic types are handled here
    array_helper_suffix = {
        idltype.tk_short:  "Short",
        idltype.tk_long:   "Long",
        idltype.tk_ushort: "UShort",
        idltype.tk_ulong:  "ULong",
        idltype.tk_float:  "Float",
        idltype.tk_double: "Double",
        }
    if isinstance(deref_type, idltype.Base) and is_array:
        if array_helper_suffix.has_key(deref_type.kind()):
           string.out("""\
CdrStreamHelper_unmarshalArray@suffix@(@from_where@, ((@type@*) @argname@@dims_string@), @num@);""",
                      suffix = array_helper_suffix[deref_type.kind()],
                      type = type_name,
                      argname = argname,
                      dims_string = dims_string,
                      num = str(num_elements),
                      from_where = from_where)
           return
        # basic type (boolean, any, tc, princ, ll, ull, ld, wc)
        raise "Don't know how to marshall type: " + repr(deref_type) +\
              "(kind = " + str(deref_type.kind()) + ") array"

    if not(is_array):
    #if tyutil.isObjRef(deref_type) or not(is_array):
        indexing_string = ""
    else:
        indexing_string = begin_loop(string, full_dims)
    
    if tyutil.isString(deref_type):
        string.out("""\
  CORBA::ULong _len;
  _len <<= @from_where@;
  if (!_len) {
    if (omniORB::traceLevel > 1)
      _CORBA_null_string_ptr(1);
    _len = 1;
  }""", name = argname, indexing_string = indexing_string,
                   from_where = from_where)
        if can_throw_marshall:
            string.out("""\
  else if ( @from_where@.RdMessageUnRead() < _len)
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);""",
                       from_where = from_where)
            string.dec_indent()
        string.out("""\
  if (!(char*)(@name@@indexing_string@ = CORBA::string_alloc(_len-1)))
    throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
  if (_len > 1)
    @from_where@.get_char_array((CORBA::Char *)((char *)@name@@indexing_string@),_len);
  else
    *((CORBA::Char*)((char*) @name@@indexing_string@)) <<= @from_where@ ;""",
                   name = argname, indexing_string = indexing_string,
                   from_where = from_where)
        string.dec_indent()
    elif tyutil.isObjRef(deref_type):
        string.out("""\
  @name@@indexing_string@ = @type@_Helper::unmarshalObjRef(@from_where@);""",
                   type = type_name,
                   name = argname,
                   indexing_string = indexing_string,
                   from_where = from_where)

    else:
        string.out("""\
  @name@@indexing_string@ <<= @from_where@;""",
                   name = argname, indexing_string = indexing_string,
                   from_where = from_where)

    if is_array:
    #if not(tyutil.isObjRef(deref_type)) and(is_array):
        end_loop(string, full_dims)

    
# ------------------------------------------------------------------

# FIXME - parameter is 1 when producing the size calc for an interface
# argument (not a struct member)
# (is this why tObjrefMember != tObjref in the old BE?)
def sizeCalculation(environment, type, decl, sizevar, argname, fixme = 0,
                    is_pointer = 0, fully_scope = 0):
    #o2be_operation::produceSizeCalculation
    assert isinstance(type, idltype.Type)
    
    # if decl == None then ignore
    if decl:
        assert isinstance(decl, idlast.Declarator)

    deref_type = tyutil.deref(type)

    if decl:
        dims = decl.sizes()
    else:
        dims = []
        
    type_dims = tyutil.typeDims(type)
    full_dims = dims + type_dims

    anonymous_array = dims      != []
    is_array        = full_dims != []
    alias_array     = type_dims != []

    num_elements = reduce(lambda x,y:x*y, full_dims, 1)

    isVariable = tyutil.isVariableType(type)

    if is_pointer:
        dereference = "->"
    else:
        dereference = "."

    string = util.StringStream()

    if not(is_array):
        if tyutil.typeSizeAlignMap.has_key(deref_type.kind()):
            size = tyutil.typeSizeAlignMap[deref_type.kind()][0]
            
            if size == 1:
                string.out("""\
@sizevar@ += 1;""", sizevar = sizevar)
                return str(string)

            string.out("""\
@sizevar@ = omni::align_to(@sizevar@, omni::ALIGN_@n@) + @n@;""",
                       sizevar = sizevar, n = str(size))
            return str(string)

        # FIXME:
        if fixme:
            if tyutil.isString(deref_type):
                string.out("""\
@sizevar@ = omni::align_to(@sizevar@, omni::ALIGN_4) + 4;
@sizevar@ += ((const char*) @argname@) ? strlen((const char*) @argname@) + 1 : 1;""",
                           sizevar = sizevar, argname = argname)
                return str(string)

        # FIXME:
        # this corresponds to case tObjref in the old BE
        # what is the difference between tObjrefMember and tObjref?
        if fixme:
            if tyutil.isObjRef(deref_type):
                name = environment.principalID(deref_type, fully_scope)
                string.out("""\
@sizevar@ = @name@_Helper::NP_alignedSize(@argname@, @sizevar@);""",
                           sizevar = sizevar, name = name, argname = argname)
                return str(string)

        # typecodes may be an exception here
        #print "[[[ sizevar = " + repr(sizevar) + "  argname = " +\
        #      repr(argname) + "]]]"
        string.out("""\
@sizevar@ = @argname@@deref@_NP_alignedSize(@sizevar@);""",
                   sizevar = sizevar, argname = argname, deref = dereference)
        return str(string)


    # thing is an array
    if not(isVariable):
        if tyutil.isOctet(deref_type):
            string.out("""\
@sizevar@ += @num_elements@;""", sizevar = sizevar,
                       num_elements = str(num_elements))
            return str(string)
        if tyutil.typeSizeAlignMap.has_key(deref_type.kind()):
            size = tyutil.typeSizeAlignMap[deref_type.kind()][0]

            if size == 1:
                string.out("""\
@sizevar@ += @num_elements@;""", sizevar = sizevar,
                           num_elements = str(num_elements))
                return str(string)

            string.out("""\
@sizevar@ = omni::align_to(@sizevar@, omni::ALIGN_@n@);
@sizevar@ += @num_elements@ * @n@;""",
                       sizevar = sizevar,
                       n = str(size),
                       num_elements = str(num_elements))
            return str(string)

        # must be an array of fixed structs or unions
        indexing_string = begin_loop(string, full_dims)

        # do the actual calculation
        string.out("""\
  @sizevar@ = @argname@@indexing_string@@deref@_NP_alignedSize(@sizevar@);""",
                   sizevar = sizevar, argname = argname,
                   indexing_string = indexing_string,
                   deref = dereference)

        end_loop(string, full_dims)

        return str(string)


    # thing is an array of variable sized elements
    indexing_string = begin_loop(string, full_dims)

    if tyutil.isString(deref_type):
        string.out("""\
@sizevar@ = omni::align_to(@sizevar@, omni::ALIGN_4);
@sizevar@ += 4 + (((const char*) @argname@@indexing_string@)? strlen((const char*) @argname@@indexing_string@) + 1 : 1);""",
                   sizevar = sizevar, argname = argname,
                   indexing_string = indexing_string)
        

    elif tyutil.isObjRef(deref_type):
        name = environment.principalID(deref_type, fully_scope)
        string.out("""\
@sizevar@ = @name@_Helper::NP_alignedSize(@argname@@indexing_string@._ptr,@sizevar@);""",
                   sizevar = sizevar, name = name, argname = argname,
                   indexing_string = indexing_string)

    else:
        # typecodes may be an exception here
        string.out("""\
@sizevar@ = @argname@@indexing_string@@deref@_NP_alignedSize(@sizevar@);""",
                   sizevar = sizevar, argname = argname,
                   indexing_string = indexing_string,
                   deref = dereference)
               

    end_loop(string, full_dims)
#    print "[[[ " + str(string) + "]]]"

    return str(string)


def unmarshal_string_via_temporary(variable_name, stream_name):
    stream = util.StringStream()
    stream.out("""\
{
  CORBA::String_member _0RL_str_tmp;
  _0RL_str_tmp <<= @stream_name@;
  @variable_name@ = _0RL_str_tmp._ptr;
  _0RL_str_tmp._ptr = 0;
}""",  variable_name = variable_name, stream_name = stream_name)

    return str(stream)
    
    

# -------------------------------------------
# Operation dispatching

def operation_dispatch(operation, environment, stream):

    id_name = tyutil.mapID(operation.identifier())

    parameters = operation.parameters()
    unmarshal_in = util.StringStream()
    marshal_out  = util.StringStream()
    size_calc    = util.StringStream()
    argument_list = []

    returnType = operation.returnType()
    deref_returnType = tyutil.deref(returnType)
    return_type_name = environment.principalID(returnType)
    return_type_dims = tyutil.typeDims(returnType)
    is_array = return_type_dims != []

    if tyutil.isString(returnType):
        return_type_name = "CORBA::String_var"
    elif is_array or tyutil.isVariableType(returnType):
        return_type_name = return_type_name + "_var"

    has_return_value = not(tyutil.isVoid(returnType))

    # item_types contains all the argument types + the result
    # item_names contains all the argument names + the result
    # item_direction contains the direction of all the arguments + result
    item_types = map(lambda x:x.paramType(), parameters)
    item_names = map(lambda x:"arg_" + x.identifier(), parameters)
    item_directions = map(lambda x:x.direction(), parameters)

    # 1st pass, build the invocation argument list
    zipped_up = util.zip(item_types, util.zip(item_names, item_directions))
    
    for (item_type, (item_name, item_direction)) in zipped_up:
        deref_item_type = tyutil.deref(item_type)
        type_dims = tyutil.typeDims(item_type)
        is_array = type_dims != []

        arg = ["", "", ""]
        arg[0] = item_name
        arg[1] = item_name
        arg[2] = item_name
        if tyutil.isString(deref_item_type):
            arg[0] = arg[0] + ".in()"
            arg[1] = arg[1] + ".out()"
            arg[2] = arg[2] + ".inout()"
        elif tyutil.isVariableType(deref_item_type):
            arg[1] = arg[1] + ".out()"

        argument_list.append(arg[item_direction])

        #if tyutil.isVariableType(deref_item_type) and \
        #   out_arg
        #if tyutil.isVariableType(deref_item_type) and \
        #   not(tyutil.isStruct(deref_item_type) or \
        #       tyutil.isUnion(deref_item_type)) and \
        #   not is_array:
        #    # and not sequence?
        #    if item_direction == 0:
        #        # in
        #        argument_list.append(item_name + ".in()")
        #    elif item_direction == 1:
        #        # out
        #        argument_list.append(item_name + ".out()")
        #    elif item_direction == 2:
        #        # inout
        #        argument_list.append(item_name + ".inout()")
        #elif (tyutil.isSequence(deref_item_type) or \
        #     tyutil.isStruct(deref_item_type) or \
        #     tyutil.isUnion(deref_item_type)) and \
        #     item_direction == 1:
        #    # out
        #    argument_list.append(item_name + ".out()")
        #else:
        #    argument_list.append(item_name)

    # 2nd thing- handle the return value
    if has_return_value:
        result_string = "result"
        is_pointer = 0
        if tyutil.isVariableType(deref_returnType) and \
           not(tyutil.isString(deref_returnType)):
        #if tyutil.isObjRef(deref_returnType) or \
        #   tyutil.isSequence(deref_returnType):
            result_string = "(result.operator->())"
            if not(tyutil.isObjRef(deref_returnType)):
                is_pointer = 1
            
        size_calc.out(sizeCalculation(environment, returnType,
                                      None, "msgsize", result_string, 1,
                                      is_pointer = is_pointer))
        if is_pointer:
            result_string = "*" + result_string
        marshall(marshal_out, environment, returnType,
                 None, result_string, "giop_s")
        

    # 3rd pass, add the rest (this includes handling the return value)
    if 0 and has_return_value:
                item_types.append(returnType)
                item_names.append("result")
                item_directions.append(1)

    zipped_up = util.zip(item_types, util.zip(item_names, item_directions))
    
    for (item_type, (item_name, item_direction)) in zipped_up:
        deref_item_type = tyutil.deref(item_type)
        item_type_name = environment.principalID(item_type)
        item_dims = tyutil.typeDims(item_type)
        is_array = item_dims != []

        if is_array:
            unmarshal_in.out("""\
@item_type_name@ @item_name@;""", item_type_name = item_type_name,
                             item_name = item_name)
            if item_direction == 0 or item_direction == 2:
                unmarshall(unmarshal_in, environment, item_type, None,
                           item_name, 1, "giop_s")
            if item_direction == 1 or item_direction == 2:
                marshall(marshal_out, environment, item_type, None,
                         item_name, "giop_s")

        is_pointer = 0
        if tyutil.isVariableType(deref_item_type) and \
           not(tyutil.isString(deref_item_type)):
            item_name = "(" + item_name + ".operator->())"
            is_pointer = 1
        if tyutil.isSequence(deref_item_type) and \
           item_direction == 1:
            is_pointer = 1
        

        # SWITCH(deref_item_type)
        #  CASE(string)
        elif tyutil.isString(deref_item_type):
            unmarshal_in.out("""\
CORBA::String_var @item_name@;""", item_name = item_name)
            if item_direction == 0 or item_direction == 2:
                # either in or inout
                unmarshal_in.out("""\
{
  CORBA::String_member _0RL_str_tmp;
  _0RL_str_tmp <<= giop_s;
  @item_name@ = _0RL_str_tmp._ptr;
  _0RL_str_tmp._ptr = 0;
}""", item_name = item_name)
            if item_direction == 1 or item_direction == 2:
                # either out or inout
                marshall(marshal_out, environment, item_type,
                                None, item_name, "giop_s")

        #  CASE(objref)
        elif tyutil.isObjRef(deref_item_type):
            unmarshal_in.out("""\
@item_type@_var @item_name@;""", item_type = item_type_name,
                             item_name = item_name)
            if item_direction == 0 or item_direction == 2:
                # in or inout
                unmarshall(unmarshal_in, environment,
                                  item_type, None,
                                  item_name, 0, "giop_s")
            if item_direction == 1:
                # out
                marshal_out.out("""\
@item_type@_Helper::marshalObjRef((@item_name@.operator->()),giop_s);""",
                                item_type = item_type_name,
                                item_name = item_name)
            if item_direction == 2:
                # inout
                marshal_out.out("""\
@item_type@_Helper::marshalObjRef(@item_name@,giop_s);""",
                                item_type = item_type_name,
                                item_name = item_name)
        #  CASE(sequence)
        elif tyutil.isSequence(deref_item_type):
            if item_direction == 1:
                unmarshal_in.out("""\
@item_type@_var @item_name@;""", item_type = item_type_name,
                                 item_name = item_name)
            else:
                unmarshal_in.out("@type@ @item_name@;",
                                 type = item_type_name,
                                 item_name = item_name)
            if item_direction == 0 or item_direction == 2:
                unmarshal_in.out("@item_name@ <<= giop_s;",
                                 item_name = item_name)
            if item_direction == 1:
                marshal_out.out("*(@item_name@.operator->()) >>= giop_s;",
                                item_name = item_name)
            elif item_direction == 2:
                marshal_out.out("@item_name@ >>= giop_s;",
                                item_name = item_name)
                
            
                            
        #  DEFAULT
        else:
            if tyutil.isVariableType(item_type) and \
               item_direction == 1:
                unmarshal_in.out("""\
// confusion reigns
@type@_var @item_name@;""", type = item_type_name,
                                 item_name = item_name)
            else:
                # this bit is probably too simplistic
                unmarshal_in.out("@type@ @item_name@;",
                                 type = item_type_name,
                                 item_name = item_name)
            if item_direction == 0 or item_direction == 2:
                unmarshal_in.out("@item_name@ <<= giop_s;",
                                 item_name = item_name)
            if item_direction == 1 or item_direction == 2:
                marshal_out.out("@item_name@ >>= giop_s;",
                                item_name = item_name)


                        
        # if parameter is out or inout, alignment matters
        if item_direction == 1 or item_direction == 2:
            #if (tyutil.isObjRef(deref_item_type) or \
            #    tyutil.isSequence(deref_item_type))and \
            #   item_direction == 1:
            #    # out only
            #    item_name = "(" + item_name + ".operator->())"

            calc = sizeCalculation(environment, item_type,
                                   None,
                                   "msgsize",
                                   item_name, 1,
                                   is_pointer = is_pointer)
            size_calc.out(calc)

    return_type_dec = ""
    return_string = ""
    if has_return_value:
        return_type_dec = return_type_name + " result;"
        return_string = "result = "

    stream.out("""\
  if( !strcmp(giop_s.operation(), \"@idname@\") ) {
    @unmarshal_in_arguments@
    giop_s.RequestReceived();
    @return_type_dec@
    @return_assign@this->@operation_name@(@argument_list@);
    if( giop_s.response_expected() ) {
      size_t msgsize = (size_t) GIOP_S::ReplyHeaderSize();
      @size_calculation@
      giop_s.InitialiseReply(GIOP::NO_EXCEPTION, (CORBA::ULong) msgsize);
      @marshal_out_arguments@
    }
    giop_s.ReplyCompleted();
    return 1;
  }""",
                   idname = id_name,
                   unmarshal_in_arguments = str(unmarshal_in),
                   operation_name = id_name,
                   marshal_out_arguments = str(marshal_out),
                   argument_list = string.join(argument_list, ", "),
                   size_calculation = str(size_calc),
                   return_type_dec = return_type_dec,
                   return_assign = return_string)

    

def attribute_read_dispatch(attribute, environment, id, stream):

    attrType = attribute.attrType()
    deref_attrType = tyutil.deref(attrType)
    attrib_type_name = environment.principalID(attrType)
    result_name = "result"
    attr_dims = tyutil.typeDims(attrType)
    is_array = attr_dims != []

    is_pointer = 0


    if tyutil.isSequence(deref_attrType):
        is_pointer = 1

    # similar code exists in skel/main.py, handling pd_result
    # basic types don't have slices
    if is_array:
        if tyutil.ttsMap.has_key(deref_attrType.kind()):
            result_name = "result"
        else:
            result_name = "((" + attrib_type_name + "_slice*)" + "result)"
            
        attrib_type_name = attrib_type_name + "_var"
        
    elif tyutil.isString(deref_attrType):
        attrib_type_name = "CORBA::String_var"

    elif tyutil.isVariableType(attrType):
        if tyutil.isObjRef(deref_attrType):
            attrib_type_name = environment.principalID(deref_attrType)
        
        attrib_type_name = attrib_type_name + "_var"
        result_name = "(result.operator->())"
        if tyutil.isStruct(deref_attrType) or \
           tyutil.isUnion(deref_attrType):
            is_pointer = 1

    # ---

    size_calc = sizeCalculation(environment, attrType,
                                None, "msgsize", result_name, 1, is_pointer)
    marshal = util.StringStream()

    if is_pointer:
        result_name = "*" + result_name
        
    marshall(marshal, environment, attrType, None, result_name, "giop_s")
    
    stream.out("""\
if( !strcmp(giop_s.operation(), \"_get_@attrib_name@\") ) {    
  giop_s.RequestReceived();
  @attrib_type@ result = this->@attrib_name@();
  if( giop_s.response_expected() ) {
    size_t msgsize = (size_t) GIOP_S::ReplyHeaderSize();
    @size_calculation@
    giop_s.InitialiseReply(GIOP::NO_EXCEPTION, (CORBA::ULong) msgsize);
    @marshall_result@
  }
  giop_s.ReplyCompleted();
  return 1;
}""",
               marshall_result = str(marshal),
               attrib_type = attrib_type_name,
               attrib_name = id,
               size_calculation = size_calc)
    
def attribute_write_dispatch(attribute, environment, id, stream):

    attrType = attribute.attrType()
    deref_attrType = tyutil.deref(attrType)
    attrib_type_name = environment.principalID(attrType)
    attr_dims = tyutil.typeDims(attrType)
    is_array = attr_dims != []
    
    is_pointer = 0

    if is_array:
        if tyutil.ttsMap.has_key(deref_attrType.kind()):
            pass
        else:
            attrib_type_name = attrib_type_name 
        
    elif tyutil.isString(deref_attrType):
        attrib_type_name = "CORBA::String_var"

    elif tyutil.isObjRef(deref_attrType):
        attrib_type_name = environment.principalID(deref_attrType)
        attrib_type_name = attrib_type_name + "_var"

    elif tyutil.isSequence(deref_attrType):
        # not a _var type
        is_pointer = 1    


    unmarshal = util.StringStream()

    if not(is_array) and tyutil.isString(deref_attrType):
        unmarshal.out("""\
{
  CORBA::String_member _0RL_str_tmp;
  _0RL_str_tmp <<= giop_s;
  @item_name@ = _0RL_str_tmp._ptr;
  _0RL_str_tmp._ptr = 0;
}""", item_name = "value")
    else:
        unmarshall(unmarshal, environment, attrType, None, "value",
                   1, "giop_s")
    
    stream.out("""\
if( !strcmp(giop_s.operation(), \"_set_@attrib_name@\") ) {
  @attrib_type@ value;
  @unmarshall_value@
  giop_s.RequestReceived();
  this->@attrib_name@(value);
  if( giop_s.response_expected() ) {
    size_t msgsize = (size_t) GIOP_S::ReplyHeaderSize();
    giop_s.InitialiseReply(GIOP::NO_EXCEPTION, (CORBA::ULong) msgsize);
  }
  giop_s.ReplyCompleted();
  return 1;
}""",
               unmarshall_value = str(unmarshal),
               attrib_type = attrib_type_name,
               attrib_name = id)


    
    
