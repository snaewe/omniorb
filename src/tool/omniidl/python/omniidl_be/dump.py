#
# Back-end which just dumps the IDL tree
#

"""Dumps the IDL tree"""

import idlast, idltype, idlutil, idlvisitor, output
import sys, string

class DumpVisitor (idlvisitor.AstVisitor, idlvisitor.TypeVisitor):

    def __init__(self, st):
        self.st = st

    def visitAST(self, node):
        for n in node.declarations():
            n.accept(self)

    def visitModule(self, node):
        self.st.out("""\
module @id@ {""", id = node.identifier())

        self.st.inc_indent()

        for n in node.definitions():
            n.accept(self)

        self.st.dec_indent()

        self.st.out("""\
};""")

    def visitInterface(self, node):
        if len(node.inherits()) != 0:
            inheritl = []
            for i in node.inherits():
                inheritl.append("::" + idlutil.ccolonName(i.scopedName()))

                inherits = ": " + string.join(inheritl, ", ") + " "
        else:
            inherits = ""

        self.st.out("""\
interface @id@ @inherits@{""",
               id = node.identifier(), inherits=inherits)

        self.st.inc_indent()

        for n in node.contents():
            n.accept(self)

        self.st.dec_indent()
        self.st.out("""\
};""")


    def visitForward(self, node):
        self.st.out("""\
interface @id@;""", id = node.identifier())


    def visitConst(self, node):
        node.constType().accept(self)
        type  = self.__result_type
        value = str(node.value())
        if node.constType().kind() == idltype.tk_string:
            value = '"' + value + '"'
        
        self.st.out("""\
const @type@ @id@ = @value@;""",
               type=type, id=node.identifier(), value=value)


    def visitTypedef(self, node):
        node.aliasType().accept(self)
        type  = self.__result_type
        decll = []
        for d in node.declarators():
            d.accept(self)
            decll.append(self.__result_declarator)

        decls = string.join(decll, ", ")

        self.st.out("""\
typedef @type@ @decls@;""",
               type=type, decls=decls)


    def visitStruct(self, node):
        self.st.out("""\
struct @id@ {""",
               id = node.identifier())

        for m in node.members():
            if m.constrType():
                self.st.inc_indent()
                m.memberType().decl().accept(self)
                self.st.dec_indent()

            m.memberType().accept(self)
            type = self.__result_type
            decll = []
            for d in m.declarators():
                d.accept(self)
                decll.append(self.__result_declarator)
            decls = string.join(decll, ", ")

            self.st.out("""\
  @type@ @decls@;""",

                   type=type, decls=decls)

        self.st.out("""\
};""")

    def visitException(self, node):
        self.st.out("""\
exception @id@ {""",
               id = node.identifier())

        for m in node.members():
            if m.constrType():
                self.st.inc_indent()
                m.memberType().decl().accept(self)
                self.st.dec_indent()

            m.memberType().accept(self)
            type = self.__result_type
            decll = []
            for d in m.declarators():
                d.accept(self)
                decll.append(self.__result_declarator)
            decls = string.join(decll, ", ")

            self.st.out("""\
  @type@ @decls@;""",

                   type=type, decls=decls)

        self.st.out("""\
};""")


    def visitUnion(self, node):
        node.switchType().accept(self)
        stype = self.__result_type

        self.st.out("""\
union @id@ switch (@stype@) {""",

               id=node.identifier(), stype=stype)

        for c in node.cases():
            if c.constrType():
                self.st.inc_indent()
                c.caseType().decl().accept(self)
                self.st.dec_indent()

            for l in c.labels():
                if l.isDefault():
                    self.st.out("""\
  default:""")
                else:
                    if l.labelKind() == idltype.tk_enum:
                        lv = idlutil.ccolonName(l.value().scopedName())
                    else:
                        lv = str(l.value())
                        
                    self.st.out("""\
  case @lv@:""",
                           lv=lv)
                    
            c.caseType().accept(self)
            type = self.__result_type
            c.declarator().accept(self)
            decl = self.__result_declarator

            self.st.out("""\
    @type@ @decl@;""",
                   
                   type=type, decl=decl)

        self.st.out("};")


    def visitEnum(self, node):
        enuml = []
        for e in node.enumerators(): enuml.append(e.identifier())

        enums = string.join(enuml, ", ")

        self.st.out("""\
enum @id@ {@enums@};""",

               id=node.identifier(), enums=enums)

    def visitAttribute(self, node):
        if node.readonly():
            readonly = "readonly "
        else:
            readonly = ""

        node.attrType().accept(self)
        type = self.__result_type

        ids  = string.join(node.identifiers(), ", ")

        self.st.out("""\
@readonly@attribute @type@ @ids@;""",

               readonly=readonly, type=type, ids=ids)


    def visitOperation(self, node):
        if node.oneway():
            oneway = "oneway "
        else:
            oneway = ""
        
        node.returnType().accept(self)
        rtype = self.__result_type

        paraml = []
        for p in node.parameters():
            if   p.is_in() and p.is_out(): inout = "inout"
            elif p.is_in():                inout = "in"
            else:                          inout = "out"
            p.paramType().accept(self)
            type = self.__result_type
            paraml.append(inout + " " + type + " " + p.identifier())

        params = string.join(paraml, ", ")

        if len(node.raises()) > 0:
            raisel = []
            for r in node.raises():
                ename  = idlutil.ccolonName(r.scopedName())
                raisel.append(ename)

            raises = " raises (" + string.join(raisel, ", ") + ")"
        else:
            raises = ""

        self.st.out("""\
@oneway@@rtype@ @id@(@params@)@raises@;""",
               
               oneway=oneway, rtype=rtype, id=node.identifier(),
               params=params, raises=raises)


    def visitDeclarator(self, node):
        l = [node.identifier()]
        for s in node.sizes():
            l.append("[" + str(s) + "]")

        self.__result_declarator = string.join(l, "")


    ttsMap = {
        idltype.tk_void:       "void",
        idltype.tk_short:      "short",
        idltype.tk_long:       "long",
        idltype.tk_ushort:     "unsigned short",
        idltype.tk_ulong:      "unsigned long",
        idltype.tk_float:      "float",
        idltype.tk_double:     "double",
        idltype.tk_boolean:    "boolean",
        idltype.tk_char:       "char",
        idltype.tk_octet:      "octet",
        idltype.tk_any:        "any",
        idltype.tk_TypeCode:   "CORBA::TypeCode",
        idltype.tk_Principal:  "CORBA::Principal",
        idltype.tk_longlong:   "long long",
        idltype.tk_ulonglong:  "unsigned long long",
        idltype.tk_longdouble: "long double",
        idltype.tk_wchar:      "wchar"
        }

    def visitBaseType(self, type):
        self.__result_type = self.ttsMap[type.kind()]

    def visitStringType(self, type):
        if type.bound() == 0:
            self.__result_type = "string"
        else:
            self.__result_type = "string <" + str(type.bound()) + ">"

    def visitWStringType(self, type):
        if type.bound() == 0:
            self.__result_type = "wstring"
        else:
            self.__result_type = "wstring <" + str(type.bound()) + ">"

    def visitSequenceType(self, type):
        type.seqType().accept(self)
        if type.bound() == 0:
            self.__result_type = "sequence <" + self.__result_type + ">"
        else:
            self.__result_type = "sequence <" + self.__result_type + ", " +\
                                 str(type.bound()) + ">"

    def visitFixedType(self, type):
        self.__result_type = "fixed <" + str(type.digits()) + ", " +\
                             str(type.scale()) + ">"

    def visitDeclaredType(self, type):
        self.__result_type = "::" + \
                             idlutil.ccolonName(type.decl().scopedName())


def run(tree):
    print "\n\n\n\nRunning...\n\n\n"

    st = output.Stream(sys.stdout, 2)
    dv = DumpVisitor(st)
    tree.accept(dv)
