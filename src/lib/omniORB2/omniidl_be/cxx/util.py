# -*- python -*-
#                           Package   : omniidl
# util.py                   Created on: 1999/11/2
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
#   General utility functions designed for the C++ backend

# $Id$
# $Log$
# Revision 1.12.2.9  2000/08/07 15:34:35  dpg1
# Partial back-port of long long from omni3_1_develop.
#
# Revision 1.12.2.8  2000/06/28 13:59:04  dpg1
# Remove dependency on traceback module.
#
# Revision 1.12.2.7  2000/06/26 16:23:11  djs
# Added new backend arguments.
# Better error handling when encountering unsupported IDL (eg valuetypes)
# Refactoring of configuration state mechanism.
#
# Revision 1.12.2.6  2000/05/31 18:02:17  djs
# Better output indenting (and preprocessor directives now correctly output at
# the beginning of lines)
#
# Revision 1.12.2.5  2000/04/26 18:22:15  djs
# Rewrote type mapping code (now in types.py)
# Rewrote identifier handling code (now in id.py)
#
# Revision 1.12.2.4  2000/03/20 11:49:28  djs
# Added a "LazyStream" class to help reduce the amount of output buffering
# required
#
# Revision 1.12.2.3  2000/03/09 15:21:40  djs
# Better handling of internal compiler exceptions (eg attempts to use
# wide string types)
#
# Revision 1.12.2.2  2000/02/18 23:01:20  djs
# Updated example implementation code generating module
#
# Revision 1.12.2.1  2000/02/14 18:34:56  dpg1
# New omniidl merged in.
#
# Revision 1.12  2000/01/20 18:25:53  djs
# Got rid of some superfluous whitespace
#
# Revision 1.11  2000/01/17 16:59:53  djs
# Some whitespace stripping in StringStream
#
# Revision 1.10  2000/01/12 17:47:38  djs
# Reverted to simpler output stream design- will probably use dpg1's version
# in common with the python back end.
#
# Revision 1.9  2000/01/07 20:31:18  djs
# Regression tests in CVSROOT/testsuite now pass for
#   * no backend arguments
#   * tie templates
#   * flattened tie templates
#   * TypeCode and Any generation
#
# Revision 1.8  1999/11/29 19:27:00  djs
# Code tidied and moved around. Some redundant code eliminated.
#
# Revision 1.7  1999/11/26 18:51:44  djs
# Generates nicer output when doing blank substitutions
#
# Revision 1.6  1999/11/19 20:05:39  djs
# Removed superfluous function. Added zip.
#
# Revision 1.5  1999/11/15 19:10:55  djs
# Added module for utility functions specific to generating skeletons
# Union skeletons working
#
# Revision 1.4  1999/11/10 20:19:32  djs
# Option to emulate scope bug in old backend
# Array struct element fix
# Union sequence element fix
#
# Revision 1.3  1999/11/04 19:05:02  djs
# Finished moving code from tmp_omniidl. Regression tests ok.
#
# Revision 1.2  1999/11/03 17:35:07  djs
# Brought more of the old tmp_omniidl code into the new tree
#
# Revision 1.1  1999/11/03 11:09:50  djs
# General module renaming
#

"""General utility functions used by the C++ backend"""

from omniidl import idlutil, idltype
from omniidl_be.cxx import config

import sys, re, string

try:
    import traceback
    have_traceback = 1
except:
    have_traceback = 0


# -----------------------------------------------------------------
# Fatal error handling function
def fatalError(explanation):
    if config.state['Debug']:
        # don't exit the program in debug mode...
        print "omniidl: fatalError occurred, in debug mode."
        for line in string.split(explanation, "\n"):
            print ">> " + line
        print "Configuration state:"
        print "-------------------------"
        config.state.dump()

        if have_traceback:
            print "Stack:"
            print "-------------------------"
            traceback.print_stack()
            print "Exception:"
            print "-------------------------"
            traceback.print_exc()
        sys.exit(1)
    
    lines = string.split(explanation, "\n")
    lines = [ "Fatal error in C++ backend", "" ] + lines
    lines = lines + [ "Debug mode is currently off" ]

    for line in lines:
        sys.stderr.write("omniidl: " + line + "\n")

    sys.stderr.write("""\

For more information (mailing list archives, bug reports etc) please visit
the webpage:
  http://www.uk.research.att.com/omniORB/omniORB.html
""")
    sys.exit(1)

# Called whenever an unsupported IDL construct is found in the input
# (necessary because the front end supports all the new CORBA 2.3
# constructs whereas the ORB and correspondingly this backend does not)
def unsupportedIDL():
    e = """\
Unsupported IDL construct encountered in input.

omniORB does not currently support:
  IDL types longdouble, wchar, wstring, fixed, valuetype
"""
    fatalError(e)
    

# ------------------------------------------------------------------
# Generic output utilities

# Stream is a wrapper around a python file stream
# StringStream is a string buffer with a stream interface
#
# Holes in output template string can consist of:
#   Strings - inserted as is
#   StringStreams - converted into strings and inserted
#   Integers - converted into strings (using str) and inserted
#   Functions - stream is updated to reflect the current indent level
#      and the function is applied (permitting template driven code
#      rather than code-driving-template style)
#
# Multiline strings are inserted at the same indent level eg
#
# ~~~~~          ~~~~~
#   @foo@   ->     ~~~~~~
#                  ~~~~~~
# ~~~~~          ~~~~~              
#
# Entire lines are buffered and if they consist of a preprocessor
# directive ( ^\s*# ) the preceeding whitespace is trimmed.
#
# Note that a line which is empty apart from a hole is still a complete
# line even if the hole itself is empty. This commonly causes large blocks of
# empty lines in the output. For aesthetic effect, runs of more than 1 empty
# line are removed.

import re, string

# dummy function which exists to get a handle on its type
def fn():
    pass

class Stream:
    """Test output stream, same as omniidl.output.Stream but with an
       indirection (through write method)"""
    def __init__(self, file, indent_size = 2):
        self.indent = 0
        self.istring = " " * indent_size
        self.file = file

        self.pre_indent = ""
        self.line_buffer = ""

        self.last_line_empty = self.false

    function_type = type(fn)
    integer_type = type(1)
    string_type = type("foo")
    true = 1 == 1
    false = 1 == 0

    regex = re.compile(r"@([^@]*)@")

    def inc_indent(self): self.indent = self.indent + 1
    def dec_indent(self): self.indent = self.indent - 1
    def reset_indent(self): self.indent = 0
    def out(self, text, ldict={}, **dict):
        """Output a multi-line string with indentation and @@ substitution."""

        dict.update(ldict)

        def replace(match, dict=dict):
            if match.group(1) == "": return "@"
            return eval(match.group(1), globals(), dict)

        for l in string.split(text, "\n"):
            is_literal_text = 1 == 0
            tokens = string.split(l, "@")
            if tokens == []: continue

            pre_indent = self.istring * self.indent
            if string.lstrip(tokens[0]) == "":
                pre_indent = pre_indent + tokens[0]
                is_literal_text = not(is_literal_text)
                tokens = tokens[1:]

            start_of_line = self.true
            for token in tokens:
                is_literal_text = not(is_literal_text)

                
                if is_literal_text:
                    if start_of_line:
                        self.write(self.pre_indent + pre_indent)
                    self.write(token)    
                else:
                    thing = eval(token, globals(), dict)

                    expanded = ""
                    if isinstance(thing, self.string_type):
                        expanded = thing
                    elif isinstance(thing, StringStream):
                        expanded = str(thing)
                    elif isinstance(thing, self.function_type):
                        old_indent_str = self.pre_indent
                        old_indent_level = self.indent
                        self.indent = 0
                        self.pre_indent = self.pre_indent + pre_indent
                        apply(thing)
                        self.pre_indent = old_indent_str
                        self.indent = old_indent_level
                        continue
                    elif isinstance(thing, self.integer_type) or \
                         hasattr(thing, "__str__"):
                        expanded = str(thing)
                    else:
                        raise "What kind of type is " + repr(thing)

                    lines = string.split(expanded, "\n")
                    for n in range(0, len(lines)):
                        if start_of_line:
                            self.write(self.pre_indent + pre_indent)

                        start_of_line = self.false

                        self.write(lines[n])
                        if n < (len(lines) - 1):
                            self.write("\n")
                            start_of_line = self.true

                start_of_line = self.false
                
            self.write("\n")

    def niout(self, text, ldict={}, **dict):
        """Output a multi-line string without indentation."""

        dict.update(ldict)

        def replace(match, dict=dict):
            if match.group(1) == "": return "@"
            return eval(match.group(1), globals(), dict)

        for l in string.split(self.regex.sub(replace, text), "\n"):
            self.write(l)
            self.write("\n")

    def preproc(self):
        lstrip = string.lstrip(self.line_buffer)
        if (lstrip != "") and (lstrip[0] == "#"):
            self.line_buffer = lstrip
        
    def write(self, text):
        if text == "\n":
            self.preproc()

            this_line_empty = string.lstrip(self.line_buffer) == ""
            if not(self.last_line_empty and this_line_empty):
                self.writeln()
            else:
               self.line_buffer = ""
            self.last_line_empty = this_line_empty
            
            return
        self.line_buffer = self.line_buffer + text

    def writeln(self):
        self.file.write(self.line_buffer)
        self.file.write("\n")
        self.line_buffer = ""


class StringStream(Stream):
    """Writes to a string buffer rather than a file."""
    def __init__(self, indent_size = 2):
        Stream.__init__(self, None, indent_size)
        self.__buffer = ""

    def writeln(self):
        self.__buffer = self.__buffer + self.line_buffer + "\n"
        self.line_buffer = ""

    def __str__(self):
        return self.__buffer

    def __add__(self, other):
        return self.__buffer + str(other)


# ------------------------------------------------------------------
# Set manipulation functions

def union(a, b):
    result = a[:]
    for x in b:
        if not(x in result):
            result.append(x)
    return result

def minus(a, b):
    result = []
    for x in a:
        if not(x in b):
            result.append(x)
    return result

def intersect(a, b):
    result = []
    for x in a:
        if x in b:
            result.append(x)
    return result

def setify(set):
    new_set = []
    for x in set:
        if not(x in new_set):
            new_set.append(x)

    return new_set

# ------------------------------------------------------------------
# List manipulation functions

def zip(a, b):
    if a == [] or b == []: return []
    return [(a[0], b[0])] + zip(a[1:], b[1:])

def fold(list, base, fn):
    if len(list) == 1:
        return fn(list[0], base)
    first = fn(list[0], list[1])
    rest = [first] + list[2:]
    return fold(rest, base, fn)

def append(x, y):
    return x + y

# ------------------------------------------------------------------
# Useful C++ things

# Build a loop iterating over every element of a multidimensional
# thing and return the indexing string
def start_loop(where, dims, prefix = "_i"):
    index = 0
    istring = ""
    iter_type = "CORBA::ULong"
    for dimension in dims:
        where.out("""\
for (@iter_type@ @prefix@@n@ = 0;@prefix@@n@ < @dim@;_i@n@++) {""",
                  iter_type = iter_type,
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
    
# Create a nested block as a container and call start_loop
def block_begin_loop(string, full_dims):
    iter_type = "CORBA::ULong"
    string.out("{")
    string.inc_indent()
    return start_loop(string, full_dims)

# finish the loop and close the nested block
def block_end_loop(string, full_dims):
    finish_loop(string, full_dims)
            
    string.dec_indent()
    string.out("}")
