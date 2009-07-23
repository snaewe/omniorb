#!/usr/bin/env python

import sys
from omniORB import CORBA, PortableServer
import PerfTest, PerfTest__POA

class I_i (PerfTest__POA.I):

    def op0(self,l): return l
    def op1(self,a): return a
    def op2(self,a): return a
    def op3(self,a): return a
    def op4(self,a): return a
    def op5(self,a): return a
    def op6(self,a): return a
    def op7(self,a): return a
    def op8(self,a): return a
    def op9(self,a): return a
    def opa(self,a): return a

orb = CORBA.ORB_init(sys.argv, CORBA.ORB_ID)
poa = orb.resolve_initial_references("RootPOA")
poa._get_the_POAManager().activate()

ii = I_i()
io = ii._this()

print orb.object_to_string(io)

if "-l" in sys.argv:
    print "Running locally...\n\n"
    import pclient
    pclient.doTests(orb, poa, io)
    orb.shutdown(1)
else:
    orb.run()
