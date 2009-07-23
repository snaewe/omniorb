#!/usr/bin/env python

import sys, time, string
from omniORB import CORBA, PortableServer
import PerfTest

cycles = 10000

def doTests(orb, poa, io):
    l = range(200)

    print "single long:    ",
    sys.stdout.flush()

    i = cycles
    b = time.time()
    while (i):
        r = io.op0(i)
        i = i - 1
    a = time.time()
    print a-b

    s = string.join(map(chr, l), "")

    print "octet sequence: ",
    sys.stdout.flush()

    i = cycles
    b = time.time()
    while (i):
        r = io.op1(s)
        i = i - 1
    a = time.time()
    print a-b

    print "octet array:    ",
    sys.stdout.flush()

    i = cycles
    b = time.time()
    while (i):
        r = io.op2(s)
        i = i - 1
    a = time.time()
    print a-b


    print "short sequence: ",
    sys.stdout.flush()

    i = cycles
    b = time.time()
    while (i):
        r = io.op3(l)
        i = i - 1
    a = time.time()
    print a-b

    print "short array:    ",
    sys.stdout.flush()

    i = cycles
    b = time.time()
    while (i):
        r = io.op4(l)
        i = i - 1
    a = time.time()
    print a-b


    print "long sequence:  ",
    sys.stdout.flush()

    i = cycles
    b = time.time()
    while (i):
        r = io.op5(l)
        i = i - 1
    a = time.time()
    print a-b

    print "long array:     ",
    sys.stdout.flush()

    i = cycles
    b = time.time()
    while (i):
        r = io.op6(l)
        i = i - 1
    a = time.time()
    print a-b

    print "ulong sequence: ",
    sys.stdout.flush()

    i = cycles
    b = time.time()
    while (i):
        r = io.op7(l)
        i = i - 1
    a = time.time()
    print a-b

    print "ulong array:    ",
    sys.stdout.flush()

    i = cycles
    b = time.time()
    while (i):
        r = io.op8(l)
        i = i - 1
    a = time.time()
    print a-b

    u = map(long, l)

    print "ulong sequence: ",
    sys.stdout.flush()

    i = cycles
    b = time.time()
    while (i):
        r = io.op7(u)
        i = i - 1
    a = time.time()
    print a-b

    print "ulong array:    ",
    sys.stdout.flush()

    i = cycles
    b = time.time()
    while (i):
        r = io.op8(u)
        i = i - 1
    a = time.time()
    print a-b

    print "double sequence:",
    sys.stdout.flush()

    i = cycles
    b = time.time()
    while (i):
        r = io.op9(l)
        i = i - 1
    a = time.time()
    print a-b

    print "double array:   ",
    sys.stdout.flush()

    i = cycles
    b = time.time()
    while (i):
        r = io.opa(l)
        i = i - 1
    a = time.time()
    print a-b

    d = map(float, l)

    print "double sequence:",
    sys.stdout.flush()

    i = cycles
    b = time.time()
    while (i):
        r = io.op9(d)
        i = i - 1
    a = time.time()
    print a-b

    print "double array:   ",
    sys.stdout.flush()

    i = cycles
    b = time.time()
    while (i):
        r = io.opa(d)
        i = i - 1
    a = time.time()
    print a-b





if __name__ == "__main__":
    orb = CORBA.ORB_init(sys.argv, CORBA.ORB_ID)
    poa = orb.resolve_initial_references("RootPOA")
    poa._get_the_POAManager().activate()

    io = orb.string_to_object(sys.argv[1])

    doTests(orb, poa, io)
