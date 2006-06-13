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
    $result = sv_newmortal();
    sv_setiv($result, (IV) $1);
    argvi++;
}

/* XXX This is wrong. Need some way to return a long long value for 
   large file support. */
%typemap(out) xcc_off_t {
    $result = sv_newmortal();
    sv_setuv($result, (UV)$1);
    argvi++;
}
