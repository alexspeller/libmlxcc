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
 * load.c - content loading example.
 *
 *============================================================================*/

#include <stdio.h>
#include <marklogic/xcc.h>
#include "test_auth.h"

int main() {
    XCC_SESS *session;
    XCC_CONTENT *content;
    XCC_ERROR_CODE errcode;
    long bytes_inserted;

    /* create new session */
    session = xcc_new_session(TEST_USER, TEST_PASSWORD, TEST_HOST, NULL, TEST_PORT);
    if(!session) {
        fprintf(stderr, "Failed to create session: %d - %s\n", xcc_errcode(session), xcc_error(session));
        return -1;
    }

    /* create new content to insert. */
    if(!(content = xcc_new_content(session))) {
        fprintf(stderr, "Error: %d - %s\n", xcc_errcode(session), xcc_error(session));
        xcc_free_session(session);
        return -1;
    }

    /* set content options -- See API Docs */

    /* load from file */
    errcode = xcc_content_setop(content, XCC_CONTENT_OPT_FILE, "data.xml");
    if(errcode != XCC_OK) {
        printf("Error: %s\n", xcc_error_message(errcode));
        xcc_free_content(content);
        xcc_free_session(session);
        return -1;
    }

    xcc_content_setop(content, XCC_CONTENT_OPT_URI, "http://mysite.com/files/data.xml");
    xcc_content_setop(content, XCC_CONTENT_OPT_REPAIR, XCC_CONTENT_ERROR_CORRECTION_NONE);

    /* insert the content */
    if(!(bytes_inserted = xcc_insert_content(content))) {
        fprintf(stderr, "Error: %d - %s\n", xcc_errcode(session), xcc_error(session));
        xcc_free_content(content);
        xcc_free_session(session);
        return -1;
    }

    printf("Inserted %ld bytes.\n", bytes_inserted);

    /* free up memory. xcc_insert_content will free content for you upon successfull load */
    xcc_free_content(content);
    xcc_free_session(session);

    return 0;
}
