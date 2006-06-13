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
 * query.c - Simple XQuery test with external variables
 *
 *============================================================================*/

#include <stdio.h>
#include <marklogic/xcc.h>
#include "test_auth.h"

int main() {
    XCC_SESS *session;
    XCC_REQ *request;
    XCC_RES *result;
    XCC_ITEM *item;
    const char *query;

    /* create new session */
    session = xcc_new_session(TEST_USER, TEST_PASSWORD, TEST_HOST, NULL, TEST_PORT);
    if(!session) {
        fprintf(stderr, "Failed to create session: %d - %s\n", xcc_errcode(session), xcc_error(session));
        return -1;
    }

    printf("Connected to: %s", xcc_server_info(session));

    /* hardcode XQuery statement */
    query = "define variable $x as xs:integer external define variable $y as xs:string external fn:concat(xs:string($x), ' ', $y)";

    /* create request */
    request = xcc_new_adhoc_query(session, query);
    if(!request) {
        fprintf(stderr, "Error: %d - %s\n", xcc_errcode(session), xcc_error(session));
        xcc_free_session(session);
        return -1;
    }

    /* set request options - See API docs */
    xcc_request_setop(request, XCC_REQ_BUFSIZE, 8190);

    /* set external variables */
    xcc_setvar(request, XCC_TYPE_INTEGER, NULL, "x", 12);
    xcc_setvar_generic(request, XCC_TYPE_STRING, NULL, "y", " - a dozen");

    /* execute query */
    if(!(result = xcc_submit_request(request))) {
        if(XCC_ERROR_XQUERY == xcc_errcode(session)) {
            /* XQuery error. xcc_error()  will return the XML error string from the server */
            fprintf(stderr, "XQuery error: %s\n", xcc_error(session));
        } else {
            /* something bad happened */
            fprintf(stderr, "Request failed: %d - %s\n", xcc_errcode(session), xcc_error(session));
        }

        /* free everything up */
        xcc_free_request(request);
        xcc_free_session(session);
        return -1;
    }

    /* loop through results */
    while(xcc_has_next(result)) {
        item = xcc_next_item(result);
        if(XCC_TYPE_STRING == xcc_item_type(item)) {
            printf("Result: %s", xcc_as_string(item));
        }

        /* free result item */
        xcc_free_item(item);
    }

    /* don't forget to free  */
    xcc_free_result(result);
    xcc_free_request(request);
    xcc_free_session(session);

    return 0;
}
