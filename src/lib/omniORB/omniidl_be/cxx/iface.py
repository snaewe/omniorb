# -*- python -*-
#                           Package   : omniidl
# iface.py                  Created on: 2000/8/10
#			    Author    : David Scott (djs)
#
#    Copyright (C) 2000 AT&T Laboratories Cambridge
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
#   Code associated with IDL interfaces

# $Id$
# $Log$
# Revision 1.1.4.5  2001/06/08 17:12:13  dpg1
# Merge all the bug fixes from omni3_develop.
#
# Revision 1.1.4.4  2001/01/25 13:09:11  sll
# Fixed up cxx backend to stop it from dying when a relative
# path name is given to the -p option of omniidl.
#
# Revision 1.1.4.3  2000/11/07 18:27:51  sll
# Pass environment to out_objrefcall.
#
# Revision 1.1.4.2  2000/11/03 19:30:21  sll
# Rationalise code generation. Consolidate all code that use call descriptors
# into the CallDescriptor class.
#
# Revision 1.1.4.1  2000/10/12 15:37:47  sll
# Updated from omni3_1_develop.
#
# Revision 1.1.2.2  2000/09/14 16:03:02  djs
# Remodularised C++ descriptor name generator
# Bug in listing all inherited interfaces if one is a forward
# repoID munging function now handles #pragma ID in bootstrap.idl
# Naming environments generating code now copes with new IDL AST types
# Modified type utility functions
# Minor tidying
#
# Revision 1.1.2.1  2000/08/21 11:34:34  djs
# Lots of omniidl/C++ backend changes
#

# o Keep related code in one place
# o Expose internals at a finer granularity than before (useful for
#   overriding one aspect (eg _objref class for AMI))

import string

from omniidl import idlast, idltype
from omniidl_be.cxx import types, id, call, ast, cxx, output, config, descriptor
# from omniidl_be.cxx import header
# from omniidl_be.cxx import skel
# XXX it seems that the above import fails when this file is import by
#     cxx.header.defs AND a relative patch -p argument is given to omniidl
#     Use the following import works.
import omniidl_be.cxx.skel
import omniidl_be.cxx.header


# Interface is a wrapper around an IDL interface
#  .callables():   get a list of Callable objects representing the operations
#                  and attributes
#  .inherits():    get a list of all directly inherited interfaces
#  .allInherits(): get all inherited interfaces (using a breadth first search)
#  .name():        return the IDL fully scoped name (as an id.Name)
#  .environment(): returns the IDL environment where this interface was
#                  declared
class Interface:
  """Wrapper around an IDL interface"""
  def __init__(self, node):
    self._node = node
    self._environment = id.lookup(node)
    self._node_name = id.Name(node.scopedName())

  def callables(self):
    """Return a list of Callable objects representing the combined operations
       and attributes for this interface"""
    
    if hasattr(self, "_callables"):
      return self._callables
    
    # build a list of all the Callable objects
    # The old backend processed all operations first
    # (FIXME: duplicate for the sake of easy checking)
    self._callables = []

    for c in self._node.callables():
      if isinstance(c, idlast.Operation):
        self._callables.append(call.operation(self, c))
        
    for c in self._node.callables():
      if isinstance(c, idlast.Attribute):
        self._callables = self._callables + call.read_attributes(self, c)
        if c.readonly(): continue
        self._callables = self._callables + call.write_attributes(self, c)
      
    return self._callables

  def inherits(self):
    return map(lambda x:Interface(x), self._node.inherits())

  def allInherits(self):
    return map(lambda x:Interface(x), ast.allInherits(self._node))

  def name(self):
    return self._node_name
    
  def environment(self):
    return self._environment
  
  
_classes = {}
_proxy_call_descriptors = {}

def instance(name):
  if _classes.has_key(name):
    return _classes[name]

  instance = eval(name)
  _classes[name] = instance
  return instance

def register_class(name, cl):
  _classes[name] = cl


# Class associated with an IDL interface.
#  .interface():   return the associated Interface object
#  .methods():     return a list of Method objects
#  .environment(): return the IDL environment associated with the interface
class Class(cxx.Class):
  def __init__(self, interface):
    assert isinstance(interface, Interface)
    cxx.Class.__init__(self, interface.name())
    
    self._interface = interface
    self._environment = interface.environment()
    self._methods = []
    self._callables = {}

  def interface(self):   return self._interface
  def methods(self):     return self._methods
  def environment(self): return self._environment


class _objref_Method(cxx.Method):
  def __init__(self, callable, parent_class):
    assert isinstance(callable, call.Callable)
    assert isinstance(parent_class, cxx.Class)
    self._callable = callable
    self._parent_class = parent_class
    self.from_Callable()

  def callable(self): return self._callable

  def from_Callable(self):
    self._from_Callable(use_out = 1)

  def _from_Callable(self, use_out):
    # Grab the IDL environment
    environment = self.callable().interface().environment()

    # Kept as a type object because in .cc part the _return_ type
    # must be fully qualified.
    self._return_type = types.Type(self.callable().returnType())

    # Parameters are always relative, both in .hh and .cc
    (param_types, param_names) = ([], [])
    for p in self.callable().parameters():
      pType = types.Type(p.paramType())
      direction = types.direction(p)
      param_types.append(pType.op(direction, environment,
                                  use_out = use_out))
      param_names.append(p.identifier())
      
    # an operation has optional context
    if self.callable().contexts() != []:
      param_types.append("CORBA::Context_ptr")
      param_names.append("_ctxt")

    self._arg_types = param_types
    self._arg_names = param_names
    self._name = self.callable().method_name()


class _impl_Method(_objref_Method):
  def __init__(self, callable, parent_class):
    _objref_Method.__init__(self, callable, parent_class)

  def from_Callable(self):
    self._from_Callable(use_out = 0)


class I_Helper(Class):
  def __init__(self, I):
    Class.__init__(self, I)
    self._name = self._name.suffix("_Helper")

  def hh(self, stream):
    class_sk_name = ""
    if config.state['BOA Skeletons']:
      class_sk_name = "class " + \
                      self.interface().name().prefix("_sk_").simple() + ";"
    stream.out(omniidl_be.cxx.header.template.interface_Helper,
               class_sk_name = class_sk_name,
               name = self.interface().name().simple(),
               guard = self.interface().name().guard())

  def cc(self, stream):
    stream.out(omniidl_be.cxx.skel.template.interface_Helper,
               name = self.interface().name().fullyQualify())


class _objref_I(Class):
  def __init__(self, I):
    Class.__init__(self, I)
    self._name = self._name.prefix("_objref_")

    for callable in self.interface().callables():
      method = _objref_Method(callable, self)
      self._methods.append(method)
      self._callables[method] = callable


  def hh(self, stream):
    # build the inheritance list
    objref_inherits = []
    for i in self.interface().inherits():
        objref_inherited_name = i.name().prefix("_objref_")
        uname = objref_inherited_name.unambiguous(self._environment)
        objref_inherits.append("public virtual " + uname)

    # if already inheriting, the base class will be present
    # (transitivity of the inherits-from relation)
    if self.interface().inherits() == []:
        objref_inherits = [ "public virtual CORBA::Object, " + \
                            "public virtual omniObjRef" ]

    methods = []
    for method in self.methods():
      methods.append(method.hh())
            
    stream.out(omniidl_be.cxx.header.template.interface_objref,
               name = self.interface().name().simple(),
               inherits = string.join(objref_inherits, ",\n"),
               operations = string.join(methods, "\n"))

  def cc(self, stream):

    def _ptrToObjRef(self = self, stream = stream):
      for i in self.interface().allInherits():
        stream.out(omniidl_be.cxx.skel.template.interface_objref_repoID,
                   inherits_fqname = i.name().fullyQualify())

    # build the inherits list
    inherits_str = ""
    for i in self.interface().inherits():
      objref_name = i.name().prefix("_objref_")

      objref_str = objref_name.unambiguous(self._environment)

      if objref_name.needFlatName(self._environment):
        objref_str = objref_name.flatName()

      this_inherits_str = objref_str + "(ior, id, lid),\n"

      # FIXME:
      # The powerpc-aix OMNIORB_BASE_CTOR workaround still works here
      # (in precendence to the flattened base name) but lacking a
      # powerpc-aix test machine I can't properly test it. It's probably
      # not required any more.
      if objref_name.relName(self._environment) != i.name().fullName():
        prefix = []
        for x in objref_name.fullName():
          if x == "_objref_" + objref_name.relName(self._environment)[0]:
            break
          prefix.append(x)
        inherits_scope_prefix = string.join(prefix, "::") + "::"
        this_inherits_str = "OMNIORB_BASE_CTOR(" + inherits_scope_prefix +\
                            ")" + this_inherits_str
      inherits_str = inherits_str + this_inherits_str

    stream.out(omniidl_be.cxx.skel.template.interface_objref,
               name = self.interface().name().fullyQualify(),
               fq_objref_name = self.name().fullyQualify(),
               objref_name = self.name().simple(),
               inherits_str = inherits_str,
               _ptrToObjRef = _ptrToObjRef)
    
      
    for method in self.methods():
      callable = self._callables[method]

      # signature is a text string form of the complete operation signature
      signature = callable.signature()

      # we only need one descriptor for each _signature_ (not operation)
      if _proxy_call_descriptors.has_key(signature):
        call_descriptor = _proxy_call_descriptors[signature]
      else:
        call_descriptor = call.CallDescriptor(signature,callable)
        call_descriptor.out_desc(stream)
        _proxy_call_descriptors[signature] = call_descriptor

      # produce a localcall function
      node_name = self.interface().name()
      localcall_fn = descriptor.local_callback_fn(node_name,
                                                  callable.operation_name(),
                                                  signature)
      call_descriptor.out_localcall(stream,node_name,callable.method_name(),
                                    localcall_fn)

      # produce member function for this operation/attribute.
      body = output.StringStream()
      argnames = []
      for parameter in callable.parameters():
        argnames.append(parameter.identifier())
        
      call_descriptor.out_objrefcall(body,
                                     callable.operation_name(),
                                     argnames,
                                     localcall_fn,
                                     self._environment)
      method.cc(stream, body)

class _pof_I(Class):
  def __init__(self, I):
    Class.__init__(self, I)
    self._name = self._name.prefix("_pof_")

  def hh(self, stream):
    stream.out(omniidl_be.cxx.header.template.interface_pof,
               name = self.interface().name().simple())

  def cc(self, stream):
    inherits = output.StringStream()
    for i in self.interface().allInherits():
      ancestor = i.name().fullyQualify()
      inherits.out(omniidl_be.cxx.skel.template.interface_pof_repoID, inherited = ancestor)

    node_name = self.interface().name()
    objref_name = node_name.prefix("_objref_")
    pof_name = node_name.prefix("_pof_")
    stream.out(omniidl_be.cxx.skel.template.interface_pof,
               pof_name = pof_name.fullyQualify(),
               objref_fqname = objref_name.fullyQualify(),
               name = node_name.fullyQualify(),
               uname = pof_name.simple(),
               Other_repoIDs = inherits,
               idname = node_name.guard())
    

class _impl_I(Class):
  def __init__(self, I):
    Class.__init__(self, I)
    self._name = self._name.prefix("_impl_")

    for callable in self.interface().callables():
      method = _impl_Method(callable, self)
      self._methods.append(method)
      self._callables[method] = callable

  def hh(self, stream):
    # build the inheritance list
    environment = self._environment
    impl_inherits = []
    for i in self.interface().inherits():
      impl_inherited_name = i.name().prefix("_impl_")
      uname = impl_inherited_name.unambiguous(environment)
      impl_inherits.append("public virtual " + uname)

    # if already inheriting, the base class will be present
    # (transitivity of the inherits-from relation)
    if self.interface().inherits() == []:
      impl_inherits   = [ "public virtual omniServant" ]


    methods = []
    for method in self.methods():
        methods.append(method.hh(virtual = 1, pure = 1))
        
    stream.out(omniidl_be.cxx.header.template.interface_impl,
               name = self.interface().name().simple(),
               inherits = string.join(impl_inherits, ",\n"),
               operations = string.join(methods, "\n"))

  def cc(self, stream):

    # Function to write the _impl_I::dispatch method
    def dispatch(self = self, stream = stream):
      # first check if method is from this interface
      dispatched = []
      for method in self.methods():
        callable = self._callables[method]
        operation_name = callable.operation_name()
        if operation_name not in dispatched:
          signature = callable.signature()
          call_descriptor = _proxy_call_descriptors[signature]
          localcall_fn = descriptor.local_callback_fn(self.interface().name(),
                                                      operation_name,signature)
          call_descriptor.out_implcall(stream,operation_name,localcall_fn)
          dispatched.append(operation_name)

      # next call dispatch methods of superclasses
      for i in self.interface().inherits():
        inherited_name = i.name().prefix("_impl_")
        impl_inherits = inherited_name.simple()
        # The MSVC workaround might be needed here again
        if inherited_name.needFlatName(self._environment):
          impl_inherits = inherited_name.flatName()
        stream.out(omniidl_be.cxx.skel.template.interface_impl_inherit_dispatch,
                   impl_inherited_name = impl_inherits)

    # For each of the inherited interfaces, check their repoId strings
    def _ptrToInterface(self = self, stream = stream):
      for i in self.interface().allInherits():
        inherited_name = i.name()
        impl_inherited_name = inherited_name.prefix("_impl_")
        inherited_str = inherited_name.unambiguous(self._environment)
        impl_inherited_str = impl_inherited_name.unambiguous(self._environment)
        if inherited_name.needFlatName(self._environment):
          inherited_str = inherited_name.flatName()
          impl_inherited_str = impl_inherited_name.flatName()
        stream.out(omniidl_be.cxx.skel.template.interface_impl_repoID,
                   inherited_name = inherited_str,
                   impl_inherited_name = impl_inherited_str)

    node_name = self.interface().name()
    impl_name = node_name.prefix("_impl_")

    stream.out(omniidl_be.cxx.skel.template.interface_impl,
               impl_fqname = impl_name.fullyQualify(),
               uname = node_name.simple(),
               dispatch = dispatch,
               impl_name = impl_name.unambiguous(self._environment),
               _ptrToInterface = _ptrToInterface,
               name = node_name.fullyQualify())
          

class _sk_I(Class):
  def __init__(self, I):
    Class.__init__(self, I)

  def hh(self, stream):
    # build the inheritance list
    environment = self._environment
    sk_inherits = []
    for i in self.interface().inherits():
      sk_inherited_name = i.name().prefix("_sk_")
      uname = sk_inherited_name.unambiguous(environment)
      sk_inherits.append("public virtual " + uname)

    # if already inheriting, the base class will be present
    # (transitivity of the inherits-from relation)
    if self.interface().inherits() == []:
      sk_inherits   = [ "public virtual omniOrbBoaServant" ]

    stream.out(omniidl_be.cxx.header.template.interface_sk,
               name = self.interface().name().simple(),
               inherits = string.join(sk_inherits, ",\n"))






