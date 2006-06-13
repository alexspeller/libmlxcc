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
#include <marklogic/xcc.h>
%}

#ifdef SWIGRUBY
%module Xcc
%mixin XCC_RES "Enumerable";
%include ruby-typemaps.i
%include ruby.i
#endif
#ifdef SWIGPYTHON
%module xcc
%pythoncode %{
from exceptions import Exception, StandardError
%}
%include python-typemaps.i
%include python.i
#endif
#ifdef SWIGPERL
%module Xcc
%include perl-typemaps.i
%include perl.i
#endif

%rename(Session) XCC_SESS;
%rename(Request) XCC_REQ;
%rename(Result) XCC_RES;
%rename(Item) XCC_ITEM;
%rename(Content) XCC_CONTENT;

#ifdef SWIGRUBY
%init %{
     _xccRubyXQueryError = rb_define_class_under(mXcc, "XQueryError", rb_eStandardError); \
     rb_define_method(_xccRubyXQueryError, "code", _xcc_ruby_error_code, 0); \
     rb_define_method(_xccRubyXQueryError, "to_xml", _xcc_ruby_error_to_xml, 0); \

     _xccRubyError = rb_define_class_under(mXcc, "Error", rb_eStandardError); \
     rb_define_method(_xccRubyError, "code", _xcc_ruby_error_code, 0); \
%}
#endif

%include "constants.i";

#ifdef SWIGPYTHON
%define PY_XCC_EXCEPTION {
    clear_exception();
    $action
    if(_xcc_error_code != 0) {
        throw_exception();
        return NULL;
    }
}
%enddef

%exception submit PY_XCC_EXCEPTION;
%exception server_info PY_XCC_EXCEPTION;
%exception new_adhoc_query PY_XCC_EXCEPTION;
%exception content PY_XCC_EXCEPTION;
%exception set_bufsize PY_XCC_EXCEPTION;
%exception set_timeout PY_XCC_EXCEPTION;
%exception setvar PY_XCC_EXCEPTION;
%exception insert PY_XCC_EXCEPTION;
#endif

#ifdef SWIGPERL
%module "Xcc::Session"
#endif SWIGPERL
typedef struct {
    %extend {

    XCC_SESS(const char *user, const char *password, const char *host, const char *db, int port) {
        XCC_SESS *session = xcc_new_session(user, password, host, db, port);
        if(!session) {
            XccError(XCC_ERROR_CONNECTION, "Failed to create new session");
        }

        return session;
    }

    ~XCC_SESS() {
        xcc_free_session(self);
    }

    const char* server_info() {
        const char *info = xcc_server_info(self);
        if(!info) return "";

        return info;
    }

    %newobject new_adhoc_query;
    XCC_REQ* new_adhoc_query(const char *xquery) {
        XCC_REQ *request = xcc_new_adhoc_query(self, xquery);
        if(!request) {
            XccError(xcc_errcode(self), xcc_error(self));
        }

        return request;
    }

    %newobject new_module_invoke;
    XCC_REQ* new_module_invoke(const char *module_name) {
        XCC_REQ *request = xcc_new_module_invoke(self, module_name);
        if(!request) {
            XccError(xcc_errcode(self), xcc_error(self));
        }

        return request;
    }

    %newobject new_module_spawn;
    XCC_REQ* new_module_spawn(const char *module_name) {
        XCC_REQ *request = xcc_new_module_spawn(self, module_name);
        if(!request) {
            XccError(xcc_errcode(self), xcc_error(self));
        }

        return request;
    }

    %newobject new_content;
    XCC_CONTENT* new_content() {
        XCC_CONTENT *content = xcc_new_content(self);
        if(!content) {
            XccError(xcc_errcode(self), xcc_error(self));
        }

        return content;
    }

    %newobject query;
    XCC_RES* query(const char *xquery) {
        XCC_RES *result = xcc_query(self, xquery);
        if(!result) {
            XccError(xcc_errcode(self), xcc_error(self));
        }

        return result;
    }

    %newobject invoke;
    XCC_RES* invoke(const char *module) {
        XCC_RES *result = xcc_invoke(self, module);
        if(!result) {
            XccError(xcc_errcode(self), xcc_error(self));
        }

        return result;
    }

    %newobject spawn;
    XCC_RES* spawn(const char *module) {
        XCC_RES *result = xcc_spawn(self, module);
        if(!result) {
            XccError(xcc_errcode(self), xcc_error(self));
        }

        return result;
    }

    }
} XCC_SESS;

#ifdef SWIGPERL
%module "Xcc::Request"
#endif SWIGPERL
typedef struct {
    %extend {

    XCC_REQ() {
        XccError(XCC_ERROR_INVALID_ARG, "You can't create new Request objects. You must use methods from Session");
        return NULL;
    }

    ~XCC_REQ() {
        xcc_free_request(self);
    }

    %newobject submit;
    XCC_RES* submit() {
        XCC_RES *result = xcc_submit_request(self);
        if(!result) {
            if(XCC_ERROR_XQUERY == xcc_req_errcode(self)) {
                XccErrorXQuery(xcc_req_error(self));
            } else {
                XccError(xcc_req_errcode(self), xcc_req_error(self));
            }
        }

        return result;
    }

#ifdef SWIGRUBY
    %rename("bufsize=") set_bufsize(int size);
#endif SWIGRUBY
    void set_bufsize(int size) {
        XCC_ERROR_CODE ret = xcc_request_setop(self, XCC_REQ_BUFSIZE, size);
        if(ret != XCC_OK) {
            XccError(ret, xcc_error_message(ret));
        }
    }

#ifdef SWIGRUBY
    %rename("timeout=") set_timeout(int seconds);
#endif
    void set_timeout(int seconds) {
        XCC_ERROR_CODE ret = xcc_request_setop(self, XCC_REQ_TIMEOUT, seconds);
        if(ret != XCC_OK) {
            XccError(ret, xcc_error_message(ret));
        }
    }

#ifdef SWIGRUBY
    %rename("dbname=") set_dbname(const char *name);
#endif
    void set_dbname(const char *name) {
        XCC_ERROR_CODE ret = xcc_request_setop(self, XCC_REQ_DBNAME, name);
        if(ret != XCC_OK) {
            XccError(ret, xcc_error_message(ret));
        }
    }

#ifdef SWIGRUBY
    %rename("dbid=") set_dbid(const char *id);
#endif
    void set_dbid(const char *id) {
        XCC_ERROR_CODE ret = xcc_request_setop(self, XCC_REQ_DBID, id);
        if(ret != XCC_OK) {
            XccError(ret, xcc_error_message(ret));
        }
    }

#ifdef SWIGRUBY
    %rename("timestamp=") set_timestamp(const char *stamp);
#endif
    void set_timestamp(const char *stamp) {
        XCC_ERROR_CODE ret = xcc_request_setop(self, XCC_REQ_TIMESTAMP, stamp);
        if(ret != XCC_OK) {
            XccError(ret, xcc_error_message(ret));
        }
    }

#ifdef SWIGRUBY
    %rename("name=") set_name(const char *name);
#endif
    void set_name(const char *name) {
        XCC_ERROR_CODE ret = xcc_request_setop(self, XCC_REQ_NAME, name);
        if(ret != XCC_OK) {
            XccError(ret, xcc_error_message(ret));
        }
    }

    void set_variable(int schema_type, const char *ns, const char *name, const char *value) {
        XCC_ERROR_CODE ret = xcc_setvar_generic(self, schema_type, ns, name, value);
        if(ret != XCC_OK) {
            XccError(ret, xcc_error_message(ret));
        }
    }

    }
} XCC_REQ;

#ifdef SWIGPERL
%module "Xcc::Result"
#endif SWIGPERL
typedef struct {
    %extend {

    XCC_RES() {
        XccError(XCC_ERROR_INVALID_ARG, "You can't create new Result objects. You must use call submit");
        return NULL;
    }

    ~XCC_RES() {
        xcc_free_result(self);
    }

#ifdef SWIGRUBY
    %predicate has_next();
#endif
    int has_next() {
        return xcc_has_next(self);
    }

    %newobject next_item;
    XCC_ITEM* next_item() {
        return xcc_next_item(self);
    }

#ifdef SWIGRUBY
    void each() {
        while(XCC_RES_has_next(self)) {
            rb_yield(SWIG_NewPointerObj((void *) XCC_RES_next_item(self), SWIGTYPE_p_XCC_ITEM,1));
        }
    }
#endif

    }
} XCC_RES;

#ifdef SWIGPERL
%module "Xcc::Item"
#endif
typedef struct {
    %extend {

    XCC_ITEM() {
        XccError(XCC_ERROR_INVALID_ARG, "You can't create new Item objects. You must use call Result.next_item()");
        return NULL;
    }

    ~XCC_ITEM() {
        xcc_free_item(self);
    }

#ifdef SWIGRUBY
    %rename("to_s") to_string();
#endif
#ifdef SWIGPYTHON
    %rename("__str__") to_string();
#endif
    const char* to_string() {
        return xcc_as_string(self);
    }

#ifdef SWIGRUBY
    %rename("to_f") to_double();
#endif
    double to_double() {
        return xcc_as_double(self);
    }

#ifdef SWIGRUBY
    %rename("to_i") to_int();
#endif
    int to_int() {
        return xcc_as_int(self);
    }

    XCC_SCHEMA_TYPE type() {
        return xcc_item_type(self);
    }

    }
} XCC_ITEM;

#ifdef SWIGPERL
%module "Xcc::Content"
#endif
typedef struct {
    %extend {

    XCC_CONTENT() {
        XccError(XCC_ERROR_INVALID_ARG, "You can't create new Content objects. You must use methods from Sesion");
        return NULL;
    }

    ~XCC_CONTENT() {
        xcc_free_content(self);
    }

    void clear_collections() {
        xcc_clear_collections(self);
    }

    void clear_permissions() {
        xcc_clear_permissions(self);
    }

    void add_collection(const char *col) {
        XCC_ERROR_CODE ret = xcc_content_addcol(self, col);
        if(ret != XCC_OK) {
            XccError(ret, xcc_error_message(ret));
        }
    }

    void add_permission(int capability, const char *role) {
        XCC_ERROR_CODE ret = xcc_content_addperm(self, capability, role);
        if(ret != XCC_OK) {
            XccError(ret, xcc_error_message(ret));
        }
    }

    xcc_off_t insert() {
        xcc_off_t bytes = xcc_insert_content(self);
        if(!bytes) {
            XccError(xcc_content_errcode(self), xcc_content_error(self));
        }

        return bytes;
    }

#ifdef SWIGRUBY
    %rename("file_name=") set_file_name(const char *fname);
#endif
    void set_file_name(const char *fname) {
        XCC_ERROR_CODE ret = xcc_content_setop(self, XCC_CONTENT_OPT_FILE, fname);
        if(ret != XCC_OK) {
            XccError(ret, xcc_error_message(ret));
        }
    }

#ifdef SWIGRUBY
    %rename("data=") set_data(const char *str);
#endif
    void set_data(const char *str) {
        XCC_ERROR_CODE ret = xcc_content_setop(self, XCC_CONTENT_OPT_DATA, str, -1L);
        if(ret != XCC_OK) {
            XccError(ret, xcc_error_message(ret));
        }
    }

#ifdef SWIGRUBY
    %rename("namespace=") set_namespace(const char *ns);
#endif
    void set_namespace(const char *ns) {
        XCC_ERROR_CODE ret = xcc_content_setop(self, XCC_CONTENT_OPT_NS, ns);
        if(ret != XCC_OK) {
            XccError(ret, xcc_error_message(ret));
        }
    }

#ifdef SWIGRUBY
    %rename("uri=") set_uri(const char *uri);
#endif
    void set_uri(const char *uri) {
        XCC_ERROR_CODE ret = xcc_content_setop(self, XCC_CONTENT_OPT_URI, uri);
        if(ret != XCC_OK) {
            XccError(ret, xcc_error_message(ret));
        }
    }

#ifdef SWIGRUBY
    %rename("timeout=") set_timeout(int seconds);
#endif
    void set_timeout(int seconds) {
        XCC_ERROR_CODE ret = xcc_content_setop(self, XCC_CONTENT_OPT_TIMEOUT, seconds);
        if(ret != XCC_OK) {
            XccError(ret, xcc_error_message(ret));
        }
    }

#ifdef SWIGRUBY
    %rename("format=") set_format(int fmt);
#endif    
    void set_format(int fmt) {
        XCC_ERROR_CODE ret = xcc_content_setop(self, XCC_CONTENT_OPT_FORMAT, fmt);
        if(ret != XCC_OK) {
            XccError(ret, xcc_error_message(ret));
        }
    }

#ifdef SWIGRUBY
    %rename("repair=") set_repair(int rpr);
#endif
    void set_repair(int rpr) {
        XCC_ERROR_CODE ret = xcc_content_setop(self, XCC_CONTENT_OPT_REPAIR, rpr);
        if(ret != XCC_OK) {
            XccError(ret, xcc_error_message(ret));
        }
    }

#ifdef SWIGRUBY
    %rename("quality=") set_quality(int qlty);
#endif
    void set_quality(int qlty) {
        XCC_ERROR_CODE ret = xcc_content_setop(self, XCC_CONTENT_OPT_QUALITY, qlty);
        if(ret != XCC_OK) {
            XccError(ret, xcc_error_message(ret));
        }
    }

    }
} XCC_CONTENT;

#ifdef SWIGPYTHON
%pythoncode %{
class XccError(StandardError):

    """Base exception class"""

    code = ""
    message = ""

    def __str__(self):
        return "%d - %s" % (self.code, self.message)

class Error(XccError):

    """Xcc Exception"""

class XQueryError(XccError):

    """Exception thrown when problem with issuing XQuery"""

del Exception, StandardError
%}
#endif
