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
# Revision 1.15.2.14  2000/07/06 09:40:44  dpg1
# Spelling mistake :-(
#
# Revision 1.15.2.13  2000/07/06 09:30:20  dpg1
# Clarify omniidl usage in documentation and omniidl -u
#
# Revision 1.15.2.12  2000/06/27 16:23:26  sll
# Merged OpenVMS port.
#
# Revision 1.15.2.11  2000/06/20 13:55:58  dpg1
# omniidl now keeps the C++ tree until after the back-ends have run.
# This means that back-ends can be C++ extension modules.
#
# Revision 1.15.2.10  2000/06/05 18:13:28  dpg1
# Comments can be attached to subsequent declarations (with -K). Better
# idea of most recent decl in operation declarations
#
# Revision 1.15.2.9  2000/03/17 16:28:46  dpg1
# Small improvement to error reporting.
#
# Revision 1.15.2.8  2000/03/16 11:09:30  dpg1
# Better error reporting if import fails.
#
# Revision 1.15.2.7  2000/03/14 14:54:17  dpg1
# Incorrect message with omniidl -E.
#
# Revision 1.15.2.6  2000/03/06 15:03:45  dpg1
# Minor bug fixes to omniidl. New -nf and -k flags.
#
# Revision 1.15.2.5  2000/03/02 11:22:46  dpg1
# omniidl.py renamed omniidlrun.py to avoid problems with import
# omniidl.main.
#
# Revision 1.15.2.4  2000/02/22 16:44:50  dpg1
# Cosmetic change.
#
# Revision 1.15.2.3  2000/02/17 11:18:05  dpg1
# Multiple -p flags now preserve the order, rather than reversing it.
#
# Revision 1.15.2.2  2000/02/16 16:23:52  dpg1
# Support things for Python neophytes.
#
# Revision 1.15.2.1  2000/02/14 18:34:52  dpg1
# New omniidl merged in.
#
# Revision 1.15  2000/02/14 12:49:57  dpg1
# Python version check. New -p option.
#
# Revision 1.14  2000/02/04 12:17:05  dpg1
# Support for VMS.
#
# Revision 1.13  2000/01/18 17:15:05  dpg1
# Changes for "small" distribution.
#
# Revision 1.12  1999/12/21 16:03:57  dpg1
# Warning if no back-ends are selected.
#
# Revision 1.11  1999/12/17 11:39:23  dpg1
# Cosmetic change
#
# Revision 1.10  1999/11/30 10:41:20  dpg1
# Back-ends can now have their own usage string.
#
# Revision 1.9  1999/11/23 16:59:22  dpg1
# New command line option -C to send output to a different directory.
#
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

import sys

if sys.version[:6] != "1.5.2 ":
    sys.stderr.write("\n\n")
    sys.stderr.write("omniidl: WARNING!!\n\n")
    sys.stderr.write("omniidl: Python version 1.5.2 is required.\n")
    sys.stderr.write("omniidl: " + sys.executable + " is version " + \
                     sys.version + "\n")
    sys.stderr.write("omniidl: Execution is likely to fail.\n")
    sys.stderr.write("\n\n\n")
    sys.stderr.flush()

import _omniidl
import getopt, os, os.path, string

import idlast, idltype

cmdname = "omniidl"

def version():
    print "omniidl version 1.0"

def usage():
    global backends
    print "\nUsage:", cmdname, "-b<back_end> [flags] file1 file2 ..."
    print """
The supported flags are:

  -Dname[=value]  Define name for preprocessor
  -Uname          Undefine name for preprocessor
  -Idir           Include dir in search path for preprocessor
  -E              Run preprocessor only, print on stdout
  -Ycmd           Set command for the preprocessor
  -N              Do not run preprocessor
  -Wparg[,arg...] Send args to the preprocessor
  -bback_end      Select a back-end to be used. More than one permitted
  -Wbarg[,arg...] Send args to the back-end
  -nf             Do not warn about unresolved forward declarations
  -k              Comments after declarations are kept for the back-ends
  -K              Comments before declarations are kept for the back-ends
  -Cdir           Change directory to dir before writing output
  -d              Dump the parsed IDL then exit
  -pdir           Path to omniidl back-ends ($TOP/lib/python)
  -V              Print version info then exit
  -u              Print this usage message and exit
  -v              Trace compilation stages"""

    if backends == []:
        print """
You must select a target back-end with -b. For C++, use -bcxx:

  omniidl -bcxx file.idl

To see options specific to C++, use:

  omniidl -bcxx -u
"""

preprocessor_args = []
preprocessor_only = 0

if hasattr(_omniidl, "__file__"):
    preprocessor_path = os.path.dirname(_omniidl.__file__)
else:
    preprocessor_path = os.path.dirname(sys.argv[0])

if sys.platform != "OpenVMS":
    preprocessor      = os.path.join(preprocessor_path, "omnicpp")
    preprocessor_cmd  = preprocessor + " -lang-c++ -undef -D__OMNIIDL__=" + \
			_omniidl.version
else:    
    names = string.split(preprocessor_path, "/")
    preprocessor_cmd = \
 	'''mcr %s:[%s]omnicpp -lang-c++ -undef "-D__OMNIIDL__=%s"'''\
 	% (names[1], string.join(names[2:],"."), _omniidl.version)

no_preprocessor   = 0
backends          = []
backends_args     = []
dump_only         = 0
cd_to             = None
verbose           = 0
quiet             = 0
print_usage       = 0

def parseArgs(args):
    global preprocessor_args, preprocessor_only, preprocessor_cmd
    global no_preprocessor, backend, backend_args, dump_only, cd_to
    global verbose, quiet, print_usage

    paths = []

    try:
        opts,files = getopt.getopt(args, "D:I:U:EY:NW:b:n:kKC:dVuhvqp:")
    except getopt.error, e:
        sys.stderr.write("Error in arguments: " + e + "\n")
        sys.stderr.write("Use `" + cmdname + " -u' for usage\n")
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
                        sys.stderr.write(cmdname + ": Error in arguments: "
                                         "no back-ends selected\n")
                    sys.exit(1)
                backends_args[-1].extend(string.split(a[1:], ","))
            else:
                if not quiet:
                    sys.stderr.write("Error in arguments: option " + o + \
                                     " not recognized\n")
                    sys.stderr.write("Use " + cmdname + " -u for usage\n")
                sys.exit(1)

        elif o == "-C":
            if not os.path.isdir(a):
                if not quiet:
                    sys.stderr.write(cmdname + ": `" + a + \
                                     "' is not a directory\n")
                sys.exit(1)
            cd_to = a

        elif o == "-b":
            backends.append(a)
            backends_args.append([])

        elif o == "-n":
            if a == "f":
                _omniidl.noForwardWarning()
            else:
                if not quiet:
                    sys.stderr.write(cmdname + ": unknown warning option `" + \
                                     a + "'\n")
                    sys.stderr.write("Use " + cmdname + " -u for usage\n")
                sys.exit(1)

        elif o == "-k":
            preprocessor_args.append("-C")
            _omniidl.keepComments(0)

        elif o == "-K":
            preprocessor_args.append("-C")
            _omniidl.keepComments(1)

        elif o == "-d":
            dump_only = 1

        elif o == "-V":
            version()
            sys.exit(0)

        elif o == "-u" or o == "-h":
            print_usage = 1

        elif o == "-v":
            verbose = verbose + 1

        elif o == "-q":
            quiet = 1
            _omniidl.quiet()

        elif o == "-p":
            paths.append(a)

    sys.path = paths + sys.path

    return files


def my_import(name):
    mod = __import__(name)
    components = string.split(name, ".")
    for comp in components[1:]:
        mod = getattr(mod, comp)
    return mod

def be_import(name):
    try:
        return my_import("omniidl_be." + name)
    except ImportError, ex:
        if sys.modules.has_key("omniidl_be." + name):
            # The first import found the module, but some nested
            # import failed.
            raise ex

        return my_import(name)

def main(argv=None):
    global preprocessor_args, preprocessor_only, preprocessor_cmd
    global no_preprocessor, backend, backend_args, dump_only, cd_to
    global verbose, quiet, print_usage

    if argv is None: argv = sys.argv

    files = parseArgs(argv[1:])

    if print_usage:
        usage()        

    elif len(files) == 0:
        if not quiet:
            sys.stderr.write(cmdname + ": No files specified. Use `" \
                             + cmdname + " -u' for usage.\n")
        sys.exit(1)

    # Import back-ends, and add any pre-processor arguments
    bemodules = []
    for backend in backends:
        if verbose:
            sys.stderr.write(cmdname + ": Importing back-end `" +\
                             backend + "'\n")
        try:
            be = be_import(backend)
        except ImportError, msg:
            if not quiet:
                sys.stderr.write(cmdname + \
                                 ": Could not import back-end `" + \
                                 backend + "'\n")
                sys.stderr.write(cmdname + \
                                 ": Maybe you need to use the -p option?\n")
                sys.stderr.write(cmdname + \
                                 ": (The error was `" + str(msg) + "')\n")
            sys.exit(1)

        if verbose:
            if hasattr(be, "__file__"):
                sys.stderr.write(cmdname + ": `" + backend + \
                                 "' imported from `" + be.__file__ + "'\n")
            else:
                sys.stderr.write(cmdname + ": `" + backend + \
                                 "' imported from unknown file\n")

        bemodules.append(be)
        if hasattr(be, "cpp_args"):
            preprocessor_args.extend(be.cpp_args)

        if print_usage and hasattr(be, "usage_string"):
            print "\nArguments for back-end `" + backend + "':\n"
            print be.usage_string

    if print_usage:
        sys.exit(0)

    if len(backends) == 0 and not (quiet or dump_only or preprocessor_only):
        sys.stderr.write(cmdname + ": Warning: No back-ends specified; " \
                         "checking IDL for validity\n")

    for file in files:
        if file != "-" and not os.path.isfile(file):
            if not quiet:
                sys.stderr.write(cmdname + ": `" + file + "' does not exist\n")
            sys.exit(1)

 	if sys.platform != 'OpenVMS' or len(preprocessor_args)==0:
 	    preproc_cmd = preprocessor_cmd + " " + \
 			  string.join(preprocessor_args, " ") + " " + file
 	else:
 	    preproc_cmd = preprocessor_cmd + ' "' + \
                       string.join(preprocessor_args, '" "') + '" ' + file

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

            if cd_to is not None:
                old_wd = os.getcwd()
                os.chdir(cd_to)
            
            i = 0
            for backend in backends:
                if verbose:
                    sys.stderr.write(cmdname + ": Running back-end `" +\
                                     backend + "'\n")

                bemodules[i].run(tree, backends_args[i])
                i = i + 1

            if cd_to is not None:
                os.chdir(old_wd)

            idlast.clear()
            idltype.clear()
            _omniidl.clear()
