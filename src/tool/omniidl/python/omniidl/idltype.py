#
# IDL type representation
#

import idlutil

tk_null               = 0
tk_void               = 1
tk_short              = 2
tk_long               = 3
tk_ushort             = 4
tk_ulong              = 5
tk_float              = 6
tk_double             = 7
tk_boolean            = 8
tk_char	              = 9
tk_octet              = 10
tk_any	              = 11
tk_TypeCode           = 12
tk_Principal          = 13
tk_objref             = 14
tk_struct             = 15
tk_union              = 16
tk_enum	              = 17
tk_string             = 18
tk_sequence           = 19
tk_array              = 20
tk_alias              = 21
tk_except             = 22
tk_longlong           = 23
tk_ulonglong          = 24
tk_longdouble         = 25
tk_wchar              = 26
tk_wstring            = 27
tk_fixed              = 28
tk_value              = 29
tk_value_box          = 30
tk_native             = 31
tk_abstract_interface = 32


class Error:
    def __init__(self, err):
        self.err = err

    def __repr__(self):
        return self.err


class Type:
    def __init__(self, kind):
        self.__kind  = kind

    # Typecode kind
    def kind(self):  return self.__kind


# Base types
class Base (Type):
    def __init__(self, kind):
        if kind != tk_null       and \
           kind != tk_void       and \
           kind != tk_short      and \
           kind != tk_long       and \
           kind != tk_ushort     and \
           kind != tk_ulong      and \
           kind != tk_float      and \
           kind != tk_double     and \
           kind != tk_boolean    and \
           kind != tk_char       and \
           kind != tk_octet      and \
           kind != tk_any        and \
           kind != tk_TypeCode   and \
           kind != tk_Principal  and \
           kind != tk_longlong   and \
           kind != tk_ulonglong  and \
           kind != tk_longdouble and \
           kind != tk_wchar:
            raise Error("Attempt to create Base type with invalid kind.")

        Type.__init__(self, kind)

    def accept(self, visitor): visitor.visitBaseType(self)


# Strings can be used like base types without a declaration. eg:
#
#   void op(in string<10> s);
#
# therefore, the String type must include its bound here, rather than
# relying on looking at the corresponding declaration

class String (Type):
    def __init__(self, bound):
        Type.__init__(self, tk_string)
        self.__bound = bound

    def accept(self, visitor): visitor.visitStringType(self)
    def bound(self): return self.__bound

class WString (Type):
    def __init__(self, bound):
        Type.__init__(self, tk_string)
        self.__bound = bound

    def accept(self, visitor): visitor.visitWStringType(self)
    def bound(self): return self.__bound


# Sequences are never declared. They either appear as
#
#   typedef sequence <...> ...
#
# or inside a struct or union

class Sequence (Type):
    def __init__(self, seqType, bound):
        Type.__init__(self, tk_sequence)
        self.__seqType = seqType
        self.__bound   = bound

    def accept(self, visitor): visitor.visitSequenceType(self)
    def seqType(self): return self.__seqType
    def bound(self):   return self.__bound


# Same goes for fixed

class Fixed (Type):
    def __init__(self, digits, scale):
        Type.__init__(self, tk_fixed)
        self.__digits = digits
        self.__scale  = scale

    def accept(self, visitor): visitor.visitFixedType(self)
    def digits(self): return self.__digits
    def scale(self):  return self.__scale


# All other types must be declared, at least implicitly, so they have
# an associated declaration object

class Declared (Type):
    def __init__(self, decl, scopedName, kind):
        if kind != tk_objref             and \
           kind != tk_struct             and \
           kind != tk_union              and \
           kind != tk_enum               and \
           kind != tk_array              and \
           kind != tk_alias              and \
           kind != tk_except             and \
           kind != tk_value              and \
           kind != tk_value_box          and \
           kind != tk_abstract_interface:
            raise Error("Attempt to create Declared type with invalid kind.")

        Type.__init__(self, kind)
        self.__decl       = decl
        self.__scopedName = scopedName

    def accept(self, visitor): visitor.visitDeclaredType(self)

    # Decl object where the type was declared.
    def decl(self):       return self.__decl

    # List containing scoped name:
    def scopedName(self): return self.__scopedName

    # Simple name
    def name(self):       return self.__scopedName[-1:]


# Map of singleton Base Type objects
baseTypeMap = {
    tk_null:       Base(tk_null),
    tk_void:       Base(tk_void),
    tk_short:      Base(tk_short),
    tk_long:       Base(tk_long),
    tk_ushort:     Base(tk_ushort),
    tk_ulong:      Base(tk_ulong),
    tk_float:      Base(tk_float),
    tk_double:     Base(tk_double),
    tk_boolean:    Base(tk_boolean),
    tk_char:       Base(tk_char),
    tk_octet:      Base(tk_octet),
    tk_any:        Base(tk_any),
    tk_TypeCode:   Base(tk_TypeCode),
    tk_Principal:  Base(tk_Principal),
    tk_longlong:   Base(tk_longlong),
    tk_ulonglong:  Base(tk_ulonglong),
    tk_longdouble: Base(tk_longdouble),
    tk_wchar:      Base(tk_wchar)
    }

# Maps of String and WString Type objects, indexed by bound
stringTypeMap =  { 0: String(0) }
wstringTypeMap = { 0: WString(0) }

# Map of Sequence Type objects, indexed by (type object,bound)
sequenceTypeMap = {}

# Map of Fixed Type objects, indexed by (digits,scale)
fixedTypeMap = {}

# Map of declared type objects, indexed by stringified scoped name
declaredTypeMap = {}


# Functions to create or return existing Type objects
def baseType(kind):
    return baseTypeMap[kind]

def stringType(bound):
    try:
        return stringTypeMap[bound]
    except KeyError:
        st = String(bound)
        stringTypeMap[bound] = st
        return st

def wstringType(bound):
    try:
        return wstringTypeMap[bound]
    except KeyError:
        wst = WString(bound)
        wstringTypeMap[bound] = wst
        return wst

def sequenceType(type_spec, bound):
    try:
        return sequenceTypeMap[(type_spec,bound)]
    except KeyError:
        st = Sequence(type_spec, bound)
        sequenceTypeMap[(type_spec,bound)] = st
        return st

def fixedType(digits, scale):
    try:
        return fixedTypeMap[(digits,scale)]
    except KeyError:
        ft = Fixed(digits, scale)
        fixedTypeMap[(digits,scale)] = ft
        return ft

def declaredType(decl, scopedName, kind):
    sname = idlutil.slashName(scopedName)
    try:
        return declaredTypeMap[sname]
    except KeyError:
        dt = Declared(decl, scopedName, kind)
        declaredTypeMap[sname] = dt
        return dt
