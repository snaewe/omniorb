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
        

def marshall(string, environment, type, decl, argname):
    assert isinstance(type, idltype.Type)
    assert isinstance(decl, idlast.Declarator)
    
    deref_type = tyutil.deref(type)

    dims = decl.sizes()
    type_dims = tyutil.typeDims(type)
    full_dims = dims + type_dims

    anonymous_array = dims      != []
    is_array        = full_dims != []
    alias_array     = type_dims != []

    num_elements = reduce(lambda x,y:x*y, full_dims, 1)

    type_name = environment.principalID(deref_type)

    if not(is_array):
        string.out("""\
@argname@ >>= _n;""", argname = argname)
        return

    # marshall an array of basic things
    # (octet separate because it doesnt have an entry in typeSizeAlignMap
    # - changing this might affect the behaviour of sequenceTemplate)
    if tyutil.isOctet(deref_type):
        num_bytes = 1 * num_elements
        string.out("""\
_n.put_char_array((const _CORBA_Char*) ((const @type@*) @argname@), @size@);""",
                   type = type_name, argname = argname,
                   size = str(num_bytes))
        return
    
    if tyutil.typeSizeAlignMap.has_key(deref_type.kind()):
        (size, align) = tyutil.typeSizeAlignMap[deref_type.kind()]
        num_bytes = size * num_elements
        if align != 1:
            align_str = ", omni::ALIGN_" + str(align)
        else:
            align_str = ""
        
        string.out("""\
_n.put_char_array((const _CORBA_Char*) ((const @type@*) @argname@), @size@@align@);""",
                   type = type_name, argname = argname,
                   size = str(num_bytes), align = align_str)
        return

    # marshall an array of strings
    if tyutil.isString(deref_type):

        indexing_string = begin_loop(string, full_dims)

        string.out("""\
    CORBA::ULong _len = (((const char*) @argname@@indexing_string@)? strlen((const char*) @argname@@indexing_string@) + 1 : 1);
    _len >>= _n;
    if (_len > 1)
      _n.put_char_array((const CORBA::Char *)((const char*)@argname@@indexing_string@),_len);
    else {
      if ((const char*) @argname@@indexing_string@ == 0 && omniORB::traceLevel > 1)
        _CORBA_null_string_ptr(0);
        
      CORBA::Char('\\0') >>= _n;
    }""",
                   argname = argname, indexing_string = indexing_string)

        end_loop(string, full_dims)

        return

    # marshall an array of object references
    if tyutil.isObjRef(deref_type):
        indexing_string = begin_loop(string, full_dims)

        string.out("""\
    @type_name@_Helper::marshalObjRef(@argname@@indexing_string@._ptr,_n);""",
                   type_name = type_name,
                   argname = argname,
                   indexing_string = indexing_string)
        
        end_loop(string, full_dims)

        return

    # marshall struct and union types
    indexing_string = begin_loop(string, full_dims)

    string.out("""\
    @argname@@indexing_string@ >>= _n;""",
               argname = argname, indexing_string = indexing_string)
    
    end_loop(string, full_dims)
    
    return

def unmarshall(string, environment, type, decl, argname,
               can_throw_marshall):
    assert isinstance(type, idltype.Type)
    assert isinstance(decl, idlast.Declarator)
    
    deref_type = tyutil.deref(type)

    dims = decl.sizes()
    type_dims = tyutil.typeDims(type)
    full_dims = dims + type_dims

    anonymous_array = dims      != []
    is_array        = full_dims != []
    alias_array     = type_dims != []

    num_elements = reduce(lambda x,y:x*y, full_dims, 1)

    type_name = environment.principalID(deref_type)
    
    if not(is_array):
        string.out("""\
@argname@ <<= _n;""", argname = argname)
        return

    # unmarshall an array of basic things
    #   octet and chars are handled directly
    if tyutil.isOctet(deref_type) or tyutil.isChar(deref_type):
        string.out("""\
_n.get_char_array((_CORBA_Char*) ((@type@*) @argname@), @num@);""",
                   argname = argname, type = type_name,
                   num = str(num_elements))
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
CdrStreamHelper_unmarshalArray@suffix@(_n, ((@type@*) @argname@), @num@);""",
                       suffix = array_helper_suffix[deref_type.kind()],
                       type = type_name,
                       argname = argname,
                       num = str(num_elements))
            return
        # basic type (boolean, any, tc, princ, ll, ull, ld, wc)
        raise "Don't know how to marshall type: " + basic_type + " array"

    indexing_string = begin_loop(string, full_dims)
    
    if tyutil.isString(deref_type):
        string.out("""\
  CORBA::ULong _len;
  _len <<= _n;
  if (!_len) {
    if (omniORB::traceLevel > 1)
      _CORBA_null_string_ptr(1);
    _len = 1;
  }""", name = argname, indexing_string = indexing_string)
        if can_throw_marshall:
            string.out("""\
  else if ( _n.RdMessageUnRead() < _len)
    throw CORBA::MARSHAL(0,CORBA::COMPLETED_NO);""")
            string.dec_indent()
        string.out("""\
  if (!(char*)(@name@@indexing_string@ = CORBA::string_alloc(_len-1)))
    throw CORBA::NO_MEMORY(0,CORBA::COMPLETED_NO);
  if (_len > 1)
    _n.get_char_array((CORBA::Char *)((char *)@name@@indexing_string@),_len);
  else
    *((CORBA::Char*)((char*) @name@@indexing_string@)) <<= _n ;""",
                   name = argname, indexing_string = indexing_string)
        string.dec_indent()
    elif tyutil.isObjRef(deref_type):
        string.out("""\
  @name@@indexing_string@ = @type@_Helper::unmarshalObjRef(_n);""",
                   type = type_name,
                   name = argname,
                   indexing_string = indexing_string)

    else:
        string.out("""\
  @name@@indexing_string@ <<= _n;""",
                   name = argname, indexing_string = indexing_string)

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
    

