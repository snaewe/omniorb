# -*- python -*-
#                           Package   : omniidl
# __init__.py               Created on: 2000/08
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
#   Initialise the AMI module

# Once __init__ has been called, AMI code will automagically be output
# along with the normal stubs.

from omniidl_be.cxx.ami import main, ami


def __init__(tree):
    # Register all the extra IDL we need
    new_tree = ami.augment(tree)

    # Initialse the hooks
    main.__init_hooks__()

    return new_tree



