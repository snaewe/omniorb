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
# Revision 1.12.2.1  2000/02/18 23:01:20  djs
# Updated example implementation code generating module
#
# Revision 1.12  2000/01/13 15:56:30  djs
# Factored out private identifier prefix rather than hard coding it all through
# the code.
#
# Revision 1.11  2000/01/13 14:16:20  djs
# Properly clears state between processing separate IDL input files
#
# Revision 1.10  2000/01/13 11:45:47  djs
# Added option to customise C++ reserved word name escaping
#
# Revision 1.9  2000/01/13 10:52:04  djs
# Rewritten argument handling
# Added options to specify the header and stubs file prefix, mirroring omniidl3
#
# Revision 1.8  2000/01/12 19:54:47  djs
# Added option to generate old CORBA 2.1 signatures for skeletons
#
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
self = config

# Location where configuration data pertinent to the current run of the
# compiler is stored
#
# similar in purpose to idl_global()-> data and o2be::global data in the
# omniidl2 c++ compiler

self._programName   = "omniidl3"    # programs own name
self._libVersion    = "omniORB_3_0" # library version
self._hdrsuffix     = ".hh"         # suffix for header files
self._skelsuffix    = "SK.cc"       # suffix for stub files
self._dynskelsuffix = "DynSK.cc"    # suffix for the dynamic stuff
self._implsuffix    = "_i.cc"

self._defs_fragment  = "_defs"      # header definitions fragment suffix
self._opers_fragment = "_operators" # header operators fragment suffix
self._poa_fragment   = "_poa"       # header POA fragment suffix


self._name_prefix   = "_0RL"        # private name prefix

self._res_prefix    = "_cxx_"       # prefix to map IDL identifiers which
                                    # are C++ reserved words

# name of the program itself
def program_Name():
    return self._programName
    
# version of the library
def omniORB_Library_Version():
    return self._libVersion
    
# base name of the file being processed
self._basename = ""
def setBasename(b):
    self._basename = b
def basename():
    return self._basename

def reservedPrefix():
    return self._res_prefix
def setReservedPrefix(prefix):
    self._res_prefix = prefix

# generate code for TypeCodes and Any
self._typecode = 0
def setTypecodeFlag(flag):
    self._typecode = flag
def TypecodeFlag():
    return self._typecode

# generate example implementation code
self._examplecode = 0
def setExampleFlag(flag):
    self._examplecode = flag
def ExampleFlag():
    return self._examplecode

# generate code for 'tie' implementational skeletons
self._tie = 0
def setTieFlag(flag):
    self._tie = flag
def TieFlag():
    return self._tie

# generate code for flattened 'tie' implementational skeletons
self._flat = 0
def setFlatTieFlag(flag):
    self._flat = flag
def FlatTieFlag():
    return self._flat

# generate fragments
self._fragment = 0
def setFragmentFlag(flag):
    self._fragment = flag
def FragmentFlag():
    return self._fragment

# generate boa compatible skeletons
self._BOA = 0
def setBOAFlag(flag):
    self._BOA = flag
def BOAFlag():
    return self._BOA

# generate old CORBA 2.1 signatures for skeletons
self._old = 0
def setOldFlag(flag):
    self._old = flag
def OldFlag():
    return self._old
    
# suffix added to basename to get header filename
def sethdrsuffix(hh):
    self._hdrsuffix = hh
    
def hdrsuffix():
    return self._hdrsuffix

    
# suffix added to basename to get the filename of the skeleton cc file
def setskelsuffix(sk):
    self._skelsuffix = sk
    
def skelsuffix():
    return self._skelsuffix
    
# suffix added to basename to get the filename of the dynamic skeleton cc file
def dynskelsuffix():
    return self._dynskelsuffix

# suffix to be added to basename to get the filename of the defs fragment file
def defs_fragment_suffix():
    return self._defs_fragment

# suffix to be added to basename to get the filename of the operators fragment file
def opers_fragment_suffix():
    return self._opers_fragment

# suffix to be added to basename to get the filename of the operators fragment file
def poa_fragment_suffix():
    return self._poa_fragment

def implsuffix():
    return self._implsuffix

# prefix to be added to avoid occasional name clashes
def privatePrefix():
    return self._name_prefix
    
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
    def __init__(self):
        config.includes = []
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
    

