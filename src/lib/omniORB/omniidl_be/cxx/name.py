# -*- python -*-
#                           Package   : omniidl
# name.py                   Created on: 1999/11/10
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
#   Functions relating to naming and environments etc

# $Id$
# $Log$
# Revision 1.11  2000/07/13 15:26:01  dpg1
# Merge from omni3_develop for 3.0 release.
#
# Revision 1.8.2.3  2000/04/26 18:22:13  djs
# Rewrote type mapping code (now in types.py)
# Rewrote identifier handling code (now in id.py)
#
# Revision 1.8.2.2  2000/03/09 15:21:40  djs
# Better handling of internal compiler exceptions (eg attempts to use
# wide string types)
#
# Revision 1.8.2.1  2000/02/14 18:34:57  dpg1
# New omniidl merged in.
#
# Revision 1.8  2000/01/19 09:36:07  djs
# *** empty log message ***
#
# Revision 1.7  2000/01/11 12:02:34  djs
# More tidying up
#
# Revision 1.6  2000/01/10 15:39:34  djs
# Better name and scope handling.
#
# Revision 1.5  2000/01/07 20:31:17  djs
# Regression tests in CVSROOT/testsuite now pass for
#   * no backend arguments
#   * tie templates
#   * flattened tie templates
#   * TypeCode and Any generation
#
# Revision 1.4  1999/12/14 11:53:56  djs
# Support for CORBA::TypeCode and CORBA::Any
#
# Revision 1.3  1999/12/09 20:41:24  djs
# Now runs typecode and any generator
#
# Revision 1.2  1999/11/23 18:49:25  djs
# Lots of fixes, especially marshalling code
# Added todo list to keep track of issues
#
# Revision 1.1  1999/11/10 20:19:31  djs
# Option to emulate scope bug in old backend
# Array struct element fix
# Union sequence element fix
#

"""Functions relating to naming and environments etc"""

assert(0)

from omniidl import idltype, idlutil

from omniidl_be.cxx import tyutil, config


def prefixName(scopedName, prefix, env = None):
    if env == None:
        env = Environment()
    prefix_scopedName = tyutil.scope(scopedName) + [prefix +\
                                                    tyutil.name(scopedName)]
    return env.nameToString(prefix_scopedName)

def suffixName(scopedName, suffix, env = None):
    if env == None:
        env = Environment()
    suffix_scopedName = tyutil.scope(scopedName) + [tyutil.name(scopedName) +\
                                                    suffix]
    return env.nameToString(env.relName(suffix_scopedName))


class Environment:

    # default constructor makes everything blank
    def __init__(self):
        self.__scope = []
        self.__names = {}
        #self.__old_scope = None

    # effectively a copy constructor
    def copy(self):
        environment = Environment()
        environment.__scope = self.__scope[:]
        environment.__names = self.__names.copy()
        #environment.__old_scope = self.__old_scope
        return environment

    # called to enter a new scope
    def enterScope(self, name):
        environment = self.copy()
        environment.__scope.append(name)
        #environment.__old_scope = self
        return environment

    def leaveScope(self):
        # keep all the names we've defined
        environment = self.copy()
        scope = environment.__scope
        #print "scope = " + repr(scope)
        del scope[len(scope) - 1]
        return environment
        #self.__old_scope.__names = self.__names.copy()
        #return self.__old_scope

    # returns the current scope
    def scope(self):
        return self.__scope[:]

    # records a newly defined name in the environment
    def add(self, name):
        slashName = idlutil.slashName(self.__scope + [name])
        if self.__names.has_key(slashName):
            raise KeyError
        self.__names[slashName] = 1

    def __str__(self):
        s = "current scope = " + repr(self.__scope) +\
            "names defined = " + repr(self.__names)
        return s

    # partialNames consist of a list [A, B, C, D] and are searched
    # for in scope moving outward from the current scope.
    # a name of the form [None, A, B, C, D] is a globally scoped name
    # function returns the globally scoped name for an identifier if
    # available, else raises KeyError if the identifier is unknown
    def lookup(self, partialName):
        if partialName[0] == None:
            # name is globally scoped
            slashName = idlutil.slashName(partialName[1:len(partialName)])
            if self.__names.has_key(slashName):
                return partialName
            raise KeyError
        # else name must be relative
        scope = self.__scope[:]
        while len(scope) >= 0:
            scopedName = scope + partialName
            slashName = idlutil.slashName(scopedName)
            if self.__names.has_key(slashName):
                return [None] + scopedName
            if len(scope) == 0:
                break
            scope = scope[0:-1]
        raise KeyError

    # prune as much scope as possible from a fully scoped name
    # without clashing with another identifier
    #
    # Oddity:
    # Consider the following IDL:
    #    typedef float alias;
    #    module Inner {
    #       typedef char alias;
    #       typedef ::alias test;
    #    };
    #
    # The generated typedef for test is:
    #   typedef alias test;
    # and NOT
    #   typedef ::alias test;
    # surely this clashes with the innermost typedef?
    def relName(self, scopedName):
        # add the name to the environment anyway. Not the job of the
        # backend to find IDL name problems
        slashName = idlutil.slashName(scopedName)
        self.__names[slashName] = 1

        if scopedName == []:
            return [None]

        # search a list of possibilities:
        #  1. Use the innermost part of the name
        #  2. Try pruning off scope in common with the current environment
        #  3. Use the scoped name as is
        #  4. Use a rooted :: scoped name

        target = [None] + scopedName
        possibilities = [ [tyutil.name(scopedName)],
                          idlutil.pruneScope(scopedName, self.__scope),
                          scopedName,
                          [None] + scopedName ]
        for p in possibilities:
            try:
                if self.lookup(p) == target:
                    return p
            except KeyError:
                pass

        assert(0)

        # Check to see if the name is ambiguous
        try:
            if self.lookup([name]) == [None] + scopedName:
                return [name]
        except KeyError:
            pass

        prunedName = idlutil.pruneScope(scopedName, self.__scope)
        try:
            lookedup = self.lookup(prunedName)
            if lookedup == [None] + scopedName:
                return prunedName
        except KeyError:
            pass
        # Is the non-globally qualified name abiguous
        try:
            lookedup = self.lookup(scopedName)
            if self.lookup(scopedName) == [None] + scopedName:
                return scopedName
        except KeyError:
            pass
        # globally specify it
        return [None] + scopedName


        # ----------------
        assert(0)
        
        prunedName = idlutil.pruneScope(scopedName, self.__scope)
        # if we just pruned the whole name away, add a bit back
        prunedName = [scopedName[-1]]
        
        # Add an extra sentinel element (the other none is a global
        # scope marker)
        bitPruned = [None, None] +\
                    scopedName[0: (len(scopedName) - len(prunedName))]

        scopedName = [None] + scopedName

        # a scopedName such as [A, B, C, D] in a scope [A, B] leaves
        # bitPruned = [None, A, B] prunedName = [C, D]
        #print "scopedName = " + repr(scopedName)
        #print "names = " + repr(self.__names.keys())
        #print "prunedname = " + repr(prunedName)
        #print "bitpruned = " + repr(bitPruned)
        while len(bitPruned) > 0:
            try:
                #print "looking up " + repr(prunedName)
                lookedup = self.lookup(prunedName)
                #print "result = " + repr(lookedup)
                if lookedup == scopedName:
                    #print "returning " + repr(prunedName)
                    return prunedName
            except KeyError:
                pass
            # Add some of the pruned scope back and try again
            prunedName = [bitPruned[-1]] + prunedName
            del(bitPruned[len(bitPruned)-1])
        # name was not found at all
        # (should never happen)
        assert 0

    def nameToString(self, partialName):
        # map the None onto an empty string
        if partialName[0] == None:
            partialName = [""] + partialName[1:len(partialName)]

        # map each component onto a legal C++ identifier
        partialName = map(tyutil.mapID, partialName)
        
        return idlutil.ccolonName(partialName)

    def principalID(self, type):
        assert isinstance(type, idltype.Type)
        # check if type is a basic type first
        if tyutil.ttsMap.has_key(type.kind()):
            return tyutil.ttsMap[type.kind()]

        if isinstance(type, idltype.String):
            return "CORBA::String_member"
        if type.kind() == idltype.tk_TypeCode:
            return "CORBA::TypeCode_member"
        if isinstance(type, idltype.Sequence):
            return self.principalID(type.seqType())

        # ----- IMPLEMENT ME -----
        if isinstance(type, idltype.WString):
            util.fatalError("Wide-strings are not supported")
            raise "No code for wide strings"
        if isinstance(type, idltype.Fixed):
            util.fatalError("Fixed types are not supported")
            raise "No code for Fixed type"
        # ----- IMPLEMENT ME -----

        scopedName = type.scopedName()
        relName = self.relName(scopedName)
        return self.nameToString(relName)


# just don't write to it
def globalScope():
    return Environment()

        
def scopetest():
    a = Environment()
    a.add("name")
    a.add("name2")
    b = a.enterScope("module")
    b.add("name")
    c = b.enterScope("inner")
    c.add("name")

    d = c.leaveScope()
    e = d.leaveScope()

    print "BUG EMULATION is ",
    if config.EMULATE_BUGS():
        print "ON"
    else:
        print "OFF"
    for env in [c, d, e]:
        scope = repr(env.scope())
        
        for name in [ ["name"],
                      ["module", "name"],
                      ["module", "inner", "name"] ]:
            print "scope = " + scope + " \t fullname = " + repr(name) + \
                  " \t C++ name = " + repr(env.nameToString(env.relName(name)))


