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
# Revision 1.2  1999/11/10 20:19:30  djs
# Option to emulate scope bug in old backend
# Array struct element fix
# Union sequence element fix
#
# Revision 1.1  1999/11/03 17:33:41  djs
# Brought more of the old tmp_omniidl code into the new tree
#

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
    return "_opers"

# suffix to be added to basename to get the filename of the operators fragment file
def poa_fragment_suffix():
    return "_poa"
    
# list of all files #included in the IDL
def include_file_names():
    # EITHER modify the FE/BE interface to fetch this information from the
    # idl_global structure
    # OR rebuild it by traversing the AST in python
    return []

# completely emulate the old backend, bugs and all
def EMULATE_BUGS():
    return 1
