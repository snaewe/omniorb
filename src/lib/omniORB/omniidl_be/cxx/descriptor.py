# -*- python -*-
#                           Package   : omniidl
# descriptor.py             Created on: 2000/08/23
#			    Author    : David Scott (djs)
#
#    Copyright (C) 1999-2000 AT&T Laboratories Cambridge
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
#   Produce internal descriptors
#

# $Id$
# $Log$
# Revision 1.1.4.2  2001/03/13 10:34:01  dpg1
# Minor Python clean-ups
#
# Revision 1.1.4.1  2000/10/12 15:37:47  sll
# Updated from omni3_1_develop.
#
# Revision 1.1.2.1  2000/09/14 16:03:02  djs
# Remodularised C++ descriptor name generator
# Bug in listing all inherited interfaces if one is a forward
# repoID munging function now handles #pragma ID in bootstrap.idl
# Naming environments generating code now copes with new IDL AST types
# Modified type utility functions
# Minor tidying
#

from omniidl import idlvisitor, idlast
from omniidl_be.cxx import config, id
import string

# All descriptors are of the form:
#  TAG _ PREFIX _ BASE
# TAG:    groups descriptors by type (eg context, call descriptor, callback)
# PREFIX: derived from a hash of the first callable in the IDL
# BASE:   counter added  to guarantee uniqueness

# There are two categories of descriptors.
#  o Signature dependent (where there is only one instance per operation
#    signature, which can be shared by multiple interfaces) eg normal
#    call descriptors.
#  o Interface & operation dependent eg local callback functions

import descriptor
self = descriptor

def __init__(ast):
    self.prefix  = ""
    self.counter = 0

    # Descriptors keyed by signature alone
    self.signature_descriptors = {}

    # Descriptors keyed by interface and operation name (a two level
    # hashtable)
    self.iface_descriptors = {}

    # initialise the prefix
    HV = HashVisitor()
    ast.accept(HV)

def call_descriptor(signature):
    return private_prefix + "_cd_" + get_signature_descriptor(signature)

def context_descriptor(signature):
    return private_prefix + "_ctx_" + get_signature_descriptor(signature)

def local_callback_fn(iname, operation_name, signature):
    return private_prefix + "_lcfn_" +\
       get_interface_operation_descriptor(iname, operation_name, signature)


####################################################################
## Internals

private_prefix = config.state['Private Prefix']

# Walks over the AST, finds the first callable and creates the prefix hash
class HashVisitor(idlvisitor.AstVisitor):
    def __init__(self):
        self.base_initialised = 0
    
    def visitAST(self, node):
        for declaration in node.declarations(): declaration.accept(self)
    def visitModule(self, node):
        for definition in node.definitions(): definition.accept(self)
    def visitInterface(self, node):
        if not(node.mainFile()): return
        
        if node.callables() != []:
            # Use the first callable?
            #self.initialise_with_callable(node, node.callables()[0])

            # Use an op first if avail?
            ops = filter(lambda x:isinstance(x, idlast.Operation),
                         node.callables())
            attrs = filter(lambda x:isinstance(x, idlast.Attribute),
                         node.callables())
            if (ops != []): self.initialise_with_callable(node, ops[0])
            if (attrs != []): self.initialise_with_callable(node, attrs[0])


    def initialise_with_callable(self, interface, callable):
        name = interface.scopedName()
        if isinstance(callable, idlast.Operation):
            name = name + [callable.identifier()]
            self.initialise_base(name)
            return
        
        name = name + [callable.identifiers()[0]]
        self.initialise_base(name)
        return
        

    # Knuth-style string -> int hash
    def initialise_base(self, name):
        if self.base_initialised:
            return
        self.base_initialised = 1
        
        string_seed = id.Name(name).guard()
    
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

        (high, low) = (0, 0)
        for char in string_seed:
            tmp = rshift((high & 0xfe000000), 25)
            high = (lshift(high, 7)) ^ (rshift((low & 0xfe000000), 25))
            low  = lshift(low, 7) ^ tmp
            low  = low ^ (ord(char))

        high = list(hex_word(high))
        low  = list(hex_word(low))

        high.reverse()
        low.reverse()
        
        descriptor.prefix = string.join(high + low, "")

# Return a unique PREFIX + BASE
def unique():
    counter = list(hex_word(descriptor.counter))
    counter.reverse()

    name = descriptor.prefix + "_" + string.join( counter, "" )
    descriptor.counter = descriptor.counter + 1
    
    return name
    

def get_signature_descriptor(signature):
    if not(self.signature_descriptors.has_key(signature)):
        self.signature_descriptors[signature] = unique()

    return self.signature_descriptors[signature]

def get_interface_operation_descriptor(iname, operation_name, signature):
    assert isinstance(iname, id.Name)
    key = iname.hash()
    if not(self.iface_descriptors.has_key(key)):
        self.iface_descriptors[key] = {}

    iface_table = self.iface_descriptors[key]
    
    key = signature + "/" + operation_name
    if iface_table.has_key(key):
        return iface_table[key]
    
    descriptor = unique()
    iface_table[key] = descriptor
    return descriptor


# takes an int and returns the int in hex, without leading 0x and
# with 0s padding
def hex_word(x):
    return "%08x" % x
