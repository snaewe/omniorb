#!/usr/bin/env python

# Single threaded POA.

import sys, time
import CORBA, PortableServer

import _GlobalIDL, _GlobalIDL__POA

class Echo_i (_GlobalIDL__POA.Echo):
    def echoString(self, mesg):
        print "echoString() called with message:", mesg
        if mesg == "re-enter":
            eo.echoString("re-entrant call")
        elif mesg == "shutdown":
            orb.shutdown(0)
        return mesg

# Initialise the ORB and activate the root POA.
orb = CORBA.ORB_init(sys.argv, CORBA.ORB_ID)
poa = orb.resolve_initial_references("RootPOA")
poaManager = poa._get_the_POAManager()
poaManager.activate()

# Create a single-threaded child POA
ps = [ poa.create_thread_policy(PortableServer.SINGLE_THREAD_MODEL) ]

child = poa.create_POA("MyPOA", poaManager, ps)

ei = Echo_i()
child.activate_object(ei)
eo = ei._this()

print orb.object_to_string(eo)

orb.run()
