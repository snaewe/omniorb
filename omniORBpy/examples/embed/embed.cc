// embed.cc -- C++ part of embedding example

#include <iostream.h>
#include <echo.hh>
#include PYTHON_INCLUDE
#include <omniORBpy.h>


#ifdef __WIN32__
#define DLL_EXPORT _declspec(dllexport)
#else
#define DLL_EXPORT
#endif

class Echo_i : public POA_Echo,
	       public PortableServer::RefCountServantBase
{
public:
  inline Echo_i() {}
  virtual ~Echo_i() {}
  virtual char* echoString(const char* mesg);
};


char* Echo_i::echoString(const char* mesg)
{
  cout << "C++ upcall '" << mesg << "'" << endl;
  return CORBA::string_dup(mesg);
}

class InterpreterUnlocker {
public:
  InterpreterUnlocker() {
    tstate_ = PyEval_SaveThread();
  }
  ~InterpreterUnlocker() {
    PyEval_RestoreThread(tstate_);
  }
private:
  PyThreadState* tstate_;
};


// This function retrieves the omniORBpyAPI struct from the _omnipy
// Python module.
static omniORBpyAPI*
getAPI()
{
  PyObject* omnipy = PyImport_ImportModule((char*)"_omnipy");
  if (!omnipy) {
    PyErr_SetString(PyExc_ImportError,
		    (char*)"Cannot import _omnipy");
    return 0;
  }
  PyObject* pyapi = PyObject_GetAttrString(omnipy, (char*)"API");
  omniORBpyAPI* api = (omniORBpyAPI*)PyCObject_AsVoidPtr(pyapi);
  Py_DECREF(pyapi);
  return api;
}


extern "C" {

  static PyObject* EmbedGetObjRef(PyObject* self, PyObject* args)
  {
    PyObject* pyorb;
    if (!PyArg_ParseTuple(args, (char*)"O", &pyorb)) return 0;

    omniORBpyAPI* api = getAPI();
    if (!api)
      return 0;

    CORBA::Object_var obj;
    CORBA::ORB_var    orb;

    try {
      // Python code gave us the ORB reference; convert it to the C++ reference
      obj = api->pyObjRefToCxxObjRef(pyorb, 1);
      orb = CORBA::ORB::_narrow(obj);
    }
    catch (CORBA::BAD_PARAM& ex) {
      PyErr_SetString(PyExc_TypeError,
		      (char*)"getObjRef() expects ORB as its argument");
      return 0;
    }
    // Activate an Echo object in the Root POA
    obj = orb->resolve_initial_references("RootPOA");
    PortableServer::POA_var poa = PortableServer::POA::_narrow(obj);

    Echo_i* myecho = new Echo_i();
      
    PortableServer::ObjectId_var myechoid = poa->activate_object(myecho);

    obj = myecho->_this();
    myecho->_remove_ref();

    PortableServer::POAManager_var pman = poa->the_POAManager();
    pman->activate();

    // Return the Echo reference to Python
    PyObject* ret = api->cxxObjRefToPyObjRef(obj, 1);
    return ret;
  }

  static PyObject* EmbedPutObjRef(PyObject* self, PyObject* args)
  {
    PyObject* pyobj;
    if (!PyArg_ParseTuple(args, (char*)"O", &pyobj)) return 0;

    omniORBpyAPI* api = getAPI();
    if (!api)
      return 0;

    CORBA::Object_var obj;
    Echo_var eobj;
    try {
      obj = api->pyObjRefToCxxObjRef(pyobj, 1);
    }
    catch (CORBA::SystemException& ex) {
      PyErr_SetString(PyExc_TypeError,
		      (char*)"putObjRef() expects CORBA object argument");
      return 0;
    }
    eobj = Echo::_narrow(obj);
    if (CORBA::is_nil(eobj)) {
      PyErr_SetString(PyExc_TypeError,
		      (char*)"putObjRef() argument not an Echo");
      return 0;
    }
    {
      // To do the call into Python, we must release the Python
      // interpreter lock, since the code doing the upcall acquires
      // it. The InterpreterUnlocker class deals with it for us.
      InterpreterUnlocker _u;
      cout << "\nTrying call to Python object..." << endl;
      CORBA::String_var ret = eobj->echoString("Hello from C++");
      cout << "The result was '" << (char*)ret << "'" << endl;
    }

    Py_INCREF(Py_None);
    return Py_None;
  }

  static PyMethodDef embed_methods[] = {
    {(char*)"getObjRef", EmbedGetObjRef, METH_VARARGS},
    {(char*)"putObjRef", EmbedPutObjRef, METH_VARARGS},
    {NULL, NULL}
  };

  void DLL_EXPORT init_embed()
  {
    PyObject* m = Py_InitModule((char*)"_embed", embed_methods);
  }
}
