package = "lua-cassandra-driver"
version = "0.0.0-0"
supported_platforms = {"linux", "macosx"}
source = {
  url = "git://github.com/smanolache/lua-cassandra-driver.git",
}
description = {
  summary = "A lua wrapper around the datastax cassandra driver",
  license = "MIT"
}
external_dependencies = {
  LIBCASSANDRA = {
    header = "cassandra.h"
  },
  LIBUUID = {
    header = "uuid.h"
  }
}
build = {
  type = "builtin",
  modules = {
     ["db.cassandra"] = {
       sources = {"src/lua-cassandra.c"},
       libraries = {"cassandra", "uuid"},
       incdirs = {"$(LIBCASSANDRA_INCDIR)", "$(LIBUUID_INCDIR)"},
       libdirs = {"$(LIBCASSANDRA_LIBDIR)"}
     }
  }
}
