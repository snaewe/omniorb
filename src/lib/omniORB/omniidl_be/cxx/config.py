 # -*- python -*-
#                           Package   : omniidl
# config.py                 Created on: 2000/10/8
#                           Author    : David Scott (djs)
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
#   Global state of the C++ backend is stored here
#
# $Id$
# $Log$
# Revision 1.14.2.7  2004/10/18 02:44:37  dgrisby
# New -Wbdll_includes option.
#
# Revision 1.14.2.6  2002/09/03 09:32:57  dgrisby
# C++ back-end bails out if asked to compile more than one file.
#
# Revision 1.14.2.5  2001/11/08 16:33:51  dpg1
# Local servant POA shortcut policy.
#
# Revision 1.14.2.4  2001/06/08 17:12:11  dpg1
# Merge all the bug fixes from omni3_develop.
#
# Revision 1.14.2.3  2000/11/03 19:25:42  sll
# Updated library name to omniORB4.
#
# Revision 1.14.2.2  2000/10/12 15:37:46  sll
# Updated from omni3_1_develop.
#
# Revision 1.15.2.2  2000/08/21 11:34:32  djs
# Lots of omniidl/C++ backend changes
#

import string

import config

# Stores the global configuration of the C++ backend and can dump it to
# stdout (for bug reporting)
class ConfigurationState:

    def __init__(self):
        self._config = {
            # Name of this program
            'Program Name':          'omniidl (C++ backend)',
            # Useful data from CVS
            'CVS ID':                '$Id$',
            # Relevant omniORB C++ library version
            'Library Version':       'omniORB_4_0',
            # Suffix of generated header file
            'HH Suffix':             '.hh',
            # Suffix of generated Skeleton file
            'SK Suffix':             'SK.cc',
            # Suffix of generated Dynamic code
            'DYNSK Suffix':          'DynSK.cc',
            # Suffix of example interface implementation code
            'IMPL Suffix':           '_i.cc',
            
            # Are we in "fragment" mode?
            'Fragment':              0,
            # In fragment mode, suffix of definitions file
            '_DEFS Fragment':        '_defs',
            # In fragment mode, suffix of file containing operators (eg <<)
            '_OPERS Fragment':       '_operators',
            # In fragment mode, suffix of file containing POA code
            '_POA Fragment':         '_poa',

            # Private prefix for internal names
            'Private Prefix':        '_0RL',
            # Prefix used to avoid clashes with C++ keywords
            'Reserved Prefix':       '_cxx_',
            # Base name of file being processed
            'Basename':              None,
            # Directory name of file being processed
            'Directory':             None,
            # Do we generate code for TypeCodes and Any?
            'Typecode':              0,
            # Do we splice reopened modules together into one large chunk?
            # (not guaranteed to always work)
            'Splice Modules':        0,
            # Do we generate example code implementing all of the interfaces
            # found in the input IDL?
            'Example Code':          0,
            # Do we generate normal (non-flattened) tie templates?
            'Normal Tie':            0,
            # Do we generate flattened tie templates?
            'Flattened Tie':         0,
            # Do we generate BOA compatible skeleton classes?
            'BOA Skeletons':         0,
            # Do we generate old CORBA 2.1 signatures for skeletons?
            'Old Signatures':        0,
            # Do we preserve the #include'd IDL path name in the generated
            # header (eg #include <A/B.idl> -> #include <A/B.hh>?
            'Keep Include Path':     0,
            # Do we #include files using double-quotes rather than
            # angled brackets (the default)
            'Use Quotes':            0,
            
            # Output AMI ReplyHandlers, Pollers etc
            #'AMI':                   0,
            # Not ported yet.
            
            # Do we make all the objref methods virtual
            'Virtual Objref Methods':0,
            # Are #included files output inline with the main output?
            'Inline Includes':       0,

            # Generate local servant shortcut code?
            'Shortcut':              0,

            # Extra ifdefs for stubs in dlls?
            'DLLIncludes':           0,

            # Are we in DEBUG mode?
            'Debug':                 0
                       
            }

    def __getitem__(self, key):
        if self._config.has_key(key):
            return self._config[key]
        util.fatalError("Configuration key not found (" + key + ")")

    def __setitem__(self, key, value):
        if self._config.has_key(key):
            self._config[key] = value
            return
        util.fatalError("Configuration key not found (" + key + ")")

    def dump(self):
        # find the longest key string
        max = 0
        for key in self._config.keys():
            if len(key) > max: max = len(key)
        # display the table
        for key in self._config.keys():
            print string.ljust(key, max), ":  ", repr(self._config[key])

# Create state-holding singleton object
if not hasattr(config, "state"):
    config.state = ConfigurationState()
