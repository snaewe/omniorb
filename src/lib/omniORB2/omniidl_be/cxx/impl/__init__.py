# This is a module

import os

from omniidl_be.cxx import config, util
from omniidl_be.cxx.impl import main

def run(tree):
    hh_filename = config.basename() + config.hdrsuffix()
    idl_filename = tree.file()
    impl_filename = config.basename() + config.implsuffix()

    stream = util.Stream(open(impl_filename, "w"), 2)
    main.__init__(stream, idl_filename, hh_filename)

    main.run(tree)
