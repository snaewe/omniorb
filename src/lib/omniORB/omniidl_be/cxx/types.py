# -*- python -*-
#                           Package   : omniidl
# types.py                  Created on: 2000/4/10
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
#   Type utility functions designed for the C++ backend

import string

from omniidl import idltype, idlast, idlutil
from omniidl_be.cxx import util, config, id

# direction constants
IN     = 0
OUT    = 1
INOUT  = 2
RET    = 3

# we don't support these yet
unsupported_typecodes =[idltype.tk_Principal, idltype.tk_longdouble,
                        idltype.tk_value,
                        idltype.tk_value_box, idltype.tk_native,
                        idltype.tk_abstract_interface,
                        idltype.tk_local_interface]

class Type:
    """Wrapper around an IDL type providing useful extra functionality"""
    def __init__(self, type):
        assert isinstance(type, idltype.Type)
        self.__type = type

    def type(self):
        """type(types.Type): idltype.Type
           returns the wrapped type"""
        return self.__type

    def kind(self):
        """type(types.Type): idltype.kind
           returns the kind of the wrapped type"""
        return self.__type.kind()

    def deref(self, keep_dims = 0):
        """deref(types.Type, keep_dims boolean option) -> types.Type
           Return the type with outer aliases removed.
           (if keep_dims is true then it will not remove aliases with
           array dimensions)"""
        type = self
        while (type.typedef()):
            decl = type.__type.decl()
            if keep_dims and decl.sizes() != []:
                return type
            type = Type(decl.alias().aliasType())

        return type

    def variable(self):
        """variable(types.Type): boolean
           Returns whether the type has a variable length representation
           under the C++ mapping"""
        type = self.__type

        if already_Variable.has_key(type.kind()):
            return already_Variable[type.kind()]

        if isinstance(type, idltype.Declared):
            decl = type.decl()
            return variableDecl(decl)

        util.fatalError("Error while computing the variable-ness of a type")

    def dims(self):
        """dims(types.Type): int list
           Returns the full dimensions of the type"""

        type = self.__type
        if isinstance(type, idltype.Declared):
            decl = type.decl()
            if type.kind() == idltype.tk_alias:
                sizes = []
                if decl.sizes() != None:
                    sizes = decl.sizes()
                if decl.alias() != None:
                    sizes = sizes + Type(decl.alias().aliasType()).dims()
                return sizes
            if isinstance(type.decl(), idlast.Typedef):
                return Type(type.decl().aliasType()).dims()
            return []
        return []

    def array(self):
        """array(types.Type): boolean
           Returns true if this type has array dimensions"""
        return self.dims() != []

    def __apply_mapping(self, (const, ref, ptr), thing):
        # __apply_mapping(types.Type, (const bool, ref bool, ptr bool), string)
        #  : string
        # Make an instance of "thing" which is optionally const, a reference
        # and a pointer types
        text = thing
        if const: text = "const " + text
        if ptr:   text = text + "*"
        if ref:   text = text + "&"
        return text
        
    def _argmapping(self, direction):
        # _argmapping(types.Type, int direction): const * reference * pointer
        #   Returns info on operation argument mapping for a type for
        #   a particular direction.

        # CORBA2.3 P1-204 Table 1-3 Basic argument and result mapping
        array = self.array()
        variable = self.variable()
        if array and not variable:
            # array of fixed size elements
            return ( (1, 0, 0), (0, 0, 0), (0, 0, 0), (0, 0, 1) )[direction]
        if array and variable:
            # array of variable size elements
            return ( (1, 0, 0), (0, 1, 1), (0, 0, 0), (0, 0, 1) )[direction]

        type = self.deref().__type
        kind = type.kind()
        if kind in [ idltype.tk_short, idltype.tk_long, idltype.tk_longlong,
                     idltype.tk_ushort, idltype.tk_ulong, idltype.tk_ulonglong,
                     idltype.tk_float, idltype.tk_double, idltype.tk_enum,
                     idltype.tk_longdouble, idltype.tk_boolean,
                     idltype.tk_char, idltype.tk_wchar, idltype.tk_octet ]:
            # from short to enum the entries are the same
            return ( (0, 0, 0), (0, 1, 0), (0, 1, 0), (0, 0, 0) )[direction]
        if kind in [ idltype.tk_objref, idltype.tk_TypeCode ]:
            # objref_ptr objref_ptr& objref_ptr& objref_ptr
            return ( (0, 0, 0), (0, 1, 0), (0, 1, 0), (0, 0, 0) )[direction]
        if (kind == idltype.tk_struct or kind == idltype.tk_union) and \
           not variable:
            # fixed struct or union
            return ( (1, 1, 0), (0, 1, 0), (0, 1, 0), (0, 0, 0) )[direction]
        if (kind == idltype.tk_struct or kind == idltype.tk_union) and \
           variable:
            # variable struct or union
            return ( (1, 1, 0), (0, 1, 1), (0, 1, 0), (0, 0, 1) )[direction]
        if kind == idltype.tk_string or kind == idltype.tk_wstring:
            return ( (1, 0, 0), (0, 1, 0), (0, 1, 0), (0, 0, 0) )[direction]
        if kind == idltype.tk_sequence or kind == idltype.tk_any:
            return ( (1, 1, 0), (0, 1, 1), (0, 1, 0), (0, 0, 1) )[direction]
        if kind == idltype.tk_fixed:
            return ( (1, 1, 0), (0, 1, 0), (0, 1, 0), (0, 0, 0) )[direction]
        if kind == idltype.tk_value or kind == idltype.tk_value_box:
            return ( (0, 0, 1), (0, 1, 1), (0, 1, 1), (0, 0, 1) )[direction]

        if kind == idltype.tk_void:
            return (0, 0, 0)

        if kind in unsupported_typecodes:
            util.unsupportedIDL()

        util.fatalError("Unknown type encountered (kind = " + str(kind) + ")")
        return

    def op_is_pointer(self, direction):
        if not self.array() and \
           (self.deref().string() or self.deref().wstring()):
            return 0
        return self._argmapping(direction)[2]

    def __var_argmapping(self, direction):
        # __var_argmapping(types.Type, direction): const * reference * pointer
        #  Returns info on argument mapping for a type in a _var
        #  context
        
        # CORBA2.3 P1-204 Table 1-4 T_var argument and result mapping
        kind = self.__type.kind()
        if kind in [ idltype.tk_objref, idltype.tk_struct, idltype.tk_union,
                     idltype.tk_string, idltype.tk_wstring,
                     idltype.tk_sequence, idltype.tk_any,
                     idltype.tk_value, idltype.tk_value_box ] or \
                     self.array():
            return ( (1, 1, 0), (0, 1, 0), (0, 1, 0), (0, 0, 0) )[direction]

        util.fatalError("T_var argmapping requested for type with no such " +\
                        "concept")
        return

    def base(self, environment = None):
        """base(types.Type, id.Environment option): C++ type string
           Returns a basic C++ mapped version of the type"""
        kind = self.kind()
        d_type = self.deref(1)
        d_kind = d_type.kind()

        # CORBA2.3 P1-15 1.5 Mapping for Basic Data Types
        if basic_map.has_key(kind):
            return basic_map[kind]
        if self.string() or d_type.string():
            return "char*"
        if self.wstring() or d_type.wstring():
            return "CORBA::WChar*"
        if self.typecode():
            return "CORBA::TypeCode_ptr"
        if self.any():
            return "CORBA::Any"
        if self.void():
            return "void"
        if self.fixed():
            return "CORBA::Fixed"
        if self.sequence():
            return self.sequenceTemplate(environment)

        name = id.Name(self.type().scopedName()).unambiguous(environment)
        if d_type.objref() or d_type.typecode():
            return name + "_ptr"
        else:
            return name

    def __base_type_OUT(self, environment = None):
        # ___base_type_OUT(types.Type, id.Environment option): special C++
        #  OUT type
        type = self.__type
        d_type = self.deref()
        d_kind = type.kind()
        if basic_map_out.has_key(d_kind):
            return basic_map_out[d_kind]
        if d_type.string():
            return "CORBA::String_out"
        if d_type.wstring():
            return "CORBA::WString_out"
        if d_type.typecode():
            return "CORBA::TypeCode_OUT_arg"
        if d_type.any():
            return "CORBA::Any_OUT_arg"
        if d_type.sequence():
            return Type(d_type.type().seqType()).__base_type_OUT(environment)

        name = id.Name(type.scopedName())
        uname = name.unambiguous(environment)

        if d_type.objref():
            name = id.Name(d_type.type().scopedName())
            objref_name = name.prefix("_objref_")
            uname = name.unambiguous(environment)
            
            if d_type.__type.scopedName() == ["CORBA", "Object"]:
                return "CORBA::Object_OUT_arg"

            return "_CORBA_ObjRef_OUT_arg< " + objref_name.fullyQualify() + \
                   ", " + name.unambiguous(environment) + "_Helper >"

        return uname + "_out"

    def __base_type_INOUT(self, environment = None):
        # __base_type_INOUT(types.Type): special C++ INOUT type string
        
        d_type = self.deref()
        kind = d_type.kind()
        if d_type.string():
            return "CORBA::String_INOUT_arg"
        if d_type.wstring():
            return "CORBA::WString_INOUT_arg"
        if d_type.typecode():
            return "CORBA::TypeCode_INOUT_arg"
        if d_type.any():
            return "CORBA::Any_INOUT_arg"

        name = id.Name(self.type().scopedName())
        uname = name.unambiguous(environment)
        
        if d_type.objref():
            name = id.Name(d_type.type().scopedName())
            objref_name = name.prefix("_objref_")
            uname = name.unambiguous(environment)
            if d_type.type().scopedName() == ["CORBA", "Object"]:
                return "CORBA::Object_INOUT_arg"

            return "_CORBA_ObjRef_INOUT_arg< " + objref_name.fullyQualify() + \
                   ", " + name.unambiguous(environment) + "_Helper >"
        return uname + "_INOUT_arg"

    def op(self, direction, environment = None, use_out = 1):
        """op(types.Type, int direction, id.Environment option, use_out bool)
           : C++ type argument mapping"""
        type = Type(self.__type)
        d_type = type.deref()

        base = self.base(environment)

        old_sig = config.state['Old Signatures']

        # this is very superfluous:
        if not self.array():
            if d_type.any() and self.typedef() and not old_sig:
                if direction == OUT:
                    return d_type.__base_type_OUT(environment)


            if d_type.typecode() and not old_sig:
                if direction == OUT:
                    return d_type.__base_type_OUT(environment)
                elif direction == INOUT and use_out:
                    return d_type.__base_type_INOUT(environment)
                else:
                    base = d_type.base(environment)


        # Use the ObjRef template for non-arrays of objrefs rather than use
        # the (equivalent?) _out type?
        if not d_type.objref() or type.array():

            # if its an out type and a typedef (to a variable type),
            # use the predefined _out type
            if type.typedef() and type.variable() and direction == OUT:

                if (not d_type.string() or not d_type.wstring() or \
                    (d_type.string() and type.array()) or \
                    (d_type.wstring() and type.array())):

                    base = id.Name(type.__type.scopedName()).unambiguous(environment)
                    base = base + "_out"
                    return base
        # superfluous deref for a typedef to an objref
        if d_type.objref() and not type.array():
            base = d_type.base(environment)

        # Deal with special cases ----------------------------------
        if not self.array():

            if direction == OUT and not use_out:
                if d_type.string() and not old_sig:
                    return self.__base_type_OUT(environment)
                if d_type.wstring() and not old_sig:
                    return self.__base_type_OUT(environment)
                if d_type.objref() and not old_sig:
                    return self.__base_type_OUT(environment)
                if d_type.typecode() and not old_sig:
                    return self.__base_type_OUT(environment)
                if d_type.any() and not old_sig:
                    return self.__base_type_OUT(environment)
                
            if direction == OUT:
                if d_type.string():
                    return self.__base_type_OUT(environment)
                if d_type.wstring():
                    return self.__base_type_OUT(environment)
                if d_type.typecode():
                    return self.__base_type_OUT(environment)
                if d_type.objref():
                    return self.__base_type_OUT(environment)
                if d_type.any() and (not old_sig or use_out):
                    return self.__base_type_OUT(environment)
                if d_type.variable() and (not old_sig or use_out):
                    return self.__base_type_OUT(environment)

            if direction == INOUT:
                if d_type.string() and use_out:
                    return self.__base_type_INOUT(environment)
                if d_type.wstring() and use_out:
                    return self.__base_type_INOUT(environment)
                if d_type.typecode() and use_out:
                    return self.__base_type_INOUT(environment)
                if d_type.objref() and use_out:
                    return self.__base_type_INOUT(environment)
                

        if d_type.string() and not type.array():
            base = d_type.base(environment)

        if d_type.wstring() and not type.array():
            base = d_type.base(environment)
    
        # P1-104 mentions two cases: returning an array and a variable
        # array out argument. For the latter rely on the _out type
        if (type.array() and direction == RET):
            base = base + "_slice"
            
        mapping = self._argmapping(direction)
        
        return self.__apply_mapping(mapping, base)

    def member(self, environment = None, decl = None):
        """member(types.Type, id.Environment option, idlast.Declarator option):
           C++ member type"""
        decl_dims = []
        if decl != None:
            assert isinstance(decl, idlast.Declarator)
            decl_dims = decl.sizes()

        is_array_declarator = decl_dims != []

        if not self.array():
            d_type = self.deref()
            if d_type.string():
                return "CORBA::String_member"
            if d_type.wstring():
                return "CORBA::WString_member"
            if d_type.objref():
                return d_type.objRefTemplate("Member", environment)
            if d_type.typecode():
                return "CORBA::TypeCode_member"

            if self.sequence():
                return d_type.sequenceTemplate(environment)
            
        if self.typedef():
            # for the type to have dimensions, it must be a typedef
            return id.Name(self.__type.scopedName()).unambiguous(environment)

        return self.base(environment)

    def objRefTemplate(self, suffix, environment = None):
        """objRefTemplate(types.Type, suffix string, id.Environment option):
           Returns a template objref instance for the current type"""
        type = self.deref().__type
        name = type.decl().scopedName()
        if name == ["CORBA", "Object"]:
            return "CORBA::Object_" + suffix

        name = id.Name(name)
        uname = name.unambiguous(environment)
        objref_name = name.prefix("_objref_")
        objref_uname = objref_name.unambiguous(environment)
        return "_CORBA_ObjRef_" + suffix + \
               "< " + objref_uname + ", " + uname + "_Helper> "

    def literal(self, value, environment = None):
        """literal(types.Type, value any, id.Environment option): string
           Returns a C++ representation of a value"""

        type = self.deref()
        kind = type.__type.kind()

        # (unsigned) short ints are themselves
        if kind in [ idltype.tk_short, idltype.tk_ushort ]:
            return str(value)

        # careful with long ints to avoid "L" postfix
        if kind in [ idltype.tk_long, idltype.tk_ulong ]:
            s = str(value)
            if s[-1] == 'L':             s = s[0:-1]
            if kind == idltype.tk_ulong: s = s + "U"
            return s

        if kind in [ idltype.tk_longlong, idltype.tk_ulonglong ]:
            s = str(value)
            if s[-1] == 'L':                 s = s[:-1]
            if kind == idltype.tk_ulonglong: s = s + "U"
            return "_CORBA_LONGLONG_CONST(" + s + ")"

        if kind in [ idltype.tk_float ]:
            return idlutil.reprFloat(value) + "F"

        if kind in [ idltype.tk_double ]:
            return idlutil.reprFloat(value)

        if kind in [ idltype.tk_longdouble ]:
            return idlutil.reprFloat(value) + "L"

        # chars are single-quoted
        if kind in [ idltype.tk_char ]:
            return "'" + idlutil.escapifyString(value) + "'"

        if kind in [ idltype.tk_wchar ]:
            return "L'" + idlutil.escapifyWString([value]) + "'"
        
        # booleans are straightforward
        if kind in [ idltype.tk_boolean ]:
            return str(value)

        if kind in [ idltype.tk_enum ]:
            # value is an enumerator
            enum_name = id.Name(value.scopedName())
            #enum_name = id.Name(type.__type.decl().scopedName() + [str(value)])
            return enum_name.unambiguous(environment)

        if kind in [ idltype.tk_string ]:
            return '"' + idlutil.escapifyString(value) + '"'

        if kind in [ idltype.tk_wstring ]:
            return 'L"' + idlutil.escapifyWString(value) + '"'

        if kind in [ idltype.tk_octet ]:
            return str(value)

        if kind in [ idltype.tk_fixed ]:
            return '"' + value + '"'

        util.fatalError("Internal error when handling value (" +\
                        repr(value) +")" )

    def sequenceTemplate(self, environment = None):
        """sequenceTemplate(types.Type, id.Environment option): C++ template
           Returns a C++ template instance for the current type as a
           sequence"""
        # returns a template instantiation suitable for the
        # sequence type
        # (similar in function to o2be_sequence::seq_template_name)
        sequence = self.__type
        assert isinstance(sequence, idltype.Sequence)
        
        SeqType = Type(sequence.seqType())
        d_SeqType = SeqType.deref()
        SeqTypeID = SeqType.base(environment)
        d_SeqTypeID = d_SeqType.base(environment)
        if d_SeqType.typecode():
            d_SeqTypeID = "CORBA::TypeCode_member"
            SeqTypeID = "CORBA::TypeCode_member"
        elif d_SeqType.objref():
            d_SeqTypeID = string.replace(d_SeqTypeID,"_ptr","")
            SeqTypeID = string.replace(SeqTypeID,"_ptr","")
        elif d_SeqType.string():
            d_SeqTypeID = "CORBA::String_member"
        elif d_SeqType.wstring():
            d_SeqTypeID = "CORBA::WString_member"
        
        if SeqType.string():
            SeqTypeID = "CORBA::String_member"

        if SeqType.wstring():
            SeqTypeID = "CORBA::WString_member"

        # silly special case (not needed?):
        #if d_SeqType.objref() and SeqType.typedef():
        #    SeqTypeID = id.Name(SeqType.type().scopedName()).\
        #                unambiguous(environment)

        seq_dims = SeqType.dims()
        is_array = seq_dims != []
        dimension = reduce(lambda x,y: x * y, seq_dims, 1)

        template = {}
        template["bounded"]   = sequence.bound()
        template["array"]     = is_array
        template["dimension"] = dimension
        
        template["seqTypeID"] = SeqTypeID
        template["derefSeqTypeID"] = d_SeqTypeID

        # if the seqType is a typedef to a sequence, use the typedef name
        # else if a direct sequence<sequence<...., do recursion
        if d_SeqType.sequence() and not SeqType.typedef():
            element_template = d_SeqType.sequenceTemplate(environment)
            template["seqTypeID"] = element_template
            template["derefSeqTypeID"] = element_template

        if is_array:
            if d_SeqType.sequence():
                template["derefSeqTypeID"] = d_SeqType.\
                                             sequenceTemplate(environment)
        
        if d_SeqType.char():
            template["suffix"] = "_Char"
        elif d_SeqType.wchar():
            template["suffix"] = "_WChar"
        elif d_SeqType.boolean():
            template["suffix"] = "_Boolean"
        elif d_SeqType.octet():
            template["suffix"] = "_Octet"
            # strings are always special
        elif d_SeqType.string() and not is_array:
            template["suffix"] = "_String"
        elif d_SeqType.wstring() and not is_array:
            template["suffix"] = "_WString"
        elif d_SeqType.structforward() or d_SeqType.unionforward():
            template["forward"] = 1
        elif typeSizeAlignMap.has_key(d_SeqType.type().kind()):
            template["fixed"] = typeSizeAlignMap[d_SeqType.type().\
                                                        kind()]
        elif d_SeqType.objref():
            scopedName = d_SeqType.type().decl().scopedName()
            is_CORBA_Object = scopedName == ["CORBA", "Object"]
            scopedName = id.Name(scopedName)
            
            if not is_CORBA_Object:
                # CORBA::Object doesn't have an _objref_xxx
                scopedName = scopedName.prefix("_objref_")
           
            objref_name = scopedName.unambiguous(environment)

            if not is_array:
                objref_template = d_SeqType.objRefTemplate("Element", environment)
            else:
                objref_template = d_SeqType.objRefTemplate("Member", environment)
            template["objref_name"]     = objref_name
            template["objref_template"] = objref_template
            template["objref_helper"]   = SeqTypeID + "_Helper"
            template["objref"]          = 1
        return self.__templateToString(template)

    # converts a hash of template properties into a template instance
    def __templateToString(self, template):
        # ------------------------------------
        # work out the template name
        if template["bounded"]:
            name = "_CORBA_Bounded_Sequence"
        else:
            name = "_CORBA_Unbounded_Sequence"

        if template["array"]:
            name = name + "_Array"
        
        if template.has_key("suffix"):
            name = name + template["suffix"]

        if template.has_key("forward"):
            name = name + "_Forward"

        elif template.has_key("objref") and not template["array"]:
            name = name + "_ObjRef"

        if template.has_key("fixed"):
            name = name + "_w_FixSizeElement"

        # ------------------------------------
        # build the argument list
        args = []

        seqTypeID      = template["seqTypeID"]
        derefSeqTypeID = template["derefSeqTypeID"]
        dimension      = template["dimension"]

        # Note the difference between an ObjRef and an array of ObjRefs
        if template["array"]:
            args.extend([seqTypeID, seqTypeID + "_slice"])
        
            if template.has_key("objref"):
                args.append(template["objref_template"])

            elif not template.has_key("suffix"):
                # __Boolean __Octet __String
                # these already contain the type info- no need for another
                # parameter...
                args.append(derefSeqTypeID)
                
            args.append(str(dimension))
        
        elif template.has_key("objref"):
            args.extend([template["objref_name"],
                         template["objref_template"],
                         template["objref_helper"]])
        elif not template.has_key("suffix"):
            # see above
            args.append(seqTypeID)
        
        if template.has_key("bounded") and \
           template["bounded"]:
            args.append(str(template["bounded"]))

        if template.has_key("fixed"):
            (element_size, alignment) = template["fixed"]
            args.extend([str(element_size), str(alignment)])

        # -----------------------------------
        # build the template instance
        if args:
            name = name + "< " + string.join(args, ", ") + " > "
            return name

        return name

    def _var(self, environment = None):
        """Returns a representation of the type which is responsible for its
           own destruction. Assigning a heap allocated thing to this type
           should allow the user to forget about deallocation."""
        d_T = self.deref()

        if self.array() or d_T.struct()    or d_T.union() or \
                           d_T.exception() or d_T.sequence() or \
                           d_T.objref():
            name = id.Name(self.type().decl().scopedName()).suffix("_var")
            return name.unambiguous(environment)

        if d_T.typecode(): return "CORBA::TypeCode_var"
        if d_T.any():      return "CORBA::Any_var"
        if d_T.string():   return "CORBA::String_var"
        if d_T.wstring():  return "CORBA::WString_var"
        if d_T.enum():
            name = id.Name(self.type().decl().scopedName())
            return name.unambiguous(environment)
        
        if self.is_basic_data_types():
            return basic_map[d_T.kind()]
        
        if d_T.void():     raise "No such thing as a void _var type"

        raise "Unknown _var type, kind = " + str(d_T.kind())

    def out(self, ident):
        if self.is_basic_data_types():
            return ident
        
        return ident + ".out()"

    def free(self, thing, environment = None):
        """Ensures that any heap allocated storage associated with this type
           has been deallocated."""

        if self.array():
            name = id.Name(self.type().decl().scopedName()).suffix("_free")
            return name.unambiguous(environment) + "(" + thing + ");"

        d_T = self.deref()

        if d_T.objref() or d_T.typecode():
            return "CORBA::release(" + thing + ");"
        if d_T.string():   return "CORBA::string_free(" + thing + ");"

        if d_T.wstring():   return "CORBA::wstring_free(" + thing + ");"

        if d_T.struct() or d_T.union() or d_T.exception() or \
           d_T.sequence() or d_T.any():
            if d_T.variable():
                return "delete " + thing + ";"
            return "" # stored by value

        if d_T.enum() or d_T.void() or (self.is_basic_data_types()):
            return ""

        raise "Don't know how to free type, kind = " + str(d_T.kind())

    def copy(self, src, dest, environment = None):
        """Copies an entity from src to dest"""

        if self.array():
            name = id.Name(self.type().decl().scopedName()).suffix("_dup")
            return dest + " = " + name.unambiguous(environment) + "("+src+");"

        d_T = self.deref()
        if d_T.typecode():
            return dest + " = CORBA::TypeCode::_duplicate(" + src + ");"
        if d_T.objref():
            # Use the internal omniORB duplicate function in case the
            # normal one isn't available
            name = id.Name(self.type().decl().scopedName()).suffix("_Helper")
            return name.unambiguous(environment) + "::duplicate" +\
                   "(" + src + ");\n" + dest + " = " + src + ";"
        if d_T.string():
            return dest + " = CORBA::string_dup(" + src + ");"
        if d_T.wstring():
            return dest + " = CORBA::wstring_dup(" + src + ");"
        if d_T.any():
            return dest + " = new CORBA::Any(" + src + ");"
        
        if d_T.struct() or d_T.union() or d_T.exception() or d_T.sequence():
            name = id.Name(self.type().decl().scopedName()).\
                   unambiguous(environment)
            if d_T.variable():
                return dest + " = new " + name + "(" + src + ");"
            return dest + " = " + src + ";"
        
        if d_T.enum() or self.is_basic_data_types():
            return dest + " = " + src + ";"

        raise "Don't know how to free type, kind = " + str(d_T.kind())
               
    def representable_by_int(self):
        """representable_by_int(types.Type): boolean
           Returns true if the type is representable by an integer"""
        return self.integer() or self.char() or self.boolean() or self.octet()

    def is_basic_data_types(self):
        d_T = self.deref()
        return d_T.kind() in basic_map.keys()
        
    def integer(self):
        type = self.__type
        return type.kind() in [ idltype.tk_short, idltype.tk_long,
                                idltype.tk_longlong, idltype.tk_ushort,
                                idltype.tk_ulong, idltype.tk_ulonglong ]
    def char(self):
        type = self.__type
        return type.kind() == idltype.tk_char

    def wchar(self):
        type = self.__type
        return type.kind() == idltype.tk_wchar
        
    def floating(self):
        type = self.__type
        return type.kind() in [ idltype.tk_float, idltype.tk_double ]

    def float(self):
        type = self.__type
        return type.kind() == idltype.tk_float

    def double(self):
        type = self.__type
        return type.kind() == idltype.tk_double    

    def boolean(self):
        type = self.__type
        return type.kind() == idltype.tk_boolean

    def enum(self):
        type = self.__type
        return type.kind() == idltype.tk_enum

    def octet(self):
        type = self.__type
        return type.kind() == idltype.tk_octet

    def string(self):
        type = self.__type
        return type.kind() == idltype.tk_string

    def wstring(self):
        type = self.__type
        return type.kind() == idltype.tk_wstring

    def objref(self):
        type = self.__type
        return type.kind() == idltype.tk_objref

    def sequence(self):
        type = self.__type
        return type.kind() == idltype.tk_sequence

    def typecode(self):
        type = self.__type
        return type.kind() == idltype.tk_TypeCode

    def typedef(self):
        type = self.__type
        return type.kind() == idltype.tk_alias

    def struct(self):
        type = self.__type
        return type.kind() == idltype.tk_struct

    def structforward(self):
        type = self.__type
        return type.kind() == idltype.ot_structforward

    def union(self):
        type = self.__type
        return type.kind() == idltype.tk_union

    def unionforward(self):
        type = self.__type
        return type.kind() == idltype.ot_unionforward

    def exception(self):
        type = self.__type
        return type.kind() == idltype.tk_except
    
    def void(self):
        type = self.__type
        return type.kind() == idltype.tk_void

    def any(self):
        type = self.__type
        return type.kind() == idltype.tk_any

    def fixed(self):
        type = self.__type
        return type.kind() == idltype.tk_fixed


def variableDecl(decl):
    """types.variableDecl(idlast.Decl): boolean
        Returns true if the declaration represents a variable type"""
    # interfaces are mapped to objects, which are always
    # variable types. same goes for exceptions.
    if isinstance(decl, idlast.Interface)       or \
       isinstance(decl, idlast.Forward)         or \
       isinstance(decl, idlast.Exception):
        return 1
    elif isinstance(decl, idlast.Const)         or \
         isinstance(decl, idlast.Enum):
        return 0
    
    # a typedef is only a type alias- as such it has no storage
    # at all. However it eventually points to something that would.
    elif isinstance(decl, idlast.Typedef):
        return Type(decl.aliasType()).variable()
    
    # a structure is variable if any one of its constituents
    # is also variable
    elif isinstance(decl, idlast.Struct):
        for m in decl.members():
            if Type(m.memberType()).variable():
                return 1
        return 0
        
    # a union is variable if any one if its constituents
    # is also variable
    elif isinstance(decl, idlast.Union):
        for c in decl.cases():
            if Type(c.caseType()).variable():
                return 1
        return 0

    # a declarator is variable if it is an alias to a variable
    # type
    elif isinstance(decl, idlast.Declarator) and \
         decl.alias() != None:
        return Type(decl.alias().aliasType()).variable()

    util.fatalError("Unknown AST node, scopedName = " +repr(decl.scopedName()))




def direction(param):
    if param.is_in() and param.is_out():
        return INOUT
    elif param.is_in():
        return IN
    elif param.is_out():
        return OUT

    # Top 12 things likely to be overheard from a Klingon Programmer: 
    # ...
    #
    #   7) "Klingon function calls do not have 'parameters' - they
    #       have 'arguments' - and they ALWAYS WIN THEM."
    # ...

    util.fatalError("Illegal parameter direction")


#################################################################
# Tables of useful data ripped from the CORBA spec

# already_Variable maps typecode kinds onto true/ false
# 
# An entry in this table indicates we already know is a type is
# variable or not, without having to look at its declaration.
# (note that eg structs and unions are only variable if one of
#  their members are)

# CORBA2.3 P1-21 1.9 Mapping for Structured Types
already_Variable = {
    idltype.tk_null:               0,
    idltype.tk_void:               0,
    idltype.tk_short:              0,
    idltype.tk_long:               0,
    idltype.tk_ushort:             0,
    idltype.tk_ulong:              0,
    idltype.tk_float:              0,
    idltype.tk_double:             0,
    idltype.tk_boolean:            0,
    idltype.tk_char:               0,
    idltype.tk_octet:              0,
    idltype.tk_any:                1,
    idltype.tk_objref:             1,
    idltype.tk_string:             1,
    idltype.tk_sequence:           1,
    idltype.tk_except:             1,
    idltype.tk_longlong:           0,
    idltype.tk_ulonglong:          0,
    idltype.tk_longdouble:         0,
    idltype.tk_wchar:              0,
    idltype.tk_wstring:            1,
    idltype.tk_fixed:              0,          
    idltype.tk_value:              1,      
    idltype.tk_value_box:          1,      
    idltype.tk_abstract_interface: 1,
    idltype.tk_any:                1,
    idltype.tk_TypeCode:           1
    }

# CORBA2.3 P1-15 1.5 Mapping for Basic Data Types
basic_map = {
    idltype.tk_short:              "CORBA::Short",
    idltype.tk_long:               "CORBA::Long",
    idltype.tk_longlong:           "CORBA::LongLong",
    idltype.tk_ushort:             "CORBA::UShort",
    idltype.tk_ulong:              "CORBA::ULong",
    idltype.tk_ulonglong:          "CORBA::ULongLong",
    idltype.tk_float:              "CORBA::Float",
    idltype.tk_double:             "CORBA::Double",
    idltype.tk_longdouble:         "CORBA::LongDouble",
    idltype.tk_char:               "CORBA::Char",
    idltype.tk_wchar:              "CORBA::WChar",
    idltype.tk_boolean:            "CORBA::Boolean",
    idltype.tk_octet:              "CORBA::Octet"
    }
basic_map_out = { }
for key,value in basic_map.items():
    basic_map_out[key] = value + "_out"


# Info on size and alignment of basic types
typeSizeAlignMap = {
    idltype.tk_char:      (1, 1),
    idltype.tk_boolean:   (1, 1),
    idltype.tk_wchar:     (2, 2),
    idltype.tk_short:     (2, 2),
    idltype.tk_ushort:    (2, 2),
    idltype.tk_long:      (4, 4),
    idltype.tk_ulong:     (4, 4),
    idltype.tk_float:     (4, 4),
    idltype.tk_enum:      (4, 4),
    idltype.tk_double:    (8, 8),
    idltype.tk_octet:     (1, 1),
    idltype.tk_longlong:  (8, 8),
    idltype.tk_ulonglong: (8, 8)
    }
