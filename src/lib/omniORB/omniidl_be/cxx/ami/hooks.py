# -*- python -*-
#                           Package   : omniidl
# hooks.py                  Created on: 2000/08/15
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
#   Hooks into the main code generation routines

import string
from omniidl import idlast
from omniidl_be.cxx import id, iface, header, skel, config
from omniidl_be.cxx.ami import ami, exholder, poller, objref, rhandler
import hooks

self = hooks

def Interface_defs(node):
    assert isinstance(node, idlast.Interface)

    if not(node.mainFile()): return

    stream = header.defs.stream
    environment = id.lookup(node)
    node_name = id.Name(node.scopedName())
    ami_name = id.Name(node.ReplyHandler.scopedName())

    # forward declare the replyHandler and the interface itself
    # (they get referenced in the new sendc_ objref methods)
    stream.out("""\
class @replyhandler@;
class _objref_@replyhandler@;
typedef _objref_@replyhandler@* @replyhandler@_ptr;
class @interface@;
class _objref_@interface@;
typedef _objref_@interface@* @interface@_ptr;
class @I_Helper@;
""", replyhandler = ami_name.simple(), interface = node_name.simple(),
               I_Helper = node_name.suffix("_Helper").simple())
    
    # This uses the Type Specific ExceptionHolder and the Type Specific
    # Poller valuetype. Better define them :)
    interface = iface.Interface(node)
    
    # Make the normal interface definitions, with the new _objref_ code
    old_objref_I = iface.instance("_objref_I")
    iface.register_class("_objref_I", objref._objref_I)
    self.defs_visitInterface(node)
    iface.register_class("_objref_I", old_objref_I)

    handler = rhandler.IHandler(node)

    _Helper = iface.I_Helper(handler)
    _Helper.hh(stream)

    stream.out(header.template.interface_type,
               name = ami_name.simple(), Other_IDL = "")

    ExceptionHolder = exholder.ExceptionHolder(interface)
    ExceptionHolder.hh(stream)

    Poller = poller.Poller(interface)
    Poller.hh(stream)


    
    _objref = rhandler._objref_IHandler(handler)
    _objref.hh(stream)

    _pof = iface._pof_I(handler)
    _pof.hh(stream)

    _impl = rhandler._impl_IHandler(handler)
    _impl.hh(stream)

    # Generate BOA compatible skeletons?
    if config.state['BOA Skeletons']:
        _sk_I = iface._sk_I(handler)
        _sk_I.hh(stream)

    
    
def Interface_opers(node):
    assert isinstance(node, idlast.Interface)
    self.opers_visitInterface(node)

    if not(node.mainFile()): return

    environment = id.lookup(node)

    node_name = id.Name(node.scopedName())
    ami_name = id.Name(node.ReplyHandler.scopedName())
    # New IDL has different name => different repoId
    new_repoId = node.ReplyHandler.repoId()
    idLen = len(new_repoId) + 1

    header.opers.stream.out(header.template.interface_marshal_forward,
                            name = ami_name.fullyQualify(),
                            idLen = str(idLen))  
    


def Interface_poa(node):
    assert isinstance(node, idlast.Interface)
    self.poa_visitInterface(node)

    if not(node.mainFile()): return

    environment = id.lookup(node)

    node_name = id.Name(node.scopedName())
    ami_name = id.Name(node.ReplyHandler.scopedName())

    poa_name = ami_name.simple()
    if ami_name.scope() == []:
        poa_name = "POA_" + poa_name

    header.poa.stream.out(header.template.POA_interface,
                POA_name = poa_name,
                impl_scopedID = ami_name.prefix("_impl_").fullyQualify(),
                inherits = "public virtual POA_Messaging::ReplyHandler",
                scopedID = ami_name.fullyQualify())



def Implementation(node):
    assert isinstance(node, idlast.Interface)
    if not(node.mainFile()): return

    environment = id.lookup(node)

    # write new _objref_I stuff
    old_objref_I = iface.instance("_objref_I")
    iface.register_class("_objref_I", objref._objref_I)
    self.skel_visitInterface(node)
    iface.register_class("_objref_I", old_objref_I)

    # New stuff
    stream = skel.main.stream
    
    interface = iface.Interface(node)
    
    ExceptionHolder = exholder.ExceptionHolder(interface)
    ExceptionHolder.cc(stream)

    Poller = poller.Poller(interface)
    Poller.cc(stream)
    
    handler = rhandler.IHandler(node)

    _Helper = iface.I_Helper(handler)
    _Helper.cc(stream)

    objref_name = handler.name().prefix("_objref_")
    repoId = node.ReplyHandler.repoId()
    stream.out(skel.template.interface_class,
               name = handler.name().fullyQualify(),
               objref_name = objref_name.unambiguous(environment),
               repoID = repoId)

    # inherit from Messaging::ReplyHandler, use flattened typedef just
    # in case of compiler problems
    stream.out(skel.template.interface_ALIAS,
               guard_name = "Messaging_ReplyHandler",
               fqname = "Messaging::ReplyHandler",
               flat_fqname = "Messaging_ReplyHandler",
               impl_fqname = "Messaging::_impl_ReplyHandler",
               impl_flat_fqname = "Messaging__impl_ReplyHandler",
               objref_fqname = "Messaging::_objref_ReplyHandler",
               objref_flat_fqname = "Messaging__objref_ReplyHandler")


    _objref = rhandler._objref_IHandler(handler)
    _objref.cc(stream)

    _pof = rhandler.iface._pof_I(handler)
    _pof.cc(stream)

    _impl = rhandler._impl_IHandler(handler)
    _impl.cc(stream)

    # BOA compatible skeletons
    if config.state['BOA Skeletons']:
        _sk_I = iface._sk_I(handler)
        _sk_I.hh(stream)

def Implementation_poa(node):
    assert isinstance(node, idlast.Interface)
    self.skel_poa_visitInterface(node)

    if not(node.mainFile()): return

    stream = skel.poa.stream

    environment = id.lookup(node)
    node_name = id.Name(node.scopedName())
    ami_name = id.Name(node.ReplyHandler.scopedName())

    prefix = ""
    if len(ami_name.fullName()) == 1: prefix = "POA_"

    stream.out(skel.template.interface_POA,
               POA_prefix = prefix,
               name = ami_name.simple(),
               fqname = ami_name.fullyQualify())

    

