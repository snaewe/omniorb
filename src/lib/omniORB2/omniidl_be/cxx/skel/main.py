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
# Revision 1.30.2.2  2000/08/21 11:35:32  djs
# Lots of tidying
#
# Revision 1.30.2.1  2000/08/02 10:52:02  dpg1
# New omni3_1_develop branch, merged from omni3_develop.
#
# Revision 1.30  2000/07/13 15:25:59  dpg1
# Merge from omni3_develop for 3.0 release.
#
# Revision 1.27.2.12  2000/07/24 16:32:18  djs
# Fixed typo in previous BOA skeleton bugfix.
# Suppressed compiler warning (from gcc -Wall) when encountering a call with
# no arguments and no return value.
#
# Revision 1.27.2.11  2000/06/26 16:24:17  djs
# Refactoring of configuration state mechanism.
#
# Revision 1.27.2.10  2000/06/06 14:45:07  djs
# Produces flattened name typedefs for _all_ inherited interfaces (not just
# those which are immediate decendents) in SK.cc
#
# Revision 1.27.2.9  2000/06/05 13:04:18  djs
# Removed union member name clash (x & pd_x, pd__default, pd__d)
# Removed name clash when a sequence is called "pd_seq"
#
# Revision 1.27.2.8  2000/05/31 18:03:38  djs
# Better output indenting (and preprocessor directives now correctly output at
# the beginning of lines)
# Calling an exception "e" resulted in a name clash (and resultant C++
# compile failure)
#
# Revision 1.27.2.7  2000/05/05 16:50:51  djs
# Existing workaround for MSVC5 scoping problems extended to help with
# base class initialisers. Instead of using the fully qualified or unambiguous
# name, a flat typedef is generated at global scope and that is used instead.
# This was a solution to a previous bug wrt operation dispatch()ing.
# This does not affect the OMNI_BASE_CTOR powerpc/aix workaround.
#
# Revision 1.27.2.6  2000/04/26 18:22:54  djs
# Rewrote type mapping code (now in types.py)
# Rewrote identifier handling code (now in id.py)
# Removed superfluous externs in front of function definitions
#
# Revision 1.27.2.5  2000/04/05 10:58:02  djs
# Scoping problem with generated proxies for attributes (not operations)
#
# Revision 1.27.2.4  2000/03/20 11:50:26  djs
# Removed excess buffering- output templates have code attached which is
# lazily evaluated when required.
#
# Revision 1.27.2.3  2000/02/16 16:30:02  djs
# Fix to proxy call descriptor code- failed to handle special case of
#   Object method(in string x)
#
# Revision 1.27.2.2  2000/02/15 15:28:35  djs
# Stupid bug in powerpc aix workaround fixed
#
# Revision 1.27.2.1  2000/02/14 18:34:53  dpg1
# New omniidl merged in.
#
# Revision 1.27  2000/02/01 09:26:49  djs
# Tracking fixes in old compiler: powerpc-aix scoped identifier workarounds
#
# Revision 1.26  2000/01/19 17:05:15  djs
# Modified to use an externally stored C++ output template.
#
# Revision 1.25  2000/01/19 11:21:52  djs
# *** empty log message ***
#
# Revision 1.24  2000/01/17 17:05:38  djs
# Marshalling and constructed types fixes
#
# Revision 1.23  2000/01/14 15:57:20  djs
# Added MSVC workaround for interface inheritance
#
# Revision 1.22  2000/01/13 17:02:05  djs
# Added support for operation contexts.
#
# Revision 1.21  2000/01/13 15:56:43  djs
# Factored out private identifier prefix rather than hard coding it all through
# the code.
#
# Revision 1.20  2000/01/13 14:16:34  djs
# Properly clears state between processing separate IDL input files
#
# Revision 1.19  2000/01/12 17:48:34  djs
# Added option to create BOA compatible skeletons (same as -BBOA in omniidl3)
#
# Revision 1.18  2000/01/11 14:13:23  djs
# Updated array mapping to include NAME_copy(to, from) as per 2.3 spec
#
# Revision 1.17  2000/01/11 12:02:45  djs
# More tidying up
#
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
from omniidl_be.cxx import cxx, ast, output, id, config, skutil, types, iface, call
from omniidl_be.cxx.skel import mangler, template

import main
self = main


def __init__(stream):
    self.stream = stream

    # To keep track of our depth with the AST
    self.__insideInterface = 0
    self.__insideModule = 0
    return self

# ------------------------------------
# Control arrives here

def visitAST(node):
    for n in node.declarations():
        # Not sure what should happen when modules are reopened
        if n.mainFile():
            n.accept(self)

def visitModule(node):
    insideModule = self.__insideModule
    self.__insideModule = 1
    
    for n in node.definitions():
        n.accept(self)

    self.__insideModule = insideModule


def visitInterface(node):
    ident = node.identifier()

    outer_environment = id.lookup(node)
    environment = outer_environment.enter(ident)

    insideInterface = self.__insideInterface
    self.__insideInterface = 1

    # produce skeletons for types declared here
    for n in node.declarations():
        n.accept(self)

    self.__insideInterface = insideInterface

    # Call descriptor names are of the form:
    #  TAG _ PREFIX _ BASE
    # Tag represents the type of thing {call descriptor, local callback...}
    # Prefix is derived from the first encountered scopedname[1]
    # Base is a counter to uniquify the identifier
    #
    # [1] Since the names are guaranteed unique, the prefix makes the
    #     names used by two different modules disjoint too. Not sure why
    #     as they are not externally visible?
    operations = filter(lambda x:isinstance(x, idlast.Operation),
                        node.callables())
    attributes = filter(lambda x:isinstance(x, idlast.Attribute),
                        node.callables())
    if operations != []:
        scopedName = node.scopedName() + [operations[0].identifier()]
        mangler.initialise_base(scopedName)
    if attributes != []:
        scopedName = node.scopedName() + [attributes[0].identifiers()[0]]
        mangler.initialise_base(scopedName)

    I = iface.Interface(node)
    I_Helper = iface.instance("I_Helper")(I)
    I_Helper.cc(stream)


    # the class itself
    node_name = id.Name(node.scopedName())
    objref_name = node_name.prefix("_objref_")
    stream.out(template.interface_class,
               name = node_name.fullyQualify(),
               objref_name = objref_name.unambiguous(environment),
               repoID = node.repoId())

    # Output flattened aliases to inherited classes, to workaround an
    # MSVC bug.
    for i in ast.allInherits(node):
        inherits_name = id.Name(i.scopedName())
        if inherits_name.needFlatName(environment):
            guard_name = inherits_name.guard()
            flat_fqname = inherits_name.flatName()
            inherits_impl_name = inherits_name.prefix("_impl_")
            inherits_objref_name = inherits_name.prefix("_objref_")

            impl_flat_fqname = inherits_impl_name.flatName()
            objref_flat_fqname = inherits_objref_name.flatName()
            
            stream.out(template.interface_ALIAS,
                       guard_name = guard_name,
                       fqname = inherits_name.fullyQualify(),
                       flat_fqname = flat_fqname,
                       impl_fqname = inherits_impl_name.fullyQualify(),
                       impl_flat_fqname = impl_flat_fqname,
                       objref_fqname = inherits_objref_name.fullyQualify(),
                       objref_flat_fqname = objref_flat_fqname)

    _objref_I = iface.instance("_objref_I")(I)
    _objref_I.cc(stream)
 
    _pof_I = iface.instance("_pof_I")(I)
    _pof_I.cc(stream)

    _impl_I = iface.instance("_impl_I")(I)
    _impl_I.cc(stream)

    
    # BOA compatible skeletons
    if config.state['BOA Skeletons']:
        sk_name = node_name.prefix("_sk_")
        stream.out(template.interface_sk,
                   sk_fqname = sk_name.fullyQualify(),
                   sk_name = sk_name.unambiguous(environment))


def visitTypedef(node):
    environment = id.lookup(node)
    is_global_scope = not(self.__insideModule or self.__insideInterface)

    aliasType = types.Type(node.aliasType())

    if node.constrType():
        aliasType.type().decl().accept(self)

    fq_aliased = aliasType.base(environment)

    for d in node.declarators():
        scopedName = id.Name(d.scopedName())
        
        decl_dims = d.sizes()
        decl_dims_str = cxx.dimsToString(decl_dims)
        decl_first_dim_str = ""
        if decl_dims != []:
            decl_first_dim_str = cxx.dimsToString([decl_dims[0]])
        
        full_dims = decl_dims + aliasType.dims()
        is_array = full_dims != []
        is_array_declarator = decl_dims != []

        fq_derived = scopedName.fullyQualify()

        if is_global_scope and is_array_declarator:
            # build _dup and _copy loops
            dup_loop = output.StringStream()
            loop = cxx.For(dup_loop, full_dims)
            index = loop.index()
            dup_loop.out("_data" + index + " = _s" + index + ";")
            loop.end()

            copy_loop = output.StringStream()
            loop = cxx.For(copy_loop, full_dims)
            index = loop.index()
            copy_loop.out("_to" + index + " = _from" + index + ";")
            loop.end()

            stream.out(template.typedef_global_array_declarator,
                       fq_derived = fq_derived,
                       decl_dims_str = decl_dims_str,
                       decl_first_dim_str = decl_first_dim_str,
                       dup_loop = str(dup_loop),
                       copy_loop = str(copy_loop))

        elif is_global_scope and is_array:
            stream.out(template.typedef_global_simple_array,
                       fq_derived = fq_derived,
                       fq_aliased = fq_aliased)
            
    
    pass

def visitEnum(node):
    return

def visitMember(node):
    memberType = node.memberType()
    if node.constrType():
        memberType.decl().accept(self)
        
def visitStruct(node):

    outer_environment = id.lookup(node)
    scopedName = id.Name(node.scopedName())

    size_calculation = "omni::align_to(_msgsize, omni::ALIGN_4) + 4"

    for n in node.members():
        n.accept(self)

    def marshal(stream = stream, node = node,
                outer_env = outer_environment):
        for n in node.members():
            memberType = types.Type(n.memberType())
            for d in n.declarators():
                scopedName = id.Name(d.scopedName())
                member_name = scopedName.simple()
                skutil.marshall_struct_union(stream, outer_env,
                                             memberType, d, member_name)
        return

    def Mem_unmarshal(stream = stream, node = node,
                      outer_env = outer_environment):
        for n in node.members():
            memberType = types.Type(n.memberType())
            for d in n.declarators():
                scopedName = id.Name(d.scopedName())
                member_name = scopedName.simple()
                skutil.unmarshall_struct_union(stream, outer_env,
                                               memberType, d, member_name, 0)
        return

    def Net_unmarshal(stream = stream, node = node,
                      outer_env = outer_environment):
        for n in node.members():
            memberType = types.Type(n.memberType())
            for d in n.declarators():
                scopedName = id.Name(d.scopedName())
                member_name = scopedName.simple()
                skutil.unmarshall_struct_union(stream, outer_env,
                                               memberType, d, member_name, 1)
        return

    def msgsize(stream = stream, node = node, outer_env = outer_environment):
        for n in node.members():
            memberType = types.Type(n.memberType())
            for d in n.declarators():
                scopedName = id.Name(d.scopedName())
                member_name = scopedName.simple()
                size = skutil.sizeCalculation(outer_env, memberType, d,
                                              "_msgsize", member_name)
                stream.out(size)
        return

    stream.out(template.struct,
               name = scopedName.fullyQualify(),
               size_calculation = msgsize,
               marshall_code = marshal,
               mem_unmarshall_code = Mem_unmarshal,
               net_unmarshall_code = Net_unmarshal)

    stream.reset_indent()
    
def visitUnion(node):
    outer_environment = id.lookup(node)
    environment = outer_environment.enter(node.identifier())

    scopedName = id.Name(node.scopedName())
    name = scopedName.fullyQualify()

    switchType = types.Type(node.switchType())

    exhaustive = ast.exhaustiveMatch(switchType, ast.allCaseLabelValues(node))
    defaultCase = ast.defaultCase(node)
    ast.markDefaultCase(node)

    defaultMember = ""
    if defaultCase:
        defaultLabel = ast.defaultLabel(defaultCase)
        default_scopedName = id.Name(defaultCase.declarator().scopedName())
        defaultMember = default_scopedName.simple()
        
    hasDefault = defaultCase != None

    # Booleans are a special case (isn't everything?)
    booleanWrap = switchType.boolean() and exhaustive


    # deal with types constructed here
    if node.constrType():
        node.switchType().decl().accept(self)
    for n in node.cases():
        if n.constrType():
            n.caseType().decl().accept(self)

    # --------------------------------------------------------------
    # union::_NP_alignedSize(size_t initialoffset) const
    #
    discriminator_size_calc = skutil.sizeCalculation(environment,
                                                     switchType,
                                                     None,
                                                     "_msgsize", "")

    # build the switch case in the alignedSize method
    # build the cases...
    def cases(stream = stream, node = node, switchType = switchType,
              environment = environment, booleanWrap = booleanWrap):
        for c in node.cases():
            caseType = types.Type(c.caseType())
            d_caseType = caseType.deref()
            decl = c.declarator()
            decl_scopedName = id.Name(decl.scopedName())
            decl_name = decl_scopedName.simple()
            for l in c.labels():
                # default case was already taken care of
                if not(l.default()):
                    discrim_value = switchType.literal(l.value(), environment)
                    
                    stream.out("case " + str(discrim_value) + ":\n")
                    stream.inc_indent()
                    size_calc = skutil.sizeCalculation(environment, caseType,
                                                       decl, "_msgsize",
                                                       "_pd_" + decl_name)
                    stream.out(size_calc)
                    stream.dec_indent()
                    stream.out("\nbreak;\n")
        if booleanWrap:
            stream.out(template.union_default_bool)
        else:
            stream.out(template.union_default)

    # build the switch itself
    def switch(stream = stream, exhaustive = exhaustive,
               hasDefault = hasDefault, defaultCase = defaultCase,
               environment = environment, defaultMember = defaultMember,
               cases = cases):
        if not(exhaustive):
            size_calc = "\n"
            if hasDefault:
                caseType = types.Type(defaultCase.caseType())
                decl = defaultCase.declarator()
                size_calc = skutil.sizeCalculation(environment, caseType,
                                                   decl, "_msgsize",
                                                   "_pd_" + defaultMember)
            stream.out(template.union_align_nonexhaustive,
                       size_calc = size_calc,
                       cases = cases)
        else:
            stream.out(template.union_align_exhaustive,
                       cases = cases)

    # output the alignedSize method
    stream.out(template.union,
               name = name,
               discriminator_size_calc = discriminator_size_calc,
               switch = switch)
    

    # --------------------------------------------------------------
    # union::operator{>>, <<}= ({Net, Mem}BufferedStream& _n) [const]
    #
    # FIXME: I thought the CORBA::MARSHAL exception thrown when
    # unmarshalling an array of strings was skipped when unmarshalling
    # from a MemBufferedStream (it is for a struct, but not for a union)
    # (This is probably due to a string-inconsistency with the old compiler
    # and can be sorted out later)

    # marshal/ unmarshal individual cases
    marshal_cases = output.StringStream()
    unmarshal_cases = output.StringStream()
    for c in node.cases():
        caseType = types.Type(c.caseType())
        decl = c.declarator()
        decl_scopedName = id.Name(decl.scopedName())
        decl_name = decl_scopedName.simple()
        isDefault = defaultCase == c
        
        for l in c.labels():
            value = l.value()
            discrim_value = switchType.literal(value, environment)
            if l.default():
                unmarshal_cases.out("default:")
            else:
                unmarshal_cases.out("case " + discrim_value + ":")

                marshal_cases.out("case " + discrim_value + ":")
                marshal_cases.inc_indent()
                skutil.marshall_struct_union(marshal_cases, environment,
                                             caseType, decl, "_pd_" + decl_name)
                marshal_cases.dec_indent()
                marshal_cases.out("break;")

            unmarshal_cases.inc_indent()
            unmarshal_cases.out("_pd__default = " + str(isDefault) + ";")
            skutil.unmarshall_struct_union(unmarshal_cases, environment,
                                           caseType, decl, "_pd_" + decl_name,
                                           can_throw_marshall = 1)
            unmarshal_cases.dec_indent()
            unmarshal_cases.out("break;")

    if not(hasDefault) and not(exhaustive):
        unmarshal_cases.out("default: _pd__default = 1; break;")
        
            
    if booleanWrap:
        marshal_cases.out(template.union_default_bool)
    else:
        marshal_cases.out(template.union_default)


    def marshal(stream = stream, exhaustive = exhaustive,
                hasDefault = hasDefault, defaultCase = defaultCase,
                environment = environment, defaultMember = defaultMember,
                marshal_cases = marshal_cases):
        if not(exhaustive):

            def default(stream = stream, exhaustive = exhaustive,
                        hasDefault = hasDefault, defaultCase = defaultCase,
                        environment = environment,
                        defaultMember = defaultMember):
                if hasDefault:
                    caseType = types.Type(defaultCase.caseType())
                    decl = defaultCase.declarator()
                    decl_scopedName = id.Name(decl.scopedName())
                    decl_name = decl_scopedName.simple()
                    skutil.marshall_struct_union(stream, environment, caseType,
                                                 decl, "_pd_" + decl_name)
            stream.out(template.union_operators_nonexhaustive,
                        default = default,
                        cases = str(marshal_cases))
        else:
            stream.out(template.union_operators_exhaustive,
                        cases = str(marshal_cases))

    # write the operators
    stream.out(template.union_operators,
               name = name,
               marshal_cases = marshal,
               unmarshal_cases = str(unmarshal_cases))
                
        
    return
    
    
def visitForward(node):
    return

def visitConst(node):
    environment = id.lookup(node)
    
    constType = types.Type(node.constType())
    d_constType = constType.deref()
    
    if d_constType.string():
        type_string = "char *"
    else:
        type_string = d_constType.base()

    scopedName = id.Name(node.scopedName())
    name = scopedName.fullyQualify()
    value = d_constType.literal(node.value(), environment)
    
    init_in_def = d_constType.representable_by_int()

    
    if init_in_def:
        if self.__insideInterface:
            stream.out("""\
const @type@ @name@ _init_in_cldef_( = @value@ );
""",
                       type = type_string, name = name, value = value)
        else:
            stream.out("""\
_init_in_def_( const @type@ @name@ = @value@; )
""",
                       type = type_string, name = name, value = value)
        return

    # not init_in_def
    if self.__insideModule and not(self.__insideInterface):
        scopedName = node.scopedName()
        scopedName = map(id.mapID, scopedName)
        scope_str = idlutil.ccolonName(scopedName[0:len(scopedName)-1])
        name_str = scopedName[len(scopedName)-1]
        stream.out(template.const_namespace,
                   type = type_string, scope = scope_str, name = name_str,
                   scopedName = name, value = value)
        
    else:
        stream.out("""\
const @type@ @name@ = @value@;
""",
                   type = type_string, name = name, value = value)
        

    
    pass
def visitDeclarator(node):
    pass
def visitException(node):
    scopedName = id.Name(node.scopedName())
    name = scopedName.simple()
    cxx_name = id.mapID(name)
    
    outer_environment = id.lookup(node)
    environment = outer_environment.enter(name)
    
    scoped_name = scopedName.fullyQualify()

    # build the default ctor, copy ctor, assignment operator
    copy_ctor_body = output.StringStream()
    default_ctor_body = output.StringStream()
    default_ctor_args = []
    assign_op_body = output.StringStream()
    has_default_ctor = 0

    for m in node.members():
        has_default_ctor = 1
        memberType = types.Type(m.memberType())
        if m.constrType():
            memberType.type().decl().accept(self)
        d_memberType = memberType.deref()

        memberType_fqname = memberType.base()
            
        for d in m.declarators():
            decl_scopedName = id.Name(d.scopedName())
            decl_name = decl_scopedName.simple()
            
            decl_dims = d.sizes()
            full_dims = decl_dims + memberType.dims()
            is_array = full_dims != []
            is_array_declarator = decl_dims != []

            memberType_name_arg = memberType.op(types.IN, environment)

            if is_array_declarator:
                # we use the internal typedef'ed type if the member is an array
                # declarator
                memberType_name_arg = "const "                       +\
                                      config.state['Private Prefix'] +\
                                      "_" + decl_name
            elif d_memberType.sequence():
                if memberType.typedef():
                    memberType_name_arg = "const " + id.Name(memberType.type().decl().scopedName()).unambiguous(environment)
                else:
                    memberType_name_arg = "const " + memberType.sequenceTemplate(environment)
            elif memberType.typecode():
                memberType_name_arg = "CORBA::TypeCode_ptr"
                
            index = ""

            if is_array:
                blocks = [cxx.Block(copy_ctor_body),
                          cxx.Block(default_ctor_body),
                          cxx.Block(assign_op_body)]
                loops = [cxx.For(copy_ctor_body, full_dims),
                         cxx.For(default_ctor_body, full_dims),
                         cxx.For(assign_op_body, full_dims)]
                index = loops[0].index() # all the same

            copy_ctor_body.out("""\
@member_name@@index@ = _s.@member_name@@index@;""", member_name = decl_name,
                               index = index)

            if d_memberType.objref() and not(is_array):
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
                for loop in loops: loop.end()
                for block in blocks: block.end()
          
        
    default_ctor = output.StringStream()
    if has_default_ctor:
        default_ctor.out(template.exception_default_ctor,
                         scoped_name = scoped_name,
                         name = name,
                         ctor_args = string.join(default_ctor_args, ", "),
                         default_ctor_body = str(default_ctor_body))

    # write the main chunk
    stream.out(template.exception,
               scoped_name = scoped_name,
               name = name,
               copy_ctor_body = str(copy_ctor_body),
               default_ctor = str(default_ctor),
               ctor_args = string.join(default_ctor_args, ", "),
               default_ctor_body = str(default_ctor_body),
               repoID = node.repoId(),
               assign_op_body = str(assign_op_body))
    

    # deal with alignment, marshalling and demarshalling
    needs_marshalling = node.members() != []
    aligned_size = output.StringStream()
    mem_marshal = output.StringStream()
    net_marshal = output.StringStream()
    mem_unmarshal = output.StringStream()
    net_unmarshal = output.StringStream()
    
    for m in node.members():
        memberType = types.Type(m.memberType())
        d_memberType = memberType.deref()
        for d in m.declarators():
            decl_scopedName = id.Name(d.scopedName())
            decl_name = decl_scopedName.simple()
            is_array_declarator = d.sizes() != []
            
            if memberType.string() and not(is_array_declarator):
                tmp = skutil.unmarshal_string_via_temporary(decl_name, "_n")
                mem_unmarshal.out(tmp)
                net_unmarshal.out(tmp)
            # TypeCodes seem to be other exceptions
            elif d_memberType.typecode():
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

            if d_memberType.typecode():
                skutil.marshall_struct_union(mem_marshal, environment,
                                             memberType, d, decl_name, "_n")
                skutil.marshall_struct_union(net_marshal, environment,
                                             memberType, d, decl_name, "_n")
            else:
                exception = "BAD_PARAM"
                if is_array_declarator:
                    exception = "MARSHAL"
                skutil.marshall(mem_marshal, environment,
                                memberType, d, decl_name, "_n",
                                exception = exception)
                skutil.marshall(net_marshal, environment,
                                memberType, d, decl_name, "_n",
                                exception = exception)

            aligned_size.out(skutil.sizeCalculation(environment, memberType,
                                                    d, "_msgsize", decl_name,
                                                    fixme = 1))

    if needs_marshalling:
        stream.out(template.exception_operators,
                   scoped_name = scoped_name,
                   aligned_size = str(aligned_size),
                   net_marshal = str(net_marshal),
                   mem_marshal = str(mem_marshal),
                   net_unmarshal = str(net_unmarshal),
                   mem_unmarshal = str(mem_unmarshal))


    return
            
            
                                           
