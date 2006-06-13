INCLUDES = -I$(top_srcdir)/include
lib_LTLIBRARIES = libmlxcc.la

libmlxcc_la_CFLAGS = @LIBCURL_CPPFLAGS@
libmlxcc_la_LIBADD = @LIBCURL@
libmlxcc_la_SOURCES = xcc.c xcc_strbuf.c
libmlxcc_la_LDFLAGS = -version-info 1:0:1
