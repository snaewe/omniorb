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
# Revision 1.15  2000/01/17 16:58:52  djs
# Marshalling code: exceptions (BAD_PARAM, MARSHAL) and bounded strings
#
# Revision 1.14  2000/01/11 12:02:34  djs
# More tidying up
#
# Revision 1.13  2000/01/07 20:31:18  djs
# Regression tests in CVSROOT/testsuite now pass for
#   * no backend arguments
#   * tie templates
#   * flattened tie templates
#   * TypeCode and Any generation
#
# Revision 1.12  1999/12/26 16:38:06  djs
# Support for bounded strings (specifically a bounds check raising
# CORBA::BAD_PARAM)
#
# Revision 1.11  1999/12/16 16:08:02  djs
# More TypeCode and Any fixes
#
# Revision 1.10  1999/12/15 12:11:54  djs
# Marshalling arrays of Anys fix
#
# Revision 1.9  1999/12/14 11:53:56  djs
# Support for CORBA::TypeCode and CORBA::Any
#
# Revision 1.8  1999/12/10 18:26:03  djs
# Added a utility function to order exceptions based on their names
#
# Revision 1.7  1999/11/29 19:26:59  djs
# Code tidied and moved around. Some redundant code eliminated.
#
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

# From http://www-i3.informatik.rwth-aachen.de/funny/babbage.html:
#
# A hotly contested issue among language designers is the method for
# passing parameters to subfunctions. Some advocate "call by name," others
# prefer "call by value." Babbage uses a new method - "call by telephone."
# This is especially effective for long-distance parameter passing.

# Code for marshalling and unmarshalling various data types.

# marshalling for struct, union and exception types
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
        marshall(string, environment, type, decl, argname, to,
                 exception = "MARSHAL")
        

def marshall(string, environment, type, decl, argname, to="_n",
             exception = "BAD_PARAM"):
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
    # becomes put_char_array(.... x[0][0]...)
    zero_dims_string = "[0]" * (len(full_dims) - 1)

    anonymous_array = dims      != []
    is_array        = full_dims != []
    alias_array     = type_dims != []

    num_elements = reduce(lambda x,y:x*y, full_dims, 1)

    type_name = environment.principalID(deref_type)

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
                       dims_string = zero_dims_string,
                       size = str(num_bytes), align = align_str, to = to)
            return

    if tyutil.isTypeCode(deref_type):
        # same as obj ref
        indexing_string = ""
        if is_array:
            indexing_string = util.block_begin_loop(string, full_dims) +\
                              "._ptr"
        string.out("""\
CORBA::TypeCode::marshalTypeCode(@argname@@indexing_string@, @to@);""",
                   argname = argname, to = to,
                   indexing_string = indexing_string)
        if is_array:
            util.block_end_loop(string, full_dims)
       
    elif tyutil.isString(deref_type):
        indexing_string = util.block_begin_loop(string, full_dims)
        bounds = util.StringStream()
        if deref_type.bound() != 0:
            bounds.out("""\
    if (_len > @n@+1) {
      throw CORBA::@exception@(0, CORBA::COMPLETED_MAYBE);
    }""", n = str(deref_type.bound()),
                       exception = exception)

        string.out("""\
    CORBA::ULong _len = (((const char*) @argname@@indexing_string@)? strlen((const char*) @argname@@indexing_string@) + 1 : 1);
    @bound@
    _len >>= @to@;
    if (_len > 1)
      @to@.put_char_array((const CORBA::Char *)((const char*)@argname@@indexing_string@),_len);
    else {
      if ((const char*) @argname@@indexing_string@ == 0 && omniORB::traceLevel > 1)
        _CORBA_null_string_ptr(0);
      CORBA::Char('\\0') >>= @to@;
    }""",
                   bound = str(bounds),
                   argname = argname,
                   indexing_string = indexing_string,
                   to = to)
        util.block_end_loop(string, full_dims)
    elif tyutil.isObjRef(deref_type):
        indexing_string = ""
        if is_array:
            indexing_string = util.block_begin_loop(string, full_dims) +\
                              "._ptr"
        string.out("""\
    @type_name@_Helper::marshalObjRef(@argname@@indexing_string@,@to@);""",
                   type_name = type_name,
                   argname = argname,
                   indexing_string = indexing_string,
                   to = to)
        if is_array:
            util.block_end_loop(string, full_dims)
    else:
        if is_array:
            indexing_string = util.block_begin_loop(string, full_dims)
        else:
            indexing_string = ""
        string.out("""\
    @argname@@indexing_string@ >>= @to@;""",
                   argname = argname, indexing_string = indexing_string,
                   to = to)
        if is_array:
            util.block_end_loop(string, full_dims)
    
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



def unmarshall(to, environment, type, decl, name,
               can_throw_marshall, from_where = "_n",
               string_via_member = 0):
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
    # becomes put_char_array(.... x[0][0]...)
    zero_dims_string = "[0]" * (len(full_dims) - 1)

    anonymous_array = dims      != []
    is_array        = full_dims != []
    alias_array     = type_dims != []

    num_elements = reduce(lambda x,y:x*y, full_dims, 1)

    type_name = environment.principalID(deref_type)
    deref_type_name = environment.principalID(deref_type)

    element_name = name + zero_dims_string

    if is_array:
        # BASIC things
        # octets, chars and booleans are handled via
        # get_char_array
        if tyutil.isOctet(deref_type) or \
           tyutil.isChar(deref_type)  or \
           tyutil.isBoolean(deref_type):
            to.out("""\
@from_where@.get_char_array((_CORBA_Char*) ((@type@*) @element_name@), @num@);""",
                   element_name = element_name,
                   type = type_name,
                   num = str(num_elements),
                   from_where = from_where)
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
            typecast = "((" + deref_type_name + "*) " + element_name + ")"
            # use the most dereferenced type
            if tyutil.isEnum(deref_type):
                typecast = "(_CORBA_ULong*) " + typecast
            to.out("""\
CdrStreamHelper_unmarshalArray@suffix@(@where@,@typecast@, @num@);""",
                       suffix = array_helper_suffix[deref_type.kind()],
                       where = from_where, typecast = typecast,
                       num = str(num_elements))
            return
        if tyutil.isAny(deref_type) or \
           tyutil.isTypeCode(deref_type):
            pass
        # not sure how to handle other basic types
        elif isinstance(deref_type, idltype.Base):
            raise "Don't know how to marshall type: " + repr(deref_type) +\
              "(kind = " + str(deref_type.kind()) + ") array"


    # superfluous bracketting
    if is_array:
        to.out("{")
        to.inc_indent()

    indexing_string = util.start_loop(to, full_dims)
    element_name = name + indexing_string

    if tyutil.isTypeCode(deref_type):
        to.out("""\
  @element_name@ = CORBA::TypeCode::unmarshalTypeCode(@from_where@);""",
               element_name = element_name,
               from_where = from_where)
    elif tyutil.isString(deref_type):
        if not(is_array) and string_via_member:
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
  _len <<= @from_where@;
  if (!_len) {
    if (omniORB::traceLevel > 1)
      _CORBA_null_string_ptr(1);
    _len = 1;
  }""", from_where = from_where)
            if can_throw_marshall:
                to.out("""\
  else if ( @from_where@.RdMessageUnRead() < _len)
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);""",
                       from_where = from_where)
                to.dec_indent()

            if deref_type.bound() != 0:
                to.out("""\
  if (_len > @n@+1) {
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_MAYBE);
  }""", n = str(deref_type.bound()))

               
            to.out("""\
  if (!(char*)(@element_name@ = CORBA::string_alloc(_len-1)))
    throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
  if (_len > 1)
    @from_where@.get_char_array((CORBA::Char *)((char *)@element_name@),_len);
  else
    *((CORBA::Char*)((char*) @element_name@)) <<= @from_where@ ;""",
                   element_name = element_name,
                   from_where = from_where)
            to.dec_indent()
    elif tyutil.isObjRef(deref_type):
        base_type_name = environment.principalID(deref_type)
        to.out("""\
  @element_name@ = @type@_Helper::unmarshalObjRef(@from_where@);""",
                   type = base_type_name,
                   element_name = element_name,
                   from_where = from_where)

    else:
        to.out("""\
  @element_name@ <<= @from_where@;""",
                   element_name = element_name,
                   from_where = from_where)

    util.finish_loop(to, full_dims)
    
    if is_array:
        to.dec_indent()
        to.out("}")

    
# ------------------------------------------------------------------

# FIXME - parameter is 1 when producing the size calc for an interface
# argument (not a struct member)
# (is this why tObjrefMember != tObjref in the old BE?)
def sizeCalculation(environment, type, decl, sizevar, argname, fixme = 0,
                    is_pointer = 0):
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
                name = environment.principalID(deref_type)
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
        indexing_string = util.block_begin_loop(string, full_dims)

        # do the actual calculation
        string.out("""\
  @sizevar@ = @argname@@indexing_string@@deref@_NP_alignedSize(@sizevar@);""",
                   sizevar = sizevar, argname = argname,
                   indexing_string = indexing_string,
                   deref = dereference)

        util.block_end_loop(string, full_dims)

        return str(string)


    # thing is an array of variable sized elements
    indexing_string = util.block_begin_loop(string, full_dims)

    if tyutil.isString(deref_type):
        string.out("""\
@sizevar@ = omni::align_to(@sizevar@, omni::ALIGN_4);
@sizevar@ += 4 + (((const char*) @argname@@indexing_string@)? strlen((const char*) @argname@@indexing_string@) + 1 : 1);""",
                   sizevar = sizevar, argname = argname,
                   indexing_string = indexing_string)
        

    elif tyutil.isObjRef(deref_type):
        name = environment.principalID(deref_type)
        string.out("""\
@sizevar@ = @name@_Helper::NP_alignedSize(@argname@@indexing_string@._ptr,@sizevar@);""",
                   sizevar = sizevar, name = name, argname = argname,
                   indexing_string = indexing_string)
    elif tyutil.isTypeCode(deref_type):
        string.out("""\
@sizevar@ = ((@argname@@indexing_string@._ptr)->_NP_alignedSize(@sizevar@));""",
                   sizevar = sizevar, argname = argname,
                   indexing_string = indexing_string)

    else:
        # typecodes may be an exception here
        string.out("""\
@sizevar@ = @argname@@indexing_string@@deref@_NP_alignedSize(@sizevar@);""",
                   sizevar = sizevar, argname = argname,
                   indexing_string = indexing_string,
                   deref = dereference)
               

    util.block_end_loop(string, full_dims)
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
    
    


def sort_exceptions(ex):
    # sort the exceptions into lexicographical order
    def lexicographic(exception_a, exception_b):
        # use their full C++ name
        name_a = string.join(tyutil.mapID(exception_a.scopedName()))
        name_b = string.join(tyutil.mapID(exception_b.scopedName()))
        # name_a <=> name_b
        if name_a < name_b: return -1
        if name_a > name_b: return 1
        return 0
        
    raises = ex[:]
    raises.sort(lexicographic)
    return raises
