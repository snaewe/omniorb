# -*- python -*-
#                           Package   : omniidl
# main.py                   Created on: 1999/11/12
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
#   Produce the main skeleton definitions

# $Id$
# $Log$
# Revision 1.7  1999/11/29 15:27:53  djs
# Minor bugfixes
#
# Revision 1.6  1999/11/26 18:51:13  djs
# Now uses proxy module for most proxy generation
#
# Revision 1.5  1999/11/23 18:48:25  djs
# Bugfixes, more interface operations and attributes code
#
# Revision 1.4  1999/11/19 20:12:03  djs
# Generates skeletons for interface operations and attributes
#
# Revision 1.3  1999/11/17 20:37:23  djs
# Code for call descriptors and proxies
#
# Revision 1.2  1999/11/15 19:13:38  djs
# Union skeletons working
#
# Revision 1.1  1999/11/12 17:18:58  djs
# Struct skeleton code added
#

"""Produce the main skeleton definitions"""
# similar to o2be_root::produce_skel in the old C++ BE

import string

from omniidl import idlast, idltype, idlutil

from omniidl.be.cxx import tyutil, util, name, config, skutil

from omniidl.be.cxx.skel import mangler
from omniidl.be.cxx.skel import dispatch
from omniidl.be.cxx.skel import proxy
#import omniidl.be.cxx.skel.util
#skutil = omniidl.be.cxx.skel.util

import main
self = main

# ------------------------------------
# environment handling functions

self.__environment = name.Environment()
self.__insideInterface = 0
self.__insideModule = 0

def enter(scope):
    self.__environment = self.__environment.enterScope(scope)
def leave():
    self.__environment = self.__environment.leaveScope()
def currentScope():
    return self.__environment.scope()

def __init__(stream):
    self.stream = stream
    return self

# ------------------------------------
# Control arrives here

def visitAST(node):
    for n in node.declarations():
        n.accept(self)

def visitModule(node):
    name = tyutil.mapID(node.identifier())
    enter(name)
    scope = currentScope()

    insideModule = self.__insideModule
    self.__insideModule = 1
    for n in node.definitions():
        n.accept(self)

    self.__insideModule = insideModule

    leave()

def visitInterface(node):
    name = tyutil.mapID(node.identifier())
    interface_environment = self.__environment
    enter(name)
    scope = currentScope()
    environment = self.__environment

    insideInterface = self.__insideInterface
    self.__insideInterface = 1

    # produce skeletons for types declared here
    for n in node.declarations():
        n.accept(self)

    self.__insideInterface = insideInterface

    #print "[[[ env = " + str(environment) + "]]]"
    #print "[[[ scopedName = " + repr(node.scopedName()) + "]]]"
    scopedName = map(tyutil.mapID, node.scopedName())
    fqname = string.join(scopedName, "::")
    name = environment.nameToString(scopedName)

    objref_scopedName = tyutil.scope(scopedName) + \
                        ["_objref_" + tyutil.name(scopedName)]
    
    objref_fqname = string.join(objref_scopedName, "::")
    objref_name = environment.nameToString(environment.relName(objref_scopedName))

    impl_scopedName = tyutil.scope(scopedName) + \
                      ["_impl_" + tyutil.name(scopedName)]
    impl_fqname = string.join(impl_scopedName, "::")
    impl_name = environment.nameToString(environment.relName(impl_scopedName))

    #print "[[[  -> objrefName = " + repr(objref_scopedName) +\
    #      "  name = " + repr(objref_name) + "]]]"
    #print "[[[  -> implName = " + repr(impl_scopedName) +\
    #      "  name = " + repr(impl_name) + "]]]"



    # build the helper class methods
    stream.out("""\
@name@_ptr @name@_Helper::_nil() {
  return @name@::_nil();
}

CORBA::Boolean @name@_Helper::is_nil(@name@_ptr p) {
  return CORBA::is_nil(p);\n
}

void @name@_Helper::release(@name@_ptr p) {
  CORBA::release(p);
}

void @name@_Helper::duplicate(@name@_ptr p) {
  if( p )  omni::duplicateObjRef(p);
}

size_t @name@_Helper::NP_alignedSize(@name@_ptr obj, size_t offset) {
  return @name@::_alignedSize(obj, offset);
}

void @name@_Helper::marshalObjRef(@name@_ptr obj, NetBufferedStream& s) {
  @name@::_marshalObjRef(obj, s);
}

@name@_ptr @name@_Helper::unmarshalObjRef(NetBufferedStream& s) {
  return @name@::_unmarshalObjRef(s);
}

void @name@_Helper::marshalObjRef(@name@_ptr obj, MemBufferedStream& s) {
  @name@::_marshalObjRef(obj, s);
}

@name@_ptr @name@_Helper::unmarshalObjRef(MemBufferedStream& s) {
  return @name@::_unmarshalObjRef(s);
}""", name = name)

    # the class itself
    stream.out("""\
@name@_ptr
@name@::_duplicate(@name@_ptr obj)
{
  if( obj )  omni::duplicateObjRef(obj);
   return obj;
  
}


@name@_ptr
@name@::_narrow(CORBA::Object_ptr obj)
{
  if( !obj || obj->_NP_is_nil() || obj->_NP_is_pseudo() ) return _nil();
  _ptr_type e = (_ptr_type) obj->_PR_getobj()->_realNarrow(_PD_repoId);
  return e ? e : _nil();
}


@name@_ptr
@name@::_nil()
{
  static @objref_name@* _the_nil_ptr = 0;
  if( !_the_nil_ptr ) {
    omni::nilRefLock().lock();
    if( !_the_nil_ptr )  _the_nil_ptr = new @objref_name@;
    omni::nilRefLock().unlock();
  }
  return _the_nil_ptr;
}""", name = name, objref_name = objref_name)
    

    # repository ID
    stream.out("""\
const char* @name@::_PD_repoId = \"@repoID@\";""",
               name = name, repoID = node.repoId())

    # proxy

    # build inherits list
    inherits_str = ""
    inherited_repoIDs = ""
    for i in node.inherits():
        inherits_scopedName = map(tyutil.mapID, i.scopedName())
        inherits_name = string.join(inherits_scopedName, "::")
        inherits_objref_scopedName =  tyutil.scope(inherits_scopedName) + \
                                     ["_objref_" + tyutil.name(inherits_scopedName)]
        inherited_repoIDs = inherited_repoIDs + "\
        if( !strcmp(id, " + inherits_name + "::_PD_repoId) )\n\
          return (" + inherits_name + "_ptr) this;\n"
        
        inherits_objref_name = environment.nameToString(environment.relName(
            inherits_objref_scopedName))
        #inherits_objref_name = string.join(inherits_objref_scopedName, "::")
        inherits_str = inherits_str + inherits_objref_name + "(mdri, p, id, lid),\n"
        
    # FIXME: may need to sort out name qualification here
    stream.out("""\
@fq_objref_name@::~@objref_name@() {}


@fq_objref_name@::@objref_name@(const char* mdri,
   IOP::TaggedProfileList* p, omniIdentity* id, omniLocalIdentity* lid) :
   @inherits_str@
   omniObjRef(@name@::_PD_repoId, mdri, p, id, lid)
{
  _PR_setobj(this);
}

void*
@fq_objref_name@::_ptrToObjRef(const char* id)
{
  if( !strcmp(id, CORBA::Object::_PD_repoId) )
    return (CORBA::Object_ptr) this;
  if( !strcmp(id, @name@::_PD_repoId) )
    return (@name@_ptr) this;
  @inherited_repoIDs@
  return 0;
}
""", name = name, fq_objref_name = objref_fqname, objref_name = objref_name,
               inherits_str = inherits_str, inherited_repoIDs = inherited_repoIDs)

    # deal with callables
    callables = node.callables()
    attributes = filter(lambda x:isinstance(x, idlast.Attribute), callables)
    operations = filter(lambda x:isinstance(x, idlast.Operation), callables)
    scopedName = node.scopedName()

    Proxy = proxy.__init__(environment, stream)

    for operation in operations:
        operationName = operation.identifier()
        
        seed = scopedName + [operation.identifier()]
        mangled_signature = mangler.produce_operation_signature(operation)

        # try the all new proxy code!
        Proxy.operation(operation, seed)
        need_proxies = 0
        descriptor = mangler.operation_descriptor_name(operation)
        #try:
        #    descriptor = mangler.operation_descriptor_name(operation)
        #    need_proxies = 0
        #except KeyError:
        #    mangler.generate_descriptors(operation, seed)
        #    descriptor = mangler.operation_descriptor_name(operation)
        #    need_proxies = 1

        parameters = operation.parameters()
        parameters_in  = filter(lambda x:x.is_in(),  parameters)
        parameters_out = filter(lambda x:x.is_out(), parameters)
        parameters_ID  = map(lambda x:tyutil.mapID(x.identifier()), parameters)
        
        parameter_argmapping = []
        parameter_vargmapping = []

        returnType = operation.returnType()
        result_type = tyutil.operationArgumentType(returnType,
                                                   environment,0,1)[0]
        has_return_value = not(tyutil.isVoid(returnType))

        # compute the argument mapping for the operation parameters
        for parameter in parameters:
            paramType = parameter.paramType()
            deref_paramType = tyutil.deref(paramType)
            param_dims = tyutil.typeDims(paramType)
            is_array = param_dims != []
            paramType_name = environment.principalID(paramType, fully_scope = 1)
            
            optypes = tyutil.operationArgumentType(paramType, environment)
            if parameter.is_in() and parameter.is_out():
                parameter_argmapping.append(optypes[3])
            elif parameter.is_in():
                parameter_argmapping.append(optypes[1])
            elif parameter.is_out():
                parameter_argmapping.append(optypes[2])
            # some day I'll figure out a better way of handling this
            if is_array:
                parameter_vargmapping.append(paramType_name + "_slice*")
            elif tyutil.isString(deref_paramType) and parameter.is_out():
                parameter_vargmapping.append("char*&")
            elif tyutil.isObjRef(deref_paramType) and parameter.is_out():
                parameter_vargmapping.append(paramType_name + "_ptr&")
            elif tyutil.isVariableType(deref_paramType) and \
                 parameter.direction() == 1:
                # out only
                parameter_vargmapping.append(paramType_name + "*&")
                
                                             
            else:
                optypes = tyutil.operationArgumentType(paramType, environment, 1, 1)
                if parameter.is_in() and parameter.is_out():
                    parameter_vargmapping.append(optypes[3])
                elif parameter.is_in():
                    parameter_vargmapping.append(optypes[1])
                elif parameter.is_out():
                    parameter_vargmapping.append(optypes[2])
                
        # builds a list of argument initialisers of the form
        #   [ arg_n(a_n) ]*
        # to add to the inheritance list
        def buildinit(x):
            result = []
            for name in x:
                result.append("arg_" + str(name) + "(a_" + str(name) + ")")
            return result

        # builds a list of variable definitions of the form
        #   [ name prefix_n ]*
        # to go in the constructor
        def buildarg(x, prefix, n=0):
            result = []
            for name in x:
                result.append(name + " " + prefix + str(n))
                n = n + 1
            return result

        # build the call descriptor for the operation
        ctor_args = ["LocalCallFn lcfn", "const char* op", "size_t oplen",
                     "_CORBA_Boolean oneway"] +\
                     buildarg(parameter_vargmapping, "a_")
        inherits_list = ["omniCallDescriptor(lcfn, op, oplen, oneway)"] +\
                        buildinit(range(0, len(parameter_argmapping)))
        members = buildarg(parameter_vargmapping, "arg_")

        if has_return_value:
            result_f = "inline " + result_type + " result() { return pd_result; }"
            result_data = result_type + " pd_result;"
        else:
            result_f = ""
            result_data = ""

        marshalArguments_str = """\
        virtual CORBA::ULong alignedSize(CORBA::ULong size_in);
        virtual void marshalArguments(GIOP_C&);"""
        
        unmarshalArguments_str = "virtual void unmarshalReturnedValues(GIOP_C&);"
        if parameters_out == [] and not(has_return_value):
            unmarshalArguments_str = ""
        if parameters_in == []:
            marshalArguments_str = ""
            
        if need_proxies:
            stream.out("""\
// Proxy call descriptor class. Mangled signature:
//  @mangled_signature@
class @call_descriptor@
  : public omniCallDescriptor
{
public:
  inline @call_descriptor@(@ctor_args@):
     @inherits_list@ {}

  @marshalArguments_str@
  @unmarshalArguments_str@
  
  @result_member_function@
  @members@
  @result_member_data@
};

""",
                       mangled_signature = mangled_signature,
                       call_descriptor = descriptor,
                       ctor_args = string.join(ctor_args, ","),
                       inherits_list = string.join(inherits_list, ",\n"),
                       marshalArguments_str = marshalArguments_str,
                       unmarshalArguments_str = unmarshalArguments_str,
                       result_member_function = result_f,
                       result_member_data = result_data,
                       result_type = result_type,
                       members = string.join(map(lambda x:x+";",members), "\n"))
        # build the align method

        # do the size calculation (if there are -in- arguments)
        if parameters_in != []:
            size = util.StringStream()
            n = 0
            for parameter in parameters:
                if parameter.is_in():
                    paramType = parameter.paramType()
                    calc = skutil.sizeCalculation(environment, paramType, None,
                                              "msgsize", "arg_" + str(n), 1)
                    size.out(calc)
                n = n + 1
                
                size_calculation = str(size)

            if need_proxies:
                stream.out("""\
CORBA::ULong @call_descriptor@::alignedSize(CORBA::ULong msgsize)
{
  @size_calculation@
  return msgsize;
}
""",
                           call_descriptor = descriptor,
                           size_calculation = size_calculation)        
        

        # code to marshall the -in- arguments
        if parameters_in != []:
            marshall = util.StringStream()
            n = 0
            for parameter in parameters:
                if parameter.is_in():
                    paramType = parameter.paramType()
                    skutil.marshall(marshall, environment, paramType, None,
                                    "arg_" + str(n), "giop_client",
                                    fully_scope = 1)
                n = n + 1

            if need_proxies:
                stream.out("""\
void @call_descriptor@::marshalArguments(GIOP_C& giop_client)
{
  @marshall@
}
""",
                           call_descriptor = descriptor,
                           marshall = str(marshall))

        # code to unmarshall the -out- arguments
        if parameters_out != [] or has_return_value:
            unmarshall = util.StringStream()
            n = 0
            # build the chunk of code in 3 pieces
            start  = util.StringStream()
            middle = util.StringStream()
            end    = util.StringStream()

            # item_types contains all the argument types + the result
            # item_names contains all the argument names + the result
            # item_direction contains the direction of all the arguments + result
            # item_via_temporary suggests whether to unmarshal via a
            #   temporary variable
            item_types = map(lambda x:x.paramType(), parameters)
            item_names = map(lambda x:"arg_" + str(x), range(0, len(parameters)))
            item_direction = map(lambda x:x.direction(), parameters)
            item_via_temporary = [1] * len(parameters)

            if has_return_value:
                item_types = [returnType] + item_types
                item_names = ["pd_result"] + item_names
                item_direction = [1] + item_direction
                item_via_temporary = [0] + item_via_temporary
                #item_types.append(returnType)
                #item_names.append("pd_result")
                #item_direction.append(1)
                #item_via_temporary.append(0)
            
            zipped_up = util.zip(item_types,
                                 util.zip(item_names,
                                          util.zip(item_direction,
                                                   item_via_temporary)))

            n = -1
            
            for (item_type, (item_name, (item_direction, via_tmp))) in zipped_up:
                deref_item_type = tyutil.deref(item_type)
                type_dims = tyutil.typeDims(item_type)
                is_array = type_dims != []
                
                item_type_name = environment.principalID(item_type, 1)
                assign_to = "tmp_" + str(n)
                n = n + 1
                # this needs uniform(-ing?, -alising?)
                if via_tmp == 0:
                    assign_to = "pd_result"
                    if is_array:
                        start.out("""\
pd_result = @item_type_name@_alloc();""",
                              item_type_name = item_type_name)
                    if tyutil.isVariableType(item_type) and \
                       not(tyutil.isString(item_type)):
                        start.out("""\
pd_result = new @item_type_name@;""", item_type_name = item_type_name)

                if is_array:
                    skutil.unmarshall(middle, environment, item_type, None,
                                      item_name, 1, "giop_client")
                # SWITCH(deref_item_type)
                #  CASE(string)
                elif tyutil.isString(deref_item_type):
                    
                    if item_direction == 1:
                        # out only
                        if via_tmp:
                            start.out("""\
char* @assign_to@ = 0;""", assign_to = assign_to)
                            end.out("""\
@item_name@ = @assign_to@;""", assign_to = assign_to, item_name = item_name)
                    if item_direction == 2:
                        # inout only
                        middle.out("""\
char* @assign_to@;""", assign_to = assign_to)
                    if item_direction == 1 or item_direction == 2:
                        # out and inout
                        middle.out(
                            skutil.unmarshal_string_via_temporary(
                            assign_to,
                            "giop_client"))
                        
                    if item_direction == 2:
                        # inout only
                        middle.out("""\
CORBA::string_free(@item_name@);
@item_name@ = @assign_to@;""", assign_to = assign_to, item_name = item_name)

                #  CASE(objref)
                elif tyutil.isObjRef(deref_item_type):
                    if item_direction == 1:
                        # out only
                        if via_tmp:
                            start.out("""\
@param_type@_ptr @assign_to@ = 0;""", param_type = item_type_name,
                                      assign_to = assign_to)
                        middle.out("""\
@assign_to@ = @param_type@_Helper::unmarshalObjRef(giop_client);""",
                                       param_type = item_type_name,
                                       assign_to = assign_to)
                        if via_tmp:
                            end.out("""\
@item_name@ = @assign_to@;""", assign_to = assign_to, item_name = item_name)
                        
                    if item_direction == 2:
                        middle.out("""\
@param_type@_ptr @assign_to@;
@assign_to@ = @param_type@_Helper::unmarshalObjRef(giop_client);
@param_type@_Helper::release(@item_name@);
@item_name@ = @assign_to@;""", param_type = item_type_name,
                                   item_name = item_name,
                                   assign_to = assign_to)
                #  CASE(sequence)
                elif tyutil.isSequence(deref_item_type):
                    if item_direction == 1:
                        # out only
                        if via_tmp:
                            start.out("""\
@param_type@* @assign_to@ = new @param_type@;""",
                                      param_type = item_type_name,
                                      assign_to = assign_to)
                        else:
                            # the pd_result

                            middle.out("""\
@assign_to@ = new @param_type@;""",
                                      param_type = item_type_name,
                                      assign_to = assign_to)

                        middle.out("""\
*@assign_to@ <<= giop_client;""", assign_to = assign_to)
                        if via_tmp:
                            end.out("""\
@item_name@ = @assign_to@;""", item_name = item_name,
                                    assign_to = assign_to)
                    if item_direction == 2:
                        # inout only
                        middle.out("""\
@item_name@ <<= giop_client;""", item_name = item_name)


                    
                #  DEFAULT
                elif item_direction == 1 or item_direction == 2:
                    
                    skutil.unmarshall(middle, environment, item_type,
                                      None, item_name, 0, "giop_client")
            
                    
            if need_proxies:
                stream.out("""\
void @call_descriptor@::unmarshalReturnedValues(GIOP_C& giop_client)
{
  @unmarshall_start@
  @unmarshall_middle@
  @unmarshall_end@
}

""",
                           call_descriptor = descriptor,
                           unmarshall_start = str(start),
                           unmarshall_middle = str(middle),
                           unmarshall_end = str(end),
                           unmarshall = str(unmarshall))

        # static call back function
        local_call_descriptor = mangler.generate_unique_name(mangler.LCALL_DESC_PREFIX)
        impl_args = map(lambda x: "tcd->arg_" + str(x), range(0, len(parameters)))

        result_string = ""
        if has_return_value:
            result_string = "tcd->pd_result = "
        stream.out("""\
// Local call call-back function.
static void
@local_call_descriptor@(omniCallDescriptor* cd, omniServant* svnt)
{
  @call_descriptor@* tcd = (@call_descriptor@*) cd;
  @impl_fqname@* impl = (@impl_fqname@*) svnt->_ptrToInterface(@name@::_PD_repoId);
  @result@impl->@operation_name@(@operation_arguments@);
}
""",
                   local_call_descriptor = local_call_descriptor,
                   call_descriptor = descriptor,
                   impl_name = impl_name,
                   impl_fqname = impl_fqname,
                   name = name,
                   operation_name = operationName,
                   operation_arguments = string.join(impl_args, ", "),
                   result = result_string)

        # objref::operation
        objref_args = util.zip(parameter_argmapping, parameters_ID)
        objref_args = map(lambda (x,y): x + " " + y, objref_args)
        call_desc_args = [local_call_descriptor, "\"" + operationName + "\"",
                          str(len(operationName) + 1)]
        if operation.oneway():
            call_desc_args.append("1/*oneway*/")
        else:
            call_desc_args.append("0")

        call_desc_args = call_desc_args + parameters_ID

        return_string = ""
        if has_return_value:
            return_string = "return _call_desc.result();"
            
        stream.out("""\
@result_type@ @objref_fqname@::@operation_name@(@arguments@)
{
  @call_descriptor@ _call_desc(@call_desc_args@);
  
  _invoke(_call_desc);
  @return_string@
}


""",
                   result_type = result_type,
                   objref_fqname = objref_fqname,
                   operation_name = operationName,
                   arguments = string.join(objref_args, ", "),
                   call_descriptor = descriptor,
                   call_desc_args = string.join(call_desc_args, ", "),
                   return_string = return_string)
                   
        
    # attributes
    for attribute in attributes:
        seed = scopedName + [attribute.identifiers()[0]]
        #read_signature = mangler.produce_read_attribute_signature(attribute)
        #write_signature = mangler.produce_write_attribute_signature(attribute)

        Proxy = proxy.__init__(environment, stream)
        Proxy.attribute(attribute, seed)
        need_proxies = 0
        #try:
        #    # see if we already have proxies generated
        #    mangler.attribute_read_descriptor_name(attribute)
        #    need_proxies = 0
        #except KeyError:
        #    mangler.generate_descriptors(attribute, seed)
        #    need_proxies = 1
        # 
        read = mangler.attribute_read_descriptor_name(attribute)
        write = mangler.attribute_write_descriptor_name(attribute)
        attrType = attribute.attrType()
        attr_dims = tyutil.typeDims(attrType)
        is_array = attr_dims != []
        
        deref_attrType = tyutil.deref(attrType)
        attrType_name = environment.principalID(attrType, 1)

        attrTypes = tyutil.operationArgumentType(attrType, environment, 0,
                                                 fully_scope = 1)
        scoped_attrTypes = tyutil.operationArgumentType(attrType, environment, 0,
                                                 fully_scope = 0)
        return_type = attrTypes[0]
        if is_array:
            in_type = attrTypes[1]+"_slice*"
            scoped_in_type = scoped_attrTypes[1]+"_slice*"
        else:
            in_type = attrTypes[1]
            scoped_in_type = scoped_attrTypes[1]

        size = skutil.sizeCalculation(environment, attrType, None ,
                                      "msgsize",
                                      "arg_0", 1, fully_scope = 1)
        marshall_stream = util.StringStream()
        skutil.marshall(marshall_stream, environment, attrType, None,
                        "arg_0", "giop_client", fully_scope = 1)

        if is_array:
            s = util.StringStream()
            s.out("pd_result = @name@_alloc();", name = attrType_name)
            # basic types don't have slices
            if tyutil.ttsMap.has_key(deref_attrType.kind()):
                result_string = "pd_result"
            else:
                result_string = "((" + attrType_name + "_slice*)" + "pd_result)"

            skutil.unmarshall(s, environment, attrType, None, result_string, 1,
                              "giop_client", fully_scope = 1)
            unmarshalReturned = str(s)
            
        elif tyutil.isString(deref_attrType):
            unmarshalReturned = skutil.unmarshal_string_via_temporary("pd_result",
                                                                      "giop_client")
        elif tyutil.isObjRef(deref_attrType):
            unmarshalReturned = "\
pd_result = " + attrType_name + "_Helper::unmarshalObjRef(giop_client);"
        elif tyutil.isVariableType(deref_attrType):
            unmarshalReturned = "\
pd_result = new " + attrType_name + ";\n" + "\
*pd_result <<= giop_client;"
        else:
            unmarshalReturned = """\
pd_result <<= giop_client;"""
           

        for id in attribute.identifiers():
            attrib_name = tyutil.mapID(id)

            if need_proxies:
                stream.out("""\
// Proxy call descriptor class. Mangled signature:
//  @read_signature@
class @read_descriptor@
  : public omniCallDescriptor
{
public:
  inline @read_descriptor@(LocalCallFn lcfn, const char* op, size_t oplen, _CORBA_Boolean oneway) :
    omniCallDescriptor(lcfn, op, oplen, oneway)  {}
  
  virtual void unmarshalReturnedValues(GIOP_C&);
  inline @return_type@ result() { return pd_result; }

  @return_type@ pd_result;
};


void @read_descriptor@::unmarshalReturnedValues(GIOP_C& giop_client)
{
  @unmarshalReturned@
}


// Proxy call descriptor class. Mangled signature:
//  @write_signature@
class @write_descriptor@
  : public omniCallDescriptor
{
public:
  inline @write_descriptor@(LocalCallFn lcfn, const char* op, size_t oplen, _CORBA_Boolean oneway, @in_type@ a_0) :
    omniCallDescriptor(lcfn, op, oplen, oneway),
    arg_0(a_0)  {}
  
  virtual CORBA::ULong alignedSize(CORBA::ULong);
  virtual void marshalArguments(GIOP_C&);

  @in_type@ arg_0;
};


CORBA::ULong @write_descriptor@::alignedSize(CORBA::ULong msgsize)
{
  @size_calculation@
  return msgsize;
}


void @write_descriptor@::marshalArguments(GIOP_C& giop_client)
{
  @marshall_argument@
}
""",
                           read_signature = read_signature,
                           write_signature = write_signature,
                           read_descriptor = read, write_descriptor = write,
                           return_type = return_type,
                           in_type = in_type,
                           size_calculation = size,
                           marshall_argument = str(marshall_stream),
                           unmarshalReturned = unmarshalReturned)


            get_attrib_name = "_get_" + attrib_name
            local_call_descriptor = mangler.generate_unique_name(mangler.LCALL_DESC_PREFIX)

            stream.out("""\
// Local call call-back function.
static void
@local_call_descriptor@(omniCallDescriptor* cd, omniServant* svnt)
{
  @read_descriptor@* tcd = (@read_descriptor@*) cd;
  @impl_fqname@* impl = (@impl_fqname@*) svnt->_ptrToInterface(@name@::_PD_repoId);
  tcd->pd_result = impl->@attrib_name@();
}


@return_type@ @objref_fqname@::@attrib_name@()
{
  @read_descriptor@ _call_desc(@local_call_descriptor@, \"@get_attrib_name@\", @len@, 0);
  
  _invoke(_call_desc);
  return _call_desc.result();
}
""",
                       local_call_descriptor = local_call_descriptor,
                       read_descriptor = read,
                       impl_name = impl_name,
                       impl_fqname = impl_fqname,
                       objref_fqname = objref_fqname,
                       name = name,
                       attrib_name = attrib_name,
                       get_attrib_name = get_attrib_name,
                       len = str(len(get_attrib_name) + 1),
                       return_type = return_type)

            if not(attribute.readonly()):
                # make another one of these
                local_call_descriptor = mangler.generate_unique_name(mangler.LCALL_DESC_PREFIX)
                set_attrib_name = "_set_" + attrib_name
                stream.out("""\
// Local call call-back function.
static void
@local_call_descriptor@(omniCallDescriptor* cd, omniServant* svnt)
{
  @write_descriptor@* tcd = (@write_descriptor@*) cd;
  @impl_fqname@* impl = (@impl_fqname@*) svnt->_ptrToInterface(@name@::_PD_repoId);
  impl->@attrib_name@(tcd->arg_0);
}


void @objref_fqname@::@attrib_name@(@in_type@ arg_0)
{
  @write_descriptor@ _call_desc(@local_call_descriptor@, \"@set_attrib_name@\", @len@, 0, arg_0);
  
  _invoke(_call_desc);
}
""",
                           local_call_descriptor = local_call_descriptor,
                           write_descriptor = write,
                           impl_name = impl_name,
                           impl_fqname = impl_fqname,
                           objref_fqname = objref_fqname,
                           name = name,
                           attrib_name = attrib_name,
                           set_attrib_name = set_attrib_name,
                           len = str(len(set_attrib_name) + 1),
                           in_type = scoped_in_type)


    # _pof_
    pof_scopedName = tyutil.scope(scopedName) + \
                      ["_pof_" + tyutil.name(scopedName)]
    pof_name = string.join(pof_scopedName, "::")
    u_name = tyutil.name(scopedName)

    stream.out("""\
@pof_name@::~_pof_@uname@() {}


omniObjRef*
@pof_name@::newObjRef(const char* mdri, IOP::TaggedProfileList* p,
               omniIdentity* id, omniLocalIdentity* lid)
{
  return new @objref_fqname@(mdri, p, id, lid);
}


CORBA::Boolean
@pof_name@::is_a(const char* id) const
{
  if( !strcmp(id, @name@::_PD_repoId) )
    return 1;
    """,
               pof_name = pof_name,
               objref_fqname = objref_fqname,
               name = name,
               uname = u_name)
    for i in node.inherits():
        ancestor = string.join(map(tyutil.mapID, i.scopedName()), "::")
        stream.out("""\
  if( !strcmp(id, @inherited@::_PD_repoId) )
    return 1;
  """, inherited = ancestor)
    stream.out("""\
  return 0;
}
""")

    stream.out("""\
const @pof_name@ _the_pof_@idname@;""",
               pof_name = pof_name, idname = mangler.produce_idname(scopedName))

    # FIXME: There should be an MSVC workaround here.

    # _impl_

    stream.out("""\
@impl_fqname@::~_impl_@uname@() {}


CORBA::Boolean
@impl_fqname@::_dispatch(GIOP_S& giop_s)
{""", impl_fqname = impl_fqname, uname = u_name)
    stream.inc_indent()

    dispatcher = dispatch.__init__(environment, stream)
    
    for callable in node.callables():
        if isinstance(callable, idlast.Operation):
            identifiers = [callable.identifier()]
        else:
            identifiers = callable.identifiers()

        # separate case for each callable thing
        for id in identifiers:
            id_name = tyutil.mapID(id)
            if isinstance(callable, idlast.Operation):
                dispatcher.operation(callable)
                #skutil.operation_dispatch(callable, interface_environment,
                #                          stream)

            elif isinstance(callable, idlast.Attribute):
                
                skutil.attribute_read_dispatch(callable, interface_environment,
                                               id_name, stream)

                if not(callable.readonly()):

                    skutil.attribute_write_dispatch(callable, interface_environment,
                                                    id_name, stream)
    stream.dec_indent()

    # if we don't implement the operation/attrib ourselves, we need to
    # call the objects we inherit from
    for i in node.inherits():
        inherited_name = tyutil.mapID(tyutil.name(i.scopedName()))
        stream.out("""\
  if( _impl_@inherited_name@::_dispatch(giop_s) ) {
    return 1;
  }""", inherited_name = inherited_name)
        
    stream.out("""\
    return 0;
}""")
    
    stream.out("""\
    
void*
@impl_fqname@::_ptrToInterface(const char* id)
{
  if( !strcmp(id, CORBA::Object::_PD_repoId) )
    return (void*) 1;
  if( !strcmp(id, @name@::_PD_repoId) )
    return (@impl_name@*) this;
""",
               impl_name = impl_name,
               impl_fqname = impl_fqname,
               name = name)
    # again deal with inheritance
    for i in node.inherits():
        inherited_name = tyutil.mapID(tyutil.name(i.scopedName()))
        stream.out("""\
  if( !strcmp(id, @inherited_name@::_PD_repoId) )
    return (_impl_@inherited_name@*) this;""",
                   inherited_name = inherited_name)

    stream.out("""\
    
  return 0;
}


const char*
@impl_fqname@::_mostDerivedRepoId()
{
  return @name@::_PD_repoId;
}
""",
               impl_fqname = impl_fqname,
               name = name)
    

    leave()

def visitTypedef(node):
    environment = self.__environment
    is_global_scope = not(self.__insideModule or self.__insideInterface)

    aliasType = node.aliasType()
    alias_dims = tyutil.typeDims(aliasType)

    fq_aliased = environment.principalID(aliasType, 1)

    for d in node.declarators():
        decl_dims = d.sizes()
        decl_dims_str = tyutil.dimsToString(decl_dims)
        
        full_dims = decl_dims + alias_dims
        is_array = full_dims != []
        is_array_declarator = decl_dims != []
        
        fq_derived = idlutil.ccolonName(map(tyutil.mapID, d.scopedName()))

        if is_global_scope and is_array_declarator:
            
            stream.out("""\
@fq_derived@_slice* @fq_derived@_alloc() {
  return new @fq_derived@_slice@decl_dims_str@;
}

@fq_derived@_slice* @fq_derived@_dup(const @fq_derived@_slice* _s)
{
  if (!_s) return 0;
  @fq_derived@_slice* _data = @fq_derived@_alloc();
  if (_data) {
  """, fq_derived = fq_derived, decl_dims_str = decl_dims_str)
            index_str = ""
            i = 0
            stream.inc_indent()
            for dim in full_dims:
                stream.out("for (unsigned int _i@i@ =0;_i@i@ < @n@;_i@i@++){",
                           i = str(i), n = str(dim))
                stream.inc_indent()
                index_str = index_str + "[_i" + str(i) + "]"
                i = i + 1
            stream.out("_data@index@ = _s@index@;", index = index_str)
            for dim in full_dims:
                stream.dec_indent()
                stream.out("}")
            stream.dec_indent()
            stream.out("""\
  }
  return _data;
}

void @fq_derived@_free(@fq_derived@_slice* _s) {
  delete [] _s;
}""", fq_derived = fq_derived)

            return
        

        if is_global_scope and is_array:
            stream.out("""\
extern @fq_derived@_slice* @fq_derived@_alloc() {
  return @fq_aliased@_alloc();
}

extern @fq_derived@_slice* @fq_derived@_dup(const @fq_derived@_slice* p) {
  return @fq_aliased@_dup(p);
}

extern void @fq_derived@_free( @fq_derived@_slice* p) {
   @fq_aliased@_free(p);
}""",
                       fq_derived = fq_derived,
                       fq_aliased = fq_aliased)
            
    
    pass

def visitEnum(node):
    pass

def visitMember(node):
    memberType = node.memberType()
    if node.constrType():
        # if the type was declared here, it must be an instance
        # of idltype.Declared!
        assert isinstance(memberType, idltype.Declared)
        memberType.decl().accept(self)
        
def visitStruct(node):

    environment = self.__environment
    
    name = map(tyutil.mapID, node.scopedName())
    name = string.join(name, "::")

    size_calculation = "omni::align_to(_msgsize, omni::ALIGN_4) + 4"

    marshall = util.StringStream()
    Mem_unmarshall = util.StringStream()
    Net_unmarshall = util.StringStream()
    msgsize = util.StringStream()


    for n in node.members():
        n.accept(self)
        memberType = n.memberType()
        type_dims = tyutil.typeDims(memberType)
        
        for d in n.declarators():
            decl_dims = d.sizes()
            full_dims = decl_dims + type_dims
            is_array = full_dims != []
            # marshall and unmarshall the struct members
            member_name = tyutil.mapID(tyutil.name(d.scopedName()))

            skutil.marshall_struct_union(marshall, environment,
                                         memberType, d, member_name)
            skutil.unmarshall_struct_union(Mem_unmarshall, environment,
                                           memberType, d, member_name, 0)
            skutil.unmarshall_struct_union(Net_unmarshall, environment,
                                           memberType, d, member_name, 1)
            # computation of aligned size
            size = skutil.sizeCalculation(environment, memberType, d,
                                          "_msgsize", member_name)
            msgsize.out("""\
  @size_calculation@""", size_calculation = size)
            
            
            
    stream.out("""\
size_t
@name@::_NP_alignedSize(size_t _initialoffset) const
{
  CORBA::ULong _msgsize = _initialoffset;
  @size_calculation@
  return _msgsize;
}
""", name = name, size_calculation = str(msgsize))
    
    stream.out("""\
void
@name@::operator>>= (NetBufferedStream &_n) const
{
  @marshall_code@
}

void
@name@::operator<<= (NetBufferedStream &_n)
{
  @net_unmarshall_code@
}

void
@name@::operator>>= (MemBufferedStream &_n) const
{
  @marshall_code@
}

void
@name@::operator<<= (MemBufferedStream &_n)
{
  @mem_unmarshall_code@
}
""", name = name,
               marshall_code = str(marshall),
               mem_unmarshall_code = str(Mem_unmarshall),
               net_unmarshall_code = str(Net_unmarshall))
    stream.reset_indent()
    
def visitUnion(node):
    environment = self.__environment
    
    name = map(tyutil.mapID, node.scopedName())
    name = string.join(name, "::")

    switchType = node.switchType()
    deref_switchType = tyutil.deref(switchType)

    exhaustive = tyutil.exhaustiveMatch(switchType,
                                        tyutil.allCaseValues(node))
    defaultCase = tyutil.getDefaultCaseAndMark(node)
    if defaultCase:
        defaultLabel = tyutil.getDefaultLabel(defaultCase)
        defaultMember = tyutil.name(map(tyutil.mapID,
                                        defaultCase.declarator().scopedName()))
        
    hasDefault = defaultCase != None

    # Booleans are a special case (isn't everything?)
    booleanWrap = tyutil.isBoolean(switchType) and exhaustive



    # --------------------------------------------------------------
    # union::_NP_alignedSize(size_t initialoffset) const
    #
    discriminator_size_calc = skutil.sizeCalculation(environment,
                                                     switchType,
                                                     None,
                                                     "_msgsize", "")

    stream.out("""\
size_t
@name@::_NP_alignedSize(size_t initialoffset) const
{
  CORBA::ULong _msgsize = initialoffset;
  @discriminator_size_calc@""",
               name = name,
               discriminator_size_calc = discriminator_size_calc)

    if not(exhaustive):
        stream.out("""\
  if (pd__default) {""")
        if hasDefault:
            caseType = defaultCase.caseType()
            decl = defaultCase.declarator()
            size_calc = skutil.sizeCalculation(environment, caseType,
                                               decl, "_msgsize",
                                               "pd_" + defaultMember)
            stream.inc_indent()
            stream.out("""\
    @size_calc@""", size_calc = size_calc)
            stream.dec_indent()

        stream.out("""\
  }
  else {""")
        stream.inc_indent()

    stream.out("""\
    switch(pd__d) {""")
    stream.inc_indent()
    for c in node.cases():
        caseType = c.caseType()
        deref_caseType = tyutil.deref(caseType)
        decl = c.declarator()
        decl_name =  tyutil.name(map(tyutil.mapID, decl.scopedName()))
        for l in c.labels():
            # default case was already taken care of
            if not(l.default()):
                value =l.value()
                discrim_value = tyutil.valueString(switchType, value, environment)
                stream.out("""\
      case @value@:""", value = str(discrim_value))

                size_calc = skutil.sizeCalculation(environment, caseType, decl,
                                           "_msgsize", "pd_" + decl_name)
                                           
                stream.inc_indent()
                stream.out("""\
        @size_calc@
        break;""", size_calc = size_calc)
                stream.dec_indent()

    if booleanWrap:
        stream.niout("""\
#ifndef HAS_Cplusplus_Bool""")
    stream.out("""\
     default: break;""")
    if booleanWrap:
        stream.niout("""\
#endif""")
        
    stream.dec_indent()
    stream.out("""\
    }""")
    if not(exhaustive):
        stream.dec_indent()
        stream.out("""\
  }""")
    stream.out("""\
  return _msgsize;
}""")

    # --------------------------------------------------------------
    # union::operator{>>, <<}= ({Net, Mem}BufferedStream& _n) [const]
    #
    # FIXME: I thought the CORBA::MARSHAL exception thrown when
    # unmarshalling an array of strings was skipped when unmarshalling
    # from a MemBufferedStream (it is for a struct, but not for a union)
    for where_to in ["NetBufferedStream", "MemBufferedStream"]:
        #can_throw_marshall = where_to == "NetBufferedStream"
        can_throw_marshall = 1

        # marshalling
        stream.out("""\
void
@name@::operator>>= (@where_to@& _n) const
{
  pd__d >>= _n;""", name = name, where_to = where_to)
        
        if not(exhaustive):
            stream.out("""\
  if (pd__default) {""")
            if hasDefault:
                caseType = defaultCase.caseType()
                decl = defaultCase.declarator()
                decl_name =  tyutil.name(map(tyutil.mapID, decl.scopedName()))
                stream.inc_indent()
                skutil.marshall_struct_union(stream, environment, caseType,
                                             decl, "pd_" + decl_name)
                stream.dec_indent()
            stream.out("""\
  }
  else {""")
            stream.inc_indent()

        stream.out("""\
    switch(pd__d) {""")
        stream.inc_indent()
        
        for c in node.cases():
            caseType = c.caseType()
            decl = c.declarator()
            decl_name = tyutil.name(map(tyutil.mapID, decl.scopedName()))
            for l in c.labels():
                if not(l.default()):
                   value =l.value()
                   discrim_value = tyutil.valueString(switchType, value,
                                                      environment)
                   stream.out("""\
      case @value@:""", value = str(discrim_value))
                   stream.inc_indent()
                   skutil.marshall_struct_union(stream, environment, caseType,
                                                decl, "pd_" + decl_name)
                   stream.out("""\
        break;""")
                   stream.dec_indent()
                   
        if booleanWrap:
           stream.niout("""\
#ifndef HAS_Cplusplus_Bool""")
        stream.out("""\
     default: break;""")
        if booleanWrap:
            stream.niout("""\
#endif""")

        stream.dec_indent()
        stream.out("""\
    }""")
        

        if not(exhaustive):
            stream.dec_indent()
            stream.out("""\
  }""")

        stream.dec_indent()
        stream.out("""\
}""")


        # unmarshalling
        stream.out("""\
void
@name@::operator<<= (@where_to@& _n)
{
  pd__d <<= _n;
  switch(pd__d) {""", name = name, where_to = where_to)
        stream.inc_indent()

        for c in node.cases():
            caseType = c.caseType()
            decl = c.declarator()
            decl_name = tyutil.name(map(tyutil.mapID, decl.scopedName()))
            
            isDefault = defaultCase == c
            
            for l in c.labels():
                if l.default():
                    stream.out("""\
      default:""")
                else:
                    value =l.value()
                    discrim_value = tyutil.valueString(switchType, value,
                                                       environment)
                    stream.out("""\
      case @value@:""", value = str(discrim_value))

            stream.inc_indent()
            stream.out("""\
        pd__default = @isDefault@;""", isDefault = str(isDefault))
            
            skutil.unmarshall_struct_union(stream, environment, caseType, decl,
                                           "pd_" + decl_name, can_throw_marshall)
            stream.out("""\
        break;""")
            stream.dec_indent()
            
        if not(hasDefault) and not(exhaustive):
            stream.out("""\
      default: pd__default = 1; break;""")

        stream.dec_indent()
        stream.out("""\
  }
}""")
        

        
        
    return
    
    
def visitForward(node):
    pass
def visitConst(node):
    environment = self.__environment
    constType = node.constType()
    if tyutil.isString(constType):
        type_string = "char *"
    else:
        type_string = environment.principalID(constType)

    scopedName = node.scopedName()
    scopedName = map(tyutil.mapID, scopedName)
    name = idlutil.ccolonName(scopedName)
    value = tyutil.valueString(constType, node.value(), environment)
    
    init_in_def = tyutil.const_init_in_def(constType)

    
    if init_in_def:
        if self.__insideInterface:
            stream.out("""\
const @type@ @name@ _init_in_cldef_( = @value@ );""",
                       type = type_string, name = name, value = value)
        else:
            stream.out("""\
_init_in_def_( const @type@ @name@ = @value@; )""",
                       type = type_string, name = name, value = value)
        return

    # not init_in_def
    if self.__insideModule and not(self.__insideInterface):
        scopedName = node.scopedName()
        scopedName = map(tyutil.mapID, scopedName)
        scope_str = idlutil.ccolonName(tyutil.scope(scopedName))
        name_str = tyutil.name(scopedName)
        stream.out("""\
#if defined(HAS_Cplusplus_Namespace) && defined(_MSC_VER)
// MSVC++ does not give the constant external linkage othewise.
namespace @scope@ {
  extern const @type@ @name@=@value@;
}
#else
const @type@ @scopedName@ = @value@;
#endif""",
        type = type_string, scope = scope_str, name = name_str,
        scopedName = name, value = value)
        
    else:
        stream.out("""\
const @type@ @name@ = @value@;""",
                   type = type_string, name = name, value = value)
        

    
    pass
def visitDeclarator(node):
    pass
def visitException(node):
    name = tyutil.mapID(tyutil.name(node.scopedName()))
    enter(name)
    environment = self.__environment
    scoped_name = environment.nameToString(node.scopedName())
    name = tyutil.mapID(tyutil.name(node.scopedName()))
    repoID = node.repoId()

    # build the default ctor, copy ctor, assignment operator
    copy_ctor_body = util.StringStream()
    default_ctor_body = util.StringStream()
    default_ctor_args = []
    assign_op_body = util.StringStream()
    has_default_ctor = 0

    for m in node.members():
        has_default_ctor = 1
        if m.constrType():
            raise "Doesn't handle types constructed within an exception"
        memberType = m.memberType()
        memberType_name = environment.principalID(memberType)
        memberType_name_arg = tyutil.makeConstructorArgumentType(memberType,
                                                                 environment)
        for d in m.declarators():
            decl_name = tyutil.mapID(tyutil.name(d.scopedName()))
            copy_ctor_body.out("""\
@member_name@ = _s.@member_name@;""", member_name = decl_name)
            
            default_ctor_args.append(memberType_name_arg + " _" + decl_name)
            default_ctor_body.out("""\
@member_name@ = _@member_name@;""", member_name = decl_name)

            assign_op_body.out("""\
@member_name@ = _s.@member_name@;""", member_name = decl_name)
            
        
          
        
    
    stream.out("""\
CORBA::Exception::insertExceptionToAny @scoped_name@::insertToAnyFn = 0;
CORBA::Exception::insertExceptionToAnyNCP @scoped_name@::insertToAnyFnNCP = 0;

@scoped_name@::@name@(const @scoped_name@& _s) : CORBA::UserException(_s)
{
  @copy_ctor_body@
}
""",
               scoped_name = scoped_name,
               name = name,
               copy_ctor_body = str(copy_ctor_body))
    if has_default_ctor:
        stream.out("""\
@scoped_name@::@name@(@ctor_args@)
{
  pd_insertToAnyFn    = @scoped_name@::insertToAnyFn;
  pd_insertToAnyFnNCP = @scoped_name@::insertToAnyFnNCP;
  @default_ctor_body@
}
""",
                   scoped_name = scoped_name,
                   name = name,
                   ctor_args = string.join(default_ctor_args, ", "),
                   default_ctor_body = str(default_ctor_body))
    stream.out("""\
@scoped_name@& @scoped_name@::operator=(const @scoped_name@& _s)
{
  ((CORBA::UserException*) this)->operator=(_s);
  @assign_op_body@
  return *this;
}

@scoped_name@::~@name@() {}

void @scoped_name@::_raise() { throw *this; }

@scoped_name@* @scoped_name@::_downcast(CORBA::Exception* e) {
  return (@name@*) _NP_is_a(e, \"Exception/UserException/@scoped_name@\");
}

const @scoped_name@* @scoped_name@::_downcast(const CORBA::Exception* e) {
  return (const @name@*) _NP_is_a(e, \"Exception/UserException/@scoped_name@\");
}

const char* @scoped_name@::_PD_repoId = \"@repoID@\";

CORBA::Exception* @scoped_name@::_NP_duplicate() const {
  return new @name@(*this);
}

const char* @scoped_name@::_NP_typeId() const {
  return \"Exception/UserException/@scoped_name@\";
}

const char* @scoped_name@::_NP_repoId(int* _size) const {
  *_size = sizeof(\"@repoID@\");
  return \"@repoID@\";
}
 
void @scoped_name@::_NP_marshal(NetBufferedStream& _s) const {
  *this >>= _s;
}

void @scoped_name@::_NP_marshal(MemBufferedStream& _s) const {
  *this >>= _s;
}
""",
               scoped_name = scoped_name,
               name = name,
               repoID = repoID,
               assign_op_body = str(assign_op_body))
    

    # deal with alignment, marshalling and demarshalling
    needs_marshalling = node.members() != []
    aligned_size = util.StringStream()
    mem_marshal = util.StringStream()
    net_marshal = util.StringStream()
    mem_unmarshal = util.StringStream()
    net_unmarshal = util.StringStream()
    
    for m in node.members():
        memberType = m.memberType()
        for d in m.declarators():
            decl_name = tyutil.mapID(tyutil.name(d.scopedName()))
            if tyutil.isString(memberType):
                tmp = skutil.unmarshal_string_via_temporary(decl_name, "_n")
                mem_unmarshal.out(tmp)
                net_unmarshal.out(tmp)
            else:
                skutil.unmarshall(mem_unmarshal, environment,
                                  memberType, d, decl_name, 0, "_n")
                skutil.unmarshall(net_unmarshal, environment,
                                  memberType, d, decl_name, 1, "_n")

            skutil.marshall(mem_marshal, environment,
                            memberType, d, decl_name, "_n")
            skutil.marshall(net_marshal, environment,
                            memberType, d, decl_name, "_n")

            aligned_size.out(skutil.sizeCalculation(environment, memberType,
                                                    d, "_msgsize", decl_name,
                                                    fixme = 1))

    if needs_marshalling:
        stream.out("""\
size_t
@scoped_name@::_NP_alignedSize(size_t _msgsize) const
{
  @aligned_size@
  return _msgsize;
}

void
@scoped_name@::operator>>= (NetBufferedStream& _n) const
{
  @net_marshal@
}

void
@scoped_name@::operator<<= (NetBufferedStream& _n)
{
  @net_unmarshal@
}

void
@scoped_name@::operator>>= (MemBufferedStream& _n) const
{
  @mem_marshal@
}

void
@scoped_name@::operator<<= (MemBufferedStream& _n)
{
  @mem_unmarshal@
}
""",
                   scoped_name = scoped_name,
                   aligned_size = str(aligned_size),
                   net_marshal = str(net_marshal),
                   mem_marshal = str(mem_marshal),
                   net_unmarshal = str(net_unmarshal),
                   mem_unmarshal = str(mem_unmarshal))


    leave()
            
            
                                           
