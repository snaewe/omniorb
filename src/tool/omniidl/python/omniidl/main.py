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
# Revision 1.8  1999/11/23 09:52:11  dpg1
# Dumb bug where maps weren't cleared between runs.
#
# Revision 1.7  1999/11/17 15:01:20  dpg1
# -Wb and -Wp now support multiple arguments separated by commas.
#
# Revision 1.6  1999/11/17 14:33:55  dpg1
# Clean up of usage info.
#
# Revision 1.5  1999/11/15 15:49:22  dpg1
# Documentation strings.
#
# Revision 1.4  1999/11/12 17:15:35  dpg1
# Verbose messages now written to stderr.
#
# Revision 1.3  1999/11/12 15:53:48  dpg1
# New functions omniORB.importIDL() and omniORB.importIDLString().
#
# Revision 1.2  1999/11/12 14:07:18  dpg1
# If back-end foo is not found in omniidl.be.foo, now tries to import
# foo.
#
# Revision 1.1  1999/11/08 11:43:34  dpg1
# Changes for NT support.
#

"""IDL Compiler front-end main function"""

import _omniidl
import sys, getopt, os, os.path, string

import idlast, idltype

def version():
    print "omniidl version 0.1"

cmdname = os.path.basename(sys.argv[0])

def usage():
    print "\nUsage:", cmdname, " [flags] -b<back_end> file1 file2 ..."
    print """
The supported flags are:

  -Dname[=value]  Define name for preprocessor
  -Uname          Undefine name for preprocessor
  -Idir           Include dir in search path for preprocessor
  -E              Run preprocessor only, print on stdout
  -Ycmd           Set command for the preprocessor
  -N              Do not run preprocessor
  -Wparg[,arg...] Send args to the preprocessor
  -bback_end      Select a back-end to be used. More than one permitted.
  -Wbarg[,arg...] Send args to the back-end
  -d              Dump the parsed IDL then exit
  -V              Print version info then exit
  -u              Print this usage message and exit
  -v              Trace compilation stages
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
quiet             = 0

def parseArgs(args):
    global preprocessor_args, preprocessor_only, preprocessor_cmd
    global no_preprocessor, backend, backend_args, dump_only
    global verbose, quiet

    try:
        opts,files = getopt.getopt(args, "D:I:U:EY:NW:b:dVuvq")
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
            if a[0] == "p":
              preprocessor_args.extend(string.split(a[1:], ","))
            elif a[0] == "b":
                if len(backends) == 0:
                    if not quiet:
                        sys.stderr.write("Error in arguments: "
                                         "no back-ends selected")
                    sys.exit(1)
                backends_args[-1].extend(string.split(a[1:], ","))
            else:
                if not quiet:
                    sys.stderr.write("Error in arguments: option " + o + \
                                     " not recognized\n")
                    sys.stderr.write("Use " + cmdname + " -u for usage\n")
                sys.exit(1)

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

        elif o == "-q":
            quiet = 1

    return files


def main(argv=None):
    global preprocessor_args, preprocessor_only, preprocessor_cmd
    global no_preprocessor, backend, backend_args, dump_only
    global verbose, quiet

    if argv is None: argv = sys.argv

    files = parseArgs(argv[1:])

    if len(files) == 0:
        if not quiet:
            sys.stderr.write(cmdname + ": No files specified\n")
        sys.exit(1)

    # Import back-ends, and add any pre-processor arguments
    bemodules = []
    for backend in backends:
        if verbose:
            sys.stderr.write(cmdname + ": Importing back-end `" +\
                             backend + "'\n")

        try:
            be = __import__("omniidl.be." + backend,
                            globals(), locals(), backend)
        except ImportError:
            try:
                be = __import__(backend, globals(), locals(), backend)
            except ImportError:
                if not quiet:
                    sys.stderr.write(cmdname + \
                                     ": Could not import back-end `" + \
                                     backend + "'\n")
                sys.exit(1)

        bemodules.append(be)
        if hasattr(be, "cpp_args"):
            preprocessor_args.extend(be.cpp_args)

    for file in files:
        if file != "-" and not os.path.isfile(file):
            if not quiet:
                sys.stderr.write(cmdname + ": `" + file + "' does not exist\n")
            sys.exit(1)

        preproc_cmd = preprocessor_cmd + " " + \
                      string.join(preprocessor_args, " ") + " " + file

        if not no_preprocessor:
            if verbose:
                sys.stderr.write(cmdname + ": Preprocessing `" +\
                                 file + "' with `" + preproc_cmd + "'\n")

            if preprocessor_only:
                err = os.system(preproc_cmd)
                if err:
                    if not quiet:
                        sys.stderr.write(cmdname + \
                                         ": Error running preprocessor\n")
                    sys.exit(1)
                sys.exit(0)

            file = os.popen(preproc_cmd, "r")
        else:
            file = open(file, "r")

        if verbose: sys.stderr.write(cmdname + ": Running front end\n")

        if dump_only:
            if verbose:
                sys.stderr.write(cmdname + ": Dumping\n")
            _omniidl.dump(file)
        else:
            tree = _omniidl.compile(file)

            if file.close():
                if not quiet:
                    sys.stderr.write(cmdname + \
                                     ": Error running preprocessor\n")
                sys.exit(1)

            if tree is None:
                sys.exit(1)

            i = 0
            for backend in backends:
                if verbose:
                    sys.stderr.write(cmdname + ": Running back-end `" +\
                                     backend + "'\n")

                bemodules[i].run(tree, backends_args[i])
                i = i + 1

            idlast.clear()
            idltype.clear()
