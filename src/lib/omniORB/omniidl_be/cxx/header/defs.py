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

from omniidl.be.cxx import tyutil, util

import defs

self = defs

# Not implemented yet:
# Flags which control the behaviour of the backend
isFragment = 0

# State information (used to be passed as arguments during recursion)
self.__insideInterface = 0
self.__insideModule = 0
self.__scope = []

def enter(scope):
    self.__scope.append(scope)
def leave(scope):
    self.__scope = self.__scope[0:-1]
def currentScope():
    return self.__scope


def __init__(stream):
    defs.stream = stream
    return defs

#
# Control arrives here
#
def visitAST(node):
    print "visitAST(" + repr(node) + ")"
    print "stream = " + repr(stream)

    for n in node.declarations():
        n.accept(self)

def visitModule(node):
    # o2be_module::produce_hdr
    name = node.identifier()
    if not(isFragment):
        stream.out("""\
_CORBA_MODULE @name@
_CORBA_MODULE_BEG""", name = name)
        stream.inc_indent()
  
    for n in node.definitions():
        n.accept(self)

    if not(isFragment):
        stream.dec_indent()
        stream.out("""\
_CORBA_MODULE_END
""")
    node.written = name;

def visitInterface(node):
#    print "[[[ visitInterface
    scope = currentScope()
    
    name = tyutil.mapID(node.identifier())
    # the ifndef guard name contains scope information
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
        n.accept(self)
        
    # build methods corresponding to attributes, operations etc
    attributes = []
    operations = []
    virtual_operations = []
        
    for c in node.callables():
        if isinstance(c, idlast.Attribute):
            attrType = c.attrType()
            type = tyutil.argumentTypeToString(attrType, scope)[0]
            for i in c.identifiers():
                attribname = tyutil.mapID(i)
                attributes.append(type + " " + attribname + "()")
                if not(c.readonly()):
                    attributes.append("void " + attribname + "(" \
                                      + type + ")")
        elif isinstance(c, idlast.Operation):
            def argumentTypeToString(arg, virtual = 0, scope = scope):
                return tyutil.operationArgumentType(arg, scope, virtual)

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
                argname = tyutil.mapID(p.identifier())
                params.append(tuple[0] + " " + argname)
                virtual_params.append(tuple[1] + " " + argname)

            return_type = argumentTypeToString(c.returnType())[0]
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
        scope = idlutil.ccolonName(tyutil.scope(i.scopedName()), scope)
        id = tyutil.mapID(i.identifier())
        objref_inherits.append("public virtual " + scope + "_objref_" + id)
        impl_inherits.append("public virtual " + scope + "_impl_" + id)
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
    

    

def visitForward(node):
#    print "[[[ visitForward ]]]"
    scope = scope()
    
    name = tyutil.mapID(node.identifier())
    guard = tyutil.guardName(scope + name)
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
    scope = scope()
    
    constType = node.constType()
    if isinstance(constType, idltype.String):
        type_string = "char *"
    else:
        type_string = tyutil.principalID(constType, scope)
    name = tyutil.mapID(node.identifier())
    value = str(node.value())
    if tyutil.isChar(constType):
        value = "'" + value + "'"
            
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
#    print "[[[ visitTypedef ]]]"

    # need to have some way of keeping track of current scope
    scope = currentScope()
    
    # don't define the same typedef twice...
    if (hasattr(node, "written")):
        raise "Is this check ever needed?"
        return node.written
    node.written = "dummy"
    aliasType = node.aliasType()
    
    # work out the actual type being aliased by walking the list
    derefType = tyutil.deref(aliasType)
    derefTypeID = tyutil.principalID(derefType, scope)

    # the name of the immediately referenced type
    referencedTypeID = tyutil.principalID(aliasType, scope)
    
    # each one is handled independently
    for d in node.declarators():
        derivedName = tyutil.mapID(d.identifier())
        dims = tyutil.typeDims(aliasType)
        is_array = (dims != [])

        # is it a simple alias (ie not an array at this level)?
        if d.sizes() == []:
            # simple alias to an array must alias all the
            # array handling functions
            if is_array:
                # we don't need to duplicate array looping code since
                # we can just call the functions for the base type
                stream.out("""\
typedef @base@ @derived@;
typedef @base@_slice @derived@_slice;
typedef @base@_copyHelper @derived@_copyHelper;
typedef @base@_var @derived@_var;
typedef @base@_out @derived@_out;
typedef @base@_forany @derived@_forany;
static @derived@_slice* @derived@_alloc() { return @base@_alloc(); }
static @derived@_slice* @derived@_dup(const @derived@_slice* p) { return @base@_dup(p); }
static void @derived@_free( @derived@_slice* p) { @base@_free(p); }
  """,
                           base = referencedTypeID,
                           derived = derivedName)
            # is it a string? special case alert.
            elif tyutil.isString(derefType):
                stream.out("""\
typedef char* @name@;
typedef CORBA::String_var @name@_var;""",
                           name = derivedName)

            # is it a simple (builtin) type
            elif isinstance(derefType, idltype.Base):
                # IDL oddity? fully qualife the base name
                referencedTypeID = tyutil.principalID(aliasType)
                stream.out("""\
typedef @base@ @derived@;""",
                           base = referencedTypeID,
                           derived = derivedName)
            
            elif isinstance(derefType, idltype.Declared):
                if derefType.kind() == idltype.tk_struct or \
                   derefType.kind() == idltype.tk_union:
                    stream.out("""\
typedef @base@ @name@;
typedef @base@_var @name@_var;
typedef @base@_out @name@_out;""",
                               base = derefTypeID,
                               name = derivedName)
                elif derefType.kind() == idltype.tk_objref:
                    stream.out("""\
typedef @base@ @name@;
typedef @base@_ptr @name@_ptr;
typedef @base@Ref @name@Ref;
typedef _impl_@base@ _sk_@name@;
typedef @base@_Helper @name@_Helper;
typedef _objref_@base@ _objref_@name@;
typedef @base@_var @name@_var;
typedef @base@_out @name@_out;""",
                               base = derefTypeID,
                               name = derivedName)

                else:
                    stream.out("""\
typedef @base@ @name@;""",
                               base = derefTypeID,
                               name = derivedName)
            elif isinstance(derefType, idltype.Sequence):
                if tyutil.isString(derefType):
                    element = "_CORBA_String_member"
                    element_IN = "char *"
                else:
                    element = tyutil.principalID(derefType, scope)
                    element_IN = element
                    
                dims = tyutil.typeDims(derefType)
                is_array = (dims != [])

                templateName = tyutil.sequenceTemplate(derefType, scope)

                # enums are a special case
                # from o2be_sequence.cc:795:
                # ----
                # gcc requires that the marshalling operators for the
                # element be declared before the sequence template is
                # typedef'd. This is a problem for enums, as the
                # marshalling operators are not yet defined (and are
                # not part of the type itself).
                # ----
                if tyutil.isEnum(derefType):
                    stream.out("""\
// Need to declare <<= for elem type, as GCC expands templates early
#if defined(__GNUG__) && __GNUG__ == 2 && __GNUC_MINOR__ == 7
  friend inline void operator >>= (@element@, NetBufferedStream&);
  friend inline void operator <<= (@element@&, NetBufferedStream&);
  friend inline void operator >>= (@element@, MemBufferedStream&);
  friend inline void operator <<= (@element@&, MemBufferedStream&);
#endif""",
                    element = element)
                        
                # derivedName is the new type identifier
                # element is the name of the basic element type
                # dims contains dimensions if a sequence of arrays
                # templateName contains the template instantiation

                stream.out("""\
  class @name@_var;

  class @name@ : public @derived@ {
  public:
    typedef @name@_var _var_type;
    inline @name@() {}
    inline @name@(const @name@& s)
      : @derived@(s) {}
    inline @name@(_CORBA_ULong _max)
      : @derived@(_max) {}
    inline @name@(_CORBA_ULong _max, _CORBA_ULong _len, @element@* _val, _CORBA_Boolean _rel=0)
      : @derived@(_max, _len, _val, _rel) {}
    inline @name@& operator = (const @name@& s) {
      @derived@::operator=(s);
      return *this;
    }
  };
  """,
                name = derivedName,
                element = element_IN,
                derived = templateName)
                
                subscript_operator_var = util.StringStream()
                subscript_operator_out = util.StringStream()
                    
                if is_array:
                    subscript_operator_var.out("""\
    inline @element@_slice* operator [] (_CORBA_ULong s) {
      return (@element@_slice*) ((pd_seq->NP_data())[s]);
    }""", element = element)
                    subscript_operator_out.out("""\
    inline @element@_slice* operator [] (_CORBA_ULong i) {
      return (@element@_slice*) ((_data->NP_data())[i]);
    }""", element = element)
                else:
                    if tyutil.isString(derefType):
                        # special case alert
                        element_reference = element
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
        # handle arrays
        elif d.sizes() != []:
            print "[[[ dims = " + repr(dims) + "  d.sizes() = " + repr(d.sizes()) + "]]]"
            dims = d.sizes() + dims
            append = lambda x,y: x + y
            dimsString = reduce(append,
                                map(lambda x: "["+repr(x)+"]", d.sizes()))
            taildims = reduce(append,
                              map(lambda x: "["+repr(x)+"]", d.sizes()[1:]),"")
            typestring = referencedTypeID
            stream.out("""\

typedef @type@ @name@@dims@;
typedef @type@ @name@_slice@taildims@;

static inline @name@_slice* @name@_alloc() {
  return new @name@_slice[@firstdim@];
}

static inline @name@_slice* @name@_dup(const @name@_slice* _s) {
   if (!_s) return 0;
   @name@_slice* _data = @name@_alloc();
   if (_data) {""",
                       type = referencedTypeID,
                       name = derivedName,
                       dims = dimsString,
                       firstdim = repr(dims[0]),
                       taildims = taildims)
            stream.inc_indent()
            index = 0
            subscript = ""
            for dimension in dims:
                stream.out("""\
     for (unsigned int _i@index@ =0;_i@index@ < @dimension@;_i@index@++) {""",
                           index = repr(index),
                           dimension = repr(dimension))
                stream.inc_indent()
                subscript = subscript + "[_i" + repr(index)+"]"
                index = index + 1
            stream.out("""\
       _data@subscript@ = _s@subscript@;""", subscript = subscript)
            for dimension in dims:
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

class @name@_copyHelper {
public:
  static inline @name@_slice* alloc() { return @name@_alloc(); }
  static inline @name@_slice* dup(const @name@_slice* p) { return @name@_dup(p); }
  static inline void free(@name@_slice* p) { @name@_free(p); }
};

typedef _CORBA_Array_Var<@name@_copyHelper,@name@_slice> @name@_var;
typedef _CORBA_Array_OUT_arg<@name@_slice,@name@_var > @name@_out;
typedef _CORBA_Array_Forany<@name@_copyHelper,@name@_slice> @name@_forany;
""",
                       name = derivedName,
                       dims = dimsString,
                       firstdim = repr(dims[0]))
        else:
            # probably an impossible condition
            raise("No code for declarator = " + repr(d))
     

def visitMember(node):
#    print "[[[ visitMember ]]]"
    memberType = node.memberType()
    print "memberType == " + repr(memberType)
    print "constrType == " + repr(node.constrType())

    if node.constrType():
        # if the type was declared here, it must be an instance
        # of idltype.Declared!
        assert isinstance(memberType, idltype.Declared)
        memberType.decl().accept(self)


def visitStruct(node):
#    print "[[[ visitStruct ]]]"

#        if (hasattr(node,"written")):
#            return node.written
    scope = currentScope()
            
    name = tyutil.mapID(node.identifier())
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
        # strings always seem to be a special case
        if tyutil.isString(derefType):
            memtype = "CORBA::String_member"
        elif tyutil.isObjRef(derefType):
            memtype = tyutil.objRefTemplate(derefType, "Member", scope)    
        elif tyutil.isTypedef(memberType):
            memtype = memberType.name()
            # Deal with sequence members
        elif tyutil.isSequence(memberType):
            sequence_template = util.sequenceTemplate(memberType, scope)

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
            memtype = tyutil.mapID(memberType.identifier())
        # If it's a user declared type then remember the type we assigned?
        elif isinstance(memberType, idltype.Declared) and \
             hasattr(m,"memtype"):
            if m.memtype != None:
#                    print "[[[stored]]]"
                memtype = m.memtype
            else:
                memtype = tyutil.principalID(memberType)
        else:
            memtype = tyutil.principalID(memberType)                
        # FIXME: doesnt deal with eg arrays
        decll = []
        for d in m.declarators():
            decll.append(d.identifier())
        decls = util.delimitedlist(decll)

#            print "[[[ memtype = " + repr(memtype) + "  decls = " + repr(decls) + "]]]"
        stream.out("""\
@type@ @decls@;""",type = memtype, decls = decls)

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

def visitException(node):
#    print "[[[ visitException ]]]"
    scope = currentScope()

    # if the exception has no members, inline some no-ops
    no_members = (node.members() == [])
    # the exception's name
    exname = tyutil.mapID(node.identifier())

    # Used to build the types for exception constructors
    def makeConstructorArgumentType(type, scope = []):
        # idl.type -> string
        param_type = tyutil.principalID(type, scope)
        isVariable = tyutil.isVariableType(type)
        derefType = tyutil.deref(type)
        if tyutil.isString(derefType):
            return "const char*"
        elif isinstance(type, idltype.Base):
            return tyutil.principalID(type, scope)
        elif tyutil.isEnum(derefType):
            return param_type
        elif tyutil.isSequence(type, 0):
            return "const " + tyutil.sequenceTemplate(derefType, scope)
        elif tyutil.isSequence(type, 1):
            return "const " + param_type
        elif tyutil.isTypedef(type):
            return param_type
        else:
            return tyutil.operationArgumentType(type, scope)[1]

    
    # deal with the datamembers and constructors
    data = util.StringStream()
    ctor_args = []
    for m in node.members():
        memberType = m.memberType()
        derefType = tyutil.deref(memberType)
        for d in m.declarators():
            ctor_arg_type = makeConstructorArgumentType(memberType, scope)

            if tyutil.isObjRef(derefType):
                type = tyutil.objRefTemplate(memberType, "Member", scope)
            elif tyutil.isTypedef(memberType):
                type = tyutil.principalID(memberType, scope)
            elif tyutil.isSequence(derefType):
                type = tyutil.sequenceTemplate(derefType, scope())
            else:
                type = tyutil.principalID(memberType, scope)
            name = tyutil.mapID(d.identifier())
            data.out("""\
    @type@ @name@;""", type = type, name = name)
            type = tyutil.operationArgumentType(memberType, scope)[1]
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


def visitUnion(node):
    print "[[[ visitUnion ]]]"

def visitEnum(node):
    print "[[[ visitEnum ]]]"


