#include <Python.h>
#include <numpy/arrayobject.h>

// Written following the tutorial : http://dan.iel.fm/posts/python-c-extensions/

/* The module doc string */
PyDoc_STRVAR(ctools__doc__,
        "Utilitary functions written in C");


PyDoc_STRVAR(cconv_onestep_utilitary__doc__,
        "Utilitary function to compute the convolution with a step kernel");
void cconv_onestep_utilitary(double* fu, unsigned int N, const double& A, const double& s, double* res) {
   
}

// The wrapper to the C function
static PyObject *
py_cconv_onestep_utilitary(PyObject *self, PyObject *args) {
    unsigned int N;   
    double A;
    double s;

    PyObject *obj_fu, *obj_res;

    /* Parse the input tuple */
    if (!PyArg_ParseTuple(args, "OddO", &obj_fu, &A, &s, &obj_res))
                    return NULL;

    /* Interpret the input objects as numpy arrays. */
    PyObject *py_fu = PyArray_FROM_OTF(obj_fu, NPY_DOUBLE, NPY_IN_ARRAY);
    PyObject *py_res = PyArray_FROM_OTF(obj_res, NPY_DOUBLE, NPY_IN_ARRAY);

    /* If that didn't work, throw an exception. */
    if (py_fu == NULL || py_res == NULL) {
        Py_XDECREF(py_fu);
        Py_XDECREF(py_res);
        return NULL;
    }

    /* Get the number of data points */
    N = (unsigned int)PyArray_DIM(py_fu, 0);

    /* Get pointers to the data as C-types. */
    double *fu    = (double*)PyArray_DATA(py_fu);
    double *res    = (double*)PyArray_DATA(py_res);

    /* Call the external C function */
    cconv_onestep_utilitary(fu, N, A, s, res);

    /* Clean up. */
    Py_DECREF(py_fu);
    Py_DECREF(py_res);

    Py_RETURN_NONE;

}

static PyMethodDef module_methods[] = {
    {"cconv_onestep_utilitary", py_cconv_onestep_utilitary, METH_VARARGS, cconv_onestep_utilitary__doc__},
    {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC initctools(void)
{
    PyObject *m = Py_InitModule3("ctools", module_methods, ctools__doc__);
    if (m == NULL)
        return;

    /* Load `numpy` functionality. */
    import_array();
}

