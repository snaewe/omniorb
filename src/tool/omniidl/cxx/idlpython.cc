// -*- c++ -*-
//                          Package   : omniidl
// idlpython.cc             Created on: 1999/10/27
//			    Author    : Duncan Grisby (dpg1)
//
//    Copyright (C) 1999 AT&T Laboratories Cambridge
//
//  This file is part of omniidl.
//
//  omniidl is free software; you can redistribute it and/or modify it
//  under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//  02111-1307, USA.
//
// Description:
//   
//   Python interface to front-end

// $Id$
// $Log$
// Revision 1.3  1999/11/01 10:05:27  dpg1
// New file attribute to AST.
// Fix stupid bug in module initialisation.
//
// Revision 1.2  1999/10/29 18:19:19  dpg1
// Added dump() function
//
// Revision 1.1  1999/10/29 15:44:45  dpg1
// First revision.
//

#include <python1.5/Python.h>

#include <idlast.h>
#include <idltype.h>
#include <idlscope.h>
#include <idlvisitor.h>
#include <idldump.h>
#include <idlerr.h>


#define ASSERT_RESULT     if (!result_) PyErr_Print(); assert(result_)
#define ASSERT_PYOBJ(pyo) if (!pyo)     PyErr_Print(); assert(pyo)

class PythonVisitor : public AstVisitor, public TypeVisitor {
public:
  PythonVisitor();
  virtual ~PythonVisitor();

  void visitAST              (AST*);
  void visitModule           (Module*);
  void visitInterface        (Interface*);
  void visitForward          (Forward*);
  void visitConst            (Const*);
  void visitDeclarator       (Declarator*);
  void visitTypedef          (Typedef*);
  void visitMember           (Member*);
  void visitStruct           (Struct*);
  void visitException        (Exception*);
  void visitCaseLabel        (CaseLabel*);
  void visitUnionCase        (UnionCase*);
  void visitUnion            (Union*);
  void visitEnumerator       (Enumerator*);
  void visitEnum             (Enum*);
  void visitAttribute        (Attribute*);
  void visitParameter        (Parameter*);
  void visitOperation        (Operation*);
  void visitNative           (Native*);
  void visitStateMember      (StateMember*);
  void visitFactory          (Factory*);
  void visitValueForward     (ValueForward*);
  void visitValueBox         (ValueBox*);
  void visitValueAbs         (ValueAbs*);
  void visitValue            (Value*);

  void visitBaseType    (BaseType*);
  void visitStringType  (StringType*);
  void visitWStringType (WStringType*);
  void visitSequenceType(SequenceType*);
  void visitFixedType   (FixedType*);
  void visitDeclaredType(DeclaredType*);

  PyObject* result() { return result_; }

private:
  PyObject* scopedNameToList(const ScopedName* sn);
  PyObject* pragmasToList(const Decl::Pragma* ps);
  void      registerPyDecl(const ScopedName* sn, PyObject* pydecl);
  PyObject* findPyDecl(const ScopedName* sn);
  PyObject* wstringToList(const _CORBA_WChar* ws);

  PyObject* idlast_;
  PyObject* idltype_;

  PyObject* result_; // Current working value
};

PythonVisitor::
PythonVisitor()
{
  idlast_  = PyImport_ImportModule("idlast");  ASSERT_PYOBJ(idlast_);
  idltype_ = PyImport_ImportModule("idltype"); ASSERT_PYOBJ(idltype_);
}

PythonVisitor::
~PythonVisitor()
{
  Py_DECREF(idlast_);
  Py_DECREF(idltype_);
}


PyObject*
PythonVisitor::
scopedNameToList(const ScopedName* sn)
{
  ScopedName::Fragment* f;
  int i;

  for (i=0, f = sn->scopeList(); f; f = f->next(), ++i);

  PyObject* pylist = PyList_New(i);

  for (i=0, f = sn->scopeList(); f; f = f->next(), ++i)
    PyList_SetItem(pylist, i, PyString_FromString(f->identifier()));

  return pylist;
}


PyObject*
PythonVisitor::
pragmasToList(const Decl::Pragma* ps)
{
  const Decl::Pragma* p;
  int i;

  for (i=0, p = ps; p; p = p->next(), ++i);

  PyObject* pylist = PyList_New(i);

  for (i=0, p = ps; p; p = p->next(), ++i)
    PyList_SetItem(pylist, i, PyString_FromString(p->pragmaText()));

  return pylist;
}

void
PythonVisitor::
registerPyDecl(const ScopedName* sn, PyObject* pydecl)
{
  PyObject* pysn = scopedNameToList(sn);
  PyObject* r    = PyObject_CallMethod(idlast_, "registerDecl", "NO",
				       pysn, pydecl);
  ASSERT_PYOBJ(r); Py_DECREF(r);
}

PyObject*
PythonVisitor::
findPyDecl(const ScopedName* sn)
{
  PyObject* pysn   = scopedNameToList(sn);
  PyObject* pydecl = PyObject_CallMethod(idlast_, "findDecl", "N", pysn);
  ASSERT_PYOBJ(pydecl);
  return pydecl;
}

PyObject*
PythonVisitor::
wstringToList(const _CORBA_WChar* ws)
{
  int i;
  const _CORBA_WChar* wc;

  for (i=0, wc=ws; *wc; ++wc, ++i);
  PyObject* pyl = PyList_New(i);

  for (i=0, wc=ws; *wc; ++wc, ++i)
    PyList_SetItem(pyl, i, PyInt_FromLong(*wc));

  return pyl;
}

//
// AST visit functions
//

void
PythonVisitor::
visitAST(AST* a)
{
  Decl* d;
  int   i;
  for (i=0, d = a->declarations(); d; d = d->next(), ++i);

  PyObject* pydecls = PyList_New(i);

  for (i=0, d = a->declarations(); d; d = d->next(), ++i) {
    d->accept(*this);
    PyList_SetItem(pydecls, i, result_);
  }
  result_ = PyObject_CallMethod(idlast_, "AST", "sN", a->file(), pydecls);
  ASSERT_RESULT;
}

void
PythonVisitor::
visitModule(Module* m)
{
  Decl* d;
  int   i;
  for (i=0, d = m->definitions(); d; d = d->next(), ++i);

  PyObject* pydecls = PyList_New(i);

  for (i=0, d = m->definitions(); d; d = d->next(), ++i) {
    d->accept(*this);
    PyList_SetItem(pydecls, i, result_);
  }
  result_ = PyObject_CallMethod(idlast_, "Module", "siiNsNsN",
				m->file(), m->line(), (int)m->mainFile(),
				pragmasToList(m->pragmas()),
				m->identifier(),
				scopedNameToList(m->scopedName()),
				m->repoId(),
				pydecls);
  registerPyDecl(m->scopedName(), result_);
  ASSERT_RESULT;
}

void
PythonVisitor::
visitInterface(Interface* i)
{
  int       l;
  PyObject* pyobj;
  Decl*     d;

  // Inherited interfaces
  InheritSpec* inh;

  for (l=0, inh = i->inherits(); inh; inh = inh->next(), ++l);
  PyObject* pyinherits = PyList_New(l);

  for (l=0, inh = i->inherits(); inh; inh = inh->next(), ++l) {
    d = inh->decl();
    if (d->kind() == Decl::D_INTERFACE)
      pyobj = findPyDecl(((Interface*)d)->scopedName());
    else if (d->kind() == Decl::D_DECLARATOR)
      pyobj = findPyDecl(((Declarator*)d)->scopedName());
    else
      assert(0);
    PyList_SetItem(pyinherits, l, pyobj);
  }

  PyObject* pyintf =
    PyObject_CallMethod(idlast_, "Interface", "siiNsNsiN",
			i->file(), i->line(), (int)i->mainFile(),
			pragmasToList(i->pragmas()),
			i->identifier(),
			scopedNameToList(i->scopedName()),
			i->repoId(),
			(int)i->abstract(), pyinherits);
  ASSERT_PYOBJ(pyintf);
  registerPyDecl(i->scopedName(), pyintf);

  // Contents
  for (l=0, d = i->contents(); d; d = d->next(), ++l);
  PyObject* pycontents = PyList_New(l);

  for (l=0, d = i->contents(); d; d = d->next(), ++l) {
    d->accept(*this);
    PyList_SetItem(pycontents, l, result_);
  }
  PyObject* r = PyObject_CallMethod(pyintf, "_setContents", "N", pycontents);
  ASSERT_PYOBJ(r); Py_DECREF(r);
  result_ = pyintf;
}

void
PythonVisitor::
visitForward(Forward* f)
{
  result_ = PyObject_CallMethod(idlast_, "Forward", "siiNsNsi",
				f->file(), f->line(), (int)f->mainFile(),
				pragmasToList(f->pragmas()),
				f->identifier(),
				scopedNameToList(f->scopedName()),
				f->repoId(),
				(int)f->abstract());
  ASSERT_RESULT;
  registerPyDecl(f->scopedName(), result_);
}

void
PythonVisitor::
visitConst(Const* c)
{
  c->constType()->accept(*this);
  PyObject* pytype = result_;
  PyObject* pyv;
  char      buffer[80];

  switch(c->constKind()) {
  case IdlType::tk_short:  pyv = PyInt_FromLong(c->constAsShort());  break;
  case IdlType::tk_long:   pyv = PyInt_FromLong(c->constAsLong());   break;
  case IdlType::tk_ushort: pyv = PyInt_FromLong(c->constAsUShort()); break;
  case IdlType::tk_ulong:
    pyv = PyLong_FromUnsignedLong(c->constAsULong()); break;

  case IdlType::tk_float:  pyv = PyFloat_FromDouble(c->constAsFloat());  break;
  case IdlType::tk_double: pyv = PyFloat_FromDouble(c->constAsDouble()); break;

  case IdlType::tk_boolean:
    pyv = PyInt_FromLong(c->constAsBoolean()); break;

  case IdlType::tk_char:  pyv = Py_BuildValue("c", c->constAsChar()); break;
  case IdlType::tk_octet: pyv = PyInt_FromLong(c->constAsOctet());    break;

  case IdlType::tk_string:
    pyv = PyString_FromString(c->constAsString()); break;

#ifdef HAS_LongLong
  case IdlType::tk_longlong:
    sprintf(buffer, "%Ld", c->constAsLongLong());
    pyv = PyLong_FromString(buffer, 0, 0);
    break;

  case IdlType::tk_ulonglong:
    sprintf(buffer, "%Lu", c->constAsULongLong());
    pyv = PyLong_FromString(buffer, 0, 0);
    break;
#endif
#ifdef HAS_LongDouble
  case IdlType::tk_longdouble:
    pyv = PyFloat_FromDouble(c->constAsLongDouble());
    IdlWarning(c->file(), c->line(),
	       "long double constant truncated to double by Python");
    break;
#endif
  case IdlType::tk_wchar:   pyv = PyInt_FromLong(c->constAsWChar());  break;
  case IdlType::tk_wstring: pyv = wstringToList(c->constAsWString()); break;
  case IdlType::tk_enum:
    pyv = findPyDecl(c->constAsEnumerator()->scopedName());
    break;

  default:
    assert(0);
  }
  result_ = PyObject_CallMethod(idlast_, "Const", "siiNsNsNN",
				c->file(), c->line(), (int)c->mainFile(),
				pragmasToList(c->pragmas()),
				c->identifier(),
				scopedNameToList(c->scopedName()),
				c->repoId(),
				pytype, pyv);
  ASSERT_RESULT;
  registerPyDecl(c->scopedName(), result_);
}

void
PythonVisitor::
visitDeclarator(Declarator* d)
{
  ArraySize* s;
  int        i;

  for (i=0, s = d->sizes(); s; s = s->next(), ++i);
  PyObject* pysizes = PyList_New(i);

  for (i=0, s = d->sizes(); s; s = s->next(), ++i)
    PyList_SetItem(pysizes, i, PyInt_FromLong(s->size()));

  result_ = PyObject_CallMethod(idlast_, "Declarator", "siiNsNsN",
				d->file(), d->line(), (int)d->mainFile(),
				pragmasToList(d->pragmas()),
				d->identifier(),
				scopedNameToList(d->scopedName()),
				d->repoId(),
				pysizes);
  ASSERT_RESULT;
  registerPyDecl(d->scopedName(), result_);
}

void
PythonVisitor::
visitTypedef(Typedef* t)
{
  if (t->constrType()) {
    ((DeclaredType*)t->aliasType())->decl()->accept(*this);
    Py_DECREF(result_);
  }
  t->aliasType()->accept(*this);
  PyObject* pyaliasType = result_;

  Declarator* d;
  int         i, l;

  for (l=0, d = t->declarators(); d; d = (Declarator*)d->next(), ++l);
  PyObject* pydeclarators = PyList_New(l);

  for (i=0, d = t->declarators(); d; d = (Declarator*)d->next(), ++i) {
    d->accept(*this);
    PyList_SetItem(pydeclarators, i, result_);
  }
  ASSERT_RESULT;
  result_ = PyObject_CallMethod(idlast_, "Typedef", "siiNNiN",
				t->file(), t->line(), (int)t->mainFile(),
				pragmasToList(t->pragmas()),
				pyaliasType, (int)t->constrType(),
				pydeclarators);

  // Give each Declarator a weak reference to the Typedef
  // *** Check this
  for (i=0; i<l; ++i) {
    PyObject_CallMethod(PyList_GetItem(pydeclarators, i),
			"_setAlias", "N", result_);
  }
}

void
PythonVisitor::
visitMember(Member* m)
{
  if (m->constrType()) {
    ((DeclaredType*)m->memberType())->decl()->accept(*this);
    Py_DECREF(result_);
  }
  m->memberType()->accept(*this);
  PyObject* pymemberType = result_;

  Declarator* d;
  int         i;

  for (i=0, d = m->declarators(); d; d = (Declarator*)d->next(), ++i);
  PyObject* pydeclarators = PyList_New(i);

  for (i=0, d = m->declarators(); d; d = (Declarator*)d->next(), ++i) {
    d->accept(*this);
    PyList_SetItem(pydeclarators, i, result_);
  }
  result_ = PyObject_CallMethod(idlast_, "Member", "siiNNiN",
				m->file(), m->line(), (int)m->mainFile(),
				pragmasToList(m->pragmas()),
				pymemberType, (int)m->constrType(),
				pydeclarators);
  ASSERT_RESULT;
}

void
PythonVisitor::
visitStruct(Struct* s)
{
  Member* m;
  int     i;

  PyObject* pystruct = 
    PyObject_CallMethod(idlast_, "Struct", "siiNsNsi",
			s->file(), s->line(), (int)s->mainFile(),
			pragmasToList(s->pragmas()),
			s->identifier(),
			scopedNameToList(s->scopedName()),
			s->repoId(),
			(int)s->recursive());
  ASSERT_PYOBJ(pystruct);
  registerPyDecl(s->scopedName(), pystruct);

  for (i=0, m = s->members(); m; m = (Member*)m->next(), ++i);
  PyObject* pymembers = PyList_New(i);

  for (i=0, m = s->members(); m; m = (Member*)m->next(), ++i) {
    m->accept(*this);
    PyList_SetItem(pymembers, i, result_);
  }
  PyObject* r = PyObject_CallMethod(pystruct, "_setMembers", "N", pymembers);
  ASSERT_PYOBJ(r); Py_DECREF(r);
  result_ = pystruct;
}

void
PythonVisitor::
visitException(Exception* e)
{
  Member* m;
  int     i;

  for (i=0, m = e->members(); m; m = (Member*)m->next(), ++i);
  PyObject* pymembers = PyList_New(i);

  for (i=0, m = e->members(); m; m = (Member*)m->next(), ++i) {
    m->accept(*this);
    PyList_SetItem(pymembers, i, result_);
  }
  result_ = PyObject_CallMethod(idlast_, "Exception", "siiNsNsN",
				e->file(), e->line(), (int)e->mainFile(),
				pragmasToList(e->pragmas()),
				e->identifier(),
				scopedNameToList(e->scopedName()),
				e->repoId(),
				pymembers);
  ASSERT_RESULT;
  registerPyDecl(e->scopedName(), result_);
}

void
PythonVisitor::
visitCaseLabel(CaseLabel* l)
{
  PyObject* pyv;
  char buffer[80];

  switch(l->labelKind()) {
  case IdlType::tk_short:  pyv = PyInt_FromLong(l->labelAsShort());  break;
  case IdlType::tk_long:   pyv = PyInt_FromLong(l->labelAsLong());   break;
  case IdlType::tk_ushort: pyv = PyInt_FromLong(l->labelAsUShort()); break;
  case IdlType::tk_ulong:
    pyv = PyLong_FromUnsignedLong(l->labelAsULong()); break;

  case IdlType::tk_boolean: pyv = PyInt_FromLong(l->labelAsBoolean());  break;
  case IdlType::tk_char:    pyv = Py_BuildValue("c", l->labelAsChar()); break;
#ifdef HAS_LongLong
  case IdlType::tk_longlong:
    sprintf(buffer, "%Ld", l->labelAsLongLong());
    pyv = PyLong_FromString(buffer, 0, 0);
    break;
  case IdlType::tk_ulonglong:
    sprintf(buffer, "%Lu", l->labelAsULongLong());
    pyv = PyLong_FromString(buffer, 0, 0);
    break;
#endif
  case IdlType::tk_wchar:   pyv = PyInt_FromLong(l->labelAsWChar());  break;
  case IdlType::tk_enum:
    pyv = findPyDecl(l->labelAsEnumerator()->scopedName());
    break;
  default:
    assert(0);
  }
  result_ = PyObject_CallMethod(idlast_, "CaseLabel", "siiNiNi",
				l->file(), l->line(), (int)l->mainFile(),
				pragmasToList(l->pragmas()),
				(int)l->isDefault(), pyv,
				(int)l->labelKind());
  ASSERT_RESULT;
}

void
PythonVisitor::
visitUnionCase(UnionCase* c)
{
  if (c->constrType()) {
    ((DeclaredType*)c->caseType())->decl()->accept(*this);
    Py_DECREF(result_);
  }
  CaseLabel* l;
  int        i;

  for (i=0, l = c->labels(); l; l = (CaseLabel*)l->next(), ++i);
  PyObject* pylabels = PyList_New(i);

  for (i=0, l = c->labels(); l; l = (CaseLabel*)l->next(), ++i) {
    l->accept(*this);
    PyList_SetItem(pylabels, i, result_);
  }
  c->caseType()->accept(*this);
  PyObject* pycaseType = result_;

  c->declarator()->accept(*this);
  PyObject* pydeclarator = result_;

  result_ = PyObject_CallMethod(idlast_, "UnionCase", "siiNNNiN",
				c->file(), c->line(), (int)c->mainFile(),
				pragmasToList(c->pragmas()),
				pylabels, pycaseType, (int)c->constrType(),
				pydeclarator);
  ASSERT_RESULT;
}

void
PythonVisitor::
visitUnion(Union* u)
{
  u->switchType()->accept(*this);
  PyObject* pyswitchType = result_;

  PyObject* pyunion =
    PyObject_CallMethod(idlast_, "Union", "siiNsNsNi",
			u->file(), u->line(), (int)u->mainFile(),
			pragmasToList(u->pragmas()),
			u->identifier(),
			scopedNameToList(u->scopedName()),
			u->repoId(),
			pyswitchType, (int)u->recursive());
  ASSERT_PYOBJ(pyunion);
  registerPyDecl(u->scopedName(), pyunion);

  UnionCase* c;
  int        i;
  for (i=0, c = u->cases(); c; c = (UnionCase*)c->next(), ++i);
  PyObject* pycases = PyList_New(i);

  for (i=0, c = u->cases(); c; c = (UnionCase*)c->next(), ++i) {
    c->accept(*this);
    PyList_SetItem(pycases, i, result_);
  }

  PyObject* r = PyObject_CallMethod(pyunion, "_setCases", "N", pycases);
  ASSERT_PYOBJ(r); Py_DECREF(r);
  result_ = pyunion;
}

void
PythonVisitor::
visitEnumerator(Enumerator* e)
{
  result_ = PyObject_CallMethod(idlast_, "Enumerator", "siiNsNs",
				e->file(), e->line(), (int)e->mainFile(),
				pragmasToList(e->pragmas()),
				e->identifier(),
				scopedNameToList(e->scopedName()),
				e->repoId());
  ASSERT_RESULT;
  registerPyDecl(e->scopedName(), result_);
}

void
PythonVisitor::
visitEnum(Enum* e)
{
  Enumerator* n;
  int         i;
  for (i=0, n = e->enumerators(); n; n = (Enumerator*)n->next(), ++i);
  PyObject* pyenumerators = PyList_New(i);

  for (i=0, n = e->enumerators(); n; n = (Enumerator*)n->next(), ++i) {
    n->accept(*this);
    PyList_SetItem(pyenumerators, i, result_);
  }
  result_ = PyObject_CallMethod(idlast_, "Enum", "siiNsNsN",
				e->file(), e->line(), (int)e->mainFile(),
				pragmasToList(e->pragmas()),
				e->identifier(),
				scopedNameToList(e->scopedName()),
				e->repoId(),
				pyenumerators);
  ASSERT_RESULT;
  registerPyDecl(e->scopedName(), result_);
}

void
PythonVisitor::
visitAttribute(Attribute* a)
{
  a->attrType()->accept(*this);
  PyObject* pyattrType = result_;

  Declarator* d;
  int         i;

  for (i=0, d = a->declarators(); d; d = (Declarator*)d->next(), ++i);
  PyObject* pyidentifiers = PyList_New(i);

  for (i=0, d = a->declarators(); d; d = (Declarator*)d->next(), ++i)
    PyList_SetItem(pyidentifiers, i, PyString_FromString(d->identifier()));    

  result_ = PyObject_CallMethod(idlast_, "Attribute", "siiNiNN",
				a->file(), a->line(), (int)a->mainFile(),
				pragmasToList(a->pragmas()),
				(int)a->readonly(), pyattrType,
				pyidentifiers);
  ASSERT_RESULT;
}

void
PythonVisitor::
visitParameter(Parameter* p)
{
  p->paramType()->accept(*this);
  PyObject* pyparamType = result_;

  result_ = PyObject_CallMethod(idlast_, "Parameter", "siiNiNs",
				p->file(), p->line(), (int)p->mainFile(),
				pragmasToList(p->pragmas()),
				p->direction(), pyparamType, p->identifier());
  ASSERT_RESULT;
}

void
PythonVisitor::
visitOperation(Operation* o)
{
  o->returnType()->accept(*this);
  PyObject* pyreturnType = result_;

  Parameter* p;
  int        i;
  for (i=0, p = o->parameters(); p; p = (Parameter*)p->next(), ++i);
  PyObject* pyparameters = PyList_New(i);

  for (i=0, p = o->parameters(); p; p = (Parameter*)p->next(), ++i) {
    p->accept(*this);
    PyList_SetItem(pyparameters, i, result_);
  }

  RaisesSpec* r;
  for (i=0, r = o->raises(); r; r = r->next(), ++i);
  PyObject* pyraises = PyList_New(i);

  for (i=0, r = o->raises(); r; r = r->next(), ++i)
    PyList_SetItem(pyraises, i, findPyDecl(r->exception()->scopedName()));

  ContextSpec* c;
  for (i=0, c = o->contexts(); c; c = c->next(), ++i);
  PyObject* pycontexts = PyList_New(i);

  for (i=0, c = o->contexts(); c; c = c->next(), ++i)
    PyList_SetItem(pycontexts, i, PyString_FromString(c->context()));

  result_ = PyObject_CallMethod(idlast_, "Operation", "siiNiNsNNN",
				o->file(), o->line(), (int)o->mainFile(),
				pragmasToList(o->pragmas()),
				(int)o->oneway(), pyreturnType,
				o->identifier(), pyparameters,
				pyraises, pycontexts);
  ASSERT_RESULT;
}

void
PythonVisitor::
visitNative(Native* n)
{
  printf("native %s\n", n->identifier());
}

void
PythonVisitor::
visitStateMember(StateMember* s)
{
  printf("StateMember\n");
}

void
PythonVisitor::
visitFactory(Factory* f)
{
  printf("Factory\n");
}

void
PythonVisitor::
visitValueForward(ValueForward* f)
{
  printf("ValueForward\n");
}

void
PythonVisitor::
visitValueBox(ValueBox* b)
{
  printf("ValueBox\n");
}

void
PythonVisitor::
visitValueAbs(ValueAbs* a)
{
  printf("ValueAbs\n");
}

void
PythonVisitor::
visitValue(Value* v)
{
  printf("Value\n");
}

// Types

void
PythonVisitor::
visitBaseType(BaseType* t)
{
  result_ = PyObject_CallMethod(idltype_, "baseType", "i", (int)t->kind());
  ASSERT_RESULT;
}

void
PythonVisitor::
visitStringType(StringType* t)
{
  result_ = PyObject_CallMethod(idltype_, "stringType", "i", t->bound());
  ASSERT_RESULT;
}

void
PythonVisitor::
visitWStringType(WStringType* t)
{
  result_ = PyObject_CallMethod(idltype_, "wstringType", "i", t->bound());
  ASSERT_RESULT;
}

void
PythonVisitor::
visitSequenceType(SequenceType* t)
{
  t->seqType()->accept(*this);
  result_ = PyObject_CallMethod(idltype_, "sequenceType", "Ni",
				result_, t->bound());
  ASSERT_RESULT;
}

void
PythonVisitor::
visitFixedType(FixedType* t)
{
  result_ = PyObject_CallMethod(idltype_, "fixedType", "i",
				t->digits(), t->scale());
  ASSERT_RESULT;
}

void
PythonVisitor::
visitDeclaredType(DeclaredType* t)
{
  if (t->decl()) {
    result_ =
      PyObject_CallMethod(idltype_, "declaredType", "NNi",
			  findPyDecl(t->declRepoId()->scopedName()),
			  scopedNameToList(t->declRepoId()->scopedName()),
			  (int)t->kind());
  }
  else {
    if (t->kind() == IdlType::tk_objref) {
      PyObject* pysn   = Py_BuildValue("[ss]", "CORBA", "Object");
      PyObject* pydecl = PyObject_CallMethod(idlast_, "findDecl", "O", pysn);
      result_ = PyObject_CallMethod(idltype_, "declaredType", "NNi",
				    pydecl, pysn, (int)t->kind());
    }
    else abort();
  }
  ASSERT_RESULT;
}

extern "C" {
  static PyObject* IdlPyCompile(PyObject* self, PyObject* args)
  {
    PyObject* pyfile;

    if (!PyArg_ParseTuple(args, "O!", &PyFile_Type, &pyfile))
      return 0;

    FILE*       file   = PyFile_AsFile(pyfile);
    PyObject*   pyname = PyFile_Name(pyfile);
    const char* name   = PyString_AsString(pyname);

    _CORBA_Boolean success = AST::process(file, name);

    PyObject* result;

    if (success) {
      PythonVisitor v;
      AST::tree()->accept(v);
      result = v.result();
    }
    else {
      Py_INCREF(Py_None);
      result = Py_None;
    }
    AST::tree()->clear();

    return result;
  }

  static PyObject* IdlPyDump(PyObject* self, PyObject* args)
  {
    PyObject* pyfile;

    if (!PyArg_ParseTuple(args, "O!", &PyFile_Type, &pyfile))
      return 0;

    FILE*       file   = PyFile_AsFile(pyfile);
    PyObject*   pyname = PyFile_Name(pyfile);
    const char* name   = PyString_AsString(pyname);

    _CORBA_Boolean success = AST::process(file, name);

    if (success) {
      DumpVisitor v;
      AST::tree()->accept(v);
    }
    AST::tree()->clear();

    Py_INCREF(Py_None);
    return Py_None;
  }

  static PyMethodDef omniidl_methods[] = {
    {"compile",        IdlPyCompile,        METH_VARARGS},
    {"dump",           IdlPyDump,           METH_VARARGS},
    {NULL, NULL}
  };

  void init_omniidl()
  {
    PyObject* m = Py_InitModule("_omniidl", omniidl_methods);
  }
}
