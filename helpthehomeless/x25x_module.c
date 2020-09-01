#include <Python.h>
#include "x25x.h"

static PyObject *x25x_getpowhash(PyObject *self, PyObject *args)
{
    char *output;
    PyObject *value;
#if PY_MAJOR_VERSION >= 3
    PyBytesObject *input;
#else
    PyStringObject *input;
#endif
    if (!PyArg_ParseTuple(args, "S", &input))
        return NULL;
    Py_INCREF(input);
    output = PyMem_Malloc(32);
#if PY_MAJOR_VERSION >= 3
    x25x_hash((char *)PyBytes_AsString((PyObject*) input), output);
#else
    x25x_hash((char *)PyString_AsString((PyObject*) input), output);
#endif
    Py_DECREF(input);
#if PY_MAJOR_VERSION >= 3
    value = Py_BuildValue("y#", output, 32);
#else
    value = Py_BuildValue("s#", output, 32);
#endif
    PyMem_Free(output);
    return value;
}

static PyMethodDef x25xMethods[] = {
    { "getPoWHash", x25x_getpowhash, METH_VARARGS, "Returns the proof of work hash using x25x" },
    { NULL, NULL, 0, NULL }
};

#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef x25xModule = {
    PyModuleDef_HEAD_INIT,
    "x25x_hash",
    "...",
    -1,
    x25xMethods
};

PyMODINIT_FUNC PyInit_x25x_hash(void) {
    return PyModule_Create(&x25xModule);
}

#else

PyMODINIT_FUNC initx25x_hash(void) {
    (void) Py_InitModule("x25x_hash", x25xMethods);
}
#endif
