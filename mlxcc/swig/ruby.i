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
VALUE _xccRubyError;
VALUE _xccRubyXQueryError;

static VALUE _xcc_ruby_error_code(VALUE obj) {
    return rb_iv_get(obj, "code");
}

static VALUE _xcc_ruby_error_to_xml(VALUE obj) {
    return rb_iv_get(obj, "error_xml");
}

#define XccError(num, msg) \
     VALUE e = rb_exc_new2(_xccRubyError, msg); \
     rb_iv_set(e, "code", INT2FIX(num)); \
     rb_exc_raise(e);

#define XccErrorXQuery(error_xml) \
     VALUE e = rb_exc_new2(_xccRubyXQueryError, "You have an error in your XQuery"); \
     rb_iv_set(e, "code", INT2FIX(XCC_ERROR_XQUERY)); \
     rb_iv_set(e, "error_xml", rb_tainted_str_new2(error_xml)); \
     rb_exc_raise(e);
%}
