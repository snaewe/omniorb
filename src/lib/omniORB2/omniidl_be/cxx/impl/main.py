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
# Revision 1.1.2.4  2000/04/26 18:22:37  djs
# Rewrote type mapping code (now in types.py)
# Rewrote identifier handling code (now in id.py)
#
# Revision 1.1.2.3  2000/03/09 15:21:53  djs
# Better handling of internal compiler exceptions (eg attempts to use
# wide string types)
#
# Revision 1.1.2.2  2000/02/18 23:01:25  djs
# Updated example implementation code generating module
#
# Revision 1.1  2000/02/13 15:54:14  djs
# Beginnings of code to generate example interface implementations
#

"""Produce example interface implementations"""

import string

from omniidl import idlast, idltype, idlutil, idlvisitor
from omniidl_be.cxx import tyutil, util, config, id, types
from omniidl_be.cxx.impl import template

import main

self = main

def __init__(stream, idl_filename, hh_filename):
    self.stream = stream
    self.idl_filename = idl_filename
    self.hh_filename = hh_filename


def run(tree):
    # first thing is to build the interface implementations
    impl = util.StringStream()
    bii = BuildInterfaceImplementations(impl)
    tree.accept(bii)

    # code for each implementation
    allocate = util.StringStream()
    activate = util.StringStream()
    reference = util.StringStream()
    refcount = util.StringStream()
    for i in bii.allInterfaces():
        name = id.Name(i.scopedName())
        
        impl_name = name.simple() + "_i"
        inst_name = "my" + impl_name
        allocate.out("@impl_name@* @inst_name@ = new @impl_name@();",
                     impl_name = impl_name, inst_name = inst_name)
        activate.out("PortableServer::ObjectId_var @inst_name@id = poa->activate_object(@inst_name@);",inst_name = inst_name)
        refcount.out("@inst_name@->_remove_ref();", inst_name = inst_name)
        reference.out(template.interface_ior,
                      fqname = name.fullyQualify(cxx = 0),
                      inst_name = inst_name)

    # output the main code
    stream.out(template.main,
               idl_hh = self.hh_filename,
               file = self.idl_filename,
               interfaces = str(impl),
               allocate_objects = str(allocate),
               activate_objects = str(activate),
               output_references = str(reference),
               dec_reference_counts = str(refcount))


#
# Build the interface implementations
#
class BuildInterfaceImplementations(idlvisitor.AstVisitor):

    def __init__(self, stream):
        self.stream = stream
        self.__nested = 0
        # keep track of all interfaces for later use
        self.__allInterfaces = []

    def allInterfaces(self):
        return self.__allInterfaces[:]

    # Tree walking code
    def visitAST(self, node):
        for n in node.declarations():
            n.accept(self)

    # modules can contain interfaces
    def visitModule(self, node):
        if not(node.mainFile()):
            return

        nested = self.__nested
        self.__nested = 1

        for n in node.definitions():
            n.accept(self)

        self.__nested = nested
        

    # interfaces cannot be further nested
    def visitInterface(self, node):
        if not(node.mainFile()):
            return

        self.__allInterfaces.append(node)
    
        name = node.identifier()
        cxx_name = id.mapID(name)

        scopedName = id.Name(node.scopedName())
        #cxx_scopedName = map(tyutil.mapID, scopedName)
        
        outer_environment = id.lookup(node)
        environment = outer_environment.enter(name)
        scope = environment.scope()

        cxx_fqname = scopedName.fullyQualify()

        fqname = scopedName.fullyQualify(cxx = 0)

        
        # build methods corresponding to attributes, operations etc
        # attributes[] and operations[] will contain lists of function
        # signatures eg
        #   [ char *echoString(const char *mesg) ]
        attributes = []
        operations = []
        virtual_operations = []

        # we need to consider all callables, including inherited ones
        # since this implementation class is not inheriting from anywhere
        # other than the IDL skeleton
        allInterfaces = [node] + tyutil.allInherits(node)
        allCallables = util.fold( map(lambda x:x.callables(), allInterfaces), [],
                                  util.append )
        for c in allCallables:
            if isinstance(c, idlast.Attribute):
                attrType = types.Type(c.attrType())
                d_attrType = attrType.deref()

                #argtypes = tyutil.operationArgumentType\
                #           (attrType, outer_environment)
                #returnType = argtypes[0]
                #inType = argtypes[1]
            
                for i in c.identifiers():
                    attribname = id.mapID(i)
                    returnType = attrType.op(types.RET, outer_environment)
                    inType = attrType.op(types.IN, outer_environment)
                    attributes.append(returnType + " " + attribname + "()")
                    if not(c.readonly()):
                        attributes.append("void " + cxx_fqname + "_i::" +\
                                          attribname + "(" + inType + ")")
            elif isinstance(c, idlast.Operation):
                params = []
                for p in c.parameters():
                    paramType = types.Type(p.paramType())
                    cxx_type = paramType.op(types.direction(p),
                                            outer_environment)
                    #virtual_types = tyutil.operationArgumentType\
                    #                (paramType, outer_environment, virtualFn = 1)
                    #type = virtual_types[p.direction() + 1]
                    
                    argname = id.mapID(p.identifier())
                    params.append(cxx_type + " " + argname)

                # deal with possible "context"
                if c.contexts() != []:
                    params.append("CORBA::Context_ptr _ctxt")
                    virtual_params.append("CORBA::Context_ptr _ctxt")

                return_type = types.Type(c.returnType()).op(types.RET,
                                                            outer_environment)
                #return_type = tyutil.operationArgumentType\
                #              (c.returnType(), outer_environment,
                #               virtualFn = 0)[0]
                opname = id.mapID(c.identifier())
                arguments = string.join(params, ", ")
                operations.append(return_type + " " + cxx_fqname + "_i::" +\
                                  opname + "(" + arguments + ")")
            else:
                util.fatalError("Internal error generating interface member")
                raise "No code for interface member: " + repr(c)

        # the class definition has no actual code...
        op_defs = ""
        for x in operations:
            op_defs = op_defs + "virtual " + x + ";\n"
        op_attr = ""
        for x in attributes:
            op_attr = op_attr + "virtual " + x + ";\n"

        # Output the _i class definition definition
        self.stream.out(template.interface_def,
                        name = cxx_name,
                        fq_name = fqname,
                        fq_POA_name = "POA_" + cxx_fqname,
                        operations = op_defs,
                        attributes = op_attr)

        # Output the implementations of the class methods
        op_str = ""
        for x in operations:
            op_str = op_str + x + "{\n// insert code here\n}"
        attr_str = ""
        for x in attributes:
            attr_str = attr_str + x + "{\n// insert code here\n}"
        
               
        self.stream.out(template.interface_code,
                        fqname = fqname,
                        name = cxx_name,
                        operations = op_str,
                        attributes = attr_str)





