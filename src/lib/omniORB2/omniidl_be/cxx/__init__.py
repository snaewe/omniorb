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
#   Entrypoint to the C++ backend

# $Id$
# $Log$
# Revision 1.4  1999/11/08 19:28:56  djs
# Rewrite of sequence template code
# Fixed lots of typedef problems
#
# Revision 1.3  1999/11/04 19:05:01  djs
# Finished moving code from tmp_omniidl. Regression tests ok.
#
# Revision 1.2  1999/11/03 17:35:06  djs
# Brought more of the old tmp_omniidl code into the new tree
#
# Revision 1.1  1999/11/03 11:09:49  djs
# General module renaming
#


# -----------------------------
# Output generation functions
from omniidl.be.cxx import header

import re

cpp_args = ["-D__OMNIIDL_CXX__"]

def run(tree, args):
    """Entrypoint to the C++ backend"""

#    print "C++ Backend called with tree = " + repr(tree)
#    print "                   and  args = " + repr(args)
#    print "Better do something!"

#    print "I know, I'll generate the header."


    filename = tree.file()
    regex = re.compile(r"\.idl")
    if regex.search(filename):
        chopped = regex.sub("", filename)
        config.setBasename(chopped)
            
    header.run(tree)

    #stream = util.Stream(sys.stdout, 2)

    #defs = header.defs.__init__(stream)
    #tree.accept(defs)
    
    
    #global main_idl_file, imported_files
    
    #main_idl_file = tree.file()

    #imported_files[outputFileName(main_idl_file)] = 1

    #checkStubDir(stub_directory)

    #outpybasename = outputFileName(main_idl_file)
    #outpymodule   = stub_module + outpybasename
    #outpyname     = os.path.join(stub_directory, outpybasename + ".py")


    #st.out(file_start, filename=main_idl_file)

    #pv = PythonVisitor(st, outpymodule)
    #tree.accept(pv)

    #st.out(file_end)

    #updateModules(exported_modules.keys(), outpymodule)
