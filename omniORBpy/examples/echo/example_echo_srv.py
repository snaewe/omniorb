#!/usr/bin/env python

import sys
from omniORB import CORBA, PortableServer

# Import the stubs and skeletons for the Example module
import Example, Example__POA

# Define an implementation of the Echo interface
class Echo_i (Example__POA.Echo):
    def echoString(self, mesg):
        print "echoString() called with message:", mesg
        return mesg

# Initialise the ORB
orb = CORBA.ORB_init(sys.argv, CORBA.ORB_ID)

# Find the root POA
poa = orb.resolve_initial_references("RootPOA")

# Create an instance of Echo_i
ei = Echo_i()

# Create an object reference, and implicitly activate the object
eo = ei._this()

# Print out the IOR
print orb.object_to_string(eo)

# Activate the POA
poaManager = poa._get_the_POAManager()
poaManager.activate()

# Everything is running now, but if this thread drops out of the end
# of the file, the process will exit. orb.run() just blocks until the
# ORB is shut down
orb.run()
