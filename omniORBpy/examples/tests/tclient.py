#!/usr/bin/env python

import sys, time, gc
from omniORB import CORBA, PortableServer
import omniORB
import TypeTest, TypeTest__POA


failed = []
output = 1

def tstart(s):
    global current_test, output
    current_test = s
    if output:
        sys.stdout.write(s + ": ")
        sys.stdout.flush()

def tresult(s):
    if output:
        sys.stdout.write(str(s))
        sys.stdout.flush()

def tpass():
    if output:
        sys.stdout.write("  pass\n")
        sys.stdout.flush()

def tfail():
    global failed, current_test
    failed.append(current_test)
    if output:
        sys.stdout.write("  fail\n")
        sys.stdout.flush()

def tdone():
    global failed
    if len(failed) > 0:
        print "\nFailed tests:"
        for fail in failed:
            print "  ", fail
    else:
        if output:
            print "\nAll tests passed."



op_called = 0
j_deleted = 0

class J_i (TypeTest__POA.J):
    def __init__(self):
        global op_called, j_deleted
        op_called = j_deleted = 0

    def op(self):
        global op_called
        op_called = 1

    def __del__(self):
        global j_deleted
        j_deleted = 1


def doTests(orb, poa, io):
    global op_called, failed

    failed = []

    tstart("Void")
    r = io.simple1()
    if r is None: tpass()
    else:         tfail()

    tstart("Oneway")
    r = io.simple2()
    if r is None: tpass()
    else:         tfail()

    tstart("Short")
    r = io.simple3(42)
    if r == 42: tpass()
    else:       tfail()

    tstart("UShort")
    r = io.simple4(42)
    if r == 42: tpass()
    else:       tfail()

    tstart("Long")
    r = io.simple5(42)
    if r == 42: tpass()
    else:       tfail()

    tstart("ULong with long")
    r = io.simple6(42L)
    if r == 42L: tpass()
    else:        tfail()

    tstart("ULong with int")
    r = io.simple6(42)
    if r == 42: tpass()
    else:       tfail()

    tstart("Float with float")
    r = io.simple7(1.234)
    tresult(r)
    tpass()

    tstart("Float with int")
    r = io.simple7(42)
    if r == 42: tpass()
    else:       tfail()

    tstart("Double with float")
    r = io.simple8(1.234)
    if r == 1.234: tpass()
    else:          tfail()

    tstart("Double with int")
    r = io.simple8(42)
    if r == 42: tpass()
    else:       tfail()

    tstart("Boolean")
    r = io.simple9(1)
    if r == 1: tpass()
    else:      tfail()

    tstart("Char")
    r = io.simple10("a")
    if r == "a": tpass()
    else:        tfail()

    tstart("Octet")
    r = io.simple11(123)
    if r == 123: tpass()
    else:        tfail()

    tstart("Invalid arguments")
    ok = 1

    try:
        io.simple1(5)
        ok = 0
        tresult("-")
    except TypeError:
        tresult("+")

    try:
        io.simple3(1.234)
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    try:
        io.simple3("Hello")
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    try:
        io.simple3(0x8123)
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    try:
        io.simple4(-1)
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    try:
        io.simple6(1233456789012345L)
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    try:
        io.simple10("Hello")
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    try:
        io.simple10(65)
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    try:
        io.simple11(1234)
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    try:
        io.simple11(-1)
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    if ok: tpass()
    else:  tfail()

    tstart("Combination 1")
    r = io.combine1(123, 456, 789L)
    if r is None: tpass()
    else:         tfail()

    tstart("Combination 2")
    r = io.combine2(123, 456L)
    tresult(r)
    tpass()

    tstart("Short attr")
    io._set_sattr1(123)
    tresult("+")
    r = io._get_sattr1()
    if r == 123: tpass()
    else:        tfail()

    tstart("UShort attr")
    io._set_sattr2(123)
    tresult("+")
    r = io._get_sattr2()
    if r == 123: tpass()
    else:        tfail()

    tstart("Long attr")
    io._set_sattr3(123)
    tresult("+")
    r = io._get_sattr3()
    if r == 123: tpass()
    else:        tfail()

    tstart("ULong attr")
    io._set_sattr4(123L)
    tresult("+")
    r = io._get_sattr4()
    if r == 123L: tpass()
    else:         tfail()

    tstart("Float attr")
    io._set_sattr5(1.234)
    tresult("+")
    r = io._get_sattr5()
    tresult(r)
    tpass()

    tstart("Double attr")
    io._set_sattr6(1.234)
    tresult("+")
    r = io._get_sattr6()
    if r == 1.234: tpass()
    else:          tfail()

    tstart("Boolean attr")
    io._set_sattr7(0)
    tresult("+")
    r = io._get_sattr7()
    if r == 0: tpass()
    else:      tfail()

    tstart("Char attr")
    io._set_sattr8("a")
    tresult("+")
    r = io._get_sattr8()
    if r == "a": tpass()
    else:        tfail()

    tstart("Octet attr")
    io._set_sattr9(123)
    tresult("+")
    r = io._get_sattr9()
    if r == 123: tpass()
    else:        tfail()


    tstart("Invalid attributes")
    ok = 1
    try:
        io._set_sattr1("hello")
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    try:
        io._set_sattr2(1.234)
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    try:
        io._set_sattr3("hello")
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    try:
        io._set_sattr4("hello")
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    try:
        io._set_sattr5("hello")
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    try:
        io._set_sattr6("hello")
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    try:
        io._set_sattr7("hello")
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    try:
        io._set_sattr8("hello")
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    try:
        io._set_sattr9("hello")
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    if ok: tpass()
    else:  tfail()


    ji = J_i()
    jo = ji._this()

    tstart("Known Object")
    r = io.complex1(jo)
    if r._is_equivalent(jo): tpass()
    else:                    tfail()

    tstart("Unknown Object")
    try:
        ns = orb.resolve_initial_references("NameService")
        r = io.complex1(ns)
        if r._is_equivalent(ns): tpass()
        else:                    tfail()
    except CORBA.NO_RESOURCES:
        tresult("skip")
        tpass()

    tstart("Nil Object")
    r = io.complex1(None)
    if r is None: tpass()
    else:         tfail()

    tstart("Known interface")
    r = io.complex2(jo)
    ok = 1
    if r._is_equivalent(jo):
        tresult("+")
    else:
        ok = 0
        tresult("-")
    if op_called:
        tresult("+")
    else:
        ok = 0
        tresult("-")
    if ok: tpass()
    else:  tfail()

    tstart("Deactivate object")
    id = poa.reference_to_id(jo)
    del ji, jo
    r = None
    poa.deactivate_object(id)
    time.sleep(0.1) # In case a separate thread is doing the deletion
    if j_deleted: tpass()
    else:         tfail()


    tstart("Struct S1")
    s1 = TypeTest.S1(1, 2, 3, 4, 5.6, 7, 1, "a", 9)
    r = io.complex3(s1)
    if r.a == 1 and \
       r.b == 2 and \
       r.c == 3 and \
       r.d == 4 and \
       r.e > 5.0 and r.e < 6.0 and \
       r.f == 7 and \
       r.g == 1 and \
       r.h == "a" and \
       r.i == 9:
        tpass()
    else:
        tfail()

    tstart("Invalid Struct S1")
    s1 = TypeTest.S1(1, 2, 3, 4, 5.6, 7.8, 1, "ab", 9)
    try:
        r = io.complex3(s1)
        tfail()
    except CORBA.BAD_PARAM:
        tpass()

    tstart("Struct S1 in class")
    class mys1:
        a = 1
        b = 2
        c = 3
        d = 4
        e = 5.6
        f = 7.8
        g = 1
        h = "a"
        i = 9

    r = io.complex3(mys1)
    if r.a == 1 and \
       r.b == 2 and \
       r.c == 3 and \
       r.d == 4 and \
       r.e > 5.0 and r.e < 6.0 and \
       r.f == 7.8 and \
       r.g == 1 and \
       r.h == "a" and \
       r.i == 9:
        tpass()
    else:
        tfail()

    tstart("Struct S1 in non-S1 instance")
    r = io.complex3(mys1())
    if r.a == 1 and \
       r.b == 2 and \
       r.c == 3 and \
       r.d == 4 and \
       r.e > 5.0 and r.e < 6.0 and \
       r.f == 7.8 and \
       r.g == 1 and \
       r.h == "a" and \
       r.i == 9:
        tpass()
    else:
        tfail()


    tstart("Union U1")
    ok = 1
    u = TypeTest.U1(a=123)
    r = io.complex4(u)
    if r.a == 123:
        tresult("+")
    else:
        ok = 0
        tresult("-")

    u = TypeTest.U1(b=42)
    r = io.complex4(u)
    if r.b == 42 and u._d == r._d:
        tresult("+" + str(u._d) + str(r._d))
    else:
        ok = 0
        tresult("-" + str(u._d) + str(r._d))

    u = TypeTest.U1(1, 42)
    r = io.complex4(u)
    if r.b == 42 and u._d == r._d:
        tresult("+" + str(u._d) + str(r._d))
    else:
        ok = 0
        tresult("-" + str(u._d) + str(r._d))

    u = TypeTest.U1(c=5)
    r = io.complex4(u)
    if r.c == 5:
        tresult("+")
    else:
        ok = 0
        tresult("-")

    u = TypeTest.U1(42, None)
    r = io.complex4(u)
    if r._d == 42:
        tresult("+" + str(r._v))
    else:
        ok = 0
        tresult("-" + str(r._v))

    u = TypeTest.U1(42, "Hello")
    r = io.complex4(u)
    if r._d == 42:
        tresult("+" + str(r._v))
    else:
        ok = 0
        tresult("-" + str(r._v))

    if ok: tpass()
    else:  tfail()


    tstart("Union U2")
    ok = 1
    u = TypeTest.U2(a=123)
    r = io.complex5(u)
    if r.a == 123:
        tresult("+")
    else:
        ok = 0
        tresult("-")

    u = TypeTest.U2(b=42)
    r = io.complex5(u)
    if r.b == 42 and u._d == r._d:
        tresult("+" + str(u._d) + str(r._d))
    else:
        ok = 0
        tresult("-" + str(u._d) + str(r._d))

    u = TypeTest.U2("b", 42)
    r = io.complex5(u)
    if r.b == 42 and u._d == r._d:
        tresult("+" + str(u._d) + str(r._d))
    else:
        ok = 0
        tresult("-" + str(u._d) + str(r._d))

    u = TypeTest.U2(c=5)
    r = io.complex5(u)
    if r.c == 5:
        tresult("+")
    else:
        ok = 0
        tresult("-")

    if ok: tpass()
    else:  tfail()


    tstart("Invalid Unions")
    s1 = TypeTest.S1(1, 2, 3, 4, 5.6, 7.8, 1, "a", 9)
    ok = 1
    try:
        r = io.complex4(s1)
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    try:
        u = TypeTest.U2(b = 0x81234)
        r = io.complex5(u)
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")
    
    if ok: tpass()
    else:  tfail()


    tstart("Enum")
    ok = 1
    r = io.complex6(TypeTest.one)
    if r == TypeTest.one:
        tresult("+")
    else:
        tresult("-")
        ok = 0

    r = io.complex6(TypeTest.four)
    if r == TypeTest.four:
        tresult("+")
    else:
        tresult("-")
        ok = 0

    if ok: tpass()
    else:  tfail()

    tstart("Invalid enum")
    ok = 1
    try:
        r = io.complex6(1234)
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    try:
        r = io.complex6(TypeTest.five)
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    try:
        r = io.complex6(TypeTest.nine)
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    if ok: tpass()
    else:  tfail()


    tstart("String")
    ok = 1
    s = "Hello there"
    r = io.complex7(s)
    if r == s:
        tresult("+")
    else:
        tresult("-")
        ok = 0
    
    s = "This is a much longer string with lots of stuff in it. Blah blah blah"
    r = io.complex7(s)
    if r == s:
        tresult("+")
    else:
        tresult("-")
        ok = 0

    s = "This is a string with a \0 in it."
    try:
        r = io.complex7(s)
        tresult("-")
        ok = 0
    except CORBA.BAD_PARAM:
        tresult("+")

    if ok: tpass()
    else:  tfail()

    tstart("Bounded string")
    ok = 1
    s = "Hello"
    r = io.complex8(s)
    if r == s:
        tresult("+")
    else:
        tresult("-")
        ok = 0
    
    s = "This is a much longer string with lots of stuff in it. Blah blah blah"
    try:
        r = io.complex8(s)
        tresult("-")
        ok = 0
    except CORBA.MARSHAL:
        tresult("+")

    s = "str\0ing"
    try:
        r = io.complex8(s)
        tresult("-")
        ok = 0
    except CORBA.BAD_PARAM:
        tresult("+")

    if ok: tpass()
    else:  tfail()


    tstart("Sequences")
    ok = 1

    s = [1, 2, 3, 4, 5]
    r = io.complex9(s)
    if list(r) == list(s):
        tresult("+")
    else:
        ok = 0
        tresult("-")

    s = (-6, 7, -8, 9, 10, 11)
    r = io.complex9(s)
    if list(r) == list(s):
        tresult("+")
    else:
        ok = 0
        tresult("-")

    s = [1, 2, 3, 4, 5]
    r = io.complex10(s)
    if list(r) == list(s):
        tresult("+")
    else:
        ok = 0
        tresult("-")

    s = (1, 2, -3, 4, 5)
    r = io.complex11(s)
    if list(r) == list(s):
        tresult("+")
    else:
        ok = 0
        tresult("-")

    s = [1L, 2, 3L, 4L, 5]
    r = io.complex12(s)
    if list(r) == list(s):
        tresult("+")
    else:
        ok = 0
        tresult("-")

    s = (1.2, 2, -3.4, 4.5, 5.6)
    r = io.complex13(s)
    if len(r) == len(s):
        tresult(r)
    else:
        ok = 0
        tresult("-")

    s = [1.2, 2, -3.4, 4.5, 5.6]
    r = io.complex14(s)
    if len(r) == len(s):
        tresult(r)
    else:
        ok = 0
        tresult("-")

    s = (1, 0, 1, 1, 0, 3, 1, 1, 1)
    r = io.complex15(s)
    if len(r) == len(s):
        tresult(r)
    else:
        ok = 0
        tresult("-")

    s = "This is a sequence of char with a \0 in it"
    r = io.complex16(s)
    if r == s:
        tresult("+")
    else:
        ok = 0
        tresult("-")

    s = "This is a sequence of octet, which is remarkably similar to a sequence of char. It also has a \0 in it."
    r = io.complex17(s)
    if r == s:
        tresult("+")
    else:
        ok = 0
        tresult("-")

    if ok: tpass()
    else:  tfail()


    tstart("Invalid sequences")
    ok = 1

    s = [1, 2, "Hi", 4, 5]
    try:
        r = io.complex9(s)
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    s = (-6, 7, -8, 9, 10, 11)
    try:
        r = io.complex10(s)
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    s = (1, 2, -3.5, 4, 5)
    try:
        r = io.complex11(s)
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    s = [1L, 2, -3L, 4L, 5]
    try:
        r = io.complex12(s)
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    s = (1.2, 2, -3.4, None, 5.6)
    try:
        r = io.complex13(s)
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    s = [1.2, "Hi", -3.4, 4.5, 5.6]
    try:
        r = io.complex14(s)
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    s = (1, 0, 1, 1.2, 0, 3, 1, 1, 1)
    try:
        r = io.complex15(s)
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    s = [1, 2, 3, 4, 5]
    try:
        r = io.complex16(s)
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    s = [1, 2, 3, 4, 5]
    try:
        r = io.complex17(s)
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    if ok: tpass()
    else:  tfail()


    tstart("Bounded Sequences")
    ok = 1

    s = [1, 2, 3, 4, 5]
    r = io.complex18(s)
    if list(r) == list(s):
        tresult("+")
    else:
        ok = 0
        tresult("-")

    s = (-6, 7, -8, 9, 10, 11)
    r = io.complex18(s)
    if list(r) == list(s):
        tresult("+")
    else:
        ok = 0
        tresult("-")

    s = [1, 2, 3, 4, 5]
    r = io.complex19(s)
    if list(r) == list(s):
        tresult("+")
    else:
        ok = 0
        tresult("-")

    s = (1, 2, -3, 4, 5)
    r = io.complex20(s)
    if list(r) == list(s):
        tresult("+")
    else:
        ok = 0
        tresult("-")

    s = [1L, 2, 3L, 4L, 5]
    r = io.complex21(s)
    if list(r) == list(s):
        tresult("+")
    else:
        ok = 0
        tresult("-")

    s = (1.2, 2, -3.4, 4.5, 5.6)
    r = io.complex22(s)
    if len(r) == len(s):
        tresult(r)
    else:
        ok = 0
        tresult("-")

    s = [1.2, 2, -3.4, 4.5, 5.6]
    r = io.complex23(s)
    if len(r) == len(s):
        tresult(r)
    else:
        ok = 0
        tresult("-")

    s = (1, 0, 1, 1, 0, 3, 1, 1, 1, 1)
    r = io.complex24(s)
    if len(r) == len(s):
        tresult(r)
    else:
        ok = 0
        tresult("-")

    s = "This is a "
    r = io.complex25(s)
    if r == s:
        tresult("+")
    else:
        ok = 0
        tresult("-")

    s = "This is a "
    r = io.complex26(s)
    if r == s:
        tresult("+")
    else:
        ok = 0
        tresult("-")

    if ok: tpass()
    else:  tfail()


    tstart("Invalid bounded sequences")
    ok = 1

    s = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11]
    try:
        r = io.complex18(s)
        ok = 0
        tresult("-")
    except CORBA.MARSHAL:
        tresult("+")

    s = (-6, 7, -8, 9, 10, 11)
    try:
        r = io.complex19(s)
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    s = (1, 2, -3, 4, 5, 6, 7, 8, 9, 10, 12, 134)
    try:
        r = io.complex20(s)
        ok = 0
        tresult("-")
    except CORBA.MARSHAL:
        tresult("+")

    s = [1L, 2, 3L, 4L, 5, 6L, 7L, 8, 9, 10, 11, 12]
    try:
        r = io.complex21(s)
        ok = 0
        tresult("-")
    except CORBA.MARSHAL:
        tresult("+")

    s = (1.2, 2, -3.4, 4.5, 5.6, 8, 9, 10, 11, 12, 14)
    try:
        r = io.complex22(s)
        ok = 0
        tresult("-")
    except CORBA.MARSHAL:
        tresult("+")

    s = [1.2, 2.3, -3.4, 4.5, 5.6, 7, 8, 9, 10, 11, 12, 145]
    try:
        r = io.complex23(s)
        ok = 0
        tresult("-")
    except CORBA.MARSHAL:
        tresult("+")

    s = (1, 0, 1, 1.2, 0, 3, 1, 1, 1)
    try:
        r = io.complex24(s)
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    s = "This is a long sequence<char>"
    try:
        r = io.complex25(s)
        ok = 0
        tresult("-")
    except CORBA.MARSHAL:
        tresult("+")

    s = "This is a long sequence<octet>"
    try:
        r = io.complex26(s)
        ok = 0
        tresult("-")
    except CORBA.MARSHAL:
        tresult("+")

    if ok: tpass()
    else:  tfail()

    
    tstart("Arrays")
    ok = 1

    s = [1, 2, -3, 4, 5]
    r = io.complex27(s)
    if list(r) == list(s):
        tresult("+")
    else:
        ok = 0
        tresult("-")

    s = [1, 2, 3, 4, 5]
    r = io.complex28(s)
    if list(r) == list(s):
        tresult("+")
    else:
        ok = 0
        tresult("-")

    s = (1, 2, -3, 4, 5)
    r = io.complex29(s)
    if list(r) == list(s):
        tresult("+")
    else:
        ok = 0
        tresult("-")

    s = [1L, 2, 3L, 4L, 5]
    r = io.complex30(s)
    if list(r) == list(s):
        tresult("+")
    else:
        ok = 0
        tresult("-")

    s = (1.2, 2, -3.4, 4.5, 5.6)
    r = io.complex31(s)
    if len(r) == len(s):
        tresult(r)
    else:
        ok = 0
        tresult("-")

    s = [1.2, 2, -3.4, 4.5, 5.6]
    r = io.complex32(s)
    if len(r) == len(s):
        tresult(r)
    else:
        ok = 0
        tresult("-")

    s = (1, 0, 1, 3, 0)
    r = io.complex33(s)
    if len(r) == len(s):
        tresult(r)
    else:
        ok = 0
        tresult("-")

    s = "12345"
    r = io.complex34(s)
    if r == s:
        tresult("+")
    else:
        ok = 0
        tresult("-")

    s = "ab\0de"
    r = io.complex35(s)
    if r == s:
        tresult("+")
    else:
        ok = 0
        tresult("-")

    s = ["12345", "hello", "abc", "aa", "This is a long string"]
    r = io.complex36(s)
    if list(r) == list(s):
        tresult("+")
    else:
        ok = 0
        tresult("-")

    if ok: tpass()
    else:  tfail()


    tstart("Invalid arrays")
    ok = 1

    s = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11]
    try:
        r = io.complex27(s)
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    s = (-6, 7, -8, 9, 10)
    try:
        r = io.complex28(s)
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    s = (1, 2, -3, 4, 5, 6, 7, 8, 9, 10, 12, 134)
    try:
        r = io.complex29(s)
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    s = [1L, 2, 3L, 4L, 5, 6L, 7L, 8, 9, 10, 11, 12]
    try:
        r = io.complex30(s)
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    s = (1.2, 2, -3.4, 4.5, 5.6, 8, 9, 10, 11, 12, 14)
    try:
        r = io.complex31(s)
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    s = [1.2, 2.3, -3.4, 4.5, 5.6, 7, 8, 9, 10, 11, 12, 145]
    try:
        r = io.complex32(s)
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    s = (1, 0, 1, 1.2, 0, 3, 1, 1, 1)
    try:
        r = io.complex33(s)
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    s = "This is a long sequence<char>"
    try:
        r = io.complex34(s)
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    s = "Hi!"
    try:
        r = io.complex35(s)
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    s = ["12345", 12345, "abc", "", "This is a long string"]
    try:
        r = io.complex36(s)
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    if ok: tpass()
    else:  tfail()

    tstart("Short with Long")
    r = io.simple3(42L)
    if r == 42: tpass()
    else:       tfail()

    tstart("UShort with Long")
    r = io.simple4(42L)
    if r == 42: tpass()
    else:       tfail()

    tstart("Long with Long")
    r = io.simple5(42L)
    if r == 42: tpass()
    else:       tfail()

    tstart("Float with Long")
    r = io.simple7(42L)
    if r == 42: tpass()
    else:       tfail()

    tstart("Double with Long")
    r = io.simple8(42L)
    if r == 42: tpass()
    else:       tfail()

    tstart("Octet with Long")
    r = io.simple11(123L)
    if r == 123: tpass()
    else:        tfail()

    tstart("Sequences with Longs")
    ok = 1

    s = [1L, 2L, 3L, 4L, 5L]
    r = io.complex9(s)
    if list(r) == list(s):
        tresult("+")
    else:
        ok = 0
        tresult("-")

    s = (-6L, 7L, -8L, 9L, 10L, 11L)
    r = io.complex9(s)
    if list(r) == list(s):
        tresult("+")
    else:
        ok = 0
        tresult("-")

    s = [1L, 2L, 3L, 4L, 5L]
    r = io.complex10(s)
    if list(r) == list(s):
        tresult("+")
    else:
        ok = 0
        tresult("-")

    s = (1L, 2L, -3L, 4L, 5L)
    r = io.complex11(s)
    if list(r) == list(s):
        tresult("+")
    else:
        ok = 0
        tresult("-")

    s = [1L, 2, 3L, 4L, 5]
    r = io.complex12(s)
    if list(r) == list(s):
        tresult("+")
    else:
        ok = 0
        tresult("-")

    s = (1L, 2L, -3L, 4L, 5L)
    r = io.complex13(s)
    if len(r) == len(s):
        tresult(r)
    else:
        ok = 0
        tresult("-")

    s = [1L, 2L, -3L, 4L, 5L]
    r = io.complex14(s)
    if len(r) == len(s):
        tresult(r)
    else:
        ok = 0
        tresult("-")

    s = (1L, 0L, 1L, 1L, 0L, 3L, 1L, 1L, 1L)
    r = io.complex15(s)
    if len(r) == len(s):
        tresult(r)
    else:
        ok = 0
        tresult("-")

    if ok: tpass()
    else:  tfail()


    tstart("Struct S2")
    ok = 1
    jo = J_i()._this()
    s2 = TypeTest.S2(TypeTest.S1(1, 2, 3, 4, 5.6, 7, 8, "a", 10),
                     TypeTest.U1(a = 5),
                     jo,
                     [1, 2, 3, 4, 5, 6, 7],
                     "octet sequence",
                     "string")
    r = io.complex37(s2)
    if r.c._is_equivalent(jo):
        tresult("+")
    else:
        ok = 0
        tresult("-")
    id = poa.reference_to_id(jo)
    poa.deactivate_object(id)

    s2 = TypeTest.S2(TypeTest.S1(1, 2, 3, 4, 5.6, 7, 8, "a", 10),
                     TypeTest.U1(a = 5),
                     None,
                     [1, 2, 3, 4, 5, 6, 7],
                     "octet sequence",
                     "string")
    r = io.complex37(s2)
    if r.c is None:
        tresult("+")
    else:
        ok = 0
        tresult("-")

    if ok: tpass()
    else:  tfail()


    tstart("Invalid Struct S2")

    s2 = TypeTest.S2(TypeTest.S1(1, 2, 3, 4, 5.6, 7, 8, "a", 10),
                     TypeTest.U1(a = 5),
                     12345,
                     [1, 2, 3, 4, 5, 6, 7],
                     "octet sequence",
                     "string")
    try:
        r = io.complex37(s2)
        tfail()
    except CORBA.BAD_PARAM:
        tpass()


    tstart("Struct S3")

    i1 = TypeTest.S3(42, TypeTest.S3.U(0,None))
    i2 = TypeTest.S3(5,  TypeTest.S3.U(a=[i1]))
    i3 = TypeTest.S3(17, TypeTest.S3.U(a=[i2, i1]))

    r = io.complex38(i3)
    if r.b.a[0].a == 5: tpass()
    else:               tfail()


    tstart("Sequence of struct S1")

    ok = 1
    s  = [TypeTest.S1(1, 2, 3, 4, 5.6, 7, 8, "a", 10),
          TypeTest.S1(-10, 0xffff, -1234567, 42L, 1.234, 5.678, 0, "z", 255)]
    r  = io.complex39(s)

    if r[1].b == 0xffff:
        tresult("+")
    else:
        ok = 0
        tresult("-")

    s = []
    r = io.complex39(s)
    if r == s:
        tresult("+")
    else:
        ok = 0
        tresult("-")

    if ok: tpass()
    else:  tfail()

    tstart("Invalid sequence of struct S1")

    ok = 1
    s  = [TypeTest.S1(1, 2, 3, 4, 5.6, 7, 8, "a", 10),
          TypeTest.S1(-10, 0x10000, -1234567, 42L, 1.234, 5.678, 0, "z", 255)]

    try:
        r  = io.complex39(s)
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    s = [123]
    try:
        r  = io.complex39(s)
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    if ok: tpass()
    else:  tfail()

    tstart("Long sequence of struct S1")

    ok = 1
    s  = [TypeTest.S1(1, 2, 3, 4, 5.6, 7, 8, "a", 10),
          TypeTest.S1(-10, 0xffff, -1234567, 42L, 1.234, 5.678, 0, "z", 255)]
    s  = s * 1000
    r  = io.complex39(s)

    if r[1].b == 0xffff:
        tresult("+")
    else:
        ok = 0
        tresult("-")

    if ok: tpass()
    else:  tfail()


    tstart("Fixed")

    ok = 1
    f = TypeTest.F1(123456789)
    r = io.complex40(f)
    if r == f:
        tresult("+")
    else:
        ok = 0
        tresult("-")
        
    try:
        f  = CORBA.fixed("123456")
        r  = io.complex40(f)
        ok = 0
        tresult("-")
    except CORBA.DATA_CONVERSION:
        tresult("+")

    if ok: tpass()
    else:  tfail()


    tstart("WString")
    ok = 1
    s = u"Hello there"
    r = io.complex41(s)
    if r == s:
        tresult("+")
    else:
        tresult("-")
        ok = 0
    
    s = u"This is a much longer string with lots of stuff in it. Blah blah blah"
    r = io.complex41(s)
    if r == s:
        tresult("+")
    else:
        tresult("-")
        ok = 0

    s = u"This is a string with a \0 in it."
    try:
        r = io.complex41(s)
        tresult("-")
        ok = 0
    except CORBA.BAD_PARAM:
        tresult("+")

    if ok: tpass()
    else:  tfail()

    tstart("Bounded wstring")
    ok = 1
    s = u"Hello"
    r = io.complex42(s)
    if r == s:
        tresult("+")
    else:
        tresult("-")
        ok = 0
    
    s = u"This is a much longer string with lots of stuff in it. Blah blah blah"
    try:
        r = io.complex42(s)
        tresult("-")
        ok = 0
    except CORBA.MARSHAL:
        tresult("+")

    s = u"str\0ing"
    try:
        r = io.complex42(s)
        tresult("-")
        ok = 0
    except CORBA.BAD_PARAM:
        tresult("+")

    if ok: tpass()
    else:  tfail()


    tstart("ValueType")
    ok = 1
    v1 = TypeTest.V1("hello", 5)
    r = io.complex43(v1)
    if r.s == v1.s and r.l == v1.l:
        tresult("+")
    else:
        ok = 0
        tresult("-")

    r = io.complex43(None)
    if r is None:
        tresult("+")
    else:
        ok = 0
        tresult("-")

    v2 = TypeTest.V2("two", 42, v1)
    r = io.complex43(v2)
    if r.s == v2.s and r.l == v2.l:
        tresult("+")
    else:
        ok = 0
        tresult("-")

    r = io.complex43(None)
    if r is None:
        tresult("+")
    else:
        ok = 0
        tresult("-")

    r = io.complex44(v2)
    if r.s == v2.s and r.l == v2.l and r.v.s == v1.s:
        tresult("+")
    else:
        ok = 0
        tresult("-")

    v2.v = v2

    r = io.complex43(v2)
    if r.s == v2.s and r.l == v2.l:
        tresult("+")
    else:
        ok = 0
        tresult("-")

    r = io.complex44(v2)
    if r.s == v2.s and r.l == v2.l and r.v.s == v2.s:
        tresult("+")
    else:
        ok = 0
        tresult("-")

    r = io.complex44(None)
    if r is None:
        tresult("+")
    else:
        ok = 0
        tresult("-")

    if ok: tpass()
    else:  tfail()

    tstart("ValueBox")
    ok = 1

    r = io.complex45(1234)
    if r == 1234:
        tresult("+")
    else:
        ok = 0
        tresult("-")

    r = io.complex45(None)
    if r is None:
        tresult("+")
    else:
        ok = 0
        tresult("-")

    if ok: tpass()
    else:  tfail()

    tstart("Exceptions")

    ok = 1
    try:
        io.except1()
        ok = 0
        tresult("-")
    except TypeTest.X1:
        tresult("+")

    try:
        io.except2(1)
        ok = 0
        tresult("-")
    except TypeTest.X1:
        tresult("+")

    try:
        io.except2(2)
        ok = 0
        tresult("-")
    except TypeTest.X2, ex:
        tresult("+" + ex.b)

    try:
        io.except2(3)
        tresult("+")
    except:
        ok = 0
        tresult("-")

    try:
        io.except3(1)
        ok = 0
        tresult("-")
    except CORBA.UNKNOWN:
        tresult("+")

    try:
        io.except3(2)
        ok = 0
        tresult("-")
    except CORBA.NO_PERMISSION:
        tresult("+")

    try:
        io.except3(3)
        tresult("+")
    except:
        ok = 0
        tresult("-")

    if ok: tpass()
    else:  tfail()


    tstart("TypeCode")
    ok = 1

    io.tc1(CORBA._tc_long);    tresult("+")
    io.tc1(CORBA._tc_short);   tresult("+")
    io.tc1(CORBA._tc_boolean); tresult("+")
    io.tc1(CORBA._tc_string);  tresult("+")

    io.tc1(TypeTest._tc_J);   tresult("+")
    io.tc1(TypeTest._tc_S1);  tresult("+")
    io.tc1(TypeTest._tc_U1);  tresult("+")
    io.tc1(TypeTest._tc_U2);  tresult("+")
    io.tc1(TypeTest._tc_E1);  tresult("+")
    io.tc1(TypeTest._tc_E2);  tresult("+")

    io.tc1(TypeTest._tc_Q1);  tresult("+")
    io.tc1(TypeTest._tc_Q2);  tresult("+")
    io.tc1(TypeTest._tc_Q3);  tresult("+")
    io.tc1(TypeTest._tc_Q4);  tresult("+")
    io.tc1(TypeTest._tc_Q5);  tresult("+")
    io.tc1(TypeTest._tc_Q6);  tresult("+")
    io.tc1(TypeTest._tc_Q7);  tresult("+")
    io.tc1(TypeTest._tc_Q8);  tresult("+")
    io.tc1(TypeTest._tc_Q9);  tresult("+")
    io.tc1(TypeTest._tc_Q10); tresult("+")

    io.tc1(TypeTest._tc_BQ1); tresult("+")
    io.tc1(TypeTest._tc_BQ2); tresult("+")
    io.tc1(TypeTest._tc_BQ3); tresult("+")
    io.tc1(TypeTest._tc_BQ4); tresult("+")
    io.tc1(TypeTest._tc_BQ5); tresult("+")
    io.tc1(TypeTest._tc_BQ6); tresult("+")
    io.tc1(TypeTest._tc_BQ7); tresult("+")
    io.tc1(TypeTest._tc_BQ8); tresult("+")
    io.tc1(TypeTest._tc_BQ9); tresult("+")

    io.tc1(TypeTest._tc_A1);  tresult("+")
    io.tc1(TypeTest._tc_A2);  tresult("+")
    io.tc1(TypeTest._tc_A3);  tresult("+")
    io.tc1(TypeTest._tc_A4);  tresult("+")
    io.tc1(TypeTest._tc_A5);  tresult("+")
    io.tc1(TypeTest._tc_A6);  tresult("+")
    io.tc1(TypeTest._tc_A7);  tresult("+")
    io.tc1(TypeTest._tc_A8);  tresult("+")
    io.tc1(TypeTest._tc_A9);  tresult("+")
    io.tc1(TypeTest._tc_A10); tresult("+")

    io.tc1(TypeTest._tc_S2);  tresult("+")

    r = io.tc1(TypeTest._tc_S3)
    if r.equivalent(TypeTest._tc_S3):
        tresult("+")
    else:
        ok = 0
        tresult("-")

    io.tc1(TypeTest._tc_X1);  tresult("+")
    io.tc1(TypeTest._tc_X2);  tresult("+")
    io.tc1(TypeTest._tc_I);   tresult("+")
    io.tc1(TypeTest._tc_F1);  tresult("+")

    if ok: tpass()
    else:  tfail()


    tstart("Any")
    ok = 1

    a = CORBA.Any(TypeTest._tc_S1,
                  TypeTest.S1(1, 2, 3, 4, 5.6, 7.8, 1, "a", 9))
    r = io.any1(a)
    if r.value().c == 3:
        tresult("+")
    else:
        ok = 0
        tresult("-")

    i1 = TypeTest.S3(42, TypeTest.S3.U(0,None))
    i2 = TypeTest.S3(5,  TypeTest.S3.U(a=[i1]))
    i3 = TypeTest.S3(17, TypeTest.S3.U(a=[i2, i1]))
    a  = CORBA.Any(TypeTest._tc_S3, i3)
    r  = io.any1(a)

    if r.value().b.a[0].a == 5:
        tresult("+")
    else:
        ok = 0
        tresult("-")

    ji = J_i()
    jo = ji._this()
    a  = CORBA.Any(TypeTest._tc_J, jo)
    r  = io.any1(a)
    
    if r.value()._is_equivalent(jo):
        tresult("+")
    else:
        ok = 0
        tresult("-")

    id = poa.reference_to_id(jo)
    del ji, jo
    r = None
    poa.deactivate_object(id)

    a = CORBA.Any(CORBA._tc_IMP_LIMIT,
                  CORBA.IMP_LIMIT(12345,CORBA.COMPLETED_YES))
    r = io.any1(a)
    tresult(str(r.value()))

    a = CORBA.Any(CORBA._tc_OBJECT_NOT_EXIST,
                  CORBA.OBJECT_NOT_EXIST(omniORB.OBJECT_NOT_EXIST_NoMatch,
                                         CORBA.COMPLETED_YES))
    r = io.any1(a)
    tresult(str(r.value()))

    if ok: tpass()
    else:  tfail()


    tstart("Invalid Any")
    ok = 1

    a = CORBA.Any(TypeTest._tc_S1,
                  TypeTest.S1(1, 2, "Hi", 4, 5.6, 7.8, 1, "a", 9))
    try:
        r  = io.any1(a)
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    i1 = TypeTest.S3(42, TypeTest.S3.U(a="Hi"))
    i2 = TypeTest.S3(5,  TypeTest.S3.U(a=[i1]))
    i3 = TypeTest.S3(17, TypeTest.S3.U(a=[i2, i1]))
    a  = CORBA.Any(TypeTest._tc_S3, i3)

    try:
        r  = io.any1(a)
        ok = 0
        tresult("-")
    except CORBA.BAD_PARAM:
        tresult("+")

    if ok: tpass()
    else:  tfail()


    tstart("Empty Any")
    a = CORBA.Any(CORBA._tc_null, None)

    try:
        r = io.any1(a)
        tpass()
    except:
        tfail()


    tstart("Context")
    ok = 1
    try:
        ctxt = orb.get_default_context()
        ctxt.set_one_value("test", "hello")
        ctxt.set_one_value("test2", "there")
        ctxt.set_values({"foo": "wib", "foo2": "wob", "foobarbaz": "wuz"})

        r = io.context1(5, ctxt)
        if r == ["test", "hello"]:
            tresult("+")
        else:
            ok = 0
            tresult("-")

        r = io.context2(5, ctxt)
        if len(r) == 10:
            tresult("+")
        else:
            ok = 0
            tresult("-")
    except:
        ok = 0
        tresult("!")

    if ok:
        tpass()
    else:
        tfail()

    tdone()


if __name__ == "__main__":
    orb = CORBA.ORB_init(sys.argv, CORBA.ORB_ID)
    poa = orb.resolve_initial_references("RootPOA")
    poa._get_the_POAManager().activate()

    io = orb.string_to_object(sys.argv[1])

    doTests(orb, poa, io)

    if "-r" in sys.argv:
        print "\nRepeating tests..."
        output = 0
        while 1:
            doTests(orb, poa, io)

    orb.destroy()
