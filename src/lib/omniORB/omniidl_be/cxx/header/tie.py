# -*- python -*-
#                           Package   : omniidl
# tie.py                    Created on: 1999/12/13
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
#   Produce the 'tie' templates
#
# $Id$
# $Log$
# Revision 1.16.2.3  2008/12/03 10:53:58  dgrisby
# Tweaks leading to Python 3 support; other minor clean-ups.
#
# Revision 1.16.2.2  2007/09/19 14:16:07  dgrisby
# Avoid namespace clashes if IDL defines modules named CORBA.
#
# Revision 1.16.2.1  2003/03/23 21:02:36  dgrisby
# Start of omniORB 4.1.x development branch.
#
# Revision 1.13.2.4  2001/06/08 17:12:18  dpg1
# Merge all the bug fixes from omni3_develop.
#
# Revision 1.13.2.3  2000/11/09 12:27:56  dpg1
# Huge merge from omni3_develop, plus full long long from omni3_1_develop.
#
# Revision 1.13.2.2  2000/10/12 15:37:52  sll
# Updated from omni3_1_develop.
#
# Revision 1.14.2.2  2000/08/21 11:35:19  djs
# Lots of tidying
#
# Revision 1.14.2.1  2000/08/02 10:52:02  dpg1
# New omni3_1_develop branch, merged from omni3_develop.
#
# Revision 1.14  2000/07/13 15:26:00  dpg1
# Merge from omni3_develop for 3.0 release.
#
# Revision 1.11.2.8  2000/07/17 09:36:40  djs
# Now handles the case where an interface inherits from a typedef to another
# interface.
#
# Revision 1.11.2.7  2000/06/26 16:24:00  djs
# Better handling of #include'd files (via new commandline options)
# Refactoring of configuration state mechanism.
#
# Revision 1.11.2.6  2000/06/12 13:22:14  djs
# Stopped generation of BOA ties #include'd from other files
#
# Revision 1.11.2.5  2000/06/05 13:03:58  djs
# Removed union member name clash (x & pd_x, pd__default, pd__d)
# Removed name clash when a sequence is called "pd_seq"
# Nested union within union fix
# Actually generates BOA non-flattened tie templates
#
# Revision 1.11.2.4  2000/05/31 18:02:58  djs
# Better output indenting (and preprocessor directives now correctly output at
# the beginning of lines)
#
# Revision 1.11.2.3  2000/04/26 18:22:31  djs
# Rewrote type mapping code (now in types.py)
# Rewrote identifier handling code (now in id.py)
#
# Revision 1.11.2.2  2000/03/13 16:01:02  djs
# Problem generating tie templates with diamond inheritance (duplicated methods
# by mistake)
#
# Revision 1.11.2.1  2000/02/14 18:34:54  dpg1
# New omniidl merged in.
#
# Revision 1.11  2000/01/20 12:46:40  djs
# Renamed a function to avoid a name clash with a module.
#
# Revision 1.10  2000/01/19 11:23:29  djs
# Moved most C++ code to template file
#
# Revision 1.9  2000/01/14 11:57:18  djs
# Added (flattened) templates missing in BOA generation mode.
#
# Revision 1.8  2000/01/13 14:16:30  djs
# Properly clears state between processing separate IDL input files
#
# Revision 1.7  2000/01/11 12:02:41  djs
# More tidying up
#
# Revision 1.6  2000/01/10 17:18:15  djs
# Removed redundant code.
#
# Revision 1.5  2000/01/10 15:38:56  djs
# Better name and scope handling.
#
# Revision 1.4  2000/01/10 11:01:57  djs
# Forgot to keep track of names already defined causing a scoping problem.
#
# Revision 1.3  2000/01/07 20:31:29  djs
# Regression tests in CVSROOT/testsuite now pass for
#   * no backend arguments
#   * tie templates
#   * flattened tie templates
#   * TypeCode and Any generation
#
# Revision 1.2  1999/12/26 16:43:53  djs
# Fix for (not) generating tie templates of #included .idl
#
# Revision 1.1  1999/12/14 11:54:43  djs
# Restructured generation of tie templates
#
#

"""Produce the 'tie' templates"""

import string

from omniidl import idlast, idltype, idlutil, idlvisitor
from omniidl_be.cxx import id, config, types, output, ast
from omniidl_be.cxx.header import template

import tie

self = tie

def __init__(stream):
    self.stream = stream
    return self


# Write a single tie template class called <name>, inheriting from <inherits>
# and grab the operations from <node>
def write_template(name, inherits, node, stream,
                   Template = template.tie_template):
    # build methods which bind the interface operations and attributes
    # note that this includes inherited callables since tie
    # templates are outside the normal inheritance structure
    where = output.StringStream()

    # defined_so_far contains keys corresponding to method names which
    # have been defined already (and which should not be included twice)
    def buildCallables(interface, where, continuation, defined_so_far = {}):
        interface = ast.remove_ast_typedefs(interface)
        
        callables = interface.callables()
        operations = filter(lambda x:isinstance(x, idlast.Operation),
                            callables)
        for operation in operations:
            returnType = types.Type(operation.returnType())
            identifier = operation.identifier()
            if (defined_so_far.has_key(identifier)):
                # don't repeat it
                continue
            defined_so_far[identifier] = 1
            
            parameters = operation.parameters()
            has_return_value = not returnType.void()
            # FIXME: return types are fully scoped but argument types
            # arent?
            returnType_name = returnType.op(types.RET)

            operation_name = id.mapID(identifier)
            
            signature = []
            call = []

            for parameter in parameters:
                paramType = types.Type(parameter.paramType())
                # Need to call the _impl operation not the _objref operation
                param_type_name = paramType.op(types.direction(parameter),
                                               use_out = 0)
                param_id = id.mapID(parameter.identifier())
                signature.append(param_type_name + " " + param_id)
                call.append(param_id)

            # deal with call contextx
            if operation.contexts() != []:
                signature.append("::CORBA::Context_ptr _ctxt")
                call.append("_ctxt")

            if has_return_value:
                return_str = "return "
            else:
                return_str = ""
                
            where.out("""\
@return_type_name@ @operation_name@(@signature@) { @return_str@pd_obj->@operation_name@(@call@); }""", return_type_name = returnType_name,
                      operation_name = operation_name,
                      return_str = return_str,
                      signature = string.join(signature, ", "),
                      call = string.join(call, ", "))
                    
        attributes = filter(lambda x:isinstance(x, idlast.Attribute),
                            callables)
        for attribute in attributes:
            identifiers = attribute.identifiers()
            attrType = types.Type(attribute.attrType())

            attrType_name_RET = attrType.op(types.RET)
            attrType_name_IN = attrType.op(types.IN)
            
            for identifier in identifiers:
                if defined_so_far.has_key(identifier):
                    # don't repeat it
                    continue
                defined_so_far[identifier] = 1
                
                ident = id.mapID(identifier)
                where.out("""\
@attr_type_ret_name@ @attribute_name@() { return pd_obj->@attribute_name@(); }""", attr_type_ret_name = attrType_name_RET,
                          attribute_name = ident)
                if not attribute.readonly():
                    where.out("""\
void @attribute_name@(@attr_type_in_name@ _value) { pd_obj->@attribute_name@(_value); }""", attribute_name = ident,
                              attr_type_in_name = attrType_name_IN)                    
        # do the recursive bit
        for i in interface.inherits():
            i = i.fullDecl()
            continuation(i, where, continuation, defined_so_far)

        # done
        return

    buildCallables(node, where, buildCallables)
                
    stream.out(Template,
               tie_name = name,
               inherits = inherits,
               callables = str(where))
    return


# Unflattened BOA tie templates are built in a block out of line.
# IDL name       template name
#  ::A             ::_tie_A
#  ::B             ::_tie_B
#  ::M::C          ::_tie_M::C

class BOATieTemplates(idlvisitor.AstVisitor):
    def __init__(self, stream):
        self.stream = stream
    def visitAST(self, node):
        for d in node.declarations():
            if ast.shouldGenerateCodeForDecl(d):
                d.accept(self)

    def visitModule(self, node):
        name = id.Name(node.scopedName())
        
        self.stream.out(template.module_begin,
                   name = "_tie_" + name.simple())
        self.stream.inc_indent()
        
        for d in node.definitions(): d.accept(self)

        self.stream.dec_indent()
        self.stream.out(template.module_end)
        

    def visitInterface(self, node):
        name = id.Name(node.scopedName())

        tie_name = name.simple()
        if len(node.scopedName()) == 1: tie_name = "_tie_" + tie_name
        
        sk_name = name.prefix("_sk_")
        
        write_template(tie_name, sk_name.fullyQualify(), node, self.stream,
                       Template = template.tie_template_old)


# Flat Tie Templates are all (by definition) in the global scope,
# so can combine POA and BOA code into one
class FlatTieTemplates(idlvisitor.AstVisitor):
    def __init__(self, stream):
        self.stream = stream
    def visitAST(self, node):
        for d in node.declarations():
            if ast.shouldGenerateCodeForDecl(d):
                d.accept(self)
    def visitModule(self, node):
        for d in node.definitions():
            d.accept(self)
    def visitInterface(self, node):
        self.generate_POA_tie(node)
        if config.state['BOA Skeletons']:
            self.generate_BOA_tie(node)

    def generate_BOA_tie(self, node):
        name = id.Name(node.scopedName())
        tie_name = "_tie_" + string.join(name.fullName(), "_")
        sk_name = name.prefix("_sk_")

        write_template(tie_name, sk_name.fullyQualify(), node, self.stream,
                       Template = template.tie_template_old)

    def generate_POA_tie(self, node):
        name = id.Name(node.scopedName())
        tie_name = "POA_" + string.join(name.fullName(), "_") + "_tie"
        poa_name = "POA_" + name.fullyQualify()

        write_template(tie_name, poa_name, node, self.stream)




