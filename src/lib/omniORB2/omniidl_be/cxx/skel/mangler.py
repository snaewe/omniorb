# -*- python -*-
#                           Package   : omniidl
# mangler.py                Created on: 1999/11/16
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
#   Mangle scoped names for proxy skeleton proxy classes
#    - a python conversion of djr's o2be_name_mangle.cc
#                             and   o2be_call_desc.cc        

# $Id$
# $Log$
# Revision 1.13.2.2  2000/02/16 16:30:03  djs
# Fix to proxy call descriptor code- failed to handle special case of
#   Object method(in string x)
#
# Revision 1.13.2.1  2000/02/14 18:34:53  dpg1
# New omniidl merged in.
#
# Revision 1.13  2000/01/13 17:02:05  djs
# Added support for operation contexts.
#
# Revision 1.12  2000/01/13 15:56:44  djs
# Factored out private identifier prefix rather than hard coding it all through
# the code.
#
# Revision 1.11  2000/01/13 14:16:35  djs
# Properly clears state between processing separate IDL input files
#
# Revision 1.10  1999/12/17 10:48:11  djs
# Typedef to a sequence<sequence< name mangling bug
#
# Revision 1.9  1999/12/14 17:38:25  djs
# Fixed anonymous sequences of sequences bug
#
# Revision 1.8  1999/12/14 11:53:23  djs
# Support for CORBA::TypeCode and CORBA::Any
# Exception member bugfix
#
# Revision 1.7  1999/12/13 10:50:07  djs
# Treats the two call descriptors associated with an attribute separately,
# since it can happen that one needs to be generated but not the other.
#
# Revision 1.6  1999/12/10 18:27:05  djs
# Fixed bug to do with marshalling arrays of things, mirrored in the old
# compiler
#
# Revision 1.5  1999/12/09 20:40:58  djs
# Bugfixes and integration with dynskel/ code
#
# Revision 1.4  1999/12/01 16:59:01  djs
# Fixed name generation for attributes with user exceptions.
#
# Revision 1.3  1999/11/23 18:48:26  djs
# Bugfixes, more interface operations and attributes code
#
# Revision 1.2  1999/11/19 20:09:03  djs
# Fixed signature generating bug when return type is void
#
# Revision 1.1  1999/11/17 20:37:23  djs
# Code for call descriptors and proxies
#

# Functions intended for external use:
#
#   generate_descriptor : Signature -> String descriptor
#   operation_descriptor_name : Operation -> String descriptor
#   attribute_read_descriptor_name : Attribute -> String descriptor
#   attribute_write_descriptor_name : Attribute -> String descriptor



import re, string

from omniidl import idlast, idltype
from omniidl_be.cxx import util, tyutil, skutil, config

import mangler
self = mangler

# -------------------------
# Standard prefixes

CALL_DESC_PREFIX            = config.privatePrefix() + "_cd_"
LCALL_DESC_PREFIX           = config.privatePrefix() + "_lcfn_"
CTX_DESC_PREFIX             = config.privatePrefix() + "_ctx_"
STD_PROXY_CALL_DESC_PREFIX  = "omniStdCallDesc::"

# -------------------------
# Separator constants

SCOPE_SEPARATOR         =    "_m"
ARRAY_SEPARATOR         =    "_a"
SEQ_SEPARATOR           =    "_s"
CANNON_NAME_SEPARATOR   =    "_c"
ONEWAY_SEPARATOR        =    "_w"
IN_SEPARATOR            =    "_i"
OUT_SEPARATOR           =    "_o"
INOUT_SEPARATOR         =    "_n"
EXCEPTION_SEPARATOR     =    "_e"

# -------------------------
# Actual code goes here

def produce_idname(scopedName):
    # better safe than sorry with mutable data
    scopedName = scopedName[:]
    # s/_/__/g
    scopedName = map(lambda x: re.sub(r"_", "__", x), scopedName)
    # join with SCOPE_SEPARATOR
    return string.join(scopedName, SCOPE_SEPARATOR)

# Basic types are flattened into these strings within canonical name
name_map = {
    idltype.tk_short:       "short",
    idltype.tk_long:        "long",
    idltype.tk_longlong:    "longlong",
    idltype.tk_ushort:      "unsigned_pshort",
    idltype.tk_ulong:       "unsigned_plong",
    idltype.tk_ulonglong:   "unsigned_plonglong",
    idltype.tk_float:       "float",
    idltype.tk_double:      "double",
    idltype.tk_longdouble:  "long_pdouble",
    idltype.tk_char:        "char",
    idltype.tk_wchar:       "wchar",
    idltype.tk_boolean:     "boolean",
    idltype.tk_octet:       "octet",
    idltype.tk_void:        "void",
    idltype.tk_any:         "any",
    idltype.tk_TypeCode:    "TypeCode",
    }

# FIXME: Think of some way of better handling types. Merging
# in information from declarators would be sensible.
def canonTypeName(type, decl = None, useScopedName = 0):
    assert isinstance(type, idltype.Type)
    
    type_dims = tyutil.typeDims(type)
    decl_dims = []
    if decl != None:
        assert isinstance(decl, idlast.Declarator)
        decl_dims = decl.sizes()

    # flatten a list of dimensions into a string
    def dims(d):
        if d == []:
            return ""
        d_str = map(str, d)
        d_str = map(lambda x:ARRAY_SEPARATOR + x, d_str)
        return string.join(d_str, "")

    full_dims = decl_dims + type_dims
    is_array = full_dims != []
    dims_string = dims(full_dims)

    # The canonical type name always has the full dimensions
    # prepended to it.
    canon_name = dims_string

    deref_type = tyutil.deref(type)

    # consider anonymous sequence<sequence<....
    if tyutil.isSequence(type) and \
          tyutil.isSequence(type.seqType()):
        bound = type.bound()
        canon_name = canon_name + SEQ_SEPARATOR + str(bound) +\
                     canonTypeName(type.seqType(), None, useScopedName)
        
        return canon_name
        

    # sometimes we don't want to call a sequence a sequence
    # (operation signatures)
    if useScopedName and not(is_array) and \
       tyutil.isTypedef(type) and tyutil.isSequence(deref_type):
        # find the last typedef in the chain
        while tyutil.isTypedef(type.decl().alias().aliasType()):
            type = type.decl().alias().aliasType()
        scopedName = produce_idname(type.scopedName())
        return canon_name + CANNON_NAME_SEPARATOR + scopedName

    # _arrays_ of sequences seem to get handled differently
    # to simple aliases to sequences
    #if (tyutil.isSequence(deref_type) and is_array) or \
    #   tyutil.isSequence(type):
    if tyutil.isSequence(deref_type):
        bound = deref_type.bound()
        canon_name = canon_name + SEQ_SEPARATOR + str(bound)
        seqType = deref_type.seqType()

        while (tyutil.isSequence(seqType)):
            bound = seqType.bound()
            canon_name = canon_name + SEQ_SEPARATOR + str(bound)
            seqType = seqType.seqType()

        # straight forward sequences of sequences use their
        # flattened scoped name
        dkd_seqType = tyutil.derefKeepDims(seqType)
        if not(tyutil.isSequence(dkd_seqType)):
            canon_name = canon_name + canonTypeName(dkd_seqType)
            return canon_name
        type = seqType
        deref_type = tyutil.deref(type)
        

    # add in the name for the most dereferenced type
    def typeName(type):
        assert isinstance(type, idltype.Type)
        deref_type = tyutil.deref(type)
        # dereference the type, until just -before- it becomes a
        # sequence. Since a sequence doesn't have a scopedName(),
        # we use the scopedName() of the immediately preceeding
        # typedef which is an instance of idltype.Declared
        while tyutil.isTypedef(type) and \
              not(tyutil.isSequence(type.decl().alias().aliasType())):
            type = type.decl().alias().aliasType()

        if name_map.has_key(type.kind()):
            return name_map[type.kind()]
        if tyutil.isString(type):
            bound = ""
            if type.bound() != 0:
                bound = str(type.bound())
            return bound + "string"
        if isinstance(type, idltype.Declared):
            return produce_idname(type.scopedName())
        raise "Don't know how to generate a simple name for type: " +\
              repr(type) + " (kind = " + repr(type.kind()) + ")"

    canon_name = canon_name + CANNON_NAME_SEPARATOR + typeName(type)
    return canon_name

# Given a type, produce a flat unique canonical name
def produce_canonical_name_for_type(type):
    assert isinstance(type, idltype.Type)

    return canonTypeName(type, None)



def produce_operation_signature(operation):
    assert isinstance(operation, idlast.Operation)

    returnType = operation.returnType()

    # return type
    if tyutil.isVoid(returnType, 1):
        sig = "void"
    else:
        sig = canonTypeName(returnType, useScopedName = 1)
        
    # parameter list
    for param in operation.parameters():
        if param.is_in() and param.is_out():
            sig = sig + INOUT_SEPARATOR
        elif param.is_in():
            sig = sig + IN_SEPARATOR
        elif param.is_out():
            sig = sig + OUT_SEPARATOR

        sig = sig + canonTypeName(param.paramType(),
                                  useScopedName = 1)

    # exception list
    raises = skutil.sort_exceptions(operation.raises())

    def exception_signature(exception):
        cname = CANNON_NAME_SEPARATOR +\
                produce_idname(exception.scopedName())
        return EXCEPTION_SEPARATOR + cname
    
    raises_sigs = map(exception_signature, raises)
    raises_str = string.join(raises_sigs, "")

    sig = sig + raises_str
    return sig
            

def produce_read_attribute_signature(attribute):
    assert isinstance(attribute, idlast.Attribute)

    return canonTypeName(attribute.attrType(), useScopedName = 1)

def produce_write_attribute_signature(attribute):
    assert isinstance(attribute, idlast.Attribute)

    return "void" + IN_SEPARATOR +\
           canonTypeName(attribute.attrType(), useScopedName = 1)
        

# ----------------
# Call Descriptor utility functions

def __init__():
    self.base_initialised = 0
    self.base_low = 0
    self.base_high = 0
    self.base_counter = 0

    prefix = STD_PROXY_CALL_DESC_PREFIX 
    pre = { "void": prefix + "void_call",
            "_cCORBA_mObject_i_cstring": prefix + "_cCORBA_mObject_i_cstring" }
    self.call_descriptor_table = pre



def initialise_base(scopedName):
    if self.base_initialised:
        return
    string_seed = produce_idname(scopedName)
    
    self.base_initialised = 1

    # equivalent to >> only without sign extension
    # (python uses 2's complement signed arithmetic)
    def rshift(x, distance):
        sign_bit = x & 0x80000000
        # remove the sign bit to make it unsigned
        x = x & (0xffffffff ^ sign_bit)
        # perform shift (thinks number is unsigned, no extension)
        x = x >> distance
        # add sign bit back in
        if sign_bit:
            x = x | (1 << (32 - distance -1))
        return x

    def lshift(x, distance):
        # same as non-sign extended case
        return x << distance
    
    for char in string_seed:
        tmp = rshift((self.base_high & 0xfe000000), 25)
        self.base_high = (lshift(self.base_high, 7)) ^\
                         (rshift((self.base_low & 0xfe000000), 25))
        self.base_low  = lshift(self.base_low, 7) ^ tmp
        self.base_low  = self.base_low ^ (ord(char))


def generate_unique_name(prefix):
    # the effect of all the messing around with nibbles is the following
    # note that we don't care about possible differences across archs
    # since this is not really external.

    # takes an int and returns the int in hex, without leading 0x and
    # with 0 padding
    def hex_word(x):
        x = hex(x)[2:]
        while len(x) < 8:
            x = "0" + x
        return x
    
    high    = hex_word(self.base_high)
    low     = hex_word(self.base_low)
    counter = hex_word(self.base_counter)

    high    = list(high)
    low     = list(low)
    counter = list(counter)

    high.reverse()
    low.reverse()
    counter.reverse()

    unique_name = prefix + string.join(high + low + ["_"] + counter, "")

    self.base_counter = self.base_counter + 1
    
    return unique_name
    


def generate_descriptor(signature):
    assert(self.base_initialised)

    if not(self.call_descriptor_table):
        initialise_call_descriptor_table()
        
    def add_to_table(signature, cdt = self.call_descriptor_table, self = self):
        if not(cdt.has_key(signature)):
            class_name = generate_unique_name(self.CALL_DESC_PREFIX)
            cdt[signature] = class_name
            return class_name

    return add_to_table(signature)


def operation_descriptor_name(operation):
    assert isinstance(operation, idlast.Operation)
    sig = produce_operation_signature(operation)

    return self.call_descriptor_table[sig]

def attribute_read_descriptor_name(attribute):
    assert isinstance(attribute, idlast.Attribute)
    sig = produce_read_attribute_signature(attribute)

    return self.call_descriptor_table[sig]


def attribute_write_descriptor_name(attribute):
    assert isinstance(attribute, idlast.Attribute)
    sig = produce_write_attribute_signature(attribute)

    return self.call_descriptor_table[sig] 
        
    
