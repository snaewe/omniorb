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
from omniidl.be.cxx import config

import re, string

# ------------------------------------------------------------------
# Generic formatting functions


# ------------------------------------------------------------------
# Generic output utilities

#   (modified from dpg1s to keep track of output indent level
#    semi-automatically)
#   
# Doesnt have a stack of preceeding whitespace length as python does, instead
# interprets an increase as a single inc_indent() and a decrease as a
# dec_indent() First output line within a Stream.out() call sets the origin
# for the successive lines. An @...@ expansion which becomes multiple lines
# is output at a contant indent.

import re, string

class Stream:
    """IDL Compiler output stream."""
    def __init__(self, file, indent_size = 4):
        self.indent  = 0
        self.file    = file
        self.istring = " " * indent_size
        #
        self.__wsregex = re.compile(r"^\s*")
 
    regex = re.compile(r"@([^@]*)@")

    def inc_indent(self): self.indent = self.indent + 1
    def dec_indent(self): self.indent = self.indent - 1
    def reset_indent(self): self.indent = 0

    def __wscount(self, text):
        match = self.__wsregex.match(text)
        if (match):
            return match.end(0)
        return 0

    def __wsstrip(self, text):
        match = self.__wsregex.match(text)
        if (match):
            return text[match.end(0):]
        return text

    def write(self, text):
        self.file.write(text + "\n")

    def out(self, text, ldict={}, **dict):
        """Output a multi-line string with indentation and @@ substitution."""

        dict.update(ldict)

        def doSub(self, text, dict=dict):
            # doSub: Stream * string -> string list
            #
            def replace(match, dict=dict):
                if match.group(1) == "": return "@"
                return eval(match.group(1), globals(), dict)
            # remove leading whitespace, but keep a note of how much
            wsCount = self.__wscount(text)
            strippedText = self.__wsstrip(text)
            lines = string.split(self.regex.sub(replace, strippedText), "\n")
            # lines now contains a list of strings (most of the time
            # a single element list)

            # if the line only had a single @subst@ expression which
            # was evaluated to nothing, then skip the extra newline
            if self.regex.search(strippedText) and \
               lines == [""]:
                return []
            
            indentedLines = []
            for line in lines:
                indentedLines.append(" "*wsCount + line)
            return indentedLines

        rawLines = string.split(text, "\n")
        lines = []
        for line in rawLines:
            afterSubs = doSub(self, line)
            lines = lines + afterSubs
            
        # count the preceeding whitespace
        whitespace = self.__wscount(lines[0])
        
        for l in lines:
            new_whitespace = self.__wscount(l)
            if (new_whitespace > whitespace):
                self.inc_indent()
            elif (new_whitespace < whitespace):
                self.dec_indent()
            whitespace = new_whitespace
            
            self.write(self.istring * self.indent + self.__wsstrip(l))

    def niout(self, text, ldict={}, **dict):
        """Output a multi-line string without indentation."""

        oldIndent = self.indent
        self.indent = 0
        self.out(text, ldict) # FIXME
        self.indent = oldIndent
        

# simpler version (almost identical to dpg1s, except with the self.write
# method):
class Stream:
    """Test output stream, same as omniidl.output.Stream but with an
       indirection (through write method)"""
    def __init__(self, file, indent_size = 4):
        self.indent = 0
        self.istring = " " * indent_size
        self.file = file
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

        for l in string.split(self.regex.sub(replace, text), "\n"):
            self.write(self.istring * self.indent)
            self.write(l)
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

    def write(self, text):
        self.file.write(text)

        

class StringStream(Stream):
    """Writes to a string buffer rather than a file."""
    def __init__(self, indent_size = 4):
        Stream.__init__(self, None, indent_size)
        self.__buffer = ""

    def write(self, text):
        strings = string.split(text, '\n')
        for s in strings:
            if s != "":
                
                if (self.__buffer != ""):
                    self.__buffer = self.__buffer + "\n"
                self.__buffer = self.__buffer + s + "\n"
                
        #self.__buffer = self.__buffer + text

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


# ------------------------------------------------------------------
# Useful C++ things

# Build a loop iterating over every element of a multidimensional
# thing and return the indexing string
# Everywhere but in exception skeletons is iter_type = CORBA::ULong.
def start_loop(where, dims, prefix = "_i", iter_type = "CORBA::ULong"):
    index = 0
    istring = ""
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
def block_begin_loop(string, full_dims, iter_type = "CORBA::ULong"):
    string.out("{")
    string.inc_indent()
    return start_loop(string, full_dims, iter_type = iter_type)

# finish the loop and close the nested block
def block_end_loop(string, full_dims):
    finish_loop(string, full_dims)
            
    string.dec_indent()
    string.out("}")
