# Based on configuration script used by gpsd

#
# Build shared library
#
libttyproc_la_LDFLAGS = -version-number 1:0:1
lib_LTLIBRARIES = libttyproc.la

libttyproc_la_SOURCES = \
	ttyproc.c

#
# Install Manpages
#
man_MANS = \
	man/man3/*.3

include_HEADERS = ttyproc.h

EXTRA_DIST = \
	examples \
	$(man_MANS) \
	html

# This is not distributed (used to test library)
test: libttyproc.la
	$(CC) -o test -DTESTLIB ttyproc.c .libs/libttyproc.a
