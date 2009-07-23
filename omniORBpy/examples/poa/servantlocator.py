#!/usr/bin/env python

# Example of using a ServantLocator.
#

# This program behaves just like the echo server, except that the
# servant object is activated and deactivated again on each request to
# the object.
#
# If you run with a -l command line argument, the program will make a
# number of local calls to the object, showing that the activator runs
# in the local, as well as the remote, case.

import sys, time
from omniORB import CORBA, PortableServer

import _GlobalIDL, _GlobalIDL__POA

class Echo_i (_GlobalIDL__POA.Echo):
    def __init__(self):
        print "Echo_i created."

    def __del__(self):
        print "Echo_i deleted."

    def echoString(self, mesg):
        print "echoString() called with message:", mesg, \
              ", ObjectId =", repr(current.get_object_id())
        return mesg

class ServantLocator_i (PortableServer.ServantLocator):
    def preinvoke(self, oid, poa, operation):
        print "preinvoke(): oid:", oid, "poa:", poa._get_the_name()
        if oid == "MyEcho2":
            raise PortableServer.ForwardRequest(rooteo)

        ei = Echo_i()
        return (ei, "Hmm, cookies")

    def postinvoke(self, oid, poa, operation, cookie, serv):
        print "postinvoke(): oid:", oid, "poa:", poa._get_the_name(), \
              "cookie: '" + cookie + "'"

# Initialise the ORB and activate the root POA.
orb = CORBA.ORB_init(sys.argv, CORBA.ORB_ID)
poa = orb.resolve_initial_references("RootPOA")
poaManager = poa._get_the_POAManager()
poaManager.activate()

current = orb.resolve_initial_references("POACurrent")

# Create a child POA with the right policies for a ServantLocator
ps = [poa.create_id_assignment_policy(PortableServer.USER_ID),
      poa.create_servant_retention_policy(PortableServer.NON_RETAIN),
      poa.create_request_processing_policy(PortableServer.USE_SERVANT_MANAGER)]

child = poa.create_POA("MyPOA", poaManager, ps)

# Create the ServantLocator and set it as the child's ServantManager
sli = ServantLocator_i()
child.set_servant_manager(sli)

rooteo = Echo_i()._this()

# Create an object reference with no servant
eo = child.create_reference_with_id("MyEcho", CORBA.id(_GlobalIDL.Echo))
eo2 = child.create_reference_with_id("MyEcho2", CORBA.id(_GlobalIDL.Echo))
print orb.object_to_string(eo)

# Run, or do some local calls...
if not (len(sys.argv) > 1 and sys.argv[1] == "-l"):
    orb.run()

time.sleep(1)

print "Calling..."

# On each of these calls, the servant is created just before the
# invocation, and deleted again just afterwards

print eo.echoString("Hello from same address space")
time.sleep(1)

print eo.echoString("Hello again")
time.sleep(1)

print eo.echoString("Hello again again")
time.sleep(1)

print eo2.echoString("Hello object 2")
time.sleep(1)

print "Destroying child POA..."
child.destroy(1, 1)
print "Child POA Destroyed"

print "Destroying ORB..."
orb.destroy()
print "ORB destroyed."
