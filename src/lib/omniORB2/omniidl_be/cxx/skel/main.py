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
#import omniidl.be.cxx.skel.util
#skutil = omniidl.be.cxx.skel.util

import main
self = main

# ------------------------------------
# environment handling functions

self.__environment = name.Environment()

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
#    enter(name)
    scope = currentScope()

    for n in node.definitions():
        n.accept(self)

#    leave()

def visitInterface(node):
    name = tyutil.mapID(node.identifier())
#    enter(name)
    scope = currentScope()
    environment = self.__environment
    

    # produce skeletons for types declared here
    for n in node.declarations():
        n.accept(self)

    scopedName = map(tyutil.mapID, node.scopedName())
    name = string.join(scopedName, "::")

    objref_scopedName = tyutil.scope(scopedName) + \
                        ["_objref_" + tyutil.name(scopedName)]
    
    objref_name = string.join(objref_scopedName, "::")

    impl_scopedName = tyutil.scope(scopedName) + \
                      ["_impl_" + tyutil.name(scopedName)]
    impl_name = string.join(impl_scopedName, "::")

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
    for i in node.inherits():
        inherits_scopedName = map(tyutil.mapID, i.scopedName())
        inherits_name = string.join(inherits_scopedName, "::")
        inherits_objref_scopedName =  tyutil.scope(inherits_scopedName) + \
                                     ["_objref_" + tyutil.name(inherits_scopedName)]
        inherits_objref_name = string.join(inherits_objref_scopedName, "::")
        inherits_str = inherits_str + inherits_objref_name + "(mdri, p, id, lid),"
        
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
  
  return 0;
}
""", name = name, fq_objref_name = objref_name, objref_name = objref_name,
               inherits_str = inherits_str)

    # deal with callables
    callables = node.callables()
    attributes = filter(lambda x:isinstance(x, idlast.Attribute), callables)
    operations = filter(lambda x:isinstance(x, idlast.Operation), callables)
    scopedName = node.scopedName()
    

    for operation in operations:
        operationName = operation.identifier()
        
        seed = scopedName + [operation.identifier()]
        mangled_signature = mangler.produce_operation_signature(operation)
        
        mangler.generate_descriptors(operation, seed)
        descriptor = mangler.operation_descriptor_name(operation)

        parameters = operation.parameters()
        parameters_in  = filter(lambda x:x.is_in(),  parameters)
        parameters_out = filter(lambda x:x.is_out(), parameters)
        parameters_ID  = map(lambda x:tyutil.mapID(x.identifier()), parameters)
        
        parameter_argmapping = []
        parameter_vargmapping = []

        returnType = operation.returnType()
        result_type = tyutil.operationArgumentType(returnType,
                                                   environment)[0]
        has_return_value = not(tyutil.isVoid(returnType))

        # compute the argument mapping for the operation parameters
        for parameter in parameters:
            paramType = parameter.paramType()
            deref_paramType = tyutil.deref(paramType)
            
            optypes = tyutil.operationArgumentType(paramType, environment)
            if parameter.is_in() and parameter.is_out():
                parameter_argmapping.append(optypes[3])
            elif parameter.is_in():
                parameter_argmapping.append(optypes[1])
            elif parameter.is_out():
                parameter_argmapping.append(optypes[2])
            # some day I'll figure out a better way of handling this
            if tyutil.isString(deref_paramType) and parameter.is_out():
                parameter_vargmapping.append("char*&")
            elif tyutil.isObjRef(deref_paramType) and parameter.is_out():
                parameter_vargmapping.append(environment.principalID(paramType)+\
                                             "_ptr&")
                                             
            else:
                optypes = tyutil.operationArgumentType(paramType, environment, 1)
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
                                    "arg_" + str(n), "giop_client")
                n = n + 1
            
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
                item_types.append(returnType)
                item_names.append("pd_result")
                item_direction.append(1)
                item_via_temporary.append(0)
            
            zipped_up = util.zip(item_types,
                                 util.zip(item_names,
                                          util.zip(item_direction,
                                                   item_via_temporary)))

            n = -1
            
            for (item_type, (item_name, (item_direction, via_tmp))) in zipped_up:
                deref_item_type = tyutil.deref(item_type)
                item_type_name = environment.principalID(item_type)
                n = n + 1
                assign_to = "tmp_" + str(n)
                # this needs uniform(-ing?, -alising?)
                if via_tmp == 0: assign_to = "pd_result"

                # SWITCH(deref_item_type)
                #  CASE(string)
                if tyutil.isString(deref_item_type):
                    
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
                #  DEFAULT
                elif item_direction == 1 or item_direction == 2:
                    skutil.unmarshall(end, environment, item_type,
                                      None, item_name, 0, "giop_client")
            
                    
            
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
  @impl_name@* impl = (@impl_name@*) svnt->_ptrToInterface(@name@::_PD_repoId);
  @result@impl->@operation_name@(@operation_arguments@);
}
""",
                   local_call_descriptor = local_call_descriptor,
                   call_descriptor = descriptor,
                   impl_name = impl_name,
                   name = name,
                   operation_name = operationName,
                   operation_arguments = string.join(impl_args, ", "),
                   result = result_string)

        # objref::operation
        objref_args = util.zip(parameter_argmapping, parameters_ID)
        objref_args = map(lambda (x,y): x + " " + y, objref_args)
        call_desc_args = [local_call_descriptor, "\"" + operationName + "\"",
                          str(len(operationName) + 1), "0"] +\
                          parameters_ID

        return_string = ""
        if has_return_value:
            return_string = "return _call_desc.result();"
            
        stream.out("""\
@result_type@ @objref_name@::@operation_name@(@arguments@)
{
  @call_descriptor@ _call_desc(@call_desc_args@);
  
  _invoke(_call_desc);
  @return_string@
}


""",
                   result_type = result_type,
                   objref_name = objref_name,
                   operation_name = operationName,
                   arguments = string.join(objref_args, ", "),
                   call_descriptor = descriptor,
                   call_desc_args = string.join(call_desc_args, ", "),
                   return_string = return_string)
                   
        
    # attributes
    for attribute in attributes:
        seed = scopedName + [attribute.identifiers()[0]]
        read_signature = mangler.produce_read_attribute_signature(attribute)
        write_signature = mangler.produce_write_attribute_signature(attribute)

        try:
            # see if we already have proxies generated
            mangler.attribute_read_descriptor_name(attribute)
            need_proxies = 0
        except KeyError:
            mangler.generate_descriptors(attribute, seed)
            need_proxies = 1
        
        read = mangler.attribute_read_descriptor_name(attribute)
        write = mangler.attribute_write_descriptor_name(attribute)
        attrType = attribute.attrType()
        deref_attrType = tyutil.deref(attrType)
        attrType_name = environment.principalID(attrType)

        attrTypes = tyutil.operationArgumentType(deref_attrType, environment)
        return_type = attrTypes[0]
        in_type = attrTypes[1]

        size = skutil.sizeCalculation(environment, attrType, None ,
                                      "msgsize",
                                      "arg_0", 1)
        marshall_stream = util.StringStream()
        skutil.marshall(marshall_stream, environment, attrType, None,
                        "arg_0", "giop_client")

        if tyutil.isString(deref_attrType):
            unmarshalReturned = skutil.unmarshal_string_via_temporary("pd_result",
                                                                      "giop_client")
        elif tyutil.isObjRef(deref_attrType):
            unmarshalReturned = "\
pd_result = " + attrType_name + "_Helper::unmarshalObjRef(giop_client);"
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
  @impl_name@* impl = (@impl_name@*) svnt->_ptrToInterface(@name@::_PD_repoId);
  tcd->pd_result = impl->@attrib_name@();
}


@return_type@ @objref_name@::@attrib_name@()
{
  @read_descriptor@ _call_desc(@local_call_descriptor@, \"@get_attrib_name@\", @len@, 0);
  
  _invoke(_call_desc);
  return _call_desc.result();
}
""",
                       local_call_descriptor = local_call_descriptor,
                       read_descriptor = read,
                       impl_name = impl_name,
                       objref_name = objref_name,
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
  @impl_name@* impl = (@impl_name@*) svnt->_ptrToInterface(@name@::_PD_repoId);
  impl->@attrib_name@(tcd->arg_0);
}


void @objref_name@::@attrib_name@(@in_type@ arg_0)
{
  @write_descriptor@ _call_desc(@local_call_descriptor@, \"@set_attrib_name@\", @len@, 0, arg_0);
  
  _invoke(_call_desc);
}
""",
                           local_call_descriptor = local_call_descriptor,
                           write_descriptor = write,
                           impl_name = impl_name,
                           objref_name = objref_name,
                           name = name,
                           attrib_name = attrib_name,
                           set_attrib_name = set_attrib_name,
                           len = str(len(set_attrib_name) + 1),
                           in_type = in_type)


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
  return new @objref_name@(mdri, p, id, lid);
}


CORBA::Boolean
@pof_name@::is_a(const char* id) const
{
  if( !strcmp(id, @name@::_PD_repoId) )
    return 1;
    """,
               pof_name = pof_name,
               objref_name = objref_name,
               name = name,
               uname = u_name)
    for i in node.inherits():
        ancestor = string.join(map(tyutil.mapID, i.scopedName()), "::")
        stream.out("""\
  if( !strcmp(id, @inherited@::_PD_repoID) )
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
@impl_name@::~_impl_@name@() {}


CORBA::Boolean
@impl_name@::_dispatch(GIOP_S& giop_s)
{""", impl_name = impl_name, name = name)
    stream.inc_indent()
    for callable in node.callables():
        if isinstance(callable, idlast.Operation):
            identifiers = [callable.identifier()]
        else:
            identifiers = callable.identifiers()

        # separate case for each callable thing
        for id in identifiers:
            id_name = tyutil.mapID(id)
            if isinstance(callable, idlast.Operation):

                skutil.operation_dispatch(callable, environment, stream)

            elif isinstance(callable, idlast.Attribute):
                
                skutil.attribute_read_dispatch(callable, environment,
                                               id_name, stream)

                if not(callable.readonly()):

                    skutil.attribute_write_dispatch(callable, environment,
                                                    id_name, stream)
    stream.dec_indent()
    stream.out("""\
    return 0;
}""")
    
    stream.out("""\
    
void*
@impl_name@::_ptrToInterface(const char* id)
{
  if( !strcmp(id, CORBA::Object::_PD_repoId) )
    return (void*) 1;
  if( !strcmp(id, @name@::_PD_repoId) )
    return (@impl_name@*) this;

  return 0;
}


const char*
@impl_name@::_mostDerivedRepoId()
{
  return @name@::_PD_repoId;
}
""",
               impl_name = impl_name,
               name = name)
    

#    leave()

def visitTypedef(node):
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
            member_name = tyutil.name(d.scopedName())

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
    pass
def visitDeclarator(node):
    pass
def visitException(node):
    pass

