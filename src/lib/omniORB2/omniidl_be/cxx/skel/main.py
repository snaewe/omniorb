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
# Revision 1.2  1999/11/15 19:13:38  djs
# Union skeletons working
#
# Revision 1.1  1999/11/12 17:18:58  djs
# Struct skeleton code added
#

"""Produce the main skeleton definitions"""
# similar to o2be_root::produce_skel in the old C++ BE

from omniidl import idlast, idltype, idlutil

from omniidl.be.cxx import tyutil, util, name, config, skutil

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
    name = util.delimitedlist(name, "::")

    size_calculation = "omni::align_to(_msgsize, omni::ALIGN_4) + 4"

    marshall = util.StringStream()
    Mem_unmarshall = util.StringStream()
    Net_unmarshall = util.StringStream()
    msgsize = util.StringStream()
    
    for n in node.members():
        n.accept(self)

        for d in n.declarators():
            # marshall and unmarshall the struct members
            member_name = tyutil.name(d.scopedName())
            memberType = n.memberType()

            skutil.marshall(marshall, environment, memberType, d,
                            member_name)
            skutil.unmarshall(Mem_unmarshall, environment, memberType, d,
                              member_name, 0)
            skutil.unmarshall(Net_unmarshall, environment, memberType, d,
                              member_name, 1)  

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
    name = util.delimitedlist(name, "::")

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
                skutil.marshall(stream, environment, caseType,
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
                   skutil.marshall(stream, environment, caseType,
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
            
            skutil.unmarshall(stream, environment, caseType, decl,
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

