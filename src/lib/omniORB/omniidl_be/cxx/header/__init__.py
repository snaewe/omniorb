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
# Revision 1.17.2.3  2005/01/06 23:10:01  dgrisby
# Big merge from omni4_0_develop.
#
# Revision 1.17.2.2  2003/10/23 11:25:55  dgrisby
# More valuetype support.
#
# Revision 1.17.2.1  2003/03/23 21:02:39  dgrisby
# Start of omniORB 4.1.x development branch.
#
# Revision 1.14.2.6  2003/01/22 12:10:55  dgrisby
# Explicitly close files in C++ backend.
#
# Revision 1.14.2.5  2001/10/18 12:45:28  dpg1
# IDL compiler tweaks.
#
# Revision 1.14.2.4  2001/03/26 11:11:54  dpg1
# Python clean-ups. Output routine optimised.
#
# Revision 1.14.2.3  2001/01/25 13:09:11  sll
# Fixed up cxx backend to stop it from dying when a relative
# path name is given to the -p option of omniidl.
#
# Revision 1.14.2.2  2000/10/12 15:37:50  sll
# Updated from omni3_1_develop.
#
# Revision 1.15.2.1  2000/08/21 11:35:14  djs
# Lots of tidying
#
# Revision 1.15  2000/07/13 15:26:00  dpg1
# Merge from omni3_develop for 3.0 release.
#
# Revision 1.12.2.7  2000/06/26 16:23:57  djs
# Better handling of #include'd files (via new commandline options)
# Refactoring of configuration state mechanism.
#
# Revision 1.12.2.6  2000/06/05 13:03:54  djs
# Removed union member name clash (x & pd_x, pd__default, pd__d)
# Removed name clash when a sequence is called "pd_seq"
# Nested union within union fix
# Actually generates BOA non-flattened tie templates
#
# Revision 1.12.2.5  2000/05/31 18:02:56  djs
# Better output indenting (and preprocessor directives now correctly output at
# the beginning of lines)
#
# Revision 1.12.2.4  2000/04/26 18:22:27  djs
# Rewrote type mapping code (now in types.py)
# Rewrote identifier handling code (now in id.py)
#
# Revision 1.12.2.3  2000/03/21 10:58:49  djs
# Forgot to make fragment mode use LazyStream instead of Stream
#
# Revision 1.12.2.2  2000/03/20 11:50:18  djs
# Removed excess buffering- output templates have code attached which is
# lazily evaluated when required.
#
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

# output generation
import omniidl_be.cxx.header.opers
import omniidl_be.cxx.header.poa
import omniidl_be.cxx.header.obv
import omniidl_be.cxx.header.tie
import omniidl_be.cxx.header.forward
import omniidl_be.cxx.header.marshal
import omniidl_be.cxx.header.tcstring
import omniidl_be.cxx.header.defs

from omniidl_be.cxx.header import template

from omniidl_be.cxx import config, output, ast, id

import re, sys, os.path, string


def header(stream, filename):
    stream.out(template.header,
               program = config.state['Program Name'],
               library = config.state['Library Version'],
               guard   = filename)

def footer(stream):
    stream.out(template.footer)

def defs_fragment(stream, tree):
    """Creates the defs fragment only"""
    filename = config.state['Basename'] + config.state['_DEFS Fragment']
    header(stream, filename)

    # generate the header definitions
    import omniidl_be
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
    filename = config.state['Basename'] + config.state['_OPERS Fragment']
    header(stream, filename)

    # see o2be_root::produce_hdr and o2be_root::produce_hdr_defs
    opers = omniidl_be.cxx.header.opers.__init__(stream)
    tree.accept(opers)

    marshal = omniidl_be.cxx.header.marshal.__init__(stream)
    tree.accept(marshal)

    footer(stream)

def poa_fragment(stream, tree):
    """Creates the poa fragment only"""
    filename = config.state['Basename'] + config.state['_POA Fragment']
    header(stream, filename)

    poa = omniidl_be.cxx.header.poa.__init__(stream)
    tree.accept(poa)

    footer(stream)

def monolithic(stream, tree):
    """Creates one large header with all definitions inside"""

    guard = id.Name([config.state['Basename']]).guard()

    header(stream, guard)

    # Extra DLL include stuff?
    if config.state['DLLIncludes']:
        sub_include_pre  = output.StringStream()
        sub_include_post = output.StringStream()
        sub_include_pre .out(template.sub_include_pre,  guard=guard)
        sub_include_post.out(template.sub_include_post, guard=guard)
    else:
        sub_include_pre  = ""
        sub_include_post = ""

    # Add in any direct C++ from toplevel pragma if present
    cxx_direct_include = []
    directive = "hh "
    for pragma in tree.pragmas():
        # ignore all pragmas but those in the main file
        if pragma.file() != tree.file(): continue
        
        if pragma.text()[:len(directive)] == directive:
            cxx_direct_include.append(pragma.text()[len(directive):])
    
    includes = output.StringStream()

    # produce #includes for all files included by the IDL
    for include in ast.includes():
        # skip the main file
        if ast.mainFile() == include:
            continue
        
        # the old C++ BE makes orb.idl a special case
        # (might now be a redundant test)
        
        # dirname  == directory containing the include file
        # filename == extensionless filename
        # ext      == extension (typically .idl)
        (root, ext) = os.path.splitext(include)
        (dirname, filename) = os.path.split(root)

        # Name for external include guard. Always use the same suffix,
        # rather than taking suffix from the config.
        guardname = id.Name([filename]).guard() + "_hh"

        cxx_include = filename + config.state['HH Suffix']
        if config.state['Keep Include Path']:
            cxx_include = os.path.join(dirname, cxx_include)

        if config.state['Use Quotes']:
            cxx_include = "\"" + cxx_include + "\""
        else:
            cxx_include = "<" + cxx_include + ">"
            
        includes.out(template.main_include,
                     guardname = guardname, filename = cxx_include)

    # see o2be_root::produce_hdr and o2be_root::produce_hdr_defs

    # generate the header definitions
    def forward_dec(stream = stream, tree = tree):
        forward = omniidl_be.cxx.header.forward.__init__(stream)
        tree.accept(forward)

    # generate the bounded string tcParser stuff
    def string_tcparser(stream = stream, tree = tree):
        tcstring = omniidl_be.cxx.header.tcstring.__init__(stream)
        tree.accept(tcstring)

    def main_defs(stream = stream, tree = tree):
        defs = omniidl_be.cxx.header.defs.__init__(stream)
        tree.accept(defs)

    def main_poa(stream = stream, tree = tree):
        # includes inline (non-flat) tie templates
        poa = omniidl_be.cxx.header.poa.__init__(stream)
        tree.accept(poa)

    def main_obv(stream = stream, tree = tree):
        obv = omniidl_be.cxx.header.obv.__init__(stream)
        tree.accept(obv)

    def other_tie(stream = stream, tree = tree):
        if config.state['Normal Tie'] and config.state['BOA Skeletons']:
            tie = omniidl_be.cxx.header.tie.BOATieTemplates(stream)
            tree.accept(tie)
        
        if config.state['Flattened Tie']:
            tie = omniidl_be.cxx.header.tie.FlatTieTemplates(stream)
            tree.accept(tie)

    # see o2be_root::produce_hdr and o2be_root::produce_hdr_defs
    def main_opers(stream = stream, tree = tree):
        opers = omniidl_be.cxx.header.opers.__init__(stream)
        tree.accept(opers)
        
    def main_marshal(stream = stream, tree = tree):
        marshal = omniidl_be.cxx.header.marshal.__init__(stream)
        tree.accept(marshal)

    # other stuff
    stream.out(template.main,
               cxx_direct_include = string.join(cxx_direct_include, "\n"),
               includes = includes,
               forward_declarations = forward_dec,
               string_tcParser_declarations = string_tcparser,
               defs = main_defs,
               poa = main_poa,
               obv = main_obv,
               other_tie = other_tie,
               operators = main_opers,
               marshalling = main_marshal,
               guard = guard,
               sub_include_pre = sub_include_pre,
               sub_include_post = sub_include_post)


def run(tree):
    if config.state['Fragment']:
        # build the defs file
        defs_filename = config.state['Basename']         +\
                        config.state['_DEFS Fragment']   +\
                        config.state['HH Suffix']
        defs_stream = output.Stream(output.createFile(defs_filename), 2)
        defs_fragment(defs_stream, tree)

        # build the opers file
        opers_filename = config.state['Basename']        +\
                         config.state['_OPERS Fragment'] +\
                         config.state['HH Suffix']
        opers_stream = output.Stream(output.createFile(opers_filename), 2)
        opers_fragment(opers_stream, tree)

        # build the poa file
        poa_filename = config.state['Basename']          +\
                       config.state['_POA Fragment']     +\
                       config.state['HH Suffix']
        poa_stream = output.Stream(output.createFile(poa_filename), 2)
        poa_fragment(poa_stream, tree)

        defs_stream.close()
        opers_stream.close()
        poa_stream.close() 
    else:
        # build the full header file
        header_filename = config.state['Basename']       +\
                          config.state['HH Suffix']
        stream = output.Stream(output.createFile(header_filename), 2)
        # generate one big chunk of header
        monolithic(stream, tree)

        stream.close()
