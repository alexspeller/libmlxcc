INCLUDES = -I$(top_srcdir)/include

EXTRA_DIST = data.xml downloadbug.gif test_auth.h

#noinst_PROGRAMS = query load load_binary query_binary thread
check_PROGRAMS = query load load_binary query_binary thread

query_SOURCES = query.c
query_LDADD = $(top_builddir)/src/libmlxcc.la
query_DEPENDENCIES = $(top_builddir)/src/libmlxcc.la

query_binary_SOURCES = query_binary.c
query_binary_LDADD = $(top_builddir)/src/libmlxcc.la
query_binary_DEPENDENCIES = $(top_builddir)/src/libmlxcc.la

load_SOURCES = load.c
load_LDADD = $(top_builddir)/src/libmlxcc.la
load_DEPENDENCIES = $(top_builddir)/src/libmlxcc.la

load_binary_SOURCES = load_binary.c
load_binary_LDADD = $(top_builddir)/src/libmlxcc.la
load_binary_DEPENDENCIES = $(top_builddir)/src/libmlxcc.la

thread_SOURCES = thread.c
thread_LDADD = $(top_builddir)/src/libmlxcc.la
thread_LDFLAGS = -lpthread
thread_DEPENDENCIES = $(top_builddir)/src/libmlxcc.la
