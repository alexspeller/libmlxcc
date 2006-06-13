/*=============================================================================
 *
 * Copyright 2006 Andrew Bruno <aeb@qnot.org> 
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at 
 *
 *  http://www.apache.org/licenses/LICENSE-2.0 
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 *
 *============================================================================*/

%{
static const char *_xcc_error_message = NULL;
static XCC_ERROR_CODE _xcc_error_code = 0;

void set_exception(const char *msg, XCC_ERROR_CODE error_code) {
    _xcc_error_message = msg;
    _xcc_error_code = error_code;
}

void clear_exception() {
    _xcc_error_code = 0;
    _xcc_error_message = NULL;
}

void throw_exception() {
    PyObject *e, *emod, *edict;
    if(!(emod = PyImport_ImportModule("xcc"))) return;
    if(!(edict = PyModule_GetDict(emod))) return;
    if(XCC_ERROR_XQUERY == _xcc_error_code) {
        if(!(e = PyDict_GetItemString(edict, "XQueryError"))) return;
    } else {
        if(!(e = PyDict_GetItemString(edict, "Error"))) return;
    }

    PyObject_SetAttrString(e, "code", PyInt_FromLong((long)_xcc_error_code));
    PyObject_SetAttrString(e, "message", PyString_FromString(_xcc_error_message));
    PyErr_SetObject(e, e);
}

#define XccError(num, msg) \
    set_exception(msg, num);

#define XccErrorXQuery(error_xml) \
    set_exception(error_xml, XCC_ERROR_XQUERY);
%}
