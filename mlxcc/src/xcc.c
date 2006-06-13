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

#include <marklogic/xcc.h>
#include <curl/curl.h>

struct st_xcc_session {
    char *user, *host, *password, *dbname, *dbid;
    char *userpw;
    char *base_url;
    char *server_string;
    int port;
    char *last_error;
    XCC_ERROR_CODE last_errcode;
};

struct st_xcc_variable {
    char *ns;
    char *name;
    char *value;
    XCC_SCHEMA_TYPE type;
    struct st_xcc_variable *next;
};

struct st_xcc_request {
    CURLM *multi_handle;
    struct curl_slist *headers;
    CURL *curl;
    XCC_REQ_TYPE type;
    XCC_SESS *session;
    XCC_VAR *varlist_head;
    char *query_string;
    char *url;
    char *xquery;
    char *dbname;
    char *name;
    char *dbid;
    char *timestamp;
    long buffer_size;
    int timeout;
};

struct st_xcc_permission {
    char *role;
    XCC_PERMISSION capability;
    struct st_xcc_permission *next;
};

struct st_xcc_collection {
    char *name;
    struct st_xcc_collection *next;
};

struct st_xcc_content {
    struct curl_slist *headers;
    CURL *curl;
    XCC_SESS *session;
    char *url;
    char *uri;
    char *data;
    FILE *file;
    char *ns;
    XCC_STRBUF *error;
    XCC_PERM *perm_head;
    XCC_COL *col_head;
    int timeout;
    int quality;
    size_t offset;
    int read_started;
    int read_ended;
    xcc_off_t size;
    XCC_CONTENT_FORMAT format;
    XCC_CONTENT_ERROR_CORRECTION repair;
};

struct st_xcc_result {
    XCC_REQ *request;
    char *content_type;
    char *boundry;
    char *buffer;
    int buffer_len;
    int buffer_pos;
    int still_running;
    int clean_request;
    int has_next;
    long content_length;
    long http_res_code;
};

struct st_xcc_item {
    XCC_STRBUF *buffer;
    char *content_type;
    char *x_primitive;
    XCC_SCHEMA_TYPE type;
};

void xcc_set_error(XCC_SESS *session, const char *msg, XCC_ERROR_CODE errcode) {
    if(session->last_error) free(session->last_error);
    session->last_error = strdup(msg);
    session->last_errcode = errcode;

    return;
}

void xcc_free_var(XCC_VAR *var) {
    if(!var) return;
    if(var->ns) free(var->ns);
    if(var->name) free(var->name);
    if(var->value) free(var->value);
    free(var);
    return;
}

/* adopted from 'man snprintf' 
   XXX consider setting a larger value for size
*/
char* xcc_to_string(const char *fmt, ...) {
    int n, size = 1024;
    char *p;
    va_list ap;
    if ((p = malloc (size)) == NULL)
        return NULL;
    while (1) {
        va_start(ap, fmt);
        n = vsnprintf (p, size, fmt, ap);
        va_end(ap);
        if (n > -1 && n < size)
            return p;
        if (n > -1)    /* glibc 2.1 */
            size = n+1; /* precisely what is needed */
        else           /* glibc 2.0 */
            size *= 2;  /* twice the old size */
        if ((p = realloc (p, size)) == NULL)
            return NULL;
    }
}


/* adopted from curl examples: fopen.c */
size_t xcc_write_curldata(char *buffer, size_t size, size_t nmemb, XCC_RES *res) {
    char *newbuff;
    int rembuff;

    size *= nmemb;

    rembuff=res->buffer_len - res->buffer_pos;

    if(size > rembuff) {
        newbuff=realloc(res->buffer,res->buffer_len + (size - rembuff));
        if(newbuff==NULL) {
            if(!res->request) {
                xcc_set_error(res->request->session, "callback buffer grow failed", XCC_ERROR_GROW_BUFFER);
            }
            size=rembuff;
        } else {
            res->buffer_len+=size - rembuff;
            res->buffer=newbuff;
        }
    }

    memcpy(&res->buffer[res->buffer_pos], buffer, size);
    res->buffer_pos += size;

    return size;
}

/* adopted from curl examples: fopen.c */
static int xcc_fill_buffer(XCC_RES *res, int want, int waittime) {
    fd_set fdread;
    fd_set fdwrite;
    fd_set fdexcep;
    int maxfd;
    struct timeval timeout;
    int rc;

    if((!res->still_running) || (res->buffer_pos > want))
        return 0;

    do {
        FD_ZERO(&fdread);
        FD_ZERO(&fdwrite);
        FD_ZERO(&fdexcep);

        timeout.tv_sec = res->request->timeout;
        timeout.tv_usec = 0;

        curl_multi_fdset(res->request->multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);

        rc = select(maxfd+1, &fdread, &fdwrite, &fdexcep, &timeout);

        switch(rc) {
        case -1:
            /* select error */
            break;

        case 0:
            break;

        default:
            while(curl_multi_perform(res->request->multi_handle, &res->still_running) ==
                  CURLM_CALL_MULTI_PERFORM);
            break;
        }
    } while(res->still_running && (res->buffer_pos < want));
    return 1;
}

/* adopted from curl examples: fopen.c */
static int xcc_use_buffer(XCC_RES *res, int want) {
    if((res->buffer_pos - want) <=0) {
        if(res->buffer) free(res->buffer);

        res->buffer=NULL;
        res->buffer_pos=0;
        res->buffer_len=0;
    } else {
        memmove(res->buffer, &res->buffer[want], (res->buffer_pos - want));

        res->buffer_pos -= want;
    }
    return 0;
}

/* adopted from curl examples: fopen.c */
static char* xcc_result_read(XCC_RES *res, char *ptr, int size) {
    int want = size - 1;

    xcc_fill_buffer(res, want, 1);

    if(!res->buffer_pos) return NULL;

    if(res->buffer_pos < want) want = res->buffer_pos;

    memcpy(ptr, res->buffer, want);
    ptr[want]=0;

    xcc_use_buffer(res, want);

    return ptr;
}

/* adopted from curl examples: fopen.c */
static int xcc_result_getline(XCC_RES *res, char *ptr, int size) {
    int want = size - 1;
    int loop;

    xcc_fill_buffer(res, want, 1);

    if(!res->buffer_pos) return 0;

    if(res->buffer_pos < want) want = res->buffer_pos;

    for(loop=0;loop < want;loop++) {
        if(res->buffer[loop] == '\n') {
            want=loop+1; /* add newline */
            break;
        }
    }

    memcpy(ptr, res->buffer, want);
    ptr[want]=0;

    xcc_use_buffer(res, want);

    return want;
}

static int xcc_result_eof(XCC_RES *res) {
    int ret = FALSE;
    if((res->buffer_pos == 0) && (!res->still_running)) {
        ret = TRUE;
    }

    return ret;
}

size_t xcc_read_headerdata(char *buf, size_t size, size_t nmemb, XCC_RES *res) {
    int bytes;
    char *size_str;

    bytes = (size * nmemb);
    if(!strncmp("Content-Type:", buf, 13)) {
        res->content_type = (char *)malloc(bytes);
        memset(res->content_type, '\0', bytes);
        memcpy(res->content_type, &buf[14], bytes - (16 * size));

        /* grab the boundry string for later use */
        if(!strncmp("multipart/mixed; boundary=", res->content_type, 25)) {
            res->boundry = (char *)malloc(bytes);
            memset(res->boundry, '\0', bytes);
            memcpy(res->boundry, &res->content_type[26], bytes - (28 * size));
        }
    } else if(!strncmp("Content-Length:", buf, 15)) {
        size_str = (char *)malloc(bytes);
        memset(size_str, '\0', bytes);
        memcpy(size_str, &buf[16], bytes - (17 * size));
        res->content_length = atol(size_str);
        free(size_str);
    }
    
    return bytes;
}

XCC_ERROR_CODE xcc_request_addvar(XCC_REQ *request, XCC_VAR *var) {
    XCC_VAR *cur;
    XCC_VAR *prev;
    XCC_VAR *end;

    if(!request || !var) return XCC_ERROR_INVALID_ARG;

    if(request->varlist_head == NULL) {
        request->varlist_head = var;
        return XCC_OK;
    }

    prev = NULL;
    end = NULL;
    cur = request->varlist_head;
    while(1) {
        if(strcmp(var->name, cur->name) == 0) {
            break;
        }
        if(cur->next == NULL) {
            end = cur;
            break;
        }
        prev = cur;
        cur = cur->next;
    }

    if(end != NULL) {
        end->next = var;
    } else {
        if(prev == NULL) {
            request->varlist_head = var;
            if(cur->next) var->next = cur->next;
            xcc_free_var(cur);
        } else {
            prev->next = var;
            var->next = cur->next;
            xcc_free_var(cur);
        }
    }

    return XCC_OK;
}

char* xcc_schema_type_as_string(XCC_SCHEMA_TYPE schema_type) {
    char *type;
    int len;

    /* some reasonable size */
    len = sizeof(char)*256;
    type = (char *)malloc(len);
    memset(type, '\0', len);

    /** @todo flesh out the rest of the schema types */
    switch(schema_type) {
    case XCC_TYPE_ANYURI:
        strcpy(type, "xs:anyURI");
        break;
    case XCC_TYPE_BASE64BINARY:
        strcpy(type, "xs:base64Binary");
        break;
    case XCC_TYPE_BINARY:
        strcpy(type, "binary()");
        break;
    case XCC_TYPE_BOOLEAN:
        strcpy(type, "xs:boolean");
        break;
    case XCC_TYPE_DATE:
        strcpy(type, "xs:date");
        break;
    case XCC_TYPE_DATETIME:
        strcpy(type, "xs:dateTime");
        break;
    case XCC_TYPE_DAYTIMEDURATION:
        strcpy(type, "xdt:dayTimeDuration");
        break;
    case XCC_TYPE_DECIMAL:
        strcpy(type, "xs:decimal");
        break;
    case XCC_TYPE_DOUBLE:
        strcpy(type, "xs:double");
        break;
    case XCC_TYPE_DURATION:
        strcpy(type, "xs:duration");
        break;
    case XCC_TYPE_FLOAT:
        strcpy(type, "xs:float");
        break;
    case XCC_TYPE_GDAY:
        strcpy(type, "xs:gDay");
        break;
    case XCC_TYPE_GMONTH:
        strcpy(type, "xs:gMonth");
        break;
    case XCC_TYPE_GMONTHDAY:
        strcpy(type, "xs:gMonthDay");
        break;
    case XCC_TYPE_GYEAR:
        strcpy(type, "xs:gYear");
        break;
    case XCC_TYPE_GYEARMONTH:
        strcpy(type, "xs:gYearMonth");
        break;
    case XCC_TYPE_HEXBINARY:
        strcpy(type, "xs:hexBinary");
        break;
    case XCC_TYPE_INTEGER:
        strcpy(type, "xs:integer");
        break;
    case XCC_TYPE_NODE:
        strcpy(type, "node()");
        break;
    case XCC_TYPE_QNAME:
        strcpy(type, "xs:QName");
        break;
    case XCC_TYPE_STRING:
        strcpy(type, "xs:string");
        break;
    case XCC_TYPE_TEXT:
        strcpy(type, "text()");
        break;
    case XCC_TYPE_TIME:
        strcpy(type, "xs:time");
        break;
    case XCC_TYPE_YEARMONTHDURATION:
        strcpy(type, "xdt:yearMonthDuration");
        break;
    default:
        free(type);
        type = NULL;
        break;
    }

    return type;
}

XCC_ERROR_CODE xcc_append_request_param(XCC_STRBUF *query, const char *name, const char *value) {
    char *escaped;
    char *param;

    if(!(escaped = curl_escape(value , 0))) {
        return XCC_ERROR_CURL;
    }

    param = xcc_to_string("&%s=%s", name, escaped);
    xcc_strbuf_append(query, param, strlen(param)*sizeof(char));
    curl_free(escaped);
    free(param);

    return XCC_OK;
}


/* XXX this function needs to be refactored  */
XCC_ERROR_CODE xcc_request_build_query(XCC_REQ *request) {
    const char *base_query;
    XCC_STRBUF *query;

    /* used for building external vars */
    XCC_VAR *cur;
    int count = 0;
    char *params;
    char *ns;
    char *name;
    char *type;
    char *etype;
    char *val;

    params = ns = name = type = etype = val = NULL;

    if(!request) return XCC_ERROR_INVALID_ARG;

    query = (XCC_STRBUF *)malloc(sizeof(XCC_STRBUF));
    if(!query) return XCC_ERROR_OUT_OF_MEM;

    memset(query, 0, sizeof(XCC_STRBUF));
    query = xcc_strbuf_init(query);

    base_query = "locale=en_US&tzoffset=-25200";
    xcc_strbuf_append(query, base_query, strlen(base_query)*sizeof(char));

    /* add xquery or module */
    if(request->type == XCC_REQ_TYPE_INVOKE || request->type == XCC_REQ_TYPE_SPAWN) {
        if(XCC_OK != xcc_append_request_param(query, "module", request->xquery)) {
            return XCC_ERROR_CURL;
        }
    } else {
        if(XCC_OK != xcc_append_request_param(query, "xquery", request->xquery)) {
            return XCC_ERROR_CURL;
        }
    }

    /* add request options if any */
    if(request->dbname && strlen(request->dbname) > 0) {
        if(XCC_OK != xcc_append_request_param(query, "dbname", request->dbname)) {
            return XCC_ERROR_CURL;
        }
    } else if(request->dbid && strlen(request->dbid) > 0) {
        if(XCC_OK != xcc_append_request_param(query, "dbid", request->dbid)) {
            return XCC_ERROR_CURL;
        }
    } else if(request->session->dbname && strlen(request->session->dbname) > 0) {
        if(XCC_OK != xcc_append_request_param(query, "dbname", request->session->dbname)) {
            return XCC_ERROR_CURL;
        }
    } else if(request->session->dbid && strlen(request->session->dbid) > 0) {
        if(XCC_OK != xcc_append_request_param(query, "dbid", request->session->dbid)) {
            return XCC_ERROR_CURL;
        }
    }

    if(request->name && strlen(request->name) > 0) {
        if(XCC_OK != xcc_append_request_param(query, "requestname", request->name)) {
            return XCC_ERROR_CURL;
        }
    }
    if(request->timestamp && strlen(request->timestamp) > 0) {
        if(XCC_OK != xcc_append_request_param(query, "timestamp", request->timestamp)) {
            return XCC_ERROR_CURL;
        }
    }

    /* add external variables if any */
    cur = request->varlist_head;
    while(cur != NULL) {
        type = xcc_schema_type_as_string(cur->type);
        if(!type) {
            return XCC_ERROR_UNKNOWN_TYPE;
        }
        if(!(etype = curl_escape(type , 0))) {
            return XCC_ERROR_CURL;
        }
        if(!(ns = curl_escape(cur->ns , 0))) {
            return XCC_ERROR_CURL;
        }
        if(!(name = curl_escape(cur->name , 0))) {
            return XCC_ERROR_CURL;
        }
        if(!(val = curl_escape(cur->value , 0))) {
            return XCC_ERROR_CURL;
        }

        params = xcc_to_string("&evn%d=%s&evl%d=%s&evt%d=%s&evv%d=%s",
                                count,
                                ns,
                                count,
                                name,
                                count,
                                etype,
                                count,
                                val);
        if(!params) {
            return XCC_ERROR_STMT_URL;
        }

        xcc_strbuf_append(query, params, strlen(params)*sizeof(char));

        curl_free(ns);
        curl_free(name);
        curl_free(val);
        curl_free(etype);
        free(type);
        free(params);
        params = ns = name = type = etype = val = NULL;

        count++;
        cur = cur->next;
    }

    if(request->query_string) free(request->query_string);
    request->query_string = strdup(xcc_strbuf_tostring(query));

    xcc_strbuf_free(query);

    return XCC_OK;
}

XCC_SCHEMA_TYPE xcc_get_schema_type(const char *typestr) {
    XCC_SCHEMA_TYPE type;

    /** @todo flesh the rest of the schema types out */
    if(!strcmp("boolean", typestr)) {
        type = XCC_TYPE_BOOLEAN;
    } else if(!strcmp("decimal", typestr)) {
        type = XCC_TYPE_DECIMAL;
    } else if(!strcmp("double", typestr)) {
        type = XCC_TYPE_DOUBLE;
    } else if(!strcmp("float", typestr)) {
        type = XCC_TYPE_FLOAT;
    } else if(!strcmp("integer", typestr)) {
        type = XCC_TYPE_INTEGER;
    } else if(!strcmp("node()", typestr)) {
        type = XCC_TYPE_NODE;
    } else if(!strcmp("string", typestr)) {
        type = XCC_TYPE_STRING;
    } else if(!strcmp("text()", typestr)) {
        type = XCC_TYPE_TEXT;
    } else if(!strcmp("binary()", typestr)) {
        type = XCC_TYPE_BINARY;
    } else {
        type = XCC_TYPE_UNKNOWN;
    }

    return type;
}

static int xcc_match_boundry(XCC_RES *res, const char *buffer) {
    int ret = 0;
    if(!res || !buffer) {
        res->has_next = 0;
        return ret;
    }
    if(!res->boundry)  {
        res->has_next = 0;
        return ret;
    }

    if(!strncmp(res->boundry, buffer+2, strlen(res->boundry))) {
        if(buffer[strlen(buffer)-2] != '-') {
            ret = 1;
            res->has_next = 1;
        } else {
            res->has_next = 0;
            ret = 1;
        }
    }

    return ret;
}

void xcc_free_perm(XCC_PERM *perm) {
    if(!perm) return;
    if(perm->role) free(perm->role);
    free(perm);
}

void xcc_free_col(XCC_COL *col) {
    if(!col) return;
    if(col->name) free(col->name);
    free(col);
}

XCC_ERROR_CODE xcc_content_build_url(XCC_CONTENT *content) {
    char *base_url;
    char *temp_escaped;
    char *temp;
    XCC_STRBUF *url;
    XCC_COL *cur_col;
    XCC_PERM *cur_perm;

    if(!content) return XCC_ERROR_INVALID_ARG;
    if(!content->uri) return XCC_ERROR_INVALID_ARG;
    if(!(strlen(content->uri) > 0)) return XCC_ERROR_INVALID_ARG;

    url = (XCC_STRBUF *)malloc(sizeof(XCC_STRBUF));
    if(!url) return XCC_ERROR_OUT_OF_MEM;

    memset(url, 0, sizeof(XCC_STRBUF));
    url = xcc_strbuf_init(url);

    /* not sure what the resolvesiz means. It seems to be always hardcoded to 1048576? 
     * @todo consider adding in support for other locales. 
     */
    base_url = xcc_to_string("%s%s?locale=en_US", 
                              content->session->base_url, 
                              XCC_UPLOAD_PATH);
    if(!base_url) {
        xcc_strbuf_free(url);
        return XCC_ERROR_OUT_OF_MEM;
    }

    xcc_strbuf_append(url, base_url, strlen(base_url)*sizeof(char));
    free(base_url);

    if(XCC_OK != xcc_append_request_param(url, "uri", content->uri)) {
        xcc_strbuf_free(url);
        return XCC_ERROR_CURL;
    }

    if(content->quality) {
        temp = xcc_to_string("&quality=%d", content->quality);
        xcc_strbuf_append(url, temp, strlen(temp)*sizeof(char));
        free(temp);
        temp = NULL;
    }

    if(content->ns && strlen(content->ns) > 0) {
        if(XCC_OK != xcc_append_request_param(url, "defaultns", content->ns)) {
            return XCC_ERROR_CURL;
        }
    }

    if(content->format != XCC_CONTENT_FORMAT_NONE) {
        switch(content->format) {
        case XCC_CONTENT_FORMAT_XML:
            temp = xcc_to_string("%s", "&format=xml");
            break;
        case XCC_CONTENT_FORMAT_BINARY:
            temp = xcc_to_string("%s", "&format=bin");
            break;
        case XCC_CONTENT_FORMAT_TEXT:
            temp = xcc_to_string("%s", "&format=text");
            break;
        default:
            temp = NULL;
            break;
        }
        if(!temp) return XCC_ERROR_UNKNOWN_OPTION;

        xcc_strbuf_append(url, temp, strlen(temp)*sizeof(char));
        free(temp);
        temp = NULL;
    }
    if(content->repair == XCC_CONTENT_ERROR_CORRECTION_NONE) {
        if(XCC_OK != xcc_append_request_param(url, "repair", "none")) {
            return XCC_ERROR_CURL;
        }
    }

    cur_col = content->col_head;
    while(cur_col != NULL) {
        if(XCC_OK != xcc_append_request_param(url, "coll", cur_col->name)) {
            return XCC_ERROR_CURL;
        }
        cur_col = cur_col->next;
    }

    cur_perm = content->perm_head;
    while(cur_perm != NULL) {
        if(!(temp_escaped = curl_escape(cur_perm->role , 0))) {
            return XCC_ERROR_CURL;
        }

        switch(cur_perm->capability) {
        case XCC_PERMISSION_INSERT:
            temp = xcc_to_string("&perm=I%s", temp_escaped);
            break;
        case XCC_PERMISSION_READ:
            temp = xcc_to_string("&perm=R%s", temp_escaped);
            break;
        case XCC_PERMISSION_UPDATE:
            temp = xcc_to_string("&perm=U%s", temp_escaped);
            break;
        case XCC_PERMISSION_EXECUTE:
            temp = xcc_to_string("&perm=E%s", temp_escaped);
            break;
        default:
            temp = NULL;
            break;
        }
        if(!temp) return XCC_ERROR_UNKNOWN_OPTION;

        xcc_strbuf_append(url, temp, strlen(temp)*sizeof(char));
        free(temp);
        curl_free(temp_escaped);
        temp = NULL;
        temp_escaped = NULL;
        cur_perm = cur_perm->next;
    }

    if(content->session->dbname && strlen(content->session->dbname) > 0) {
        if(XCC_OK != xcc_append_request_param(url, "dbname", content->session->dbname)) {
            return XCC_ERROR_CURL;
        }
    }

    if(content->url) free(content->url);
    content->url = strdup(xcc_strbuf_tostring(url));
    xcc_strbuf_free(url);

    return XCC_OK;
}

size_t xcc_write_content_curldata(char *buffer, size_t size, size_t nmemb, XCC_CONTENT *content) {
    return xcc_strbuf_append(content->error, buffer, size*nmemb);
}

size_t xcc_read_content_data(char *ptr, size_t size, size_t nmemb, XCC_CONTENT *content) {
    /* XXX These types are all messed up. Trying to figure out how to support
           large files. For now typedef xcc_off_t to long long instead of using
           off_t directly because it's easier to bind into other languages knowing 
           that xcc_off_t will always be a long long. The thing that i'm not sure about
           is the offsets are type size_t so it seems like things could get weird. Also,
           this function is used for both loading content from a file and from a char buffer.
           Consider breaking out into 2 separate functions. Tests are passing but 
           there is sure to be some problems here. Need to fix soon */
    size_t bytes, remaining;
    char *size_str;
    char *tmp;
    const char *end_str = "10\n";
    size_t size_str_bytes;

    /* If this is our first read then we have to send the size of the data were about
     * to load. It looks like Mark Logic requires the number of bytes being uploaded
     * to be the first data that gets sent followed by a newline. It also seems like
     * the number 1 is always prepended to the number of bytes? Not sure if this is 
     * correct. It seems off but it works. Not sure why they don't just use the 
     * Content-Length header...
     */
    if(!content->read_started) {
        tmp = xcc_to_string(XCC_FORMAT_OFF_T, content->size);
        size_str = xcc_to_string("%s%s%s", "0", tmp, "\n");
        size_str_bytes = sizeof(char)*strlen(size_str);
        memcpy(ptr, &size_str[0], size_str_bytes);
        free(size_str);
        free(tmp);
        content->read_started = TRUE;
        return size_str_bytes;
    }

    /* if file then read from that */
    if(content->file) {
        bytes = fread(ptr, size, nmemb, content->file);
        if(bytes) return bytes;
        if(content->read_ended) return 0;

        size_str_bytes = sizeof(char)*strlen(end_str);
        memcpy(ptr, &end_str[0], size_str_bytes);
        content->read_ended = TRUE;
        return size_str_bytes;
    }

    /* else read from data buffer */

    if(content->read_ended) return 0;

    /* test for end of data */
    if(content->offset >= content->size) {
        size_str_bytes = sizeof(char)*strlen(end_str);
        memcpy(ptr, &end_str[0], size_str_bytes);
        content->read_ended = TRUE;
        return size_str_bytes;
    }

    bytes = size * nmemb;
    remaining = content->size - content->offset;

    if(remaining >= bytes) {
        memcpy(ptr, &content->data[content->offset], bytes);
        content->offset += bytes;
        return bytes;
    }

    memcpy(ptr, &content->data[content->offset], remaining);
    content->offset = content->size;
    return remaining;
}

/*------- PUBLIC API -------------------*/

XCC_SESS* xcc_new_session(const char *user, 
                            const char *password, 
                            const char *host, 
                            const char *db, 
                            int port) {
    XCC_SESS *session;

    session = (XCC_SESS *)malloc(sizeof(XCC_SESS));
    if(!session) {
        return NULL;
    }

    /* Init */
    memset(session, 0, sizeof(session));
    session->user = NULL;
    session->host = NULL;
    session->password = NULL;
    session->dbname = NULL;
    session->dbid = NULL;
    session->userpw = NULL;
    session->base_url = NULL;
    session->server_string = NULL;
    session->port = XCC_DEFAULT_PORT;
    session->last_error = NULL;
    session->last_errcode = XCC_OK;
    
    if(!user) user = "";
    session->user = strdup(user);

    if(!host) host = "localhost";
    session->host = strdup(host);

    if(!db) db = "";
    session->dbname = strdup(db);

    if(!password) password = "";
    session->password = strdup(password);

    session->port = port;
    if(session->port <= 0) session->port = XCC_DEFAULT_PORT;

    session->base_url = xcc_to_string("http://%s:%d", host, port);
    if(!session->base_url) {
        xcc_free_session(session);
        return NULL;
    }

    session->userpw = xcc_to_string("%s:%s", user, password);
    if(!session->userpw) {
        xcc_free_session(session);
        return NULL;
    }

    return session;
}

void xcc_free_session(XCC_SESS *session) {
    if(!session) return;
    if(session->user) free(session->user);
    if(session->host) free(session->host);
    if(session->password) free(session->password);
    if(session->dbname) free(session->dbname);
    if(session->dbid) free(session->dbid);
    if(session->userpw) free(session->userpw);
    if(session->base_url) free(session->base_url);
    if(session->server_string) free(session->server_string);
    if(session->last_error) free(session->last_error);
    free(session);

    return;
}

const char* xcc_fatal_error() {
    const char *fatal_err = "FATAL ERROR: no way to get error messages";
    return fatal_err;
}

const char* xcc_error(XCC_SESS *session) {
    if(!session) return xcc_fatal_error();
    if(session->last_error) return session->last_error;
    return xcc_fatal_error();
}

const char* xcc_req_error(XCC_REQ *request) {
    if(!request) return xcc_fatal_error();
    return xcc_error(request->session);
}

const char* xcc_content_error(XCC_CONTENT *content) {
    if(!content) return xcc_fatal_error();
    return xcc_error(content->session);
}

XCC_ERROR_CODE xcc_errcode(XCC_SESS *session) {
    if(!session) return XCC_ERROR_FATAL;
    return session->last_errcode;
}

XCC_ERROR_CODE xcc_req_errcode(XCC_REQ *request) {
    if(!request) return XCC_ERROR_FATAL;
    return xcc_errcode(request->session);
}

XCC_ERROR_CODE xcc_content_errcode(XCC_CONTENT *content) {
    if(!content) return XCC_ERROR_FATAL;
    return xcc_errcode(content->session);
}

#define XCC_REQ_FREE() \
    xcc_set_error(session, curl_easy_strerror(code), XCC_ERROR_CURL); \
    xcc_free_request(request);

XCC_REQ* xcc_new_request(XCC_SESS *session, const char *xquery, XCC_REQ_TYPE request_type) {
    CURLcode code;
    XCC_REQ *request;

    request = (XCC_REQ *)malloc(sizeof(XCC_REQ));
    if(!request) {
        xcc_set_error(session, "failed to allocate memory for request", XCC_ERROR_OUT_OF_MEM);
        return NULL;
    }
    memset(request, 0, sizeof(request));

    request->session = session;
    request->multi_handle = curl_multi_init();
    request->buffer_size = XCC_DEFAULT_BUFSIZE;
    request->timeout = XCC_DEFAULT_QUERY_TIMEOUT;
    request->varlist_head = NULL;
    request->query_string = NULL;
    request->name = NULL;
    request->dbname = NULL;
    request->dbid = NULL;
    request->timestamp = NULL;
    if(!xquery) xquery = "";
    request->xquery = strdup(xquery);

    switch(request_type) {
    case XCC_REQ_TYPE_INVOKE:
        request->url = xcc_to_string("%s%s", request->session->base_url, XCC_INVOKE_PATH);
        request->type = XCC_REQ_TYPE_INVOKE;
        break;
    case XCC_REQ_TYPE_SPAWN:
        request->url = xcc_to_string("%s%s", request->session->base_url, XCC_SPAWN_PATH);
        request->type = XCC_REQ_TYPE_SPAWN;
        break;
    default:
        request->url = xcc_to_string("%s%s", request->session->base_url, XCC_EVAL_PATH);
        request->type = XCC_REQ_TYPE_QUERY;
        break;
    }

    request->curl = curl_easy_init();
    if(!request->curl) {
        xcc_set_error(session, "Failed to initialize curl connection", XCC_ERROR_CURL);
        xcc_free_request(request);
        return NULL;
    }

#ifdef DEBUG
    code = curl_easy_setopt(request->curl, CURLOPT_VERBOSE, TRUE);
    if(code != CURLE_OK) {
        XCC_REQ_FREE();
        return NULL;
    }
#endif

    code = curl_easy_setopt(request->curl, CURLOPT_URL, request->url);
    if(code != CURLE_OK) {
        XCC_REQ_FREE();
        return NULL;
    }

    /* Set custom HTTP Headers */
    request->headers = NULL;
    request->headers = curl_slist_append(request->headers, XCC_USER_AGENT);

    /* Disable the Expect: 100-contiue header */
    request->headers = curl_slist_append(request->headers, "Expect:");
    request->headers = curl_slist_append(request->headers, "Transfer-Encoding:");
    request->headers = curl_slist_append(request->headers, "Pragma:");

    code = curl_easy_setopt(request->curl, CURLOPT_HTTPHEADER, request->headers);
    if(code != CURLE_OK) {
        XCC_REQ_FREE();
        return NULL;
    }

    code = curl_easy_setopt(request->curl, CURLOPT_WRITEFUNCTION, xcc_write_curldata);
    if(code != CURLE_OK) {
        XCC_REQ_FREE();
        return NULL;
    }

    code = curl_easy_setopt(request->curl, CURLOPT_HEADERFUNCTION, xcc_read_headerdata);
    if(code != CURLE_OK) {
        XCC_REQ_FREE();
        return NULL;
    }

    code = curl_easy_setopt(request->curl, CURLOPT_USERPWD, request->session->userpw);
    if(code != CURLE_OK) {
        XCC_REQ_FREE();
        return NULL;
    }

    code = curl_easy_setopt(request->curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
    if(code != CURLE_OK) {
        XCC_REQ_FREE();
        return NULL;
    }

    return request;
}

XCC_REQ* xcc_new_adhoc_query(XCC_SESS *session, const char *xquery) {
    return xcc_new_request(session, xquery, XCC_REQ_TYPE_QUERY);
}


XCC_REQ* xcc_new_module_invoke(XCC_SESS *session, const char *module) {
    return xcc_new_request(session, module, XCC_REQ_TYPE_INVOKE);
}

XCC_REQ* xcc_new_module_spawn(XCC_SESS *session, const char *module) {
    return xcc_new_request(session, module, XCC_REQ_TYPE_SPAWN);
}

void xcc_free_request(XCC_REQ *request) {
    if(!request) return;
    if(request->headers) curl_slist_free_all(request->headers);
    if(request->curl && request->multi_handle) { 
        curl_multi_remove_handle(request->multi_handle, request->curl);
    }
    if(request->multi_handle) curl_multi_cleanup(request->multi_handle);
    if(request->curl) curl_easy_cleanup(request->curl);
    if(request->query_string) free(request->query_string);
    if(request->xquery) free(request->xquery);
    if(request->dbname) free(request->dbname);
    if(request->dbid) free(request->dbid);
    if(request->name) free(request->name);
    if(request->timestamp) free(request->timestamp);
    if(request->url) free(request->url);
    xcc_clear_varlist(request);
    free(request);

    return;
}

void xcc_clear_varlist(XCC_REQ *request) {
    XCC_VAR *cur;
    XCC_VAR *next;

    if(!request) return;
    if(request->varlist_head) {
        next = cur = request->varlist_head->next;
        xcc_free_var(request->varlist_head);
        while(cur != NULL) {
            next = cur->next;
            xcc_free_var(cur);
            cur = next;
        }

        request->varlist_head = NULL;
    }

    return;
}

XCC_ERROR_CODE xcc_request_setop(XCC_REQ *request, XCC_REQ_OPTION opt, ...) {
    va_list arg;
    int ret = XCC_OK;

    if(!request) return XCC_ERROR_INVALID_ARG;

    va_start(arg, opt);

    /** @todo need to add more options */
    switch(opt) {
    case XCC_REQ_BUFSIZE:
        request->buffer_size = va_arg(arg, long);
        break;
    case XCC_REQ_TIMEOUT:
        request->timeout = va_arg(arg, int);
        break;
    case XCC_REQ_DBNAME:
        request->dbname = xcc_to_string("%s", va_arg(arg, char *));;
        break;
    case XCC_REQ_DBID:
        request->dbid = xcc_to_string("%s", va_arg(arg, char *));;
        break;
    case XCC_REQ_NAME:
        request->name = xcc_to_string("%s", va_arg(arg, char *));;
        break;
    case XCC_REQ_TIMESTAMP:
        request->timestamp = xcc_to_string("%s", va_arg(arg, char *));
        break;
    default:
        ret = XCC_ERROR_UNKNOWN_OPTION;
        break;
    }

    va_end(arg);
    return ret;
}

XCC_ERROR_CODE xcc_setvar(XCC_REQ *request, 
                            XCC_SCHEMA_TYPE type, 
                            const char *ns, 
                            const char *name, ...) {
    va_list arg;
    XCC_VAR *var;
    char *val;
    int ret = XCC_OK;

    if(!request || !name) return XCC_ERROR_INVALID_ARG;

    va_start(arg, name);

    var = (XCC_VAR *)malloc(sizeof(XCC_VAR));
    memset(var, 0, sizeof(var));
    if(!name) name = "";
    var->name = strdup(name);
    var->type = type;
    var->next = NULL;
    var->value = var->ns = NULL;
    if(!ns) ns = "";
    var->ns = strdup(ns);

    /** @todo need to flesh out the rest of the schema types */
    switch(type) {
    case XCC_TYPE_STRING:
        val = va_arg(arg, char *);
        if(val) {
            var->value = strdup(val);
        } else {
            ret = XCC_ERROR_INVALID_ARG;
        }
        break;
    case XCC_TYPE_INTEGER:
        var->value = xcc_to_string("%d", va_arg(arg, int));
        break;
    case XCC_TYPE_DOUBLE:
        var->value = xcc_to_string("%f", va_arg(arg, double));
        break;
    default:
        ret = XCC_ERROR_UNKNOWN_TYPE;
        break;
    }

    va_end(arg);

    if(ret != XCC_OK) {
        xcc_free_var(var);
        return ret;
    }

    return xcc_request_addvar(request, var);
}

XCC_ERROR_CODE xcc_setvar_generic(XCC_REQ *request, 
                                    XCC_SCHEMA_TYPE type, 
                                    const char *ns, 
                                    const char *name, 
                                    const char *val) {
    XCC_VAR *var;

    if(!request || !name) return XCC_ERROR_INVALID_ARG;

    var = (XCC_VAR *)malloc(sizeof(XCC_VAR));
    memset(var, 0, sizeof(var));
    if(!name) name = "";
    var->name = strdup(name);
    var->next = NULL;
    var->value = var->ns = NULL;
    if(!ns) ns = "";
    var->ns = strdup(ns);
    if(!val) val = "";
    var->value = strdup(val);
    var->type = type;

    return xcc_request_addvar(request, var);
}

#define XCC_XQUERY_FREE() \
    xcc_set_error(request->session, curl_easy_strerror(code), XCC_ERROR_CURL); \
    xcc_free_result(res);

XCC_RES* xcc_submit_request(XCC_REQ *request) {
    CURLcode code;
    XCC_ERROR_CODE errcode;
    XCC_RES *res;
    CURLMsg *curl_msg;
    int msg_in_queue;
    char error_buffer[XCC_ERROR_SIZE];

    if(!request) { 
        return NULL;
    }
    if(!request->session) { 
        return NULL;
    }
    if(!request->curl) { 
        xcc_set_error(request->session, "request curl handle was not intialized", XCC_ERROR_INVALID_ARG);
        return NULL;
    }
    if((errcode = xcc_request_build_query(request)) != XCC_OK) {
        xcc_set_error(request->session, "failed to build request query", errcode);
        return NULL;
    }

    res = (XCC_RES *)malloc(sizeof(XCC_RES));
    if(!res) {
        xcc_set_error(request->session, "failed to allocate memory for xcc result", XCC_ERROR_OUT_OF_MEM);
        return NULL;
    }
    memset(res, 0, sizeof(XCC_RES));
    
    res->request = request;
    res->clean_request = 0;
    res->boundry = res->content_type = res->buffer = NULL;

    code = curl_easy_setopt(res->request->curl, CURLOPT_WRITEDATA, res);
    if(code != CURLE_OK) {
        XCC_XQUERY_FREE();
        return NULL;
    }

    code = curl_easy_setopt(res->request->curl, CURLOPT_WRITEHEADER, res);
    if(code != CURLE_OK) {
        XCC_XQUERY_FREE();
        return NULL;
    }

    code = curl_easy_setopt(res->request->curl, CURLOPT_POSTFIELDS, res->request->query_string);
    if(code != CURLE_OK) {
        XCC_XQUERY_FREE();
        return NULL;
    }
    
    if(!res->request->multi_handle) res->request->multi_handle = curl_multi_init();

    curl_multi_add_handle(res->request->multi_handle, res->request->curl);

    do {
        while(curl_multi_perform(res->request->multi_handle, &res->still_running) == CURLM_CALL_MULTI_PERFORM);
    } while(res->still_running);

    if((res->buffer_pos == 0) && (!res->still_running)) {
        /* Already done so check quick for errors */
        curl_msg = curl_multi_info_read(res->request->multi_handle, &msg_in_queue);
        if(curl_msg) {
            if(curl_msg->data.result) {
                xcc_set_error(request->session, curl_easy_strerror(curl_msg->data.result), XCC_ERROR_CURL);
                curl_multi_remove_handle(res->request->multi_handle, res->request->curl);
                xcc_free_result(res);

                return NULL;
            }
        }
    }

    /* Kick start the HTTP request so we can get the headers */
    xcc_fill_buffer(res, 1, 1);

    code = curl_easy_getinfo(res->request->curl, CURLINFO_RESPONSE_CODE, &res->http_res_code);
    if(code != CURLE_OK) {
        XCC_XQUERY_FREE();
        return NULL;
    }

    if(res->http_res_code == XCC_HTTP_RES_ERROR) {
        xcc_result_read(res, error_buffer, sizeof(error_buffer));
        xcc_set_error(res->request->session, error_buffer, XCC_ERROR_XQUERY);
        curl_multi_remove_handle(res->request->multi_handle, res->request->curl);
        xcc_free_result(res);
        return NULL;
    } else if(res->http_res_code == XCC_HTTP_AUTH_FAILURE) {
        xcc_set_error(res->request->session, "Authentication failure. Please check username/password.", XCC_ERROR_AUTH_FAILURE);
        curl_multi_remove_handle(res->request->multi_handle, res->request->curl);
        xcc_free_result(res);
        return NULL;
    } else if(res->http_res_code == XCC_HTTP_NOT_FOUND) {
        xcc_set_error(res->request->session, "Your version of Mark Logic does not seem to support this feature.", XCC_ERROR_AUTH_FAILURE);
        curl_multi_remove_handle(res->request->multi_handle, res->request->curl);
        xcc_free_result(res);
        return NULL;
    } else if(res->http_res_code != XCC_HTTP_RES_OK) {
        curl_msg = curl_multi_info_read(res->request->multi_handle, &msg_in_queue);
        if(curl_msg) {
            if(curl_msg->data.result) {
                xcc_set_error(request->session, curl_easy_strerror(curl_msg->data.result), XCC_ERROR_CURL);
            } else {
                xcc_set_error(res->request->session, "Unsupported HTTP Response code returned", XCC_ERROR_HTTP_CODE);
            }
        } else {
            xcc_set_error(res->request->session, "Unsupported HTTP Response code returned", XCC_ERROR_HTTP_CODE);
        }
        curl_multi_remove_handle(res->request->multi_handle, res->request->curl);
        xcc_free_result(res);
        return NULL;
    }

    res->has_next = -1;

    return res;
}

XCC_RES* xcc_execute(XCC_REQ *request) {
    XCC_RES *result;

    result = xcc_submit_request(request);
    if(!result) { 
        xcc_free_request(request);
        return NULL;
    }

    result->clean_request = 1;

    return result;
}

XCC_RES* xcc_query(XCC_SESS *session, const char *query) {
    XCC_REQ *request;

    request = xcc_new_adhoc_query(session, query);
    if(!request) return NULL;
    return xcc_execute(request);
}

XCC_RES* xcc_invoke(XCC_SESS *session, const char *module) {
    XCC_REQ *request;

    request = xcc_new_module_invoke(session, module);
    if(!request) return NULL;
    return xcc_execute(request);
}

XCC_RES* xcc_spawn(XCC_SESS *session, const char *module) {
    XCC_REQ *request;

    request = xcc_new_module_spawn(session, module);
    if(!request) return NULL;
    return xcc_execute(request);
}

void xcc_free_result(XCC_RES *res) {
    if(!res) return;
    if(res->content_type) free(res->content_type);
    if(res->boundry) free(res->boundry);
    if(res->buffer) free(res->buffer);
    if(res->clean_request) {
        xcc_free_request(res->request);
    }
    free(res);
}

XCC_ITEM* xcc_next_item(XCC_RES *res) {
    XCC_ITEM *item;
    char *buffer;
    int bytes,size,read;

    if(!res) return NULL;
    if(!res->request) return NULL;
    if(!res->request->session) return NULL;
    if(!res->has_next) return NULL;
    if(res->has_next == -1) {
        if(!xcc_has_next(res)) return NULL;
    }

    buffer = (char *)malloc(res->request->buffer_size);
    memset(buffer, '\0', res->request->buffer_size);

    bytes = res->request->buffer_size;
    item = (XCC_ITEM *)malloc(sizeof(XCC_ITEM));
    if(!item) {
        xcc_set_error(res->request->session, "failed to allocate memory for result item", XCC_ERROR_OUT_OF_MEM);
        free(buffer);
        return NULL;
    }
    memset(item, 0, sizeof(XCC_ITEM));

    item->buffer = (XCC_STRBUF *)malloc(sizeof(XCC_STRBUF));
    item->buffer = xcc_strbuf_init(item->buffer);
    item->content_type = NULL;
    item->x_primitive = NULL;

    while(!xcc_result_eof(res)) {
        read = xcc_result_getline(res, buffer, bytes);
        if(xcc_match_boundry(res, buffer)) {
            break;
        }
        size = strlen(buffer)*sizeof(char)+1;

        /* looks like Mark Logic returns 2 headers Content-Type and X-Primitive 
         * so we grab the info from these headers.    
         */
        if(!strncmp("Content-Type:", buffer, 13)) {
            item->content_type = (char *)malloc(bytes);
            memset(item->content_type, '\0', bytes);
            memcpy(item->content_type, &buffer[14], size - (16 * sizeof(char)));
        } else if(!strncmp("X-Primitive:", buffer, 12)) {
            item->x_primitive = (char *)malloc(bytes);
            memset(item->x_primitive, '\0', bytes);
            memcpy(item->x_primitive, &buffer[13], size - (15 * sizeof(char)));
            item->type = xcc_get_schema_type(item->x_primitive);
        } else if(strcmp("\n", buffer) != 0) {
            xcc_strbuf_append(item->buffer, buffer, read);
        }
    }

    free(buffer);

    return item;
}

void xcc_free_item(XCC_ITEM *item) {
    if(item->buffer) xcc_strbuf_free(item->buffer);
    if(item->content_type) free(item->content_type);
    if(item->x_primitive) free(item->x_primitive);
    free(item);
}

int xcc_has_next(XCC_RES *res) {
    char *buffer;
    int ret = 0;

    if(!res) return ret;
    if(!res->request) return ret;

    buffer = (char *)malloc(res->request->buffer_size);
    memset(buffer, '\0', res->request->buffer_size);

    if(res->has_next == 1) {
        ret = 1;
    } else if(res->has_next == -1) {
        while(!xcc_result_eof(res)) {
            xcc_result_getline(res, buffer, res->request->buffer_size);
            if(xcc_match_boundry(res, buffer)) {
                ret = 1;
                break;
            }
        }
    }

    free(buffer);

    return ret;
}

const char* xcc_as_string(XCC_ITEM *item) {
    return xcc_strbuf_tostring(item->buffer);
}

const char* xcc_as_text(XCC_ITEM *item) {
    return xcc_as_string(item);
}

double xcc_as_double(XCC_ITEM *item) {
    return atof(xcc_strbuf_tostring(item->buffer));
}

int xcc_as_int(XCC_ITEM *item) {
    return atoi(xcc_strbuf_tostring(item->buffer));
}

long xcc_as_long(XCC_ITEM *item) {
    return atol(xcc_strbuf_tostring(item->buffer));
}

int xcc_as_bool(XCC_ITEM *item) {
    const char *str;
    int ret = FALSE;

    str = xcc_as_string(item);
    if(!strcmp("true", str)) {
        ret = TRUE;
    }

    return ret;
}

void xcc_free_content(XCC_CONTENT *content) {
    if(!content) return;
    if(content->headers) curl_slist_free_all(content->headers);
    if(content->curl) curl_easy_cleanup(content->curl);
    if(content->url) free(content->url);
    if(content->uri) free(content->uri);
    if(content->ns) free(content->ns);
    if(content->error) xcc_strbuf_free(content->error);
    if(content->file) fclose(content->file);;
    xcc_clear_permissions(content);
    xcc_clear_collections(content);
    free(content);
}

void xcc_clear_permissions(XCC_CONTENT *content) {
    XCC_PERM *cur;
    XCC_PERM *next;

    if(!content) return;
    if(content->perm_head) {
        next = cur = content->perm_head->next;
        xcc_free_perm(content->perm_head);
        while(cur != NULL) {
            next = cur->next;
            xcc_free_perm(cur);
            cur = next;
        }

        content->perm_head = NULL;
    }

    return;
}

void xcc_clear_collections(XCC_CONTENT *content) {
    XCC_COL *cur;
    XCC_COL *next;

    if(!content) return;
    if(content->col_head) {
        next = cur = content->col_head->next;
        xcc_free_col(content->col_head);
        while(cur != NULL) {
            next = cur->next;
            xcc_free_col(cur);
            cur = next;
        }

        content->col_head = NULL;
    }

    return;
}

XCC_ERROR_CODE xcc_content_addperm(XCC_CONTENT *content, 
                                     XCC_PERMISSION capability, 
                                     const char *role) {
    XCC_PERM *perm;
    XCC_PERM *cur;

    if(!content) return XCC_ERROR_INVALID_ARG;

    perm = (XCC_PERM *)malloc(sizeof(XCC_PERM));
    memset(perm, 0, sizeof(XCC_PERM));
    perm->capability = capability;
    if(!role) role = "";
    perm->role = strdup(role);

    if(content->perm_head == NULL) {
        content->perm_head = perm;
        return XCC_OK;
    }

    cur = content->perm_head;
    while(cur->next != NULL) cur = cur->next;

    cur->next = perm;

    return XCC_OK;
}

XCC_ERROR_CODE xcc_content_addcol(XCC_CONTENT *content, const char *name) {
    XCC_COL *col;
    XCC_COL *cur;

    if(!content) return XCC_ERROR_INVALID_ARG;

    col = (XCC_COL *)malloc(sizeof(XCC_COL));
    memset(col, 0, sizeof(XCC_COL));
    if(!name) name = "";
    col->name = strdup(name);

    if(!content->col_head) {
        content->col_head = col;
        return XCC_OK;
    }

    cur = content->col_head;
    while(cur->next != NULL) cur = cur->next;

    cur->next = col;

    return XCC_OK;
}

XCC_ERROR_CODE xcc_content_setop(XCC_CONTENT *content, XCC_CONTENT_OPTION opt, ...) {
    va_list arg;
    int ret = XCC_OK;
    int stat_ret;
    char *filename;
    struct stat file_info;

    if(!content) return XCC_ERROR_INVALID_ARG;

    va_start(arg, opt);

    switch(opt) {
    case XCC_CONTENT_OPT_FILE:
        filename = va_arg(arg, char *);
        if(!filename) {
            ret = XCC_ERROR_INVALID_ARG;
            break;
        }
        stat_ret = stat(filename, &file_info);
        if(stat_ret == -1) {
            ret = XCC_ERROR_FILE_IO;
            break;
        }
        content->size = file_info.st_size;
        content->file = fopen(filename, "rb");
        if(!content->file) {
            ret = XCC_ERROR_FILE_IO;
            break;
        }
        break;
    case XCC_CONTENT_OPT_DATA:
        /* XXX this seems very bad. need to change this soon. Need to find a way
               for the binding libraries to send along the size of the data to upload. using
               strlen seems very sketchy. right now the swig code sends along a -1 indicating 
               to use strlen */
        content->data = va_arg(arg, char *);
        content->size = (long long)va_arg(arg, long);
        if(!content->data) {
            ret = XCC_ERROR_INVALID_ARG;
        } 
        if(content->size == -1LL) {
            content->size = (long long)strlen(content->data);
        } 
        if(content->size <= 0) {
            ret = XCC_ERROR_INVALID_ARG;
        }
        break;
    case XCC_CONTENT_OPT_NS:
        content->ns = xcc_to_string("%s", va_arg(arg, char *));
        break;
    case XCC_CONTENT_OPT_URI:
        content->uri = xcc_to_string("%s", va_arg(arg, char *));
        break;
    case XCC_CONTENT_OPT_TIMEOUT:
        content->timeout = va_arg(arg, int);
        break;
    case XCC_CONTENT_OPT_QUALITY:
        content->quality = va_arg(arg, int);
        break;
    case XCC_CONTENT_OPT_FORMAT:
        content->format = va_arg(arg, int);
        break;
    case XCC_CONTENT_OPT_REPAIR:
        content->repair = va_arg(arg, int);
        break;
    default:
        ret = XCC_ERROR_UNKNOWN_OPTION;
        break;
    }

    va_end(arg);
    return ret;
}

#define XCC_CONTENT_FREE() \
    xcc_set_error(session, curl_easy_strerror(code), XCC_ERROR_CURL); \
    xcc_free_content(content);

XCC_CONTENT* xcc_new_content(XCC_SESS *session) {
    CURLcode code;
    XCC_CONTENT *content;

    content = (XCC_CONTENT *)malloc(sizeof(XCC_CONTENT));
    if(!content) {
        xcc_set_error(session, "failed to allocate memory for contentument struct", XCC_ERROR_OUT_OF_MEM);
        return NULL;
    }
    memset(content, 0, sizeof(content));

    content->session = session;
    content->timeout = XCC_DEFAULT_QUERY_TIMEOUT;
    content->perm_head = NULL;
    content->col_head = NULL;
    content->url = NULL;
    content->ns = NULL;
    content->uri = NULL;
    content->file = NULL;
    content->quality = 0;
    content->offset = 0;
    content->size = 0;
    content->repair = XCC_CONTENT_ERROR_CORRECTION_NONE;
    content->format = XCC_CONTENT_FORMAT_XML;
    content->data = NULL;
    content->error = (XCC_STRBUF *)malloc(sizeof(XCC_STRBUF));
    memset(content->error, 0, sizeof(XCC_STRBUF));
    content->error = xcc_strbuf_init(content->error);
    content->curl = curl_easy_init();
    if(!content->curl) {
        xcc_set_error(session, "Failed to initialize curl connection", XCC_ERROR_CURL);
        xcc_free_content(content);
        return NULL;
    }

#ifdef DEBUG
    code = curl_easy_setopt(content->curl, CURLOPT_VERBOSE, TRUE);
    if(code != CURLE_OK) {
        XCC_CONTENT_FREE();
        return NULL;
    }
#endif


    /* Set custom HTTP Headers */
    content->headers = NULL;
    content->headers = curl_slist_append(content->headers, XCC_USER_AGENT);

    /* Set the headers to be the same as the Java Library. Seems odd to have 
       to turn off these headers but looks like that's what the server is
       expecting. Also the Content-Type seems to be always set to text/xml
       even when uploading plain text and binary. */
    content->headers = curl_slist_append(content->headers, "Content-Type: text/xml");
    content->headers = curl_slist_append(content->headers, "Expect:");
    content->headers = curl_slist_append(content->headers, "Transfer-Encoding:");
    content->headers = curl_slist_append(content->headers, "Pragma:");

    code = curl_easy_setopt(content->curl, CURLOPT_HTTPHEADER, content->headers);
    if(code != CURLE_OK) {
        XCC_CONTENT_FREE();
        return NULL;
    }

    code = curl_easy_setopt(content->curl, CURLOPT_WRITEFUNCTION, xcc_write_content_curldata);
    if(code != CURLE_OK) {
        XCC_CONTENT_FREE();
        return NULL;
    }

    code = curl_easy_setopt(content->curl, CURLOPT_READFUNCTION, xcc_read_content_data);
    if(code != CURLE_OK) {
        XCC_CONTENT_FREE();
        return NULL;
    }

    code = curl_easy_setopt(content->curl, CURLOPT_USERPWD, content->session->userpw);
    if(code != CURLE_OK) {
        XCC_CONTENT_FREE();
        return NULL;
    }

    code = curl_easy_setopt(content->curl, CURLOPT_UPLOAD, TRUE);
    if(code != CURLE_OK) {
        XCC_CONTENT_FREE();
        return NULL;
    }

    code = curl_easy_setopt(content->curl, CURLOPT_PUT, TRUE);
    if(code != CURLE_OK) {
        XCC_CONTENT_FREE();
        return NULL;
    }

    code = curl_easy_setopt(content->curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
    if(code != CURLE_OK) {
        XCC_CONTENT_FREE();
        return NULL;
    }

    return content;
}

#define XCC_INSERT_FREE() \
    xcc_set_error(content->session, curl_easy_strerror(code), XCC_ERROR_CURL); \

xcc_off_t xcc_insert_content(XCC_CONTENT *content) {
    CURLcode code;
    XCC_ERROR_CODE errcode;
    xcc_off_t ret = 0;
    int http_res_code;

    if(!content) { 
        return ret;
    }
    if(!content->session) { 
        return ret;
    }
    if(!content->curl) { 
        xcc_set_error(content->session, "content curl handle was not intialized", XCC_ERROR_INVALID_ARG);
        return ret;
    }
    if(content->size <= 0) { 
        xcc_set_error(content->session, "you must provide the size of the contentument to be loaded", XCC_ERROR_INVALID_ARG);
        return ret;
    }
    if((errcode = xcc_content_build_url(content)) != XCC_OK) {
        xcc_set_error(content->session, "failed to build contentument upload url", errcode);
        return ret;
    }

    code = curl_easy_setopt(content->curl, CURLOPT_URL, content->url);
    if(code != CURLE_OK) {
        XCC_INSERT_FREE();
        return ret;
    }

    code = curl_easy_setopt(content->curl, CURLOPT_WRITEDATA, content);
    if(code != CURLE_OK) {
        XCC_INSERT_FREE();
        return ret;
    }

    code = curl_easy_setopt(content->curl, CURLOPT_READDATA, content);
    if(code != CURLE_OK) {
        XCC_INSERT_FREE();
        return ret;
    }

    /* By not setting the size of the data to upload we go into chunked encoding mode.
       But we remove the Transfer-Encoding header anyway because Mark Logic doesn't
       seem to pay attention to that or the Content-Length header.  
    */
    /*
    code = curl_easy_setopt(content->curl, CURLOPT_INFILESIZE_LARGE, content->size);
    if(code != CURLE_OK) {
        XCC_INSERT_FREE();
        return ret;
    }
    */

    content->read_started = FALSE;
    content->read_ended = FALSE;

    code = curl_easy_perform(content->curl);
    if(code != CURLE_OK) {
        XCC_INSERT_FREE();
        return ret;
    }

    code = curl_easy_getinfo(content->curl, CURLINFO_RESPONSE_CODE, &http_res_code);
    if(code != CURLE_OK) {
        XCC_INSERT_FREE();
        return ret;
    }

    if(http_res_code == XCC_HTTP_RES_ERROR) {
        xcc_set_error(content->session, xcc_strbuf_tostring(content->error), XCC_ERROR_XQUERY);
        return ret;
    } else if(http_res_code == XCC_HTTP_AUTH_FAILURE) {
        xcc_set_error(content->session, "Authentication failure. Please check username/password.", XCC_ERROR_AUTH_FAILURE);
        return ret;
    } else if(http_res_code != XCC_HTTP_RES_OK) {
        xcc_set_error(content->session, "Unknown HTTP Response code returned", XCC_ERROR_HTTP_CODE);
        return ret;
    }

    ret = content->size;
    if(content->file) rewind(content->file);
    content->offset = 0;

    // xcc_free_content(content);

    return ret;
}

const char* xcc_error_message(XCC_ERROR_CODE code) {
    const char *message;
    switch(code) {
    case XCC_ERROR_CURL:
        message = "There has been an internal error with the curl library. This is most likley due to an invalid paramter being set";
        break;
    case XCC_ERROR_XQUERY:
        message = "An error has occured trying to issue an XQuery statement against the server. Call xcc_error(xcc) for the XML error output from the server.";
        break;
    case XCC_ERROR_HTTP_CODE:
        message = "An unknown HTTP error was returned from the server.";
        break;
    case XCC_ERROR_GROW_BUFFER:
        message = "Failed to grow the strbuffer. Most likely due to a memory allocation problem.";
        break;
    case XCC_ERROR_OUT_OF_MEM:
        message = "Out of memory!";
        break;
    case XCC_ERROR_INVALID_ARG:
        message = "In invalid argument was passed to a function. Please check the API docs for correct arguments.";
        break;
    case XCC_ERROR_UNKNOWN_OPTION:
        message = "Trying to set an option that is not defined in an enum.";
        break;
    case XCC_ERROR_UNKNOWN_TYPE:
        message = "Trying to use a schema type that is not defined.";
        break;
    case XCC_ERROR_STMT_URL:
        message = "Failed to allocate memory when building the statment url string.";
        break;
    case XCC_ERROR_BUILD_QUERY:
        message = "Failed to build the query string for use in issuing an XQuery statement against the server.";
        break;
    case XCC_ERROR_CONNECTION:
        message = "Connection test failed. Tried to issue a query to get information about the server and this query failed.";
        break;
    case XCC_ERROR_AUTH_FAILURE:
        message = "Authentication error. Please check that you have provided a valid username/password.";
        break;
    case XCC_ERROR_FILE_IO:
        message = "Failed to stat/read file for inserting. Please check filename/permissions are valid.";
        break;
    default:
        message = "Unknown error code";
        break;
    }

    return message;
}

const void* xcc_as_binary(XCC_ITEM *item) {
    if(!item) return NULL;
    return item->buffer->buffer;
}

const char* xcc_server_info(XCC_SESS *session) {
    XCC_REQ *request;
    XCC_RES *result;
    XCC_ITEM *item;
    const char *query;

    if(!session) return NULL;

    if(!session->server_string) {
        query = "concat(xdmp:product-name(), \" \",xdmp:product-edition(),\" - \",xdmp:version(),\" (\", xdmp:platform(),\")\")";
        if(!(request = xcc_new_adhoc_query(session, query))) {
            return NULL;
        }

        if(!(result = xcc_submit_request(request))) {
            xcc_free_request(request);
            return NULL;
        }

        if(xcc_has_next(result)) {
            item = xcc_next_item(result);
            if(XCC_TYPE_STRING == item->type) {
                session->server_string = strdup(xcc_as_string(item));
            }
            xcc_free_item(item);
        }

        xcc_free_result(result);
        xcc_free_request(request);
    }

    return session->server_string;
}

int xcc_item_size(XCC_ITEM *item) {
    if(!item) return 0;
    return item->buffer->length-1;
}

XCC_SCHEMA_TYPE xcc_item_type(XCC_ITEM *item) {
    if(!item) return XCC_TYPE_UNKNOWN;
    return item->type;
}
