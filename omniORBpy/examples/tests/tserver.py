#!/usr/bin/env python

import sys, time, gc
from omniORB import CORBA, PortableServer
import TypeTest, TypeTest__POA

class I_i (TypeTest__POA.I):

    def simple1 (self): return
    def simple2 (self): return

    def simple3 (self, a): return a
    def simple4 (self, a): return a
    def simple5 (self, a): return a
    def simple6 (self, a): return a
    def simple7 (self, a): return a
    def simple8 (self, a): return a
    def simple9 (self, a): return a
    def simple10(self, a): return a
    def simple11(self, a): return a

    def combine1(self, a, b, c): return
    def combine2(self, a, c):    return (41, 42)

    def _set_sattr1(self, a): self._sattr1 = a
    def _set_sattr2(self, a): self._sattr2 = a
    def _set_sattr3(self, a): self._sattr3 = a
    def _set_sattr4(self, a): self._sattr4 = a
    def _set_sattr5(self, a): self._sattr5 = a
    def _set_sattr6(self, a): self._sattr6 = a
    def _set_sattr7(self, a): self._sattr7 = a
    def _set_sattr8(self, a): self._sattr8 = a
    def _set_sattr9(self, a): self._sattr9 = a

    def _get_sattr1(self): return self._sattr1
    def _get_sattr2(self): return self._sattr2
    def _get_sattr3(self): return self._sattr3
    def _get_sattr4(self): return self._sattr4
    def _get_sattr5(self): return self._sattr5
    def _get_sattr6(self): return self._sattr6
    def _get_sattr7(self): return self._sattr7
    def _get_sattr8(self): return self._sattr8
    def _get_sattr9(self): return self._sattr9

    def complex1 (self, a): return a
    def complex2 (self, a):
        a.op()
        return a
    
    def complex3 (self, a): return a
    def complex4 (self, a): return a
    def complex5 (self, a): return a
    def complex6 (self, a): return a
    def complex7 (self, a): return a
    def complex8 (self, a): return a
    def complex9 (self, a): return a
    def complex10(self, a): return a
    def complex11(self, a): return a
    def complex12(self, a): return a
    def complex13(self, a): return a
    def complex14(self, a): return a
    def complex15(self, a): return a
    def complex16(self, a): return a
    def complex17(self, a): return a
    def complex18(self, a): return a
    def complex19(self, a): return a
    def complex20(self, a): return a
    def complex21(self, a): return a
    def complex22(self, a): return a
    def complex23(self, a): return a
    def complex24(self, a): return a
    def complex25(self, a): return a
    def complex26(self, a): return a
    def complex27(self, a): return a
    def complex28(self, a): return a
    def complex29(self, a): return a
    def complex30(self, a): return a
    def complex31(self, a): return a
    def complex32(self, a): return a
    def complex33(self, a): return a
    def complex34(self, a): return a
    def complex35(self, a): return a
    def complex36(self, a): return a
    def complex37(self, a): return a
    def complex38(self, a): return a
    def complex39(self, a): return a
    def complex40(self, a): return a
    def complex41(self, a): return a
    def complex42(self, a): return a
    def complex43(self, a): return a
    def complex44(self, a): return a
    def complex45(self, a): return a

    def except1(self):
        raise TypeTest.X1()

    def except2(self, a):
        if a == 1:
            raise TypeTest.X1()
        elif a == 2:
            raise TypeTest.X2(123, "Hi")
        else:
            return

    def except3(self, a):
        if a == 1:
            sys.stderr.write("\nExpect to see an exception traceback here:\n")
            raise TypeTest.X1()
        elif a == 2:
            raise CORBA.NO_PERMISSION(123, CORBA.COMPLETED_YES)
        else:
            return

    def any1(self, a): return a
    def tc1 (self, a): return a

    def context1(self, l, ctxt):
        vs = ctxt.get_values("*").items()
        r  = []
        for v in vs:
            r.append(v[0])
            r.append(v[1])
        return r

    def context2(self, l, ctxt):
        vs = ctxt.get_values("*").items()
        r  = []
        for v in vs:
            r.append(v[0])
            r.append(v[1])
        return r


orb = CORBA.ORB_init(sys.argv, CORBA.ORB_ID)
poa = orb.resolve_initial_references("RootPOA")
poa._get_the_POAManager().activate()

ii = I_i()
io = ii._this()

print orb.object_to_string(io)

if "-l" in sys.argv:
    print "Running locally...\n\n"
    import tclient
    tclient.doTests(orb, poa, io)

    if "-r" in sys.argv:
        print "\nRepeating tests..."
        tclient.output = 0
        while 1:
            tclient.doTests(orb, poa, io)

    orb.destroy()

else:
    orb.run()
