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

from omniidl.be.cxx import tyutil, name, config, util

import tie

self = tie

def __init__(stream):
    self.stream = stream
    return self


self.__nested = 0

self.__environment = name.Environment()

def enter(scope):
    self.__environment = self.__environment.enterScope(scope)
def leave():
    self.__environment = self.__environment.leaveScope()
def currentScope():
    return self.__environment.scope()



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

    enter(node.identifier())
    
    for n in node.definitions():
        nested = self.__nested
        # we're outside of the CORBA_MODULE stuff so "not nested"
        #self.__nested = 1
        
        n.accept(self)

        self.__nested = nested

    leave()

def template(env, node, nested = self.__nested):
    
    scope = tyutil.scope(node.scopedName())
    iname = tyutil.mapID(node.identifier())
    prefix = POA_prefix(nested)

    if scope == []:
        scope = [prefix]
        scope_str = env.nameToString(env.relName(scope))
        POA_name = prefix + "_" + iname
    else:
        if prefix != "": scope[0] = prefix + "_" + scope[0]
        full_name = scope + [iname]
        POA_name = env.nameToString(env.relName(full_name))
    
    flat_scope = string.join(scope, "_")
    if config.FlatTieFlag():        
        POA_tie_name = flat_scope + "_" +  iname + "_tie"
    else:
        prefix = POA_prefix(nested)
        if prefix != "": prefix = prefix + "_"
        
        POA_tie_name = prefix + iname + "_tie"
        
    # FIXME: hack because operationArgumentType strips off outermost
    # scope
    env = env.enterScope("dummy")

    # build methods which bind the interface operations and attributes
    # note that this includes inherited callables since tie
    # templates are outside the normal inheritance structure
    where = util.StringStream()

    def buildCallables(interface, where, env, continuation):
        callables = interface.callables()
        operations = filter(lambda x:isinstance(x, idlast.Operation),
                            callables)
        for operation in operations:
            returnType = operation.returnType()
            identifier = operation.identifier()
            parameters = operation.parameters()
            has_return_value = not(tyutil.isVoid(returnType))
            # FIXME: return types are fully scoped but argument types
            # arent?
            returnType_name = tyutil.operationArgumentType(returnType, env,
                                                           fully_scope = 1)[0]
            operation_name = tyutil.mapID(identifier)
            
            signature = []
            call = []

            for parameter in parameters:
                paramType = parameter.paramType()
                dir = parameter.direction() + 1
                argtypes = tyutil.operationArgumentType(paramType, env,
                                                        fully_scope = 0,
                                                        virtualFn = 1)
                param_type_name = argtypes[dir]
                param_id = tyutil.mapID(parameter.identifier())
                signature.append(param_type_name + " " + param_id)
                call.append(param_id)

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
            attrType_names = tyutil.operationArgumentType(attrType, env,
                                                          fully_scope = 1)
            attrType_name_RET = attrType_names[0]
            attrType_name_IN = attrType_names[1]
            
            for identifier in identifiers:
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
            continuation(i, where, env, continuation)

        # done
        return

    buildCallables(node, where, env, buildCallables)
                
        
    stream.out("""\
template <class T>
class @POA_tie_name@ : public virtual @POA_name@
{
public:
  @POA_tie_name@(T& t)
    : pd_obj(&t), pd_poa(0), pd_rel(0) {}
  @POA_tie_name@(T& t, PortableServer::POA_ptr p)
    : pd_obj(&t), pd_poa(p), pd_rel(0) {}
  @POA_tie_name@(T* t, CORBA::Boolean r=1)
    : pd_obj(t), pd_poa(0), pd_rel(r) {}
  @POA_tie_name@(T* t, PortableServer::POA_ptr p,CORBA::Boolean r=1)
    : pd_obj(t), pd_poa(p), pd_rel(r) {}
  ~@POA_tie_name@() {
    if( pd_poa )  CORBA::release(pd_poa);
    if( pd_rel )  delete pd_obj;
  }

  T* _tied_object() { return pd_obj; }

  void _tied_object(T& t) {
    if( pd_rel )  delete pd_obj;
    pd_obj = &t;
    pd_rel = 0;
  }

  void _tied_object(T* t, CORBA::Boolean r=1) {
    if( pd_rel )  delete pd_obj;
    pd_obj = t;
    pd_rel = r;
  }

  CORBA::Boolean _is_owner()        { return pd_rel; }
  void _is_owner(CORBA::Boolean io) { pd_rel = io;   }

  PortableServer::POA_ptr _default_POA() {
    if( !pd_poa )  return PortableServer::POA::_the_root_poa();
    else           return PortableServer::POA::_duplicate(pd_poa);
  }

  @callables@

private:
  T*                      pd_obj;
  PortableServer::POA_ptr pd_poa;
  CORBA::Boolean          pd_rel;
};
""",
               POA_tie_name = POA_tie_name,
               POA_name = POA_name,
               callables = str(where))
    
def visitInterface(node):
    if not(node.mainFile()):
        return
    
    template(self.__environment, node)

def visitEnum(node):
    pass
def visitStruct(node):
    pass
def visitUnion(node):
    pass
def visitForward(node):
    pass
def visitConst(node):
    pass
def visitDeclarator(node):
    pass
def visitMember(node):
    pass
def visitException(node):
    pass
def visitTypedef(node):
    pass
