#!/usr/bin/env python
# -*- python -*-
#                           Package   : omniidl
# main.py                   Created on: 1999/11/05
#			    Author    : Duncan Grisby (dpg1)
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
#   IDL compiler main function

# $Id$
# $Log$
# Revision 1.2  1999/11/12 14:07:18  dpg1
# If back-end foo is not found in omniidl.be.foo, now tries to import
# foo.
#
# Revision 1.1  1999/11/08 11:43:34  dpg1
# Changes for NT support.
#

import _omniidl
import sys, getopt, os, os.path, string

def version():
    print "omniidl version 0.1"

cmdname = os.path.basename(sys.argv[0])

def usage():
    print "\nUsage:", cmdname, " [flags] -b<back_end> file1 file2 ..."
    print """
The supported flags are:

  -Dname[=value]  Defines name for preprocessor
  -Idir           Includes dir in search path for preprocessor
  -Uname          Undefines name for preprocessor
  -E              Runs preprocessor only, prints of stdout
  -Ycmd           Sets command for the preprocessor
  -N              Does not run pre-processor
  -Wparg          Sends arg to the preprocessor
  -bback_end      Selects the back-end to be used. More than one permitted.
  -Wbarg          Sends arg to the back-end
  -d              Dumps the syntax tree then exits
  -V              Prints version info then exits
  -u              Prints usage message and exits
  -v              Traces compilation stages
"""

preprocessor_args = []
preprocessor_only = 0

if hasattr(_omniidl, "__file__"):
    preprocessor_path = os.path.dirname(_omniidl.__file__)
else:
    preprocessor_path = os.path.dirname(sys.argv[0])

preprocessor      = os.path.join(preprocessor_path, "omni-cpp")
preprocessor_cmd  = preprocessor + " -lang-c++ -undef -D__OMNIIDL__=" + \
                    _omniidl.version

no_preprocessor   = 0
backends          = []
backends_args     = []
dump_only         = 0
verbose           = 0


def parseArgs(args):
    global preprocessor_args, preprocessor_only, preprocessor_cmd
    global no_preprocessor, backend, backend_args, dump_only, verbose

    try:
        opts,files = getopt.getopt(args, "D:I:U:EY:NW:b:dVuv")
    except getopt.error, e:
        sys.stderr.write("Error in arguments: " + e + "\n")
        sys.stderr.write("Use " + cmdname + " -u for usage\n")
        sys.exit(1)

    for opt in opts:
        o,a = opt

        if o == "-D":
            preprocessor_args.append("-D" + a)

        elif o == "-I":
            preprocessor_args.append("-I" + a)

        elif o == "-U":
            preprocessor_args.append("-U" + a)

        elif o == "-E":
            preprocessor_only = 1

        elif o == "-Y":
            preprocessor_cmd = a

        elif o == "-N":
            no_preprocessor = 1

        elif o == "-W":
            if   a[0] == "p": preprocessor_args.append(a[1:])
            elif a[0] == "b":
                if len(backends) == 0:
                    sys.stderr.write("Error in arguments: "
                                     "no back-ends selected")
                    sys.exit(1)
                backends_args[-1].append(a[1:])
            else:
                sys.stderr.write("Error in arguments: option " + o + \
                                 " not recognized\n")
                sys.stderr.write("Use " + cmdname + " -u for usage\n")

        elif o == "-b":
            backends.append(a)
            backends_args.append([])

        elif o == "-d":
            dump_only = 1

        elif o == "-V":
            version()
            sys.exit(0)

        elif o == "-u":
            usage()
            sys.exit(0)

        elif o == "-v":
            verbose = verbose + 1

    return files


def main(argv=None):
    global preprocessor_args, preprocessor_only, preprocessor_cmd
    global no_preprocessor, backend, backend_args, dump_only, verbose

    if argv is None: argv = sys.argv

    files = parseArgs(argv[1:])

    if len(files) == 0:
        sys.stderr.write(cmdname + ": No files specified\n")
        sys.exit(1)

    # Import back-ends, and add any pre-processor arguments
    bemodules = []
    for backend in backends:
        if verbose:
            print cmdname + ": Importing back-end `" + backend + "'"

        try:
            be = __import__("omniidl.be." + backend,
                            globals(), locals(), backend)
        except ImportError:
            try:
                be = __import__(backend, globals(), locals(), backend)
            except ImportError:
                sys.stderr.write(cmdname + \
                                 ": Could not import back-end `" + \
                                 backend + "'\n")
                sys.exit(1)

        bemodules.append(be)
        if hasattr(be, "cpp_args"):
            preprocessor_args.extend(be.cpp_args)

    for file in files:
        if not os.path.isfile(file):
            sys.stderr.write(cmdname + ": `" + file + "' does not exist\n")
            break

        preproc_cmd = preprocessor_cmd + " " + \
                      string.join(preprocessor_args, " ") + " " + file

        if not no_preprocessor:
            if verbose:
                print cmdname + ": Preprocessing `" + file + "' with `" + \
                      preproc_cmd + "'"

            if preprocessor_only:
                err = os.system(preproc_cmd)
                if err:
                    sys.stderr.write(cmdname + \
                                     ": Error running preprocessor\n")
                    sys.exit(1)
                sys.exit(0)

            file = os.popen(preproc_cmd, "r")
        else:
            file = open(file, "r")

        if verbose: print cmdname + ": Running front end"

        if dump_only:
            if verbose:
                print cmdname + ": Dumping"
            _omniidl.dump(file)
        else:
            tree = _omniidl.compile(file)

            if file.close():
                sys.stderr.write(cmdname + ": Error running preprocessor\n")
                sys.exit(1)

            if tree is None:
                sys.exit(1)

            i = 0
            for backend in backends:
                if verbose:
                    print cmdname + ": Running back-end `" + backend + "'"

                bemodules[i].run(tree, backends_args[i])
                i = i + 1
