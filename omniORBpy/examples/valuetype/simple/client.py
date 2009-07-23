#!/usr/bin/env python

import CORBA, ValueTest


def main(args):
    orb = CORBA.ORB_init(args)

    obj = orb.string_to_object(args[1])
    obj = obj._narrow(ValueTest.Test)

    v1 = ValueTest.One("hello", 123)
    v2 = ValueTest.One("test", 42)

    ret = obj.op1(v1, v2)
    ret = obj.op1(v1, v1)
    ret = obj.op1(v1, None)
    ret = obj.op1(None, v1)
    ret = obj.op1(None, None)

    orb.destroy()


if __name__ == "__main__":
    import sys
    main(sys.argv)
