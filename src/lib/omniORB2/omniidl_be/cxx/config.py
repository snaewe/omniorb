# -*- python -*-
#                           Package   : omniidl
# config.py                 Created on: 1999/11/2
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
#   Global config info for the C++ backend

# $Id$
# $Log$
# Revision 1.7  2000/01/12 17:48:28  djs
# Added option to create BOA compatible skeletons (same as -BBOA in omniidl3)
#
# Revision 1.6  2000/01/11 11:35:36  djs
# Added support for fragment generation (-F) mode
#
# Revision 1.5  1999/12/24 18:20:12  djs
# Builds list of IDL files #included by walking the AST and examining the
# file() of each Decl node.
#
# Revision 1.4  1999/12/01 17:04:21  djs
# Added global config options for Typecodes and Anys, and forms of tie templates
#
# Revision 1.3  1999/11/12 17:17:45  djs
# Creates output files rather than using stdout
# Utility functions useful for skeleton generation added
#
# Revision 1.2  1999/11/10 20:19:30  djs
# Option to emulate scope bug in old backend
# Array struct element fix
# Union sequence element fix
#
# Revision 1.1  1999/11/03 17:33:41  djs
# Brought more of the old tmp_omniidl code into the new tree
#

from omniidl import idlvisitor
import config

#
# Location where configuration data pertinent to the current run of the
# compiler is stored
#
# similar in purpose to idl_global()-> data and o2be::global data in the
# omniidl2 c++ compiler

# name of the program itself
def program_Name():
    return "omniidl3"
    
# version of the library
def omniORB_Library_Version():
    return "omniORB_3_0"
    
# base name of the file being processed
def setBasename(b):
    global __basename
    __basename = b
def basename():
    global __basename
    return __basename

# generate code for TypeCodes and Any
def setTypecodeFlag(flag):
    global __typecode
    __typecode = flag
def TypecodeFlag():
    global __typecode
    return __typecode

# generate code for 'tie' implementational skeletons
def setTieFlag(flag):
    global __tie
    __tie = flag
def TieFlag():
    global __tie
    return __tie

# generate code for flattened 'tie' implementational skeletons
def setFlatTieFlag(flag):
    global __flattie
    __flattie = flag
def FlatTieFlag():
    global __flattie
    return __flattie

# generate fragments
def setFragmentFlag(flag):
    global __fragment
    __fragment = flag
def FragmentFlag():
    global __fragment
    return __fragment

# generate boa compatible skeletons
def setBOAFlag(flag):
    global __BOA
    __BOA = flag
def BOAFlag():
    global __BOA
    return __BOA
    
# suffix added to basename to get header filename
def hdrsuffix():
    return ".hh"
    
# suffix added to basename to get the filename of the skeleton cc file
def skelsuffix():
    return "SK.cc"
    
# suffix added to basename to get the filename of the dynamic skeleton cc file
def dynskelsuffix():
    return "DynSK.cc"

# suffix to be added to basename to get the filename of the defs fragment file
def defs_fragment_suffix():
    return "_defs"

# suffix to be added to basename to get the filename of the operators fragment file
def opers_fragment_suffix():
    return "_operators"

# suffix to be added to basename to get the filename of the operators fragment file
def poa_fragment_suffix():
    return "_poa"

# prefix to be added to avoid occasional name clashes
def name_prefix():
    return "_0RL"
    
# list of all files #included in the IDL
includes = []
def include_file_names():
    # EITHER modify the FE/BE interface to fetch this information from the
    # idl_global structure
    # OR rebuild it by traversing the AST in python
    return config.includes

# completely emulate the old backend, bugs and all
def EMULATE_BUGS():
    return 1




# Traverses the AST compiling the list of files #included by the main
# IDL file. Note that types constructed within other types must necessarily
# be in the same IDL file
class WalkTreeForIncludes(idlvisitor.AstVisitor):
    def add(self, node):
        file = node.file()
        if not(file in config.includes):
            config.includes = [file] + config.includes

    def visitAST(self, node):
        self.add(node)
        for d in node.declarations(): d.accept(self)
    def visitModule(self, node):
        self.add(node)
        for d in node.definitions(): d.accept(self)
    def visitInterface(self, node): self.add(node)
    def visitForward(self, node):   self.add(node)
    def visitConst(self, node):     self.add(node)
    def visitTypedef(self, node):   self.add(node)
    def visitStruct(self, node):    self.add(node)
    def visitException(self, node): self.add(node)
    def visitUnion(self, node):     self.add(node)
    def visitEnum(self, node):      self.add(node)
    
