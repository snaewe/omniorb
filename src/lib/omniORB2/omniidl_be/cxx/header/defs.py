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

from omniidl import idlast, idltype, idlutil

from omniidl.be.cxx import tyutil, util, name

import defs

self = defs

# Not implemented yet:
# Flags which control the behaviour of the backend
isFragment = 0

# State information (used to be passed as arguments during recursion)
self.__insideInterface = 0
self.__insideModule = 0

# IDL (not C++) scope, should be filtered through tyutil.mapID before
# being used
#self.__scope = []
self.__environment = name.Environment()
#self.__scope = util.Environment()

def enter(scope):
    #self.__scope.append(scope)
    self.__environment = self.__environment.enterScope(scope)
def leave():
    #self.__scope = self.__scope[0:-1]
    self.__environment = self.__environment.leaveScope()
def currentScope():
    #return self.__scope[:]
    return self.__environment.scope()
def addName(name):
    self.__environment.add(name)


def __init__(stream):
    defs.stream = stream
    return defs

#
# Control arrives here
#
def visitAST(node):
#    print "visitAST(" + repr(node) + ")"
#    print "stream = " + repr(stream)

    for n in node.declarations():
        n.accept(self)

def visitModule(node):
    # o2be_module::produce_hdr
    addName(node.identifier())
    
    name = tyutil.mapID(node.identifier())
    
    if not(isFragment):
        stream.out("""\
_CORBA_MODULE @name@
_CORBA_MODULE_BEG""", name = name)
        stream.inc_indent()

    insideModule = self.__insideModule
    self.__insideModule = 1

    enter(node.identifier())
    
    for n in node.definitions():
        n.accept(self)

    leave()

    self.__insideModule = insideModule
    if not(isFragment):
        stream.dec_indent()
        stream.out("""\
_CORBA_MODULE_END
""")
    node.written = name;

def visitInterface(node):
#    print "[[[ visitInterface
    try:
        addName(node.identifier())
    except KeyError:
        pass
    
    name = tyutil.mapID(node.identifier())
    interface_scope = currentScope()
#    print "[[[ interface scope = " + repr(interface_scope) + "]]]"
    enter(node.identifier())    
    scope = currentScope()
    environment = self.__environment

    insideInterface = self.__insideInterface
    self.__insideInterface = 1
    
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
               name = name,
               guard = guard)
    # output code for other declarations within this scope
    for n in node.declarations():
#        print "[[[ scope = " + repr(scope) + "  interface_scope = " + \
#              repr(interface_scope) + "]]]"
        n.accept(self)
#        print "[[[ scope = " + repr(scope) + "  interface_scope = " + \
#              repr(interface_scope) + "]]]"
#    print "[[[ ---- MARK ---- ]]]"
        
    # build methods corresponding to attributes, operations etc
    attributes = []
    operations = []
    virtual_operations = []
        
    for c in node.callables():
        if isinstance(c, idlast.Attribute):
            attrType = c.attrType()
            derefAttrType = tyutil.deref(attrType)
            
            returnType = tyutil.operationArgumentType(attrType, environment)[0]
            inType = tyutil.operationArgumentType(attrType, environment)[1]
            
            #if tyutil.isObjRef(derefAttrType):
            #    type = tyutil.principalID(derefAttrType, scope) + "_ptr"
            #
            #else:
            #    type = tyutil.operationArgumentType(attrType, interface_scope)[0]
            for i in c.identifiers():
                attribname = tyutil.mapID(i)
                attributes.append(returnType + " " + attribname + "()")
                if not(c.readonly()):
                    attributes.append("void " + attribname + "(" \
                                      + inType + ")")
        elif isinstance(c, idlast.Operation):
            def argumentTypeToString(arg, virtual = 0, env = environment):
                return tyutil.operationArgumentType(arg, env, virtual)

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
#                print "[[[ tuple = " + repr(tuple) + "]]]"
            return_type = argumentTypeToString(c.returnType())[0]
#            print "[[[ returnType = " + repr(c.returnType()) + \
#                  "    text = " + repr(return_type) + "]]]"
            opname = tyutil.mapID(c.identifier())
            arguments = util.delimitedlist(params, ", ")
            virtual_arguments = util.delimitedlist(virtual_params, ", ")
            operations.append(return_type + " " + opname + \
                              "(" + arguments + ")")
            virtual_operations.append(return_type + " " + opname + \
                                      "(" + virtual_arguments + ")")
        else:
            raise "No code for interface member: " + repr(c)

    attributes_str = util.delimitedlist(map(lambda x: x + ";\n",
                                            attributes ),"")
    operations_str = util.delimitedlist(map(lambda x: x + ";\n",
                                            operations ),"")
        
    virtual_attributes_str = util.delimitedlist(
        map( lambda x: "virtual " + x + " = 0;\n", attributes ), "")
    virtual_operations_str = util.delimitedlist(
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
            
    objref_inherits = util.delimitedlist(objref_inherits, ",\n")
    impl_inherits = util.delimitedlist(impl_inherits, ", \n")
        
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
               name = name,
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
               name = name,
               virtual_operations = virtual_operations_str,
               virtual_attributes = virtual_attributes_str)

    self.__insideInterface = insideInterface

    leave()

    

def visitForward(node):
#    print "[[[ visitForward ]]]"
    addName(node.identifier())
    
    name = tyutil.mapID(node.identifier())

    scope = currentScope()
    
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
#    print "[[[ visitConst ]]]"
    scope = currentScope()
    environment = self.__environment
    
    constType = node.constType()
    if isinstance(constType, idltype.String):
        type_string = "char *"
    else:
        type_string = environment.principalID(constType)
    name = tyutil.mapID(node.identifier())
    value = tyutil.valueString(constType, node.value(), environment)
#    value = str(node.value())
#    if tyutil.isChar(constType):
#        value = "'" + value + "'"
            
    # depends on whether enclosed by an interface or not
    if self.__insideInterface:
        stream.out("""\
  static _core_attr const @type@ @name@ _init_in_cldecl_( = @val@ );""",
                   type = type_string, name = name, val = value)
    else:
        representedByInteger = tyutil.isInteger(constType) or \
                               tyutil.isChar(constType)    or \
                               tyutil.isBoolean(constType) or \
                               tyutil.isOctet(constType)
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


def visitDeclarator(node):
    print "[[[ visitDeclarator ]]]"
    print "[[[ node = " + repr(node) + "]]]"
    raise "Shouldn't be using the visitor pattern to visit a declarator!"


def visitTypedef(node):
    # need to have some way of keeping track of current scope
    scope = currentScope()
    environment = self.__environment

    is_global_scope = not(self.__insideModule or self.__insideInterface)
    
    # don't define the same typedef twice...
    if (hasattr(node, "written")):
        raise "Is this check ever needed?"
        return node.written
    node.written = "dummy"

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
        addName(d.identifier())
        
        # derivedName is the new typedef'd name
        # alias_dims is a list of dimensions of the type being aliased
        # is_array is true iff the aliased type is an array
        derivedName = tyutil.mapID(d.identifier())
        alias_dims = tyutil.typeDims(aliasType)
        is_array = (alias_dims != [])

        # array_declarator indicates whether this is a simple (non-array)
        # declarator or not
        array_declarator = d.sizes() != []

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
typedef @base@_forany @derived@_forany;""",
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
typedef CORBA::String_var @name@_var;""",
                           name = derivedName)

            # Non-array of basic type
            elif isinstance(derefType, idltype.Base):
                # typedefs to basic types are always fully qualified?
                # IDL oddity?
                basicReferencedTypeID = environment.principalID(aliasType, 1)
                stream.out("""\
typedef @base@ @derived@;""",
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
typedef @base@_out @name@_out;""",
                           base = basicReferencedTypeID,
                           name = derivedName)
            # Non-array of objrect reference
            elif tyutil.isObjRef(derefType):
                # Note that the base name is fully flattened
                    stream.out("""\
typedef @base@ @name@;
typedef @base@_ptr @name@_ptr;
typedef @base@Ref @name@Ref;
typedef _impl_@base@ _impl_@name@;
typedef @base@_Helper @name@_Helper;
typedef _objref_@base@ _objref_@name@;
typedef @base@_var @name@_var;
typedef @base@_out @name@_out;""",
                               base = derefTypeID,
                               name = derivedName)
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
            
            #append = lambda x,y: x + y
            #dimsString = reduce(append,
            #                    map(lambda x: "["+repr(x)+"]", d.sizes()))
            #taildims = reduce(append,
            #                  map(lambda x: "["+repr(x)+"]", d.sizes()[1:]),"")
            typestring = basicReferencedTypeID
            if tyutil.isString(derefType) and \
               not(is_array):
                typestring = "CORBA::String_member"
            elif tyutil.isObjRef(derefType) and \
                 not(is_array):
                typestring = objRefTemplate
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
#    print "[[[ visitMember ]]]"
    memberType = node.memberType()
#    print "memberType == " + repr(memberType)
#    print "constrType == " + repr(node.constrType())

    if node.constrType():
        # if the type was declared here, it must be an instance
        # of idltype.Declared!
        assert isinstance(memberType, idltype.Declared)
        memberType.decl().accept(self)


def visitStruct(node):
#    print "[[[ visitStruct ]]]"

#        if (hasattr(node,"written")):
#            return node.written
    addName(node.identifier())
    
    name = tyutil.mapID(node.identifier())
    enter(node.identifier())

    scope = currentScope()
    environment = self.__environment
#    print "[[[ scope = " + repr(scope) + "]]]"
            
    stream.out("""\
struct @name@ {""", name = name)
    stream.inc_indent()
        
    if tyutil.isVariableDecl(node):
        type = "Variable"
    else:
        type = "Fix"
            
    stream.out("""\
typedef _CORBA_ConstrType_@type@_Var<@name@> _var_type;""",
               name = name, type = type)
    # First pass through the members outputs code for all the user
    # declared new types
    user_decls = filter(lambda x: isinstance(x.memberType(),
                                             idltype.Declared),
                        node.members())
    # we remember the assigned type by storing it in the ast node itself
    # the actual member instance can then access it in a mo'
    for m in user_decls:
        m.accept(self)
#        m.memtype = defs(stream, m.type_spec().decl(),
#                             isFragment, insideInterface, insideModule)
#            print "[[[ just stored memtype = " + repr(m.memtype) + " ]]]"

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
            elif tyutil.isTypedef(memberType):
                memtype = environment.principalID(memberType)
                #memtype = tyutil.name(memberType.name())
                #            print "[[[ memtype = " + repr(memtype) + "]]]"
                # Deal with sequence members
            elif tyutil.isSequence(memberType):
                sequence_template = tyutil.sequenceTemplate(memberType, environment)

                for d in m.declarators():
                    instname = tyutil.mapID(d.identifier())
                    memtype = instname + "_seq"
                    stream.out("""\
typedef @sequence_template@ _@memtype@;
_@memtype@ @instname@;""",
                               sequence_template = sequence_template,
                               memtype = memtype,
                               instname = instname)
                continue

            elif tyutil.isEnum(memberType):
                memtype = tyutil.mapID(memberType.decl().identifier())
                #            print "[[[ memtype = " + repr(memtype) + "]]]"
                # If it's a user declared type then remember the type we assigned?
            elif isinstance(memberType, idltype.Declared) and \
                 hasattr(m,"memtype"):
                if m.memtype != None:
                    #                    print "[[[stored]]]"
                    memtype = m.memtype
                else:
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
""", type = type,name = name)

    node.written = name
#    print "[[[ scope = " + repr(currentScope()) + "]]]"
    leave()

def visitException(node):
#    print "[[[ visitException ]]]"
    # the exception's name
    addName(node.identifier())
    
    exname = tyutil.mapID(node.identifier())

    enter(node.identifier())

    scope = currentScope()
    environment = self.__environment
    
    # if the exception has no members, inline some no-ops
    no_members = (node.members() == [])


    # Used to build the types for exception constructors
    def makeConstructorArgumentType(type, environment = environment):
        # idl.type -> string
        typeName = environment.principalID(type)
        isVariable = tyutil.isVariableType(type)
        derefType = tyutil.deref(type)
        derefTypeName = environment.principalID(derefType)

        if isinstance(derefType, idltype.Base) or \
           tyutil.isEnum(derefType):
            return typeName
        if tyutil.isStruct(derefType)          or \
           tyutil.isUnion(derefType):
            return "const " + typeName + "&"
        if tyutil.isObjRef(derefType):
            return derefTypeName + "_ptr"
        if tyutil.isSequence(type, 0):
            return "const " + tyutil.sequenceTemplate(derefType, environment)
        elif tyutil.isSequence(type, 1):
            return "const " + typeName
        else:
            return tyutil.operationArgumentType(type, environment)[1]

    
    # deal with the datamembers and constructors
    data = util.StringStream()
    ctor_args = []
    for m in node.members():
        memberType = m.memberType()
        derefType = tyutil.deref(memberType)
        for d in m.declarators():
            ctor_arg_type = makeConstructorArgumentType(memberType, environment)

            if tyutil.isObjRef(derefType):
                type = tyutil.objRefTemplate(derefType, "Member", environment)
            elif tyutil.isTypedef(memberType):
                type = environment.principalID(memberType)
            elif tyutil.isSequence(derefType):
                type = tyutil.sequenceTemplate(derefType, environment)
            else:
                type = environment.principalID(memberType)
            name = tyutil.mapID(d.identifier())
            data.out("""\
    @type@ @name@;""", type = type, name = name)
            type = tyutil.operationArgumentType(memberType, environment)[1]
            ctor_args.append(ctor_arg_type + " i_" + name)
    ctor = ""
    if ctor_args != []:
        ctor = exname + "(" + util.delimitedlist(ctor_args) + ");"
            
    if no_members:
        inline = "inline"
        body = "{ }"
        alignedSize = ""
    else:
        inline = ""
        body = ";"
        alignedSize = "size_t _NP_alignedSize(size_t) const;"
            
    stream.out("""\
class @name@ : public CORBA::UserException {
public:
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
               name = exname, datamembers = str(data),
               constructor = ctor,
               inline = inline, body = body,
               alignedSize = alignedSize)
    leave()


def visitUnion(node):
#    print "[[[ visitUnion ]]]"
    addName(node.identifier())
    
    enter(node.identifier())

    scope = currentScope()
    environment = self.__environment
    
    switchType = node.switchType()
    
    # returns a representation of the union discriminator in a form
    # that is a legal value in C++
    def discrimValueToString(switchType, caselabel, environment):
        # CORBA 2.3draft 3.10.2.2
        #   switch type ::= <integer_type>
        #               |   <char_type>
        #               |   <boolean_type>
        #               |   <enum_type>
        #               |   <scoped_name>
        # where scoped_name must be an already declared version of one
        # of the simpler types
        discrimvalue = caselabel.value()

        return tyutil.valueString(switchType, discrimvalue, environment)
        
        # CASE <integer_type>
#        if (switchType.kind() == idltype.tk_short     or
#            switchType.kind() == idltype.tk_ushort):
#            return str(discrimvalue)
#        elif (switchType.kind() == idltype.tk_long      or
#              switchType.kind() == idltype.tk_longlong  or
#              switchType.kind() == idltype.tk_ulong     or
#              switchType.kind() == idltype.tk_ulonglong):
#            # discrimvalue is of the form "\d+L"
#            string = str(int(eval(str(discrimvalue))))
#            return string
#        # CASE <char_type>
#        elif (switchType.kind() == idltype.tk_char    or
#              switchType.kind() == idltype.tk_wchar):
#            # HACK!
#            if (repr(discrimvalue) != "'None'"):
#                return "'" + discrimvalue + "'"
#            return "'" + "\\000" + "'"
#        # CASE <boolean_type>
#        elif (switchType.kind() == idltype.tk_boolean):
#            return str(discrimvalue)
#        # CASE <enum_type>
#        elif (switchType.kind() == idltype.tk_enum):
#            discrimvalue = tyutil.name(discrimvalue.scopedName())
#            target_scope = tyutil.scope(switchType.decl().scopedName())
#            rel_scope = idlutil.pruneScope(target_scope, from_scope)
#            return idlutil.ccolonName(rel_scope) + str(discrimvalue)
#        # CASE <scoped_name>
#        elif (switchType.kind() == idltype.tk_alias):
#            switchType = tyutil.deref(switchType)
#            return discrimValueToString(switchType, discrimvalue, from_scope)
#        else:
#            raise "discrimValueToString type="+repr(switchType)+ \
#                  " val="+repr(discrimvalue)

    def allCaseValues(node = node):
        list = []
        for n in node.cases():
            for l in n.labels():
                if not(l.default()):
                    list.append(l)
        return list

    # determines whether a set of cases represents an Exhaustive Match
    def exhaustiveMatch(switchType = switchType,
                        allCaseValues = allCaseValues):
        # return all the used case values
        
        # dereference the switch_type (ie if CASE <scoped_name>)
        switchType = tyutil.deref(switchType)
            
        # same as discrimValueToString
        # CASE <integer_type>
        if tyutil.isInteger(switchType):
            # Assume that we can't possibly do an exhaustive match
            # on the integers, since they're (mostly) very big.
            # maybe should rethink this for the short ints...
            return 0
        # CASE <char_type>
        elif tyutil.isChar(switchType):
            # make a set with all the characters inside
            s = []
            for char in range(0,256):
                s.append(chr(char))
            # make a set with all the used characters
            used = allCaseValues()
            # subtract all the used ones from the possibilities
            difference = util.minus(s, used)
            # if the difference is empty, match is exhaustive
            return (len(difference) == 0)
        # CASE <boolean_type>
        elif tyutil.isBoolean(switchType):
            s = [0, 1]
            used = map(lambda x: x.value(), allCaseValues())
            difference = util.minus(s, used)
            return (len(difference) == 0)
        # CASE <enum_type>
        elif tyutil.isEnum(switchType):
            s = switchType.decl().enumerators()
            used = map(lambda x: x.value(), allCaseValues())
            difference = util.minus(s, used)
            return (len(difference) == 0)
        else:
            raise "exhaustiveMatch type="+repr(switchType)+ \
                  " val="+repr(discrimvalue)

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
                               allCaseValues = allCaseValues):
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
            return "'\\000'"
        # CASE <boolean_type>
        elif tyutil.isBoolean(switchType):
            return "0"
        # CASE <enum_type>
        elif tyutil.isEnum(switchType):
            enums = switchType.decl().enumerators()
            # pick the first enum not already in a case
            allcases = map(lambda x: x.value(), allCaseValues())
#            print "[[[ enums = " + repr(enums) + "]]]"
#            print "[[[ allcases = " + repr(allcases) + "]]]"
            difference = util.minus(enums, allcases)
            return tyutil.name(difference[0].scopedName())
        else:
            raise "chooseArbitraryDefault type="+repr(switchType)+\
                  " val="+repr(discrimvalue)            

    # does the IDL union have any default case?
    # It'll be handy to know which case is the default one later-
    # so add a new attribute to mark it
    hasDefault = 0
    for c in node.cases():
        c.isDefault = 0
        for l in c.labels():
            if l.default():
                hasDefault = 1
                c.isDefault = 1
            
    # CORBA 2.3 C++ Mapping 1-34
    # "A union has an implicit default member if it does not have
    # a default case and not all permissible values of the union
    # discriminant are listed"
    exhaustive = exhaustiveMatch()
    implicitDefault = not(hasDefault) and not(exhaustive)
    
    name = tyutil.mapID(node.identifier())
    if tyutil.isVariableDecl(node):
        fixed = "Variable"
    else:
        fixed = "Fix"
    stream.out("""\
class @unionname@ {
public:

  typedef _CORBA_ConstrType_@fixed@_Var<@unionname@> _var_type;
  @unionname@() {""",unionname = name, fixed = fixed)
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
  @unionname@(const @unionname@& _value) {""", unionname = name)
        else:
            stream.out("""\
  @unionname@& operator=(const @unionname@& _value) {""", unionname = name)
        stream.inc_indent()
        if not(exhaustive):
            stream.out("""\
    if ((pd__default = _value.pd__default)) {
      pd__d = _value.pd__d;""", unionname = name)
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
                stream.out("""\
         case @discrimvalue@: @name@(_value.pd_@name@); break;""",
                           discrimvalue = discrimValueToString(switchType,
                                                               l,
                                                               environment),
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
  """, unionname= name)
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
        
        member = tyutil.mapID(decl.identifier())
        # the name of the member type (not flattened)
        type = environment.principalID(caseType)
        # FIXME: multiple labels might be broken
        for l in c.labels():
            # depends entirely on the dereferenced type of the member
            if l.default():
                discrimvalue = chooseArbitraryDefault()
            else:
                discrimvalue = discrimValueToString(switchType,
                                                    l, environment)
            
            if is_array:
                # arrays
                # build the loop
                loop = util.StringStream()
                append = lambda x,y: x + y
                dimsString = reduce(append, map(lambda x: "[_i"+repr(x)+"]",
                                                (range(0,len(full_dims)))))
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
  void @name@ (const @type@ _value) {
    pd__d = @discrimvalue@;
    pd__default = @isDefault@;
    @loop@
  }""",
                           type = type,
                           name = member,
                           isDefault = str(c.isDefault),
                           discrimvalue = discrimvalue,
                           loop = str(loop))
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
                           type = type,
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
                type = environment.principalID(derefType)
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
                           type = type,
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
                           type = type,
                           name = member,
                           isDefault = str(c.isDefault),
                           discrimvalue = discrimvalue)

            elif isinstance(derefType, idltype.Sequence):
                sequence_template  = tyutil.sequenceTemplate(derefType,
                                                             environment)
                
                c.memtype = "_"+member+"_seq"
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

        case_dims = tyutil.typeDims(caseType)
        is_array = case_dims != []

        if hasattr(c,"memtype"):
            type_str = c.memtype
        elif tyutil.isString(derefType) and not(is_array):
            type_str = "CORBA::String_member"
        elif tyutil.isObjRef(derefType):
            type_str = tyutil.objRefTemplate(derefType, "Member", environment)
        else:
            type_str = environment.principalID(caseType)

        member_name = tyutil.mapID(c.declarator().identifier())
        # put fixed types inside the union and variable types
        # outside. FIXME: is this if correct?
#            if (((isinstance(c.type_spec(), idltypes.Declared))and
#                not(c.type_spec().kind() == idltypes.tk_alias))or
#                (util.isVariableType(c.type_spec()))):
            # floats and doubles are special cases
        if tyutil.isFloating(derefType) and not(is_array):
            inside.out("""\
#ifndef USING_PROXY_FLOAT
  @type@ pd_@name@;
#endif""", type = type_str, name = member_name)
            outside.out("""\
#ifdef USING_PROXY_FLOAT
  @type@ pd_@name@;
#endif""", type = type_str, name = member_name)
            used_inside = used_outside = 1
        else:
            if (isinstance(derefType, idltype.Declared) or  \
                isinstance(derefType, idltype.Sequence) or  \
                isinstance(derefType, idltype.String))  and \
                not(tyutil.isEnum(derefType)):
                this_stream = outside
                used_outside = 1
            else:
                this_stream = inside
                used_inside = 1
            this_stream.out("""\
    @type@ pd_@name@;""",
                            type = type_str,
                            name = member_name)
  
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
               Name = name)
    leave()


def visitEnum(node):
#    print "[[[ visitEnum ]]]"
    addName(node.identifier())
    
    if hasattr(node,"written"):
        return node.written;
    name = tyutil.mapID(node.identifier())
    enumerators = node.enumerators()
    memberlist = map(lambda x: tyutil.name(x.scopedName()), enumerators)
    stream.out("""\
enum @name@ { @memberlist@ };
typedef @name@& @name@_out;
""", name = name, memberlist = util.delimitedlist(memberlist))
    node.written = name
    return name

