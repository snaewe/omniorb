# -*- python -*-
#                           Package   : omniidl
# output.py                 Created on: 2000/8/10
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
#   Routines for managing output

# $Id$
# $Log$
# Revision 1.1.4.1  2000/10/12 15:37:48  sll
# Updated from omni3_1_develop.
#
# Revision 1.1.2.1  2000/08/21 11:34:35  djs
# Lots of omniidl/C++ backend changes
#

"""Routines for managing output"""

import output
self = output

# Wrapper around the standard library file opening routines, keeps track of
# which files have been created by the backend so they can be cleaned up on
# error.
self.createdFiles = []

def createFile(filename):
    file = open(filename, "w")
    self.createdFiles.append(file)
    return file

def listAllCreatedFiles():
    return self.createdFiles


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
import output


# dummy function which exists to get a handle on its type
def fn():
    pass


class Stream:
    def __init__(self, file, indent_size = 2):
        self.indent = 0
        self.istring = " " * indent_size
        self.file = file

        self.pre_indent = ""
        self.line_buffer = ""

        self.last_line_empty = self.false

    function_type = type(output.fn)
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


