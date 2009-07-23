#!/usr/bin/env python

import sys
from omniORB import CORBA, PortableServer

# Import the stubs for the Naming service
import CosNaming

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

# Obtain a reference to the root naming context
obj         = orb.resolve_initial_references("NameService")
rootContext = obj._narrow(CosNaming.NamingContext)

if rootContext is None:
    print "Failed to narrow the root naming context"
    sys.exit(1)

# Bind a context named "test.my_context" to the root context
name = [CosNaming.NameComponent("test", "my_context")]

try:
    testContext = rootContext.bind_new_context(name)
    print "New test context bound"
    
except CosNaming.NamingContext.AlreadyBound, ex:
    print "Test context already exists"
    obj = rootContext.resolve(name)
    testContext = obj._narrow(CosNaming.NamingContext)
    if testContext is None:
        print "test.mycontext exists but is not a NamingContext"
        sys.exit(1)

# Bind the Echo object to the test context
name = [CosNaming.NameComponent("ExampleEcho", "Object")]

try:
    testContext.bind(name, eo)
    print "New ExampleEcho object bound"

except CosNaming.NamingContext.AlreadyBound:
    testContext.rebind(name, eo)
    print "ExampleEcho binding already existed -- rebound"

    # Note that is should be sufficient to just call rebind() without
    # calling bind() first. Some Naming service implementations
    # incorrectly raise NotFound if rebind() is called for an unknown
    # name, so we use the two-stage approach above

# Activate the POA
poaManager = poa._get_the_POAManager()
poaManager.activate()

# Everything is running now, but if this thread drops out of the end
# of the file, the process will exit. orb.run() just blocks until the
# ORB is shut down
orb.run()
