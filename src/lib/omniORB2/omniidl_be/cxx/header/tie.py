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

from omniidl import idlast, idltype, idlutil
from omniidl_be.cxx import tyutil, name, env, config, util
from omniidl_be.cxx.header import template

import tie

self = tie

def __init__(stream):
    self.stream = stream
    self.__environment = name.Environment()
    return self


def POA_prefix(nested):
    if not(nested):
        return "POA"
    return ""



# Control arrives here
#
def visitAST(node):
    for n in node.declarations():
        n.accept(self)

def visitModule(node):

    for n in node.definitions():
        n.accept(self)


#def template(environment, node, nested = self.__nested):
def write_template(environment, node, nested = 0):

    scopedName = node.scopedName()
    scope = tyutil.scope(scopedName)
    iname = tyutil.mapID(node.identifier())
    prefix = POA_prefix(nested)
    if scope == []:
        scope = [prefix]
        scope_str = environment.nameToString(environment.relName(scope))
        POA_name = prefix + "_" + iname
    else:
        if prefix != "": scope[0] = prefix + "_" + scope[0]
        full_name = scope + [iname]
        POA_name = environment.nameToString(environment.relName(full_name))
    
    flat_scope = string.join(scope, "_")
    if config.FlatTieFlag():        
        POA_tie_name = flat_scope + "_" +  iname + "_tie"
    else:
        prefix = POA_prefix(nested)
        if prefix != "": prefix = prefix + "_"
        
        POA_tie_name = prefix + iname + "_tie"
        
    # FIXME: hack because operationArgumentType strips off outermost
    # scope
    environment = environment.enterScope("dummy")

    # build methods which bind the interface operations and attributes
    # note that this includes inherited callables since tie
    # templates are outside the normal inheritance structure
    where = util.StringStream()

    # defined_so_far contains keys corresponding to method names which
    # have been defined already (and which should not be included twice)
    def buildCallables(interface, where, environment, continuation,
                       defined_so_far = {}):
        global_env = name.Environment()
        callables = interface.callables()
        operations = filter(lambda x:isinstance(x, idlast.Operation),
                            callables)
        for operation in operations:
            returnType = operation.returnType()
            identifier = operation.identifier()
            if (defined_so_far.has_key(identifier)):
                # don't repeat it
                continue
            defined_so_far[identifier] = 1
            
            parameters = operation.parameters()
            has_return_value = not(tyutil.isVoid(returnType))
            # FIXME: return types are fully scoped but argument types
            # arent?
            returnType_name = tyutil.operationArgumentType(returnType,
                                                           global_env)[0]
            operation_name = tyutil.mapID(identifier)
            
            signature = []
            call = []

            for parameter in parameters:
                paramType = parameter.paramType()
                dir = parameter.direction() + 1
                if config.EMULATE_BUGS() and not(config.OldFlag()):
                    # the old compiler scopes this bit wrong (but only when
                    # _not_ generating old skeletons?!)
                    argtypes = tyutil.operationArgumentType(paramType,
                                                            environment,
                                                            virtualFn = 1)
                else:
                    argtypes = tyutil.operationArgumentType(paramType,
                                                            global_env,
                                                            virtualFn = 1)
                param_type_name = argtypes[dir]
                param_id = tyutil.mapID(parameter.identifier())
                signature.append(param_type_name + " " + param_id)
                call.append(param_id)

            # deal with call contextx
            if operation.contexts() != []:
                signature.append("CORBA::Context_ptr _ctxt")
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
            attrType = attribute.attrType()
            attrType_names = tyutil.operationArgumentType(attrType, global_env)

            attrType_name_RET = attrType_names[0]
            attrType_name_IN = attrType_names[1]
            
            for identifier in identifiers:
                if defined_so_far.has_key(identifier):
                    # don't repeat it
                    continue
                defined_so_far[identifier] = 1
                
                id = tyutil.mapID(identifier)
                where.out("""\
  @attr_type_ret_name@ @attribute_name@() { return pd_obj->@attribute_name@(); }""", attr_type_ret_name = attrType_name_RET,
                          attribute_name = id)
                if not(attribute.readonly()):
                    where.out("""\
  void @attribute_name@(@attr_type_in_name@ _value) { pd_obj->@attribute_name@(_value); }""", attribute_name = id,
                              attr_type_in_name = attrType_name_IN)                    
        # do the recursive bit
        for i in interface.inherits():
            continuation(i, where, environment, continuation, defined_so_far)

        # done
        return

    buildCallables(node, where, environment, buildCallables)
                
        
    stream.out(template.tie_template,
               tie_name = POA_tie_name,
               inherits = POA_name,
               callables = str(where))
    if config.FlatTieFlag() and config.BOAFlag():
        tie_name = "_tie_" + string.join(map(tyutil.mapID,scopedName), "_")
        sk_name = name.prefixName(map(tyutil.mapID,scopedName), "_sk_")
        stream.out(template.tie_template,
                   tie_name = tie_name,
                   inherits = sk_name,
                   callables = str(where))
    
def visitInterface(node):
    if not(node.mainFile()):
        return

    environment = env.lookup(node)
    write_template(environment, node)

    for n in node.declarations():
        n.accept(self)
    

def visitEnum(node):
    pass
def visitStruct(node):
    for n in node.members():
        n.accept(self)

def visitUnion(node):
    pass
def visitForward(node):
    pass
def visitConst(node):
    pass
def visitDeclarator(node):
    pass
def visitMember(node):
    if node.constrType():
        node.memberType().decl().accept(self)
    pass
def visitException(node):
    pass
def visitTypedef(node):
    pass
