#!/usr/bin/env python

import sys, os
import CORBA, PortableServer
import Fortune, Fortune__POA

FORTUNE_PATH = "/usr/games/fortune"

class CookieServer_i (Fortune__POA.CookieServer):
    def get_cookie(self):
        pipe   = os.popen(FORTUNE_PATH)
        cookie = pipe.read()
        if pipe.close():
            # An error occurred with the pipe
            raise Fortune.Failure("popen of fortune failed")
        return cookie

# By default, listen on port 2809, the default corbaloc port, so
# clients can access the object with corbaloc::host.name/fortune
if "-ORBendPoint" not in sys.argv:
    sys.argv.extend(["-ORBendPoint", "giop:tcp::2809"])

orb = CORBA.ORB_init(sys.argv)
poa = orb.resolve_initial_references("omniINSPOA")

servant = CookieServer_i()
poa.activate_object_with_id("fortune", servant)

poa._get_the_POAManager().activate()
orb.run()
