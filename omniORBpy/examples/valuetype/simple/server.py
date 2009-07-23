#!/usr/bin/env python

import CORBA, ValueTest, ValueTest__POA

class Test_i (ValueTest__POA.Test):

    def op1(self, a, b):
        print

        print "a:", a

        print "b:", b

        if a is b:
            print "a is b"
        else:
            print "a is not b"

        return a


def main(args):
    orb = CORBA.ORB_init(args)
    poa = orb.resolve_initial_references("RootPOA")

    ti = Test_i()
    to = ti._this()

    print orb.object_to_string(to)

    poa._get_the_POAManager().activate()
    orb.run()

if __name__ == "__main__":
    import sys
    main(sys.argv)
