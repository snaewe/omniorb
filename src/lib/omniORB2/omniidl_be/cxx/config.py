
import string

from omniidl import idlvisitor
import config

# Stores the global configuration of the C++ backend and can dump it to
# stdout (for bug reporting)
class ConfigurationState:

    def __init__(self):
        self._config = {
            # Name of this program
            'Program Name':          'omniidl (C++ backend)',
            # Useful data from CVS
            'CVS ID':                '$Id$',
            # Relevant omniORB C++ library version
            'Library Version':       'omniORB_3_0',
            # Suffix of generated header file
            'HH Suffix':             '.hh',
            # Suffix of generated Skeleton file
            'SK Suffix':             'SK.cc',
            # Suffix of generated Dynamic code
            'DYNSK Suffix':          'DynSK.cc',
            # Suffix of example interface implementation code
            'IMPL Suffix':           '_i.cc',
            
            # Are we in "fragment" mode?
            'Fragment':              0,
            # In fragment mode, suffix of definitions file
            '_DEFS Fragment':        '_defs',
            # In fragment mode, suffix of file containing operators (eg <<)
            '_OPERS Fragment':       '_operators',
            # In fragment mode, suffix of file containing POA code
            '_POA Fragment':         '_poa',

            # Private prefix for internal names
            'Private Prefix':        '_0RL',
            # Prefix used to avoid clashes with C++ keywords
            'Reserved Prefix':       '_cxx_',
            # Base name of file being processed
            'Basename':              None,
            # Directory name of file being processed
            'Directory':             None,
            # Do we generate code for TypeCodes and Any?
            'Typecode':              0,
            # Do we splice reopened modules together into one large chunk?
            # (not guaranteed to always work)
            'Splice Modules':        0,
            # Do we generate example code implementing all of the interfaces
            # found in the input IDL?
            'Example Code':          0,
            # Do we generate normal (non-flattened) tie templates?
            'Normal Tie':            0,
            # Do we generate flattened tie templates?
            'Flattened Tie':         0,
            # Do we generate BOA compatible skeleton classes?
            'BOA Skeletons':         0,
            # Do we generate old CORBA 2.1 signatures for skeletons?
            'Old Signatures':        0,
            # Do we preserve the #include'd IDL path name in the generated
            # header (eg #include <A/B.idl> -> #include <A/B.hh>?
            'Keep Include Path':     0,
            # Do we #include files using double-quotes rather than
            # angled brackets (the default)
            'Use Quotes':            0,
            # Are we in DEBUG mode?
            'Debug':                 0
                       
            }

    def __getitem__(self, key):
        if self._config.has_key(key):
            return self._config[key]
        util.fatalError("Configuration key not found (" + key + ")")

    def __setitem__(self, key, value):
        if self._config.has_key(key):
            self._config[key] = value
            return
        util.fatalError("Configuration key not found (" + key + ")")

    def dump(self):
        # find the longest key string
        max = 0
        for key in self._config.keys():
            if len(key) > max: max = len(key)
        # display the table
        for key in self._config.keys():
            print string.ljust(key, max), ":  ", repr(self._config[key])

# Create state-holding singleton object
if not(hasattr(config, "state")):
    config.state = ConfigurationState()

# list of all files #included in the IDL
includes = []

# Traverses the AST compiling the list of files #included by the main
# IDL file. Note that types constructed within other types must necessarily
# be in the same IDL file
class WalkTreeForIncludes(idlvisitor.AstVisitor):
    def __init__(self):
        config.includes = []
    def add(self, node):
        file = node.file()
        if not(file in config.includes):
            config.includes = [file] + config.includes

    def visitAST(self, node):
        self.add(node)
        for d in node.declarations(): d.accept(self)
    def visitModule(self, node):
        self.add(node)
        for d in node.definitions(): d.accept(self)
    def visitInterface(self, node): self.add(node)
    def visitForward(self, node):   self.add(node)
    def visitConst(self, node):     self.add(node)
    def visitTypedef(self, node):   self.add(node)
    def visitStruct(self, node):    self.add(node)
    def visitException(self, node): self.add(node)
    def visitUnion(self, node):     self.add(node)
    def visitEnum(self, node):      self.add(node)
    

