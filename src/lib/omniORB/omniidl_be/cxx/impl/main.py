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
# Revision 1.3.2.3  2001/06/08 17:12:18  dpg1
# Merge all the bug fixes from omni3_develop.
#
# Revision 1.3.2.2  2000/10/12 15:37:52  sll
# Updated from omni3_1_develop.
#
# Revision 1.4.2.2  2000/09/27 17:11:28  djs
# Bugfix
#
# Revision 1.4.2.1  2000/08/21 11:35:23  djs
# Lots of tidying
#
# Revision 1.4  2000/07/13 15:26:00  dpg1
# Merge from omni3_develop for 3.0 release.
#
# Revision 1.1.2.5  2000/05/16 11:16:01  djs
# Updated to simplify memory management, correct errors in function prototypes,
# add missing attribute functions and generate #warnings which the user should
# remove when they fill in the gaps in the output.
#
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

from omniidl import idlast, idlvisitor
from omniidl_be.cxx import ast, cxx, util, id, types, output
from omniidl_be.cxx.impl import template

import main

self = main

def __init__(stream, idl_filename, hh_filename):
    self.stream = stream
    self.idl_filename = idl_filename
    self.hh_filename = hh_filename


# Create the implementation classes in the toplevel namespace.
# For an IDL interface A::B::C generate an implementation class
# A_B_C_i

# Given an IDL name convert it into the fully qualified name of the
# implementation class
def impl_fullname(name):
    bits = name.suffix("_i").fullName()
    return string.join(bits, "_")

# Convert an IDL name into the simple name of the implementation class
def impl_simplename(name):
    return impl_fullname(name)


# Main code entrypoint
def run(tree):
    # first thing is to build the interface implementations
    impl = output.StringStream()
    bii = BuildInterfaceImplementations(impl)
    tree.accept(bii)

    # for each interface we implement we require:
    # 1. heap allocation
    allocate = output.StringStream()
    # 2. POA activation
    activate = output.StringStream()
    # 3. reference generation, stringification and output
    reference = output.StringStream()

    for i in bii.allInterfaces():
        name = id.Name(i.scopedName())

        impl_name = impl_fullname(name)
        # for an implementation class A_B_C_i, generate an instance myA_B_C_i
        inst_name = "my" + impl_name
        # allocate an instance of the implementation on the heap
        allocate.out("@impl_name@* @inst_name@ = new @impl_name@();",
                     impl_name = impl_name, inst_name = inst_name)
        # activate the object and get a T_var reference to it
        activate.out("PortableServer::ObjectId_var @inst_name@id = " +\
                     "poa->activate_object(@inst_name@);",
                     inst_name = inst_name)
        # get the reference and output it
        reference.out(template.interface_ior,
                      fqname = name.fullyQualify(cxx = 0),
                      inst_name = inst_name)

    # output the main() routine (all ORB initialisation code etc)
    stream.out(template.main,
               idl_hh = self.hh_filename,
               file = self.idl_filename,
               interfaces = str(impl),
               allocate_objects = str(allocate),
               activate_objects = str(activate),
               output_references = str(reference))



# Build the interface implementations
#
class BuildInterfaceImplementations(idlvisitor.AstVisitor):

    def __init__(self, stream):
        self.stream = stream
        # keep track of all interfaces for later use
        self.__allInterfaces = []

    # Returns the list of all present interfaces (each one will be
    # implemented)
    def allInterfaces(self):
        return self.__allInterfaces[:]

    # Tree walking code
    def visitAST(self, node):
        for n in node.declarations():
            if ast.shouldGenerateCodeForDecl(n):
                n.accept(self)

    # modules can contain interfaces
    def visitModule(self, node):
        for n in node.definitions():
            n.accept(self)

    # interfaces cannot be further nested
    def visitInterface(self, node):
        self.__allInterfaces.append(node)
    
        scopedName = id.Name(node.scopedName())
        
        cxx_fqname = scopedName.fullyQualify()
        impl_flat_name = impl_fullname(scopedName)

        fqname = scopedName.fullyQualify(cxx = 0)

        
        # build methods corresponding to attributes, operations etc.
        # attributes[] and operations[] will contain lists of function
        # signatures eg
        #   [ char *echoString(const char *mesg) ]
        attributes = []
        operations = []
        virtual_operations = []

        # we need to consider all callables, including inherited ones
        # since this implementation class is not inheriting from anywhere
        # other than the IDL skeleton
        allInterfaces = [node] + ast.allInherits(node)
        allCallables = util.fold( map(lambda x:x.callables(), allInterfaces),
                                  [], lambda x, y: x + y )


        # declarations[] contains a list of in-class decl signatures
        # implementations[] contains a list of out of line impl signatures
        # (typically differ by classname::)
        declarations = []
        implementations = []
        
        for c in allCallables:
            if isinstance(c, idlast.Attribute):
                attrType = types.Type(c.attrType())
                d_attrType = attrType.deref()

                for i in c.identifiers():
                    attribname = id.mapID(i)
                    returnType = attrType.op(types.RET)
                    inType = attrType.op(types.IN)
                    attributes.append(returnType + " " + attribname + "()")
                    # need a set method if not a readonly attribute
                    if not c.readonly():
                        args = attribname + "(" + inType + ")"
                        declarations.append("void " + args)
                        implementations.append("void " + impl_flat_name +\
                                               "::" + args)
                    declarations.append(returnType + " " + attribname + "()")
                    implementations.append(returnType + " " + impl_flat_name+\
                                           "::" + attribname + "()")
            elif isinstance(c, idlast.Operation):
                params = []
                for p in c.parameters():
                    paramType = types.Type(p.paramType())
                    cxx_type = paramType.op(types.direction(p), use_out = 0)
                    
                    argname = id.mapID(p.identifier())
                    params.append(cxx_type + " " + argname)

                # deal with possible "context"
                if c.contexts() != []:
                    params.append("CORBA::Context_ptr _ctxt")

                return_type = types.Type(c.returnType()).op(types.RET)

                opname = id.mapID(c.identifier())
                arguments = string.join(params, ", ")
                args = opname + "(" + arguments + ")"
                declarations.append(return_type + " " + args)
                implementations.append(return_type + " " + impl_flat_name +\
                                       "::" + args)
            else:
                util.fatalError("Internal error generating interface member")
                raise "No code for interface member: " + repr(c)

        # the class definition has no actual code...
        defs = string.join(map(lambda x:x + ";\n", declarations), "")
            
        # Output the _i class definition definition
        self.stream.out(template.interface_def,
                        impl_fqname = impl_flat_name,
                        impl_name = impl_flat_name,
                        fq_name = fqname,
                        fq_POA_name = "POA_" + cxx_fqname,
                        operations = defs)

        # Output the implementations of the class methods
        impls = string.join(map(lambda x: x + """\
{
  // insert code here and remove the warning
  #warning "Code missing in function <""" + x + """>"
}

""",
                                implementations), "")
               
        self.stream.out(template.interface_code,
                        fqname = fqname,
                        impl_name = impl_flat_name,
                        impl_fqname = impl_flat_name,
                        operations = impls)



