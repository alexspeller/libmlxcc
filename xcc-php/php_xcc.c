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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/php_string.h"
#include <marklogic/xcc.h>
#include "php_xcc.h"

#define CHECK_LINK(link) { if (link==-1) { php_error_docref(NULL TSRMLS_CC, E_WARNING, "A link to the connection could not be established"); RETURN_FALSE; } }

#define REGISTER_XCC_CONSTANT(__c) REGISTER_LONG_CONSTANT(#__c, __c, CONST_CS | CONST_PERSISTENT)

typedef struct _php_xcc_sess {
	XCC_SESS *session;
} php_xcc_sess;

typedef struct _php_xcc_req {
	XCC_REQ *request;
} php_xcc_req;

typedef struct _php_xcc_res {
	XCC_RES *result;
} php_xcc_res;

typedef struct _php_xcc_content {
	XCC_CONTENT *content;
} php_xcc_content;

ZEND_DECLARE_MODULE_GLOBALS(xcc)

/* True global resources - no need for thread safety here */
static int le_session, le_result, le_request, le_content;

/* {{{ xcc_functions[]
 *
 * Every user visible function must have an entry in xcc_functions[].
 */
function_entry xcc_functions[] = {
	PHP_FE(xcc_new_session,						NULL)
	PHP_FE(xcc_server_info,						NULL)
	PHP_FE(xcc_submit_query,					NULL)
	PHP_FE(xcc_fetch_next,						NULL)
	PHP_FE(xcc_new_adhoc_query,					NULL)
	PHP_FE(xcc_new_module_invoke,				NULL)
	PHP_FE(xcc_new_module_spawn,				NULL)
	PHP_FE(xcc_content_from_file,				NULL)
	PHP_FE(xcc_content_from_string,				NULL)
	PHP_FE(xcc_add_permission,					NULL)
	PHP_FE(xcc_set_variable,					NULL)
	PHP_FE(xcc_content_collections,				NULL)
	PHP_FE(xcc_insert_content,					NULL)
	PHP_FE(xcc_content_quality,					NULL)
	PHP_FE(xcc_content_repair,					NULL)
	PHP_FE(xcc_content_format,					NULL)
	PHP_FE(xcc_content_namespace,				NULL)
	PHP_FE(xcc_submit_request,					NULL)
	PHP_FE(xcc_error,							NULL)
	PHP_FE(xcc_close,							NULL)
	{NULL, NULL, NULL}	/* Must be the last line in xcc_functions[] */
};
/* }}} */

/* {{{ xcc_module_entry
 */
zend_module_entry xcc_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	PHP_XCC_EXTNAME,
	xcc_functions,
	ZEND_MINIT(xcc),
	ZEND_MSHUTDOWN(xcc),
	ZEND_RINIT(xcc),
	ZEND_RSHUTDOWN(xcc),	/* Replace with NULL if there's nothing to do at request end */
	ZEND_MINFO(xcc),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_XCC_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_XCC
ZEND_GET_MODULE(xcc)
#endif

/* {{{ ZEND_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
ZEND_INI_BEGIN()
    STD_ZEND_INI_ENTRY("xcc.global_value",      "42", ZEND_INI_ALL, OnUpdateInt, global_value, zend_xcc_globals, xcc_globals)
    STD_ZEND_INI_ENTRY("xcc.global_string", "foobar", ZEND_INI_ALL, OnUpdateString, global_string, zend_xcc_globals, xcc_globals)
ZEND_INI_END()
*/
/* }}} */

/* {{{ php_xcc_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_xcc_init_globals(zend_xcc_globals *xcc_globals)
{
	xcc_globals->global_value = 0;
	xcc_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ _close_xcc_session
 */
static void _close_xcc_session(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	php_xcc_sess *link = (php_xcc_sess *)rsrc->ptr;
	if(link) {
		if(link->session) xcc_free_session(link->session);
		efree(link);
	}
}
/* }}} */

/* {{{ _free_xcc_result
 */
static void _free_xcc_result(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	php_xcc_res *link= (php_xcc_res *)rsrc->ptr;
	if(link) {
		if(link->result) xcc_free_result(link->result);
		efree(link);
	}
}
/* }}} */

/* {{{ _free_xcc_request
 */
static void _free_xcc_request(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	php_xcc_req *link = (php_xcc_req *)rsrc->ptr;
	if(link) {
		if(link->request) xcc_free_request(link->request);
		efree(link);
	}
}
/* }}} */

/* {{{ _free_xcc_content
 */
static void _free_xcc_content(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	php_xcc_content *link = (php_xcc_content *)rsrc->ptr;
	if(link) {
		if(link->content) xcc_free_content(link->content);
		efree(link);
	}
}
/* }}} */


/* {{{ _php_xcc_set_default_link
 */
static void _php_xcc_set_default_link(int id TSRMLS_DC)
{
	if (XCC_G(default_link) != -1) {
		zend_list_delete(XCC_G(default_link));
	}
	XCC_G(default_link) = id;
	zend_list_addref(id);
}
/* }}} */


/* {{{ ZEND_MINIT_FUNCTION
 */
ZEND_MINIT_FUNCTION(xcc)
{
	/* If you have INI entries, uncomment these lines 
	ZEND_INIT_MODULE_GLOBALS(xcc, php_xcc_init_globals, NULL);
	REGISTER_INI_ENTRIES();
	*/
	le_result = zend_register_list_destructors_ex(_free_xcc_result, NULL, "xcc result", module_number);
	le_request = zend_register_list_destructors_ex(_free_xcc_request, NULL, "xcc request", module_number);
	le_content = zend_register_list_destructors_ex(_free_xcc_content, NULL, "xcc content", module_number);
	le_session = zend_register_list_destructors_ex(_close_xcc_session, NULL, "xcc session", module_number);

	/* Constants for content format types */
	REGISTER_XCC_CONSTANT(XCC_CONTENT_FORMAT_XML);
	REGISTER_XCC_CONSTANT(XCC_CONTENT_FORMAT_TEXT);
	REGISTER_XCC_CONSTANT(XCC_CONTENT_FORMAT_BINARY);
	REGISTER_XCC_CONSTANT(XCC_CONTENT_FORMAT_NONE);

	/* Constants for XCC permissions */
	REGISTER_XCC_CONSTANT(XCC_PERMISSION_INSERT);
	REGISTER_XCC_CONSTANT(XCC_PERMISSION_READ);
	REGISTER_XCC_CONSTANT(XCC_PERMISSION_UPDATE);
	REGISTER_XCC_CONSTANT(XCC_PERMISSION_EXECUTE);

	/* Constants for XCC schema types */
	REGISTER_XCC_CONSTANT(XCC_TYPE_ANYURI);
	REGISTER_XCC_CONSTANT(XCC_TYPE_BASE64BINARY);
	REGISTER_XCC_CONSTANT(XCC_TYPE_BINARY);
	REGISTER_XCC_CONSTANT(XCC_TYPE_BOOLEAN);
	REGISTER_XCC_CONSTANT(XCC_TYPE_DATE);
	REGISTER_XCC_CONSTANT(XCC_TYPE_DATETIME);
	REGISTER_XCC_CONSTANT(XCC_TYPE_DAYTIMEDURATION);
	REGISTER_XCC_CONSTANT(XCC_TYPE_DECIMAL);
	REGISTER_XCC_CONSTANT(XCC_TYPE_DOUBLE);
	REGISTER_XCC_CONSTANT(XCC_TYPE_DURATION);
	REGISTER_XCC_CONSTANT(XCC_TYPE_FLOAT);
	REGISTER_XCC_CONSTANT(XCC_TYPE_GDAY);
	REGISTER_XCC_CONSTANT(XCC_TYPE_GMONTH);
	REGISTER_XCC_CONSTANT(XCC_TYPE_GMONTHDAY);
	REGISTER_XCC_CONSTANT(XCC_TYPE_GYEAR);
	REGISTER_XCC_CONSTANT(XCC_TYPE_GYEARMONTH);
	REGISTER_XCC_CONSTANT(XCC_TYPE_HEXBINARY);
	REGISTER_XCC_CONSTANT(XCC_TYPE_INTEGER);
	REGISTER_XCC_CONSTANT(XCC_TYPE_NODE);
	REGISTER_XCC_CONSTANT(XCC_TYPE_QNAME);
	REGISTER_XCC_CONSTANT(XCC_TYPE_STRING);
	REGISTER_XCC_CONSTANT(XCC_TYPE_TEXT);
	REGISTER_XCC_CONSTANT(XCC_TYPE_TIME);
	REGISTER_XCC_CONSTANT(XCC_TYPE_YEARMONTHDURATION);
	REGISTER_XCC_CONSTANT(XCC_TYPE_UNKNOWN);

	return SUCCESS;
}
/* }}} */

/* {{{ ZEND_MSHUTDOWN_FUNCTION
 */
ZEND_MSHUTDOWN_FUNCTION(xcc)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ ZEND_RINIT_FUNCTION
 */
ZEND_RINIT_FUNCTION(xcc)
{
	XCC_G(default_link)=-1;
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ ZEND_RSHUTDOWN_FUNCTION
 */
ZEND_RSHUTDOWN_FUNCTION(xcc)
{
	return SUCCESS;
}
/* }}} */

/* {{{ ZEND_MINFO_FUNCTION
 */
ZEND_MINFO_FUNCTION(xcc)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "mark logic xcc support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */


/* {{{ proto resourceid xcc_new_session(string user, string password, string host, string db, int port)
   Create a new session to a Mark Logic Server */
PHP_FUNCTION(xcc_new_session)
{
	_php_xcc_new_session(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ _php_xcc_new_session
 */
int _php_xcc_new_session(INTERNAL_FUNCTION_PARAMETERS)
{
	char *user = NULL;
	char *host = NULL;
	char *password = NULL;
	char *db = NULL;
	zend_bool check_conn;
	long port;
	int u_len, h_len, p_len, d_len;
	php_xcc_sess *session;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ssssl", 
									&user, 
									&u_len,
									&password,
									&p_len,
									&host,
									&h_len,
									&db,
									&d_len,
									&port) == FAILURE) {
		return;
	}

	session = (php_xcc_sess *)emalloc(sizeof(php_xcc_sess));
	/* create new session */
	if(!(session->session = xcc_new_session(user, password, host, db, port))) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to create new XCC session");
		RETURN_FALSE;
	}

	ZEND_REGISTER_RESOURCE(return_value, session, le_session);
	_php_xcc_set_default_link(Z_LVAL_P(return_value) TSRMLS_CC);
}
/* }}} */

/* {{{ _php_xcc_get_default_link
 */
static int _php_xcc_get_default_link(INTERNAL_FUNCTION_PARAMETERS)
{
	if (XCC_G(default_link)==-1) { /* no link opened yet, implicitly open one */
		ht = 0;
		_php_xcc_new_session(INTERNAL_FUNCTION_PARAM_PASSTHRU);
	}
	return XCC_G(default_link);
}
/* }}} */

/* {{{ proto string xcc_server_info([int link_identifier])
   Return info about Mark Logic Server */
PHP_FUNCTION(xcc_server_info)
{
	zval *xcc_link;
	int id = -1;
	char *str = NULL;
	php_xcc_sess *session;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|r", &xcc_link) == FAILURE) {
		return;
	}

	if (ZEND_NUM_ARGS() == 0) {
		id = _php_xcc_get_default_link(INTERNAL_FUNCTION_PARAM_PASSTHRU);
		CHECK_LINK(id);
	}

	ZEND_FETCH_RESOURCE(session, php_xcc_sess *, &xcc_link, id, "XCC-Session", le_session);
	if ((str = (char *)xcc_server_info(session->session))) {
		RETURN_STRING(str, 1);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto bool xcc_close([int link_identifier])
   Close an XCC session */
PHP_FUNCTION(xcc_close)
{
	zval **xcc_link=NULL;
	int id;
	php_xcc_sess *session;

	switch (ZEND_NUM_ARGS()) {
		case 0:
			id = XCC_G(default_link);
			break;
		case 1:
			if (zend_get_parameters_ex(1, &xcc_link)==FAILURE) {
				RETURN_FALSE;
			}
			id = -1;
			break;
		default:
			WRONG_PARAM_COUNT;
			break;
	}
	
	ZEND_FETCH_RESOURCE(session, php_xcc_sess *, xcc_link, id, "XCC-Session", le_session);

	if (id==-1) { /* explicit resource number */
		zend_list_delete(Z_RESVAL_PP(xcc_link));
	}

	if (id!=-1 
		|| (xcc_link && Z_RESVAL_PP(xcc_link)==XCC_G(default_link))) {
		zend_list_delete(XCC_G(default_link));
		XCC_G(default_link) = -1;
	}

	RETURN_TRUE;
}
/* }}} */

/* {{{ _php_xcc_run_query
 */
static void _php_xcc_run_query(zval **req_link, int link_id, zval *return_value TSRMLS_DC)
{
	php_xcc_req *request;
	php_xcc_res *res;

	ZEND_FETCH_RESOURCE(request, php_xcc_req *, req_link, link_id, "XCC-Request", le_request);

	res = (php_xcc_res *)emalloc(sizeof(php_xcc_res));
    /* submit query */
    if(!(res->result = xcc_submit_request(request->request))) {
		efree(res);
		RETURN_FALSE;
    }

	ZEND_REGISTER_RESOURCE(return_value, res, le_result);
}
/* }}} */

/* {{{ proto int xcc_new_adhoc_query(string query[, int link_identifier])
   create a new request for an adhoc query */
PHP_FUNCTION(xcc_new_adhoc_query)
{
	zval *sess_link;
	zval *req_link;
	int id = -1;
	char *query = NULL;
	int q_len;
	php_xcc_req *request;
	php_xcc_sess *session;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|r", &query, &q_len, &sess_link) == FAILURE) {
		return;
	}

	if (ZEND_NUM_ARGS() == 1) {
		id = _php_xcc_get_default_link(INTERNAL_FUNCTION_PARAM_PASSTHRU);
		CHECK_LINK(id);
	}

	ZEND_FETCH_RESOURCE(session, php_xcc_sess *, &sess_link, id, "XCC-Session", le_session);

	request = (php_xcc_req *)emalloc(sizeof(php_xcc_req));
	/* create adhoc query request */
	if(!(request->request = xcc_new_adhoc_query(session->session, query))) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to create adhoc query request: %d - %s", xcc_errcode(session->session), xcc_error(session->session));
		efree(request);
		RETURN_FALSE;
	}

	ZEND_REGISTER_RESOURCE(return_value, request, le_request);
}
/* }}} */

/* {{{ proto int xcc_new_module_invoke(string module[, int link_identifier])
   create a new request for invoking a module */
PHP_FUNCTION(xcc_new_module_invoke)
{
	zval *sess_link;
	zval *req_link;
	int id = -1;
	char *module = NULL;
	int m_len;
	php_xcc_req *request;
	php_xcc_sess *session;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|r", &module, &m_len, &sess_link) == FAILURE) {
		return;
	}

	if (ZEND_NUM_ARGS() == 1) {
		id = _php_xcc_get_default_link(INTERNAL_FUNCTION_PARAM_PASSTHRU);
		CHECK_LINK(id);
	}

	ZEND_FETCH_RESOURCE(session, php_xcc_sess *, &sess_link, id, "XCC-Session", le_session);

	request = (php_xcc_req *)emalloc(sizeof(php_xcc_req));
	if(!(request->request = xcc_new_module_invoke(session->session, module))) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to create new module invoke request: %d - %s", xcc_errcode(session->session), xcc_error(session->session));
		efree(request);
		RETURN_FALSE;
	}

	ZEND_REGISTER_RESOURCE(return_value, request, le_request);
}
/* }}} */

/* {{{ proto int xcc_new_module_spawn(string module[, int link_identifier])
   create a new request for spawning a module */
PHP_FUNCTION(xcc_new_module_spawn)
{
	zval *sess_link;
	zval *req_link;
	int id = -1;
	char *module = NULL;
	int m_len;
	php_xcc_req *request;
	php_xcc_sess *session;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|r", &module, &m_len, &sess_link) == FAILURE) {
		return;
	}

	if (ZEND_NUM_ARGS() == 1) {
		id = _php_xcc_get_default_link(INTERNAL_FUNCTION_PARAM_PASSTHRU);
		CHECK_LINK(id);
	}

	ZEND_FETCH_RESOURCE(session, php_xcc_sess *, &sess_link, id, "XCC-Session", le_session);

	request = (php_xcc_req *)emalloc(sizeof(php_xcc_req));
	if(!(request->request = xcc_new_module_spawn(session->session, module))) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to create new module spawn request: %d - %s", xcc_errcode(session->session), xcc_error(session->session));
		efree(request);
		RETURN_FALSE;
	}

	ZEND_REGISTER_RESOURCE(return_value, request, le_request);
}
/* }}} */

/* {{{ proto int xcc_submit_request(int link_identifier)
   submit request to database */
PHP_FUNCTION(xcc_submit_request)
{
	zval *req_link;
	php_xcc_req *request;
	int id = -1;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &req_link) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(request, php_xcc_req *, &req_link, id, "XCC-Request", le_request);

	_php_xcc_run_query(&req_link, id, return_value);
}
/* }}} */

/* {{{ proto int xcc_submit_query(string xquery[, int link_identifier])
   Execute an XQuery statement */
PHP_FUNCTION(xcc_submit_query)
{
	zval *sess_link;
	zval *req_link;
	int id = -1;
	int req_id;
	char *query = NULL;
	int q_len;
	php_xcc_req *request;
	php_xcc_sess *session;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|r", &query, &q_len, &sess_link) == FAILURE) {
		return;
	}

	if (ZEND_NUM_ARGS() == 1) {
		id = _php_xcc_get_default_link(INTERNAL_FUNCTION_PARAM_PASSTHRU);
		CHECK_LINK(id);
	}

	ZEND_FETCH_RESOURCE(session, php_xcc_sess *, &sess_link, id, "XCC-Session", le_session);

	request = (php_xcc_req *)emalloc(sizeof(php_xcc_req));
	/* create adhoc query request */
	if(!(request->request = xcc_new_adhoc_query(session->session, query))) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to adhoc query request: %d - %s", xcc_errcode(session->session), xcc_error(session->session));
		efree(request);
		RETURN_FALSE;
	}

	MAKE_STD_ZVAL(req_link);
	req_id = ZEND_REGISTER_RESOURCE(req_link, request, le_request);

	_php_xcc_run_query(&req_link, req_id, return_value);
}
/* }}} */

/* {{{ proto string xcc_error([int linkid])
   Return an xcc error string */
PHP_FUNCTION(xcc_error)
{
	zval *sess_link;
	int id = -1;
	char *str = NULL;
	php_xcc_sess *session;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|r", &sess_link) == FAILURE) {
		return;
	}

	if (ZEND_NUM_ARGS() == 0) {
		id = _php_xcc_get_default_link(INTERNAL_FUNCTION_PARAM_PASSTHRU);
		CHECK_LINK(id);
	}

	ZEND_FETCH_RESOURCE(session, php_xcc_sess *, &sess_link, id, "XCC-Session", le_session);
	if ((str = (char *)xcc_error(session->session))) {
		RETURN_STRING(str, 1);
	} else {
		RETURN_FALSE;
	}
}

/* {{{ proto hash xcc_fetch_next(int link_identifier)
   Return the next result as a hash */
PHP_FUNCTION(xcc_fetch_next)
{
	zval *res_link;
	int id = -1;
	php_xcc_res *res;
	XCC_ITEM *item;
	char *data = NULL;
	char *schema_type = NULL;
	int data_len, type_len;
	int should_copy;


	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &res_link) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(res, php_xcc_res *, &res_link, id, "XCC-Result", le_result);

	if(xcc_has_next(res->result)) {
		item = xcc_next_item(res->result);

		if (array_init(return_value)==FAILURE) {
			RETURN_FALSE;
		}

		if (PG(magic_quotes_runtime)) {
			data = php_addslashes((char *)xcc_as_string(item), xcc_item_size(item), &data_len, 0 TSRMLS_CC);
			should_copy = 0;
		} else {
			data = (char *)xcc_as_string(item);
			data_len = xcc_item_size(item);
			should_copy = 1;
		}
		schema_type = (char *)xcc_schema_type_as_string(xcc_item_type(item));
		if(!schema_type) schema_type = strdup("unknown");
		type_len = sizeof(char)*strlen(schema_type);
		add_assoc_stringl(return_value, "data", data, data_len, should_copy);
		add_assoc_stringl(return_value, "type", schema_type, type_len, 1);
		if(schema_type) free(schema_type);

		/* free result item */
		xcc_free_item(item);
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto int xcc_content_from_file(string filepath, string uri[, int link_identifier])
   create new content for loading from a file path */
PHP_FUNCTION(xcc_content_from_file)
{
	_php_xcc_new_content(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ proto int xcc_content_from_string(string data, string uri[, int link_identifier])
   create new content for loading from a string */
PHP_FUNCTION(xcc_content_from_string)
{
	_php_xcc_new_content(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ _php_xcc_new_content
   */
int _php_xcc_new_content(INTERNAL_FUNCTION_PARAMETERS, int from_string)
{
	zval *sess_link;
	zval *content_link;
	int id = -1;
	char *data = NULL;
	char *uri = NULL;
	int d_len, u_len;
	php_xcc_content *content;
	php_xcc_sess *session;
	XCC_ERROR_CODE code;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, 
								"ss|r", 
								&data, 
								&d_len, 
								&uri, 
								&u_len, 
								&sess_link
								) == FAILURE) {
		return;
	}

	if (ZEND_NUM_ARGS() == 2) {
		id = _php_xcc_get_default_link(INTERNAL_FUNCTION_PARAM_PASSTHRU);
		CHECK_LINK(id);
	}

	ZEND_FETCH_RESOURCE(session, php_xcc_sess *, &sess_link, id, "XCC-Session", le_session);

	content = (php_xcc_content *)emalloc(sizeof(php_xcc_content));
	/* create content handle */
	if(!(content->content = xcc_new_content(session->session))) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to create content: %d - %s", xcc_errcode(session->session), xcc_error(session->session));
		efree(content);
		RETURN_FALSE;
	}

	if(from_string == 1) {
		/* load from string */
		code = xcc_content_setop(content->content, XCC_CONTENT_OPT_DATA, data, (long)d_len);
		if(code != XCC_OK) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to set data for content: %d - %s", code, xcc_error_message(code));
			xcc_free_content(content->content);
			efree(content);
			RETURN_FALSE;
		}
	} else {
		/* load from file */
		code = xcc_content_setop(content->content, XCC_CONTENT_OPT_FILE, data);
		if(code != XCC_OK) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to read file: %d - %s", code, xcc_error_message(code));
			xcc_free_content(content->content);
			efree(content);
			RETURN_FALSE;
		}
	}

	/* set uri */
	code = xcc_content_setop(content->content, XCC_CONTENT_OPT_URI, uri);
	if(code != XCC_OK) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to set uri for content: %d - %s", code, xcc_error_message(code));
		xcc_free_content(content->content);
		efree(content);
		RETURN_FALSE;
	}

	ZEND_REGISTER_RESOURCE(return_value, content, le_content);
}
/* }}} */

/* {{{ proto bool xcc_content_quality(int link_identifier, int quality)
   set the quality for content  */
PHP_FUNCTION(xcc_content_quality)
{
	zval *content_ref;
	int content_id = -1;
	php_xcc_content *content;
	long quality; 
	XCC_ERROR_CODE code;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &content_ref, &quality) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(content, php_xcc_content *, &content_ref, content_id, "XCC-Content", le_content);
	code = xcc_content_setop(content->content, XCC_CONTENT_OPT_QUALITY, (int)quality);
	if(code != XCC_OK) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to set content quality: %d - %s", code, xcc_error_message(code));
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto bool xcc_content_quality(int link_identifier, string ns)
   set the namespace for a content */
PHP_FUNCTION(xcc_content_namespace)
{
	zval *content_ref;
	int content_id = -1;
	php_xcc_content *content;
	char *ns; 
	int n_len; 
	XCC_ERROR_CODE code;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &content_ref, &ns, &n_len) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(content, php_xcc_content *, &content_ref, content_id, "XCC-Content", le_content);
	code = xcc_content_setop(content->content, XCC_CONTENT_OPT_NS, ns);
	if(code != XCC_OK) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to set content namespace: %d - %s", code, xcc_error_message(code));
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto bool xcc_content_repair(int link_identifier, bool repair_flag)
   set to true to have Mark Logic repair the content */
PHP_FUNCTION(xcc_content_repair)
{
	zval *content_ref;
	int content_id = -1;
	php_xcc_content *content;
	zend_bool repair_flag; 
	XCC_ERROR_CODE code;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rb", &content_ref, &repair_flag) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(content, php_xcc_content *, &content_ref, content_id, "XCC-Content", le_content);
	if(repair_flag == 1) {
		code = xcc_content_setop(content->content, XCC_CONTENT_OPT_REPAIR, XCC_CONTENT_ERROR_CORRECTION_FULL);
	} else {
		code = xcc_content_setop(content->content, XCC_CONTENT_OPT_REPAIR, XCC_CONTENT_ERROR_CORRECTION_NONE);
	}

	if(code != XCC_OK) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to set error correction: %d - %s", code, xcc_error_message(code));
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto bool xcc_content_format(int link_identifier, long format)
   set the format for the content */
PHP_FUNCTION(xcc_content_format)
{
	zval *content_ref;
	int content_id = -1;
	php_xcc_content *content;
	long format;
	XCC_ERROR_CODE code;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &content_ref, &format) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(content, php_xcc_content *, &content_ref, content_id, "XCC-Content", le_content);

	code = xcc_content_setop(content->content, XCC_CONTENT_OPT_FORMAT, (int)format);
	if(code != XCC_OK) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to set content format: %d - %s", code, xcc_error_message(code));
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto bool xcc_add_permission(int link_identifier, string role, long permission)
   add a permission to content */
PHP_FUNCTION(xcc_add_permission)
{
	zval *content_ref;
	int content_id = -1;
	php_xcc_content *content;
	char *role = NULL; 
	int r_len;
	long permission;
	XCC_ERROR_CODE code;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rsl", &content_ref, &role, &r_len, &permission) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(content, php_xcc_content *, &content_ref, content_id, "XCC-Content", le_content);
	code = xcc_content_addperm(content->content, (int)permission, role);
	if(code != XCC_OK) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to add permission to content: %d - %s", code, xcc_error_message(code));
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto bool xcc_set_variable(int link_identifier, string name, string value, long type[, string ns])
   set an external variable */
PHP_FUNCTION(xcc_set_variable)
{
	zval *req_ref;
	int req_id = -1;
	php_xcc_req *request;
	char *name = NULL; 
	char *value = NULL; 
	char *ns = NULL;
	long schema_type;
	int n_len, v_len, ns_len;
	XCC_ERROR_CODE code;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rssl|s", 
									&req_ref,
									&name,
									&n_len,
									&value,
									&v_len,
									&schema_type,
									&ns,
									&ns_len
									) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(request, php_xcc_req *, &req_ref, req_id, "XCC-Request", le_request);
	code = xcc_setvar_generic(request->request, (int)schema_type, ns, name, value);
	if(code != XCC_OK) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to set external variable: %d - %s", code, xcc_error_message(code));
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto bool xcc_content_collections(int link_identifier, array collections)
   set the list of collections for a document */
PHP_FUNCTION(xcc_content_collections)
{
	zval *content_ref;
	int content_id = -1;
	php_xcc_content *content;
	zval *collections; 
	zval **item; 
	XCC_ERROR_CODE code;
	HashPosition pos;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ra", &content_ref, &collections) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(content, php_xcc_content *, &content_ref, content_id, "XCC-Content", le_content);

	if(collections && (Z_TYPE_P(collections) == IS_ARRAY)) {
		zend_hash_internal_pointer_reset_ex(HASH_OF(collections), &pos);
		for (;; zend_hash_move_forward_ex(HASH_OF(collections), &pos)) {
			if (zend_hash_get_current_data_ex(HASH_OF(collections), (void**)&item, &pos) == FAILURE)
				break;

			convert_to_string_ex(item);

			code = xcc_content_addcol(content->content, Z_STRVAL_PP(item));
			if(code != XCC_OK) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to add collection to contentument: %d - %s", code, xcc_error_message(code));
				RETURN_FALSE;
			}
		}
	}
}
/* }}} */

/* {{{ proto bool xcc_insert_content(int link_identifier)
   inserts content into into Mark Logic */
PHP_FUNCTION(xcc_insert_content)
{
	zval *content_ref;
	int content_id = -1;
	php_xcc_content *content;
	xcc_off_t bytes_inserted;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &content_ref) == FAILURE) {
		return;
	}

	ZEND_FETCH_RESOURCE(content, php_xcc_content *, &content_ref, content_id, "XCC-Content", le_content);

	/* insert the content */
	if(!(bytes_inserted = xcc_insert_content(content->content))) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to insert content: %d - %s", xcc_content_errcode(content->content), xcc_content_error(content->content));
		RETURN_FALSE;
	}
	

	/* force remove of resource */
	zend_list_delete(content_id);

	if (bytes_inserted < LONG_MAX) {
		RETURN_LONG((long) bytes_inserted);
	} else {
		char *ret;
		int l = spprintf(&ret, 0, XCC_FORMAT_OFF_T, bytes_inserted);
		RETURN_STRINGL(ret, l, 0);
	}
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
