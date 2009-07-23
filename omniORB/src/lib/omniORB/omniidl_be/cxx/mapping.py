# -*- python -*-
#                           Package   : omniidl
# mapping.py                Created on: 2003/09/29
#			    Author    : Duncan Grisby
#
#    Copyright (C) 2003 Apasphere Ltd.
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
#  Abstract base class for C++ type mappings
#


from omniidl import idlast, idltype

class Decl:
    def __init__(self, astdecl):
        self._astdecl = astdecl

    def astdecl(self):
        return self._astdecl

    #
    # Header functions
    #

    def pre_module_decls(self, stream, visitor):
        """Declarations that come before IDL modules are opened."""
        pass

    def module_decls(self, stream, visitor):
        """Declarations that come inside the IDL module."""
        pass

    def poa_module_decls(self, stream, visitor):
        """Declarations in the POA_ module."""
        pass

    def obv_module_decls(self, stream, visitor):
        """Declarations in the OBV_ module."""
        pass

    def post_module_decls(self, stream, visitor):
        """Declarations after modules."""
        pass

    #
    # SK functions
    #

    def skel_defs(self, stream, visitor):
        """Definitions in the SK.cc file."""
        pass

    def dynskel_defs(self, stream, visitor):
        """Definitions in the DynSK.cc file."""
        pass
