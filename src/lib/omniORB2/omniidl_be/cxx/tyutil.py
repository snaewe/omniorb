# -*- python -*-
#                           Package   : omniidl
# tyutil.py                 Created on: 1999/11/3
#			    Author    : David Scott (djs)
#
#    Copyright (C) 1999 AT&T Laboratories Cambridge
#
#  This file is part of omniidl.
#
#  omniidl is free software; you can redistribute it and/or modify it
#  under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
#  02111-1307, USA.
#
# Description:
#   
#   General utility functions designed for the C++ backend

# $Id$
# $Log$
# Revision 1.30.2.8  2000/07/17 09:36:16  djs
# Added function to strip typedef chains from AST nodes
# Fixed allInherits() function to handle inheriting from a typedef to an
# interface
#
# Revision 1.30.2.7  2000/06/26 16:23:11  djs
# Added new backend arguments.
# Better error handling when encountering unsupported IDL (eg valuetypes)
# Refactoring of configuration state mechanism.
#
# Revision 1.30.2.6  2000/04/26 18:22:14  djs
# Rewrote type mapping code (now in types.py)
# Rewrote identifier handling code (now in id.py)
#
# Revision 1.30.2.5  2000/03/24 16:25:10  djs
# Added in " "s around "<" and ">" in templates to make the output parse
# correctly when they are nested.
#
# Revision 1.30.2.4  2000/03/13 16:00:38  djs
# Fixed problem with scoping (was emulating old problem)
#
# Revision 1.30.2.3  2000/03/10 16:16:32  djs
# Improper handling of scopes when performing name prefixes in ObjRef_OUT
# arguments fixed.
#
# Revision 1.30.2.2  2000/03/09 15:21:41  djs
# Better handling of internal compiler exceptions (eg attempts to use
# wide string types)
#
# Revision 1.30.2.1  2000/02/14 18:34:57  dpg1
# New omniidl merged in.
#
# Revision 1.30  2000/01/19 11:21:35  djs
# *** empty log message ***
#
# Revision 1.29  2000/01/18 13:05:57  djs
# Nice new memberType function
#
# Revision 1.28  2000/01/17 16:59:31  djs
# Better support for outputting {char, string} values in a C++ friendly way
#
# Revision 1.27  2000/01/13 18:16:19  djs
# Added check to mapRepoID function- it is only there to emulate broken
# behaviour.
#
# Revision 1.26  2000/01/13 11:45:47  djs
# Added option to customise C++ reserved word name escaping
#
# Revision 1.25  2000/01/12 19:54:47  djs
# Added option to generate old CORBA 2.1 signatures for skeletons
#
# Revision 1.24  2000/01/11 12:02:34  djs
# More tidying up
#
# Revision 1.23  2000/01/11 11:35:46  djs
# Removed redundant code
#
# Revision 1.22  2000/01/10 15:39:34  djs
# Better name and scope handling.
#
# Revision 1.21  2000/01/07 20:31:18  djs
# Regression tests in CVSROOT/testsuite now pass for
#   * no backend arguments
#   * tie templates
#   * flattened tie templates
#   * TypeCode and Any generation
#
# Revision 1.20  1999/12/26 16:40:28  djs
# Better char value -> C++ char constant handling (needs rewriting)
# Typedef to Struct / Union passed as operation argument fix
#
# Revision 1.19  1999/12/25 21:44:58  djs
# Better TypeCode support
#
# Revision 1.18  1999/12/16 16:08:54  djs
# Added allInherits function to return a list of all the interfaces an
# interface inherits from (ie under the transitive closure of the inherits
# relation)
#
# Revision 1.17  1999/12/15 12:12:31  djs
# Fix building string form of ulong constants
#
# Revision 1.16  1999/12/14 17:38:13  djs
# Fixed anonymous sequences of sequences bug
#
# Revision 1.15  1999/12/14 11:53:57  djs
# Support for CORBA::TypeCode and CORBA::Any
#
# Revision 1.14  1999/12/09 20:41:25  djs
# Now runs typecode and any generator
#
# Revision 1.13  1999/12/01 17:04:47  djs
# Added utility function useful for Typecode declarations
#
# Revision 1.12  1999/11/29 19:26:59  djs
# Code tidied and moved around. Some redundant code eliminated.
#
# Revision 1.11  1999/11/23 18:49:26  djs
# Lots of fixes, especially marshalling code
# Added todo list to keep track of issues
#
# Revision 1.10  1999/11/19 20:06:30  djs
# Removed references to a removed utility function
#
# Revision 1.9  1999/11/17 20:37:09  djs
# General util functions
#
# Revision 1.8  1999/11/15 19:10:55  djs
# Added module for utility functions specific to generating skeletons
# Union skeletons working
#
# Revision 1.7  1999/11/12 17:17:46  djs
# Creates output files rather than using stdout
# Utility functions useful for skeleton generation added
#
# Revision 1.6  1999/11/10 20:19:31  djs
# Option to emulate scope bug in old backend
# Array struct element fix
# Union sequence element fix
#
# Revision 1.5  1999/11/08 19:28:56  djs
# Rewrite of sequence template code
# Fixed lots of typedef problems
#
# Revision 1.4  1999/11/04 19:05:02  djs
# Finished moving code from tmp_omniidl. Regression tests ok.
#
# Revision 1.3  1999/11/03 17:35:07  djs
# Brought more of the old tmp_omniidl code into the new tree
#
# Revision 1.2  1999/11/03 12:06:33  djs
# Type utility functions now work with the new AST representation
#
# Revision 1.1  1999/11/03 11:09:50  djs
# General module renaming
#

"""General utility functions specific to the C++ backend"""

from omniidl import idlutil, idltype, idlast

from omniidl_be.cxx import util, id

import string, re

import tyutil
self = tyutil


# ------------------------------------------------------ DEPRECATED FUNCTIONS
#
be_nice = 0

def deprecated(fn):
    if be_nice: return
    
    print "Function " + fn.__name__ + " is now deprecated"
    print fn.__doc__


def deref(type):
    """tyutil.deref(idltype.Type): idltype.Type should be replaced by
       calling the deref() method of a types.Type object
       eg
         # wrap type
         new_type = types.Type(idltype_type_instance)
         # new operation
         deref_new_type = new_type.deref()
         # unwrap type
         deref_idltype_instance = deref_new_type.type()"""
    assert isinstance(type, idltype.Type)
    deprecated(deref)
    
    return types.Type(type).deref().type()

def derefKeepDims(type):
    """tyutil.derefKeepDims(idltype.Type): idltype.Type should be replaced
       by calling the deref(keep_dims = 1) method of a types.Type object
       (see tyutil.deref.__doc__)"""
    assert isinstance(type, idltype.Type)
    deprecated(derefKeepDims)
    
    return types.Type(type).deref(keep_dims = 1).type()

def mapID(identifier):
    """tyutil.mapID(identifier string): identifier string should be replaced
       by a call to id.mapID(identifier)"""
    assert isinstance(identifier, string)
    deprecated(mapID)
    
    return id.mapID(identifier)


def mapRepoID(id):
    """tyutil.mapRepoID(repoID string) : repoID string has no useful purpose
       anylonger- it served to emulate an old bug where omniidl{2,3} would
       break repository IDs eg
        IDL:Module/If/Then:1.0 -> IDL:Module/_cxx_If/_cxx_Then:1.0"""
    deprecated(mapRepoID)
    assert(0)


def isVariableType(type, self):
    """tyutil.isVariableType(idltype.Type) : boolean should be replaced by
       a call to the .variable() method of a types.Type object"""
    deprecated(isVariableType)

    return types.Type(type).variable()

def isVariableDecl(node):
    """tyutil.isVariableDecl(idlast.Decl) : boolean should be replaced by
       a call to types.variableDecl(idlast.Decl)"""
    deprecated(isVariableDecl)
    
    return types.variableDecl(node)

def typeDims(type):
    """tyutil.typeDims(idltype.Type): int dims list should be replaced by
       a call to the .dims() method of a types.Type object"""
    deprecated(typeDims)

    return types.Type(type).dims()

def escapeChars(text):
    """tyutil.escapeChars(string): string has been removed"""
    deprecated(escapeChars)
    assert(0)
    
def guardName(scopedName):
    """tyutil.guardName(scopedName string list): string guard name has
       been replaced by the .guard() method of a id.Name object"""
    deprecated(guardName)

    return id.Name(scopedName).guard()

def principalID(environment, type, fully_qualify = 0):
    """tyutil.principalID(environment, idltype.Type, fully_qualify boolean)
       has been replaced by methods in a types.Type object eg:
          types.Type.base(environment = None): a simple C++ type for this type
          types.Type.member(environment = None): a member type
       (This needs more engineering)"""
    deprecated(principalID)

    assert(0)
    
def memberType(environment, type, decl = None):
    """tyutil.memberType(environment, type, decl = None)
       has been replaced by methods in a types.Type object. See
       tyutil.principalID.__doc__ for details"""
    deprecated(memberType)

    assert(0)
    
def objRefTemplate(type, suffix, environment):
    """tyutil.objRefTemplate(idltype.Type, suffix string, environment) has
       been replaced by the .objRefTemplate(suffix, environment) method of a
       types.Type object"""
    deprecated(objRefTemplate)

    return types.Type(type).objRefTemplate(suffix, environment)

def operationArgumentType(type, environment, virtualFn = 0):
    """tyutil.operationArgumentType(idltype.Type, environment, boolean virtual)
       has been replaced by the .op(direction, environment = None, use_out = 1)
       method of the types.Type object. Note that the old function returned a
       tuple of mappings (one entry per direction) the new function has an
       extra direction argument.

       Inconsistencies abound with the use of out types. This needs further
       engineering"""
    deprecated(operationArgumentType)

    assert(0)
    
def makeConstructorArgumentType(type, environment, decl = None):
    """tyutil.makeConstructorArgumentType(idltype.Type, environment, decl)
       has been removed. New code uses mapping functions of the types.Type
       objects with inlined special cases. Further investigation/
       simplification desirable"""
    deprecated(makeConstructorArgumentType)

    assert(0)

def templateToString(template):
    """tyutil.templateToString(template hash)
       was an internal function and has been hidden inside the types.Type
       object"""
    deprecated(templateToString)

    assert(0)

def sequenceTemplate(sequence, environment):
    """tyutil.sequenceTemplate(sequence idltype.Type, environment)
       has been replaced with the .sequenceTemplate(environment) method of
       the types.Type object"""
    deprecated(sequenceTemplate)

    return types.Type(sequence).sequenceTemplate(environment)

def valueString(type, value, environment):
    """tyutil.valueString(idltype.Type, value, environment)
       has been replaced with the .literal(value, environment) method of
       the types.Type object"""
    deprecated(valueString)

    return types.Type(type).literal(value, environment)

def const_init_in_def(type):
    """tyutil.const_init_in_def(idltype.Type): boolean
       has been replaced with the .representable_by_int() method of the
       types.Type object"""
    deprecated(const_init_in_def)

    return types.Type(type).representable_by_int()

def isInteger(type, force_deref = 0):
    """tyutil.isInteger(idltype.Type): boolean
       has been replaced with the .integer() method of the types.Type object
       (the force_deref parameter has been removed)"""
    deprecated(isInteger)
    assert(not(force_deref))

    return types.Type(type).integer()

def isChar(type, force_deref = 0):
    """tyutil.isChar(idltype.Type): boolean
       has been replaced with the .char() method of the types.Type object
       (the force_deref parameter has been removed)"""
    deprecated(isChar)
    assert(not(force_deref))

    return types.Type(type).char()


def isFloating(type, force_deref = 0):
    """tyutil.isFloating(idltype.Type): boolean
       has been replaced with the following types.Type methods:
          types.Type.float(): boolean
          types.Type.double(): boolean
       (the force_deref parameter has been removed)"""
    deprecated(isFloating)
    assert(not(force_deref))
    t = types.Type(type)
    return t.float() or t.double()

def isBoolean(type, force_deref = 0):
    """tyutil.isBoolean(idltype.Type): boolean
       has been replaced with the .boolean() method of the types.Type object
       (the force_deref parameter has been removed)"""
    deprecated(isBoolean)
    assert(not(force_deref))

    return types.Type(type).boolean()


def isEnum(type, force_deref = 0):
    """tyutil.isEnum(idltype.Type): boolean
       has been replaced with the .enum() method of the types.Type object
       (the force_deref parameter has been removed)"""
    deprecated(isEnum)
    assert(not(force_deref))

    return types.Type(type).enum()


def isOctet(type, force_deref = 0):
    """tyutil.isOctet(idltype.Type): boolean
       has been replaced with the .octet() method of the types.Type object
       (the force_deref parameter has been removed)"""
    deprecated(isOctet)
    assert(not(force_deref))

    return types.Type(type).octet()


def isString(type, force_deref = 0):
    """tyutil.isString(idltype.Type): boolean
       has been replaced with the .string() method of the types.Type object
       (the force_deref parameter has been removed)"""
    deprecated(isString)
    assert(not(force_deref))

    return types.Type(type).string()


def isObjRef(type, force_deref = 0):
    """tyutil.isObjRef(idltype.Type): boolean
       has been replaced with the .objref() method of the types.Type object
       (the force_deref parameter has been removed)"""
    deprecated(isObjRef)
    assert(not(force_deref))

    return types.Type(type).objref()


def isSequence(type, force_deref = 0):
    """tyutil.isSequence(idltype.Type): boolean
       has been replaced with the .sequence() method of the types.Type object
       (the force_deref parameter has been removed)"""
    deprecated(isSequence)
    assert(not(force_deref))

    return types.Type(type).sequence()


def isTypeCode(type, force_deref = 0):
    """tyutil.isTypeCode(idltype.Type): boolean
       has been replaced with the .typecode() method of the types.Type object
       (the force_deref parameter has been removed)"""
    deprecated(isTypeCode)
    assert(not(force_deref))

    return types.Type(type).typecode()


def isTypedef(type, force_deref = 0):
    """tyutil.isTypedef(idltype.Type): boolean
       has been replaced with the .typedef() method of the types.Type object
       (the force_deref parameter has been removed)"""
    deprecated(isTypedef)
    assert(not(force_deref))

    return types.Type(type).typedef()


def isStruct(type, force_deref = 0):
    """tyutil.isStruct(idltype.Type): boolean
       has been replaced with the .struct() method of the types.Type object
       (the force_deref parameter has been removed)"""
    deprecated(isStruct)
    assert(not(force_deref))

    return types.Type(type).struct()


def isUnion(type, force_deref = 0):
    """tyutil.isUnion(idltype.Type): boolean
       has been replaced with the .union() method of the types.Type object
       (the force_deref parameter has been removed)"""
    deprecated(isUnion)
    assert(not(force_deref))

    return types.Type(type).union()


def isVoid(type, force_deref = 0):
    """tyutil.isVoid(idltype.Type): boolean
       has been replaced with the .void() method of the types.Type object
       (the force_deref parameter has been removed)"""
    deprecated(isVoid)
    assert(not(force_deref))

    return types.Type(type).void()


def isAny(type, force_deref = 0):
    """tyutil.isAny(idltype.Type): boolean
       has been replaced with the .any() method of the types.Type object
       (the force_deref parameter has been removed)"""
    deprecated(isAny)
    assert(not(force_deref))

    return types.Type(type).any()


# -----------------------------------------------END OF DEPRECATED FUNCTIONS


# Info on size and alignment of basic types
typeSizeAlignMap = {
    idltype.tk_char:    (1, 1),
    idltype.tk_boolean: (1, 1),
    idltype.tk_wchar:   (2, 2),
    idltype.tk_short:   (2, 2),
    idltype.tk_ushort:  (2, 2),
    idltype.tk_long:    (4, 4),
    idltype.tk_ulong:   (4, 4),
    idltype.tk_float:   (4, 4),
    idltype.tk_enum:    (4, 4),
    idltype.tk_double:  (8, 8),
    idltype.tk_octet:   (1, 1)
    }


def dimsToString(dims, prefix = ""):
    """tyutil.dimsToString: dims string list * optional prefix -> dims string
       Function takes a list of int dimensions (eg [1,2,3]) and a possible
       prefix string and returns a string [prefix + 1][prefix + 2][prefix + 3]
       """
    new_dims = []
    for x in dims:
        new_dims.append("[" + prefix + repr(x) + "]")
    append = lambda x,y: x + y
    return reduce(append, new_dims, "")


def sizeCalculation(environment, type, decl, sizevar, argname):
    """tyutil.sizeCalculation: environment * idltype.Type * idlast Declarator
                        * sizevar string * argname string -> calc string
       Returns block of code which will work out the size of an argument
       type (for marshalling purposes)"""
    # o2be_operation::produceSizeCalculation
    assert isinstance(type, idltype.Type)
    assert isinstance(decl, idlast.Declarator)

    d_type = type.deref()

    dims = decl.sizes()
    type_dims = type.dims()
    full_dims = dims + type_dims

    anonymous_array = dims      != []
    is_array        = full_dims != []
    alias_array     = type_dims != []

    num_elements = reduce(lambda x,y:x*y, full_dims, 1)

    isVariable = type.variable()

    string = util.StringStream()

    if not(is_array):
        if typeSizeAlignMap.has_key(type.type().kind()):
            size = typeSizeAlignMap[type.type().kind()][0]
            
            if size == 1:
                string.out("""\
@sizevar@ += 1""", sizevar = sizevar)
                return str(string)

            string.out("""\
@sizevar@ = omni::align_to(@sizevar@, omni::ALIGN_@n@) + @n@""",
                       sizevar = sizevar, n = str(size))
            return str(string)

        # typecodes may be an exception here
        string.out("""\
@sizevar@ = @argname@._NP_alignedSize(@sizevar@)""",
                   sizevar = sizevar, argname = argname)
        return str(string)


    # thing is an array
    if not(isVariable):
        if typeSizeAlignMap.has_key(type.type().kind()):
            size = typeSizeAlignMap[type.type().kind()][0]

            if size == 1:
                string.out("""\
@sizevar@ += @num_elements@""", num_elements = num_elements)
                return str(string)

            string.out("""\
@sizevar@ = omni::align_to(@sizevar@, omni::ALIGN_@n@);
@sizevar@ += @num_elements@ * @n@""",
                       sizevar = sizevar,
                       n = size,
                       num_elements = num_elements)
            return str(string)

        # must be an array of fixed structs or unions
        indexing_string = util.begin_loop(string, full_dims)

        # do the actual calculation
        string.out("""\
  @sizevar@ = @argname@@indexing_string@._NP_alignedSize(@sizevar@);""",
                   sizevar = sizevar, argname = argname,
                   indexing_string = indexing_string)

        end_loop(string, full_dims)

        return str(string)


    # thing is an array of variable sized elements
    indexing_string = begin_loop(string, full_dims)

    if d_type.string():
        string.out("""\
@sizevar@ = omni::align_to(@sizevar@, omni::ALIGN_4);
@sizevar@ += 4 + (((const char*) @argname@@indexing_string@)? strlen((const char*) @argname@@indexing_string@) + 1 : 1);""",
                   sizevar = sizevar, argname = argname,
                   indexing_string = indexing_string)
        

    elif d_type.objref():
        #name = environment.principalID(deref_type)
        name = d_type.base(environment)
        string.out("""\
@sizevar@ = @name@_Helper::NP_alignedSize(@argname@@indexing_string@._ptr,@sizevar@);""",
                   sizevar = sizevar, name = name, argname = argname,
                   indexing_string = indexing_string)

    else:
        # typecodes may be an exception here
        string.out("""\
@sizevar@ = @argname@@indexing_string@._NP_alignedSize(@sizevar@);""",
                   sizevar = sizevar, argname = argname,
                   indexing_string = indexing_string)
               

    end_loop(string, full_dims)

    return str(string)
    

def allCases(node):
    """tyutil.allCases(idlast.Decl): idlast.UnionCaseLabel list
       Returns a list of all the non-default union case labels"""
    assert isinstance(node, idlast.Union)
    list = []
    for n in node.cases():
        for l in n.labels():
            if not(l.default()):
                list.append(l)
    return list


def getDefaultCaseAndMark(node):
    """tyutil.getDefaultCaseAndMark(idlast.Union): option idlast.UnionCase
       Returns the default case of a union (if it exists) and sets the
       isDefault attribute on the default case"""
    assert isinstance(node, idlast.Union)
    default = None
    for c in node.cases():
        c.isDefault = 0
        for l in c.labels():
            if l.default():
                default = c
                c.isDefault = 1
    return default


def getDefaultLabel(case):
    """tyutil.getDefaultLabel(idlast.UnionCase): boolean
       Returns true if the union case has the default label"""
    assert isinstance(case, idlast.UnionCase)
    for l in case.labels():
        if l.default():
            return l
    assert 0


def exhaustiveMatch(switchType, allCaseValues):
    """tyutil.exhaustiveMatch(types.Type, case value list): boolean
       Returns whether the supplied list of case label values
       represents an exhaustive match for this switch type. IE is it
       not possible to have a value:switchType for which there is no
       corresponding union case"""
    
    # dereference the switch_type (ie if CASE <scoped_name>)
    switchType = switchType.deref()
            
    # CASE <integer_type>
    if switchType.integer():
        # Assume that we can't possibly do an exhaustive match
        # on the integers, since they're (mostly) very big.
        # maybe should rethink this for the short ints...
        return 0
    # CASE <char_type>
    elif switchType.char():
        # make a set with all the characters inside
        s = []
        for char in range(0,256):
            s.append(chr(char))
        # make a set with all the used characters
        used = allCaseValues
        # subtract all the used ones from the possibilities
        difference = util.minus(s, used)
        # if the difference is empty, match is exhaustive
        return (len(difference) == 0)
    # CASE <boolean_type>
    elif switchType.boolean():
        s = [0, 1]
        used = map(lambda x: x.value(), allCaseValues)
        difference = util.minus(s, used)
        return (len(difference) == 0)
    # CASE <enum_type>
    elif switchType.enum():
        s = switchType.type().decl().enumerators()
        used = map(lambda x: x.value(), allCaseValues)
        difference = util.minus(s, used)
        return (len(difference) == 0)
    else:
        util.fatalError("Internal error processing union")
        raise "exhaustiveMatch type="+repr(switchType)+ \
              " val="+repr(discrimvalue)


def const_qualifier(insideModule, insideClass):
    """tyutil.const_qualifier(insideModule boolean, insideClass boolean
       : string
       Returns the prefix required for the const declaration (depends on
       scoping)"""
    if not(insideModule) and not(insideClass):
        return "_CORBA_GLOBAL_VAR"
    elif insideClass:
        return "static"
    else:
        return "_CORBA_MODULE_VAR"

# Return the base AST node after following all the typedef chains
def remove_ast_typedefs_(node, recurse):
    if isinstance(node, idlast.Declarator):
        typedef = node.alias()
        return recurse(typedef.aliasType().decl(), recurse)
    return node

def remove_ast_typedefs(node, chain = remove_ast_typedefs_):
    return chain(node, chain)


def allInherits(interface):
    """tyutil.allInherits(idlast.Interface) -> idlast.Interface list
       Performs a breadth first search of an interface's inheritance
       heirarchy. Returns the _set_ (ie no duplicates) of ancestor
       interfaces"""
    assert isinstance(interface, idlast.Interface)

    # It is possible to inherit from an interface through a chain of
    # typedef nodes. These need to be removed...
    
    # breadth first search
    def bfs(current, bfs, remove_typedefs = remove_ast_typedefs):
        if current == []:
            return []
        
        # extend search one level deeper than current
        next = []
        for c in map(remove_typedefs, current):
            next = next + c.inherits()

        return next + bfs(next, bfs)

    start = map(remove_ast_typedefs, interface.inherits())
    list = start + bfs(start, bfs)

    return util.setify(list)

