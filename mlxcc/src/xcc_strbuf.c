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

#include <marklogic/xcc_strbuf.h>

XCC_STRBUF* xcc_strbuf_init(XCC_STRBUF *strbuf) {
    if(!strbuf) return NULL;
    strbuf->chunksize = strbuf->unused = strbuf->size = XCC_STRBUF_CHUNKSIZE;
    strbuf->length = 0;
    strbuf->buffer = (char *)malloc(strbuf->size+sizeof(char)); /* leave room for \0 */
    memset(strbuf->buffer, '\0', strbuf->size);
    return strbuf;
}

void xcc_strbuf_free(XCC_STRBUF *strbuf) {
    if(!strbuf) return;
    if(strbuf->buffer) {
        free(strbuf->buffer);
    }
    free(strbuf);
    return;
}

int xcc_strbuf_append(XCC_STRBUF *strbuf, const char *str, int bufsize) {
    int freebytes;
    int newsize;

    if(!str) return 0;
    if(!strbuf) strbuf = xcc_strbuf_init(strbuf);
    freebytes = strbuf->unused;
    if(bufsize > strbuf->unused) {
        while(freebytes < bufsize) freebytes += strbuf->chunksize;
        newsize = freebytes + strbuf->size;
        strbuf->buffer = (char *)realloc(strbuf->buffer, newsize+sizeof(char)); /* leave room for \0 */ 
        memset(&strbuf->buffer[strbuf->length], '\0', freebytes);
        strbuf->size = newsize;
        strbuf->unused = freebytes;
    }

    memcpy(&strbuf->buffer[strbuf->length], str, bufsize);
    strbuf->unused -= bufsize;
    strbuf->length += bufsize;

    return bufsize;
}

char* xcc_strbuf_tostring(XCC_STRBUF *strbuf) {
    if(!strbuf) return NULL;

    return (strbuf->buffer);
}

