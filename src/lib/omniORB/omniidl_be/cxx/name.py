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
# Revision 1.1  1999/11/10 20:19:31  djs
# Option to emulate scope bug in old backend
# Array struct element fix
# Union sequence element fix
#

"""Functions relating to naming and environments etc"""

from omniidl import idltype, idlutil

from omniidl.be.cxx import tyutil, config


class Environment:

    # default constructor makes everything blank
    def __init__(self):
        self.__scope = []
        self.__names = {}

    # effectively a copy constructor
    def copy(self):
        environment = Environment()
        environment.__scope = self.__scope[:]
        environment.__names = self.__names.copy()
        return environment

    # called to enter a new scope
    def enterScope(self, name):
        environment = self.copy()
        environment.__scope.append(name)
        environment.__old_scope = self
        return environment

    def leaveScope(self):
        # keep all the names we've defined
        self.__old_scope.__names = self.__names.copy()
        return self.__old_scope

    # returns the current scope
    def scope(self):
        return self.__scope[:]

    # records a newly defined name in the environment
    def add(self, name):
        slashName = idlutil.slashName(self.__scope + [name])
        if self.__names.has_key(slashName):
            raise KeyError
        self.__names[slashName] = 1

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
        
        prunedName = idlutil.pruneScope(scopedName, self.__scope)
        # Add an extra sentinel element (the other none is a global
        # scope marker)
        bitPruned = [None, None] +\
                    scopedName[0: (len(scopedName) - len(prunedName))]

        scopedName = [None] + scopedName

        # a scopedName such as [A, B, C, D] in a scope [A, B] leaves
        # bitPruned = [None, A, B] prunedName = [C, D]

        #print "names = " + repr(self.__names.keys())
        #print "prunedname = " + repr(prunedName)
        #print "bitpruned = " + repr(bitPruned)
        while len(bitPruned) > 0:
            try:
                #print "looking up " + repr(prunedName)
                lookedup = self.lookup(prunedName)
                #print "result = " + repr(lookedup)
                if lookedup == scopedName:
                    #print "returning"
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
        # a bug in the old backend (as described above) means
        # that names are never globally scoped (::alias)
        if config.EMULATE_BUGS() and partialName[0] == None:
            partialName = partialName[1:len(partialName)]
        # map the None onto an empty string
        if partialName[0] == None:
            partialName = [""] + partialName[1:len(partialName)]

        # map each component onto a legal C++ identifier
        partialName = map(tyutil.mapID, partialName)
        
        return idlutil.ccolonName(partialName)

    def principalID(self, type, fully_scope = 0):
        assert isinstance(type, idltype.Type)
        # check if type is a basic type first
        if tyutil.ttsMap.has_key(type.kind()):
            return tyutil.ttsMap[type.kind()]

        if isinstance(type, idltype.String):
            return "CORBA::String_member"
        if isinstance(type, idltype.Sequence):
            return self.principalID(type.seqType())

        # ----- IMPLEMENT ME -----
        if isinstance(type, idltype.WString):
            raise "No code for wide strings"
        if isinstance(type, idltype.Fixed):
            raise "No code for Fixed type"
        # ----- IMPLEMENT ME -----

        scopedName = type.scopedName()
        if fully_scope:
            if config.EMULATE_BUGS():
                relName = scopedName
            else:
                relName = [None] + scopedName
        else:
            relName = self.relName(scopedName)
        return self.nameToString(relName)
               
        
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

