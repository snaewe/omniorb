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
    raise "tyutil.principalID deprecated"
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
        # escape all escapes
        text = re.sub(r"_", "__", text)
        return re.sub(r"\W", "_", text)
    scopedName = map(escapeNonAlphanumChars, scopedName)

    # all but the identifier have _m appended (signifies a module?)
    scope = map(lambda x: x + "_m", scopedName[0:-1])
    guard = reduce(lambda x,y: x + y, scope, "") + scopedName[-1]
    
    return guard

# ------------------------------------------------------------------

def objRefTemplate(type, suffix, environment):
    name = type.decl().scopedName()
    rel_name = environment.relName(name)
    objref_rel_name = self.scope(rel_name) +\
                      ["_objref_" + self.name(rel_name)]

    rel_name_string = environment.nameToString(rel_name)
    objref_rel_name_string = environment.nameToString(objref_rel_name)
    
    return "_CORBA_ObjRef_" + suffix + \
           "<" + objref_rel_name_string + ", " + rel_name_string + "_Helper>"

# ------------------------------------------------------------------

def operationArgumentType(type, environment, virtualFn = 0):
    outer_env = environment.leaveScope()
    environment = outer_env
    
    param_type = environment.principalID(type)
    isVariable = isVariableType(type)
    type_dims = typeDims(type)
    is_array = type_dims != []
    
    deref_type = deref(type)

    if is_array and isVariable:
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
            return [ "char *",
                     "const char* ",
                     "CORBA::String_out ",
                     "char*& " ]
        elif isObjRef(deref_type):
            param_type = environment.principalID(deref_type)
            return [ param_type + "_ptr",
                     param_type + "_ptr",
                     "_CORBA_ObjRef_OUT_arg<_objref_" + param_type + "," + \
                     param_type + "_Helper >",
                     param_type + "_ptr&" ]
        else:
            pass
            # same as the other kind
            
    if isinstance(deref_type, idltype.String):
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
        param_type = environment.principalID(deref_type)
        return [ param_type + "_ptr",
                 param_type + "_ptr",
                 "_CORBA_ObjRef_OUT_arg<_objref_" + param_type + "," + \
                 param_type + "_Helper >",
                 "_CORBA_ObjRef_INOUT_arg<_objref_" + param_type + "," + \
                 param_type + "_Helper >" ,
                 param_type + "_Helper >" ]                 
                 #param_type + "_ptr&" ]
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

def sequenceTemplate(sequence, environment):
    # returns a template instantiation suitable for the
    # sequence type
    # (similar in function to o2be_sequence::seq_template_name)
    assert isinstance(sequence, idltype.Sequence)

    seqType = sequence.seqType()
    seqTypeID = environment.principalID(seqType)
    derefSeqType = deref(seqType)
    derefSeqTypeID = environment.principalID(derefSeqType)
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
            template["derefSeqTypeID"] = sequenceTemplate(derefSeqType,
                                                          environment)
            
    
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
       rel_name = environment.relName(scopedName)
       objref_name = environment.nameToString(rel_name)
       #scopedName = idlutil.pruneScope(scopedName, scope)
       #objref_name = idlutil.ccolonName(map(mapID, scopedName))
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
        target_name = target_scope + [str(value)]
        rel_name = environment.relName(target_name)
        rel_name_string = environment.nameToString(rel_name)
        return rel_name_string
#        return environment.principalID(rel_name_string)
#       rel_scope = idlutil.pruneScope(target_scope, from_scope)
#       return idlutil.ccolonName(rel_scope) + str(value)
    if type.kind() == idltype.tk_string:
        return value
    if type.kind() == idltype.tk_octet:
        return str(value)

    raise "Cannot convert a value (" + repr(value) + ") of type: " +\
          repr(type) + " into a string."
        
        
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
            size = typeSizeAlignMap[type.kind()]
            
            if size == 1:
                string.out("""\
@sizevar@ += 1""", sizevar = sizevar)
                return str(string)

            string.out("""\
@sizevar@ = omni::align_to(@sizevar@, omni::ALIGN_@n@) + @n@""",
                       sizevar = sizevar, n = str(size))
            return str(string)

        # FIXME:
        if 0:
            if isString(deref_type):
                string.out("""\
@sizevar@ = omni::align_to(@sizevar, omni::ALIGN_4) + 4;
@sizevar@ += ((const char*) @argname@) + 1 : 1""",
                           sizevar = sizevar, argname = argname)
                return str(string)

        # FIXME:
        # this corresponds to case tObjref in the old BE
        # what is the difference between tObjrefMember and tObjref?
        if 0:
            if isObjRef(deref_type):
                name = environment.principalID(deref_type)
                string.out("""\
@sizevar@ = @name@_Helper::NP_alignedSize(@argname@, @sizevar@)""",
                           sizevar = sizevar, name = name, argname = argname)
                return str(string)

        # typecodes may be an exception here
        string.out("""\
@sizevar@ = @argname@._NP_alignedSize(@sizevar@)""",
                   sizevar = sizevar, argname = argname)
        return str(string)

    def begin_loop(string = string, full_dims = full_dims):
        string.out("{")
        string.inc_indent()
        index = 0
        indexing_string = ""
        for dim in full_dims:
            string.out("""\
for (CORBA::ULong _i@n@ = 0;_i < @dim@;_i@n@++) {""",
                       n = str(index), dim = str(dim))
            string.inc_indent()
            indexing_string = indexing_string + "[_i" + str(index) + "]"
            index = index + 1

        return indexing_string

    def end_loop(string = string, full_dims = full_dims):
        for dim in full_dims:
            string.dec_indent()
            string.out("}")
            
        string.dec_indent()
        string.out("}")
        

    # thing is an array
    if not(isVariable):
        if typeSizeAlignMap.has_key(type.kind()):
            size = typeSizeAlignMap[type.kind()]

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
        indexing_string = begin_loop()

        # do the actual calculation
        string.out("""\
  @sizevar@ = @argname@@indexing_string@._NP_alignedSize(@sizevar@);""",
                   sizevar = sizevar, argname = argname,
                   indexing_string = indexing_string)

        end_loop()

        return str(string)


    # thing is an array of variable sized elements
    indexing_string = begin_loop()

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
               

    end_loop()

    return str(string)
    


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
