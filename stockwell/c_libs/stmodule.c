/*
 * stmodule.c
 *
 * Original version downloaded from
 *   http://kurage.nimh.nih.gov/meglab/Meg/Stockwell
 *
 * Modified for Python3 compatibility
 */

/* Stockwell Transform wrapper code. */
#include <string.h>
#include <stdio.h>
#include <Python.h>
#include <numpy/arrayobject.h>
#include "my_types.h"

extern void st(int, int, int, double, enum WINDOW window_code, double *, double *);
extern void ist(int, int, int, double *, double *);
extern void hilbert(int, double *, double *);

static char Doc_st[] =
"st(x[, gamma, window type, lo, hi]) returns the 2d, complex Stockwell transform of the real\n\
array x. If lo and hi are specified, only those frequencies (rows) are\n\
returned; lo and hi default to 0 and n/2, resp., where n is the length of x.\n\
Gamma coeficcfent is set default to 1. Gamma is the adjustable parameter, \n\
which has been defined to tune the time and frequency resolutions of the S-transform. \n\
It presents the number of Fourier sinusoidal periods within one standard deviation of the Gaussian window. \n\
Raising gamma increases the frequency resolution and consequently decreases the time resolution of the S-transform and vice versa (Kazemi, 2014). \n\
Two window types available: gauss (default) and kazemi (Kazemi, 2014).";

static PyObject *st_wrap(PyObject *self, PyObject *args)
{
	int n, dim[2];
	int lo = 0;
	int hi = 0;
	double gamma = 1;
	char *win_type;
	char buff[10];
	PyObject *o;
	PyArrayObject *a, *r;
	enum WINDOW window_type = GAUSS;
	if (!PyArg_ParseTuple(args, "O|sdii", &o, &win_type, &gamma, &lo, &hi)) {
		return NULL;
	}
	if(strcmp(win_type, "kazemi") == 0){
		window_type = KAZEMI;
	}
	a = (PyArrayObject *)
		PyArray_ContiguousFromObject(o, PyArray_DOUBLE, 1, 1);
	if (a == NULL) {
		return NULL;
	}
	n = a->dimensions[0];

	if (lo == 0 && hi == 0) {
		hi = n / 2;
	}

	dim[0] = hi - lo + 1;
	dim[1] = n;
	r = (PyArrayObject *)PyArray_FromDims(2, dim, PyArray_CDOUBLE);
	if (r == NULL) {
		Py_DECREF(a);
		return NULL;
	}
	st(n, lo, hi, gamma, window_type, (double *)a->data, (double *)r->data);

	Py_DECREF(a);
	return PyArray_Return(r);
}

static char Doc_ist[] =
"ist(y[, lo, hi]) returns the inverse Stockwell transform of the 2d, complex\n\
array y.";

static PyObject *ist_wrap(PyObject *self, PyObject *args)
{
	int n, m, dim[1];
	int lo = 0;
	int hi = 0;
	PyObject *o;
	PyArrayObject *a, *r;

	if (!PyArg_ParseTuple(args, "O|ii", &o, &lo, &hi)) {
		return NULL;
	}

	a = (PyArrayObject *)
		PyArray_ContiguousFromObject(o, PyArray_CDOUBLE, 2, 2);
	if (a == NULL) {
		return NULL;
	}
	n = a->dimensions[0];
	m = a->dimensions[1];

	if (lo == 0 && hi == 0) {
		hi = m / 2;
	}
	if (hi - lo + 1 != n) {
		/* inconsistent dimensions */
		Py_DECREF(a);
		return NULL;
	}

	dim[0] = m;
	r = (PyArrayObject *)PyArray_FromDims(1, dim, PyArray_DOUBLE);
	if (r == NULL) {
		Py_DECREF(a);
		return NULL;
	}

	ist(m, lo, hi, (double *)a->data, (double *)r->data);

	Py_DECREF(a);
	return PyArray_Return(r);
}

static char Doc_hilbert[] =
"hilbert(x) returns the complex Hilbert transform of the real array x.";

static PyObject *hilbert_wrap(PyObject *self, PyObject *args)
{
	int n, dim[1];
	PyObject *o;
	PyArrayObject *a, *r;

	if (!PyArg_ParseTuple(args, "O", &o)) {
		return NULL;
	}

	a = (PyArrayObject *)
		PyArray_ContiguousFromObject(o, PyArray_DOUBLE, 1, 1);
	if (a == NULL) {
		return NULL;
	}
	n = a->dimensions[0];

	dim[0] = n;
	r = (PyArrayObject *)PyArray_FromDims(1, dim, PyArray_CDOUBLE);
	if (r == NULL) {
		Py_DECREF(a);
		return NULL;
	}

	hilbert(n, (double *)a->data, (double *)r->data);

	Py_DECREF(a);
	return PyArray_Return(r);
}

static PyMethodDef Methods[] = {
	{ "st", st_wrap, METH_VARARGS, Doc_st },
	{ "ist", ist_wrap, METH_VARARGS, Doc_ist },
	{ "hilbert", hilbert_wrap, METH_VARARGS, Doc_hilbert },
	{ NULL, NULL, 0, NULL }
};

struct module_state {
    PyObject *error;
};

#if PY_MAJOR_VERSION >= 3
#define GETSTATE(m) ((struct module_state*)PyModule_GetState(m))
#else
#define GETSTATE(m) (&_state)
static struct module_state _state;
#endif

#if PY_MAJOR_VERSION >= 3

static int st_traverse(PyObject *m, visitproc visit, void *arg) {
    Py_VISIT(GETSTATE(m)->error);
    return 0;
}

static int st_clear(PyObject *m) {
    Py_CLEAR(GETSTATE(m)->error);
    return 0;
}


static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "st",
        NULL,
        sizeof(struct module_state),
        Methods,
        NULL,
        st_traverse,
        st_clear,
        NULL
};

#define INITERROR return NULL

PyMODINIT_FUNC
PyInit_st(void)

#else
#define INITERROR return

void initst()
#endif
{
#if PY_MAJOR_VERSION >= 3
	PyObject *module = PyModule_Create(&moduledef);
#else
	Py_InitModule("st", Methods);
#endif
	import_array();

#if PY_MAJOR_VERSION >= 3
	return module;
#endif
}
