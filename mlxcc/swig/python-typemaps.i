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

%typemap(out) XCC_SCHEMA_TYPE {
    $result = PyInt_FromLong((long)$1);
}

/* XXX need to check defines for large file support. off_t won't
   always be a long long. In for now to test large files */
%typemap(out) xcc_off_t {
    $result = PyLong_FromLongLong($1);
}
