# -*- python -*-
#                           Package   : omniidl
# main.py                   Created on: 2000/02/13
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
#   Produce example interface implementations

# $Id$
# $Log$
# Revision 1.1  2000/02/13 15:54:14  djs
# Beginnings of code to generate example interface implementations
#

"""Produce example interface implementations"""

import string

from omniidl import idlast, idltype, idlutil
from omniidl.be.cxx import tyutil, util, name, env, config
from omniidl.be.cxx.impl import template

import main

self = main

def __init__(stream):
    main.stream = stream
    self.__nested = 0

    self.__globalScope = name.globalScope()
    
    return main

def POA_prefix():
    if not(self.__nested):
        return "POA_"
    return ""



#
# Control arrives here
#
def visitAST(node):
    self.__completedModules = {}
    for n in node.declarations():
        n.accept(self)

def visitModule(node):
    if not(node.mainFile()):
        return

    nested = self.__nested
    self.__nested = 1

    for n in node.definitions():
        n.accept(self)

    self.__nested = nested
        

def visitInterface(node):
    if not(node.mainFile()):
        return

    name = node.identifier()
    cxx_name = tyutil.mapID(name)

    outer_environment = env.lookup(node)
    environment = outer_environment.enterScope(name)
    scope = environment.scope()
        
    # build methods corresponding to attributes, operations etc
    attributes = []
    operations = []
    virtual_operations = []
        
    for c in node.callables():
        if isinstance(c, idlast.Attribute):
            attrType = c.attrType()
            derefAttrType = tyutil.deref(attrType)

            argtypes = tyutil.operationArgumentType\
                       (attrType, outer_environment)
            returnType = argtypes[0]
            inType = argtypes[1]
            
            for i in c.identifiers():
                attribname = tyutil.mapID(i)
                attributes.append(returnType + " " + attribname + "()")
                if not(c.readonly()):
                    attributes.append("void " + attribname + "(" \
                                      + inType + ")")
        elif isinstance(c, idlast.Operation):
            params = []
            for p in c.parameters():
                paramType = p.paramType()
                virtual_types = tyutil.operationArgumentType\
                                (paramType, outer_environment, virtualFn = 1)
                type = virtual_types[p.direction()]

                argname = tyutil.mapID(p.identifier())
                params.append(type + " " + argname)

            # deal with possible "context"
            if c.contexts() != []:
                params.append("CORBA::Context_ptr _ctxt")
                virtual_params.append("CORBA::Context_ptr _ctxt")
                
            return_type = tyutil.operationArgumentType\
                          (c.returnType(), outer_environment,
                           virtualFn = 0)[0]
            opname = tyutil.mapID(c.identifier())
            arguments = string.join(params, ", ")
            operations.append(return_type + " " + opname + \
                              "(" + arguments + ")")
        else:
            raise "No code for interface member: " + repr(c)

    operations_str = ""
    for x in operations:
        operations_str = operations_str + x + "{\n// insert code here\n}"
    attributes_str = ""
    for x in attributes:
        attributes_str = attributes_str + x + "{\n// insert code here\n}"
        
    # deal with inheritance
    objref_inherits = []
    impl_inherits = []
    for i in node.inherits():
        # do something
        pass

    # Output the _i class definition definition
    stream.out(template.interface_def,
               name = cxx_name,
               POA_name = 
    stream.out(template.interface_impl,
               inherits = impl_inherits,
               virtual_operations = virtual_operations_str,
               virtual_attributes = virtual_attributes_str,
               name = cxx_name)


def visitForward(node):
    if not(node.mainFile()):
        return
    pass
def visitConst(node):
    pass
def visitTypedef(node):
    pass
def visitMember(node):
    pass
def visitStruct(node):
    pass
def visitException(node):
    pass
def visitUnion(node):
    pass
def visitEnum(node):
    pass

