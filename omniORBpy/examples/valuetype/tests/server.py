#!/usr/bin/env python

import CORBA, ValueTest, ValueTest__POA

class Three_i(ValueTest.Three):
    def test(self):
        print "test local call"
        return "value"


class Test_i (ValueTest__POA.Test):

    def show(self, s):
        print
        print s

    def op1(self, a):
        if a:
            print "op1:", id(a), a
        else:
            print "op1:", a
        return a

    def op2(self, a, b):
        if a:
            print "op2: a:", id(a), a
        else:
            print "op2: a:", a
        if b:
            print "op2: b:", id(b), b
        else:
            print "op2: b:", b
        if a is b:
            print "op2: a is b"
        else:
            print "op2: a is not b"

    def op3(self, a):
        if a:
            print "op3:", id(a), a
        else:
            print "op3:", a

        if a.a is a.b:
            print "op3: a.a is a.b"
        else:
            print "op2: a.a is not a.b"

        return a

    def op4(self, a):
        print "op4:", a
        return a

    def op5(self, a):
        print "op5:", a
        if a is not None:
            print a.test()

    def op6(self, a):
        av = a.value()
        if av:
            print "op6:", id(av), av
            print "op6: keys =", av.__dict__.keys()
            print "op6: derived from One =", isinstance(av, ValueTest.One)
        else:
            print "op6:", av

    def op7(self, a, b):
        av = a.value()
        bv = b.value()

        if av:
            print "op7 a:", id(av), av
            print "op7 a: keys =", av.__dict__.keys()
            print "op7 a: derived from One =", isinstance(av, ValueTest.One)
        else:
            print "op7 a:", av

        if bv:
            print "op7 b:", id(bv), bv
            print "op7 b: keys =", bv.__dict__.keys()
            print "op7 b: derived from One =", isinstance(bv, ValueTest.One)
        else:
            print "op7:", bv

        if av is bv:
            print "op7: av is bv"
        else:
            print "op7: av is not bv"

    def op8(self, a, bv):
        av = a.value()

        if av:
            print "op8 a:", id(av), av
            print "op8 a: keys =", av.__dict__.keys()
            print "op8 a: derived from One =", isinstance(av, ValueTest.One)
        else:
            print "op8 a:", av

        if bv:
            print "op8 b:", id(bv), bv
            print "op8 b: keys =", bv.__dict__.keys()
            print "op8 b: derived from One =", isinstance(bv, ValueTest.One)
        else:
            print "op8:", bv

        if av is bv:
            print "op8: av is bv"
        else:
            print "op8: av is not bv"

    def op9(self, av, b):
        bv = b.value()

        if av:
            print "op9 a:", id(av), av
            print "op9 a: keys =", av.__dict__.keys()
            print "op9 a: derived from One =", isinstance(av, ValueTest.One)
        else:
            print "op9 a:", av

        if bv:
            print "op9 b:", id(bv), bv
            print "op9 b: keys =", bv.__dict__.keys()
            print "op9 b: derived from One =", isinstance(bv, ValueTest.One)
        else:
            print "op9:", bv

        if av is bv:
            print "op9: av is bv"
        else:
            print "op9: av is not bv"

    def op10(self, e):
        print "op10 e:", e

    def op11(self, e1, e2):
        print "op11 e1:", e1, id(e1)
        print "op11 e2:", e2, id(e2)

    def op12(self, c):
        print "op12 c :", c
        if c is not None:
            print "op12 e1:", c.e1, id(c.e1)
            print "op12 e2:", c.e2, id(c.e2)


def main(args):
    orb = CORBA.ORB_init(args)
    poa = orb.resolve_initial_references("RootPOA")

    orb.register_value_factory(CORBA.id(ValueTest.Three), Three_i)

    ti = Test_i()
    to = ti._this()

    print orb.object_to_string(to)

    poa._get_the_POAManager().activate()
    orb.run()

if __name__ == "__main__":
    import sys
    main(sys.argv)
