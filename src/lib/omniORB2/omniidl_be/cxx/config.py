
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
            # Do we make all the objref methods virtual
            'Virtual Objref Methods':0,
            # Are #included files output inline with the main output?
            'Inline Includes':       0,
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
if not hasattr(config, "state"):
    config.state = ConfigurationState()

# list of all files #included in the IDL
includes = []

# Traverses the AST compiling the list of files #included at top level
# in the main IDL file. Marks all nodes for which code should be
# generated.

class WalkTreeForIncludes(idlvisitor.AstVisitor):
    def __init__(self):
        global includes
        includes = []

    def add(self, node):
        global includes
        file = node.file()
        if not file in includes:
            includes.append(file)

    def visitAST(self, node):
        global state
        self.add(node)
        for d in node.declarations():
            if not state['Inline Includes']:
                self.add(d)
            if d.mainFile() or state['Inline Includes']:
                d.accept(self)

    def visitModule(self, node):
        node.cxx_generate = 1
        for n in node.definitions():
            n.accept(self)

    def visitInterface(self, node):
        node.cxx_generate = 1
        for n in node.contents():
            n.accept(self)

    def visitForward(self, node):
        node.cxx_generate = 1

    def visitConst(self, node):
        node.cxx_generate = 1

    def visitDeclarator(self, node):
        node.cxx_generate = 1

    def visitTypedef(self, node):
        node.cxx_generate = 1
        for n in node.declarators():
            n.accept(self)

        if node.constrType():
            node.aliasType().decl().accept(self)

    def visitMember(self, node):
        node.cxx_generate = 1
        for n in node.declarators():
            n.accept(self)

        if node.constrType():
            node.memberType().decl().accept(self)

    def visitStruct(self, node):
        node.cxx_generate = 1
        for n in node.members():
            n.accept(self)

    def visitException(self, node):
        node.cxx_generate = 1
        for n in node.members():
            n.accept(self)

    def visitCaseLabel(self, node):
        node.cxx_generate = 1

    def visitUnionCase(self, node):
        node.cxx_generate = 1
        for n in node.labels():
            n.accept(self)

        if node.constrType():
            node.caseType().decl().accept(self)

    def visitUnion(self, node):
        node.cxx_generate = 1
        for n in node.cases():
            n.accept(self)

        if node.constrType():
            node.switchType().decl().accept(self)

    def visitEnumerator(self, node):
        node.cxx_generate = 1

    def visitEnum(self, node):
        node.cxx_generate = 1
        for n in node.enumerators():
            n.accept(self)

    def visitAttribute(self, node):
        node.cxx_generate = 1
        for n in node.declarators():
            n.accept(self)

    def visitParameter(self, node):
        node.cxx_generate = 1

    def visitOperation(self, node):
        node.cxx_generate = 1
        for n in node.parameters():
            n.accept(self)

    def visitNative(self, node):
        node.cxx_generate = 1

    def visitStateMember(self, node):
        node.cxx_generate = 1
        for n in node.declarators():
            n.accept(self)

        if node.constrType():
            node.memberType().decl().accept(self)

    def visitFactory(self, node):
        node.cxx_generate = 1
        for n in node.parameters():
            n.accept(self)

    def visitValueForward(self, node):
        node.cxx_generate = 1

    def visitValueBox(self, node):
        node.cxx_generate = 1

        if node.constrType():
            node.boxedType().decl().accept(self)

    def visitValueAbs(self, node):
        node.cxx_generate = 1
        for n in node.contents():
            n.accept(self)

    def visitValue(self, node):
        node.cxx_generate = 1
        for n in node.contents():
            n.accept(self)

def shouldGenerateCodeForDecl(decl):

    """Return true if full code should be generated for the specified
    Decl node"""

    return hasattr(decl, "cxx_generate")
