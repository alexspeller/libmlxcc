dnl $Id$
dnl config.m4 for extension xcc

AC_DEFUN([XCC_LIB_CHK], [
  str="$XCC_DIR/$1/libmlxcc.*"
  for j in `echo $str`; do
    if test -r $j; then
      XCC_LIB_DIR=$XCC_DIR/$1
      break 2
    fi
  done
])


PHP_ARG_WITH(xcc, for Mark Logic XCC support,
[  --with-xcc[=DIR]      Include Mark Logic XCC support. DIR is the libmlxcc base directory.
                          If unspecified, the bundled libmlxcc library will be used.], yes)


if test "$PHP_XCC" = "yes"; then
  XCC_MODULE_TYPE=builtin
  sources="libmlxcc/src/xcc.c libmlxcc/src/xcc_strbuf.c"

  PHP_NEW_EXTENSION(xcc, php_xcc.c $sources, $ext_shared,,-I@ext_srcdir@/libmlxcc/include)
  PHP_ADD_BUILD_DIR($ext_builddir/libmlxcc/src)

elif test "$PHP_XCC" != "no"; then

  PHP_NEW_EXTENSION(xcc, php_xcc.c, $ext_shared)

  for i in $PHP_XCC; do
    if test -r $i/include/marklogic/xcc.h; then
      XCC_DIR=$i
      XCC_INC_DIR=$i/include
    fi
  done

  if test -z "$XCC_DIR"; then
    AC_MSG_ERROR(Cannot find Mark Logic XCC header files under $PHP_XCC)
  fi

  XCC_MODULE_TYPE=external

  for i in lib; do
    XCC_LIB_CHK($i)
  done

  if test -z "$XCC_LIB_DIR"; then
    AC_MSG_ERROR(Cannot find libmlxcc library under $XCC_DIR)
  fi

  PHP_CHECK_LIBRARY(xcc, xcc_new_session, [ ], [ ], [ -L$XCC_LIB_DIR ])

  PHP_ADD_LIBRARY_WITH_PATH(mlxcc, $XCC_LIB_DIR, XCC_SHARED_LIBADD)
  XCC__LIBS="-L$XCC_LIB_DIR -lmlxcc $XCC_LIBS"

  PHP_ADD_INCLUDE($XCC_INC_DIR)
  XCC__INCLUDE=-I$XCC_INC_DIR

else
  XCC_MODULE_TYPE=none
fi

PHP_SUBST(XCC_SHARED_LIBADD)
PHP_SUBST_OLD(XCC_MODULE_TYPE)
PHP_SUBST_OLD(XCC_LIBS)
PHP_SUBST_OLD(XCC_INCLUDE)
