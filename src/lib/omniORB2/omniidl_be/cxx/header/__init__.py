# -*- python -*-
#                           Package   : omniidl
# __init__.py               Created on: 1999/11/3
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
#   Entrypoint to header generation code

# $Id$
# $Log$
# Revision 1.12.2.1  2000/02/14 18:34:55  dpg1
# New omniidl merged in.
#
# Revision 1.12  2000/01/20 18:26:13  djs
# Tie template output order problem
#
# Revision 1.11  2000/01/19 11:23:27  djs
# Moved most C++ code to template file
#
# Revision 1.10  2000/01/17 17:00:21  djs
# Runs tcParser #ifdefs for bounded strings
#
# Revision 1.9  2000/01/11 11:34:27  djs
# Added support for fragment generation (-F) mode
#
# Revision 1.8  2000/01/10 17:18:14  djs
# Removed redundant code.
#
# Revision 1.7  1999/12/24 18:14:29  djs
# Fixed handling of #include'd .idl files
#
# Revision 1.6  1999/12/13 15:40:26  djs
# Added generation of "flattened" tie templates
#
# Revision 1.5  1999/12/01 17:03:37  djs
# Added new modules
#
# Revision 1.4  1999/11/12 17:18:39  djs
# Lots of header generation bugfixes
#
# Revision 1.3  1999/11/04 19:05:08  djs
# Finished moving code from tmp_omniidl. Regression tests ok.
#
# Revision 1.2  1999/11/03 17:35:11  djs
# Brought more of the old tmp_omniidl code into the new tree
#

# -----------------------------
# Configuration data
from omniidl_be.cxx import config

# -----------------------------
# Output generation functions
import omniidl_be.cxx.header.defs
import omniidl_be.cxx.header.opers
import omniidl_be.cxx.header.poa
import omniidl_be.cxx.header.tie
import omniidl_be.cxx.header.forward
import omniidl_be.cxx.header.marshal
import omniidl_be.cxx.header.tcstring

from omniidl_be.cxx.header import template

# -----------------------------
# Utility functions
from omniidl_be.cxx import tyutil, util

# -----------------------------
# System functions
import re, sys


def header(stream, filename):
    #guard = re.sub(r"\W", "_", filename)
    
    stream.out(template.header,
               Config = config, guard = filename)

def footer(stream):
    stream.out(template.footer)

def defs_fragment(stream, tree):
    """Creates the defs fragment only"""
    filename = config.basename() + config.defs_fragment_suffix()
    header(stream, filename)

    # generate the header definitions
    forward = omniidl_be.cxx.header.forward.__init__(stream)
    tree.accept(forward)
    
    # generate the bounded string tcParser stuff
    tcstring = omniidl_be.cxx.header.tcstring.__init__(stream)
    tree.accept(tcstring)

    defs = omniidl_be.cxx.header.defs.__init__(stream)
    tree.accept(defs)

    footer(stream)

def opers_fragment(stream, tree):
    """Creates the opers fragment only"""
    filename = config.basename() + config.opers_fragment_suffix()
    header(stream, filename)

    # see o2be_root::produce_hdr and o2be_root::produce_hdr_defs
    opers = omniidl_be.cxx.header.opers.__init__(stream)
    tree.accept(opers)

    marshal = omniidl_be.cxx.header.marshal.__init__(stream)
    tree.accept(marshal)

    footer(stream)

def poa_fragment(stream, tree):
    """Creates the poa fragment only"""
    filename = config.basename() + config.poa_fragment_suffix()
    header(stream, filename)
    
    poa = omniidl_be.cxx.header.poa.__init__(stream)
    tree.accept(poa)

    footer(stream)

def monolithic(stream, tree):
    """Creates one large header with all definitions inside"""

    # inconsistancy- everywhere else transforms "_" into "__"
    # but the old BE doesn't here
    guard = re.sub(r"\W", "_", config.basename())

    header(stream, guard)
    
    includes = util.StringStream()
    # produce #includes for all files included by the IDL
    for include in config.include_file_names():
        # skip the main file
        if tree.file() == include:
            continue
        
        # the old C++ BE makes orb.idl a special case
        # (might now be a redundant test)
        
        # chop off the current extension and replace with a .hh
        # s/(.*)\./\1\.hh/
        match = re.compile(r"(.*)\.").match(include)
        filename = match.group(1)
        # chop off the directories and just get the filename
        # I bet this causes lots of trouble later...
        elements = re.split(r"/", filename)
        filename = elements[len(elements) - 1]
        filename = filename + config.hdrsuffix()
        # s/\W/_/g
        guardname = tyutil.guardName([filename])

        includes.out(template.main_include,
                     guardname = guardname, filename = filename)

    # see o2be_root::produce_hdr and o2be_root::produce_hdr_defs

    forward_dec = util.StringStream()
    # generate the header definitions
    forward = omniidl_be.cxx.header.forward.__init__(forward_dec)
    tree.accept(forward)

    string_tcparser = util.StringStream()
    # generate the bounded string tcParser stuff
    tcstring = omniidl_be.cxx.header.tcstring.__init__(string_tcparser)
    tree.accept(tcstring)

    main_defs = util.StringStream()
    defs = omniidl_be.cxx.header.defs.__init__(main_defs)
    tree.accept(defs)

    main_poa = util.StringStream()
    main_tie = util.StringStream()
    
    poa = omniidl_be.cxx.header.poa.__init__(main_poa)
    tree.accept(poa)
    if config.FlatTieFlag():
        tie = omniidl_be.cxx.header.tie.__init__(main_tie)
        tree.accept(tie)

    # see o2be_root::produce_hdr and o2be_root::produce_hdr_defs
    main_opers = util.StringStream()
    opers = omniidl_be.cxx.header.opers.__init__(main_opers)
    tree.accept(opers)

    main_marshal = util.StringStream()
    marshal = omniidl_be.cxx.header.marshal.__init__(main_marshal)
    tree.accept(marshal)

    # other stuff
    stream.out(template.main,
               includes = str(includes),
               forward_declarations = str(forward_dec),
               string_tcParser_declarations = str(string_tcparser),
               defs = str(main_defs),
               poa = str(main_poa),
               tie = str(main_tie),
               operators = str(main_opers),
               marshalling = str(main_marshal),
               guard = guard)


def run(tree):
    if config.FragmentFlag():
        # build the defs file
        defs_filename = config.basename() + config.defs_fragment_suffix() +\
                        config.hdrsuffix()
        defs_stream = util.Stream(open(defs_filename, "w"), 2)
        defs_fragment(defs_stream, tree)

        # build the opers file
        opers_filename = config.basename() + config.opers_fragment_suffix() +\
                         config.hdrsuffix()
        opers_stream = util.Stream(open(opers_filename, "w"), 2)
        opers_fragment(opers_stream, tree)

        # build the poa file
        poa_filename = config.basename() + config.poa_fragment_suffix() +\
                       config.hdrsuffix()
        poa_stream = util.Stream(open(poa_filename, "w"), 2)
        poa_fragment(poa_stream, tree)
    else:
        # build the full header file
        header_filename = config.basename() + config.hdrsuffix()
        stream = util.Stream(open(header_filename, "w"), 2)
        # generate one big chunk of header
        monolithic(stream, tree)

