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
# Revision 1.2  1999/11/17 20:37:09  djs
# General util functions
#
# Revision 1.1  1999/11/15 19:10:55  djs
# Added module for utility functions specific to generating skeletons
# Union skeletons working
#

from omniidl import idlutil, idltype, idlast

from omniidl.be.cxx import util, tyutil



def begin_loop(string, full_dims):
    string.out("{")
    string.inc_indent()
    index = 0
    indexing_string = ""
    for dim in full_dims:
        string.out("""\
for (CORBA::ULong _i@n@ = 0;_i@n@ < @dim@;_i@n@++) {""",
                   n = str(index), dim = str(dim))
        string.inc_indent()
        indexing_string = indexing_string + "[_i" + str(index) + "]"
        index = index + 1

    return indexing_string

def end_loop(string, full_dims):
    for dim in full_dims:
        string.dec_indent()
        string.out("}")
            
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
        

def marshall(string, environment, type, decl, argname, to="_n"):
    assert isinstance(type, idltype.Type)
    if decl:
        assert isinstance(decl, idlast.Declarator)
        dims = decl.sizes()
    else:
        dims = []
        
    deref_type = tyutil.deref(type)

    type_dims = tyutil.typeDims(type)
    full_dims = dims + type_dims

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
@to@.put_char_array((const _CORBA_Char*) ((const @type@*) @argname@), @size@@align@);""",
                           type = type_name, argname = argname,
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
        indexing_string = begin_loop(string, full_dims)
        string.out("""\
    @type_name@_Helper::marshalObjRef(@argname@@indexing_string@._ptr,@to@);""",
                   type_name = type_name,
                   argname = argname,
                   indexing_string = indexing_string,
                   to = to)
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
        unmarshall(string, environment, type, decl, argname, from_where)


def unmarshall(string, environment, type, decl, argname,
               can_throw_marshall, from_where = "_n"):
    assert isinstance(type, idltype.Type)
    if decl:
        assert isinstance(decl, idlast.Declarator)
        dims = decl.sizes()
    else:
        dims = []
    
    deref_type = tyutil.deref(type)

    type_dims = tyutil.typeDims(type)
    full_dims = dims + type_dims

    anonymous_array = dims      != []
    is_array        = full_dims != []
    alias_array     = type_dims != []

    num_elements = reduce(lambda x,y:x*y, full_dims, 1)

    type_name = environment.principalID(deref_type)
    
    if not(is_array):
        string.out("""\
@argname@ <<= @from_where@;""", argname = argname, from_where = from_where)
        return

    # unmarshall an array of basic things
    #   octet and chars are handled directly
    if tyutil.isOctet(deref_type) or tyutil.isChar(deref_type):
        string.out("""\
@from_where@.get_char_array((_CORBA_Char*) ((@type@*) @argname@), @num@);""",
                   argname = argname, type = type_name,
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
    if isinstance(deref_type, idltype.Base):
        if array_helper_suffix.has_key(deref_type.kind()):
            string.out("""\
CdrStreamHelper_unmarshalArray@suffix@(@from_where@, ((@type@*) @argname@), @num@);""",
                       suffix = array_helper_suffix[deref_type.kind()],
                       type = type_name,
                       argname = argname,
                       num = str(num_elements),
                       from_where = from_where)
            return
        # basic type (boolean, any, tc, princ, ll, ull, ld, wc)
        raise "Don't know how to marshall type: " + basic_type + " array"

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

    end_loop(string, full_dims)

    
# ------------------------------------------------------------------

def sizeCalculation(environment, type, decl, sizevar, argname):
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
        if 0:
            if tyutil.isString(deref_type):
                string.out("""\
@sizevar@ = omni::align_to(@sizevar, omni::ALIGN_4) + 4;
@sizevar@ += ((const char*) @argname@) + 1 : 1;""",
                           sizevar = sizevar, argname = argname)
                return str(string)

        # FIXME:
        # this corresponds to case tObjref in the old BE
        # what is the difference between tObjrefMember and tObjref?
        if 0:
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
@sizevar@ = @argname@._NP_alignedSize(@sizevar@);""",
                   sizevar = sizevar, argname = argname)
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
  @sizevar@ = @argname@@indexing_string@._NP_alignedSize(@sizevar@);""",
                   sizevar = sizevar, argname = argname,
                   indexing_string = indexing_string)

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
        name = environment.principalID(deref_type)
        string.out("""\
@sizevar@ = @name@_Helper::NP_alignedSize(@argname@@indexing_string@._ptr,@sizevar@);""",
                   sizevar = sizevar, name = name, argname = argname,
                   indexing_string = indexing_string)

    else:
        # typecodes may be an exception here
        string.out("""\
@sizevar@ = @argname@@indexing_string@._NP_alignedSize(@sizevar@);""",
                   sizevar = sizevar, argname = argname,
                   indexing_string = indexing_string)
               

    end_loop(string, full_dims)
#    print "[[[ " + str(string) + "]]]"

    return str(string)
    

