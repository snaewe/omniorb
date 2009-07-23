# -*- python -*-
#                           Package   : omniidl
# cxx.py                    Created on: 2000/8/10
#			    Author    : David Scott (djs)
#
#    Copyright (C) 2003-2008 Apasphere Ltd
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
#   
#   Routines for generating bits of C++ syntax dynamically

# $Id$
# $Log$
# Revision 1.1.6.5  2008/12/03 10:53:58  dgrisby
# Tweaks leading to Python 3 support; other minor clean-ups.
#
# Revision 1.1.6.4  2007/09/19 14:16:08  dgrisby
# Avoid namespace clashes if IDL defines modules named CORBA.
#
# Revision 1.1.6.3  2005/11/09 12:22:17  dgrisby
# Local interfaces support.
#
# Revision 1.1.6.2  2003/10/23 11:25:54  dgrisby
# More valuetype support.
#
# Revision 1.1.6.1  2003/03/23 21:02:42  dgrisby
# Start of omniORB 4.1.x development branch.
#
# Revision 1.1.4.2  2001/06/08 17:12:12  dpg1
# Merge all the bug fixes from omni3_develop.
#
# Revision 1.1.4.1  2000/10/12 15:37:46  sll
# Updated from omni3_1_develop.
#
# Revision 1.1.2.1  2000/08/21 11:34:33  djs
# Lots of omniidl/C++ backend changes
#

"""Routines for generating bits of C++ syntax dynamically"""

from omniidl_be.cxx import id, types

import sys, re, string

# For: Generates a nested C++ for loop ranging over
#          {0, bounds[0]}{0, bounds[1]}...
# If destroyed before properly terminated, will output a warning on stderr
# (exceptions are apparently unwise in destructors)#
#
# Example usage:
#    loop = For(stream, [1,2,3,4])
#    stream.out("// do something with @index@", index = loop.index())
#    loop.end()
#
class For:
    def __init__(self, stream, bounds):
        prefix = "_0i"
        index = 0
        index_string = ""
        for bound in bounds:
            i = prefix + str(index)
            stream.out("""\
for (_CORBA_ULong @i@ = 0; @i@ < @bound@; @i@++){""", i = i, bound = bound)
            stream.inc_indent()
            index_string = index_string + "[" + i + "]"
            index = index + 1
        self.__index = index_string
        self.__bounds = bounds
        self.__stream = stream
        self.__closed = (bounds != [])
    def __del__(self):
        if not self.__closed:
            warning("Possibly unterminated For loop generated")
    def index(self): return self.__index
    def end(self):
        for bound in self.__bounds:
            self.__stream.dec_indent()
            self.__stream.out("}")
        self.__closed = 1

# Block: Generates a C++ block with appropriate indenting
# Will generate a warning on premature destruction
#
# Example usage:
#    block = Block(stream)
#    stdout.out("// do something")
#    block.end()
#
class Block:
    def __init__(self, stream):
        stream.out("{")
        stream.inc_indent()
        self.__stream = stream
        self.__closed = 0
    def __del__(self):
        if not self.__closed:
            warning("Possibly unclosed Block generated")
    def end(self):
        self.__stream.dec_indent()
        self.__stream.out("}")
        self.__closed = 1


# Class: Generates a C++ class (both header and implementation code)
# Should be subclassed.
#
# .name():    return the class scoped name (id.Name)
# .forward(): forwrad declare this class
# .hh():      produce the class header
# .cc():      produce the class implementation
#
class Class:
  def __init__(self, name):
      assert isinstance(name, id.Name)

      self._name = name

  def name(self):
      return self._name

  def forward(self, stream):
      stream.out("class @name@;", name = self.name().simple())

  def hh(self, stream):
      raise NotImplementedError("Class header missing")

  def cc(self, stream):
      raise NotImplementedError("Class implementation missing")


# Method: Generates a C++ method (both header and implementation code)
#
# Note C++ method scoping rules:
#   module M{
#      typedef long T;
#      interface I{
#          T op(in T a);
#      };
#   };
# In the class _objref_I there is the prototype:
#    T    op(T& a)
# The implementation looks like this:
#    M::T op(T& a)
# ie in the implementation signature we must fully scope the return value.
class Method:
    def __init__(self, parent_class, name, return_type, arg_types, arg_names):
        self._parent_class = parent_class
        self._name = name
        self._return_type = return_type
        self._arg_types = arg_types
        self._arg_names = arg_names

    def parent_class(self): return self._parent_class
    def name(self):         return self._name
    def return_type(self):  return self._return_type
    def arg_types(self):    return self._arg_types
    def arg_names(self):    return self._arg_names

    # The types in the argument list never need to be fully qualified
    def __arglist(self, ignore_parameter_names = 0):
        arglist = []
        for x in range(len(self._arg_types)):
            this_arg = self._arg_types[x] + " " + self._arg_names[x]
            if ignore_parameter_names: # optional in method declaration
                this_arg = self._arg_types[x]
            arglist.append(this_arg)
        return string.join(arglist, ", ")

    def __return(self, fullyScope):
        if fullyScope:
            return self._return_type.op(types.RET)
        environment = self.parent_class().environment()
        return self._return_type.op(types.RET, environment)

    def hh(self, virtual = 0, pure = 0):
        args = "(" + self.__arglist(ignore_parameter_names = 0) + ")"
        proto = self.__return(fullyScope = 0) + " " + self.name()  + args
        if virtual: proto = "virtual " + proto
        if pure: proto = proto + " = 0"
        proto = proto + ";"
        return proto

    def cc(self, stream, body):
        args = "(" + self.__arglist() + ")"
        name = self.parent_class().name().fullyQualify() + "::" + self.name()
        proto = self.__return(fullyScope = 1) + " " + name + args
        stream.out("""\
@proto@
{
  @body@
}""", proto = proto, body = body)
        

# dimsToString: takes a list of int dimensions (eg [1,2,3]) and a prefix string
# returning a strings "[prefix1][prefix2]..."
def dimsToString(dims, prefix = ""):
    new_dims = []
    for x in dims:
        new_dims.append("[" + prefix + repr(x) + "]")

    return string.join(new_dims, "")


def warning(text):
    print "WARNING: " + text

            

