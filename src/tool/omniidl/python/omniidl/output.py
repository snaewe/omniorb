# -*- python -*-
#                           Package   : omniidl
# output.py                 Created on: 1999/10/27
#			    Author    : Duncan Grisby (dpg1)
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
#   IDL compiler output functions

"""Output stream

Class:

  Stream -- output stream which outputs templates, performing
            key/value substitution and indentation."""

import re, string

class Stream:
    """IDL Compiler output stream class

The output stream takes a template string containing keys enclosed in
'@' characters and replaces the keys with their associated values. It
also provides counted indentation levels.

  eg. Given the template string:

    template = \"\"\"\\
class @id@ {
public:
  @id@(@type@ a) : a_(a) {}

private:
  @type@ a_;
};\"\"\"

  Calling s.out(template, id="foo", type="int") results in:

    class foo {
    public:
      foo(int a) : a_(a) {}

    private:
      int a_;
    };


Functions:

  __init__(file, indent_size)   -- Initialise the stream with the
                                   given file and indent size.
  inc_indent()                  -- Increment the indent level.
  dec_indent()                  -- Decrement the indent level.
  out(template, key=val, ...)   -- Output the given template with
                                   key/value substitution and
                                   indenting.
  niout(template, key=val, ...) -- As out(), but with no indenting."""


    def __init__(self, file, indent_size = 4):
        self.indent  = 0
        self.file    = file
        self.istring = " " * indent_size

    regex = re.compile(r"@([^@]*)@")

    def inc_indent(self): self.indent = self.indent + 1
    def dec_indent(self): self.indent = self.indent - 1

    def out(self, text, ldict={}, **dict):
        """Output a multi-line string with indentation and @@ substitution."""

        dict.update(ldict)

        def replace(match, dict=dict):
            if match.group(1) == "": return "@"
            return eval(match.group(1), globals(), dict)

        for l in string.split(self.regex.sub(replace, text), "\n"):
            self.file.write(self.istring * self.indent)
            self.file.write(l)
            self.file.write("\n")

    def niout(self, text, ldict={}, **dict):
        """Output a multi-line string without indentation."""

        dict.update(ldict)

        def replace(match, dict=dict):
            if match.group(1) == "": return "@"
            return eval(match.group(1), globals(), dict)

        for l in string.split(self.regex.sub(replace, text), "\n"):
            self.file.write(l)
            self.file.write("\n")
