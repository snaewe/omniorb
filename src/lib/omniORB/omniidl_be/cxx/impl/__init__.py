# -*- python -*-
#                           Package   : omniidl
# __init__.py               Created on: 2000/02/03
#			    Author    : David Scott (djs)
#
#    Copyright (C) 2000 AT&T Laboratories Cambridge
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
#   Entrypoint to example implementation generation code

# $Id$
# $Log$
# Revision 1.2.2.2  2003/01/22 12:10:55  dgrisby
# Explicitly close files in C++ backend.
#
# Revision 1.2.2.1  2000/10/12 15:37:52  sll
# Updated from omni3_1_develop.
#
# Revision 1.2.4.1  2000/08/21 11:35:22  djs
# Lots of tidying
#

import os

from omniidl_be.cxx import config, util, output
from omniidl_be.cxx.impl import main

def run(tree):
    hh_filename = config.state['Basename'] + config.state['HH Suffix']
    idl_filename = tree.file()
    impl_filename = config.state['Basename'] + config.state['IMPL Suffix']

    stream = output.Stream(output.createFile(impl_filename), 2)
    main.__init__(stream, idl_filename, hh_filename)

    main.run(tree)

    stream.close()
