# -*- python -*-
#                           Package   : omniidl
# defs.py                   Created on: 1999/11/2
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
#   Produce the main header definitions for the C++ backend

# $Id$
# $Log$
# Revision 1.22  2000/01/11 11:34:27  djs
# Added support for fragment generation (-F) mode
#
# Revision 1.21  2000/01/10 16:13:13  djs
# Removed a chunk of redundant code.
#
# Revision 1.20  2000/01/10 15:38:55  djs
# Better name and scope handling.
#
# Revision 1.19  2000/01/07 20:31:27  djs
# Regression tests in CVSROOT/testsuite now pass for
#   * no backend arguments
#   * tie templates
#   * flattened tie templates
#   * TypeCode and Any generation
#
# Revision 1.18  1999/12/26 16:43:06  djs
# Enum scope fix
# Handling default case of char discriminated switch fixed
#
# Revision 1.17  1999/12/25 21:47:18  djs
# Better TypeCode support
#
# Revision 1.16  1999/12/24 18:14:29  djs
# Fixed handling of #include'd .idl files
#
# Revision 1.15  1999/12/16 16:10:46  djs
# TypeCode fixes
#
# Revision 1.14  1999/12/15 12:13:16  djs
# Multiple forward declarations of interface fix
#
# Revision 1.13  1999/12/14 17:38:22  djs
# Fixed anonymous sequences of sequences bug
#
# Revision 1.12  1999/12/14 11:52:30  djs
# Support for CORBA::TypeCode and CORBA::Any
#
# Revision 1.11  1999/12/01 17:03:15  djs
# Added support for Typecodes and Anys
#
# Revision 1.10  1999/11/23 18:46:34  djs
# Constant fixes
# Moved exception constructor argument generator into a more central place
#
# Revision 1.9  1999/11/19 20:08:09  djs
# Removed references to a non-existant utility function
#
# Revision 1.8  1999/11/15 19:12:45  djs
# Tidied up sequence template handling
# Moved useful functions into separate utility module
#
# Revision 1.7  1999/11/12 17:18:39  djs
# Lots of header generation bugfixes
#
# Revision 1.6  1999/11/10 20:19:43  djs
# Array struct element fix
# Union sequence element fix
#
# Revision 1.5  1999/11/08 19:29:03  djs
# Rewrite of sequence template code
# Fixed lots of typedef problems
#
# Revision 1.4  1999/11/04 19:05:08  djs
# Finished moving code from tmp_omniidl. Regression tests ok.
#
# Revision 1.3  1999/11/03 18:18:12  djs
# Struct and Exception fixes
#
# Revision 1.2  1999/11/03 17:35:11  djs
# Brought more of the old tmp_omniidl code into the new tree
#
# Revision 1.1  1999/11/03 11:09:56  djs
# General module renaming
#

"""Produce the main header definitions"""
# similar to o2be_root::produce_hdr_defs in the old C++ BE

import string

from omniidl import idlast, idltype, idlutil

from omniidl.be.cxx import tyutil, util, name, env, config

import defs

self = defs

# State information (used to be passed as arguments during recursion)
self.__insideInterface = 0
self.__insideModule = 0
self.__insideClass = 0

self.__globalScope = name.globalScope()


def __init__(stream):
    defs.stream = stream
    return defs

#
# Control arrives here
#
def visitAST(node):
    for n in node.declarations():
        n.accept(self)

def visitModule(node):
    # This may be incorrect wrt reopened modules in multiple
    # files
    if not(node.mainFile()):
        return
    
    name = tyutil.mapID(node.identifier())
    
    if not(config.FragmentFlag()):
        stream.out("""\
_CORBA_MODULE @name@

_CORBA_MODULE_BEG
""", name = name)
        stream.inc_indent()

    insideModule = self.__insideModule
    self.__insideModule = 1

    for n in node.definitions():
        n.accept(self)

    self.__insideModule = insideModule
    if not(config.FragmentFlag()):
        stream.dec_indent()
        stream.out("""\
_CORBA_MODULE_END
""")
        

def visitInterface(node):
    if not(node.mainFile()):
        return

    name = node.identifier()
    cxx_name = tyutil.mapID(name)

    outer_environment = env.lookup(node)
    environment = outer_environment.enterScope(name)
    scope = environment.scope()

    insideInterface = self.__insideInterface
    self.__insideInterface = 1
    insideClass = self.__insideClass
    self.__insideClass = 1
    
    # the ifndef guard name contains scope information
    guard = tyutil.guardName(scope)

    stream.out("""\
#ifndef __@guard@__
#define __@guard@__

class @name@;
class _objref_@name@;
class _impl_@name@;
typedef _objref_@name@* @name@_ptr;
typedef @name@_ptr @name@Ref;

class @name@_Helper {
public:
  typedef @name@_ptr _ptr_type;

  static _ptr_type _nil();
  static _CORBA_Boolean is_nil(_ptr_type);
  static void release(_ptr_type);
  static void duplicate(_ptr_type);
  static size_t NP_alignedSize(_ptr_type, size_t);
  static void marshalObjRef(_ptr_type, NetBufferedStream&);
  static _ptr_type unmarshalObjRef(NetBufferedStream&);
  static void marshalObjRef(_ptr_type, MemBufferedStream&);
  static _ptr_type unmarshalObjRef(MemBufferedStream&);
};

typedef _CORBA_ObjRef_Var<_objref_@name@, @name@_Helper> @name@_var;
typedef _CORBA_ObjRef_OUT_arg<_objref_@name@,@name@_Helper > @name@_out;

#endif


class @name@ {
public:
  // Declarations for this interface type.
  typedef @name@_ptr _ptr_type;
  typedef @name@_var _var_type;

  static _ptr_type _duplicate(_ptr_type);
  static _ptr_type _narrow(CORBA::Object_ptr);
  static _ptr_type _nil();

  static inline size_t _alignedSize(_ptr_type, size_t);
  static inline void _marshalObjRef(_ptr_type, NetBufferedStream&);
  static inline void _marshalObjRef(_ptr_type, MemBufferedStream&);

  static inline _ptr_type _unmarshalObjRef(NetBufferedStream& s) {
    CORBA::Object_ptr obj = CORBA::UnMarshalObjRef(_PD_repoId, s);
    _ptr_type result = _narrow(obj);
    CORBA::release(obj);
    return result;
  }

  static inline _ptr_type _unmarshalObjRef(MemBufferedStream& s) {
    CORBA::Object_ptr obj = CORBA::UnMarshalObjRef(_PD_repoId, s);
    _ptr_type result = _narrow(obj);
    CORBA::release(obj);
    return result;
  }

  static _core_attr const char* _PD_repoId;

  // Other IDL defined within this scope.
  """,
               name = cxx_name,
               guard = guard)
    # output code for other declarations within this scope
    for n in node.declarations():
        n.accept(self)
        
    # build methods corresponding to attributes, operations etc
    attributes = []
    operations = []
    virtual_operations = []
        
    for c in node.callables():
        if isinstance(c, idlast.Attribute):
            attrType = c.attrType()
            derefAttrType = tyutil.deref(attrType)
            
            returnType = tyutil.operationArgumentType(attrType, outer_environment)[0]
            inType = tyutil.operationArgumentType(attrType, outer_environment)[1]
            
            for i in c.identifiers():
                attribname = tyutil.mapID(i)
                attributes.append(returnType + " " + attribname + "()")
                if not(c.readonly()):
                    attributes.append("void " + attribname + "(" \
                                      + inType + ")")
        elif isinstance(c, idlast.Operation):
            def argumentTypeToString(arg, virtual = 0, envir = outer_environment):
                return tyutil.operationArgumentType(arg, envir, virtual)

            params = []
            virtual_params = []
            for p in c.parameters():
                paramType = p.paramType()
                types = argumentTypeToString(paramType)
                virtual_types = argumentTypeToString(paramType, 1)
                tuple = ("", "")
                if   p.is_in() and p.is_out():
                    tuple = (types[3], virtual_types[3])
                elif p.is_in():
                    tuple = (types[1], virtual_types[1])
                elif p.is_out():
                    tuple = (types[2], virtual_types[2])
                else:
                    assert 0
                argname = tyutil.mapID(p.identifier())
                params.append(tuple[0] + " " + argname)
                virtual_params.append(tuple[1] + " " + argname)

            return_type = argumentTypeToString(c.returnType())[0]
            opname = tyutil.mapID(c.identifier())
            arguments = string.join(params, ", ")
            virtual_arguments = string.join(virtual_params, ", ")
            operations.append(return_type + " " + opname + \
                              "(" + arguments + ")")
            virtual_operations.append(return_type + " " + opname + \
                                      "(" + virtual_arguments + ")")
        else:
            raise "No code for interface member: " + repr(c)

    attributes_str = string.join(map(lambda x: x + ";\n", attributes ),"")
    operations_str = string.join(map(lambda x: x + ";\n", operations ),"")
        
    virtual_attributes_str = string.join(
        map( lambda x: "virtual " + x + " = 0;\n", attributes ), "")
    virtual_operations_str = string.join(
        map( lambda x: "virtual " + x + " = 0;\n", virtual_operations ), "")

    # deal with inheritance
    objref_inherits = []
    impl_inherits = []
    for i in node.inherits():
        scope = tyutil.scope(i.scopedName())
        id = tyutil.mapID(i.identifier())
        
        objref_scoped_name = scope + ["_objref_" + id]
        impl_scoped_name   = scope + ["_impl_" + id]

        objref_rel_name = environment.relName(objref_scoped_name)
        impl_rel_name   = environment.relName(impl_scoped_name)

        objref_string = environment.nameToString(objref_rel_name)
        impl_string   = environment.nameToString(impl_rel_name)
        
        objref_inherits.append("public virtual " + objref_string)
        impl_inherits.append("public virtual " + impl_string)
    # if already inheriting, the base classes will be present
    # (transitivity of the inherits-from relation)
    if node.inherits() == []:
        objref_inherits = [ "public virtual CORBA::Object, " + \
                            "public virtual omniObjRef" ]
        impl_inherits   = [ "public virtual omniServant" ]
            
    objref_inherits = string.join(objref_inherits, ",\n")
    impl_inherits = string.join(impl_inherits, ", \n")
        
    stream.out("""\
    
};


class _objref_@name@ :
  @inherits@
{
public:
  @operations@
  @attributes@

  inline _objref_@name@() { _PR_setobj(0); }  // nil
  _objref_@name@(const char*, IOP::TaggedProfileList*, omniIdentity*, omniLocalIdentity*);

protected:
  virtual ~_objref_@name@();

private:
  virtual void* _ptrToObjRef(const char*);

  _objref_@name@(const _objref_@name@&);
  _objref_@name@& operator = (const _objref_@name@&);
  // not implemented
};


class _pof_@name@ : public proxyObjectFactory {
public:
  inline _pof_@name@() : proxyObjectFactory(@name@::_PD_repoId) {}
  virtual ~_pof_@name@();

  virtual omniObjRef* newObjRef(const char*, IOP::TaggedProfileList*,
                                omniIdentity*, omniLocalIdentity*);
  virtual _CORBA_Boolean is_a(const char*) const;
};

""",
               inherits = objref_inherits,
               name = cxx_name,
               operations = operations_str,
               attributes = attributes_str)
    stream.out("""\
class _impl_@name@ :
  @inherits@
{
public:
  virtual ~_impl_@name@();

  @virtual_operations@
  @virtual_attributes@

protected:
  virtual _CORBA_Boolean _dispatch(GIOP_S&);

private:
  virtual void* _ptrToInterface(const char*);
  virtual const char* _mostDerivedRepoId();
};

""",
               inherits = impl_inherits,
               name = cxx_name,
               virtual_operations = virtual_operations_str,
               virtual_attributes = virtual_attributes_str)

    self.__insideInterface = insideInterface
    self.__insideClass = insideClass

    # Typecode and Any
    if config.TypecodeFlag():
        qualifier = tyutil.const_qualifier(self.__insideModule,
                                               self.__insideClass)
        stream.out("""\
@qualifier@ _dyn_attr const CORBA::TypeCode_ptr _tc_@name@;
""", qualifier = qualifier, name = cxx_name)
        
    return
    

def visitForward(node):
    if not(node.mainFile()):
        return
    
    # Note it's legal to have multiple forward declarations
    # of the same name. ignore the duplicates
    environment = env.lookup(node)
    try:
        environment.lookup([None] + node.scopedName())
        # the name already exists in this scope so we
        # just ignore it
        return
    except KeyError:
        pass

    environment = env.lookup(node)
    scope = environment.scope()
    name = tyutil.mapID(node.identifier())

    guard = tyutil.guardName(scope + [name])
    
    stream.out("""\
#ifndef __@guard@__
#define __@guard@__

  class @name@;
  class _objref_@name@;
  class _impl_@name@;
  typedef _objref_@name@* @name@_ptr;
  typedef @name@_ptr @name@Ref;

  class @name@_Helper {
  public:
    typedef @name@_ptr _ptr_type;

    static _ptr_type _nil();
    static _CORBA_Boolean is_nil(_ptr_type);
    static void release(_ptr_type);
    static void duplicate(_ptr_type);
    static size_t NP_alignedSize(_ptr_type, size_t);
    static void marshalObjRef(_ptr_type, NetBufferedStream&);
    static _ptr_type unmarshalObjRef(NetBufferedStream&);
    static void marshalObjRef(_ptr_type, MemBufferedStream&);
    static _ptr_type unmarshalObjRef(MemBufferedStream&);
  };

  typedef _CORBA_ObjRef_Var<_objref_@name@, @name@_Helper> @name@_var;
  typedef _CORBA_ObjRef_OUT_arg<_objref_@name@,@name@_Helper > @name@_out;

#endif
""", guard = guard, name = name)    

def visitConst(node):
    if not(node.mainFile()):
        return

    environment = env.lookup(node)
    scope = environment.scope()
    
    constType = node.constType()
    deref_constType = tyutil.deref(constType)
    if isinstance(deref_constType, idltype.String):
        type_string = "char *"
    else:
        type_string = environment.principalID(deref_constType)
    name = tyutil.mapID(node.identifier())
    value = tyutil.valueString(deref_constType, node.value(), environment)

    representedByInteger =tyutil.const_init_in_def(deref_constType)

    # depends on whether we are inside a class / in global scope
    # etc
    # should be rationalised with tyutil.const_qualifier
    if self.__insideClass:
        if representedByInteger:
            stream.out("""\
  static _core_attr const @type@ @name@ _init_in_cldecl_( = @val@ );""",
                       type = type_string, name = name, val = value)
        else:
            stream.out("""\
  static _core_attr const @type@ @name@;""",
                       type = type_string, name = name)
    else:
        if self.__insideModule:
            where = "MODULE"
        else:
            where = "GLOBAL"
        if representedByInteger:
            stream.out("""\
  _CORBA_@where@_VARINT const @type@ @name@ _init_in_decl_( = @val@ );""",
                       where = where,
                       type = type_string,
                       name = name,
                       val = value)
        else:
            stream.out("""\
  _CORBA_@where@_VAR _core_attr const @type@ @name@;""",
                       where = where,
                       type = type_string,
                       name = name)


def visitTypedef(node):
    if not(node.mainFile()):
        return
    
    environment = env.lookup(node)
    scope = environment.scope()

    is_global_scope = not(self.__insideModule or self.__insideInterface)
    
    aliasType = node.aliasType()
    aliasTypeID = environment.principalID(aliasType)
    
    # work out the actual type being aliased by walking the list
    derefType = tyutil.deref(aliasType)
    derefTypeID = environment.principalID(derefType)

    # the basic name of the immediately aliased type
    basicReferencedTypeID = environment.principalID(aliasType)
    
    # for sequences and object references, construct the template
    # instance
    if tyutil.isSequence(derefType):
        sequenceTemplate = tyutil.sequenceTemplate(derefType, environment)
    elif tyutil.isObjRef(derefType):
        objRefTemplate = tyutil.objRefTemplate(derefType, "Member", environment)
    
    # each one is handled independently
    for d in node.declarators():
        
        # derivedName is the new typedef'd name
        # alias_dims is a list of dimensions of the type being aliased
        # is_array is true iff the aliased type is an array
        derivedName = tyutil.mapID(d.identifier())
        alias_dims = tyutil.typeDims(aliasType)
        is_array = (alias_dims != [])

        # array_declarator indicates whether this is a simple (non-array)
        # declarator or not
        array_declarator = d.sizes() != []

        # Typecode and Any
        if config.TypecodeFlag():
            qualifier = tyutil.const_qualifier(self.__insideModule,
                                               self.__insideClass)
            stream.out("""\
@qualifier@ _dyn_attr const CORBA::TypeCode_ptr _tc_@name@;
""",
                       qualifier = qualifier, name = derivedName)
                    


        # is it a simple alias (ie not an array at this level)?
        if not(array_declarator):
            # not an array declarator but a simple declarator to an array
            if is_array:
                # simple alias to an array should alias all the
                # array handling functions, but we don't need to duplicate
                # array looping code since we can just call the functions
                # for the base type
                stream.out("""\
typedef @base@ @derived@;
typedef @base@_slice @derived@_slice;
typedef @base@_copyHelper @derived@_copyHelper;
typedef @base@_var @derived@_var;
typedef @base@_out @derived@_out;
typedef @base@_forany @derived@_forany;
""",
                           base = basicReferencedTypeID,
                           derived = derivedName)
                # the declaration of the alloc(), dup() and free() methods
                # depend on whether the declaration is in global scope
                if not(is_global_scope):
                    stream.out("""\
static @derived@_slice* @derived@_alloc() { return @base@_alloc(); }
static @derived@_slice* @derived@_dup(const @derived@_slice* p) { return @base@_dup(p); }
static void @derived@_free( @derived@_slice* p) { @base@_free(p); }
""",
                               base = basicReferencedTypeID,
                               derived = derivedName)
                else:
                    stream.out("""\
extern @derived@_slice* @derived@_alloc();
extern @derived@_slice* @derived@_dup(const @derived@_slice* p);
extern void @derived@_free( @derived@_slice* p);
""",
                               base = basicReferencedTypeID,
                               derived = derivedName)
                           
            # Non-array of string
            elif tyutil.isString(derefType):
                stream.out("""\
typedef char* @name@;
typedef CORBA::String_var @name@_var;
""",
                           name = derivedName)
            elif tyutil.isTypeCode(derefType):
                stream.out("""\
typedef CORBA::TypeCode_ptr @name@_ptr;
typedef CORBA::TypeCode_var @name@_var;""",
                           name = derivedName)
            elif tyutil.isAny(derefType):
                stream.out("""\
typedef CORBA::Any @name@;
typedef CORBA::Any_var @name@_var;""",
                           name = derivedName)
            # Non-array of basic type
            elif isinstance(derefType, idltype.Base):

                # typedefs to basic types are always fully qualified?
                # IDL oddity?
                basicReferencedTypeID = self.__globalScope.principalID(aliasType)
                #basicReferencedTypeID = environment.principalID(aliasType, 1)
                stream.out("""\
typedef @base@ @derived@;
""",
                           base = basicReferencedTypeID,
                           derived = derivedName)
            # a typedef to a struct or union, or a typedef to a
            # typedef to a sequence
            elif tyutil.isStruct(derefType) or \
                 tyutil.isUnion(derefType)  or \
                 (tyutil.isSequence(derefType) and \
                  tyutil.isTypedef(aliasType)):

                stream.out("""\
typedef @base@ @name@;
typedef @base@_var @name@_var;
typedef @base@_out @name@_out;
""",
                           base = basicReferencedTypeID,
                           name = derivedName)
                    
            # Non-array of objrect reference
            elif tyutil.isObjRef(derefType):
                # Note that the base name is fully flattened
                is_CORBA_Object = derefType.scopedName() == ["CORBA", "Object"]
                impl_base = ""
                objref_base = ""
                if not(is_CORBA_Object):
                    scopedName = derefType.decl().scopedName()
                    relName = environment.relName(scopedName)
                    
                    impl_scopedName = tyutil.scope(relName) + \
                                      ["_impl_" + tyutil.name(relName)]
                    objref_scopedName = tyutil.scope(relName) + \
                                        ["_objref_" + tyutil.name(relName)]
                    impl_name = environment.nameToString(impl_scopedName)
                    objref_name = environment.nameToString(objref_scopedName)
                    
                    impl_base = "typedef " + impl_name + "_impl_" + derivedName + ";"
                    objref_base = "typedef " + objref_name + "_objref_" + \
                                  derivedName + ";"

                stream.out("""\
typedef @base@ @name@;
typedef @base@_ptr @name@_ptr;
typedef @base@Ref @name@Ref;
@impl_base@
typedef @base@_Helper @name@_Helper;
@objref_base@
typedef @base@_var @name@_var;
typedef @base@_out @name@_out;""",
                           base = derefTypeID,
                           name = derivedName,
                           impl_base = impl_base,
                           objref_base = objref_base)
            # Non-array of user declared types
            elif isinstance(derefType, idltype.Declared):
                stream.out("""\
typedef @base@ @name@;""",
                           base = basicReferencedTypeID,
                           name = derivedName)
            # Non-array of sequence
            elif isinstance(derefType, idltype.Sequence):
                seqType = derefType.seqType()
                seqDerefType = tyutil.deref(seqType)
                bounded = derefType.bound()
                
                seq_dims = tyutil.typeDims(seqType)
                is_array = (seq_dims != [])

                templateName = tyutil.sequenceTemplate(derefType, environment)

                if tyutil.isString(seqDerefType):
                    element = "_CORBA_String_member"
                    element_IN = "char *"
                elif tyutil.isObjRef(seqDerefType):
                    element = environment.principalID(seqType) + "_ptr"
                    element_IN = element
                # only if an anonymous sequence
                elif tyutil.isSequence(seqType):
                    element = tyutil.sequenceTemplate(seqDerefType,
                                                      environment)
                    element_IN = element
                else:
                    element = environment.principalID(seqType)
                    element_IN = element
                element_ptr = element_IN
                if tyutil.isString(seqDerefType) and \
                   not(is_array):
                    element_ptr = "char*"
                elif tyutil.isObjRef(seqDerefType) and \
                     not(is_array):
                    element_ptr = environment.principalID(seqType) + "_ptr"
                # only if an anonymous sequence
                elif tyutil.isSequence(seqType) and \
                     not(is_array):
                    element_ptr = element
                elif tyutil.isTypeCode(seqDerefType):
                    element_ptr = "CORBA::TypeCode_member"
                    element = element_ptr
                    element_IN = element_ptr
                else:
                    element_ptr = environment.principalID(seqType)
                    
                # enums are a special case
                # from o2be_sequence.cc:795:
                # ----
                # gcc requires that the marshalling operators for the
                # element be declared before the sequence template is
                # typedef'd. This is a problem for enums, as the
                # marshalling operators are not yet defined (and are
                # not part of the type itself).
                # ----
                # Note that the fully dereferenced name is used
                if is_global_scope:
                    friend = ""
                else:
                    friend = "friend"
                    
                if tyutil.isEnum(seqDerefType) and \
                   not(is_array):
                    stream.out("""\
// Need to declare <<= for elem type, as GCC expands templates early
#if defined(__GNUG__) && __GNUG__ == 2 && __GNUC_MINOR__ == 7
 @friend@ inline void operator >>= (@element@, NetBufferedStream&);
 @friend@ inline void operator <<= (@element@&, NetBufferedStream&);
 @friend@ inline void operator >>= (@element@, MemBufferedStream&);
 @friend@ inline void operator <<= (@element@&, MemBufferedStream&);
#endif""",
                    element = environment.principalID(seqDerefType),
                    friend = friend)
                        
                # derivedName is the new type identifier
                # element is the name of the basic element type
                # seq_dims contains dimensions if a sequence of arrays
                # templateName contains the template instantiation

                stream.out("""\
  class @name@_var;

  class @name@ : public @derived@ {
  public:
    typedef @name@_var _var_type;
    inline @name@() {}
    inline @name@(const @name@& s)
      : @derived@(s) {}
    """,
                name = derivedName,
                element = element_IN,
                derived = templateName)
                if not(bounded):
                    
                    stream.out("""\
    inline @name@(_CORBA_ULong _max)
      : @derived@(_max) {}
    inline @name@(_CORBA_ULong _max, _CORBA_ULong _len, @element@* _val, _CORBA_Boolean _rel=0)
      : @derived@(_max, _len, _val, _rel) {}
    """,
                               name = derivedName,
                               element = element_ptr,
                               derived = templateName)
                stream.out("""\
    inline @name@& operator = (const @name@& s) {
      @derived@::operator=(s);
      return *this;
    }
  };""",
                name = derivedName,
                element = element_IN,
                derived = templateName)
                
                subscript_operator_var = util.StringStream()
                subscript_operator_out = util.StringStream()
                    
                if is_array:
                    subscript_operator_var.out("""\
    inline @element@_slice* operator [] (_CORBA_ULong s) {
      return (@element@_slice*) ((pd_seq->NP_data())[s]);
    }""", element = element_ptr)
                    subscript_operator_out.out("""\
    inline @element@_slice* operator [] (_CORBA_ULong i) {
      return (@element@_slice*) ((_data->NP_data())[i]);
    }""", element = element_ptr)
                else:
                    if tyutil.isString(seqDerefType):
                        # special case alert
                        element_reference = element
                    elif tyutil.isObjRef(seqDerefType):
                        element_reference = tyutil.objRefTemplate(seqDerefType,
                                                                  "Member",
                                                                  environment)
                    # only if an anonymous sequence
                    elif tyutil.isSequence(seqType):
                        element_reference = tyutil.sequenceTemplate(seqDerefType, environment) + "&"
                    else:
                        element_reference = element + "&"
                    subscript_operator_var.out("""\
    inline @element@ operator [] (_CORBA_ULong s) {
      return (*pd_seq)[s];
    }""", element = element_reference)
                    subscript_operator_out.out("""\
    inline @element@ operator [] (_CORBA_ULong i) {
      return (*_data)[i];
    }""", element = element_reference)                        
                        
                stream.out("""\
  class @name@_out;

  class @name@_var {
  public:
    typedef @name@ T;
    typedef @name@_var T_var;
    
    inline @name@_var() : pd_seq(0) {}
    inline @name@_var(T* s) : pd_seq(s) {}
    inline @name@_var(const T_var& s) {
      if( s.pd_seq )  pd_seq = new T(*s.pd_seq);
      else             pd_seq = 0;
    }
    inline ~@name@_var() { if( pd_seq )  delete pd_seq; }
    
    inline T_var& operator = (T* s) {
      if( pd_seq )  delete pd_seq;
      pd_seq = s;
      return *this;
    }
    inline T_var& operator = (const T_var& s) {
      if( s.pd_seq ) {
        if( !pd_seq )  pd_seq = new T;
        *pd_seq = *s.pd_seq;
      } else if( pd_seq ) {
        delete pd_seq;
        pd_seq = 0;
      }
      return *this;
    }
    @subscript_operator_var@
    inline T* operator -> () { return pd_seq; }
#if defined(__GNUG__) && __GNUG__ == 2 && __GNUC_MINOR__ == 7
    inline operator T& () const { return *pd_seq; }
#else
    inline operator const T& () const { return *pd_seq; }
    inline operator T& () { return *pd_seq; }
#endif
    
    inline const T& in() const { return *pd_seq; }
    inline T&       inout()    { return *pd_seq; }
    inline T*&      out() {
      if( pd_seq ) { delete pd_seq; pd_seq = 0; }
      return pd_seq;
    }
    inline T* _retn() { T* tmp = pd_seq; pd_seq = 0; return tmp; }
    
    friend class @name@_out;
  
  private:
    T* pd_seq;
  };
  
  class @name@_out {
  public:
    typedef @name@ T;
    typedef @name@_var T_var;

    inline @name@_out(T*& s) : _data(s) { _data = 0; }
    inline @name@_out(T_var& s)
      : _data(s.pd_seq) { s = (T*) 0; }
    inline @name@_out(const @name@_out& s) : _data(s._data) {}
    inline @name@_out& operator = (const @name@_out& s) {
      _data = s._data;
      return *this;
    }  inline @name@_out& operator = (T* s) {
      _data = s;
      return *this;
    }
    inline operator T*&()  { return _data; }
    inline T*& ptr()       { return _data; }
    inline T* operator->() { return _data; }
    @subscript_operator_out@

    T*& _data;
  
  private:
    @name@_out();
    @name@_out& operator=(const T_var&);
  };

  """,
                           name = derivedName,
                           element = element,
                           subscript_operator_var = str(subscript_operator_var),
                           subscript_operator_out = str(subscript_operator_out))
            else:                  
                # FIXME: finish the rest later
                raise "No code for type = " + repr(type)


        # ----------------------------------------------------------------
        # declarator is an array typedef declarator
        elif array_declarator:

            all_dims = d.sizes() + alias_dims
            dimsString = tyutil.dimsToString(d.sizes())
            taildims = tyutil.dimsToString(d.sizes()[1:])
            
            typestring = basicReferencedTypeID
            if tyutil.isString(derefType) and \
               not(is_array):
                typestring = "CORBA::String_member"
            elif tyutil.isObjRef(derefType) and \
                 not(is_array):
                typestring = objRefTemplate
            elif tyutil.isTypeCode(derefType) and \
                 not(is_array):
                typestring = "CORBA::TypeCode_member"
            elif tyutil.isSequence(aliasType) and \
                 not(is_array):
                typestring = sequenceTemplate
                             
            stream.out("""\
typedef @type@ @name@@dims@;
typedef @type@ @name@_slice@taildims@;""",
                       name = derivedName,
                       type = typestring,
                       dims = dimsString,
                       taildims = taildims)

            # if in global scope we define the functions as extern
            if is_global_scope:
                stream.out("""\
extern @name@_slice* @name@_alloc();
extern @name@_slice* @name@_dup(const @name@_slice* _s);
extern void @name@_free(@name@_slice* _s);
""", name = derivedName)
            else:
                stream.out("""
static inline @name@_slice* @name@_alloc() {
  return new @name@_slice[@firstdim@];
}

static inline @name@_slice* @name@_dup(const @name@_slice* _s) {
   if (!_s) return 0;
   @name@_slice* _data = @name@_alloc();
   if (_data) {""",
                           type = basicReferencedTypeID,
                           name = derivedName,
                           dims = dimsString,
                           firstdim = repr(all_dims[0]),
                           taildims = taildims)
                stream.inc_indent()
                index = 0
                subscript = ""
                for dimension in all_dims:
                    stream.out("""\
     for (unsigned int _i@index@ =0;_i@index@ < @dimension@;_i@index@++) {""",
                               index = repr(index),
                               dimension = repr(dimension))
                    stream.inc_indent()
                    subscript = subscript + "[_i" + repr(index)+"]"
                    index = index + 1
                stream.out("""\
       _data@subscript@ = _s@subscript@;""", subscript = subscript)
                for dimension in all_dims:
                    stream.dec_indent()
                    stream.out("""\
     }""")
                stream.dec_indent()
                stream.out("""\
   }
   return _data;
}

static inline void @name@_free(@name@_slice* _s) {
    delete [] _s;
}
""", name = derivedName)
            stream.out("""\
class @name@_copyHelper {
public:
  static inline @name@_slice* alloc() { return @name@_alloc(); }
  static inline @name@_slice* dup(const @name@_slice* p) { return @name@_dup(p); }
  static inline void free(@name@_slice* p) { @name@_free(p); }
};

typedef _CORBA_Array_Var<@name@_copyHelper,@name@_slice> @name@_var;
typedef _CORBA_Array_OUT_arg<@name@_slice,@name@_var > @name@_out;
typedef _CORBA_Array_Forany<@name@_copyHelper,@name@_slice> @name@_forany;
""", name = derivedName)
                
     

def visitMember(node):
    if not(node.mainFile()):
        return
    
    memberType = node.memberType()
    if node.constrType():
        # if the type was declared here, it must be an instance
        # of idltype.Declared!
        assert isinstance(memberType, idltype.Declared)
        memberType.decl().accept(self)


def visitStruct(node):
    if not(node.mainFile()):
        return

    name = node.identifier()
    cxx_name = tyutil.mapID(name)

    outer_environment = env.lookup(node)
    environment = outer_environment.enterScope(name)
    
    scope = environment.scope()
    
    insideClass = self.__insideClass
    self.__insideClass = 1
            
    stream.out("""\
struct @name@ {""", name = cxx_name)
    stream.inc_indent()
        
    if tyutil.isVariableDecl(node):
        type = "Variable"
    else:
        type = "Fix"
            
    stream.out("""\
typedef _CORBA_ConstrType_@type@_Var<@name@> _var_type;""",
               name = cxx_name, type = type)

    # First pass through the members outputs code for all the user
    # declared new types
    user_decls = filter(lambda x: isinstance(x.memberType(),
                                             idltype.Declared),
                        node.members())
    for m in user_decls:
        m.accept(self)

    for m in node.members():
        memberType = m.memberType()
        derefType = tyutil.deref(memberType)

        is_array = tyutil.typeDims(memberType) != []

        if is_array:
            memtype = environment.principalID(memberType)
        else:
            # strings always seem to be a special case
            if tyutil.isString(derefType):
                memtype = "CORBA::String_member"
            elif tyutil.isObjRef(derefType):
                memtype = tyutil.objRefTemplate(derefType, "Member", environment)    
            elif tyutil.isTypeCode(derefType):
                memtype = "CORBA::TypeCode_member"
            elif tyutil.isTypedef(memberType):
                memtype = environment.principalID(memberType)
            elif tyutil.isSequence(memberType):
                sequence_template = tyutil.sequenceTemplate(memberType, environment)

                for d in m.declarators():
                    instname = tyutil.mapID(d.identifier())
                    if d.sizes() != []:
                        dims_string = tyutil.dimsToString(d.sizes())
                        stream.out("""\
@sequence_template@ @instname@@dims@;""",
                                   sequence_template = sequence_template,
                                   instname = instname,
                                   dims = dims_string)
                    else:
                        memtype = instname + "_seq"
                        stream.out("""\
typedef @sequence_template@ _@memtype@;
_@memtype@ @instname@;""",
                                   sequence_template = sequence_template,
                                   memtype = memtype,
                                   instname = instname)
                continue

            elif tyutil.isEnum(memberType):
                memtype = environment.principalID(memberType)

            else:
                memtype = environment.principalID(memberType)

        for d in m.declarators():
            dims = d.sizes()
            dims_string = tyutil.dimsToString(dims)
            id = tyutil.mapID(d.identifier())
            stream.out("""\
@type@ @identifier@@dims@;""", type = memtype,
                       identifier = id,
                       dims = dims_string)


    stream.out("""\
  size_t _NP_alignedSize(size_t initialoffset) const;
  void operator>>= (NetBufferedStream &) const;
  void operator<<= (NetBufferedStream &);
  void operator>>= (MemBufferedStream &) const;
  void operator<<= (MemBufferedStream &);
};

typedef @name@::_var_type @name@_var;

typedef _CORBA_ConstrType_@type@_OUT_arg< @name@,@name@_var > @name@_out;
""", type = type,name = cxx_name)

    self.__insideClass = insideClass

    # TypeCode and Any
    if config.TypecodeFlag():
        # structs in C++ are classes with different default privacy policies
        qualifier = tyutil.const_qualifier(self.__insideModule,
                                           self.__insideClass)
        stream.out("""\
@qualifier@ _dyn_attr const CORBA::TypeCode_ptr _tc_@name@;""",
                   qualifier = qualifier, name = cxx_name)



def visitException(node):
    if not(node.mainFile()):
        return
    
    exname = node.identifier()
    cxx_exname = tyutil.mapID(exname)

    outer_environment = env.lookup(node)
    environment = outer_environment.enterScope(exname)
    
    scope = environment.scope()
    insideClass = self.__insideClass
    self.__insideClass = 1

    # if the exception has no members, inline some no-ops
    no_members = (node.members() == [])


    stream.out("""\
class @name@ : public CORBA::UserException {
public:
  """, name = cxx_exname)

    # deal with the datamembers and constructors
    data = util.StringStream()
    ctor_args = []
    for m in node.members():
        memberType = m.memberType()
        derefType = tyutil.deref(memberType)
        type_dims = tyutil.typeDims(memberType)

        # is it constructed here?
        if m.constrType():
            memberType.decl().accept(self)
        
        for d in m.declarators():
            decl_dims = d.sizes()
            full_dims = decl_dims + type_dims
            is_array = full_dims != []
            is_array_declarator = decl_dims != []

            if is_array:
                type = environment.principalID(memberType)
            elif tyutil.isObjRef(derefType):
                type = tyutil.objRefTemplate(derefType, "Member", environment)
            elif tyutil.isTypeCode(derefType):
                type = "CORBA::TypeCode_member"
            elif tyutil.isString(derefType):
                type = "CORBA::String_member"
            elif tyutil.isTypedef(memberType):
                type = environment.principalID(memberType)
            elif tyutil.isSequence(derefType):
                type = tyutil.sequenceTemplate(derefType, environment)
                
            else:
                type = environment.principalID(memberType)
            name = tyutil.mapID(d.identifier())
            dims = tyutil.dimsToString(decl_dims)

            ctor_arg_type = tyutil.makeConstructorArgumentType(memberType,
                                                               environment)

            if is_array_declarator:
                ctor_arg_type = "const _0RL_" + name                
                data.out("""\
    typedef @type@ _0RL_@name@@dims@;
    typedef @type@ _@name@_slice;""", type = type, name = name, dims = dims)
                
            data.out("""\
    @type@ @name@@dims@;""", type = type, name = name, dims = dims)
            type = tyutil.operationArgumentType(memberType, environment)[1]
            ctor_args.append(ctor_arg_type + " i_" + name)
    ctor = ""
    if ctor_args != []:
        ctor = cxx_exname + "(" + string.join(ctor_args, ", ") + ");"
            
    if no_members:
        inline = "inline"
        body = "{ }"
        alignedSize = ""
    else:
        inline = ""
        body = ";"
        alignedSize = "size_t _NP_alignedSize(size_t) const;"
            
    stream.out("""\
  @datamembers@

  inline @name@() {
    pd_insertToAnyFn    = insertToAnyFn;
    pd_insertToAnyFnNCP = insertToAnyFnNCP;
  }
  @name@(const @name@&);
  @constructor@
  @name@& operator=(const @name@&);
  virtual ~@name@();
  virtual void _raise();
  static @name@* _downcast(CORBA::Exception*);
  static const @name@* _downcast(const CORBA::Exception*);
  static inline @name@* _narrow(CORBA::Exception* e) {
    return _downcast(e);
  }
  
  @alignedSize@

  @inline@ void operator>>=(NetBufferedStream&) const @body@
  @inline@ void operator>>=(MemBufferedStream&) const @body@
  @inline@ void operator<<=(NetBufferedStream&) @body@
  @inline@ void operator<<=(MemBufferedStream&) @body@

  static _core_attr insertExceptionToAny    insertToAnyFn;
  static _core_attr insertExceptionToAnyNCP insertToAnyFnNCP;

  static _core_attr const char* _PD_repoId;

private:
  virtual CORBA::Exception* _NP_duplicate() const;
  virtual const char* _NP_typeId() const;
  virtual const char* _NP_repoId(int*) const;
  virtual void _NP_marshal(NetBufferedStream&) const;
  virtual void _NP_marshal(MemBufferedStream&) const;
};

""",
               name = cxx_exname, datamembers = str(data),
               constructor = ctor,
               inline = inline, body = body,
               alignedSize = alignedSize)
    self.__insideClass = insideClass

    # Typecode and Any
    if config.TypecodeFlag():
        qualifier = tyutil.const_qualifier(self.__insideModule,
                                           self.__insideClass)
        stream.out("""\
@qualifier@ _dyn_attr const CORBA::TypeCode_ptr _tc_@name@;
""", qualifier = qualifier, name = cxx_exname)
    


def visitUnion(node):
    if not(node.mainFile()):
        return
    
    name = node.identifier()
    cxx_name = tyutil.mapID(name)
    outer_environment = env.lookup(node)
    environment = outer_environment.enterScope(name)
    
    scope = environment.scope()
    insideClass = self.__insideClass
    self.__insideClass = 1
    
    switchType = node.switchType()
    
    # returns a representation of the union discriminator in a form
    # that is a legal value in C++
    #def discrimValueToString(switchType, caselabel, environment):
        # CORBA 2.3draft 3.10.2.2
        #   switch type ::= <integer_type>
        #               |   <char_type>
        #               |   <boolean_type>
        #               |   <enum_type>
        #               |   <scoped_name>
        # where scoped_name must be an already declared version of one
        # of the simpler types
    #    discrimvalue = caselabel.value()#

    #    return tyutil.valueString(switchType, discrimvalue, environment)
        

    # in the case where there is no default case and an implicit default
    # member, choose a discriminator value to set. Note that attempting
    # to access the data is undefined
    #
    # FIXME: The new AST representation now has
    #   CaseLabel.default() : true if representing the default case
    #   CaseLabel.value()   : if the default case, returns a value
    #                         used by none of the other labels
    # For the time being, keep the independent mechanism because it seems
    # to closely match the old compiler's behaviour.
    # Once this backend is trusted independantly, convert to use the
    # new mechanism?
    def chooseArbitraryDefault(switchType = switchType,
                               allCaseValues = tyutil.allCaseValues(node),
                               environment = environment):
        # dereference the switch_type (ie if CASE <scoped_name>)
        switchType = tyutil.deref(switchType)
                
        # CASE <integer_type>
        if switchType.kind() == idltype.tk_short:
            # - [ 2 ^ (32-1) -1 ]
            return "-32767"
        elif switchType.kind() == idltype.tk_long:
            # - [ 2 ^ (64-1) -1 ]
            return "-2147483647"
        elif switchType.kind() == idltype.tk_ushort    or \
             switchType.kind() == idltype.tk_longlong  or \
             switchType.kind() == idltype.tk_ulong     or \
             switchType.kind() == idltype.tk_ulonglong:
            return "1"
        # CASE <char_type>
        elif tyutil.isChar(switchType):
            # choose the first one not already used
            allcases = map(lambda x: x.value(), allCaseValues)
            possibles = map(chr, range(0, 255))
            difference = util.minus(possibles, allcases)
            return tyutil.valueString(switchType, difference[0], None)
            return "'\\000'"
        # CASE <boolean_type>
        elif tyutil.isBoolean(switchType):
            return "0"
        # CASE <enum_type>
        elif tyutil.isEnum(switchType):
            enums = switchType.decl().enumerators()
            # pick the first enum not already in a case
            allcases = map(lambda x: x.value(), allCaseValues)
            difference = util.minus(enums, allcases)
            scopedName = difference[0].scopedName()
            # need to be careful of scope
            rel_name = environment.relName(scopedName)
            return environment.nameToString(rel_name)
            return tyutil.name(difference[0].scopedName())
        else:
            raise "chooseArbitraryDefault type="+repr(switchType)+\
                  " val="+repr(discrimvalue)            

    # does the IDL union have any default case?
    # It'll be handy to know which case is the default one later-
    # so add a new attribute to mark it
    hasDefault = tyutil.getDefaultCaseAndMark(node) != None
        
    # CORBA 2.3 C++ Mapping 1-34
    # "A union has an implicit default member if it does not have
    # a default case and not all permissible values of the union
    # discriminant are listed"
    exhaustive = tyutil.exhaustiveMatch(switchType, tyutil.allCaseValues(node))
    implicitDefault = not(hasDefault) and not(exhaustive)
    
    if tyutil.isVariableDecl(node):
        fixed = "Variable"
    else:
        fixed = "Fix"
    stream.out("""\
class @unionname@ {
public:

  typedef _CORBA_ConstrType_@fixed@_Var<@unionname@> _var_type;
  @unionname@() {""",unionname = cxx_name, fixed = fixed)
    stream.inc_indent()
        
    if implicitDefault:
        stream.out("""\
    _default();""")
                      
    elif hasDefault:
        stream.out("""\
    pd__default = 1;
    pd__d = @default@;""", default = chooseArbitraryDefault())
    stream.dec_indent()
    stream.out("""\
  }
  """)
    for section in ['copyconst', 'equalsop']:
        if (section is 'copyconst'):
            stream.out("""\
  @unionname@(const @unionname@& _value) {""", unionname = cxx_name)
        else:
            stream.out("""\
  @unionname@& operator=(const @unionname@& _value) {""", unionname = cxx_name)
        stream.inc_indent()
        if not(exhaustive):
            stream.out("""\
    if ((pd__default = _value.pd__default)) {
      pd__d = _value.pd__d;""", unionname = cxx_name)
            # the default case (if it exists) need initialising here
            for c in node.cases():
                if c.isDefault:
                    stream.out("""\
      @default@(_value.pd_@default@);""",
                               default=tyutil.mapID(c.declarator().identifier()))
            stream.dec_indent()
            stream.out("""\
    }
    else {""")
        stream.out("""\
      switch(_value.pd__d) {""")
        stream.inc_indent()
        # iterate over cases
        for c in node.cases():
            # FIXME: multiple case labels not taken care of
            # FIXME: need to represent discriminator value properly
            for l in c.labels():
                if l.default(): continue
                # FIXME: stupid special case. An explicit discriminator
                # value of \0 -> 0000 whereas an implicit one (valueString)
                # \0 -> '\000'
                discrimvalue = tyutil.valueString(switchType, l.value(), environment)
                #discrimvalue = discrimValueToString(switchType, l, environment)
                if tyutil.isChar(switchType) and l.value() == '\0':
                    discrimvalue = "0000"
                stream.out("""\
         case @discrimvalue@: @name@(_value.pd_@name@); break;""",
                           discrimvalue = discrimvalue,
                           name = tyutil.mapID(c.declarator().identifier()))
        # Booleans are a special case (isn't everything?)
        booleanWrap = tyutil.isBoolean(switchType) \
                      and exhaustive
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
            stream.out("""\
    }""")
        if section is 'equalsop':
            stream.out("""\
    return *this;
  }""")
        else:
            stream.dec_indent()
            stream.out("""\
  }

  ~@unionname@() {}
  """, unionname= cxx_name)
            # deal with the discriminator
    stream.out("""\
  
  @discrimtype@ _d() const { return pd__d;}
  void _d(@discrimtype@ _value) {}
  """, discrimtype = environment.principalID(switchType))

    if implicitDefault:
        stream.out("""\
  void _default()
  {
    pd__d = @arbitraryDefault@;
    pd__default = 1;
  }
  """, arbitraryDefault = chooseArbitraryDefault())

    # get and set functions for each case:
    for c in node.cases():
        # Following the typedef chain will deliver the base type of
        # the alias. Whether or not it is an array is stored in an
        # ast.Typedef node.
        caseType = c.caseType()
        dims = tyutil.typeDims(caseType)
            
        # find the dereferenced type of the member if its an alias
        derefType = tyutil.deref(caseType)

        # the mangled name of the member
        decl = c.declarator()
        decl_dims = decl.sizes()

        full_dims = decl_dims + dims

        is_array = full_dims != []
        anonymous_array = decl_dims != []
        alias_array = dims != []
        
        member = tyutil.mapID(decl.identifier())
        # the name of the member type (not flattened)
        type = environment.principalID(caseType)
        # FIXME: multiple labels might be broken
        for l in c.labels():
            # depends entirely on the dereferenced type of the member
            if l.default():
                discrimvalue = chooseArbitraryDefault()
            else:
                discrimvalue = tyutil.valueString(switchType, l.value(),
                                                  environment)
                #discrimvalue = discrimValueToString(switchType,
                #                                    l, environment)

            # FIXME: stupid special case, see above
            if tyutil.isChar(switchType) and l.value() == '\0':
                discrimvalue = "0000"

            type_str = type

            type_predefined = anonymous_array and not(alias_array) or \
                              not(is_array)
            
            if tyutil.isString(derefType) and \
               (anonymous_array and not(alias_array) or \
                not(is_array)):
                type_str = "CORBA::String_member"
            if tyutil.isObjRef(derefType):
                if alias_array:
                    type_str = type_str
                elif anonymous_array:
                    type_str = tyutil.objRefTemplate(derefType, "Member",
                                                     environment)
                else:
                    type_str = environment.principalID(derefType)
            elif tyutil.isSequence(caseType):
                type_str = tyutil.sequenceTemplate(caseType, environment)
                #type_str = "_" + member + "_seq"

            # only different when array is anonymous
            const_type_str = type_str
                
            # anonymous arrays are handled slightly differently
            if anonymous_array:
                prefix = config.name_prefix()
                stream.out("""\
   typedef @type_str@ @prefix@_@name@@dims@;
   typedef @type_str@ _@name@_slice@tail_dims@;
   """,
                        prefix = prefix,
                        type_str = type_str,
                        name = member,
                        dims = tyutil.dimsToString(decl.sizes()),
                        tail_dims = tyutil.dimsToString(decl.sizes()[1:]))
                const_type_str = prefix + "_" + member
                type_str = "_" + member
             
            if is_array:
                # arrays
                # build the loop
                loop = util.StringStream()
                dimsString = tyutil.dimsToString(range(0, len(full_dims)), "_i")
                index = 0
                for size in full_dims:
                    loop.out("""\
    for (unsigned int _i@index@ =0;_i@index@ < @size@;_i@index@++) {""",
                             index = str(index), size = str(size))
                    index = index + 1
                    loop.inc_indent()
                loop.out("""\
      pd_@name@@dimsString@ = _value@dimsString@;""",
                         name = member, dimsString = dimsString)
                for size in full_dims:
                    loop.dec_indent()
                    loop.out("""\
    }""")             
                stream.out("""\
  const @type@_slice *@name@ () const { return pd_@name@; }
  void @name@ (const @const_type@ _value) {
    pd__d = @discrimvalue@;
    pd__default = @isDefault@;
    @loop@
  }""",
                           type = type_str,
                           const_type = const_type_str,
                           name = member,
                           isDefault = str(c.isDefault),
                           discrimvalue = discrimvalue,
                           loop = str(loop))
            elif derefType.kind() == idltype.tk_any:
                # note type != CORBA::Any when its an alias...
                stream.out("""\
  const @type@ &@name@ () const { return pd_@name@; }
  @type@ &@name@ () { return pd_@name@; }
  void @name@ (const @type@& _value) {
    pd__d = @discrimvalue@;
    pd__default = @isDefault@;
    pd_@name@ = _value;
  }""",
                           type = type_str,
                           name = member,
                           isDefault = str(c.isDefault),
                           discrimvalue = discrimvalue)
            elif derefType.kind() == idltype.tk_TypeCode:
                stream.out("""\
  CORBA::TypeCode_ptr @name@ () const { return pd_@name@._ptr; }
  void @name@(CORBA::TypeCode_ptr _value) {
    pd__d = @discrimvalue@;
    pd__default = @isDefault@;
    pd_@name@ = CORBA::TypeCode::_duplicate(_value);
  }
  void @name@(const CORBA::TypeCode_member& _value) {
    pd__d = @discrimvalue@;
    pd__default = @isDefault@;
    pd_@name@ = _value;
  }
  void @name@(const CORBA::TypeCode_var& _value) {
    pd__d = @discrimvalue@;
    pd__default = @isDefault@;
    pd_@name@ = _value;
  }""",
                           name = member,
                           isDefault = str(c.isDefault),
                           discrimvalue = discrimvalue)
                
                
            elif isinstance(derefType, idltype.Base) or \
                            tyutil.isEnum(derefType):
                # basic type
                stream.out("""\
  @type@ @name@ () const { return pd_@name@; }
  void @name@ (@type@  _value) {
    pd__d = @discrimvalue@;
    pd__default = @isDefault@;
    pd_@name@ = _value;
  }""",
                           type = type_str,
                           name = member,
                           isDefault = str(c.isDefault),
                           discrimvalue = discrimvalue)

            elif isinstance(derefType, idltype.String):
                stream.out("""\
  const char * @name@ () const { return (const char*) pd_@name@; }
  void @name@(char* _value) {
    pd__d = @discrimvalue@;
    pd__default = @isDefault@;
    pd_@name@ = _value;
  }
  void @name@(const char*  _value) {
    pd__d = @discrimvalue@;
    pd__default = @isDefault@;
    pd_@name@ = _value;
  }
  void @name@(const CORBA::String_var& _value) {
    pd__d = @discrimvalue@;
    pd__default = @isDefault@;
    pd_@name@ = _value;
  }
  void @name@(const CORBA::String_member& _value) {
    pd__d = @discrimvalue@;
    pd__default = @isDefault@;
    pd_@name@ = _value;
  }""",
                           name = member,
                           isDefault = str(c.isDefault),
                           discrimvalue = discrimvalue)
            elif tyutil.isObjRef(derefType):
                objref = tyutil.objRefTemplate(derefType, "Member", environment)
                stream.out("""\
  @type@_ptr @member@ () const { return pd_@member@._ptr; }
  void @member@(@type@_ptr _value) {
    pd__d = @discrimvalue@;
    pd__default = @isDefault@;
    @type@_Helper::duplicate(_value);
    pd_@member@ = _value;
  }
  void @member@(const @objref@& _value) {
    pd__d = @discrimvalue@;
    pd__default = @isDefault@;
    pd_@member@ = _value;
  }
  void @member@(const @type@_var&  _value) {
    pd__d = @discrimvalue@;
    pd__default = @isDefault@;
    pd_@member@ = _value;
  }""",
                           member = member,
                           type = type_str,
                           objref = objref,
                           isDefault = str(c.isDefault),
                           discrimvalue = discrimvalue)
            elif tyutil.isTypedef(caseType) or \
                 tyutil.isStruct(derefType) or \
                 tyutil.isUnion(derefType):

                stream.out("""\
  const @type@ &@name@ () const { return pd_@name@; }
  @type@ &@name@ () { return pd_@name@; }
  void @name@ (const @type@& _value) {
    pd__d = @discrimvalue@;
    pd__default = @isDefault@;
    pd_@name@ = _value;
  }""",
                           type = type_str,
                           name = member,
                           isDefault = str(c.isDefault),
                           discrimvalue = discrimvalue)

            elif isinstance(derefType, idltype.Sequence):
                sequence_template  = tyutil.sequenceTemplate(derefType,
                                                             environment)
                stream.out("""\
  typedef @sequence_template@ _@member@_seq;
  const _@member@_seq& @member@ () const { return pd_@member@; }
  _@member@_seq& @member@ () { return pd_@member@; }
  void @member@ (const _@member@_seq& _value) {
    pd__d = @discrimvalue@;
    pd__default = @isDefault@;
    pd_@member@ = _value;
  }""",
                           sequence_template = sequence_template,
                           member = member,
                           isDefault = str(c.isDefault),
                           discrimvalue = discrimvalue)

            else:
                raise "Don't know how to output code for union type: "+type
    stream.out("""\

  size_t _NP_alignedSize(size_t initialoffset) const;
  void operator>>= (NetBufferedStream&) const;
  void operator<<= (NetBufferedStream&);
  void operator>>= (MemBufferedStream&) const;
  void operator<<= (MemBufferedStream&);
  """)
    # Typecode and Any
    if config.TypecodeFlag():
        guard_name = tyutil.guardName(node.scopedName())
        stream.out("""\
#if defined(__GNUG__) || defined(__DECCXX) && (__DECCXX_VER < 60000000)
    friend class _0RL_tcParser_unionhelper_@name@;
#else
    friend class ::_0RL_tcParser_unionhelper_@name@;
#endif
""", name = guard_name)
    stream.out("""\
private:
  """)

    # declare the instance of the discriminator and
    # the actual data members (shock, horror)
    # FIXME: there is some interesting behaviour in
    # o2be_union::produce_hdr which I should examine more
    # carefully
    inside = util.StringStream()
    outside = util.StringStream()
    used_inside = 0
    used_outside = 0
    for c in node.cases():

        # find the dereferenced type of the member if its an alias
        caseType = c.caseType()
        derefType = tyutil.deref(caseType)
        is_variable = tyutil.isVariableType(derefType)

        case_dims = tyutil.typeDims(caseType)

        decl = c.declarator()
        decl_dims = decl.sizes()

        full_dims = case_dims + decl_dims
        
        is_array = full_dims != []
        anonymous_array = decl_dims != []
        alias_array = case_dims != []

        member_name = tyutil.mapID(c.declarator().identifier())
        # put fixed types inside the union and variable types
        # outside. FIXME: is this if correct?
        type_str = environment.principalID(caseType)
        if tyutil.isString(derefType) and \
               (anonymous_array and not(alias_array) or \
                not(is_array)):
                type_str = "CORBA::String_member"
        if tyutil.isObjRef(derefType):
            if alias_array:
                type_str = type_str
            else:
                type_str = tyutil.objRefTemplate(derefType, "Member",
                                                 environment)
        elif tyutil.isTypeCode(derefType) and not(is_array):
            type_str = "CORBA::TypeCode_member"
        elif tyutil.isSequence(caseType) and anonymous_array:
            # the typedef _name_seq is not defined in this case
            type_str = tyutil.sequenceTemplate(caseType, environment)
        elif tyutil.isSequence(caseType) and not(anonymous_array):
            # sequence template typedef already exists
            type_str = "_" + member_name + "_seq"

        dims_str = tyutil.dimsToString(decl_dims)
            
        # floats in unions are special cases
        if tyutil.isFloating(derefType) and not(is_array):
            inside.out("""\
#ifndef USING_PROXY_FLOAT
  @type@ pd_@name@@dims@;
#endif""", type = type_str, name = member_name, dims = dims_str)
            outside.out("""\
#ifdef USING_PROXY_FLOAT
  @type@ pd_@name@@dims@;
#endif""", type = type_str, name = member_name, dims = dims_str)
            used_inside = used_outside = 1
        else:
            if is_array and tyutil.isStruct(derefType) and not(is_variable):
                this_stream = inside
                used_inside = 1
            else:
                if (isinstance(derefType, idltype.Declared) or  \
                    isinstance(derefType, idltype.Sequence) or  \
                    isinstance(derefType, idltype.String)   or \
                    derefType.kind() == idltype.tk_any      or \
                    derefType.kind() == idltype.tk_TypeCode)  and \
                    not(tyutil.isEnum(derefType)):
                    this_stream = outside
                    used_outside = 1
                else:
                    this_stream = inside
                    used_inside = 1
            this_stream.out("""\
    @type@ pd_@name@@dims@;""",
                            type = type_str,
                            name = member_name,
                            dims = dims_str)
  
    discrimtype = environment.principalID(switchType)
        
    if tyutil.isVariableDecl(node):
        isVariable = "Variable"
    else:
        isVariable = "Fix"

    stream.out("""\
    @discrimtype@ pd__d;
    CORBA::Boolean pd__default;""",
               discrimtype = discrimtype)
    if used_inside:
        stream.out("""\
    union {
      @insideUnion@
    };""",
                   insideUnion=str(inside))
    if used_outside:
        stream.out("""\
    @outsideUnion@
  };""", outsideUnion = str(outside))
    else:
        stream.dec_indent()
        stream.out("""\
  };""")
    stream.dec_indent()
    stream.out("""\
typedef @Name@::_var_type @Name@_var;
typedef _CORBA_ConstrType_@isVariable@_OUT_arg< @Name@,@Name@_var > @Name@_out;
""",
               isVariable = isVariable,
               Name = cxx_name)

    self.__insideClass = insideClass

    # TypeCode and Any
    if config.TypecodeFlag():
        qualifier = tyutil.const_qualifier(self.__insideModule,
                                           self.__insideClass)
        stream.out("""\
@qualifier@ _dyn_attr const CORBA::TypeCode_ptr _tc_@name@;""",
                   qualifier = qualifier, name = cxx_name)

    return


def visitEnum(node):
    if not(node.mainFile()):
        return
    
    name = tyutil.mapID(node.identifier())
    cxx_name = tyutil.mapID(name)
    enumerators = node.enumerators()
    memberlist = map(lambda x: tyutil.name(x.scopedName()), enumerators)
    stream.out("""\
enum @name@ { @memberlist@ };
typedef @name@& @name@_out;
""", name = cxx_name, memberlist = string.join(memberlist, ", "))

    # TypeCode and Any
    if config.TypecodeFlag():
        insideModule = self.__insideModule
        insideClass = self.__insideClass
        qualifier = tyutil.const_qualifier(insideModule, insideClass)
        stream.out("""\
@qualifier@ _dyn_attr const CORBA::TypeCode_ptr _tc_@name@;""",
                   qualifier = qualifier, name = cxx_name)
    
    node.written = cxx_name
    return

