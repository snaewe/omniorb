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
#   Type utility functions designed for the C++ backend

# $Id$
# $Log$
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

"""Type utility functions specific to the C++ backend"""

# Contents list: (out of date)
#
# deref : type -> type
#   Returns the most dereferenced type (ie recurses along chains
#   of typedefs)
#
# mapID : identifier -> identifier
#   Returns a legal C++ identifier corresponding to an IDL identifier
#   (escapes C++ keywords etc)
#
# principalID : type * scope -> string
#   Maps basic types onto their C++ counterparts and maps all other
#   types onto scope::identifier
#
# objRefTemplate : type * suffix * scope -> string
#   Returns a template instance corresponding to a reference to the
#   argument type
#
# sequenceTemplate : sequence type * scope -> string
#   Returns a template instance corresponding to a sequence
#   eg _CORBA_Bounded_sequence.....
#
# isVariableType : type -> bool
# isVariableDecl : decl -> bool
#   Returns true if the representation of the type will occupy a
#   variable amount of storage (eg strings)
#
# typeDims : type -> int list
#   Recovers the full dimensions of a type, by recursing through
#   array typedef declarators
#
# guardName : scoped name -> string
#   Returns a string useful as an #ifdef style guard. Escapes non-
#   alphanumeric chars
#
# operationArgumentType : type * scope * boolean -> string
#   Returns a list containing [return type, in type, out type, inout type]
#   for a specific type. The boolean argument is true for the virtual
#   functions of the _impl_<name> class
#
# isInteger   : type * boolean -> boolean
# isChar      : type * boolean -> boolean
# isFloating  : type * boolean -> boolean
# isBoolean   : type * boolean -> boolean
# isEnum      : type * boolean -> boolean
# isOctet     : type * boolean -> boolean
# isString    : type * boolean -> boolean
# isObjRef    : type * boolean -> boolean
# isSequence  : type * boolean -> boolean
# isTypeCode  : type * boolean -> boolean
# isTypedef   : type * boolean -> boolean
# isStruct    : type * boolean -> boolean
# isUnion     : type * boolean -> boolean
#   Type of type :) functions. If boolean argument is true, will
#   perform a full dereference first. Default is false.
#
# scope : scoped name -> scope
#   Returns the scope part of the scoped name
#
# name : scoped name -> name
#   Returns the name part of the scoped name
#
from omniidl import idlutil, idltype, idlast

from omniidl.be.cxx import util, config

import string, re

import tyutil
self = tyutil

def deref(type):
    assert isinstance(type, idltype.Type)
    while type.kind() == idltype.tk_alias:
        type = type.decl().alias().aliasType()
    return type

# dereference type but keep dimensions (ie don't deref across
# array declarators)
def derefKeepDims(type):
    assert isinstance(type, idltype.Type)

    while tyutil.isTypedef(type):
        decl = type.decl()
        if decl.sizes() != []:
            return type
        type = decl.alias().aliasType()

    return type

# ------------------------------------------------------------------

# List of C++ reserved words, minus IDL reserved words
#
reservedWords = [
    "and", "and_eq", "asm", "auto", 
    "bool", "bitand", "bitor", "break", 
    "catch",  "class", "compl", "const_cast", "continue",
    "delete", "do", "dynamic_cast",
    "else", "explicit", "export", "extern",
    "false", "for", "friend", 
    "goto", "if", "inline", "int", 
    "mutable", 
    "namespace", "new", "not", "not_eq",
    "operator", "or", "or_eq",
    "private", "protected", "public",
    "register", "reinterpret_cast", "return",
    "signed", "sizeof", "static", "static_cast",
    "template", "this", "throw", "true", "try", "typeid", "typename",
    "using", 
    "virtual", "volatile", "wchar_t", "while",
    "xor", "xor_eq" ]

def mapID(identifier):
    for i in reservedWords:
        if i == identifier:
            return config.reservedPrefix() + identifier
    return identifier



# It appears that the old compiler will map names in repository IDs
# to avoid supposed clashes with C++ keywords, but this is totally
# broken
# eg mapRepoID("IDL:Module/If/Then") -> "IDL:Module/_cxx_If/_cxx_Then"
def mapRepoID(id):
    if not(config.EMULATE_BUGS()):
        raise RuntimeError("Shouldn't be _breaking_ repository IDs if " +\
                           "not emulating bugs in the old backend!")
    # extract the naming part of the ID
    regex = re.compile(r"(IDL:)*(.+):(.+)")
    match = regex.match(id)
    first_bit = match.group(1)
    if not(first_bit):
        first_bit = ""
    the_name = match.group(2)
    ver = match.group(3)
    # extract the name 
    elements = re.split(r"/", the_name)
    mapped_elements = []
    for element in elements:
        mapped_elements.append(tyutil.mapID(element))
    # put it all back together again
    return first_bit + string.join(mapped_elements, "/") + ":" + ver



# ------------------------------------------------------------------

ttsMap = {
    # From C++ Language Mapping - Mapping for Basic Data Types - June 1999
    # 1-15
    idltype.tk_short:      "CORBA::Short",
    idltype.tk_long:       "CORBA::Long",
    idltype.tk_longlong:   "CORBA::LongLong",
    idltype.tk_ushort:     "CORBA::UShort",
    idltype.tk_ulong:      "CORBA::ULong",
    idltype.tk_ulonglong:  "CORBA::ULongLong",
    idltype.tk_float:      "CORBA::Float",
    idltype.tk_double:     "CORBA::Double",
    idltype.tk_longdouble: "CORBA::LongDouble",
    idltype.tk_char:       "CORBA::Char",
    idltype.tk_wchar:      "CORBA::WChar",
    idltype.tk_boolean:    "CORBA::Boolean",
    idltype.tk_octet:      "CORBA::Octet",
    #
    idltype.tk_void:       "void",
    idltype.tk_any:        "CORBA::Any",
    #idltype.tk_TypeCode:   "CORBA::TypeCode"
    
    }


# ------------------------------------------------------------------

# An entry in this table indicates we already know is a type is
# variable or not, without having to look at its declaration.
already_Variable = {
    idltype.tk_null:               0,
    idltype.tk_void:               0,
    idltype.tk_short:              0,
    idltype.tk_long:               0,
    idltype.tk_ushort:             0,
    idltype.tk_ulong:              0,
    idltype.tk_float:              0,
    idltype.tk_double:             0,
    idltype.tk_boolean:            0,
    idltype.tk_char:               0,
    idltype.tk_octet:              0,
    idltype.tk_any:                1,
    idltype.tk_objref:             1,
    idltype.tk_string:             1,
    idltype.tk_sequence:           1,
    idltype.tk_except:             1,
    idltype.tk_longlong:           0,
    idltype.tk_ulonglong:          0,
    idltype.tk_longdouble:         0,
    idltype.tk_wchar:              0,
    idltype.tk_wstring:            1,
    idltype.tk_fixed:              0,          
    idltype.tk_value:              1,      #?
    idltype.tk_value_box:          1,      #?
    idltype.tk_abstract_interface: 1,
    }

def isVariableType(type):
    assert isinstance(type, idltype.Type)
    
    if isinstance(type, idltype.Base):
        if type.kind() == idltype.tk_any or \
           type.kind() == idltype.tk_TypeCode:
            return 1
        return 0
    elif isinstance(type, idltype.Sequence) or \
         isinstance(type, idltype.Fixed)    or \
         isinstance(type, idltype.String):
        return 1
    elif (isinstance(type, idltype.Declared)):
        return isVariableDecl(type.decl())
    else:
        raise ("An illegal type: " + repr(type))

def isVariableDecl(node):
    assert isinstance(node, idlast.Decl)
    
    # interfaces are mapped to objects, which are always
    # variable types. same goes for exceptions.
    if isinstance(node, idlast.Interface)       or \
       isinstance(node, idlast.Forward)         or \
       isinstance(node, idlast.Exception):
        return 1

    elif isinstance(node, idlast.Const)            or \
         isinstance(node, idlast.Enum):
        return 0

    # a typedef is only a type alias- as such it has no storage
    # at all. However it eventually points to something that would.
    elif isinstance(node, idlast.Typedef):
        return isVariableType(node.aliasType())
        
        
    # a structure is variable if any one of its constituents
    # is also variable
    elif isinstance(node, idlast.Struct):
        for n in node.members():
            if isVariableType(n.memberType()):
                return 1
        return 0
    
    # a union is variable if any one if its constituents
    # is also variable
    elif isinstance(node, idlast.Union):
        for c in node.cases():
            if isVariableType(c.caseType()):
                return 1
        return 0

    # a declarator is variable if it is an alias to a variable
    # type
    elif isinstance(node, idlast.Declarator) and \
         node.alias() != None:
        return isVariableType(node.alias().aliasType())

    else:
        raise "util.isVariable called with a " + repr(node) + \
              ". Case match incomplete."

# ------------------------------------------------------------------

def typeDims(type):
    assert isinstance(type, idltype.Type)
    
    if isinstance(type, idltype.Declared):
        tyname = type.name()
        if type.kind() == idltype.tk_alias:
            tydecl = type.decl()
            sizes = []
            if tydecl.sizes() != None:
                sizes = tydecl.sizes()
            if tydecl.alias() != None:
                sizes = sizes + typeDims(tydecl.alias().aliasType())
            return sizes

        # no arrays at this level
        if (hasattr(type.decl(), "aliasType")):
            return typeDims(type.decl().aliasType())
        else:
            return []
    return []

# ------------------------------------------------------------------
def escapeChars(text):
    # escape all escapes
    text = re.sub(r"_", "__", text)
    return re.sub(r"\W", "_", text)
    
def guardName(scopedName):
    scopedName = map(escapeChars, scopedName)

    # all but the identifier have _m appended (signifies a module?)
    scope = map(lambda x: x + "_m", scopedName[0:-1])
    guard = reduce(lambda x,y: x + y, scope, "") + scopedName[-1]
    
    return guard

# ------------------------------------------------------------------

def objRefTemplate(type, suffix, environment):
    name = type.decl().scopedName()
    if name == ["CORBA", "Object"]:
        return "CORBA::Object_member"
    
    rel_name = environment.relName(name)
    objref_rel_name = self.scope(rel_name) +\
                      ["_objref_" + self.name(rel_name)]

    rel_name_string = environment.nameToString(rel_name)
    objref_rel_name_string = environment.nameToString(objref_rel_name)
    
    return "_CORBA_ObjRef_" + suffix + \
           "<" + objref_rel_name_string + ", " + rel_name_string + "_Helper>"

# ------------------------------------------------------------------

def operationArgumentType(type, environment, virtualFn = 0):
    param_type = environment.principalID(type)
    isVariable = isVariableType(type)
    type_dims = typeDims(type)
    is_array = type_dims != []
    
    deref_type = deref(type)

    if is_array and isVariable:
        if config.OldFlag() and virtualFn:
            return [ param_type + "_slice*",
                     "const " + param_type,
                     param_type + "_slice*&",
                     param_type ]
        else:
            return [ param_type + "_slice*",
                     "const " + param_type,
                     param_type + "_out",
                     param_type ]

    if is_array and not(isVariable):

            return [ param_type + "_slice*",
                 "const " + param_type,
                 param_type,
                 param_type ]

        
    if virtualFn:
        if isinstance(deref_type, idltype.String):
            if config.OldFlag():
                return [ "char *",
                         "const char* ",
                         "char*& ",
                         "char*& " ]
            else:
                return [ "char *",
                         "const char* ",
                         "CORBA::String_out ",
                         "char*& " ]
        elif isObjRef(deref_type):
            scopedName = deref_type.scopedName()
            if scopedName == ["CORBA", "Object"]:
                if config.OldFlag():
                    return [ "CORBA::Object_ptr",
                             "CORBA::Object_ptr",
                             "CORBA::Object_ptr&",
                             "CORBA::Object_ptr&" ]
                else:
                    return [ "CORBA::Object_ptr",
                             "CORBA::Object_ptr",
                             "CORBA::Object_OUT_arg",
                             "CORBA::Object_ptr&" ]
            param_type = environment.principalID(deref_type)
            if config.OldFlag():
                return [ param_type + "_ptr",
                         param_type + "_ptr",
                         param_type + "_ptr&",
                         param_type + "_ptr&" ]
            return [ param_type + "_ptr",
                     param_type + "_ptr",
                     "_CORBA_ObjRef_OUT_arg<_objref_" + param_type + "," + \
                     param_type + "_Helper >",
                     param_type + "_ptr&" ]
        elif deref_type.kind() == idltype.tk_TypeCode:
            if config.OldFlag():
                return [ "CORBA::TypeCode_ptr",
                         "CORBA::TypeCode_ptr",
                         "CORBA::TypeCode_ptr&",
                         "CORBA::TypeCode_ptr&" ]
            else:
                return [ "CORBA::TypeCode_ptr",
                         "CORBA::TypeCode_ptr",
                         "CORBA::TypeCode_OUT_arg",
                         "CORBA::TypeCode_ptr&" ]
        else:
            pass
            # same as the other kind

    # typedefs to Anys are a little strange
    if isTypedef(type) and isAny(deref_type):
        if config.OldFlag() and virtualFn:
            return [ param_type + "*",
                     "const " + param_type + "&",
                     param_type + "*&",
                     param_type + "&" ]
        else:
            return [ param_type + "*",
                     "const " + param_type + "&",
                     "CORBA::Any_OUT_arg",
                     param_type + "&" ]
            
    if isinstance(deref_type, idltype.String):
        return [ "char *",
                 "const char* ",
                 "CORBA::String_out ",
                 "CORBA::String_INOUT_arg " ]
    elif deref_type.kind() == idltype.tk_any:
        if config.OldFlag() and virtualFn:
            return [ "CORBA::Any*",
                     "const CORBA::Any&",
                     "CORBA::Any*&",
                     "CORBA::Any&" ]
        else:
            return [ "CORBA::Any*",
                     "const CORBA::Any&",
                     "CORBA::Any_OUT_arg",
                     "CORBA::Any&" ]
    elif deref_type.kind() == idltype.tk_TypeCode:
        return [ "CORBA::TypeCode_ptr",
                 "CORBA::TypeCode_ptr",
                 "CORBA::TypeCode_OUT_arg",
                 "CORBA::TypeCode_INOUT_arg" ]
    elif isinstance(deref_type, idltype.Base) or \
         deref_type.kind() == idltype.tk_enum:
        return [ param_type,
                 param_type,
                 param_type + "& ",
                 param_type + "& " ]
    elif deref_type.kind() == idltype.tk_objref:
        if deref_type.scopedName() == ["CORBA", "Object"]:
            return [ "CORBA::Object_ptr",
                     "CORBA::Object_ptr",
                     "CORBA::Object_OUT_arg",
                     "CORBA::Object_INOUT_arg" ]
        param_type = environment.principalID(deref_type)
        return [ param_type + "_ptr",
                 param_type + "_ptr",
                 "_CORBA_ObjRef_OUT_arg<_objref_" + param_type + "," + \
                 param_type + "_Helper >",
                 "_CORBA_ObjRef_INOUT_arg<_objref_" + param_type + "," + \
                 param_type + "_Helper >" ,
                 param_type + "_Helper >" ]                 
                 #param_type + "_ptr&" ]

    out_base_type = param_type
    if isVariable:
        # Strangeness: if actually a typedef to a struct or union, the _out
        # type is dereferenced, whilst the others aren't?
        if isTypedef(type) and (isStruct(deref_type) or isUnion(deref_type)) and\
           not(config.OldFlag() and virtualFn):
            out_base_type = environment.principalID(deref_type)

        if config.OldFlag() and virtualFn:
            return [ param_type + "*",
                     "const " + param_type + "& ",
                     out_base_type + "*& ",
                     param_type + "&"]
        else:
            return [ param_type + "*",
                     "const " + param_type + "& ",
                     out_base_type + "_out ",
                     param_type + "& "]
    else:
        return [ param_type,
                 "const " + param_type + "& ",
                 out_base_type + "& ",
                 param_type + "& " ]
    
# ------------------------------------------------------------------


# Used to build the types for exception constructors
def makeConstructorArgumentType(type, environment, decl = None):
    # idl.type -> string
    typeName = environment.principalID(type)
    isVariable = tyutil.isVariableType(type)
    derefType = tyutil.deref(type)
    derefTypeName = environment.principalID(derefType)
    dims = typeDims(type)
    decl_dims = []
    if decl != None:
        decl_dims = decl.sizes()
    full_dims = decl_dims + dims
    
    is_array = full_dims != []
    is_array_declarator = decl_dims != []

    if is_array:
        return "const " + typeName

    if isTypeCode(derefType):
        return "CORBA::TypeCode_ptr"
    if isStruct(derefType) or isUnion(derefType) or isAny(derefType):
        return "const " + typeName + "&"
    if isinstance(derefType, idltype.Base) or isEnum(derefType):
        return typeName
    if isObjRef(derefType):
        return derefTypeName + "_ptr"
    if isSequence(type, 0):
        return "const " + sequenceTemplate(derefType, environment)
    elif isSequence(type, 1):
        return "const " + typeName
    else:
        return operationArgumentType(type, environment)[1]





# ------------------------------------------------------------------

# This needs to be redesigned. It's too much like the old backend.

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

# converts a hash of template properties into a template instance
def templateToString(template):
    # ------------------------------------
    # work out the template name
    if template["bounded"]:
        name = "_CORBA_Bounded_Sequence"
    else:
        name = "_CORBA_Unbounded_Sequence"

    if template["array"]:
        name = name + "_Array"
        
    if template.has_key("suffix"):
        name = name + template["suffix"]

    elif template.has_key("objref") and not(template["array"]):
        name = name + "_ObjRef"

    if template.has_key("fixed"):
        name = name + "_w_FixSizeElement"

    # ------------------------------------
    # build the argument list
    args = []

    seqTypeID      = template["seqTypeID"]
    derefSeqTypeID = template["derefSeqTypeID"]
    dimension      = template["dimension"]

    # Note the difference between an ObjRef and an array of ObjRefs
    if template["array"]:
        args = args + [seqTypeID, seqTypeID + "_slice"]
        
        if template.has_key("objref"):
            args = args + [template["objref_template"]]

        elif not(template.has_key("suffix")):
            # __Boolean __Octet __String
            # these already contain the type info- no need for another
            # parameter...
            args = args + [derefSeqTypeID]
            
        args = args + [str(dimension)]
        
    elif template.has_key("objref"):
        args = args + [template["objref_name"],
                       template["objref_template"],
                       template["objref_helper"]]
    elif not(template.has_key("suffix")):
        # see above
        args = args + [seqTypeID]
        


    if template.has_key("bounded") and \
       template["bounded"]:
        args = args + [str(template["bounded"])]

    if template.has_key("fixed"):
        (element_size, alignment) = template["fixed"]
        args = args + [str(element_size), str(alignment)]

    # -----------------------------------
    # build the template instance
    args_string = string.join(args, ", ")
#    print "[[[ args = " + repr(args_string) + "]]]"
    if (args_string != ""):
        name = name + "<" + args_string + ">"
        return name

    return name

def sequenceTemplate(sequence, environment):
    # returns a template instantiation suitable for the
    # sequence type
    # (similar in function to o2be_sequence::seq_template_name)
    assert isinstance(sequence, idltype.Sequence)

    seqType = sequence.seqType()
    derefSeqType = deref(seqType)
    seqTypeID = environment.principalID(seqType)
    derefSeqTypeID = environment.principalID(derefSeqType)
    if tyutil.isTypeCode(derefSeqType):
        derefSeqTypeID = "CORBA::TypeCode_member"
        seqTypeID = "CORBA::TypeCode_member"


    seq_dims = typeDims(seqType)
    is_array = seq_dims != []
    dimension = reduce(lambda x,y: x * y, seq_dims, 1)

    template = {}
    template["bounded"]   = sequence.bound()
    template["array"]     = is_array
    template["dimension"] = dimension

    template["seqTypeID"] = seqTypeID
    template["derefSeqTypeID"] = derefSeqTypeID

    # if the seqType is a typedef to a sequence, use the typedef name
    # else if a direct sequence<sequence<...., do recursion
    if isSequence(derefSeqType) and not(isTypedef(seqType)):
        element_template = sequenceTemplate(derefSeqType, environment)
        template["seqTypeID"] = element_template
        template["derefSeqTypeID"] = element_template

    if is_array:
        if isSequence(derefSeqType):
            template["derefSeqTypeID"] = sequenceTemplate(derefSeqType,
                                                          environment)
            
    
    if isBoolean(derefSeqType):
        template["suffix"] = "__Boolean"
    # strings are always special
    elif isString(derefSeqType) and not(is_array):
        template["suffix"] = "__String"
    elif isOctet(derefSeqType):
        template["suffix"] = "__Octet"
                    
    elif typeSizeAlignMap.has_key(derefSeqType.kind()):
        template["fixed"] = typeSizeAlignMap[derefSeqType.kind()]
        
    elif isObjRef(derefSeqType):
       scopedName = derefSeqType.decl().scopedName()
       is_CORBA_Object = scopedName == ["CORBA", "Object"]

       if not(is_CORBA_Object):
           # CORBA::Object doesn't have an _objref_xxx
           scopedName = self.scope(scopedName) + \
                        ["_objref_" + self.name(scopedName)]
           
       rel_name = environment.relName(scopedName)
       objref_name = environment.nameToString(rel_name)

       objref_template = objRefTemplate(derefSeqType, "Member", environment)
       template["objref_name"]     = objref_name
       template["objref_template"] = objref_template
       template["objref_helper"]   = seqTypeID + "_Helper"
       template["objref"]          = 1

       

    return templateToString(template)

    
# ------------------------------------------------------------------

def valueString(type, value, environment):
    type = deref(type)
    # (unsigned) short ints are themselves
    if type.kind() == idltype.tk_short     or \
       type.kind() == idltype.tk_ushort:
        return str(value)
    # careful with long ints to avoid "L" postfix
    if type.kind() == idltype.tk_long      or \
       type.kind() == idltype.tk_longlong  or \
       type.kind() == idltype.tk_ulong     or \
       type.kind() == idltype.tk_ulonglong:
        s = str(value)
        if s[-1] == 'L':
            return s[0:-1]
        return s
    if type.kind() == idltype.tk_float     or \
       type.kind() == idltype.tk_double:
        return str(value)
    # chars are single-quoted
    if type.kind() == idltype.tk_char      or \
       type.kind() == idltype.tk_wchar:
        # FIXME: need isalphanum() fn and proper formatting
        if ord(value) < 32:
            return r"'\00" + str(ord(value)) + r"'" 
        return "'" + str(value) + "'"
    # booleans are straightforward
    if type.kind() == idltype.tk_boolean:
        return str(value)
    if type.kind() == idltype.tk_enum:
        value = name(value.scopedName())
        target_scope = scope(type.decl().scopedName())
        target_name = target_scope + [str(value)]
        rel_name = environment.relName(target_name)
        rel_name_string = environment.nameToString(rel_name)
        return rel_name_string
    if type.kind() == idltype.tk_string:
        return "\"" + value + "\""
    if type.kind() == idltype.tk_octet:
        return str(value)

    raise "Cannot convert a value (" + repr(value) + ") of type: " +\
          repr(type) + "(kind == " + repr(type.kind()) + ") into a string."
        
        
# ------------------------------------------------------------------


def dimsToString(dims, prefix = ""):
    new_dims = []
    for x in dims:
        new_dims.append("[" + prefix + repr(x) + "]")
    append = lambda x,y: x + y
    return reduce(append, new_dims, "")


# ------------------------------------------------------------------

def sizeCalculation(environment, type, decl, sizevar, argname):
    #o2be_operation::produceSizeCalculation
    assert isinstance(type, idltype.Type)
    assert isinstance(decl, idlast.Declarator)

    deref_type = deref(type)

    dims = decl.sizes()
    type_dims = typeDims(type)
    full_dims = dims + type_dims

    anonymous_array = dims      != []
    is_array        = full_dims != []
    alias_array     = type_dims != []

    num_elements = reduce(lambda x,y:x*y, full_dims, 1)

    isVariable = isVariableType(type)

    string = util.StringStream()

    if not(is_array):
        if typeSizeAlignMap.has_key(type.kind()):
            size = typeSizeAlignMap[type.kind()][0]
            
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
        if typeSizeAlignMap.has_key(type.kind()):
            size = typeSizeAlignMap[type.kind()][0]

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

    if isString(deref_type):
        string.out("""\
@sizevar@ = omni::align_to(@sizevar@, omni::ALIGN_4);
@sizevar@ += 4 + (((const char*) @argname@@indexing_string@)? strlen((const char*) @argname@@indexing_string@) + 1 : 1);""",
                   sizevar = sizevar, argname = argname,
                   indexing_string = indexing_string)
        

    elif isObjRef(deref_type):
        name = environment.principalID(deref_type)
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
    
# ------------------------------------------------------------------


def allCaseValues(node):
    assert isinstance(node, idlast.Union)
    list = []
    for n in node.cases():
        for l in n.labels():
            if not(l.default()):
                list.append(l)
    return list

# marks each case node with isDefault : boolean and returns the
# default case or None if none found
def getDefaultCaseAndMark(node):
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
    assert isinstance(case, idlast.UnionCase)
    for l in case.labels():
        if l.default():
            return l
    assert 0
    # default case must have a default label!

# ------------------------------------------------------------------

# determines whether a set of cases represents an Exhaustive Match
def exhaustiveMatch(switchType, allCaseValues):
    # return all the used case values
    
    # dereference the switch_type (ie if CASE <scoped_name>)
    switchType = tyutil.deref(switchType)
            
    # same as discrimValueToString
    # CASE <integer_type>
    if tyutil.isInteger(switchType):
        # Assume that we can't possibly do an exhaustive match
        # on the integers, since they're (mostly) very big.
        # maybe should rethink this for the short ints...
        return 0
    # CASE <char_type>
    elif tyutil.isChar(switchType):
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
    elif tyutil.isBoolean(switchType):
        s = [0, 1]
        used = map(lambda x: x.value(), allCaseValues)
        difference = util.minus(s, used)
        return (len(difference) == 0)
    # CASE <enum_type>
    elif tyutil.isEnum(switchType):
        s = switchType.decl().enumerators()
        used = map(lambda x: x.value(), allCaseValues)
        difference = util.minus(s, used)
        return (len(difference) == 0)
    else:
        raise "exhaustiveMatch type="+repr(switchType)+ \
              " val="+repr(discrimvalue)


# ------------------------------------------------------------------

# returns whether a constant is initialised in the header file definition
# or in the skeleton code. Equivalent to "is the value representable by
# an integer?"
def const_init_in_def(type):
    assert isinstance(type, idltype.Type)
    return isInteger(type) or isChar(type) or isBoolean(type) or \
           isOctet(type)


# ------------------------------------------------------------------

def const_qualifier(insideModule, insideClass):
    if not(insideModule) and not(insideClass):
        return "_CORBA_GLOBAL_VAR"
    elif insideClass:
        return "static"
    else:
        return "_CORBA_MODULE_VAR"

# ------------------------------------------------------------------

def allInherits(interface):
    assert isinstance(interface, idlast.Interface)
    list = []
    for inherited in interface.inherits():
        list.append(inherited)
        list = list + allInherits(inherited)

    return list

# ------------------------------------------------------------------


def isInteger(type, force_deref = 0):
    if force_deref:
        type = deref(type)
    return type.kind() == idltype.tk_short     or \
           type.kind() == idltype.tk_long      or \
           type.kind() == idltype.tk_longlong  or \
           type.kind() == idltype.tk_ushort    or \
           type.kind() == idltype.tk_ulong     or \
           type.kind() == idltype.tk_ulonglong

def isChar(type, force_deref = 0):
    if force_deref:
        type = deref(type)
    return type.kind() == idltype.tk_char     or \
           type.kind() == idltype.tk_wchar

def isFloating(type, force_deref = 0):
    if force_deref:
        type = deref(type)
    return type.kind() == idltype.tk_float    or \
           type.kind() == idltype.tk_double

def isBoolean(type, force_deref = 0):
    if force_deref:
        type = deref(type)
    return type.kind() == idltype.tk_boolean

def isEnum(type, force_deref = 0):
    if force_deref:
        type = deref(type)
    return type.kind() == idltype.tk_enum

def isOctet(type, force_deref = 0):
    if force_deref:
        type = deref(type)
    return type.kind() == idltype.tk_octet

def isString(type, force_deref = 0):
    if force_deref:
        type = deref(type)
    return isinstance(type, idltype.String)

def isObjRef(type, force_deref = 0):
    if force_deref:
        type = deref(type)
    return isinstance(type, idltype.Declared) and \
           type.kind() == idltype.tk_objref

def isSequence(type, force_deref = 0):
    if force_deref:
        type = deref(type)
    return isinstance(type, idltype.Sequence)

def isTypeCode(type, force_deref = 0):
    if force_deref:
        type = deref(type)
    return type.kind() == idltype.tk_TypeCode

# Ignore the force_deref argument
def isTypedef(type, force_deref = 0):
    return type.kind() == idltype.tk_alias

def isStruct(type, force_deref = 0):
    if force_deref:
        type = deref(type)
    return type.kind() == idltype.tk_struct

def isUnion(type, force_deref = 0):
    if force_deref:
        type = deref(type)
    return type.kind() == idltype.tk_union

def isVoid(type, force_deref = 0):
    if force_deref:
        type = deref(type)
    return type.kind() == idltype.tk_void

def isAny(type, force_deref = 0):
    if force_deref:
        type = deref(type)
    return type.kind() == idltype.tk_any


# ------------------------------------------------------------------

def scope(scopedName):
    return scopedName[0:-1]

def name(scopedName):
    return scopedName[-1]
