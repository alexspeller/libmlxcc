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

#ifndef MARKLOGIC_GLOBAL_H
#define MARKLOGIC_GLOBAL_H

/* mlxcc version */
#define MLXCC_VERSION "0.5.3"
#define MLXCC_VERSION_MAJOR 0
#define MLXCC_VERSION_MINOR 5
#define MLXCC_VERSION_PATCH 3

/* results from configure script */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* XXX FIX me once we have configure script */
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <wchar.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>

#ifdef  __cplusplus
extern "C" {
#endif

/* XXX FIX me this is for large file support */
typedef long long xcc_off_t;
#define XCC_FORMAT_OFF_T "%lld"

/* Boolean values */
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/** @defgroup SchemaTypes */
/*@{*/
/** @enum XCC_SCHEMA_TYPE
 * List of all XML Schema types supported by the API.
 */
typedef enum {
    XCC_TYPE_ANYURI,
    XCC_TYPE_BASE64BINARY,
    XCC_TYPE_BINARY,
    XCC_TYPE_BOOLEAN,
    XCC_TYPE_DATE,
    XCC_TYPE_DATETIME,
    XCC_TYPE_DAYTIMEDURATION,
    XCC_TYPE_DECIMAL,
    XCC_TYPE_DOUBLE,
    XCC_TYPE_DURATION,
    XCC_TYPE_FLOAT,
    XCC_TYPE_GDAY,
    XCC_TYPE_GMONTH,
    XCC_TYPE_GMONTHDAY,
    XCC_TYPE_GYEAR,
    XCC_TYPE_GYEARMONTH,
    XCC_TYPE_HEXBINARY,
    XCC_TYPE_INTEGER,
    XCC_TYPE_NODE,
    XCC_TYPE_QNAME,
    XCC_TYPE_STRING,
    XCC_TYPE_TEXT,
    XCC_TYPE_TIME,
    XCC_TYPE_YEARMONTHDURATION,
    XCC_TYPE_UNKNOWN
} XCC_SCHEMA_TYPE;
/*@}*/

/* #define XCC_OK      0 */

/** @defgroup ErrorCodes */
/*@{*/
/** @enum XCC_ERROR_CODE 
 * XCC error codes
 */
typedef enum {
    XCC_OK = 0,
    XCC_ERROR_CURL = 1000,
    XCC_ERROR_XQUERY = 1001,
    XCC_ERROR_HTTP_CODE = 1002,
    XCC_ERROR_GROW_BUFFER = 1003,
    XCC_ERROR_OUT_OF_MEM = 1004,
    XCC_ERROR_INVALID_ARG = 1005,
    XCC_ERROR_UNKNOWN_OPTION = 1006,
    XCC_ERROR_UNKNOWN_TYPE = 1007,
    XCC_ERROR_STMT_URL = 1008,
    XCC_ERROR_BUILD_QUERY = 1009,
    XCC_ERROR_CONNECTION = 1010,
    XCC_ERROR_AUTH_FAILURE = 1011,
    XCC_ERROR_FILE_IO = 1012,
    XCC_ERROR_FATAL = 1013,
    XCC_ERROR_UNSUPPORTED = 1014
} XCC_ERROR_CODE;
/*@}*/

/** @defgroup RequestOptions */
/*@{*/
/** @enum XCC_REQ_OPTION 
 * XCC request options
 */
typedef enum {
    XCC_REQ_BUFSIZE,
    XCC_REQ_TIMEOUT,
    XCC_REQ_DBNAME,
    XCC_REQ_DBID,
    XCC_REQ_TIMESTAMP,
    XCC_REQ_NAME
} XCC_REQ_OPTION;
/*@}*/

/** @defgroup RequestType */
/*@{*/
/** @enum XCC_REQ_TYPE 
 * XCC request types
 */
typedef enum {
    XCC_REQ_TYPE_QUERY,
    XCC_REQ_TYPE_INVOKE,
    XCC_REQ_TYPE_SPAWN
} XCC_REQ_TYPE;
/*@}*/

/** @defgroup ContentFormatOptions */
/*@{*/
/** @enum XCC_CONTENT_FORMAT 
 * XCC Content Formatting options
 */
typedef enum {
    XCC_CONTENT_FORMAT_NONE = 0,
    XCC_CONTENT_FORMAT_BINARY = 1,
    XCC_CONTENT_FORMAT_TEXT = 2,
    XCC_CONTENT_FORMAT_XML = 3
} XCC_CONTENT_FORMAT;
/*@}*/

/** @defgroup ContentErrorCorrection */
/*@{*/
/** @enum XCC_CONTENT_ERROR_CORRECTION 
 * XCC Content Error correction options
 */
typedef enum {
    XCC_CONTENT_ERROR_CORRECTION_NONE = 0,
    XCC_CONTENT_ERROR_CORRECTION_FULL = 1
} XCC_CONTENT_ERROR_CORRECTION;
/*@}*/

/** @defgroup Permissions */
/*@{*/
/** @enum XCC_PERMISSION 
 * XCC Content Permissions
 */
typedef enum {
    XCC_PERMISSION_INSERT = 2,
    XCC_PERMISSION_READ = 3,
    XCC_PERMISSION_UPDATE = 4,
    XCC_PERMISSION_EXECUTE = 5
} XCC_PERMISSION;
/*@}*/

/** @defgroup ContentOptions */
/*@{*/
/** @enum XCC_CONTENT_OPTION 
 * XCC Content Options
 */
typedef enum {
    XCC_CONTENT_OPT_FILE,
    XCC_CONTENT_OPT_DATA,
    XCC_CONTENT_OPT_NS,
    XCC_CONTENT_OPT_TIMEOUT,
    XCC_CONTENT_OPT_QUALITY,
    XCC_CONTENT_OPT_FORMAT,
    XCC_CONTENT_OPT_URI,
    XCC_CONTENT_OPT_REPAIR
} XCC_CONTENT_OPTION;
/*@}*/

/* XCC Specific Config */
/* Spoof user agent for now */
#define XCC_USER_AGENT                      "User-Agent: Java/1.5.0_02 MarkXDBC/2.2-8"
#define XCC_EVAL_PATH                       "/eval"
#define XCC_INVOKE_PATH                     "/invoke"
#define XCC_SPAWN_PATH                      "/spawn"
#define XCC_UPLOAD_PATH                     "/insert"
#define XCC_MAX_PORT_LENGTH                 20
#define XCC_HTTP_RES_ERROR                  500
#define XCC_HTTP_AUTH_FAILURE               401
#define XCC_HTTP_RES_OK                     200
#define XCC_HTTP_NOT_FOUND                  404
#define XCC_ERROR_SIZE                      5000

/* Defaults */
#define XCC_STRBUF_CHUNKSIZE                1*1024 /* 1KB */
#define XCC_DEFAULT_BUFSIZE                 256
#define XCC_DEFAULT_QUERY_TIMEOUT           60
#define XCC_DEFAULT_PORT                    8002

/**
 * @todo lots more testing. test compiling on other platforms. large documents.
 * @todo Don't know anything about encodings. check to see if char * work for UTF-8.
 * @todo test uploading binary data and also retrieving binary data from result set
 * @todo need to finish all schema types. figure out how to support other types like Dates
 * @todo add in support for executing xquery files. New features in 3.0
 * @todo implement better error handling mechanism. Remove error codes if not needed?
 * @todo test thread safe?
 * 
 */

/** @mainpage mlxcc - Mark Logic XML Content Connector C API 
 *
 * \section intro_sec About
 *
 * mlxcc is a C API for communicating with Mark Logic. It provides an
 * implementation of the underlying wire protocol used to communicate with the
 * server and is based off of the Java XCC/J libraries provided by Mark Logic. The
 * goal of mlxcc is to provide a base interface into Mark Logic in which bindings
 * into other popular languages can be built.<BR>
 * <BR>
 * mlxcc supports most of the features of the Java XCC/J libraries including
 * executing XQuery statements, external variables, and document inserting.
 * mlxcc is still very experimental and the API is subject to change. Any feedback,
 * patches, testing would be greatly appreciated. <BR>
 * <BR>
 * For more information about Mark Logic: http://xqzone.marklogic.com
 *
 * \section docs_sec Documentation
 * See the <A HREF="group__PublicAPI.html">PublicAPI documentation</A><BR>
 * more to come..
 *
 */

#ifdef  __cplusplus
}
#endif

#endif /* globals.h */
