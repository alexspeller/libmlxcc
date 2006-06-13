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
#include <sys/types.h>
#include <pthread.h>
#include <marklogic/xcc.h>
#include "test_auth.h"

typedef struct {
	int id;
} st_thread;

void *run_query(void *thread) {
    XCC_SESS *session;
    XCC_REQ *request;
    XCC_RES *result;
    XCC_ITEM *item;

	st_thread *t=(st_thread *)thread;
	printf("Running query for thread %d\n", t->id);

    session = xcc_new_session(TEST_USER, TEST_PASSWORD, TEST_HOST, NULL, TEST_PORT);
    if(!session) {
        fprintf(stderr, "Failed to create session: %d - %s\n", xcc_errcode(session), xcc_error(session));
        return NULL;
    }

    /* request = xcc_new_adhoc_query(session, "('Hello', xdmp:sleep(2000))"); */
    request = xcc_new_adhoc_query(session, "(for $i in input() return base-uri($i)), xdmp:sleep(2000)");
    if(!request) {
        fprintf(stderr, "Error: %d - %s\n", xcc_errcode(session), xcc_error(session));
        xcc_free_session(session);
        return NULL;
    }

    if(!(result = xcc_submit_request(request))) {
        if(XCC_ERROR_XQUERY == xcc_errcode(session)) {
            fprintf(stderr, "XQuery error: %s\n", xcc_error(session));
        } else {
            fprintf(stderr, "Request failed: %d - %s\n", xcc_errcode(session), xcc_error(session));
        }

        xcc_free_request(request);
        xcc_free_session(session);
        return NULL;
    }

    while(xcc_has_next(result)) {
        item = xcc_next_item(result);
        if(XCC_TYPE_STRING == xcc_item_type(item)) {
            printf("Result from thread %d: %s", t->id, xcc_as_string(item));
        }

        xcc_free_item(item);
    }

    xcc_free_result(result);
    xcc_free_request(request);
    xcc_free_session(session);
	return (NULL);
}

int main() {
    int n,i;

    /* Number of threads to run - hard code to 10 for now */
    n = 10;
    pthread_t *threads;
    pthread_attr_t pthread_custom_attr;
    st_thread *p;

    threads=(pthread_t *)malloc(n*sizeof(*threads));
    pthread_attr_init(&pthread_custom_attr);
    p = (st_thread *)malloc(sizeof(st_thread)*n);

    /* Create the threads */
    for(i = 0; i < n; i++) {
        p[i].id = i;
        pthread_create(&threads[i], &pthread_custom_attr, run_query, (void *)(p+i));
    }

    /* Synchronize the completion of each thread. */
    for(i = 0; i < n; i++) {
        pthread_join(threads[i],NULL);
    }

    free(p);
    return 0;
}
