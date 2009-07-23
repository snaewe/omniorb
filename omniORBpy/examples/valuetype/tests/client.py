#!/usr/bin/env python

import CORBA, ValueTest, ValueTest__POA, Derived


class Three_i(ValueTest.Three):
    def test(self):
        print "test local call"
        return "value"

class Four_i(ValueTest__POA.Four):
    def test(self):
        print "test callback"
        return "object"


def main(args):
    orb = CORBA.ORB_init(args)
    poa = orb.resolve_initial_references("RootPOA")
    poa._get_the_POAManager().activate()

    orb.register_value_factory(CORBA.id(ValueTest.Three), Three_i)

    obj = orb.string_to_object(args[1])
    obj = obj._narrow(ValueTest.Test)

    v1 = ValueTest.One("hello", 123)
    v2 = ValueTest.One("test", 42)
    v3 = ValueTest.Two(None, None)
    v4 = ValueTest.Two(v1, None)
    v5 = ValueTest.Two(v1, v2)
    v6 = ValueTest.Two(v1, v1)
    v7 = Derived.Five("abc", 456, "more")

    obj.show("Simple values")
    r1 = obj.op1(v1)
    r2 = obj.op1(v2)
    r3 = obj.op1(None)

    obj.show("Two different values")
    obj.op2(v1, v2)

    obj.show("Nil, value")
    obj.op2(None, v1)

    obj.show("Value, nil")
    obj.op2(v1, None)

    obj.show("Two nils")
    obj.op2(None, None)

    obj.show("Two references to the same value")
    obj.op2(v1, v1)

    obj.show("Value containing two nils")
    r4 = obj.op3(v3)

    obj.show("Value containing value, nil")
    r5 = obj.op3(v4)

    obj.show("Value containing val1, val2")
    r6 = obj.op3(v5)

    obj.show("Value containing two references to same value")
    r7 = obj.op3(v6)

    obj.show("Derived value (should be truncated)")
    r8 = obj.op1(v7)

    obj.show("Same derived value twice")
    obj.op2(v7, v7)

    obj.show("Base value, derived value")
    obj.op2(v1, v7)

    obj.show("Derived value, base value")
    obj.op2(v7, v1)

    obj.show("String in valuebox")
    r9  = obj.op4("Hello")

    obj.show("Empty value box")
    r10 = obj.op4(None)

    obj.show("Nil abstract interface")
    obj.op5(None)

    fi = Four_i()
    fo = fi._this()

    obj.show("Abstract interface set to object reference")
    obj.op5(fo)

    t = Three_i("experiment")
    obj.show("Abstract interface set to value")
    obj.op5(t)

    # Any tests
    a1 = CORBA.Any(ValueTest._tc_One, v1)
    a2 = CORBA.Any(Derived._tc_Five, v7)
    v8 = Derived.Six(1.234, "test")
    a3 = CORBA.Any(Derived._tc_Six, v8)
    a4 = CORBA.Any(ValueTest._tc_One, None)
    a5 = CORBA.Any(ValueTest._tc_One, v2)
    a6 = CORBA.Any(ValueTest._tc_One, v2)

    obj.show("Value in Any")
    obj.op6(a1)

    obj.show("Derived value in Any")
    obj.op6(a2)

    obj.show("Completely unknown value in any")
    obj.op6(a3)

    obj.show("Nil value in any")
    obj.op6(a4)


    obj.show("Two anys")
    obj.op7(a1, a5)

    obj.show("Same any twice")
    obj.op7(a1, a1)

    obj.show("Different anys containing same value")
    obj.op7(a5, a6)

    obj.show("Same derived value twice")
    obj.op7(a2, a2)


    obj.show("Any and value")
    obj.op8(a1, v2)

    obj.show("Any and same value")
    obj.op8(a1, v1)

    obj.show("Any and derived")
    obj.op8(a1, v7)

    obj.show("Same derived value in any and value")
    obj.op8(a2, v7)


    obj.show("Value and any")
    obj.op9(v2, a1)

    obj.show("Value and same value in any")
    obj.op9(v1, a1)

    obj.show("Derived value and any")
    obj.op9(v7, a1)

    obj.show("Same derived value as value and in any")
    obj.op9(v7, a2)

    obj.show("Empty value")
    e1 = ValueTest.Empty()
    e2 = ValueTest.Empty()
    obj.op10(e1)

    obj.show("Different empty values")
    obj.op11(e1, e2)

    obj.show("Same empty values")
    obj.op11(e1, e1)

    obj.show("Empty value, None")
    obj.op11(e1, None)

    obj.show("None, empty value")
    obj.op11(None, e1)

    obj.show("None, None")
    obj.op11(None, None)

    obj.show("Container of empty values")
    c1 = ValueTest.Container(e1, e2)
    obj.op12(c1)

    orb.destroy()


if __name__ == "__main__":
    import sys
    main(sys.argv)
