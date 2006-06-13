#=============================================================================
#
# Copyright 2005 Andrew Bruno <aeb@qnot.org> 
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License. You may obtain a copy of
# the License at 
#
#  http://www.apache.org/licenses/LICENSE-2.0 
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations under
# the License.
#
#============================================================================

AC_PREREQ(2.57)
AC_INIT([mlxcc], [-], [Andrew Bruno => aeb@qnot.org])
AC_CONFIG_AUX_DIR(config)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

# Checks for programs.
AC_PROG_CC

AC_PATH_PROG( SED, sed, sed-was-not-found-by-configure, $PATH:/usr/bin:/usr/local/bin)
AC_SUBST(SED)

VERSION=`$SED -ne 's/^#define MLXCC_VERSION "\(.*\)"/\1/p' ${srcdir}/include/marklogic/globals.h`
AM_INIT_AUTOMAKE(mlxcc, $VERSION)
AC_MSG_CHECKING([mlxcc version])
AC_MSG_RESULT($VERSION)

AC_PROG_LIBTOOL

# Checks for libraries.
LIBCURL_CHECK_CONFIG(["yes"], [], [], [AC_MSG_ERROR([libcurl was not found or is not usable! Please install the libcurl library or specify the path using --withlibcurl])])

# Checks for header files.
AC_CHECK_HEADERS([unistd.h strings.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T
AC_HEADER_TIME

# Checks for library functions.
AC_FUNC_MALLOC
AC_HEADER_STDC
AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_CHECK_FUNCS([memmove memset select strdup strcpy strcat strncmp strcmp])

AC_CONFIG_FILES([
	Makefile 
	src/Makefile
	test/Makefile
	docs/Makefile
	docs/Doxyfile
	packages/Makefile
	packages/mlxcc.spec
	packages/ruby-xcc.spec
	packages/perl-xcc.spec
	swig/Makefile
	include/Makefile
	include/marklogic/Makefile
])
AC_OUTPUT
