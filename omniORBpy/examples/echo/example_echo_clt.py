#!/usr/bin/env python

import sys

# Import the CORBA module
from omniORB import CORBA

# Import the stubs for the Example module
import Example

# Initialise the ORB
orb = CORBA.ORB_init(sys.argv, CORBA.ORB_ID)

# Get the IOR of an Echo object from the command line (without
# checking that the arguments are sensible!)
ior = sys.argv[1]

# Convert the IOR to an object reference
obj = orb.string_to_object(ior)

# Narrow reference to an Example::Echo object
eo = obj._narrow(Example.Echo)

if eo is None:
    print "Object reference is not an Example::Echo"
    sys.exit(1)

# Invoke the echoString operation
message = "Hello from Python"
result  = eo.echoString(message)

print "I said '%s'. The object said '%s'." % (message,result)
