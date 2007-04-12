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
# Revision 1.20.2.10  2007/04/12 19:50:32  dgrisby
# A few cases of sizeof(bool) > 1 were not handled correctly.
#
# Revision 1.20.2.9  2006/04/28 18:40:46  dgrisby
# Merge from omni4_0_develop.
#
# Revision 1.20.2.8  2005/11/09 12:22:17  dgrisby
# Local interfaces support.
#
# Revision 1.20.2.7  2005/08/16 13:51:21  dgrisby
# Problems with valuetype / abstract interface C++ mapping.
#
# Revision 1.20.2.6  2005/01/06 23:09:50  dgrisby
# Big merge from omni4_0_develop.
#
# Revision 1.20.2.5  2004/10/13 17:58:22  dgrisby
# Abstract interfaces support; values support interfaces; value bug fixes.
#
# Revision 1.20.2.4  2004/02/16 10:10:30  dgrisby
# More valuetype, including value boxes. C++ mapping updates.
#
# Revision 1.20.2.3  2003/10/23 11:25:54  dgrisby
# More valuetype support.
#
# Revision 1.20.2.2  2003/05/20 16:53:14  dgrisby
# Valuetype marshalling support.
#
# Revision 1.20.2.1  2003/03/23 21:02:41  dgrisby
# Start of omniORB 4.1.x development branch.
#
# Revision 1.17.2.9  2001/06/19 16:41:49  sll
# Type cast now correctly distinguishes between normal and array types.
#
# Revision 1.17.2.8  2001/06/19 14:23:21  sll
# In the marshalling and unmarshalling code, only cast from a sequence T_var to
# T&. Otherwise, the generate code is wrong with gcc 3.0. Suppose T is a
# sequence of A, even though T is always a derived class of the template
# instance seq<A>, casting a T_var to a seq<A>& causes GCC 3.0 to generate
# wrong code quietly!
#
# Revision 1.17.2.7  2001/06/15 10:22:09  sll
# Work around for MSVC++ bug. Changed the casting of the array of base types
# when they are marshalled using the quick method.
#
# Revision 1.17.2.6  2001/06/08 17:12:13  dpg1
# Merge all the bug fixes from omni3_develop.
#
# Revision 1.17.2.5  2001/04/19 10:04:13  dpg1
# Bug in sort_exceptions()
#
# Revision 1.17.2.4  2000/11/07 18:28:21  sll
# Use helper marshal functions if the interface is a forward declaration.
#
# Revision 1.17.2.3  2000/11/03 19:26:01  sll
# Simplified the marshalling functions.
#
# Revision 1.17.2.2  2000/10/12 15:37:48  sll
# Updated from omni3_1_develop.
#
# Revision 1.18.2.2  2000/08/21 11:34:35  djs
# Lots of omniidl/C++ backend changes
#
# Revision 1.18.2.1  2000/08/04 17:10:26  dpg1
# Long long support
#
# Revision 1.18  2000/07/13 15:26:01  dpg1
# Merge from omni3_develop for 3.0 release.
#
# Revision 1.15.2.4  2000/05/31 18:02:16  djs
# Better output indenting (and preprocessor directives now correctly output at
# the beginning of lines)
#
# Revision 1.15.2.3  2000/04/26 18:22:13  djs
# Rewrote type mapping code (now in types.py)
# Rewrote identifier handling code (now in id.py)
#
# Revision 1.15.2.2  2000/03/09 15:21:40  djs
# Better handling of internal compiler exceptions (eg attempts to use
# wide string types)
#
# Revision 1.15.2.1  2000/02/14 18:34:57  dpg1
# New omniidl merged in.
#
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

from omniidl_be.cxx import util, types, id, ast, output, cxx

# From http://www-i3.informatik.rwth-aachen.de/funny/babbage.html:
#
# A hotly contested issue among language designers is the method for
# passing parameters to subfunctions. Some advocate "call by name," others
# prefer "call by value." Babbage uses a new method - "call by telephone."
# This is especially effective for long-distance parameter passing.

# Code for marshalling and unmarshalling various data types.

def marshall(to, environment, type, decl, argname, to_where,
             exception = "BAD_PARAM"):
    assert isinstance(type, types.Type)

    d_type = type.deref()

    # If this is an array of base types, the quick marshalling option is used.
    # slice_cast is set to the casting required to cast the variable to a
    # pointer to the beginning of the array. There are 3 possibilities:
    #  1. The variable is a declarator, e.g. it is a member of a struct or a
    #     union. No casting is required. slice_cast = ""
    #  2. The variable is not a declarator and its dimension is > 1,
    #     cast the variable to its array slice pointer.
    #  3. Same as 2 but its dimension == 1, just cast the variable to a pointer
    #     to its base class (because the type's array slice may not be
    #     defined).

    if decl:
        assert isinstance(decl, idlast.Declarator)
        dims = decl.sizes() + type.dims()
        slice_cast = ""
    else:
        dims = type.dims()
        if len(dims) != 1:
            slice_cast = "(" + type.base(environment) + "_slice" + "*)"
        else:
            slice_cast = "(" + d_type.base(environment) + "*)"

    if dims != []:
        n_elements = reduce(lambda x,y:x*y, dims, 1)
        array_marshal_helpers = {
          idltype.tk_octet:     ("omni::ALIGN_1",1),
          idltype.tk_boolean:   ("omni::ALIGN_1",1),
          idltype.tk_short:     ("omni::ALIGN_2",2),
          idltype.tk_long:      ("omni::ALIGN_4",4),
          idltype.tk_ushort:    ("omni::ALIGN_2",2),
          idltype.tk_ulong:     ("omni::ALIGN_4",4),
          idltype.tk_float:     ("omni::ALIGN_4",4),
          idltype.tk_double:    ("omni::ALIGN_8",8),
          idltype.tk_longlong:  ("omni::ALIGN_8",8),
          idltype.tk_ulonglong: ("omni::ALIGN_8",8)
        }
        kind = d_type.type().kind()
        if array_marshal_helpers.has_key(kind):
            (alignment,elmsize) = array_marshal_helpers[kind]
            if alignment != "omni::ALIGN_1":
                if kind == idltype.tk_double:
                    to.out("""
#ifndef OMNI_MIXED_ENDIAN_DOUBLE""")

                to.out("""\
if (! @where@.marshal_byte_swap()) {
  @where@.put_octet_array((CORBA::Octet*)(@slice_cast@@name@),@num@,@align@);
}
else """,
                       where = to_where,
                       name = argname,
                       slice_cast = slice_cast,
                       num = str(n_elements * elmsize),
                       align = alignment)

                if kind == idltype.tk_double:
                    to.out("""\
#endif""")

                # Do not return here.
                # let the code below to deal with the else block.
            else:
                if kind == idltype.tk_boolean:
                    to.out("""
#if !defined(HAS_Cplusplus_Bool) || (SIZEOF_BOOL == 1)""")

                to.out("@where@.put_octet_array((CORBA::Octet*)(@slice_cast@@name@),@num@);",
                       where = to_where,
                       name = argname,
                       slice_cast = slice_cast,
                       num = str(n_elements))

                if kind == idltype.tk_boolean:
                    to.out("""\
#else""")
                else:
                    return

        # No quick route, generate iteration loop
        block = cxx.Block(to)
        akind = d_type.type().kind()

        # Valuetype chunked encoding needs to know array length before
        # we marshal it item by item.
        if array_marshal_helpers.has_key(akind):
            to.out("""\
@where@.declareArrayLength(@align@, @num@);""",
                   where = to_where,
                   num = str(n_elements * elmsize),
                   align = alignment)

        elif akind == idltype.tk_char:
            to.out("""\
@where@.declareArrayLength(omni::ALIGN_1, @num@);""",
                   where = to_where,
                   num = str(n_elements))

        elif akind == idltype.tk_longdouble:
            to.out("""\
@where@.declareArrayLength(omni::ALIGN_8, @num@);""",
                   where = to_where,
                   num = str(n_elements * 16))

    loop = cxx.For(to, dims)
    indexing_string = loop.index()
    element_name = argname + indexing_string

    if dims != []:
        type_name = d_type.base(environment)
        if type_name == element_name:
            type_name = d_type.base()
    else:
        type_name = type.base(environment)
        if type_name == element_name:
            type_name = type.base()
            
    bounded = ""
    kind = d_type.type().kind()
    
    if d_type.interface():
        type_name = string.replace(type_name,"_ptr","")
        if isinstance(d_type.type().decl(),idlast.Forward):
            # hack to denote an interface forward declaration
            # kind is used to index the associative array below
            kind = idltype.tk_objref * 1000

    elif d_type.value() or d_type.valuebox():
        if isinstance(d_type.type().decl(),idlast.ValueForward):
            kind = idltype.tk_value * 1000

    elif d_type.string() or d_type.wstring():
        bounded = str(d_type.type().bound())

    if not d_type.is_basic_data_types() and not d_type.enum():
        type_cast = "(const " + type_name + "&) "
    else:
        type_cast = ""

    special_marshal_functions = {
      idltype.tk_boolean:
      "@to_where@.marshalBoolean(@element_name@);",
      idltype.tk_octet:
      "@to_where@.marshalOctet(@element_name@);",
      idltype.tk_char:
      "@to_where@.marshalChar(@element_name@);",
      idltype.tk_wchar:
      "@to_where@.marshalWChar(@element_name@);",
      idltype.tk_string:
      "@to_where@.marshalString(@element_name@,@bounded@);",
      idltype.tk_wstring:
      "@to_where@.marshalWString(@element_name@,@bounded@);",
      idltype.tk_objref:
      "@type@::_marshalObjRef(@element_name@,@to_where@);",
      idltype.tk_TypeCode:
      "CORBA::TypeCode::marshalTypeCode(@element_name@,@to_where@);",
      idltype.tk_objref * 1000:
      "@type@_Helper::marshalObjRef(@element_name@,@to_where@);",
      idltype.tk_value:
      "@type@::_NP_marshal(@element_name@,@to_where@);",
      idltype.tk_value * 1000:
      "@type@_Helper::marshal(@element_name@,@to_where@);",
      idltype.tk_value_box:
      "@type@::_NP_marshal(@element_name@,@to_where@);",
      idltype.tk_abstract_interface:
      "@type@::_marshalObjRef(@element_name@,@to_where@);",
      idltype.tk_local_interface:
      "@type@::_marshalObjRef(@element_name@,@to_where@);",
      }
    if special_marshal_functions.has_key(kind):
        out_template = special_marshal_functions[kind]
    else:
        out_template = "@type_cast@@element_name@ >>= @to_where@;"

    to.out(out_template,
           to_where = to_where,
           element_name = element_name,
           bounded = bounded,
           type = type_name,
           type_cast = type_cast)
    loop.end()

    if dims != []:
        block.end()
        if kind == idltype.tk_boolean:
            to.out("""\
#endif""")


        
def unmarshall(to, environment, type, decl, name, from_where):
    assert isinstance(type, types.Type)

    d_type = type.deref()

    # If this is an array of base types, the quick marshalling option is used.
    # slice_cast is set to the casting required to cast the variable to a
    # pointer to the beginning of the array. There are 3 possibilities:
    #  1. The variable is a declarator, e.g. it is a member of a struct or a
    #     union. No casting is required. slice_cast = ""
    #  2. The variable is not a declarator and its dimension is > 1,
    #     cast the variable to its array slice pointer.
    #  3. Same as 2 but its dimension == 1, just cast the variable to a pointer
    #     to its base class (because the type's array slice may not be
    #     defined).

    if decl:
        assert isinstance(decl, idlast.Declarator)
        dims = decl.sizes() + type.dims()
        slice_cast = ""
    else:
        dims = type.dims()
        if len(dims) != 1:
            slice_cast = "(" + type.base(environment) + "_slice" + "*)"
        else:
            slice_cast = "(" + d_type.base(environment) + "*)"

    if dims != []:

        n_elements = reduce(lambda x,y:x*y, dims, 1)
        array_unmarshal_helpers = {
          idltype.tk_octet:  ("get_octet_array","(CORBA::Octet*)"),
          idltype.tk_boolean:("unmarshalArrayBoolean","(CORBA::Boolean*)"),
          idltype.tk_short:  ("unmarshalArrayShort","(CORBA::Short*)"),
          idltype.tk_long:   ("unmarshalArrayLong","(CORBA::Long*)"),
          idltype.tk_ushort: ("unmarshalArrayUShort","(CORBA::UShort*)"),
          idltype.tk_ulong:  ("unmarshalArrayULong","(CORBA::ULong*)"),
          idltype.tk_float:  ("unmarshalArrayFloat","(CORBA::Float*)"),
          idltype.tk_double: ("unmarshalArrayDouble","(CORBA::Double*)"),
          idltype.tk_longlong:("unmarshalArrayLongLong","(CORBA::LongLong*)"),
          idltype.tk_ulonglong:("unmarshalArrayULongLong","(CORBA::ULongLong*)")
          }
        kind = d_type.type().kind()

        if array_unmarshal_helpers.has_key(kind):
            (helper,typecast) = array_unmarshal_helpers[kind]
            to.out("@where@.@helper@(@typecast@(@slice_cast@@name@), @num@);",
                   helper = helper,
                   where = from_where, typecast = typecast,
                   name = name,
                   slice_cast = slice_cast,
                   num = str(n_elements))
            return

        # No quick route, generate iteration loop
        block = cxx.Block(to)

    loop = cxx.For(to, dims)
    indexing_string = loop.index()
    element_name = name + indexing_string
    
    if dims != []:
        type_name = d_type.base(environment)
        if type_name == element_name:
            type_name = d_type.base()
    else:
        type_name = type.base(environment)
        if type_name == element_name:
            type_name = type.base()
        
    bounded = ""
    kind = d_type.type().kind()

    if d_type.interface():
        type_name = string.replace(type_name,"_ptr","")
        if isinstance(d_type.type().decl(),idlast.Forward):
            # hack to denote an interface forward declaration
            # kind is used to index the associative array below
            kind = idltype.tk_objref * 1000

    elif d_type.value() or d_type.valuebox():
        if isinstance(d_type.type().decl(),idlast.ValueForward):
            kind = idltype.tk_value * 1000

    elif d_type.string() or d_type.wstring():
        bounded = str(d_type.type().bound())
        
    special_unmarshal_functions = {
      idltype.tk_boolean:
      "@element_name@ = @where@.unmarshalBoolean();",
      idltype.tk_octet:
      "@element_name@ = @where@.unmarshalOctet();",
      idltype.tk_char:
      "@element_name@ = @where@.unmarshalChar();",
      idltype.tk_wchar:
      "@element_name@ = @where@.unmarshalWChar();",
      idltype.tk_string:
      "@element_name@ = @where@.unmarshalString(@bounded@);",
      idltype.tk_wstring:
      "@element_name@ = @where@.unmarshalWString(@bounded@);",
      idltype.tk_objref:
      "@element_name@ = @type@::_unmarshalObjRef(@where@);",
      idltype.tk_TypeCode:
      "@element_name@ = CORBA::TypeCode::unmarshalTypeCode(@where@);",
      idltype.tk_objref * 1000:
      "@element_name@ = @type@_Helper::unmarshalObjRef(@where@);",
      idltype.tk_value:
      "@element_name@ = @type@::_NP_unmarshal(@where@);",
      idltype.tk_value * 1000:
      "@element_name@ = @type@_Helper::unmarshal(@where@);",
      idltype.tk_value_box:
      "@element_name@ = @type@::_NP_unmarshal(@where@);",
      idltype.tk_abstract_interface:
      "@element_name@ = @type@::_unmarshalObjRef(@where@);",
      idltype.tk_local_interface:
      "@element_name@ = @type@::_unmarshalObjRef(@where@);",
      }

    if special_unmarshal_functions.has_key(kind):
        out_template = special_unmarshal_functions[kind]
    else:
        out_template = "(@type@&)@element_name@ <<= @where@;"

    to.out(out_template,
           type = type_name,
           element_name = element_name,
           where = from_where,
           bounded = bounded)

    loop.end()

    if dims != []:
        block.end()


def sort_exceptions(ex):
    # sort the exceptions into lexicographical order
    def lexicographic(exception_a, exception_b):
        name_a = exception_a.repoId()
        name_b = exception_b.repoId()
        return cmp(name_a, name_b)
        
    raises = ex[:]
    raises.sort(lexicographic)
    return raises
