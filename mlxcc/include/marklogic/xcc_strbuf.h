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

#ifndef MARKLOGIC_XCC_STRBUF_H
#define MARKLOGIC_XCC_STRBUF_H

#include <marklogic/globals.h>

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct st_xcc_strbuf {
    char *buffer;
    int size;
    int unused;
    int chunksize;
    int length;
} XCC_STRBUF;

int xcc_strbuf_append(XCC_STRBUF *strbuf, const char *str, int bufsize); 
XCC_STRBUF* xcc_strbuf_init(XCC_STRBUF *strbuf); 
char* xcc_strbuf_tostring(XCC_STRBUF *strbuf); 
void xcc_strbuf_free(XCC_STRBUF *strbuf); 

#ifdef  __cplusplus
}
#endif

#endif /* xcc_strbuf.h */
