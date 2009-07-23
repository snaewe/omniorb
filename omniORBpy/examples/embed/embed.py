#!/usr/bin/env python

# embed.py -- Python part of embedding example

import sys
import CORBA, PortableServer
import _GlobalIDL, _GlobalIDL__POA
import _embed

# Define an implementation of the Echo interface
class Echo_i (_GlobalIDL__POA.Echo):
    def echoString(self, mesg):
        print "Python upcall '" + mesg + "'"
        return mesg

# Initialise the ORB
orb = CORBA.ORB_init(sys.argv, CORBA.ORB_ID)

# Find the root POA
poa = orb.resolve_initial_references("RootPOA")

# Activate the POA
poaManager = poa._get_the_POAManager()
poaManager.activate()


# Test 1: Python calls C++

obj = _embed.getObjRef(orb)
eo  = obj._narrow(_GlobalIDL.Echo)

if eo is None:
    print "Failed to narrow Echo object:", ior
    sys.exit(1)

print "\nPython calling C++..."
ret = eo.echoString("Hello from Python")
print "The object said '" + ret + "'"

print "\n"


# Test 2: C++ calls Python

ei  = Echo_i()
eo  = ei._this()
ior = orb.object_to_string(eo)

print "\nC++ calling Python..."

_embed.putObjRef(eo)

print "\nTest done"
