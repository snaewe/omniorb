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
# Revision 1.16.2.1  2000/08/21 11:35:33  djs
# Lots of tidying
#
# Revision 1.16  2000/07/13 15:25:59  dpg1
# Merge from omni3_develop for 3.0 release.
#
# Revision 1.13.2.5  2000/06/26 16:24:18  djs
# Refactoring of configuration state mechanism.
#
# Revision 1.13.2.4  2000/04/26 18:22:56  djs
# Rewrote type mapping code (now in types.py)
# Rewrote identifier handling code (now in id.py)
# Removed superfluous externs in front of function definitions
#
# Revision 1.13.2.3  2000/03/09 15:21:57  djs
# Better handling of internal compiler exceptions (eg attempts to use
# wide string types)
#
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

# Functions which produce a signature string from a callable:
#
# produce_operation_signature       :                                 
# produce_read_attribute_signature  : callable -> signature string    
# produce_write_attribute_signature :                                 '

# Every signature has a unique "call descriptor" object which serves
# to encapsulate the calling and returning arguments. Note that two
# interfaces with an identical method can share the call descriptors.

# Every Interface::Method has a unique "local callback function" which
# calls the local _impl_ method in the case of colocated calls. These
# are never shared.

# call_descriptor(signature) : call descriptor class name string
# context_descriptor(signature) : context descriptor for signature
# local_callback_fn(interface, signature) : callback function name


import re, string

from omniidl import idlast, idltype
from omniidl_be.cxx import util, skutil, config, types, id

import mangler
self = mangler

# -------------------------
# Standard prefixes

private_prefix = config.state['Private Prefix']
CALL_DESC_PREFIX            = private_prefix + "_cd_"
LCALL_DESC_PREFIX           = private_prefix + "_lcfn_"
CTX_DESC_PREFIX             = private_prefix + "_ctx_"
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
    assert isinstance(type, types.Type)
    
    type_dims = type.dims()
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

    d_type = type.deref()

    # consider anonymous sequence<sequence<....
    if type.sequence() and types.Type(type.type().seqType()).sequence():
        bound = type.type().bound()
        canon_name = canon_name + SEQ_SEPARATOR + str(bound) +\
                     canonTypeName(types.Type(type.type().seqType()), None, useScopedName)
        
        return canon_name
        

    # sometimes we don't want to call a sequence a sequence
    # (operation signatures)
    if useScopedName and not(is_array) and \
       type.typedef() and d_type.sequence():
        # find the last typedef in the chain
        while types.Type(type.type().decl().alias().aliasType()).typedef():
            type = types.Type(type.type().decl().alias().aliasType())
        scopedName = id.Name(type.type().scopedName()).guard()
        return canon_name + CANNON_NAME_SEPARATOR + scopedName

    # _arrays_ of sequences seem to get handled differently
    # to simple aliases to sequences
    if d_type.sequence():
        bound = d_type.type().bound()
        canon_name = canon_name + SEQ_SEPARATOR + str(bound)
        seqType = types.Type(d_type.type().seqType())

        while seqType.sequence():
            bound = seqType.type().bound()
            canon_name = canon_name + SEQ_SEPARATOR + str(bound)
            seqType = types.Type(seqType.type().seqType())

        # straight forward sequences of sequences use their
        # flattened scoped name
        dkd_seqType = seqType.deref(keep_dims = 1)
        if not(dkd_seqType.sequence()):
            canon_name = canon_name + canonTypeName(dkd_seqType)
            return canon_name
        type = seqType
        d_type = type.deref()
        

    # add in the name for the most dereferenced type
    def typeName(type):
        assert isinstance(type, types.Type)
        d_type = type.deref()
        # dereference the type, until just -before- it becomes a
        # sequence. Since a sequence doesn't have a scopedName(),
        # we use the scopedName() of the immediately preceeding
        # typedef which is an instance of idltype.Declared
        while type.typedef() and \
              not(types.Type(type.type().decl().alias().aliasType()).sequence()):
            type = types.Type(type.type().decl().alias().aliasType())

        if name_map.has_key(type.type().kind()):
            return name_map[type.type().kind()]
        if type.string():
            bound = ""
            if type.type().bound() != 0:
                bound = str(type.type().bound())
            return bound + "string"
        if isinstance(type.type(), idltype.Declared):
            return id.Name(type.type().scopedName()).guard()
        if isinstance(type.type(), idltype.WString):
            util.fatalError("Wide-strings are not supported")

        util.fatalError("Error generating mangled name")

    canon_name = canon_name + CANNON_NAME_SEPARATOR + typeName(type)
    return canon_name

# Given a type, produce a flat unique canonical name
def produce_canonical_name_for_type(type):
    assert isinstance(type, types.Type)

    return canonTypeName(type, None)


def produce_signature(returnType, parameters, raises):

    returnType = types.Type(returnType)
    d_returnType = returnType.deref()

    # return type
    if d_returnType.void():
        sig = "void"
    else:
        sig = canonTypeName(returnType, useScopedName = 1)
        
    # parameter list
    for param in parameters:
        if param.is_in() and param.is_out():
            sig = sig + INOUT_SEPARATOR
        elif param.is_in():
            sig = sig + IN_SEPARATOR
        elif param.is_out():
            sig = sig + OUT_SEPARATOR

        sig = sig + canonTypeName(types.Type(param.paramType()),
                                  useScopedName = 1)

    # exception list
    raises = skutil.sort_exceptions(raises)

    def exception_signature(exception):
        cname = CANNON_NAME_SEPARATOR +\
                id.Name(exception.scopedName()).guard()
        return EXCEPTION_SEPARATOR + cname
    
    raises_sigs = map(exception_signature, raises)
    raises_str = string.join(raises_sigs, "")

    sig = sig + raises_str
    return sig
            

def produce_operation_signature(operation):
    assert isinstance(operation, idlast.Operation)
    return produce_signature(operation.returnType(),
                             operation.parameters(),
                             operation.raises())


def produce_read_attribute_signature(attribute):
    assert isinstance(attribute, idlast.Attribute)

    return canonTypeName(types.Type(attribute.attrType()),
                         useScopedName = 1)

def produce_write_attribute_signature(attribute):
    assert isinstance(attribute, idlast.Attribute)

    return "void" + IN_SEPARATOR +\
           canonTypeName(types.Type(attribute.attrType()),
                         useScopedName = 1)
        

# ----------------
# Call Descriptor utility functions

def __init__():
    self.base_initialised = 0
    self.base_low = 0
    self.base_high = 0
    self.base_counter = 0

    # Some call descriptors are already available in the runtime libraries

    prefix = STD_PROXY_CALL_DESC_PREFIX
    self.system_descriptors = \
          { "void":                      prefix + "void_call",
            "_cCORBA_mObject_i_cstring": prefix + "_cCORBA_mObject_i_cstring" }

    # Proxy call descriptor table keyed by signature
    self.call_descriptors = system_descriptors.copy()
    # make sure we dont redefine these either

    # Local callback function table keyed by interface and then signature
    self.callbacks = {}

def call_descriptor(signature):
    if self.system_descriptors.has_key(signature):
        return self.system_descriptors[signature]
    
    if not(self.call_descriptors.has_key(signature)):
        self.call_descriptors[signature] = generate_unique_name("")

    return CALL_DESC_PREFIX + self.call_descriptors[signature]

def context_descriptor(signature):
    if not(self.call_descriptors.has_key(signature)):
        self.call_descriptors[signature] = generate_unique_name("")

    return CTX_DESC_PREFIX + self.call_descriptors[signature]

def local_callback_fn(interface, operation_name, signature):
    assert isinstance(interface, idlast.Interface)

    if not(self.callbacks.has_key(interface)):
        self.callbacks[interface] = {}

    iface_table = self.callbacks[interface]
    key = signature + "/" + operation_name
    if iface_table.has_key(key):
        # we've got this exact function already
        return iface_table[key]
 
    # generate a new function name
    function = generate_unique_name(LCALL_DESC_PREFIX)
    iface_table[key] = function
    return function



######################################################################
# Hashing functions use a Knuth-style string -> int hash             #
# (This is just used _once_ to get a pseudo-random string. Why?)
def initialise_base(scopedName):
    if self.base_initialised:
        return
    string_seed = id.Name(scopedName).guard()
    
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


def generate_new_descriptor(signature):
    descriptor = generate_unique_name("")
    self.available_descriptors[signature] = descriptor
    return descriptor

def generate_unique_name(prefix):
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
    


        
    
