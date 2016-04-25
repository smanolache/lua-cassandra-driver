ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src
EXTRA_DIST = Makefile.cvs src/unit/testDriver src/unit/setup.cql m4 debian

lib_LTLIBRARIES = liblua5.1-cassandra-driver.la

liblua5_1_cassandra_driver_la_SOURCES =
liblua5_1_cassandra_driver_la_LIBADD = \
	src/libwrapper.la \
	#

liblua5_1_cassandra_driver_la_LDFLAGS = -version-info 0:0:0 \
	-lcassandra \
	-luuid \
	#

install-exec-hook:
	mkdir -p $(DESTDIR)$(libdir)/lua/5.1/db
	ln -sf ../../../liblua5.1-cassandra-driver.so $(DESTDIR)$(libdir)/lua/5.1/db/cassandra.so
