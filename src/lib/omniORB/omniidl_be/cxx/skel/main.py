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
# Revision 1.16  2000/01/10 18:42:22  djs
# Removed redundant code, tidied up.
#
# Revision 1.15  2000/01/10 15:39:48  djs
# Better name and scope handling.
#
# Revision 1.14  2000/01/07 20:31:33  djs
# Regression tests in CVSROOT/testsuite now pass for
#   * no backend arguments
#   * tie templates
#   * flattened tie templates
#   * TypeCode and Any generation
#
# Revision 1.13  1999/12/25 21:47:18  djs
# Better TypeCode support
#
# Revision 1.12  1999/12/24 18:18:32  djs
# #include bug fixed
#
# Revision 1.11  1999/12/16 16:11:21  djs
# Now uses transitive closure of inherits relation where appropriate
#
# Revision 1.10  1999/12/14 11:53:22  djs
# Support for CORBA::TypeCode and CORBA::Any
# Exception member bugfix
#
# Revision 1.9  1999/12/09 20:40:57  djs
# Bugfixes and integration with dynskel/ code
#
# Revision 1.8  1999/11/29 19:27:05  djs
# Code tidied and moved around. Some redundant code eliminated.
#
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

from omniidl.be.cxx import tyutil, util, name, env, config, skutil

from omniidl.be.cxx.skel import mangler, dispatch, proxy

import main
self = main

# ------------------------------------
# environment handling functions

#self.__environment = name.Environment()
self.__globalScope = name.globalScope()
self.__insideInterface = 0
self.__insideModule = 0


def __init__(stream):
    self.stream = stream
    return self

# ------------------------------------
# Control arrives here

def visitAST(node):
    for n in node.declarations():
        # check what happens with reopened modules
        if n.mainFile():
            n.accept(self)

def visitModule(node):
    insideModule = self.__insideModule
    self.__insideModule = 1
    for n in node.definitions():
        n.accept(self)

    self.__insideModule = insideModule


def visitInterface(node):
    name = node.identifier()
    cxx_name = tyutil.mapID(name)
    
    outer_environment = env.lookup(node)
    environment = outer_environment.enterScope(name)

    insideInterface = self.__insideInterface
    self.__insideInterface = 1

    # produce skeletons for types declared here
    for n in node.declarations():
        n.accept(self)

    self.__insideInterface = insideInterface

    scopedName = map(tyutil.mapID, node.scopedName())
    fqname = string.join(scopedName, "::")
    name = environment.nameToString(scopedName)

    # we need to generate several useful classes for object
    # references
    objref_scopedName = tyutil.scope(scopedName) + \
                        ["_objref_" + tyutil.name(scopedName)]
    
    objref_fqname = string.join(map(tyutil.mapID, objref_scopedName), "::")
    objref_name = environment.nameToString(environment.relName(objref_scopedName))

    impl_scopedName = tyutil.scope(scopedName) + \
                      ["_impl_" + tyutil.name(scopedName)]
    impl_fqname = string.join(map(tyutil.mapID, impl_scopedName), "::")
    impl_name = environment.nameToString(environment.relName(impl_scopedName))


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
    

    # output the repository ID
    stream.out("""\
const char* @name@::_PD_repoId = \"@repoID@\";""",
               name = name, repoID = tyutil.mapRepoID(node.repoId()))

    # gather information for possible interface inheritance
    # (needs to use the transitive closure of inheritance)
    all_inherits = tyutil.allInherits(node)
    inherits_str = ""
    inherited_repoIDs = ""
    for i in all_inherits:
        inherits_scopedName = map(tyutil.mapID, i.scopedName())
        inherits_name = string.join(inherits_scopedName, "::")

        inherited_repoIDs = inherited_repoIDs + "\
        if( !strcmp(id, " + inherits_name + "::_PD_repoId) )\n\
          return (" + inherits_name + "_ptr) this;\n"

    for i in node.inherits():
        inherits_scopedName = map(tyutil.mapID, i.scopedName())        
        inherits_objref_scopedName =  tyutil.scope(inherits_scopedName) + \
                                     ["_objref_" + tyutil.name(inherits_scopedName)]
        inherits_objref_name = environment.nameToString(environment.relName(
            inherits_objref_scopedName))
        inherits_str = inherits_str + inherits_objref_name + "(mdri, p, id, lid),\n"

    # generate the _objref_ methods
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

    # deal with callables (attributes and interfaces)
    callables = node.callables()
    attributes = filter(lambda x:isinstance(x, idlast.Attribute), callables)
    operations = filter(lambda x:isinstance(x, idlast.Operation), callables)
    scopedName = node.scopedName()

    # every callable has an associated call descriptor proxy.
    Proxy = proxy.__init__(environment, stream)

    # ------------------------------------
    # Operations
    for operation in operations:
        operationName = operation.identifier()
        cxx_operationName = tyutil.mapID(operationName)
        
        seed = scopedName + [operation.identifier()]

        # try the all new proxy code!
        Proxy.operation(operation, seed)
        descriptor = mangler.operation_descriptor_name(operation)

        parameters = operation.parameters()
        parameters_ID  = map(lambda x:tyutil.mapID(x.identifier()), parameters)
        
        parameter_argmapping = []

        returnType = operation.returnType()
        result_type = tyutil.operationArgumentType(returnType,
                                                   self.__globalScope,
                                                   #environment,
                                                   0,1)[0]
        has_return_value = not(tyutil.isVoid(returnType))

        # compute the argument mapping for the operation parameters
        for parameter in parameters:
            paramType = parameter.paramType()
            deref_paramType = tyutil.deref(paramType)
            param_dims = tyutil.typeDims(paramType)
            is_array = param_dims != []
            paramType_name = self.__globalScope.principalID(paramType)
            
            optypes = tyutil.operationArgumentType(paramType,
                                                   outer_environment)
            # optypes[0] is return [1] is in [2] is out [3] is inout
            parameter_argmapping.append(optypes[parameter.direction() + 1])

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
  @result@impl->@cxx_operation_name@(@operation_arguments@);
}
""",
                   local_call_descriptor = local_call_descriptor,
                   call_descriptor = descriptor,
                   impl_name = impl_name,
                   impl_fqname = impl_fqname,
                   name = name,
                   cxx_operation_name = cxx_operationName,
                   operation_arguments = string.join(impl_args, ", "),
                   result = result_string)

        # objref::operation name
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
                   operation_name = cxx_operationName,
                   arguments = string.join(objref_args, ", "),
                   call_descriptor = descriptor,
                   call_desc_args = string.join(call_desc_args, ", "),
                   return_string = return_string)
                   
    # ------------------------------------
    # Attributes
    for attribute in attributes:
        seed = scopedName + [attribute.identifiers()[0]]

        Proxy.attribute(attribute, seed)

        read = mangler.attribute_read_descriptor_name(attribute)
        write = mangler.attribute_write_descriptor_name(attribute)
        attrType = attribute.attrType()
        attr_dims = tyutil.typeDims(attrType)
        is_array = attr_dims != []
        
        deref_attrType = tyutil.deref(attrType)
        attrType_name = environment.principalID(attrType, 1)

        # we need the type with and without its full scope
        attrTypes = tyutil.operationArgumentType(attrType, self.__globalScope, 0,
                                                 fully_scope = 0)
        scoped_attrTypes = tyutil.operationArgumentType(attrType, outer_environment, 0,
                                                 fully_scope = 0)
        return_type = attrTypes[0]

        # FIXME: change to the old compiler might make this always use
        # fully scoped names
        if is_array:
            scoped_in_type = attrTypes[1]+"_slice*"
        else:
            scoped_in_type = attrTypes[1]


        for id in attribute.identifiers():
            attrib_name = id
            cxx_attrib_name = tyutil.mapID(attrib_name)

            get_attrib_name = "_get_" + attrib_name
            local_call_descriptor = mangler.generate_unique_name(mangler.LCALL_DESC_PREFIX)

            stream.out("""\
// Local call call-back function.
static void
@local_call_descriptor@(omniCallDescriptor* cd, omniServant* svnt)
{
  @read_descriptor@* tcd = (@read_descriptor@*) cd;
  @impl_fqname@* impl = (@impl_fqname@*) svnt->_ptrToInterface(@name@::_PD_repoId);
  tcd->pd_result = impl->@cxx_attrib_name@();
}


@return_type@ @objref_fqname@::@cxx_attrib_name@()
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
                       cxx_attrib_name = cxx_attrib_name,
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
  impl->@cxx_attrib_name@(tcd->arg_0);
}


void @objref_fqname@::@cxx_attrib_name@(@in_type@ arg_0)
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
                           cxx_attrib_name = cxx_attrib_name,
                           attrib_name = attrib_name,
                           set_attrib_name = set_attrib_name,
                           len = str(len(set_attrib_name) + 1),
                           in_type = scoped_in_type)


    # _pof_ class
    pof_scopedName = map(tyutil.mapID, tyutil.scope(scopedName)) + \
                      ["_pof_" + tyutil.mapID(tyutil.name(scopedName))]
    pof_name = string.join(pof_scopedName, "::")
    u_name = tyutil.mapID(tyutil.name(scopedName))

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
    # again this must deal with _all_ inheritance
    for i in all_inherits:
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

    # _impl_ class (contains the callable dispatch code)

    stream.out("""\
@impl_fqname@::~_impl_@uname@() {}


CORBA::Boolean
@impl_fqname@::_dispatch(GIOP_S& giop_s)
{""", impl_fqname = impl_fqname, uname = u_name)
    stream.inc_indent()

    dispatcher = dispatch.__init__(environment, stream)
    
    for callable in node.callables():
        # This isn't quite as neat as it could be
        if isinstance(callable, idlast.Operation):
            identifiers = [callable.identifier()]
        else:
            identifiers = callable.identifiers()

        # separate case for each callable thing
        for id in identifiers:
            id_name = tyutil.mapID(id)
            if isinstance(callable, idlast.Operation):
                dispatcher.operation(callable)

            elif isinstance(callable, idlast.Attribute):
                dispatcher.attribute_read(callable, id)

                if not(callable.readonly()):
                    dispatcher.attribute_write(callable, id)

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
    for i in all_inherits:
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
    

    #leave()

def visitTypedef(node):
    environment = env.lookup(node)
    is_global_scope = not(self.__insideModule or self.__insideInterface)

    aliasType = node.aliasType()
    alias_dims = tyutil.typeDims(aliasType)

    fq_aliased = environment.principalID(aliasType, 1)

    for d in node.declarators():
        # record in the environment
        environment.add(tyutil.name(d.scopedName()))
        
        decl_dims = d.sizes()
        decl_dims_str = tyutil.dimsToString(decl_dims)
        decl_first_dim_str = ""
        if decl_dims != []:
            decl_first_dim_str = tyutil.dimsToString([decl_dims[0]])
        
        full_dims = decl_dims + alias_dims
        is_array = full_dims != []
        is_array_declarator = decl_dims != []
        
        fq_derived = idlutil.ccolonName(map(tyutil.mapID, d.scopedName()))

        if is_global_scope and is_array_declarator:
            
            stream.out("""\
@fq_derived@_slice* @fq_derived@_alloc() {
  return new @fq_derived@_slice@decl_first_dim_str@;
}

@fq_derived@_slice* @fq_derived@_dup(const @fq_derived@_slice* _s)
{
  if (!_s) return 0;
  @fq_derived@_slice* _data = @fq_derived@_alloc();
  if (_data) {
  """, fq_derived = fq_derived, decl_dims_str = decl_dims_str,
                       decl_first_dim_str = decl_first_dim_str)
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

        
        

        elif is_global_scope and is_array:
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
    return

def visitMember(node):
    memberType = node.memberType()
    if node.constrType():
        # if the type was declared here, it must be an instance
        # of idltype.Declared!
        assert isinstance(memberType, idltype.Declared)
        memberType.decl().accept(self)
        
def visitStruct(node):

    outer_environment = env.lookup(node)
    
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

            skutil.marshall_struct_union(marshall, outer_environment,
                                         memberType, d, member_name)
            skutil.unmarshall_struct_union(Mem_unmarshall, outer_environment,
                                           memberType, d, member_name, 0)
            skutil.unmarshall_struct_union(Net_unmarshall, outer_environment,
                                           memberType, d, member_name, 1)
            # computation of aligned size
            size = skutil.sizeCalculation(outer_environment, memberType, d,
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
    environment = env.lookup(node)
    
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
                # FIXME: stupid special case. An explicit discriminator
                # value of \0 -> 0000 whereas an implicit one (valueString)
                # \0 -> '\000'
                discrim_value = tyutil.valueString(switchType, value, environment)
                if tyutil.isChar(switchType) and value == '\0':
                    discrim_value = "0000"
                    
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
    # (This is probably due to a string-inconsistency with the old compiler
    # and can be sorted out later)
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
                   # FIXME: stupid special case. An explicit discriminator
                   # value of \0 -> 0000 whereas an implicit one (valueString)
                   # \0 -> '\000'
                   discrim_value = tyutil.valueString(switchType, value, environment)
                   if tyutil.isChar(switchType) and value == '\0':
                       discrim_value = "0000"

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
                    # FIXME: stupid special case. An explicit discriminator
                    # value of \0 -> 0000 whereas an implicit one (valueString)
                    # \0 -> '\000'
                    discrim_value = tyutil.valueString(switchType, value, environment)
                    if tyutil.isChar(switchType) and value == '\0':
                        discrim_value = "0000"

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
    return

def visitConst(node):
    environment = env.lookup(node)
    
    constType = node.constType()
    deref_constType = tyutil.deref(constType)
    
    if tyutil.isString(deref_constType):
        type_string = "char *"
    else:
        type_string = environment.principalID(deref_constType)

    scopedName = node.scopedName()
    scopedName = map(tyutil.mapID, scopedName)
    name = idlutil.ccolonName(scopedName)
    value = tyutil.valueString(deref_constType, node.value(), environment)
    
    init_in_def = tyutil.const_init_in_def(deref_constType)

    
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
    name = tyutil.name(node.scopedName())
    cxx_name = tyutil.mapID(name)
    
    outer_environment = env.lookup(node)
    environment = outer_environment.enterScope(name)
    
    scoped_name = environment.nameToString(node.scopedName())
    name = tyutil.mapID(tyutil.name(node.scopedName()))
    repoID = tyutil.mapRepoID(node.repoId())

    # build the default ctor, copy ctor, assignment operator
    copy_ctor_body = util.StringStream()
    default_ctor_body = util.StringStream()
    default_ctor_args = []
    assign_op_body = util.StringStream()
    has_default_ctor = 0

    for m in node.members():
        has_default_ctor = 1
        memberType = m.memberType()
        if m.constrType():
            memberType.decl().accept(self)
        deref_memberType = tyutil.deref(memberType)
        memberType_name = environment.principalID(memberType)
        memberType_fqname = self.__globalScope.principalID(memberType)
        type_dims = tyutil.typeDims(memberType)
        for d in m.declarators():
            decl_name = tyutil.mapID(tyutil.name(d.scopedName()))
            
            decl_dims = d.sizes()
            full_dims = decl_dims + type_dims
            is_array = full_dims != []
            is_array_declarator = decl_dims != []

            if is_array_declarator:
                # we use the internal typedef'ed type if the member is an array
                # declarator
                memberType_name_arg = "const _0RL_" + decl_name
            else:
                # we normally use the utility function for this purpose
                memberType_name_arg = tyutil.makeConstructorArgumentType(memberType,
                                                                         environment,
                                                                         d)
                
            index = ""

            if is_array:
                index = util.block_begin_loop(copy_ctor_body, full_dims,
                                              iter_type = "unsigned int")
                index = util.block_begin_loop(default_ctor_body, full_dims,
                                              iter_type = "unsigned int")
                index = util.block_begin_loop(assign_op_body, full_dims,
                                              iter_type = "unsigned int")
                
            copy_ctor_body.out("""\
@member_name@@index@ = _s.@member_name@@index@;""", member_name = decl_name,
                               index = index)

            if tyutil.isObjRef(deref_memberType) and not(is_array):
                # these are special resources which need to be explicitly
                # duplicated (but not if an array?)
                default_ctor_body.out("""\
@member_type_name@_Helper::duplicate(_@member_name@@index@);""",
                                      member_type_name = memberType_fqname,
                                      member_name = decl_name,
                                      index = index)
            
            default_ctor_args.append(memberType_name_arg + " _" + decl_name)
            default_ctor_body.out("""\
@member_name@@index@ = _@member_name@@index@;""", member_name = decl_name,
                                  index = index)

            assign_op_body.out("""\
@member_name@@index@ = _s.@member_name@@index@;""", member_name = decl_name,
                               index = index)
            
            if is_array:
                util.block_end_loop(copy_ctor_body, full_dims)
                util.block_end_loop(default_ctor_body, full_dims)
                util.block_end_loop(assign_op_body, full_dims)
        
          
        
    
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
        deref_memberType = tyutil.deref(memberType)
        for d in m.declarators():
            decl_name = tyutil.mapID(tyutil.name(d.scopedName()))
            if tyutil.isString(memberType):
                tmp = skutil.unmarshal_string_via_temporary(decl_name, "_n")
                mem_unmarshal.out(tmp)
                net_unmarshal.out(tmp)
            # TypeCodes seem to be other exceptions
            elif tyutil.isTypeCode(deref_memberType):
                skutil.unmarshall_struct_union(mem_unmarshal, environment,
                                               memberType, d, decl_name, 0,
                                               "_n")
                skutil.unmarshall_struct_union(net_unmarshal, environment,
                                               memberType, d, decl_name, 0,
                                               "_n")
                
            else:
                skutil.unmarshall(mem_unmarshal, environment,
                                  memberType, d, decl_name, 0, "_n",
                                  string_via_member = 1)
                skutil.unmarshall(net_unmarshal, environment,
                                  memberType, d, decl_name, 1, "_n",
                                  string_via_member = 1)

            if tyutil.isTypeCode(deref_memberType):
                skutil.marshall_struct_union(mem_marshal, environment,
                                             memberType, d, decl_name, "_n")
                skutil.marshall_struct_union(net_marshal, environment,
                                             memberType, d, decl_name, "_n")
            else:
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


    return
            
            
                                           
