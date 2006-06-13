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

#ifndef MARKLOGIC_XCC_H
#define MARKLOGIC_XCC_H

#include <marklogic/globals.h>
#include <marklogic/xcc_strbuf.h>

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct st_xcc_session XCC_SESS;
typedef struct st_xcc_variable XCC_VAR;
typedef struct st_xcc_request XCC_REQ;
typedef struct st_xcc_permission XCC_PERM;
typedef struct st_xcc_collection XCC_COL;
typedef struct st_xcc_content XCC_CONTENT;
typedef struct st_xcc_result XCC_RES;
typedef struct st_xcc_item XCC_ITEM;

/** @defgroup PublicAPI */

/** @ingroup PublicAPI
 * Create a new session with the provided connection information
 *
 * @param user the user to connect as
 * @param password  password for the user
 * @param host the hostname of the Mark Logic server (defaults to localhost)
 * @param db the default database to connect to
 * @param port the port of the Mark Logic server (defaults to 8003)
 *
 * @return a new xcc session pointer or NULL on failure.
 */
XCC_SESS* xcc_new_session(const char *user, 
                            const char *password, 
                            const char *host, 
                            const char *db, 
                            int port); 

/** @ingroup PublicAPI
 * Closes a session with a Mark Logic server and free up associated memory.
 *
 * @param session a pointer to an xcc session
 */
void xcc_free_session(XCC_SESS *session);

/** @ingroup PublicAPI
 * Gets the last error message that occurred within the given session
 *
 * @param session a pointer to an xcc session
 *
 * @return the error message
 */
const char* xcc_error(XCC_SESS *session);

/** @ingroup PublicAPI
 * Gets the last error message that occurred within the given request
 *
 * @param request a pointer to an xcc request
 *
 * @return the error message
 */
const char* xcc_req_error(XCC_REQ *request);

/** @ingroup PublicAPI
 * Gets the last error message that occurred for the given content
 *
 * @param content a pointer to an xcc content 
 *
 * @return the error message
 */
const char* xcc_content_error(XCC_CONTENT *content);

/** @ingroup PublicAPI
 * Gets the last error code that occurred within the given session
 *
 * @param session a pointer to an xcc session
 *
 * @return the XCC_ERROR_CODE error code
 */
XCC_ERROR_CODE xcc_errcode(XCC_SESS *session);

/** @ingroup PublicAPI
 * Gets the last error code that occurred within the given request
 *
 * @param request a pointer to an xcc request
 *
 * @return the XCC_ERROR_CODE error code
 */
XCC_ERROR_CODE xcc_req_errcode(XCC_REQ *request);

/** @ingroup PublicAPI
 * Gets the last error code that occurred for the given content
 *
 * @param content a pointer to an xcc content
 *
 * @return the XCC_ERROR_CODE error code
 */
XCC_ERROR_CODE xcc_content_errcode(XCC_CONTENT *content);

/** @ingroup PublicAPI
 * Create a new adhoc query request.
 *
 * @param session a pointer to an xcc session
 * @param xquery the query 
 *
 * @return a pointer to a new request or NULL on failure
 */
XCC_REQ* xcc_new_adhoc_query(XCC_SESS *session, const char *xquery);

/** @ingroup PublicAPI
 * Create a new module invoke request.
 *
 * @param session a pointer to an xcc session
 * @param module the module to invoke 
 *
 * @return a pointer to a new request or NULL on failure
 */
XCC_REQ* xcc_new_module_invoke(XCC_SESS *session, const char *module);

/** @ingroup PublicAPI
 * Create a new module spawn request.
 *
 * @param session a pointer to an xcc session
 * @param module the module to spawn 
 *
 * @return a pointer to a new request or NULL on failure
 */
XCC_REQ* xcc_new_module_spawn(XCC_SESS *session, const char *module);

/** @ingroup PublicAPI
 * Free up a request
 *
 * @param request a pointer to a request
 */
void xcc_free_request(XCC_REQ *request);

/** @ingroup PublicAPI
 * Resets the external variable list for a request. Call this function
 * when you want to reset values for external variables.
 *
 * @param request a pointer to a request
 */
void xcc_clear_varlist(XCC_REQ *request);

/** @ingroup PublicAPI
 * Set options for a request.
 *
 * Valid options are defined here #XCC_REQ_OPTION:
 *  XCC_REQ_BUFSIZE - the buffer size to use when reading data from the server
 *  XCC_REQ_TIMEOUT - the timeout in seconds to wait for a response from the server 
 *
 * @param request a pointer to a request
 * @param opt a valid XCC_REQ_OPTION
 * @param param value for the option
 *
 * @return XCC_OK on success or an #XCC_ERROR_CODE on failure
 */
XCC_ERROR_CODE xcc_request_setop(XCC_REQ *request, XCC_REQ_OPTION opt, ...);

/** @ingroup PublicAPI
 * Set a value for an external variable. The last argument is the value of the variable. It
 * must match the schema type of the variable you are setting. So for example, if you are setting
 * an external variable of type XCC_TYPE_STRING, you must pass in a char * as the last arg. If 
 * you are setting an external variable of type XCC_TYPE_INTEGER you must pass in an int as the 
 * last argument.
 *
 * @param request a pointer to a request
 * @param type a valid #XCC_SCHEMA_TYPE
 * @param ns the namespace for the variable
 * @param name the name for the variable
 * @param param value for the variable
 *
 * @return XCC_OK on success or an #XCC_ERROR_CODE on failure
 */
XCC_ERROR_CODE xcc_setvar(XCC_REQ *request, 
                            XCC_SCHEMA_TYPE type, 
                            const char *ns, 
                            const char *name, ...);

/** @ingroup PublicAPI
 * Set a value for an external variable. The schema type can be any valid type and 
 * the value for the variable is passed in last argument as a char *.
 *
 * @param request a pointer to a request
 * @param type a valid #XCC_SCHEMA_TYPE
 * @param ns the namespace for the variable
 * @param name the name for the variable
 * @param val for the variable as a string
 *
 * @return XCC_OK on success or an #XCC_ERROR_CODE on failure
 */
XCC_ERROR_CODE xcc_setvar_generic(XCC_REQ *request, 
                                    XCC_SCHEMA_TYPE type, 
                                    const char *ns, 
                                    const char *name, 
                                    const char *val);

/** @ingroup PublicAPI
 * Submit a request to the mark logic server
 *
 * @param request a pointer to a request
 *
 * @return a pointer to an xcc result set or NULL on failure
 */
XCC_RES* xcc_submit_request(XCC_REQ *request); 

/** @ingroup PublicAPI
 * Execute a query
 *
 * @param session a pointer to an xcc session
 * @param query the query to run
 *
 * @return a pointer to an xcc result set or NULL on failure
 */
XCC_RES* xcc_query(XCC_SESS *session, const char *query); 

/** @ingroup PublicAPI
 * Invoke a module on the server
 *
 * @param session a pointer to an xcc session
 * @param module the module to invoke. ex. 'mymodule.xqy'
 *
 * @return a pointer to an xcc result set or NULL on failure
 */
XCC_RES* xcc_invoke(XCC_SESS *session, const char *module); 

/** @ingroup PublicAPI
 * Spawn a module on the server
 *
 * @param session a pointer to an xcc session
 * @param module the module to Spawn. ex. 'mymodule.xqy'
 *
 * @return a pointer to an (empty) xcc result set or NULL on failure
 */
XCC_RES* xcc_spawn(XCC_SESS *session, const char *module); 

/** @ingroup PublicAPI
 * Free an xcc result set.
 *
 * @param res a pointer to an xcc result set
 */
void xcc_free_result(XCC_RES *res);

/** @ingroup PublicAPI
 * Retrieve the next item in an xcc result set. Call #xcc_has_next() to check
 * and see if the result set has any more items.
 *
 * @param res a pointer to an xcc result set
 *
 * @return a pointer to an xcc result item or NULL on failure
 */
XCC_ITEM* xcc_next_item(XCC_RES *res);

/** @ingroup PublicAPI
 * Free an xcc result item.
 *
 * @param item a pointer to an xcc result item
 */
void xcc_free_item(XCC_ITEM *item);

/** @ingroup PublicAPI
 * Check to see if the given xcc result set has any more items.

 * @param res a pointer to an xcc result set
 *
 * @return 1 (TRUE) if result set has any more items else 0 (FALSE)
 */
int xcc_has_next(XCC_RES *res);

/** @ingroup PublicAPI
 * Retrieve the given xcc result item as a string
 *
 * @param item a pointer to an xcc result item
 *
 * @return the item as a string
 */
const char* xcc_as_string(XCC_ITEM *item);

/** @ingroup PublicAPI
 * Retrieve the given xcc result item as text. Currently same as #xcc_as_string().
 *
 * @param item a pointer to an xcc result item
 *
 * @return the item as a text
 */
const char* xcc_as_text(XCC_ITEM *item);

/** @ingroup PublicAPI
 * Retrieve the given xcc result item as a double.
 *
 * @param item a pointer to an xcc result item
 *
 * @return the item as a double
 */
double xcc_as_double(XCC_ITEM *item);

/** @ingroup PublicAPI
 * Retrieve the given xcc result item as a integer.
 *
 * @param item a pointer to an xcc result item
 *
 * @return the item as an int
 */
int xcc_as_int(XCC_ITEM *item);

/** @ingroup PublicAPI
 * Retrieve the given xcc result item as a long.
 *
 * @param item a pointer to an xcc result item
 *
 * @return the item as an long
 */
long xcc_as_long(XCC_ITEM *item);

/** @ingroup PublicAPI
 * Retrieve the given xcc result item as a boolean.
 *
 * @param item a pointer to an xcc result item
 *
 * @return 1 (TRUE) or 0 (FALSE)
 */
int xcc_as_bool(XCC_ITEM *item);

/** @ingroup PublicAPI
 * Free content.
 *
 * @param content a pointer to content
 */
void xcc_free_content(XCC_CONTENT *content);

/** @ingroup PublicAPI
 * Clears the permission list on the given content
 *
 * @param content a pointer to content
 */
void xcc_clear_permissions(XCC_CONTENT *content);

/** @ingroup PublicAPI
 * Clears the collection list the given content
 *
 * @param content a pointer to content
 */
void xcc_clear_collections(XCC_CONTENT *content);

/** @ingroup PublicAPI
 * Add permissions to content
 *
 * @param content a pointer content
 * @param capability the #XCC_PERMISSION for the permission. read, insert, update
 * @param role the role for this permission
 *
 * @return XCC_OK on success or an #XCC_ERROR_CODE on failure
 */
XCC_ERROR_CODE xcc_content_addperm(XCC_CONTENT *content, 
                                     XCC_PERMISSION capability, 
                                     const char *role);

/** @ingroup PublicAPI
 * Add content to a collection 
 *
 * @param content a pointer to content
 * @param name the name of the collection
 *
 * @return XCC_OK on success or an #XCC_ERROR_CODE on failure
 */
XCC_ERROR_CODE xcc_content_addcol(XCC_CONTENT *content, const char *name);

/** @ingroup PublicAPI
 * Set options for content loading
 *
 * Valid options are defined here #XCC_CONTENT_OPTION:
 *
 * @param content a pointer to content
 * @param opt a valid XCC_DOC_OPTION
 * @param param value for the option
 *
 * @return XCC_OK on success or an #XCC_ERROR_CODE on failure
 */
XCC_ERROR_CODE xcc_content_setop(XCC_CONTENT *content, XCC_CONTENT_OPTION opt, ...);

/** @ingroup PublicAPI
 * Create new content for inserting into database.
 *
 * @param session a pointer to a session
 *
 * @return a pointer to a XCC_CONTENT or NULL on failure
 */
XCC_CONTENT* xcc_new_content(XCC_SESS *session);

/** @ingroup PublicAPI
 * Inserts content into the database
 *
 * @param content a pointer to content
 *
 * @return the number of bytes inserted or 0 on failure
 */
xcc_off_t xcc_insert_content(XCC_CONTENT *content);

/** @ingroup PublicAPI
 * Get a descriptive error message for the given #XCC_ERROR_CODE 
 *
 * @param code an #XCC_ERROR_CODE
 *
 * @return the error message for the error code
 */
const char* xcc_error_message(XCC_ERROR_CODE code);

/** @ingroup PublicAPI
 * Retrieve the given xcc result item as a void * for access to raw item data.
 * This is usefull when binary data is returned.
 *
 * @param item a pointer to an xcc result item
 *
 * @return the item data as a void *
 */
const void* xcc_as_binary(XCC_ITEM *item);

/** @ingroup PublicAPI
 * Retrieve information about the Mark Logic Server.
 * Returns a string in the format: 
 *
 * @param session a pointer to an xcc session
 *
 * @return the server information string
 */
const char* xcc_server_info(XCC_SESS *session);

/** @ingroup PublicAPI
 * Returns the size in bytes of the item data
 *
 * @param item a pointer to an xcc result item
 *
 * @return the size in bytes of the item data
 */
int xcc_item_size(XCC_ITEM *item);

/** @ingroup PublicAPI
 * Returns the schema type of the item
 *
 * @param item a pointer to an xcc result item
 *
 * @return the xcc schema type
 */
XCC_SCHEMA_TYPE xcc_item_type(XCC_ITEM *item);

#ifdef  __cplusplus
}
#endif

#endif /* xcc.h */
