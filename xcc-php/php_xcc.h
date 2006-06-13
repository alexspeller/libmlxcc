/*=============================================================================
 *
 * Copyright 2005 Andrew Bruno <aeb@qnot.org>
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

#ifndef PHP_XCC_H
#define PHP_XCC_H

#define PHP_XCC_VERSION "0.5.3"
#define PHP_XCC_EXTNAME "xcc"

extern zend_module_entry xcc_module_entry;
#define phpext_xcc_ptr &xcc_module_entry

#ifdef PHP_WIN32
#define PHP_XCC_API __declspec(dllexport)
#else
#define PHP_XCC_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(xcc);
PHP_MSHUTDOWN_FUNCTION(xcc);
PHP_RINIT_FUNCTION(xcc);
PHP_RSHUTDOWN_FUNCTION(xcc);
PHP_MINFO_FUNCTION(xcc);

PHP_FUNCTION(xcc_new_session);
PHP_FUNCTION(xcc_server_info);
PHP_FUNCTION(xcc_submit_query);
PHP_FUNCTION(xcc_fetch_next);
PHP_FUNCTION(xcc_new_adhoc_query);
PHP_FUNCTION(xcc_new_module_invoke);
PHP_FUNCTION(xcc_new_module_spawn);
PHP_FUNCTION(xcc_content_from_file);
PHP_FUNCTION(xcc_content_from_string);
PHP_FUNCTION(xcc_add_permission);
PHP_FUNCTION(xcc_set_variable);
PHP_FUNCTION(xcc_content_collections);
PHP_FUNCTION(xcc_content_quality);
PHP_FUNCTION(xcc_content_namespace);
PHP_FUNCTION(xcc_content_repair);
PHP_FUNCTION(xcc_content_format);
PHP_FUNCTION(xcc_submit_request);
PHP_FUNCTION(xcc_insert_content);
PHP_FUNCTION(xcc_error);
PHP_FUNCTION(xcc_close);

/* GLOBALS */
ZEND_BEGIN_MODULE_GLOBALS(xcc)
	long default_link;
ZEND_END_MODULE_GLOBALS(xcc)

/* In every utility function you add that needs to use variables 
   in php_xcc_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as XCC_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define XCC_G(v) TSRMG(xcc_globals_id, zend_xcc_globals *, v)
#else
#define XCC_G(v) (xcc_globals.v)
#endif

#endif	/* PHP_XCC_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */
