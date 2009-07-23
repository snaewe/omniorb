#!/usr/bin/env python

import sys, urllib
from omniORB import CORBA
import _GlobalIDL

# omniORBpy normally performs a call to _is_a() on the first
# invocation to an object, to be sure that future invocations will
# work properly. Unfortunately, Orbix 2.3 running on random.org does
# not support this standard call. The following prevents omniORBpy
# from calling _is_a():
argv = sys.argv[:]
argv.extend(["-ORBverifyObjectExistsAndType", "0"])

print "Initialising ORB..."
orb = CORBA.ORB_init(argv, CORBA.ORB_ID)

print "Retrieving IOR from random.org..."
ior = urllib.urlopen("http://www.random.org/Random.ior").readline()

print "Calling string_to_object..."
obj = orb.string_to_object(ior)

print "Getting 10 random numbers..."
for i in range(10):
    print obj.lrand48()
