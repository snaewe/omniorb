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

# Contents list:
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

from omniidl.be.cxx import util

import re

import tyutil
self = tyutil

def deref(type):
    assert isinstance(type, idltype.Type)
    while type.kind() == idltype.tk_alias:
        type = type.decl().alias().aliasType()
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
            return "_cxx_" + identifier
    return identifier

# ------------------------------------------------------------------

# Basic IDL types
#
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
    idltype.tk_void:       "void"
    }

def principalID(type, from_scope=[]):
    assert isinstance(type, idltype.Type)
    # check if type is a basic type first
    if ttsMap.has_key(type.kind()):
        return ttsMap[type.kind()]

    if isinstance(type, idltype.String):
        return "CORBA::String_member"
    if isinstance(type, idltype.Sequence):
        return principalID(type.seqType(), from_scope)

    # ----- IMPLEMENT ME -----
    if isinstance(type, idltype.WString):
        raise "No code for wide strings"
    if isinstance(type, idltype.Fixed):
        raise "No code for Fixed type"
    # ----- IMPLEMENT ME -----

    scopedName = type.scopedName()

    scopedName = idlutil.pruneScope(scope(scopedName), from_scope) + \
                 [name(scopedName)]
    
    # escape all components of the name
    scopedName = map(mapID, scopedName)

    return util.delimitedlist(scopedName, "::")

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

def guardName(scopedName):
    def escapeNonAlphanumChars(text):
        return re.sub(r"\W","_",text)
    scopedName = map(escapeNonAlphanumChars, scopedName)

    # all but the identifier have _m appended (signifies a module?)
    scope = map(lambda x: x + "_m", scopedName[0:-1])
    guard = reduce(lambda x,y: x + y, scope, "") + scopedName[-1]
    
    return guard

# ------------------------------------------------------------------

def objRefTemplate(type, suffix, scope = []):
    name = type.decl().scopedName()
    name = idlutil.pruneScope(name, scope)
    name = map(mapID, name)
    objref_name = idlutil.ccolonName(self.scope(name) + \
                                     ["_objref_" + self.name(name)])
    obj_name = idlutil.ccolonName(name)
    return "_CORBA_ObjRef_" + suffix + \
           "<" + objref_name + ", " + obj_name + "_Helper>"

# ------------------------------------------------------------------

def operationArgumentType(type, scope = [], virtualFn = 0):
    param_type = principalID(type, scope)
    isVariable = isVariableType(type)
    deref_type = deref(type)

#    print "[[[ type = " + repr(type) + " scope = " + repr(scope) + "]]]"
#    print "[[[ param_type = "+ repr(param_type) + "]]]"
#    print "[[[ isVariable = " + repr(isVariable) + "]]]"

    if virtualFn:
        if isinstance(type, idltype.String):
            return [ "char *",
                     "const char* ",
                     "CORBA::String_out ",
                     "char*& " ]            
        elif type.kind() == idltype.tk_objref:
            return [ param_type + "_ptr",
                     param_type + "_ptr",
                     "_CORBA_ObjRef_OUT_arg<_objref_" + param_type + "," + \
                     param_type + "_Helper >",
                     param_type + "_ptr&" ]
        else:
            pass
            # same as the other kind
    if isinstance(type, idltype.String):
        return [ "char *",
                 "const char* ",
                 "CORBA::String_out ",
                 "CORBA::String_INOUT_arg " ]
    elif isinstance(deref_type, idltype.Base) or \
         deref_type.kind() == idltype.tk_enum:
        return [ param_type,
                 param_type,
                 param_type + "& ",
                 param_type + "& " ]
    elif deref_type.kind() == idltype.tk_objref:
        param_type =principalID(deref_type, scope)
        return [ param_type + "_ptr",
                 param_type + "_ptr",
                 "_CORBA_ObjRef_OUT_arg<_objref_" + param_type + "," + \
                 param_type + "_Helper >",
                 "_CORBA_ObjRef_INOUT_arg<_objref_" + param_type + "," + \
                 param_type + "_Helper >" ,
                 param_type + "_Helper >" ]                 
    elif isVariable:
 #       param_type =principalID(deref_type, scope)
        return [ param_type + "*",
                 "const " + param_type + "& ",
                 param_type + "_out ",
                 param_type + "& "]
    else:
        return [ param_type,
                 "const " + param_type + "& ",
                 param_type + "& ",
                 param_type + "& " ]
    
# ------------------------------------------------------------------

# This needs to be redesigned. It's too much like the old backend.

typeSizeAlignMap = {
    idltype.tk_char:    (1, 1),
    idltype.tk_wchar:   (2, 2),
    idltype.tk_short:   (2, 2),
    idltype.tk_ushort:  (2, 2),
    idltype.tk_long:    (4, 4),
    idltype.tk_ulong:   (4, 4),
    idltype.tk_float:   (4, 4),
    idltype.tk_enum:    (4, 4),
    idltype.tk_double:  (8, 8)
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
    args_string = util.delimitedlist(args)
#    print "[[[ args = " + repr(args_string) + "]]]"
    if (args_string != ""):
        return name + "<" + args_string + ">"
    return name

def sequenceTemplate(sequence, scope=[]):
    # returns a template instantiation suitable for the
    # sequence type
    # (similar in function to o2be_sequence::seq_template_name)
    assert isinstance(sequence, idltype.Sequence)

    seqType = sequence.seqType()
    seqTypeID = principalID(seqType, scope)
    derefSeqType = deref(seqType)
    derefSeqTypeID = principalID(derefSeqType, scope)
    seq_dims = typeDims(seqType)
    is_array = seq_dims != []
    dimension = reduce(lambda x,y: x * y, seq_dims, 1)

    template = {}
    template["bounded"]   = sequence.bound()
    template["array"]     = is_array
    template["dimension"] = dimension

    template["seqTypeID"] = seqTypeID
    template["derefSeqTypeID"] = derefSeqTypeID


    if is_array:
        if isSequence(derefSeqType):
            template["derefSeqTypeID"] = sequenceTemplate(derefSeqType, scope)
            
    
    if isBoolean(derefSeqType):
        template["suffix"] = "__Boolean"
    # strings are always special
    if isString(derefSeqType) and not(is_array):
        template["suffix"] = "__String"
    if isOctet(derefSeqType):
        template["suffix"] = "__Octet"
                    
    if typeSizeAlignMap.has_key(derefSeqType.kind()):
        template["fixed"] = typeSizeAlignMap[derefSeqType.kind()]
        
    if isObjRef(derefSeqType):
       scopedName = derefSeqType.decl().scopedName()
       scopedName = self.scope(scopedName) + \
                    ["_objref_" + self.name(scopedName)]
       scopedName = idlutil.pruneScope(scopedName, scope)
       objref_name = idlutil.ccolonName(map(mapID, scopedName))
       objref_template = objRefTemplate(derefSeqType, "Member", scope)
       template["objref_name"]     = objref_name
       template["objref_template"] = objref_template
       template["objref_helper"]   = seqTypeID + "_Helper"
       template["objref"]          = 1

       

    return templateToString(template)

    
        
    
def sequenceTemplate_old(sequence, scope=[]):
    # returns a template instantiation suitable for the
    # sequence type
    # (similar in function to o2be_sequence::seq_template_name)
    if not(isinstance(sequence, idltype.Sequence)):
        raise "sequenceTemplate called with a non-sequence: " + repr(sequence)
    base_type = deref(sequence.seqType())
    base_type_name = principalID(base_type, scope)
    type_name = principalID(sequence.seqType(), scope)
    kind = base_type.kind()
    dims = typeDims(sequence.seqType())
    is_array = (dims != [])

    dimension = reduce(lambda x,y: x * y, dims, 1)
    
    is_variable = isVariableType(base_type)
    
    template_args = []
    template_name = "(should have a template name for " + repr(sequence) + ")"
    if (sequence.bound()):
        CORBA_SEQUENCE = "_CORBA_Bounded_Sequence"
        template_args.append(str(sequence.bound()))
    else:
        CORBA_SEQUENCE = "_CORBA_Unbounded_Sequence"

    if is_array:
        CORBA_SEQUENCE = CORBA_SEQUENCE + "_Array"


    # simple fixed types (char, integer types, real types, enum types)
    def fixedSizeElements(elmsize, alignment, prefix = CORBA_SEQUENCE,
                          type_name = type_name,
                          args = template_args):
        args = [type_name] + args
        args = args + [str(elmsize), str(alignment)]
        template = prefix + "_w_FixSizeElement"
        return (template, args)
    def fixedSizeArrayElements(elmsize, alignment, prefix = CORBA_SEQUENCE,
                               type_name = type_name,
                               dimension = dimension,
                               base_type_name = base_type_name,
                               args = template_args):
        args = [type_name, type_name + "_slice",
                base_type_name, str(dimension)] + args
        args = args + [str(elmsize), str(alignment)]
        template = prefix + "_w_FixSizeElement"
        return (template, args)
        

    if not(is_array):
        if isBoolean(base_type):
            template_name = CORBA_SEQUENCE + "__Boolean"
        elif isOctet(base_type):
            template_name = CORBA_SEQUENCE + "__Octet"
        elif isString(base_type):
            template_name = CORBA_SEQUENCE + "__String"
        elif isObjRef(base_type):
            scopedName = base_type.decl().scopedName()
            scopedName = self.scope(scopedName) + \
                         ["_objref_" + self.name(scopedName)]
            scopedName = idlutil.pruneScope(scopedName, scope)
            objref_name = idlutil.ccolonName(map(mapID, scopedName))
            template_name = CORBA_SEQUENCE + "_ObjRef"
            template_args = [objref_name,
                             objRefTemplate(base_type, "Member", scope),
                             type_name + "_Helper"] +\
                             template_args
        # a basic type or an enum with a fixed size
        else:
            try:
                (size, alignment) = typeSizeAlignMap[base_type.kind()]
                (template_name, template_args) = \
                                fixedSizeElements(size, alignment)
            except KeyError:
                template_name = CORBA_SEQUENCE
                template_args = [type_name] + template_args
        
    #CORBA_SEQUENCE = CORBA_SEQUENCE + "_Array"
    
    # arrays of fixed length elements
    if is_array:
        if not(is_variable):
            if isBoolean(base_type):
                template_name = CORBA_SEQUENCE + "__Boolean"
                template_args = [type_name, type_name + "_slice",
                                 str(dimension)] + template_args
            elif isOctet(base_type):
                template_name = CORBA_SEQUENCE + "__Octet"
                template_args = [type_name, type_name + "_slice",
                                 str(dimension)] + template_args
                # simple types go here again

            else:
                try:
                    (size, alignment) = typeSizeAlignMap[base_type.kind()]
                    (template_name, template_args) = \
                                    fixedSizeArrayElements(size, alignment)
                except KeyError:
                    pass
#                    raise "sequenceTemplate of an unknown non-variable " + \
#                          "type: " + repr(base_type) + " kind = " + \
#                          repr(base_type.kind())
                
        elif is_variable:
            if isObjRef(base_type):
                scopedName = base_type.decl().scopedName()
                scopedName = self.scope(scopedName) + \
                             ["_objref_" + self.name(scopedName)]
                scopedName = idlutil.pruneScope(scopedName, scope)
                objref_name = idlutil.ccolonName(map(mapID, scopedName))
                template_args = [type_name, type_name + "_slice",
                                 objRefTemplate(base_type, "Member", scope),
                                 str(dimension)] + template_args
            else:
                if isString(base_type):
                    pass
                elif isSequence(base_type):
                    base_type_name = sequenceTemplate(base_type, scope)
                    pass
                elif isTypeCode(base_type):
                    pass
                else:
                    pass
                template_name = CORBA_SEQUENCE
                template_args = [type_name, type_name + "_slice",
                                 base_type_name, str(dimension)] + template_args
        
    # any handling goes here
#    print "[[[ args = " + repr(template_args) + "]]]"
    template_args = util.delimitedlist(template_args)
    if (template_args != ""):
        return template_name + "<" + template_args + ">"
    return template_name



# ------------------------------------------------------------------

def valueString(type, value, from_scope = []):
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
        return str(int(eval(str(value))))
    if type.kind() == idltype.tk_float     or \
       type.kind() == idltype.tk_double:
        return str(value)
    # chars are single-quoted
    if type.kind() == idltype.tk_char      or \
       type.kind() == idltype.tk_wchar:
        return "'" + value + "'"
    # booleans are straightforward
    if type.kind() == idltype.tk_boolean:
        return str(value)
    if type.kind() == idltype.tk_enum:
        value = name(value.scopedName())
        target_scope = scope(type.decl().scopedName())
        rel_scope = idlutil.pruneScope(target_scope, from_scope)
        return idlutil.ccolonName(rel_scope) + str(value)
    if type.kind() == idltype.tk_string:
        return value
    if type.kind() == idltype.tk_octet:
        return str(value)

    raise "Cannot convert a value (" + repr(value) + ") of type: " +\
          repr(type) + " into a string."
        
        

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
    return type.kind() == idltype.tk_typecode

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

# ------------------------------------------------------------------

def scope(scopedName):
    return scopedName[0:-1]

def name(scopedName):
    return scopedName[-1]
