#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <cassandra.h>
#include <string.h>
#include <uuid/uuid.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <endian.h>
#include <pthread.h>
#if __BYTE_ORDER == __LITTLE_ENDIAN
#include <byteswap.h>
#endif

static pthread_mutex_t cb_mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t log_mtx = PTHREAD_MUTEX_INITIALIZER;

struct __CassFuture {
	CassFuture *future;
	int ref;
};

static int
new_cass_batch(lua_State *s, const CassBatch *batch) {
	if (NULL != batch) {
		const CassBatch **impl = (const CassBatch **)
			lua_newuserdata(s, sizeof(const CassBatch *));
		luaL_getmetatable(s, "datastax.cass_batch");
		lua_setmetatable(s, -2);
		*impl = batch;
	} else
		lua_pushnil(s);
	return 1;
}

static int
new_cass_collection(lua_State *s, const CassCollection *coll) {
	if (NULL != coll) {
		const CassCollection **impl = (const CassCollection **)
			lua_newuserdata(s, sizeof(const CassCollection *));
		luaL_getmetatable(s, "datastax.cass_collection");
		lua_setmetatable(s, -2);
		*impl = coll;
	} else
		lua_pushnil(s);
	return 1;
}

static int
new_cass_data_type(lua_State *s, const CassDataType *data_type) {
	if (NULL != data_type) {
		const CassDataType **impl = (const CassDataType **)
			lua_newuserdata(s, sizeof(const CassDataType *));
		luaL_getmetatable(s, "datastax.cass_data_type");
		lua_setmetatable(s, -2);
		*impl = data_type;
	} else
		lua_pushnil(s);
	return 1;
}

static int
new_cass_error_result(lua_State *s, const CassErrorResult *err) {
	if (NULL != err) {
		const CassErrorResult **impl = (const CassErrorResult **)
			lua_newuserdata(s, sizeof(const CassErrorResult *));
		luaL_getmetatable(s, "datastax.cass_error_result");
		lua_setmetatable(s, -2);
		*impl = err;
	} else
		lua_pushnil(s);
	return 1;
}

static int
new_cass_future(lua_State *s, CassFuture *impl) {
	if (NULL != impl) {
		struct __CassFuture *future = (struct __CassFuture *)lua_newuserdata(s, sizeof(struct __CassFuture));
		luaL_getmetatable(s, "datastax.cass_future");
		lua_setmetatable(s, -2);
		future->future = impl;
		future->ref = LUA_NOREF;
	} else
		lua_pushnil(s);
	return 1;
}

static int
new_cass_iterator(lua_State *s, CassIterator *impl) {
	if (NULL != impl) {
		CassIterator **iterator = (CassIterator **)lua_newuserdata(s, sizeof(CassIterator *));
		luaL_getmetatable(s, "datastax.cass_iterator");
		lua_setmetatable(s, -2);
		*iterator = impl;
	} else
		lua_pushnil(s);
	return 1;
}

static int
new_cass_prepared(lua_State *s, const CassPrepared *impl) {
	if (NULL != impl) {
		const CassPrepared **prepared = (const CassPrepared **)
			lua_newuserdata(s, sizeof(const CassPrepared *));
		luaL_getmetatable(s, "datastax.cass_prepared");
		lua_setmetatable(s, -2);
		*prepared = impl;
	} else
		lua_pushnil(s);
	return 1;
}

static int
new_cass_metrics(lua_State *s, const CassMetrics *metrics) {
	if (NULL != metrics) {
		lua_newtable(s);

		lua_pushstring(s, "requests");
		lua_newtable(s);

		lua_pushstring(s, "min");
		lua_pushinteger(s, metrics->requests.min);
		lua_settable(s, -3);

		lua_pushstring(s, "max");
		lua_pushinteger(s, metrics->requests.max);
		lua_settable(s, -3);

		lua_pushstring(s, "mean");
		lua_pushinteger(s, metrics->requests.mean);
		lua_settable(s, -3);

		lua_pushstring(s, "stddev");
		lua_pushinteger(s, metrics->requests.stddev);
		lua_settable(s, -3);

		lua_pushstring(s, "median");
		lua_pushinteger(s, metrics->requests.median);
		lua_settable(s, -3);

		lua_pushstring(s, "percentile_75th");
		lua_pushinteger(s, metrics->requests.percentile_75th);
		lua_settable(s, -3);

		lua_pushstring(s, "percentile_95th");
		lua_pushinteger(s, metrics->requests.percentile_95th);
		lua_settable(s, -3);

		lua_pushstring(s, "percentile_98th");
		lua_pushinteger(s, metrics->requests.percentile_98th);
		lua_settable(s, -3);

		lua_pushstring(s, "percentile_99th");
		lua_pushinteger(s, metrics->requests.percentile_99th);
		lua_settable(s, -3);

		lua_pushstring(s, "percentile_999th");
		lua_pushinteger(s, metrics->requests.percentile_999th);
		lua_settable(s, -3);

		lua_pushstring(s, "mean_rate");
		lua_pushnumber(s, metrics->requests.mean_rate);
		lua_settable(s, -3);

		lua_pushstring(s, "one_minute_rate");
		lua_pushnumber(s, metrics->requests.one_minute_rate);
		lua_settable(s, -3);

		lua_pushstring(s, "five_minute_rate");
		lua_pushnumber(s, metrics->requests.five_minute_rate);
		lua_settable(s, -3);

		lua_pushstring(s, "fifteen_minute_rate");
		lua_pushnumber(s, metrics->requests.fifteen_minute_rate);
		lua_settable(s, -3);

		lua_settable(s, -3);

		lua_pushstring(s, "stats");
		lua_newtable(s);

		lua_pushstring(s, "total_connections");
		lua_pushinteger(s, metrics->stats.total_connections);
		lua_settable(s, -3);

		lua_pushstring(s, "available_connections");
		lua_pushinteger(s, metrics->stats.available_connections);
		lua_settable(s, -3);

		lua_pushstring(s, "exceeded_pending_requests_water_mark");
		lua_pushinteger(s, metrics->stats.exceeded_pending_requests_water_mark);
		lua_settable(s, -3);

		lua_pushstring(s, "exceeded_write_bytes_water_mark");
		lua_pushinteger(s, metrics->stats.exceeded_write_bytes_water_mark);
		lua_settable(s, -3);

		lua_settable(s, -3);

		lua_pushstring(s, "errors");
		lua_newtable(s);

		lua_pushstring(s, "connection_timeouts");
		lua_pushinteger(s, metrics->errors.connection_timeouts);
		lua_settable(s, -3);

		lua_pushstring(s, "pending_request_timeouts");
		lua_pushinteger(s, metrics->errors.pending_request_timeouts);
		lua_settable(s, -3);

		lua_pushstring(s, "request_timeouts");
		lua_pushinteger(s, metrics->errors.request_timeouts);
		lua_settable(s, -3);

		lua_settable(s, -3);
	} else
		lua_pushnil(s);
	return 1;
}

static int
new_cass_result(lua_State *s, const CassResult *impl) {
	if (NULL != impl) {
		const CassResult **result = (const CassResult **)
			lua_newuserdata(s, sizeof(const CassResult *));
		luaL_getmetatable(s, "datastax.cass_result");
		lua_setmetatable(s, -2);
		*result = impl;
	} else
		lua_pushnil(s);
	return 1;
}

static int
new_cass_retry_policy(lua_State *s, const CassRetryPolicy *pol) {
	if (NULL != pol) {
		const CassRetryPolicy **impl = (const CassRetryPolicy **)
			lua_newuserdata(s, sizeof(CassRetryPolicy *));
		luaL_getmetatable(s, "datastax.cass_retry_policy");
		lua_setmetatable(s, -2);
		*impl = pol;
	} else
		lua_pushnil(s);
	return 1;
}

static int
new_cass_schema_meta(lua_State *s, const CassSchemaMeta *meta) {
	if (NULL != meta) {
		const CassSchemaMeta **impl = (const CassSchemaMeta **)
			lua_newuserdata(s, sizeof(const CassSchemaMeta *));
		luaL_getmetatable(s, "datastax.cass_schema_meta");
		lua_setmetatable(s, -2);
		*impl = meta;
	} else
		lua_pushnil(s);
	return 1;
}

static int
new_cass_statement(lua_State *s, CassStatement *impl) {
	if (NULL != impl) {
		CassStatement **statement = (CassStatement **)lua_newuserdata(s, sizeof(CassStatement *));
		luaL_getmetatable(s, "datastax.cass_statement");
		lua_setmetatable(s, -2);
		*statement = impl;
	} else
		lua_pushnil(s);
	return 1;
}

static int
new_cass_timestamp_gen(lua_State *s, const CassTimestampGen *g) {
	if (NULL != g) {
		const CassTimestampGen **impl = (const CassTimestampGen **)
			lua_newuserdata(s, sizeof(const CassTimestampGen *));
		luaL_getmetatable(s, "datastax.cass_timestamp_gen");
		lua_setmetatable(s, -2);
		*impl = g;
	} else
		lua_pushnil(s);
	return 1;
}

static int
new_cass_tuple(lua_State *s, const CassTuple *tuple) {
	if (NULL != tuple) {
		const CassTuple **impl = (const CassTuple **)
			lua_newuserdata(s, sizeof(const CassTuple *));
		luaL_getmetatable(s, "datastax.cass_tuple");
		lua_setmetatable(s, -2);
		*impl = tuple;
	} else
		lua_pushnil(s);
	return 1;
}

static int
new_cass_user_type(lua_State *s, const CassUserType *ut) {
	if (NULL != ut) {
		const CassUserType **impl = (const CassUserType **)
			lua_newuserdata(s, sizeof(const CassUserType *));
		luaL_getmetatable(s, "datastax.cass_user_type");
		lua_setmetatable(s, -2);
		*impl = ut;
	} else
		lua_pushnil(s);
	return 1;
}

static int
new_cass_uuid_gen(lua_State *s, const CassUuidGen *ug) {
	if (NULL != ug) {
		const CassUuidGen **impl = (const CassUuidGen **)
			lua_newuserdata(s, sizeof(const CassUuidGen *));
		luaL_getmetatable(s, "datastax.cass_uuid_get");
		lua_setmetatable(s, -2);
		*impl = ug;
	} else
		lua_pushnil(s);
	return 1;
}

static int
new_cass_aggregate_meta(lua_State *s, const CassAggregateMeta *v) {
	if (NULL != v) {
		const CassAggregateMeta **impl = (const CassAggregateMeta **)
			lua_newuserdata(s, sizeof(const CassAggregateMeta *));
		luaL_getmetatable(s, "datastax.cass_aggregate_meta");
		lua_setmetatable(s, -2);
		*impl = v;
	} else
		lua_pushnil(s);
	return 1;
}

static int
new_cass_column_meta(lua_State *s, const CassColumnMeta *v) {
	if (NULL != v) {
		const CassColumnMeta **impl = (const CassColumnMeta **)
			lua_newuserdata(s, sizeof(const CassColumnMeta *));
		luaL_getmetatable(s, "datastax.cass_column_meta");
		lua_setmetatable(s, -2);
		*impl = v;
	} else
		lua_pushnil(s);
	return 1;
}

static int
new_cass_function_meta(lua_State *s, const CassFunctionMeta *v) {
	if (NULL != v) {
		const CassFunctionMeta **impl = (const CassFunctionMeta **)
			lua_newuserdata(s, sizeof(const CassFunctionMeta *));
		luaL_getmetatable(s, "datastax.cass_function_meta");
		lua_setmetatable(s, -2);
		*impl = v;
	} else
		lua_pushnil(s);
	return 1;
}

static int
new_cass_index_meta(lua_State *s, const CassIndexMeta *v) {
	if (NULL != v) {
		const CassIndexMeta **impl = (const CassIndexMeta **)
			lua_newuserdata(s, sizeof(const CassIndexMeta *));
		luaL_getmetatable(s, "datastax.cass_index_meta");
		lua_setmetatable(s, -2);
		*impl = v;
	} else
		lua_pushnil(s);
	return 1;
}

static int
new_cass_keyspace_meta(lua_State *s, const CassKeyspaceMeta *v) {
	if (NULL != v) {
		const CassKeyspaceMeta **impl = (const CassKeyspaceMeta **)
			lua_newuserdata(s, sizeof(const CassKeyspaceMeta *));
		luaL_getmetatable(s, "datastax.cass_keyspace_meta");
		lua_setmetatable(s, -2);
		*impl = v;
	} else
		lua_pushnil(s);
	return 1;
}

static int
new_cass_materialized_view_meta(lua_State *s, const CassMaterializedViewMeta *v) {
	if (NULL != v) {
		const CassMaterializedViewMeta **impl = (const CassMaterializedViewMeta **)
			lua_newuserdata(s, sizeof(const CassMaterializedViewMeta *));
		luaL_getmetatable(s, "datastax.cass_materialized_view_meta");
		lua_setmetatable(s, -2);
		*impl = v;
	} else
		lua_pushnil(s);
	return 1;
}

static int
new_cass_row(lua_State *s, const CassRow *v) {
	if (NULL != v) {
		const CassRow **impl = (const CassRow **)
			lua_newuserdata(s, sizeof(const CassRow *));
		luaL_getmetatable(s, "datastax.cass_row");
		lua_setmetatable(s, -2);
		*impl = v;
	} else
		lua_pushnil(s);
	return 1;
}

static int
new_cass_table_meta(lua_State *s, const CassTableMeta *v) {
	if (NULL != v) {
		const CassTableMeta **impl = (const CassTableMeta **)
			lua_newuserdata(s, sizeof(const CassTableMeta *));
		luaL_getmetatable(s, "datastax.cass_table_meta");
		lua_setmetatable(s, -2);
		*impl = v;
	} else
		lua_pushnil(s);
	return 1;
}

static int
new_cass_value(lua_State *s, const CassValue *v) {
	if (NULL != v) {
		const CassValue **impl = (const CassValue **)
			lua_newuserdata(s, sizeof(const CassValue *));
		luaL_getmetatable(s, "datastax.cass_value");
		lua_setmetatable(s, -2);
		*impl = v;
	} else
		lua_pushnil(s);
	return 1;
}

static CassBatch **
check_cass_batch(lua_State *s, int index) {
	void *ud = luaL_checkudata(s, index, "datastax.cass_batch");
	luaL_argcheck(s, NULL != ud, 1, "'cass_batch' expected");
	return (CassBatch **)ud;
}

static CassCluster **
check_cass_cluster(lua_State *s, int index) {
	void *ud = luaL_checkudata(s, index, "datastax.cass_cluster");
	luaL_argcheck(s, NULL != ud, 1, "'cass_cluster' expected");
	return (CassCluster **)ud;
}

static CassCollection **
check_cass_collection(lua_State *s, int index) {
	void *ud = luaL_checkudata(s, index, "datastax.cass_collection");
	luaL_argcheck(s, NULL != ud, 1, "'cass_collection' expected");
	return (CassCollection **)ud;
}

static CassCustomPayload **
check_cass_custom_payload(lua_State *s, int index) {
	void *ud = luaL_checkudata(s, index, "datastax.cass_custom_payload");
	luaL_argcheck(s, NULL != ud, 1, "'cass_custom_payload' expected");
	return (CassCustomPayload **)ud;
}

static CassDataType **
check_cass_data_type(lua_State *s, int index) {
	void *ud = luaL_checkudata(s, index, "datastax.cass_data_type");
	luaL_argcheck(s, NULL != ud, 1, "'cass_data_type' expected");
	return (CassDataType **)ud;
}

static CassErrorResult **
check_cass_error_result(lua_State *s, int index) {
	void *ud = luaL_checkudata(s, index, "datastax.cass_error_result");
	luaL_argcheck(s, NULL != ud, 1, "'cass_error_result' expected");
	return (CassErrorResult **)ud;
}

static struct __CassFuture *
check_cass_future(lua_State *s, int index) {
	void *ud = luaL_checkudata(s, index, "datastax.cass_future");
	luaL_argcheck(s, NULL != ud, 1, "'cass_future' expected");
	return (struct __CassFuture *)ud;
}

static CassIterator **
check_cass_iterator(lua_State *s, int index) {
	void *ud = luaL_checkudata(s, index, "datastax.cass_iterator");
	luaL_argcheck(s, NULL != ud, 1, "'cass_iterator' expected");
	return (CassIterator **)ud;
}

static CassPrepared **
check_cass_prepared(lua_State *s, int index) {
	void *ud = luaL_checkudata(s, index, "datastax.cass_prepared");
	luaL_argcheck(s, NULL != ud, 1, "'cass_prepared' expected");
	return (CassPrepared **)ud;
}

static CassResult **
check_cass_result(lua_State *s, int index) {
	void *ud = luaL_checkudata(s, index, "datastax.cass_result");
	luaL_argcheck(s, NULL != ud, 1, "'cass_result' expected");
	return (CassResult **)ud;
}

static CassRetryPolicy **
check_cass_retry_policy(lua_State *s, int index) {
	void *ud = luaL_checkudata(s, index, "datastax.cass_retry_policy");
	luaL_argcheck(s, NULL != ud, 1, "'cass_retry_policy' expected");
	return (CassRetryPolicy **)ud;
}

static CassSchemaMeta **
check_cass_schema_meta(lua_State *s, int index) {
	void *ud = luaL_checkudata(s, index, "datastax.cass_schema_meta");
	luaL_argcheck(s, NULL != ud, 1, "'cass_schema_meta' expected");
	return (CassSchemaMeta **)ud;
}

static CassSession **
check_cass_session(lua_State *s, int index) {
	void *ud = luaL_checkudata(s, index, "datastax.cass_session");
	luaL_argcheck(s, NULL != ud, 1, "'cass_session' expected");
	return (CassSession **)ud;
}

static CassSsl **
check_cass_ssl(lua_State *s, int index) {
	void *ud = luaL_checkudata(s, index, "datastax.cass_ssl");
	luaL_argcheck(s, NULL != ud, 1, "'cass_ssl' expected");
	return (CassSsl **)ud;
}

static CassStatement **
check_cass_statement(lua_State *s, int index) {
	void *ud = luaL_checkudata(s, index, "datastax.cass_statement");
	luaL_argcheck(s, NULL != ud, 1, "'cass_statement' expected");
	return (CassStatement **)ud;
}

static CassTimestampGen **
check_cass_timestamp_gen(lua_State *s, int index) {
	void *ud = luaL_checkudata(s, index, "datastax.cass_timestamp_gen");
	luaL_argcheck(s, NULL != ud, 1, "'cass_timestamp_gen' expected");
	return (CassTimestampGen **)ud;
}

static CassTuple **
check_cass_tuple(lua_State *s, int index) {
	void *ud = luaL_checkudata(s, index, "datastax.cass_tuple");
	luaL_argcheck(s, NULL != ud, 1, "'cass_tuple' expected");
	return (CassTuple **)ud;
}

static CassUserType **
check_cass_user_type(lua_State *s, int index) {
	void *ud = luaL_checkudata(s, index, "datastax.cass_user_type");
	luaL_argcheck(s, NULL != ud, 1, "'cass_user_type' expected");
	return (CassUserType **)ud;
}

static CassUuidGen **
check_cass_uuid_gen(lua_State *s, int index) {
	void *ud = luaL_checkudata(s, index, "datastax.cass_uuid_gen");
	luaL_argcheck(s, NULL != ud, 1, "'cass_uuid_gen' expected");
	return (CassUuidGen **)ud;
}

static CassAggregateMeta **
check_cass_aggregate_meta(lua_State *s, int index) {
	void *ud = luaL_checkudata(s, index, "datastax.cass_aggregate_meta");
	luaL_argcheck(s, NULL != ud, 1, "'cass_aggregate_meta' expected");
	return (CassAggregateMeta **)ud;
}

static CassColumnMeta **
check_cass_column_meta(lua_State *s, int index) {
	void *ud = luaL_checkudata(s, index, "datastax.cass_column_meta");
	luaL_argcheck(s, NULL != ud, 1, "'cass_column_meta' expected");
	return (CassColumnMeta **)ud;
}

static CassFunctionMeta **
check_cass_function_meta(lua_State *s, int index) {
	void *ud = luaL_checkudata(s, index, "datastax.cass_function_meta");
	luaL_argcheck(s, NULL != ud, 1, "'cass_function_meta' expected");
	return (CassFunctionMeta **)ud;
}

static CassIndexMeta **
check_cass_index_meta(lua_State *s, int index) {
	void *ud = luaL_checkudata(s, index, "datastax.cass_index_meta");
	luaL_argcheck(s, NULL != ud, 1, "'cass_index_meta' expected");
	return (CassIndexMeta **)ud;
}

static CassKeyspaceMeta **
check_cass_keyspace_meta(lua_State *s, int index) {
	void *ud = luaL_checkudata(s, index, "datastax.cass_keyspace_meta");
	luaL_argcheck(s, NULL != ud, 1, "'cass_keyspace_meta' expected");
	return (CassKeyspaceMeta **)ud;
}

static CassMaterializedViewMeta **
check_cass_materialized_view_meta(lua_State *s, int index) {
	void *ud = luaL_checkudata(s, index, "datastax.cass_materialized_view_meta");
	luaL_argcheck(s, NULL != ud, 1, "'cass_materialized_view_meta' expected");
	return (CassMaterializedViewMeta **)ud;
}

static CassRow **
check_cass_row(lua_State *s, int index) {
	void *ud = luaL_checkudata(s, index, "datastax.cass_row");
	luaL_argcheck(s, NULL != ud, 1, "'cass_row' expected");
	return (CassRow **)ud;
}

static CassTableMeta **
check_cass_table_meta(lua_State *s, int index) {
	void *ud = luaL_checkudata(s, index, "datastax.cass_table_meta");
	luaL_argcheck(s, NULL != ud, 1, "'cass_table_meta' expected");
	return (CassTableMeta **)ud;
}

static CassValue **
check_cass_value(lua_State *s, int index) {
	void *ud = luaL_checkudata(s, index, "datastax.cass_value");
	luaL_argcheck(s, NULL != ud, 1, "'cass_value' expected");
	return (CassValue **)ud;
}

static int
lc_cass_cluster_new(lua_State *s) {
	CassCluster *cluster = cass_cluster_new();
	if (NULL != cluster) {
		CassCluster **impl = (CassCluster **)lua_newuserdata(s, sizeof(CassCluster *));
		luaL_getmetatable(s, "datastax.cass_cluster");
		lua_setmetatable(s, -2);
		*impl = cluster;
	} else
		lua_pushnil(s);
	return 1;
}

static int
lc_cass_cluster_free(lua_State *s) {
	CassCluster **cluster = check_cass_cluster(s, 1);
	cass_cluster_free(*cluster);
	*cluster = NULL;
	return 0;
}

static int
lc_cass_cluster_set_contact_points(lua_State *s) {
	CassCluster *cluster = *check_cass_cluster(s, 1);
	size_t len;
	const char *contact_points = luaL_checklstring(s, 2, &len);
	CassError err = cass_cluster_set_contact_points(cluster, contact_points);
	lua_pushinteger(s, err);
	return 1;
}

static int
lc_cass_cluster_set_port(lua_State *s) {
	CassCluster *cluster = *check_cass_cluster(s, 1);
	int port = (int)luaL_checkinteger(s, 2);
	CassError err = cass_cluster_set_port(cluster, port);
	lua_pushinteger(s, err);
	return 1;
}

static int
lc_cass_cluster_set_ssl(lua_State *s) {
	CassCluster *cluster = *check_cass_cluster(s, 1);
	CassSsl *ssl = *check_cass_ssl(s, 2);
	cass_cluster_set_ssl(cluster, ssl);
	return 0;
}

static int
lc_cass_cluster_set_protocol_version(lua_State *s) {
	CassCluster *cluster = *check_cass_cluster(s, 1);
	int protocol_version = (int)luaL_checkinteger(s, 2);
	CassError err = cass_cluster_set_protocol_version(cluster, protocol_version);
	lua_pushinteger(s, err);
	return 1;
}

static int
lc_cass_cluster_set_num_threads_io(lua_State *s) {
	CassCluster *cluster = *check_cass_cluster(s, 1);
	unsigned int num_threads = (unsigned int)luaL_checkinteger(s, 2);
	CassError err = cass_cluster_set_num_threads_io(cluster, num_threads);
	lua_pushinteger(s, err);
	return 1;
}

static int
lc_cass_cluster_set_queue_size_io(lua_State *s) {
	CassCluster *cluster = *check_cass_cluster(s, 1);
	unsigned int queue_size = (unsigned int)luaL_checkinteger(s, 2);
	CassError err = cass_cluster_set_queue_size_io(cluster, queue_size);
	lua_pushinteger(s, err);
	return 1;
}

static int
lc_cass_cluster_set_queue_size_event(lua_State *s) {
	CassCluster *cluster = *check_cass_cluster(s, 1);
	unsigned int queue_size = (unsigned int)luaL_checkinteger(s, 2);
	CassError err = cass_cluster_set_queue_size_event(cluster, queue_size);
	lua_pushinteger(s, err);
	return 1;
}

/* static int */
/* lc_cass_cluster_set_queue_size_log(lua_State *s) { */
/*	CassCluster *cluster = *check_cass_cluster(s, 1); */
/* 	unsigned int queue_size = (unsigned int)luaL_checkinteger(s, 2); */
/* 	CassError err = cass_cluster_set_queue_size_log(cluster, queue_size); */
/* 	lua_pushinteger(s, err); */
/* 	return 1; */
/* } */

static int
lc_cass_cluster_set_core_connections_per_host(lua_State *s) {
	CassCluster *cluster = *check_cass_cluster(s, 1);
	unsigned int num_connections = (unsigned int)luaL_checkinteger(s, 2);
	CassError err = cass_cluster_set_core_connections_per_host(cluster, num_connections);
	lua_pushinteger(s, err);
	return 1;
}

static int
lc_cass_cluster_set_max_connections_per_host(lua_State *s) {
	CassCluster *cluster = *check_cass_cluster(s, 1);
	unsigned int num_connections = (unsigned int)luaL_checkinteger(s, 2);
	CassError err = cass_cluster_set_max_connections_per_host(cluster, num_connections);
	lua_pushinteger(s, err);
	return 1;
}

static int
lc_cass_cluster_set_reconnect_wait_time(lua_State *s) {
	CassCluster *cluster = *check_cass_cluster(s, 1);
	unsigned int wait_time = (unsigned int)luaL_checkinteger(s, 2);
	cass_cluster_set_reconnect_wait_time(cluster, wait_time);
	return 1;
}

static int
lc_cass_cluster_set_max_concurrent_creation(lua_State *s) {
	CassCluster *cluster = *check_cass_cluster(s, 1);
	unsigned int num_connections = (unsigned int)luaL_checkinteger(s, 2);
	CassError err = cass_cluster_set_max_concurrent_creation(cluster, num_connections);
	lua_pushinteger(s, err);
	return 1;
}

static int
lc_cass_cluster_set_max_concurrent_requests_threshold(lua_State *s) {
	CassCluster *cluster = *check_cass_cluster(s, 1);
	unsigned int num_requests = (unsigned int)luaL_checkinteger(s, 2);
	CassError err = cass_cluster_set_max_concurrent_requests_threshold(cluster, num_requests);
	lua_pushinteger(s, err);
	return 1;
}

static int
lc_cass_cluster_set_max_requests_per_flush(lua_State *s) {
	CassCluster *cluster = *check_cass_cluster(s, 1);
	unsigned int num_requests = (unsigned int)luaL_checkinteger(s, 2);
	CassError err = cass_cluster_set_max_requests_per_flush(cluster, num_requests);
	lua_pushinteger(s, err);
	return 1;
}

static int
lc_cass_cluster_set_write_bytes_high_water_mark(lua_State *s) {
	CassCluster *cluster = *check_cass_cluster(s, 1);
	unsigned int num_bytes = (unsigned int)luaL_checkinteger(s, 2);
	CassError err = cass_cluster_set_write_bytes_high_water_mark(cluster, num_bytes);
	lua_pushinteger(s, err);
	return 1;
}

static int
lc_cass_cluster_set_write_bytes_low_water_mark(lua_State *s) {
	CassCluster *cluster = *check_cass_cluster(s, 1);
	unsigned int num_bytes = (unsigned int)luaL_checkinteger(s, 2);
	CassError err = cass_cluster_set_write_bytes_low_water_mark(cluster, num_bytes);
	lua_pushinteger(s, err);
	return 1;
}

static int
lc_cass_cluster_set_pending_requests_high_water_mark(lua_State *s) {
	CassCluster *cluster = *check_cass_cluster(s, 1);
	unsigned int num_requests = (unsigned int)luaL_checkinteger(s, 2);
	CassError err = cass_cluster_set_pending_requests_high_water_mark(cluster, num_requests);
	lua_pushinteger(s, err);
	return 1;
}

static int
lc_cass_cluster_set_pending_requests_low_water_mark(lua_State *s) {
	CassCluster *cluster = *check_cass_cluster(s, 1);
	unsigned int num_requests = (unsigned int)luaL_checkinteger(s, 2);
	CassError err = cass_cluster_set_pending_requests_low_water_mark(cluster, num_requests);
	lua_pushinteger(s, err);
	return 1;
}

static int
lc_cass_cluster_set_connect_timeout(lua_State *s) {
	CassCluster *cluster = *check_cass_cluster(s, 1);
	unsigned int timeout_ms = (unsigned int)luaL_checkinteger(s, 2);
	cass_cluster_set_connect_timeout(cluster, timeout_ms);
	return 0;
}

static int
lc_cass_cluster_set_request_timeout(lua_State *s) {
	CassCluster *cluster = *check_cass_cluster(s, 1);
	unsigned int timeout_ms = (unsigned int)luaL_checkinteger(s, 2);
	cass_cluster_set_request_timeout(cluster, timeout_ms);
	return 0;
}

static int
lc_cass_cluster_set_credentials(lua_State *s) {
	CassCluster *cluster = *check_cass_cluster(s, 1);
	size_t len;
	const char *username = luaL_checklstring(s, 2, &len);
	const char *password = luaL_checklstring(s, 3, &len);
	cass_cluster_set_credentials(cluster, username, password);
	return 0;
}

static int
lc_cass_cluster_set_load_balance_round_robin(lua_State *s) {
	CassCluster *cluster = *check_cass_cluster(s, 1);
	cass_cluster_set_load_balance_round_robin(cluster);
	return 0;
}

static int
lc_cass_cluster_set_load_balance_dc_aware(lua_State *s) {
	CassCluster *cluster = *check_cass_cluster(s, 1);
	size_t dcl;
	const char *local_dc = luaL_checklstring(s, 2, &dcl);
	unsigned int used_hosts_per_remote_dc = (unsigned int)luaL_checkinteger(s, 3);
	luaL_checktype(s, 4, LUA_TBOOLEAN);
	cass_bool_t allow_remote_dcs_for_local_cl = (cass_bool_t)lua_toboolean(s, 4);
	CassError err = cass_cluster_set_load_balance_dc_aware(cluster, local_dc,
							       used_hosts_per_remote_dc,
							       allow_remote_dcs_for_local_cl);
	lua_pushinteger(s, err);
	return 1;
}

static int
lc_cass_cluster_set_token_aware_routing(lua_State *s) {
	CassCluster *cluster = *check_cass_cluster(s, 1);
	luaL_checktype(s, 2, LUA_TBOOLEAN);
	cass_bool_t enabled = (cass_bool_t)lua_toboolean(s, 2);
	cass_cluster_set_token_aware_routing(cluster, enabled);
	return 0;
}

static int
lc_cass_cluster_set_latency_aware_routing(lua_State *s) {
	CassCluster *cluster = *check_cass_cluster(s, 1);
	luaL_checktype(s, 2, LUA_TBOOLEAN);
	cass_bool_t enabled = (cass_bool_t)lua_toboolean(s, 2);
	cass_cluster_set_latency_aware_routing(cluster, enabled);
	return 0;
}

static int
lc_cass_cluster_set_latency_aware_routing_settings(lua_State *s) {
	CassCluster *cluster = *check_cass_cluster(s, 1);
	cass_double_t exclusion_threshold = (cass_double_t)luaL_checknumber(s, 2);
	cass_uint64_t scale_ms = (cass_uint64_t)luaL_checkinteger(s, 3);
	cass_uint64_t retry_period_ms = (cass_uint64_t)luaL_checkinteger(s, 4);
	cass_uint64_t update_rate_ms = (cass_uint64_t)luaL_checkinteger(s, 5);
	cass_uint64_t min_measured = (cass_uint64_t)luaL_checkinteger(s, 6);
	cass_cluster_set_latency_aware_routing_settings(cluster, exclusion_threshold, scale_ms,
							retry_period_ms, update_rate_ms,
							min_measured);
	return 0;
}

static int
lc_cass_cluster_set_whitelist_filtering(lua_State *s) {
	CassCluster *cluster = *check_cass_cluster(s, 1);
	size_t len;
	const char *hosts = luaL_checklstring(s, 2, &len);
	cass_cluster_set_whitelist_filtering(cluster, hosts);
	return 0;
}

static int
lc_cass_cluster_set_blacklist_filtering(lua_State *s) {
	CassCluster *cluster = *check_cass_cluster(s, 1);
	size_t len;
	const char *hosts = luaL_checklstring(s, 2, &len);
	cass_cluster_set_blacklist_filtering(cluster, hosts);
	return 0;
}

static int
lc_cass_cluster_set_whitelist_dc_filtering(lua_State *s) {
	CassCluster *cluster = *check_cass_cluster(s, 1);
	size_t len;
	const char *dcs = luaL_checklstring(s, 2, &len);
	cass_cluster_set_whitelist_dc_filtering(cluster, dcs);
	return 0;
}

static int
lc_cass_cluster_set_blacklist_dc_filtering(lua_State *s) {
	CassCluster *cluster = *check_cass_cluster(s, 1);
	size_t len;
	const char *hosts = luaL_checklstring(s, 2, &len);
	cass_cluster_set_blacklist_dc_filtering(cluster, hosts);
	return 0;
}

static int
lc_cass_cluster_set_tcp_nodelay(lua_State *s) {
	CassCluster *cluster = *check_cass_cluster(s, 1);
	luaL_checktype(s, 2, LUA_TBOOLEAN);
	cass_bool_t enabled = (cass_bool_t)lua_toboolean(s, 2);
	cass_cluster_set_tcp_nodelay(cluster, enabled);
	return 0;
}

static int
lc_cass_cluster_set_tcp_keepalive(lua_State *s) {
	CassCluster *cluster = *check_cass_cluster(s, 1);
	luaL_checktype(s, 2, LUA_TBOOLEAN);
	cass_bool_t enabled = (cass_bool_t)lua_toboolean(s, 2);
	unsigned int delay_secs = (unsigned int)luaL_checkinteger(s, 3);
	cass_cluster_set_tcp_keepalive(cluster, enabled, delay_secs);
	return 0;
}

static int
lc_cass_cluster_set_timestamp_gen(lua_State *s) {
	CassCluster *cluster = *check_cass_cluster(s, 1);
	CassTimestampGen *timestamp_gen = *check_cass_timestamp_gen(s, 2);
	cass_cluster_set_timestamp_gen(cluster, timestamp_gen);
	return 0;
}

static int
lc_cass_cluster_set_connection_heartbeat_interval(lua_State *s) {
	CassCluster *cluster = *check_cass_cluster(s, 1);
	unsigned int interval_secs = (unsigned int)luaL_checkinteger(s, 2);
	cass_cluster_set_connection_heartbeat_interval(cluster, interval_secs);
	return 0;
}

static int
lc_cass_cluster_set_connection_idle_timeout(lua_State *s) {
	CassCluster *cluster = *check_cass_cluster(s, 1);
	unsigned int timeout_secs = (unsigned int)luaL_checkinteger(s, 2);
	cass_cluster_set_connection_idle_timeout(cluster, timeout_secs);
	return 0;
}

static int
lc_cass_cluster_set_retry_policy(lua_State *s) {
	CassCluster *cluster = *check_cass_cluster(s, 1);
	CassRetryPolicy *retry_policy = *check_cass_retry_policy(s, 2);
	cass_cluster_set_retry_policy(cluster, retry_policy);
	return 0;
}

static int
lc_cass_cluster_set_use_schema(lua_State *s) {
	CassCluster *cluster = *check_cass_cluster(s, 1);
	luaL_checktype(s, 2, LUA_TBOOLEAN);
	cass_bool_t enabled = (cass_bool_t)lua_toboolean(s, 2);
	cass_cluster_set_use_schema(cluster, enabled);
	return 0;
}

static int
lc_cass_session_new(lua_State *s) {
	CassSession *session = cass_session_new();
	if (NULL != session) {
		CassSession **impl = (CassSession **)lua_newuserdata(s, sizeof(CassSession *));
		luaL_getmetatable(s, "datastax.cass_session");
		lua_setmetatable(s, -2);
		*impl = session;
	} else
		lua_pushnil(s);
	return 1;
}

static int
lc_cass_session_free(lua_State *s) {
	CassSession **session = check_cass_session(s, 1);
	cass_session_free(*session);
	*session = NULL;
	return 0;
}

static int
lc_cass_session_connect(lua_State *s) {
	CassSession *session = *check_cass_session(s, 1);
	const CassCluster *cluster = *check_cass_cluster(s, 2);
	CassFuture *future = cass_session_connect(session, cluster);
	return new_cass_future(s, future);
}

static int
lc_cass_session_connect_keyspace(lua_State *s) {
	CassSession *session = *check_cass_session(s, 1);
	const CassCluster *cluster = *check_cass_cluster(s, 2);
	size_t len;
	const char *keyspace = luaL_checklstring(s, 3, &len);
	CassFuture *future = cass_session_connect_keyspace(session, cluster, keyspace);
	return new_cass_future(s, future);
}

static int
lc_cass_session_close(lua_State *s) {
	CassSession *session = *check_cass_session(s, 1);
	CassFuture *future = cass_session_close(session);
	return new_cass_future(s, future);
}

static int
lc_cass_session_prepare(lua_State *s) {
	CassSession *session = *check_cass_session(s, 1);
	size_t len;
	const char *query = luaL_checklstring(s, 2, &len);
	CassFuture *future = cass_session_prepare(session, query);
	return new_cass_future(s, future);
}

static int
lc_cass_session_execute(lua_State *s) {
	CassSession *session = *check_cass_session(s, 1);
	CassStatement *statement = *check_cass_statement(s, 2);
	CassFuture *future = cass_session_execute(session, statement);
	return new_cass_future(s, future);
}

static int
lc_cass_session_execute_batch(lua_State *s) {
	CassSession *session = *check_cass_session(s, 1);
	const CassBatch *batch = *check_cass_batch(s, 2);
	CassFuture *future = cass_session_execute_batch(session, batch);
	return new_cass_future(s, future);
}

static int
lc_cass_session_get_schema_meta(lua_State *s) {
	const CassSession *session = *check_cass_session(s, 1);
	const CassSchemaMeta *meta = cass_session_get_schema_meta(session);
	return new_cass_schema_meta(s, meta);
}

static int
lc_cass_session_get_metrics(lua_State *s) {
	const CassSession *session = *check_cass_session(s, 1);
	CassMetrics output;
	cass_session_get_metrics(session, &output);
	return new_cass_metrics(s, &output);
}

static int
lc_cass_schema_meta_free(lua_State *s) {
	CassSchemaMeta **schema_meta = check_cass_schema_meta(s, 1);
	cass_schema_meta_free(*schema_meta);
	*schema_meta = NULL;
	return 0;
}

static int
lc_cass_schema_meta_snapshot_version(lua_State *s) {
	const CassSchemaMeta *schema_meta = *check_cass_schema_meta(s, 1);
	cass_uint32_t version = cass_schema_meta_snapshot_version(schema_meta);
	lua_pushinteger(s, version);
	return 1;
}

static int
lc_cass_schema_meta_version(lua_State *s) {
	const CassSchemaMeta *schema_meta = *check_cass_schema_meta(s, 1);
	CassVersion version = cass_schema_meta_version(schema_meta);
	lua_pushinteger(s, version.major_version);
	lua_pushinteger(s, version.minor_version);
	lua_pushinteger(s, version.patch_version);
	return 3;
}

static int
lc_cass_schema_meta_keyspace_by_name(lua_State *s) {
	const CassSchemaMeta *schema_meta = *check_cass_schema_meta(s, 1);
	size_t len;
	const char *keyspace = luaL_checklstring(s, 2, &len);
	const CassKeyspaceMeta *meta = cass_schema_meta_keyspace_by_name(schema_meta, keyspace);
	return new_cass_keyspace_meta(s, meta);
}

static int
lc_cass_keyspace_meta_name(lua_State *s) {
	const CassKeyspaceMeta *keyspace_meta = *check_cass_keyspace_meta(s, 1);
	size_t len;
	const char *name;
	cass_keyspace_meta_name(keyspace_meta, &name, &len);
	lua_pushlstring(s, name, len);
	return 1;
}

static int
lc_cass_keyspace_meta_table_by_name(lua_State *s) {
	const CassKeyspaceMeta *keyspace_meta = *check_cass_keyspace_meta(s, 1);
	size_t len;
	const char *table = luaL_checklstring(s, 2, &len);
	const CassTableMeta *meta = cass_keyspace_meta_table_by_name(keyspace_meta, table);
	return new_cass_table_meta(s, meta);
}

static int
lc_cass_keyspace_meta_materialized_view_by_name(lua_State *s) {
	const CassKeyspaceMeta *keyspace_meta = *check_cass_keyspace_meta(s, 1);
	size_t len;
	const char *view = luaL_checklstring(s, 2, &len);
	const CassMaterializedViewMeta *materialised_view = cass_keyspace_meta_materialized_view_by_name(keyspace_meta, view);
	return new_cass_materialized_view_meta(s, materialised_view);
}

static int
lc_cass_keyspace_meta_user_type_by_name(lua_State *s) {
	const CassKeyspaceMeta *keyspace_meta = *check_cass_keyspace_meta(s, 1);
	size_t len;
	const char *type = luaL_checklstring(s, 2, &len);
	const CassDataType *data_type = cass_keyspace_meta_user_type_by_name(keyspace_meta, type);
	return new_cass_data_type(s, data_type);
}

static int
lc_cass_keyspace_meta_function_by_name(lua_State *s) {
	const CassKeyspaceMeta *keyspace_meta = *check_cass_keyspace_meta(s, 1);
	size_t nl, al;
	const char *name = luaL_checklstring(s, 2, &nl);
	const char *args = luaL_checklstring(s, 3, &al);
	const CassFunctionMeta *fnc = cass_keyspace_meta_function_by_name(keyspace_meta, name, args);
	return new_cass_function_meta(s, fnc);
}

static int
lc_cass_keyspace_meta_aggregate_by_name(lua_State *s) {
	const CassKeyspaceMeta *keyspace_meta = *check_cass_keyspace_meta(s, 1);
	size_t nl, al;
	const char *name = luaL_checklstring(s, 2, &nl);
	const char *args = luaL_checklstring(s, 3, &al);
	const CassAggregateMeta *fnc = cass_keyspace_meta_aggregate_by_name(keyspace_meta,
									    name, args);
	return new_cass_aggregate_meta(s, fnc);
}

static int
lc_cass_keyspace_meta_field_by_name(lua_State *s) {
	const CassKeyspaceMeta *keyspace_meta = *check_cass_keyspace_meta(s, 1);
	size_t nl;
	const char *name = luaL_checklstring(s, 2, &nl);
	const CassValue *value = cass_keyspace_meta_field_by_name(keyspace_meta, name);
	return new_cass_value(s, value);
}

static int
lc_cass_table_meta_name(lua_State *s) {
	const CassTableMeta *table_meta = *check_cass_table_meta(s, 1);
	size_t len;
	const char *name;
	cass_table_meta_name(table_meta, &name, &len);
	lua_pushlstring(s, name, len);
	return 1;
}

static int
lc_cass_table_meta_column_by_name(lua_State *s) {
	const CassTableMeta *table_meta = *check_cass_table_meta(s, 1);
	size_t len;
	const char *column = luaL_checklstring(s, 2, &len);
	const CassColumnMeta *column_meta = cass_table_meta_column_by_name(table_meta, column);
	return new_cass_column_meta(s, column_meta);
}

static int
lc_cass_table_meta_column_count(lua_State *s) {
	const CassTableMeta *table_meta = *check_cass_table_meta(s, 1);
	size_t count = cass_table_meta_column_count(table_meta);
	lua_pushinteger(s, count);
	return 1;
}

static int
lc_cass_table_meta_column(lua_State *s) {
	const CassTableMeta *table_meta = *check_cass_table_meta(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	const CassColumnMeta *column_meta = cass_table_meta_column(table_meta, index);
	return new_cass_column_meta(s, column_meta);
}

static int
lc_cass_table_meta_index_by_name(lua_State *s) {
	const CassTableMeta *table_meta = *check_cass_table_meta(s, 1);
	size_t len;
	const char *index = luaL_checklstring(s, 2, &len);
	const CassIndexMeta *meta = cass_table_meta_index_by_name(table_meta, index);
	return new_cass_index_meta(s, meta);
}

static int
lc_cass_table_meta_index_count(lua_State *s) {
	const CassTableMeta *table_meta = *check_cass_table_meta(s, 1);
	size_t n = cass_table_meta_index_count(table_meta);
	lua_pushinteger(s, n);
	return 1;
}

static int
lc_cass_table_meta_index(lua_State *s) {
	const CassTableMeta *table_meta = *check_cass_table_meta(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	const CassIndexMeta *meta = cass_table_meta_index(table_meta, index);
	return new_cass_index_meta(s, meta);
}

static int
lc_cass_table_meta_materialized_view_by_name(lua_State *s) {
	const CassTableMeta *table_meta = *check_cass_table_meta(s, 1);
	size_t len;
	const char *view = luaL_checklstring(s, 2, &len);
	const CassMaterializedViewMeta *meta = cass_table_meta_materialized_view_by_name(table_meta, view);
	return new_cass_materialized_view_meta(s, meta);
}

static int
lc_cass_table_meta_meterialized_view_count(lua_State *s) {
	const CassTableMeta *table_meta = *check_cass_table_meta(s, 1);
	size_t n = cass_table_meta_materialized_view_count(table_meta);
	lua_pushinteger(s, n);
	return 1;
}

static int
lc_cass_table_meta_meterialized_view(lua_State *s) {
	const CassTableMeta *table_meta = *check_cass_table_meta(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	const CassMaterializedViewMeta *meta = cass_table_meta_materialized_view(table_meta, index);
	return new_cass_materialized_view_meta(s, meta);
}

static int
lc_cass_table_meta_partition_key_count(lua_State *s) {
	const CassTableMeta *table_meta = *check_cass_table_meta(s, 1);
	size_t n = cass_table_meta_partition_key_count(table_meta);
	lua_pushinteger(s, n);
	return 1;
}

static int
lc_cass_table_meta_partition_key(lua_State *s) {
	const CassTableMeta *table_meta = *check_cass_table_meta(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	const CassColumnMeta *meta = cass_table_meta_partition_key(table_meta, index);
	return new_cass_column_meta(s, meta);
}

static int
lc_cass_table_meta_clustering_key_count(lua_State *s) {
	const CassTableMeta *table_meta = *check_cass_table_meta(s, 1);
	size_t n = cass_table_meta_clustering_key_count(table_meta);
	lua_pushinteger(s, n);
	return 1;
}

static int
lc_cass_table_meta_clustering_key(lua_State *s) {
	const CassTableMeta *table_meta = *check_cass_table_meta(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	const CassColumnMeta *meta = cass_table_meta_clustering_key(table_meta, index);
	return new_cass_column_meta(s, meta);
}

static int
lc_cass_table_meta_clustering_key_order(lua_State *s) {
	const CassTableMeta *table_meta = *check_cass_table_meta(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	CassClusteringOrder order = cass_table_meta_clustering_key_order(table_meta, index);
	lua_pushinteger(s, order);
	return 1;
}

static int
lc_cass_table_meta_field_by_name(lua_State *s) {
	const CassTableMeta *table_meta = *check_cass_table_meta(s, 1);
	size_t len;
	const char *name = luaL_checklstring(s, 2, &len);
	const CassValue *value = cass_table_meta_field_by_name(table_meta, name);
	return new_cass_value(s, value);
}

static int
lc_cass_materialized_view_meta_column_by_name(lua_State *s) {
	const CassMaterializedViewMeta *view_meta = *check_cass_materialized_view_meta(s, 1);
	size_t len;
	const char *column = luaL_checklstring(s, 2, &len);
	const CassColumnMeta *meta = cass_materialized_view_meta_column_by_name(view_meta, column);
	return new_cass_column_meta(s, meta);
}

static int
lc_cass_materialized_view_meta_name(lua_State *s) {
	const CassMaterializedViewMeta *view_meta = *check_cass_materialized_view_meta(s, 1);
	const char *name;
	size_t len;
	cass_materialized_view_meta_name(view_meta, &name, &len);
	lua_pushlstring(s, name, len);
	return 1;
}

static int
lc_cass_materialized_view_meta_base_table(lua_State *s) {
	const CassMaterializedViewMeta *view_meta = *check_cass_materialized_view_meta(s, 1);
	const CassTableMeta *meta = cass_materialized_view_meta_base_table(view_meta);
	return new_cass_table_meta(s, meta);
}

static int
lc_cass_materialized_view_meta_column_count(lua_State *s) {
	const CassMaterializedViewMeta *view_meta = *check_cass_materialized_view_meta(s, 1);
	size_t n = cass_materialized_view_meta_column_count(view_meta);
	lua_pushinteger(s, n);
	return 1;
}

static int
lc_cass_materialized_view_meta_column(lua_State *s) {
	const CassMaterializedViewMeta *view_meta = *check_cass_materialized_view_meta(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	const CassColumnMeta *meta = cass_materialized_view_meta_column(view_meta, index);
	return new_cass_column_meta(s, meta);
}

static int
lc_cass_materialized_view_meta_partition_key_count(lua_State *s) {
	const CassMaterializedViewMeta *view_meta = *check_cass_materialized_view_meta(s, 1);
	size_t n = cass_materialized_view_meta_partition_key_count(view_meta);
	lua_pushinteger(s, n);
	return 1;
}

static int
lc_cass_materialized_view_meta_partition_key(lua_State *s) {
	const CassMaterializedViewMeta *view_meta = *check_cass_materialized_view_meta(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	const CassColumnMeta *meta = cass_materialized_view_meta_partition_key(view_meta, index);
	return new_cass_column_meta(s, meta);
}


static int
lc_cass_materialized_view_meta_clustering_key_count(lua_State *s) {
	const CassMaterializedViewMeta *view_meta = *check_cass_materialized_view_meta(s, 1);
	size_t n = cass_materialized_view_meta_clustering_key_count(view_meta);
	lua_pushinteger(s, n);
	return 1;
}

static int
lc_cass_materialized_view_meta_clustering_key(lua_State *s) {
	const CassMaterializedViewMeta *view_meta = *check_cass_materialized_view_meta(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	const CassColumnMeta *meta = cass_materialized_view_meta_clustering_key(view_meta, index);
	return new_cass_column_meta(s, meta);
}

static int
lc_cass_materialized_view_meta_clustering_key_order(lua_State *s) {
	const CassMaterializedViewMeta *view_meta = *check_cass_materialized_view_meta(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	CassClusteringOrder order = cass_materialized_view_meta_clustering_key_order(view_meta, index);
	lua_pushinteger(s, order);
	return 1;
}

static int
lc_cass_materialized_view_meta_field_by_name(lua_State *s) {
	const CassMaterializedViewMeta *view_meta = *check_cass_materialized_view_meta(s, 1);
	size_t len;
	const char *name = luaL_checklstring(s, 2, &len);;
	const CassValue *value = cass_materialized_view_meta_field_by_name(view_meta, name);
	return new_cass_value(s, value);
}

static int
lc_cass_column_meta_name(lua_State *s) {
	const CassColumnMeta *col_meta = *check_cass_column_meta(s, 1);
	const char *name;
	size_t len;
	cass_column_meta_name(col_meta, &name, &len);
	lua_pushlstring(s, name, len);
	return 1;
}

static int
lc_cass_column_meta_type(lua_State *s) {
	const CassColumnMeta *col_meta = *check_cass_column_meta(s, 1);
	CassColumnType col_type = cass_column_meta_type(col_meta);
	lua_pushinteger(s, col_type);
	return 1;
}

static int
lc_cass_column_meta_data_type(lua_State *s) {
	const CassColumnMeta *col_meta = *check_cass_column_meta(s, 1);
	const CassDataType *data_type = cass_column_meta_data_type(col_meta);
	return new_cass_data_type(s, data_type);
}

static int
lc_cass_column_meta_field_by_name(lua_State *s) {
	const CassColumnMeta *col_meta = *check_cass_column_meta(s, 1);
	size_t len;
	const char *name = luaL_checklstring(s, 2, &len);
	const CassValue *value = cass_column_meta_field_by_name(col_meta, name);
	return new_cass_value(s, value);
}

static int
lc_cass_index_meta_name(lua_State *s) {
	const CassIndexMeta *index_meta = *check_cass_index_meta(s, 1);
	const char *name;
	size_t len;
	cass_index_meta_name(index_meta, &name, &len);
	lua_pushlstring(s, name, len);
	return 1;
}

static int
lc_cass_index_meta_type(lua_State *s) {
	const CassIndexMeta *index_meta = *check_cass_index_meta(s, 1);
	CassIndexType index_type = cass_index_meta_type(index_meta);
	lua_pushinteger(s, index_type);
	return 1;
}

static int
lc_cass_index_meta_target(lua_State *s) {
	const CassIndexMeta *index_meta = *check_cass_index_meta(s, 1);
	const char *target;
	size_t len;
	cass_index_meta_target(index_meta, &target, &len);
	lua_pushlstring(s, target, len);
	return 1;
}

static int
lc_cass_index_meta_options(lua_State *s) {
	const CassIndexMeta *index_meta = *check_cass_index_meta(s, 1);
	const CassValue *value = cass_index_meta_options(index_meta);
	return new_cass_value(s, value);
}

static int
lc_cass_index_meta_field_by_name(lua_State *s) {
	const CassIndexMeta *index_meta = *check_cass_index_meta(s, 1);
	size_t len;
	const char *name = luaL_checklstring(s, 2, &len);
	const CassValue *value = cass_index_meta_field_by_name(index_meta, name);
	return new_cass_value(s, value);
}

static int
lc_cass_function_meta_name(lua_State *s) {
	const CassFunctionMeta *func_meta = *check_cass_function_meta(s, 1);
	const char *name;
	size_t len;
	cass_function_meta_name(func_meta, &name, &len);
	lua_pushlstring(s, name, len);
	return 1;
}

static int
lc_cass_function_meta_full_name(lua_State *s) {
	const CassFunctionMeta *func_meta = *check_cass_function_meta(s, 1);
	const char *name;
	size_t len;
	cass_function_meta_full_name(func_meta, &name, &len);
	lua_pushlstring(s, name, len);
	return 1;
}

static int
lc_cass_function_meta_body(lua_State *s) {
	const CassFunctionMeta *func_meta = *check_cass_function_meta(s, 1);
	const char *name;
	size_t len;
	cass_function_meta_body(func_meta, &name, &len);
	lua_pushlstring(s, name, len);
	return 1;
}

static int
lc_cass_function_meta_language(lua_State *s) {
	const CassFunctionMeta *func_meta = *check_cass_function_meta(s, 1);
	const char *name;
	size_t len;
	cass_function_meta_language(func_meta, &name, &len);
	lua_pushlstring(s, name, len);
	return 1;
}

static int
lc_cass_function_meta_called_on_null_input(lua_State *s) {
	const CassFunctionMeta *func_meta = *check_cass_function_meta(s, 1);
	cass_bool_t b = cass_function_meta_called_on_null_input(func_meta);
	lua_pushboolean(s, b);
	return 1;
}

static int
lc_cass_function_meta_argument_count(lua_State *s) {
	const CassFunctionMeta *func_meta = *check_cass_function_meta(s, 1);
	size_t n = cass_function_meta_argument_count(func_meta);
	lua_pushinteger(s, n);
	return 1;
}

static int
lc_cass_function_meta_argument(lua_State *s) {
	const CassFunctionMeta *func_meta = *check_cass_function_meta(s, 1);
	size_t index = (size_t)luaL_checkinteger(s, 2);
	const char *name;
	size_t len;
	const CassDataType *type;
	CassError e = cass_function_meta_argument(func_meta, index, &name, &len, &type);
	lua_pushinteger(s, e);
	lua_pushlstring(s, name, len);
	return 2 + new_cass_data_type(s, type);
}

static int
lc_cass_function_meta_argument_type_by_name(lua_State *s) {
	const CassFunctionMeta *func_meta = *check_cass_function_meta(s, 1);
	size_t len;
	const char *name = luaL_checklstring(s, 2, &len);
	const CassDataType *type = cass_function_meta_argument_type_by_name(func_meta, name);
	return new_cass_data_type(s, type);
}

static int
lc_cass_function_meta_return_type(lua_State *s) {
	const CassFunctionMeta *func_meta = *check_cass_function_meta(s, 1);
	const CassDataType *type = cass_function_meta_return_type(func_meta);
	return new_cass_data_type(s, type);
}

static int
lc_cass_function_meta_field_by_name(lua_State *s) {
	const CassFunctionMeta *func_meta = *check_cass_function_meta(s, 1);
	size_t len;
	const char *name = luaL_checklstring(s, 2, &len);
	const CassValue *value = cass_function_meta_field_by_name(func_meta, name);
	return new_cass_value(s, value);
}

static int
lc_cass_aggregate_meta_name(lua_State *s) {
	const CassAggregateMeta *agg_meta = *check_cass_aggregate_meta(s, 1);
	const char *name;
	size_t len;
	cass_aggregate_meta_name(agg_meta, &name, &len);
	lua_pushlstring(s, name, len);
	return 1;
}

static int
lc_cass_aggregate_meta_full_name(lua_State *s) {
	const CassAggregateMeta *agg_meta = *check_cass_aggregate_meta(s, 1);
	const char *name;
	size_t len;
	cass_aggregate_meta_full_name(agg_meta, &name, &len);
	lua_pushlstring(s, name, len);
	return 1;
}

static int
lc_cass_aggregate_meta_argument_count(lua_State *s) {
	const CassAggregateMeta *agg_meta = *check_cass_aggregate_meta(s, 1);
	size_t n = cass_aggregate_meta_argument_count(agg_meta);
	lua_pushinteger(s, n);
	return 1;
}

static int
lc_cass_aggregate_meta_argument_type(lua_State *s) {
	const CassAggregateMeta *agg_meta = *check_cass_aggregate_meta(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	const CassDataType *type = cass_aggregate_meta_argument_type(agg_meta, index);
	return new_cass_data_type(s, type);
}

static int
lc_cass_aggregate_meta_return_type(lua_State *s) {
	const CassAggregateMeta *agg_meta = *check_cass_aggregate_meta(s, 1);
	const CassDataType *type = cass_aggregate_meta_return_type(agg_meta);
	return new_cass_data_type(s, type);
}

static int
lc_cass_aggregate_meta_state_type(lua_State *s) {
	const CassAggregateMeta *agg_meta = *check_cass_aggregate_meta(s, 1);
	const CassDataType *type = cass_aggregate_meta_state_type(agg_meta);
	return new_cass_data_type(s, type);
}

static int
lc_cass_aggregate_meta_state_func(lua_State *s) {
	const CassAggregateMeta *agg_meta = *check_cass_aggregate_meta(s, 1);
	const CassFunctionMeta *func_meta = cass_aggregate_meta_state_func(agg_meta);
	return new_cass_function_meta(s, func_meta);
}

static int
lc_cass_aggregate_meta_final_func(lua_State *s) {
	const CassAggregateMeta *agg_meta = *check_cass_aggregate_meta(s, 1);
	const CassFunctionMeta *func_meta = cass_aggregate_meta_final_func(agg_meta);
	return new_cass_function_meta(s, func_meta);
}

static int
lc_cass_aggregate_meta_init_cond(lua_State *s) {
	const CassAggregateMeta *agg_meta = *check_cass_aggregate_meta(s, 1);
	const CassValue *value = cass_aggregate_meta_init_cond(agg_meta);
	return new_cass_value(s, value);
}

static int
lc_cass_aggregate_meta_field_by_name(lua_State *s) {
	const CassAggregateMeta *agg_meta = *check_cass_aggregate_meta(s, 1);
	size_t len;
	const char *name = luaL_checklstring(s, 2, &len);
	const CassValue *value = cass_aggregate_meta_field_by_name(agg_meta, name);
	return new_cass_value(s, value);
}

static int
lc_cass_ssl_new(lua_State *s) {
	CassSsl *ssl = cass_ssl_new();
	if (NULL != ssl) {
		CassSsl **impl = (CassSsl **)lua_newuserdata(s, sizeof(CassSsl *));
		luaL_getmetatable(s, "datastax.cass_ssl");
		lua_setmetatable(s, -2);
		*impl = ssl;
	} else
		lua_pushnil(s);
	return 1;
}

static int
lc_cass_ssl_free(lua_State *s) {
	CassSsl **ssl = check_cass_ssl(s, 1);
	cass_ssl_free(*ssl);
	*ssl = NULL;
	return 0;
}

static int
lc_cass_ssl_add_trusted_cert(lua_State *s) {
	CassSsl *ssl = *check_cass_ssl(s, 1);
	size_t len;
	const char *cert = luaL_checklstring(s, 2, &len);
	CassError e = cass_ssl_add_trusted_cert(ssl, cert);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_ssl_set_verify_flags(lua_State *s) {
	CassSsl *ssl = *check_cass_ssl(s, 1);
	int flags = luaL_checkinteger(s, 2);
	cass_ssl_set_verify_flags(ssl, flags);
	return 0;
}

static int
lc_cass_ssl_set_cert(lua_State *s) {
	CassSsl *ssl = *check_cass_ssl(s, 1);
	size_t len;
	const char *cert = luaL_checklstring(s, 2, &len);
	CassError e = cass_ssl_set_cert(ssl, cert);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_ssl_set_private_key(lua_State *s) {
	CassSsl *ssl = *check_cass_ssl(s, 1);
	size_t klen, plen;
	const char *key = luaL_checklstring(s, 2, &klen);
	const char *password = luaL_checklstring(s, 3, &plen);
	CassError e = cass_ssl_set_private_key(ssl, key, password);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_future_free(lua_State *s) {
	struct __CassFuture *future = check_cass_future(s, 1);
	if (LUA_NOREF != future->ref) {
		luaL_unref(s, LUA_REGISTRYINDEX, future->ref);
		future->ref = LUA_NOREF;
	}
	cass_future_free(future->future);
	future->future = NULL;
	return 0;
}

static lua_State *cb_thread = NULL;
static int cb_thread_ndx = LUA_NOREF;

static void
future_callback(CassFuture *dummy, int ref) {
	pthread_mutex_lock(&cb_mtx);
	/* push the {cb, future, data} table on the stack */
	lua_rawgeti(cb_thread, LUA_REGISTRYINDEX, ref);
	/* push the cb on top of the stack */
	lua_pushstring(cb_thread, "cb");
	lua_gettable(cb_thread, -2);
	/* push the future on top of the stack */
	lua_pushstring(cb_thread, "future");
	lua_gettable(cb_thread, -3);
	/* push the opaque data on top of the stack */
	lua_pushstring(cb_thread, "data");
	lua_gettable(cb_thread, -4);
	/* remove the {cb, future, data} table from the stack */
	lua_remove(cb_thread, -4);

	if (0 != lua_pcall(cb_thread, 2, 0, 0)) {
		const char *err = lua_tostring(cb_thread, -1);
		(void)err;
		lua_pop(cb_thread, 1);
	}

	pthread_mutex_unlock(&cb_mtx);
}

static int
lc_cass_future_set_callback(lua_State *s) {
	struct __CassFuture *future = check_cass_future(s, 1);

	/* lock */
	if (NULL == cb_thread) {
		pthread_mutex_lock(&cb_mtx);
		if (NULL == cb_thread) {
			cb_thread = lua_newthread(s);
			cb_thread_ndx = luaL_ref(s, LUA_REGISTRYINDEX);
		}
		pthread_mutex_unlock(&cb_mtx);
	}
	lua_newtable(s);
	/* add the future to the table */
	lua_pushstring(s, "future");
	lua_pushvalue(s, 1);
	lua_settable(s, -3);
	/* add the callback to the table */
	lua_pushstring(s, "cb");
	lua_pushvalue(s, 2);
	lua_settable(s, -3);
	/* add the opaque data to the table */
	lua_pushstring(s, "data");
	lua_pushvalue(s, 3);
	lua_settable(s, -3);
	/* store {cb, data, future} in the registry */
	if (LUA_NOREF != future->ref)
		luaL_unref(s, LUA_REGISTRYINDEX, future->ref);
	future->ref = luaL_ref(s, LUA_REGISTRYINDEX);
	/* unlock */

	CassError e =
		cass_future_set_callback(future->future,
					 (CassFutureCallback)future_callback,
					 (void *)future->ref);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_future_ready(lua_State *s) {
	CassFuture *future = check_cass_future(s, 1)->future;
	cass_bool_t b = cass_future_ready(future);
	lua_pushboolean(s, b);
	return 1;
}

static int
lc_cass_future_wait(lua_State *s) {
	CassFuture *future = check_cass_future(s, 1)->future;
	cass_future_wait(future);
	return 0;
}

static int
lc_cass_future_wait_timed(lua_State *s) {
	CassFuture *future = check_cass_future(s, 1)->future;
	cass_duration_t timeout_us = (cass_duration_t)luaL_checkinteger(s, 2);
	cass_bool_t b = cass_future_wait_timed(future, timeout_us);
	lua_pushboolean(s, b);
	return 1;
}

static int
lc_cass_future_get_result(lua_State *s) {
	CassFuture *future = check_cass_future(s, 1)->future;
	const CassResult *rlt = cass_future_get_result(future);
	return new_cass_result(s, rlt);
}

static int
lc_cass_future_get_error_result(lua_State *s) {
	CassFuture *future = check_cass_future(s, 1)->future;
	const CassErrorResult *rlt = cass_future_get_error_result(future);
	return new_cass_error_result(s, rlt);
}

static int
lc_cass_future_get_prepared(lua_State *s) {
	CassFuture *future = check_cass_future(s, 1)->future;
	const CassPrepared *prepared = cass_future_get_prepared(future);
	return new_cass_prepared(s, prepared);
}

static int
lc_cass_future_error_code(lua_State *s) {
	CassFuture *future = check_cass_future(s, 1)->future;
	CassError e = cass_future_error_code(future);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_future_error_message(lua_State *s) {
	CassFuture *future = check_cass_future(s, 1)->future;
	const char *msg;
	size_t len;
	cass_future_error_message(future, &msg, &len);
	lua_pushlstring(s, msg, len);
	return 1;
}

static int
lc_cass_future_custom_payload_item_count(lua_State *s) {
	CassFuture *future = check_cass_future(s, 1)->future;
	size_t n = cass_future_custom_payload_item_count(future);
	lua_pushinteger(s, n);
	return 1;
}

static int
lc_cass_future_custom_payload_item(lua_State *s) {
	CassFuture *future = check_cass_future(s, 1)->future;
	size_t index = (size_t)luaL_checkinteger(s, 2);
	const char *name;
	size_t len, sz;
	const cass_byte_t *value;
	CassError e = cass_future_custom_payload_item(future, index, &name, &len, &value, &sz);
	lua_pushinteger(s, e);
	lua_pushlstring(s, name, len);
	lua_pushlstring(s, (const char *)value, sz);
	return 3;
}

static int
lc_cass_statement_new(lua_State *s) {
	size_t len;
	const char *query = luaL_checklstring(s, 1, &len);
	size_t pc = (size_t)luaL_checkinteger(s, 2);
	CassStatement *stmt = cass_statement_new(query, pc);
	return new_cass_statement(s, stmt);
}

static int
lc_cass_statement_free(lua_State *s) {
	CassStatement **stmt = check_cass_statement(s, 1);
	cass_statement_free(*stmt);
	*stmt = NULL;
	return 0;
}

static int
lc_cass_statement_add_key_index(lua_State *s) {
	CassStatement *stmt = *check_cass_statement(s, 1);
	size_t index = (size_t)luaL_checkinteger(s, 2);
	CassError e = cass_statement_add_key_index(stmt, index);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_statement_set_keyspace(lua_State *s) {
	CassStatement *stmt = *check_cass_statement(s, 1);
	size_t len;
	const char *name = luaL_checklstring(s, 2, &len);
	CassError e = cass_statement_set_keyspace(stmt, name);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_statement_set_consistency(lua_State *s) {
	CassStatement *stmt = *check_cass_statement(s, 1);
	CassConsistency c = (CassConsistency)luaL_checkinteger(s, 2);
	CassError e = cass_statement_set_consistency(stmt, c);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_statement_set_serial_consistency(lua_State *s) {
	CassStatement *stmt = *check_cass_statement(s, 1);
	CassConsistency c = (CassConsistency)luaL_checkinteger(s, 2);
	CassError e = cass_statement_set_serial_consistency(stmt, c);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_statement_set_paging_size(lua_State *s) {
	CassStatement *stmt = *check_cass_statement(s, 1);
	int page_size = (int)luaL_checkinteger(s, 2);
	CassError e = cass_statement_set_paging_size(stmt, page_size);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_statement_set_paging_state(lua_State *s) {
	CassStatement *stmt = *check_cass_statement(s, 1);
	const CassResult *rlt = *check_cass_result(s, 2);
	CassError e = cass_statement_set_paging_state(stmt, rlt);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_statement_set_paging_state_token(lua_State *s) {
	CassStatement *stmt = *check_cass_statement(s, 1);
	size_t len;
	const char *paging_state = luaL_checklstring(s, 2, &len);
	size_t n = (size_t)luaL_checkinteger(s, 3);
	CassError e = cass_statement_set_paging_state_token(stmt, paging_state, n);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_statement_set_timestamp(lua_State *s) {
	CassStatement *stmt = *check_cass_statement(s, 1);
	cass_int64_t timestamp = (cass_int64_t)luaL_checkinteger(s, 2);
	CassError e = cass_statement_set_timestamp(stmt, timestamp);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_statement_set_retry_policy(lua_State *s) {
	CassStatement *stmt = *check_cass_statement(s, 1);
	CassRetryPolicy *pol = *check_cass_retry_policy(s, 2);
	CassError e = cass_statement_set_retry_policy(stmt, pol);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_statement_set_custom_payload(lua_State *s) {
	CassStatement *stmt = *check_cass_statement(s, 1);
	const CassCustomPayload *payload = *check_cass_custom_payload(s, 2);
	CassError e = cass_statement_set_custom_payload(stmt, payload);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_statement_bind_null(lua_State *s) {
	CassStatement *stmt = *check_cass_statement(s, 1);
	size_t index = (size_t)luaL_checkinteger(s, 2);
	CassError e = cass_statement_bind_null(stmt, index);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_statement_bind_null_by_name(lua_State *s) {
	CassStatement *stmt = *check_cass_statement(s, 1);
	size_t len;
	const char *name = luaL_checklstring(s, 2, &len);
	CassError e = cass_statement_bind_null_by_name(stmt, name);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_statement_bind_int8(lua_State *s) {
	CassStatement *stmt = *check_cass_statement(s, 1);
	size_t index = (size_t)luaL_checkinteger(s, 2);
	cass_int8_t v = (cass_int8_t)luaL_checkinteger(s, 3);
	CassError e = cass_statement_bind_int8(stmt, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_statement_bind_int8_by_name(lua_State *s) {
	CassStatement *stmt = *check_cass_statement(s, 1);
	size_t len;
	const char *name = luaL_checklstring(s, 2, &len);
	cass_int8_t v = (cass_int8_t)luaL_checkinteger(s, 3);
	CassError e = cass_statement_bind_int8_by_name(stmt, name, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_statement_bind_int16(lua_State *s) {
	CassStatement *stmt = *check_cass_statement(s, 1);
	size_t index = (size_t)luaL_checkinteger(s, 2);
	cass_int16_t v = (cass_int16_t)luaL_checkinteger(s, 3);
	CassError e = cass_statement_bind_int16(stmt, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_statement_bind_int16_by_name(lua_State *s) {
	CassStatement *stmt = *check_cass_statement(s, 1);
	size_t len;
	const char *name = luaL_checklstring(s, 2, &len);
	cass_int16_t v = (cass_int16_t)luaL_checkinteger(s, 3);
	CassError e = cass_statement_bind_int16_by_name(stmt, name, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_statement_bind_int32(lua_State *s) {
	CassStatement *stmt = *check_cass_statement(s, 1);
	size_t index = (size_t)luaL_checkinteger(s, 2);
	cass_int32_t v = (cass_int32_t)luaL_checkinteger(s, 3);
	CassError e = cass_statement_bind_int32(stmt, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_statement_bind_int32_by_name(lua_State *s) {
	CassStatement *stmt = *check_cass_statement(s, 1);
	size_t len;
	const char *name = luaL_checklstring(s, 2, &len);
	cass_int32_t v = (cass_int32_t)luaL_checkinteger(s, 3);
	CassError e = cass_statement_bind_int32_by_name(stmt, name, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_statement_bind_uint32(lua_State *s) {
	CassStatement *stmt = *check_cass_statement(s, 1);
	size_t index = (size_t)luaL_checkinteger(s, 2);
	cass_uint32_t v = (cass_uint32_t)luaL_checkinteger(s, 3);
	CassError e = cass_statement_bind_uint32(stmt, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_statement_bind_uint32_by_name(lua_State *s) {
	CassStatement *stmt = *check_cass_statement(s, 1);
	size_t len;
	const char *name = luaL_checklstring(s, 2, &len);
	cass_uint32_t v = (cass_uint32_t)luaL_checkinteger(s, 3);
	CassError e = cass_statement_bind_uint32_by_name(stmt, name, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_statement_bind_int64(lua_State *s) {
	CassStatement *stmt = *check_cass_statement(s, 1);
	size_t index = (size_t)luaL_checkinteger(s, 2);
	cass_int64_t v = (cass_int64_t)luaL_checkinteger(s, 3);
	CassError e = cass_statement_bind_int64(stmt, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_statement_bind_int64_by_name(lua_State *s) {
	CassStatement *stmt = *check_cass_statement(s, 1);
	size_t len;
	const char *name = luaL_checklstring(s, 2, &len);
	cass_int64_t v = (cass_int64_t)luaL_checkinteger(s, 3);
	CassError e = cass_statement_bind_int64_by_name(stmt, name, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_statement_bind_float(lua_State *s) {
	CassStatement *stmt = *check_cass_statement(s, 1);
	size_t index = (size_t)luaL_checkinteger(s, 2);
	cass_float_t v = (cass_float_t)luaL_checknumber(s, 3);
	CassError e = cass_statement_bind_float(stmt, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_statement_bind_float_by_name(lua_State *s) {
	CassStatement *stmt = *check_cass_statement(s, 1);
	size_t len;
	const char *name = luaL_checklstring(s, 2, &len);
	cass_float_t v = (cass_float_t)luaL_checknumber(s, 3);
	CassError e = cass_statement_bind_float_by_name(stmt, name, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_statement_bind_double(lua_State *s) {
	CassStatement *stmt = *check_cass_statement(s, 1);
	size_t index = (size_t)luaL_checkinteger(s, 2);
	cass_double_t v = (cass_double_t)luaL_checknumber(s, 3);
	CassError e = cass_statement_bind_double(stmt, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_statement_bind_double_by_name(lua_State *s) {
	CassStatement *stmt = *check_cass_statement(s, 1);
	size_t len;
	const char *name = luaL_checklstring(s, 2, &len);
	cass_double_t v = (cass_double_t)luaL_checknumber(s, 3);
	CassError e = cass_statement_bind_double_by_name(stmt, name, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_statement_bind_bool(lua_State *s) {
	CassStatement *stmt = *check_cass_statement(s, 1);
	size_t index = (size_t)luaL_checkinteger(s, 2);
	luaL_checktype(s, 3, LUA_TBOOLEAN);
	cass_bool_t v = (cass_bool_t)lua_toboolean(s, 3);
	CassError e = cass_statement_bind_bool(stmt, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_statement_bind_bool_by_name(lua_State *s) {
	CassStatement *stmt = *check_cass_statement(s, 1);
	size_t len;
	const char *name = luaL_checklstring(s, 2, &len);
	luaL_checktype(s, 3, LUA_TBOOLEAN);
	cass_bool_t v = (cass_bool_t)lua_toboolean(s, 3);
	CassError e = cass_statement_bind_bool_by_name(stmt, name, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_statement_bind_string(lua_State *s) {
	CassStatement *stmt = *check_cass_statement(s, 1);
	size_t index = (size_t)luaL_checkinteger(s, 2);
	size_t vl;
	const char *v = luaL_checklstring(s, 3, &vl);
	CassError e = cass_statement_bind_string(stmt, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_statement_bind_string_by_name(lua_State *s) {
	CassStatement *stmt = *check_cass_statement(s, 1);
	size_t len, vl;
	const char *name = luaL_checklstring(s, 2, &len);
	const char *v = luaL_checklstring(s, 3, &vl);
	CassError e = cass_statement_bind_string_by_name(stmt, name, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_statement_bind_bytes(lua_State *s) {
	CassStatement *stmt = *check_cass_statement(s, 1);
	size_t index = (size_t)luaL_checkinteger(s, 2);
	size_t n;
	const cass_byte_t *v = (const cass_byte_t *)luaL_checklstring(s, 3, &n);
	CassError e = cass_statement_bind_bytes(stmt, index, v, n);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_statement_bind_bytes_by_name(lua_State *s) {
	CassStatement *stmt = *check_cass_statement(s, 1);
	size_t len;
	const char *name = luaL_checklstring(s, 2, &len);
	size_t n;
	const cass_byte_t *v = (const cass_byte_t *)luaL_checklstring(s, 3, &n);
	CassError e = cass_statement_bind_bytes_by_name(stmt, name, v, n);
	lua_pushinteger(s, e);
	return 1;
}

static CassUuid
to_cass_uuid(lua_State *s, int index) {
	CassUuid v;
	if (lua_istable(s, index)) {
		lua_pushstring(s, "time_and_version");
		lua_gettable(s, index);
		v.time_and_version = (cass_uint64_t)luaL_checkinteger(s, -1);
		lua_pop(s, 1);
		
		lua_pushstring(s, "clock_seq_and_node");
		lua_gettable(s, index);
		v.clock_seq_and_node = (cass_uint64_t)luaL_checkinteger(s, -1);
		lua_pop(s, 1);
	} else {
		uuid_t uid;
		size_t len;
		const char *uuid = luaL_checklstring(s, index, &len);
		if (-1 == uuid_parse(uuid, uid)) {
			memset(uid, 0, sizeof(uid));
			strncpy((char *)uid, uuid, sizeof(uid));
		}
		memcpy(&v.time_and_version, uid,
		       sizeof(v.time_and_version));
		memcpy(&v.clock_seq_and_node, uid + sizeof(v.time_and_version),
		       sizeof(v.clock_seq_and_node));
		*((uint32_t *)&v.time_and_version) = ntohl(*((uint32_t *)&v.time_and_version));
		*((uint16_t *)&v.time_and_version + 2) = ntohs(*((uint16_t *)&v.time_and_version + 2));
		*((uint16_t *)&v.time_and_version + 3) = ntohs(*((uint16_t *)&v.time_and_version + 3));
#if __BYTE_ORDER == __LITTLE_ENDIAN
		v.clock_seq_and_node = bswap_64(v.clock_seq_and_node);
#endif
	}
	return v;
}

static void
from_cass_uuid(CassUuid v, unsigned char uid[]) {
	*((uint32_t *)&v.time_and_version) = htonl(*((uint32_t *)&v.time_and_version));
	*((uint16_t *)&v.time_and_version + 2) = htons(*((uint16_t *)&v.time_and_version + 2));
	*((uint16_t *)&v.time_and_version + 3) = htons(*((uint16_t *)&v.time_and_version + 3));
#if __BYTE_ORDER == __LITTLE_ENDIAN
	v.clock_seq_and_node = bswap_64(v.clock_seq_and_node);
#endif
	memcpy(uid, &v.time_and_version, 8);
	memcpy(uid + 8, &v.clock_seq_and_node, 8);
}

static CassInet
to_cass_inet(lua_State *s, int index) {
	CassInet inet;
	if (lua_istable(s, index)) {
		size_t len;
		const char *addr;
		lua_pushstring(s, "address_length");
		lua_gettable(s, index);
		inet.address_length = (cass_uint8_t)luaL_checkinteger(s, -1);
		lua_pop(s, 1);
		
		lua_pushstring(s, "address");
		lua_gettable(s, index);
		addr = luaL_checklstring(s, -1, &len);
		memset(&inet.address, 0, CASS_INET_V6_LENGTH);
		memcpy(&inet.address, addr, len > CASS_INET_V6_LENGTH ? CASS_INET_V6_LENGTH : len);
		lua_pop(s, 1);
		return inet;
	}
	if (lua_isstring(s, index)) {
		size_t len;
		const char *addr_str = luaL_checklstring(s, index, &len);
		int ipv6 = NULL != strchr(addr_str, ':');
		switch (inet_pton(ipv6 ? AF_INET6 : AF_INET, addr_str, inet.address)) {
		case 1:
			inet.address_length = ipv6 ? sizeof(struct in6_addr) : sizeof(struct in_addr);
			break;
		case 0:
			/* TODO */
			break;
		case -1:
			/* TODO */
			break;
		default:
			/* TODO */
			break;
		}
		return inet;
	}
	/* TODO */
	return inet;
}

static void
from_cass_inet(CassInet inet, char addr[], size_t len) {
	if (sizeof(struct in_addr) == inet.address_length) {
		if (NULL != inet_ntop(AF_INET, inet.address, addr, len))
			return;
		/* TODO */
	}
	if (sizeof(struct in6_addr) == inet.address_length) {
		if (NULL != inet_ntop(AF_INET6, inet.address, addr, len))
			return;
		/* TODO */
	}
	/* TODO */
}

static int
lc_cass_statement_bind_uuid(lua_State *s) {
	CassStatement *stmt = *check_cass_statement(s, 1);
	size_t index = (size_t)luaL_checkinteger(s, 2);
	CassUuid v = to_cass_uuid(s, 3);
	CassError e = cass_statement_bind_uuid(stmt, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_statement_bind_uuid_by_name(lua_State *s) {
	CassStatement *stmt = *check_cass_statement(s, 1);
	size_t len;
	const char *name = luaL_checklstring(s, 2, &len);
	CassUuid v = to_cass_uuid(s, 3);
	CassError e = cass_statement_bind_uuid_by_name(stmt, name, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_statement_bind_inet(lua_State *s) {
	CassStatement *stmt = *check_cass_statement(s, 1);
	size_t index = (size_t)luaL_checkinteger(s, 2);
	CassInet v = to_cass_inet(s, 3);
	CassError e = cass_statement_bind_inet(stmt, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_statement_bind_inet_by_name(lua_State *s) {
	CassStatement *stmt = *check_cass_statement(s, 1);
	size_t len;
	const char *name = luaL_checklstring(s, 2, &len);
	CassInet v = to_cass_inet(s, 3);
	CassError e = cass_statement_bind_inet_by_name(stmt, name, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_statement_bind_decimal(lua_State *s) {
	CassStatement *stmt = *check_cass_statement(s, 1);
	size_t index = (size_t)luaL_checkinteger(s, 2);
	size_t n;
	const cass_byte_t *v = (const cass_byte_t *)luaL_checklstring(s, 3, &n);
	cass_int32_t scale = (cass_int32_t)luaL_checkinteger(s, 4);
	CassError e = cass_statement_bind_decimal(stmt, index, v, n, scale);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_statement_bind_decimal_by_name(lua_State *s) {
	CassStatement *stmt = *check_cass_statement(s, 1);
	size_t len;
	const char *name = luaL_checklstring(s, 2, &len);
	size_t n;
	const cass_byte_t *v = (const cass_byte_t *)luaL_checklstring(s, 3, &n);
	cass_int32_t scale = (cass_int32_t)luaL_checkinteger(s, 4);
	CassError e = cass_statement_bind_decimal_by_name(stmt, name, v, n, scale);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_statement_bind_collection(lua_State *s) {
	CassStatement *stmt = *check_cass_statement(s, 1);
	size_t index = (size_t)luaL_checkinteger(s, 2);
	const CassCollection *v = *check_cass_collection(s, 3);
	CassError e = cass_statement_bind_collection(stmt, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_statement_bind_collection_by_name(lua_State *s) {
	CassStatement *stmt = *check_cass_statement(s, 1);
	size_t len;
	const char *name = luaL_checklstring(s, 2, &len);
	const CassCollection *v = *check_cass_collection(s, 3);
	CassError e = cass_statement_bind_collection_by_name(stmt, name, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_statement_bind_tuple(lua_State *s) {
	CassStatement *stmt = *check_cass_statement(s, 1);
	size_t index = (size_t)luaL_checkinteger(s, 2);
	const CassTuple *v = *check_cass_tuple(s, 3);
	CassError e = cass_statement_bind_tuple(stmt, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_statement_bind_tuple_by_name(lua_State *s) {
	CassStatement *stmt = *check_cass_statement(s, 1);
	size_t len;
	const char *name = luaL_checklstring(s, 2, &len);
	const CassTuple *v = *check_cass_tuple(s, 3);
	CassError e = cass_statement_bind_tuple_by_name(stmt, name, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_statement_bind_user_type(lua_State *s) {
	CassStatement *stmt = *check_cass_statement(s, 1);
	size_t index = (size_t)luaL_checkinteger(s, 2);
	const CassUserType *v = *check_cass_user_type(s, 3);
	CassError e = cass_statement_bind_user_type(stmt, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_statement_bind_user_type_by_name(lua_State *s) {
	CassStatement *stmt = *check_cass_statement(s, 1);
	size_t len;
	const char *name = luaL_checklstring(s, 2, &len);
	const CassUserType *v = *check_cass_user_type(s, 3);
	CassError e = cass_statement_bind_user_type_by_name(stmt, name, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_prepared_free(lua_State *s) {
	CassPrepared **prepared = check_cass_prepared(s, 1);
	if (NULL != *prepared) {
		cass_prepared_free(*prepared);
		*prepared = NULL;
	}
	return 0;
}

static int
lc_cass_prepared_bind(lua_State *s) {
	const CassPrepared *prepared = *check_cass_prepared(s, 1);
	CassStatement *stmt = cass_prepared_bind(prepared);
	return new_cass_statement(s, stmt);
}

static int
lc_cass_prepared_parameter_name(lua_State *s) {
	const CassPrepared *prepared = *check_cass_prepared(s, 1);
	size_t index = (size_t)luaL_checkinteger(s, 2);
	const char *name;
	size_t len;
	CassError e = cass_prepared_parameter_name(prepared, index, &name, &len);
	lua_pushinteger(s, e);
	lua_pushlstring(s, name, len);
	return 2;
}

static int
lc_cass_prepared_parameter_data_type(lua_State *s) {
	const CassPrepared *prepared = *check_cass_prepared(s, 1);
	size_t index = (size_t)luaL_checkinteger(s, 2);
	const CassDataType *type = cass_prepared_parameter_data_type(prepared, index);
	return new_cass_data_type(s, type);
}

static int
lc_cass_prepared_parameter_data_type_by_name(lua_State *s) {
	const CassPrepared *prepared = *check_cass_prepared(s, 1);
	size_t len;
	const char *name = luaL_checklstring(s, 2, &len);
	const CassDataType *type = cass_prepared_parameter_data_type_by_name(prepared, name);
	return new_cass_data_type(s, type);
}

static int
lc_cass_batch_new(lua_State *s) {
	CassBatchType bt = (CassBatchType)luaL_checkinteger(s, 1);
	const CassBatch *batch = cass_batch_new(bt);
	return new_cass_batch(s, batch);
}

static int
lc_cass_batch_free(lua_State *s) {
	CassBatch **batch = check_cass_batch(s, 1);
	cass_batch_free(*batch);
	*batch = NULL;
	return 0;
}

static int
lc_cass_batch_set_consistency(lua_State *s) {
	CassBatch *batch = *check_cass_batch(s, 1);
	CassConsistency c = (CassConsistency)luaL_checkinteger(s, 2);
	CassError e = cass_batch_set_consistency(batch, c);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_batch_set_serial_consistency(lua_State *s) {
	CassBatch *batch = *check_cass_batch(s, 1);
	CassConsistency c = (CassConsistency)luaL_checkinteger(s, 2);
	CassError e = cass_batch_set_serial_consistency(batch, c);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_batch_set_timestamp(lua_State *s) {
	CassBatch *batch = *check_cass_batch(s, 1);
	cass_int64_t timestamp = (cass_int64_t)luaL_checkinteger(s, 2);
	CassError e = cass_batch_set_timestamp(batch, timestamp);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_batch_set_retry_policy(lua_State *s) {
	CassBatch *batch = *check_cass_batch(s, 1);
	CassRetryPolicy *pol = *check_cass_retry_policy(s, 2);
	CassError e = cass_batch_set_retry_policy(batch, pol);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_batch_set_custom_payload(lua_State *s) {
	CassBatch *batch = *check_cass_batch(s, 1);
	const CassCustomPayload *payload = *check_cass_custom_payload(s, 2);
	CassError e = cass_batch_set_custom_payload(batch, payload);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_batch_add_statement(lua_State *s) {
	CassBatch *batch = *check_cass_batch(s, 1);
	CassStatement *stmt = *check_cass_statement(s, 2);
	CassError e = cass_batch_add_statement(batch, stmt);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_data_type_new(lua_State *s) {
	CassValueType type = (CassValueType)luaL_checkinteger(s, 1);
	CassDataType *d = cass_data_type_new(type);
	return new_cass_data_type(s, d);
}

static int
lc_cass_data_type_new_from_existing(lua_State *s) {
	const CassDataType *type = *check_cass_data_type(s, 1);
	CassDataType *d = cass_data_type_new_from_existing(type);
	return new_cass_data_type(s, d);
}

static int
lc_cass_data_type_new_tuple(lua_State *s) {
	size_t count = (size_t)luaL_checkinteger(s, 1);
	CassDataType *d = cass_data_type_new_tuple(count);
	return new_cass_data_type(s, d);
}

static int
lc_cass_data_type_new_udt(lua_State *s) {
	size_t count = (size_t)luaL_checkinteger(s, 1);
	CassDataType *d = cass_data_type_new_udt(count);
	return new_cass_data_type(s, d);
}

static int
lc_cass_data_type_free(lua_State *s) {
	CassDataType **type = check_cass_data_type(s, 1);
	cass_data_type_free(*type);
	*type = NULL;
	return 0;
}

static int
lc_cass_data_type_type(lua_State *s) {
	const CassDataType *type = *check_cass_data_type(s, 1);	
	CassValueType v = cass_data_type_type(type);
	lua_pushinteger(s, v);
	return 1;
}

static int
lc_cass_data_type_type_name(lua_State *s) {
	const CassDataType *type = *check_cass_data_type(s, 1);
	const char *name;
	size_t len;
	CassError e = cass_data_type_type_name(type, &name, &len);
	lua_pushinteger(s, e);
	lua_pushlstring(s, name, len);
	return 2;
}

static int
lc_cass_data_type_set_type_name(lua_State *s) {
	CassDataType *type = *check_cass_data_type(s, 1);
	size_t len;
	const char *name = luaL_checklstring(s, 2, &len);
	CassError e = cass_data_type_set_type_name(type, name);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_data_type_keyspace(lua_State *s) {
	const CassDataType *type = *check_cass_data_type(s, 1);
	const char *name;
	size_t len;
	CassError e = cass_data_type_keyspace(type, &name, &len);
	lua_pushinteger(s, e);
	lua_pushlstring(s, name, len);
	return 2;
}

static int
lc_cass_data_type_set_keyspace(lua_State *s) {
	CassDataType *type = *check_cass_data_type(s, 1);
	size_t len;
	const char *name = luaL_checklstring(s, 2, &len);
	CassError e = cass_data_type_set_keyspace(type, name);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_data_type_class_name(lua_State *s) {
	CassDataType *type = *check_cass_data_type(s, 1);
	const char *name;
	size_t len;
	CassError e = cass_data_type_class_name(type, &name, &len);
	lua_pushinteger(s, e);
	lua_pushlstring(s, name, len);
	return 2;
}

static int
lc_cass_data_type_set_class_name(lua_State *s) {
	CassDataType *type = *check_cass_data_type(s, 1);
	size_t len;
	const char *name = luaL_checklstring(s, 2, &len);
	CassError e = cass_data_type_set_class_name(type, name);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_data_type_sub_type_count(lua_State *s) {
	const CassDataType *type = *check_cass_data_type(s, 1);
	size_t n = cass_data_type_sub_type_count(type);
	lua_pushinteger(s, n);
	return 1;
}

/* static int */
/* lc_cass_data_sub_type_count(lua_State *s) { */
/*	const CassDataType *type = *check_cass_data_type(s, 1); */
/* 	size_t n = cass_data_sub_type_count(type); */
/* 	lua_pushinteger(s, n); */
/* 	return 1; */
/* } */

static int
lc_cass_data_type_sub_data_type(lua_State *s) {
	const CassDataType *type = *check_cass_data_type(s, 1);
	size_t index = (size_t)luaL_checkinteger(s, 2);
	const CassDataType *data = cass_data_type_sub_data_type(type, index);
	return new_cass_data_type(s, data);
}

static int
lc_cass_data_type_sub_data_type_by_name(lua_State *s) {
	const CassDataType *type = *check_cass_data_type(s, 1);
	size_t len;
	const char *name = luaL_checklstring(s, 2, &len);
	const CassDataType *data = cass_data_type_sub_data_type_by_name(type, name);
	return new_cass_data_type(s, data);
}

static int
lc_cass_data_type_sub_type_name(lua_State *s) {
	const CassDataType *type = *check_cass_data_type(s, 1);
	size_t index = (size_t)luaL_checkinteger(s, 2);
	size_t len;
	const char *name;
	CassError e = cass_data_type_sub_type_name(type, index, &name, &len);
	lua_pushinteger(s, e);
	lua_pushlstring(s, name, len);
	return 2;
}

static int
lc_cass_data_type_add_sub_type(lua_State *s) {
	CassDataType *type = *check_cass_data_type(s, 1);
	const CassDataType *sub_type = *check_cass_data_type(s, 2);
	CassError e = cass_data_type_add_sub_type(type, sub_type);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_data_type_add_sub_type_by_name(lua_State *s) {
	CassDataType *type = *check_cass_data_type(s, 1);
	size_t len;
	const char *name = luaL_checklstring(s, 2, &len);
	const CassDataType *sub_type = *check_cass_data_type(s, 3);
	CassError e = cass_data_type_add_sub_type_by_name(type, name, sub_type);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_data_type_add_sub_value_type(lua_State *s) {
	CassDataType *type = *check_cass_data_type(s, 1);
	CassValueType sub_value_type = (CassValueType)luaL_checkinteger(s, 2);
	CassError e = cass_data_type_add_sub_value_type(type, sub_value_type);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_data_type_add_sub_value_type_by_name(lua_State *s) {
	CassDataType *type = *check_cass_data_type(s, 1);
	size_t len;
	const char *name = luaL_checklstring(s, 2, &len);
	CassValueType sub_value_type = (CassValueType)luaL_checkinteger(s, 3);
	CassError e = cass_data_type_add_sub_value_type_by_name(type, name, sub_value_type);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_collection_new(lua_State *s) {
	CassCollectionType cct = luaL_checkinteger(s, 1);
	size_t count = luaL_checkinteger(s, 2);
	CassCollection *c = cass_collection_new(cct, count);
	return new_cass_collection(s, c);
}

static int
lc_cass_collection_new_from_data_type(lua_State *s) {
	const CassDataType *type = *check_cass_data_type(s, 1);
	size_t count = luaL_checkinteger(s, 2);
	CassCollection *c = cass_collection_new_from_data_type(type, count);
	return new_cass_collection(s, c);
}

static int
lc_cass_collection_free(lua_State *s) {
	CassCollection **c = check_cass_collection(s, 1);
	cass_collection_free(*c);
	*c = NULL;
	return 0;
}

static int
lc_cass_collection_data_type(lua_State *s) {
	const CassCollection *c = *check_cass_collection(s, 1);
	const CassDataType *type = cass_collection_data_type(c);
	return new_cass_data_type(s, type);
}

static int
lc_cass_collection_append_int8(lua_State *s) {
	CassCollection *c = *check_cass_collection(s, 1);
	cass_int8_t v = luaL_checkinteger(s, 2);
	CassError e = cass_collection_append_int8(c, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_collection_append_int16(lua_State *s) {
	CassCollection *c = *check_cass_collection(s, 1);
	cass_int16_t v = luaL_checkinteger(s, 2);
	CassError e = cass_collection_append_int16(c, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_collection_append_int32(lua_State *s) {
	CassCollection *c = *check_cass_collection(s, 1);
	cass_int32_t v = luaL_checkinteger(s, 2);
	CassError e = cass_collection_append_int32(c, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_collection_append_uint32(lua_State *s) {
	CassCollection *c = *check_cass_collection(s, 1);
	cass_uint32_t v = luaL_checkinteger(s, 2);
	CassError e = cass_collection_append_uint32(c, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_collection_append_int64(lua_State *s) {
	CassCollection *c = *check_cass_collection(s, 1);
	cass_int64_t v = luaL_checkinteger(s, 2);
	CassError e = cass_collection_append_int64(c, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_collection_append_float(lua_State *s) {
	CassCollection *c = *check_cass_collection(s, 1);
	cass_float_t v = luaL_checknumber(s, 2);
	CassError e = cass_collection_append_float(c, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_collection_append_double(lua_State *s) {
	CassCollection *c = *check_cass_collection(s, 1);
	cass_double_t v = luaL_checknumber(s, 2);
	CassError e = cass_collection_append_double(c, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_collection_append_bool(lua_State *s) {
	CassCollection *c = *check_cass_collection(s, 1);
	luaL_checktype(s, 2, LUA_TBOOLEAN);
	cass_bool_t v = lua_toboolean(s, 2);
	CassError e = cass_collection_append_bool(c, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_collection_append_string(lua_State *s) {
	CassCollection *c = *check_cass_collection(s, 1);
	size_t len;
	const char *v = luaL_checklstring(s, 2, &len);
	CassError e = cass_collection_append_string(c, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_collection_append_bytes(lua_State *s) {
	CassCollection *c = *check_cass_collection(s, 1);
	size_t len;
	const cass_byte_t *v = (const cass_byte_t *)luaL_checklstring(s, 2, &len);
	CassError e = cass_collection_append_bytes(c, v, len);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_collection_append_uuid(lua_State *s) {
	CassCollection *c = *check_cass_collection(s, 1);
	CassUuid v = to_cass_uuid(s, 2);
	CassError e = cass_collection_append_uuid(c, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_collection_append_inet(lua_State *s) {
	CassCollection *c = *check_cass_collection(s, 1);
	CassInet v = to_cass_inet(s, 2);;
	CassError e = cass_collection_append_inet(c, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_collection_append_decimal(lua_State *s) {
	CassCollection *c = *check_cass_collection(s, 1);
	size_t vsz;
	const cass_byte_t *v = (const cass_byte_t *)luaL_checklstring(s, 2, &vsz);
	cass_int32_t scale = luaL_checkinteger(s, 3);
	CassError e = cass_collection_append_decimal(c, v, vsz, scale);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_collection_append_collection(lua_State *s) {
	CassCollection *c = *check_cass_collection(s, 1);
	const CassCollection *v = *check_cass_collection(s, 2);
	CassError e = cass_collection_append_collection(c, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_collection_append_tuple(lua_State *s) {
	CassCollection *c = *check_cass_collection(s, 1);
	const CassTuple *v = *check_cass_tuple(s, 2);
	CassError e = cass_collection_append_tuple(c, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_collection_append_user_type(lua_State *s) {
	CassCollection *c = *check_cass_collection(s, 1);
	const CassUserType *v = *check_cass_user_type(s, 2);
	CassError e = cass_collection_append_user_type(c, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_tuple_new(lua_State *s) {
	size_t count = luaL_checkinteger(s, 1);
	CassTuple *tuple = cass_tuple_new(count);
	return new_cass_tuple(s, tuple);
}

static int
lc_cass_tuple_new_from_data_type(lua_State *s) {
	const CassDataType *type = *check_cass_data_type(s, 1);
	CassTuple *tuple = cass_tuple_new_from_data_type(type);
	return new_cass_tuple(s, tuple);
}

static int
lc_cass_tuple_free(lua_State *s) {
	CassTuple **tuple = check_cass_tuple(s, 1);
	cass_tuple_free(*tuple);
	*tuple = NULL;
	return 0;
}

static int
lc_cass_tuple_data_type(lua_State *s) {
	const CassTuple *tuple = *check_cass_tuple(s, 1);
	const CassDataType *type = cass_tuple_data_type(tuple);
	return new_cass_data_type(s, type);
}

static int
lc_cass_tuple_set_null(lua_State *s) {
	CassTuple *tuple = *check_cass_tuple(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	CassError e = cass_tuple_set_null(tuple, index);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_tuple_set_int8(lua_State *s) {
	CassTuple *tuple = *check_cass_tuple(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	cass_int8_t v = luaL_checkinteger(s, 3);
	CassError e = cass_tuple_set_int8(tuple, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_tuple_set_int16(lua_State *s) {
	CassTuple *tuple = *check_cass_tuple(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	cass_int16_t v = luaL_checkinteger(s, 3);
	CassError e = cass_tuple_set_int16(tuple, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_tuple_set_int32(lua_State *s) {
	CassTuple *tuple = *check_cass_tuple(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	cass_int32_t v = luaL_checkinteger(s, 3);
	CassError e = cass_tuple_set_int32(tuple, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_tuple_set_uint32(lua_State *s) {
	CassTuple *tuple = *check_cass_tuple(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	cass_uint32_t v = luaL_checkinteger(s, 3);
	CassError e = cass_tuple_set_uint32(tuple, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_tuple_set_int64(lua_State *s) {
	CassTuple *tuple = *check_cass_tuple(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	cass_int64_t v = luaL_checkinteger(s, 3);
	CassError e = cass_tuple_set_int64(tuple, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_tuple_set_float(lua_State *s) {
	CassTuple *tuple = *check_cass_tuple(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	cass_float_t v = luaL_checknumber(s, 3);
	CassError e = cass_tuple_set_float(tuple, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_tuple_set_double(lua_State *s) {
	CassTuple *tuple = *check_cass_tuple(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	cass_double_t v = luaL_checknumber(s, 3);
	CassError e = cass_tuple_set_double(tuple, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_tuple_set_bool(lua_State *s) {
	CassTuple *tuple = *check_cass_tuple(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	luaL_checktype(s, 3, LUA_TBOOLEAN);
	cass_bool_t v = lua_toboolean(s, 3);
	CassError e = cass_tuple_set_bool(tuple, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_tuple_set_string(lua_State *s) {
	CassTuple *tuple = *check_cass_tuple(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	size_t len;
	const char *v = luaL_checklstring(s, 3, &len);
	CassError e = cass_tuple_set_string(tuple, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_tuple_set_bytes(lua_State *s) {
	CassTuple *tuple = *check_cass_tuple(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	size_t n;
	const cass_byte_t *v = (const cass_byte_t *)luaL_checklstring(s, 2, &n);
	CassError e = cass_tuple_set_bytes(tuple, index, v, n);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_tuple_set_uuid(lua_State *s) {
	CassTuple *tuple = *check_cass_tuple(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	CassUuid v = to_cass_uuid(s, 3);
	CassError e = cass_tuple_set_uuid(tuple, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_tuple_set_inet(lua_State *s) {
	CassTuple *tuple = *check_cass_tuple(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	CassInet v = to_cass_inet(s, 3);
	CassError e = cass_tuple_set_inet(tuple, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_tuple_set_decimal(lua_State *s) {
	CassTuple *tuple = *check_cass_tuple(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	size_t vsz;
	const cass_byte_t *v = (const cass_byte_t *)luaL_checklstring(s, 3, &vsz);
	cass_int32_t scale = luaL_checkinteger(s, 4);
	CassError e = cass_tuple_set_decimal(tuple, index, v, vsz, scale);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_tuple_set_collection(lua_State *s) {
	CassTuple *tuple = *check_cass_tuple(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	const CassCollection *v = *check_cass_collection(s, 3);
	CassError e = cass_tuple_set_collection(tuple, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_tuple_set_tuple(lua_State *s) {
	CassTuple *tuple = *check_cass_tuple(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	const CassTuple *v = *check_cass_tuple(s, 3);
	CassError e = cass_tuple_set_tuple(tuple, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_tuple_set_user_type(lua_State *s) {
	CassTuple *tuple = *check_cass_tuple(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	const CassUserType *v = *check_cass_user_type(s, 3);
	CassError e = cass_tuple_set_user_type(tuple, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_user_type_new_from_data_type(lua_State *s) {
	const CassDataType *type = *check_cass_data_type(s, 1);
	CassUserType *ut = cass_user_type_new_from_data_type(type);
	return new_cass_user_type(s, ut);
}

static int
lc_cass_user_type_free(lua_State *s) {
	CassUserType **type = check_cass_user_type(s, 1);
	cass_user_type_free(*type);
	*type = NULL;
	return 0;
}

static int
lc_cass_user_type_data_type(lua_State *s) {
	const CassUserType *ut = *check_cass_user_type(s, 1);
	const CassDataType *dt = cass_user_type_data_type(ut);
	return new_cass_data_type(s, dt);
}

static int
lc_cass_user_type_set_null(lua_State *s) {
	CassUserType *ut = *check_cass_user_type(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	CassError e = cass_user_type_set_null(ut, index);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_user_type_set_null_by_name(lua_State *s) {
	CassUserType *ut = *check_cass_user_type(s, 1);
	size_t len;
	const char *index = luaL_checklstring(s, 2, &len);
	CassError e = cass_user_type_set_null_by_name(ut, index);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_user_type_set_int8(lua_State *s) {
	CassUserType *ut = *check_cass_user_type(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	cass_int8_t v = luaL_checkinteger(s, 3);
	CassError e = cass_user_type_set_int8(ut, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_user_type_set_int8_by_name(lua_State *s) {
	CassUserType *ut = *check_cass_user_type(s, 1);
	size_t len;
	const char *index = luaL_checklstring(s, 2, &len);
	cass_int8_t v = luaL_checkinteger(s, 3);
	CassError e = cass_user_type_set_int8_by_name(ut, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_user_type_set_int16(lua_State *s) {
	CassUserType *ut = *check_cass_user_type(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	cass_int16_t v = luaL_checkinteger(s, 3);
	CassError e = cass_user_type_set_int16(ut, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_user_type_set_int16_by_name(lua_State *s) {
	CassUserType *ut = *check_cass_user_type(s, 1);
	size_t len;
	const char *index = luaL_checklstring(s, 2, &len);
	cass_int16_t v = luaL_checkinteger(s, 3);
	CassError e = cass_user_type_set_int16_by_name(ut, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_user_type_set_int32(lua_State *s) {
	CassUserType *ut = *check_cass_user_type(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	cass_int32_t v = luaL_checkinteger(s, 3);
	CassError e = cass_user_type_set_int32(ut, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_user_type_set_int32_by_name(lua_State *s) {
	CassUserType *ut = *check_cass_user_type(s, 1);
	size_t len;
	const char *index = luaL_checklstring(s, 2, &len);
	cass_int32_t v = luaL_checkinteger(s, 3);
	CassError e = cass_user_type_set_int32_by_name(ut, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_user_type_set_uint32(lua_State *s) {
	CassUserType *ut = *check_cass_user_type(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	cass_uint32_t v = luaL_checkinteger(s, 3);
	CassError e = cass_user_type_set_uint32(ut, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_user_type_set_uint32_by_name(lua_State *s) {
	CassUserType *ut = *check_cass_user_type(s, 1);
	size_t len;
	const char *index = luaL_checklstring(s, 2, &len);
	cass_uint32_t v = luaL_checkinteger(s, 3);
	CassError e = cass_user_type_set_uint32_by_name(ut, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_user_type_set_int64(lua_State *s) {
	CassUserType *ut = *check_cass_user_type(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	cass_int64_t v = luaL_checkinteger(s, 3);
	CassError e = cass_user_type_set_int64(ut, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_user_type_set_int64_by_name(lua_State *s) {
	CassUserType *ut = *check_cass_user_type(s, 1);
	size_t len;
	const char *index = luaL_checklstring(s, 2, &len);
	cass_int64_t v = luaL_checkinteger(s, 3);
	CassError e = cass_user_type_set_int64_by_name(ut, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_user_type_set_float(lua_State *s) {
	CassUserType *ut = *check_cass_user_type(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	cass_float_t v = luaL_checknumber(s, 3);
	CassError e = cass_user_type_set_float(ut, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_user_type_set_float_by_name(lua_State *s) {
	CassUserType *ut = *check_cass_user_type(s, 1);
	size_t len;
	const char *index = luaL_checklstring(s, 2, &len);
	cass_float_t v = luaL_checknumber(s, 3);
	CassError e = cass_user_type_set_float_by_name(ut, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_user_type_set_double(lua_State *s) {
	CassUserType *ut = *check_cass_user_type(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	cass_double_t v = luaL_checknumber(s, 3);
	CassError e = cass_user_type_set_double(ut, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_user_type_set_double_by_name(lua_State *s) {
	CassUserType *ut = *check_cass_user_type(s, 1);
	size_t len;
	const char *index = luaL_checklstring(s, 2, &len);
	cass_double_t v = luaL_checknumber(s, 3);
	CassError e = cass_user_type_set_double_by_name(ut, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_user_type_set_bool(lua_State *s) {
	CassUserType *ut = *check_cass_user_type(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	luaL_checktype(s, 3, LUA_TBOOLEAN);
	cass_bool_t v = lua_toboolean(s, 3);
	CassError e = cass_user_type_set_bool(ut, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_user_type_set_bool_by_name(lua_State *s) {
	CassUserType *ut = *check_cass_user_type(s, 1);
	size_t len;
	const char *index = luaL_checklstring(s, 2, &len);
	luaL_checktype(s, 3, LUA_TBOOLEAN);
	cass_bool_t v = lua_toboolean(s, 3);
	CassError e = cass_user_type_set_bool_by_name(ut, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_user_type_set_string(lua_State *s) {
	CassUserType *ut = *check_cass_user_type(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	size_t len;
	const char *v = luaL_checklstring(s, 3, &len);
	CassError e = cass_user_type_set_string(ut, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_user_type_set_string_by_name(lua_State *s) {
	CassUserType *ut = *check_cass_user_type(s, 1);
	size_t len, vlen;
	const char *index = luaL_checklstring(s, 2, &len);
	const char *v = luaL_checklstring(s, 3, &vlen);
	CassError e = cass_user_type_set_string_by_name(ut, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_user_type_set_bytes(lua_State *s) {
	CassUserType *ut = *check_cass_user_type(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	size_t sz;
	const cass_byte_t *v = (const cass_byte_t *)luaL_checklstring(s, 3, &sz);
	CassError e = cass_user_type_set_bytes(ut, index, v, sz);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_user_type_set_bytes_by_name(lua_State *s) {
	CassUserType *ut = *check_cass_user_type(s, 1);
	size_t len;
	const char *index = luaL_checklstring(s, 2, &len);
	size_t sz;
	const cass_byte_t *v = (const cass_byte_t *)luaL_checklstring(s, 3, &sz);
	CassError e = cass_user_type_set_bytes_by_name(ut, index, v, sz);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_user_type_set_uuid(lua_State *s) {
	CassUserType *ut = *check_cass_user_type(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	CassUuid v = to_cass_uuid(s, 3);
	CassError e = cass_user_type_set_uuid(ut, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_user_type_set_uuid_by_name(lua_State *s) {
	CassUserType *ut = *check_cass_user_type(s, 1);
	size_t len;
	const char *index = luaL_checklstring(s, 2, &len);
	CassUuid v = to_cass_uuid(s, 3);
	CassError e = cass_user_type_set_uuid_by_name(ut, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_user_type_set_inet(lua_State *s) {
	CassUserType *ut = *check_cass_user_type(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	CassInet v = to_cass_inet(s, 3);
	CassError e = cass_user_type_set_inet(ut, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_user_type_set_inet_by_name(lua_State *s) {
	CassUserType *ut = *check_cass_user_type(s, 1);
	size_t len;
	const char *index = luaL_checklstring(s, 2, &len);
	CassInet v = to_cass_inet(s, 3);
	CassError e = cass_user_type_set_inet_by_name(ut, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_user_type_set_decimal(lua_State *s) {
	CassUserType *ut = *check_cass_user_type(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	size_t vsz;
	const cass_byte_t *v = (const cass_byte_t *)luaL_checklstring(s, 3, &vsz);
	cass_int32_t scale = luaL_checkinteger(s, 4);
	CassError e = cass_user_type_set_decimal(ut, index, v, vsz, scale);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_user_type_set_decimal_by_name(lua_State *s) {
	CassUserType *ut = *check_cass_user_type(s, 1);
	size_t len;
	const char *index = luaL_checklstring(s, 2, &len);
	size_t vsz;
	const cass_byte_t *v = (const cass_byte_t *)luaL_checklstring(s, 3, &vsz);
	cass_int32_t scale = luaL_checkinteger(s, 4);
	CassError e = cass_user_type_set_decimal_by_name(ut, index, v, vsz, scale);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_user_type_set_collection(lua_State *s) {
	CassUserType *ut = *check_cass_user_type(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	const CassCollection *v = *check_cass_collection(s, 3);
	CassError e = cass_user_type_set_collection(ut, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_user_type_set_collection_by_name(lua_State *s) {
	CassUserType *ut = *check_cass_user_type(s, 1);
	size_t len;
	const char *index = luaL_checklstring(s, 2, &len);
	const CassCollection *v = *check_cass_collection(s, 3);
	CassError e = cass_user_type_set_collection_by_name(ut, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_user_type_set_tuple(lua_State *s) {
	CassUserType *ut = *check_cass_user_type(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	const CassTuple *v = *check_cass_tuple(s, 3);
	CassError e = cass_user_type_set_tuple(ut, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_user_type_set_tuple_by_name(lua_State *s) {
	CassUserType *ut = *check_cass_user_type(s, 1);
	size_t len;
	const char *index = luaL_checklstring(s, 2, &len);
	const CassTuple *v = *check_cass_tuple(s, 3);
	CassError e = cass_user_type_set_tuple_by_name(ut, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_user_type_set_user_type(lua_State *s) {
	CassUserType *ut = *check_cass_user_type(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	const CassUserType *v = *check_cass_user_type(s, 3);
	CassError e = cass_user_type_set_user_type(ut, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_user_type_set_user_type_by_name(lua_State *s) {
	CassUserType *ut = *check_cass_user_type(s, 1);
	size_t len;
	const char *index = luaL_checklstring(s, 2, &len);
	const CassUserType *v = *check_cass_user_type(s, 3);
	CassError e = cass_user_type_set_user_type_by_name(ut, index, v);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_result_free(lua_State *s) {
	CassResult **rlt = check_cass_result(s, 1);
	cass_result_free(*rlt);
	*rlt = NULL;
	return 0;
}

static int
lc_cass_result_row_count(lua_State *s) {
	const CassResult *rlt = *check_cass_result(s, 1);
	size_t n = cass_result_row_count(rlt);
	lua_pushinteger(s, n);
	return 1;
}

static int
lc_cass_result_column_count(lua_State *s) {
	const CassResult *rlt = *check_cass_result(s, 1);
	size_t n = cass_result_column_count(rlt);
	lua_pushinteger(s, n);
	return 1;
}

static int
lc_cass_result_column_name(lua_State *s) {
	const CassResult *rlt = *check_cass_result(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	const char *name;
	size_t len;
	CassError e = cass_result_column_name(rlt, index, &name, &len);
	lua_pushinteger(s, e);
	lua_pushlstring(s, name, len);
	return 2;
}

static int
lc_cass_result_column_type(lua_State *s) {
	const CassResult *rlt = *check_cass_result(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	CassValueType t = cass_result_column_type(rlt, index);
	lua_pushinteger(s, t);
	return 1;
}

static int
lc_cass_result_column_data_type(lua_State *s) {
	const CassResult *rlt = *check_cass_result(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	const CassDataType *type = cass_result_column_data_type(rlt, index);
	return new_cass_data_type(s, type);
}

static int
lc_cass_result_first_row(lua_State *s) {
	const CassResult *rlt = *check_cass_result(s, 1);
	const CassRow *row = cass_result_first_row(rlt);
	return new_cass_row(s, row);
}

static int
lc_cass_result_has_more_pages(lua_State *s) {
	const CassResult *rlt = *check_cass_result(s, 1);
	cass_bool_t b = cass_result_has_more_pages(rlt);
	lua_pushboolean(s, b);
	return 1;
}

static int
lc_cass_result_paging_state_token(lua_State *s) {
	const CassResult *rlt = *check_cass_result(s, 1);
	size_t len;
	const char *state;
	CassError e = cass_result_paging_state_token(rlt, &state, &len);
	lua_pushinteger(s, e);
	lua_pushlstring(s, state, len);
	return 2;
}

static int
lc_cass_error_result_free(lua_State *s) {
	const CassErrorResult **err = 
		(const CassErrorResult **)lua_touserdata(s, 1);
	cass_error_result_free(*err);

	*err = NULL;
	return 0;
}

static int
lc_cass_error_result_code(lua_State *s) {
	const CassErrorResult *err = *check_cass_error_result(s, 1);
	CassError e = cass_error_result_code(err);
	lua_pushinteger(s, e);
	return 1;
}

static int
lc_cass_error_result_consistency(lua_State *s) {
	const CassErrorResult *err = *check_cass_error_result(s, 1);
	CassConsistency c = cass_error_result_consistency(err);
	lua_pushinteger(s, c);
	return 1;
}

/* static int */
/* lc_cass_error_result_responses_received(lua_State *s) { */
/*	const CassErrorResult *err = *check_cass_error_result(s, 1); */
/* 	cass_int32_t r = cass_error_result_responses_received(err); */
/* 	lua_pushinteger(s, r); */
/* 	return 1; */
/* } */

/* static int */
/* lc_cass_error_result_responses_required(lua_State *s) { */
/*	const CassErrorResult *err = *check_cass_error_result(s, 1); */
/* 	cass_int32_t r = cass_error_result_responses_required(err); */
/* 	lua_pushinteger(s, r); */
/* 	return 1; */
/* } */

static int
lc_cass_error_result_num_failures(lua_State *s) {
	const CassErrorResult *err = *check_cass_error_result(s, 1);
	cass_int32_t r = cass_error_result_num_failures(err);
	lua_pushinteger(s, r);
	return 1;
}

static int
lc_cass_error_result_data_present(lua_State *s) {
	const CassErrorResult *err = *check_cass_error_result(s, 1);
	cass_bool_t r = cass_error_result_data_present(err);
	lua_pushboolean(s, r);
	return 1;
}

static int
lc_cass_error_result_write_type(lua_State *s) {
	const CassErrorResult *err = *check_cass_error_result(s, 1);
	CassWriteType wt = cass_error_result_write_type(err);
	lua_pushinteger(s, wt);
	return 1;
}

static int
lc_cass_error_result_keyspace(lua_State *s) {
	const CassErrorResult *err = *check_cass_error_result(s, 1);
	size_t len;
	const char *keyspace;
	CassError e = cass_error_result_keyspace(err, &keyspace, &len);
	lua_pushinteger(s, e);
	lua_pushlstring(s, keyspace, len);
	return 2;
}

static int
lc_cass_error_result_table(lua_State *s) {
	const CassErrorResult *err = *check_cass_error_result(s, 1);
	size_t len;
	const char *table;
	CassError e = cass_error_result_table(err, &table, &len);
	lua_pushinteger(s, e);
	lua_pushlstring(s, table, len);
	return 2;
}

static int
lc_cass_error_result_function(lua_State *s) {
	const CassErrorResult *err = *check_cass_error_result(s, 1);
	size_t len;
	const char *func;
	CassError e = cass_error_result_function(err, &func, &len);
	lua_pushinteger(s, e);
	lua_pushlstring(s, func, len);
	return 2;
}

static int
lc_cass_error_num_arg_types(lua_State *s) {
	const CassErrorResult *err = *check_cass_error_result(s, 1);
	size_t n = cass_error_num_arg_types(err);
	lua_pushinteger(s, n);
	return 1;
}

static int
lc_cass_error_result_arg_type(lua_State *s) {
	const CassErrorResult *err = *check_cass_error_result(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	size_t len;
	const char *arg_type;
	CassError e = cass_error_result_arg_type(err, index, &arg_type, &len);
	lua_pushinteger(s, e);
	lua_pushlstring(s, arg_type, len);
	return 2;
}

static int
lc_cass_iterator_free(lua_State *s) {
	CassIterator **it = check_cass_iterator(s, 1);
	cass_iterator_free(*it);
	*it = NULL;
	return 0;
}

static int
lc_cass_iterator_type(lua_State *s) {
	CassIterator *it = *check_cass_iterator(s, 1);
	CassIteratorType t = cass_iterator_type(it);
	lua_pushinteger(s, t);
	return 1;
}

static int
lc_cass_iterator_from_result(lua_State *s) {
	const CassResult *rlt = *check_cass_result(s, 1);
	CassIterator *it = cass_iterator_from_result(rlt);
	return new_cass_iterator(s, it);
}

static int
lc_cass_iterator_from_row(lua_State *s) {
	const CassRow *row = *check_cass_row(s, 1);
	CassIterator *it = cass_iterator_from_row(row);
	return new_cass_iterator(s, it);
}

static int
lc_cass_iterator_from_collection(lua_State *s) {
	const CassValue *c = *check_cass_value(s, 1);
	CassIterator *it = cass_iterator_from_collection(c);
	return new_cass_iterator(s, it);
}

static int
lc_cass_iterator_from_map(lua_State *s) {
	const CassValue *c = *check_cass_value(s, 1);
	CassIterator *it = cass_iterator_from_map(c);
	return new_cass_iterator(s, it);
}

static int
lc_cass_iterator_from_tuple(lua_State *s) {
	const CassValue *c = *check_cass_value(s, 1);
	CassIterator *it = cass_iterator_from_tuple(c);
	return new_cass_iterator(s, it);}

static int
lc_cass_iterator_fields_from_user_type(lua_State *s) {
	const CassValue *c = *check_cass_value(s, 1);
	CassIterator *it = cass_iterator_fields_from_user_type(c);
	return new_cass_iterator(s, it);}

static int
lc_cass_iterator_keyspaces_from_schema_meta(lua_State *s) {
	const CassSchemaMeta *c = *check_cass_schema_meta(s, 1);
	CassIterator *it = cass_iterator_keyspaces_from_schema_meta(c);
	return new_cass_iterator(s, it);}

static int
lc_cass_iterator_tables_from_keyspace_meta(lua_State *s) {
	const CassKeyspaceMeta *c = *check_cass_keyspace_meta(s, 1);
	CassIterator *it = cass_iterator_tables_from_keyspace_meta(c);
	return new_cass_iterator(s, it);}

static int
lc_cass_iterator_materialized_views_from_keyspace_meta(lua_State *s) {
	const CassKeyspaceMeta *c = *check_cass_keyspace_meta(s, 1);
	CassIterator *it = cass_iterator_materialized_views_from_keyspace_meta(c);
	return new_cass_iterator(s, it);}

static int
lc_cass_iterator_user_types_from_keyspace_meta(lua_State *s) {
	const CassKeyspaceMeta *c = *check_cass_keyspace_meta(s, 1);
	CassIterator *it = cass_iterator_user_types_from_keyspace_meta(c);
	return new_cass_iterator(s, it);}

static int
lc_cass_iterator_functions_from_keyspace_meta(lua_State *s) {
	const CassKeyspaceMeta *c = *check_cass_keyspace_meta(s, 1);
	CassIterator *it = cass_iterator_functions_from_keyspace_meta(c);
	return new_cass_iterator(s, it);}

static int
lc_cass_iterator_aggregates_from_keyspace_meta(lua_State *s) {
	const CassKeyspaceMeta *c = *check_cass_keyspace_meta(s, 1);
	CassIterator *it = cass_iterator_aggregates_from_keyspace_meta(c);
	return new_cass_iterator(s, it);}

static int
lc_cass_iterator_fields_from_keyspace_meta(lua_State *s) {
	const CassKeyspaceMeta *c = *check_cass_keyspace_meta(s, 1);
	CassIterator *it = cass_iterator_fields_from_keyspace_meta(c);
	return new_cass_iterator(s, it);}

static int
lc_cass_iterator_columns_from_table_meta(lua_State *s) {
	const CassTableMeta *c = *check_cass_table_meta(s, 1);
	CassIterator *it = cass_iterator_columns_from_table_meta(c);
	return new_cass_iterator(s, it);}

static int
lc_cass_iterator_indexes_from_table_meta(lua_State *s) {
	const CassTableMeta *c = *check_cass_table_meta(s, 1);
	CassIterator *it = cass_iterator_indexes_from_table_meta(c);
	return new_cass_iterator(s, it);}

static int
lc_cass_iterator_materialized_views_from_table_meta(lua_State *s) {
	const CassTableMeta *c = *check_cass_table_meta(s, 1);
	CassIterator *it = cass_iterator_materialized_views_from_table_meta(c);
	return new_cass_iterator(s, it);}

static int
lc_cass_iterator_fields_from_table_meta(lua_State *s) {
	const CassTableMeta *c = *check_cass_table_meta(s, 1);
	CassIterator *it = cass_iterator_fields_from_table_meta(c);
	return new_cass_iterator(s, it);}

static int
lc_cass_iterator_columns_from_materialized_view_meta(lua_State *s) {
	const CassMaterializedViewMeta *c = *check_cass_materialized_view_meta(s, 1);
	CassIterator *it = cass_iterator_columns_from_materialized_view_meta(c);
	return new_cass_iterator(s, it);}

static int
lc_cass_iterator_fields_from_materialized_view_meta(lua_State *s) {
	const CassMaterializedViewMeta *c = *check_cass_materialized_view_meta(s, 1);
	CassIterator *it = cass_iterator_fields_from_materialized_view_meta(c);
	return new_cass_iterator(s, it);}

static int
lc_cass_iterator_fields_from_column_meta(lua_State *s) {
	const CassColumnMeta *c = *check_cass_column_meta(s, 1);
	CassIterator *it = cass_iterator_fields_from_column_meta(c);
	return new_cass_iterator(s, it);}

static int
lc_cass_iterator_fields_from_index_meta(lua_State *s) {
	const CassIndexMeta *c = *check_cass_index_meta(s, 1);
	CassIterator *it = cass_iterator_fields_from_index_meta(c);
	return new_cass_iterator(s, it);}

static int
lc_cass_iterator_fields_from_function_meta(lua_State *s) {
	const CassFunctionMeta *c = *check_cass_function_meta(s, 1);
	CassIterator *it = cass_iterator_fields_from_function_meta(c);
	return new_cass_iterator(s, it);}

static int
lc_cass_iterator_fields_from_aggregate_meta(lua_State *s) {
	const CassAggregateMeta *c = *check_cass_aggregate_meta(s, 1);
	CassIterator *it = cass_iterator_fields_from_aggregate_meta(c);
	return new_cass_iterator(s, it);}

static int
lc_cass_iterator_next(lua_State *s) {
	CassIterator *it = *check_cass_iterator(s, 1);
	cass_bool_t b = cass_iterator_next(it);
	lua_pushboolean(s, b);
	return 1;
}

static int
lc_cass_iterator_get_row(lua_State *s) {
	const CassIterator *it = *check_cass_iterator(s, 1);
	const CassRow *row = cass_iterator_get_row(it);
	return new_cass_row(s, row);
}

static int
lc_cass_iterator_get_column(lua_State *s) {
	const CassIterator *it = *check_cass_iterator(s, 1);
	const CassValue *col = cass_iterator_get_column(it);
	return new_cass_value(s, col);
}

static int
lc_cass_iterator_get_value(lua_State *s) {
	const CassIterator *it = *check_cass_iterator(s, 1);
	const CassValue *val = cass_iterator_get_value(it);
	return new_cass_value(s, val);
}

static int
lc_cass_iterator_get_map_key(lua_State *s) {
	const CassIterator *it = *check_cass_iterator(s, 1);
	const CassValue *val = cass_iterator_get_map_key(it);
	return new_cass_value(s, val);
}

static int
lc_cass_iterator_get_map_value(lua_State *s) {
	const CassIterator *it = *check_cass_iterator(s, 1);
	const CassValue *val = cass_iterator_get_map_value(it);
	return new_cass_value(s, val);
}

static int
lc_cass_iterator_get_user_type_field_name(lua_State *s) {
	const CassIterator *it = *check_cass_iterator(s, 1);
	size_t len;
	const char *name;
	CassError e = cass_iterator_get_user_type_field_name(it, &name, &len);
	lua_pushinteger(s, e);
	lua_pushlstring(s, name, len);
	return 2;
}

static int
lc_cass_iterator_get_user_type_field_value(lua_State *s) {
	const CassIterator *it = *check_cass_iterator(s, 1);
	const CassValue *val = cass_iterator_get_user_type_field_value(it);
	return new_cass_value(s, val);
}

static int
lc_cass_iterator_get_keyspace_meta(lua_State *s) {
	const CassIterator *it = *check_cass_iterator(s, 1);
	const CassKeyspaceMeta *meta = cass_iterator_get_keyspace_meta(it);
	return new_cass_keyspace_meta(s, meta);
}

static int
lc_cass_iterator_get_table_meta(lua_State *s) {
	const CassIterator *it = *check_cass_iterator(s, 1);
	const CassTableMeta *meta = cass_iterator_get_table_meta(it);
	return new_cass_table_meta(s, meta);
}

static int
lc_cass_iterator_get_materialized_view_meta(lua_State *s) {
	const CassIterator *it = *check_cass_iterator(s, 1);
	const CassMaterializedViewMeta *meta = cass_iterator_get_materialized_view_meta(it);
	return new_cass_materialized_view_meta(s, meta);
}

static int
lc_cass_iterator_get_user_type(lua_State *s) {
	const CassIterator *it = *check_cass_iterator(s, 1);
	const CassDataType *type = cass_iterator_get_user_type(it);
	return new_cass_data_type(s, type);
}

static int
lc_cass_iterator_get_function_meta(lua_State *s) {
	const CassIterator *it = *check_cass_iterator(s, 1);
	const CassFunctionMeta *meta = cass_iterator_get_function_meta(it);
	return new_cass_function_meta(s, meta);
}

static int
lc_cass_iterator_get_aggregate_meta(lua_State *s) {
	const CassIterator *it = *check_cass_iterator(s, 1);
	const CassAggregateMeta *meta = cass_iterator_get_aggregate_meta(it);
	return new_cass_aggregate_meta(s, meta);
}

static int
lc_cass_iterator_get_column_meta(lua_State *s) {
	const CassIterator *it = *check_cass_iterator(s, 1);
	const CassColumnMeta *meta = cass_iterator_get_column_meta(it);
	return new_cass_column_meta(s, meta);
}

static int
lc_cass_iterator_get_index_meta(lua_State *s) {
	const CassIterator *it = *check_cass_iterator(s, 1);
	const CassIndexMeta *meta = cass_iterator_get_index_meta(it);
	return new_cass_index_meta(s, meta);
}

static int
lc_cass_iterator_get_meta_field_name(lua_State *s) {
	const CassIterator *it = *check_cass_iterator(s, 1);
	size_t len;
	const char *name;
	CassError e = cass_iterator_get_meta_field_name(it, &name, &len);
	lua_pushinteger(s, e);
	lua_pushlstring(s, name, len);
	return 2;
}

static int
lc_cass_iterator_get_meta_field_value(lua_State *s) {
	const CassIterator *it = *check_cass_iterator(s, 1);
	const CassValue *val = cass_iterator_get_meta_field_value(it);
	return new_cass_value(s, val);
}

static int
lc_cass_row_get_column(lua_State *s) {
	const CassRow *row = *check_cass_row(s, 1);
	size_t index = luaL_checkinteger(s, 2);
	const CassValue *val = cass_row_get_column(row, index);
	return new_cass_value(s, val);
}

static int
lc_cass_row_get_column_by_name(lua_State *s) {
	const CassRow *row = *check_cass_row(s, 1);
	size_t len;
	const char *name = luaL_checklstring(s, 2, &len);
	const CassValue *val = cass_row_get_column_by_name(row, name);
	return new_cass_value(s, val);
}

static int
lc_cass_value_data_type(lua_State *s) {
	const CassValue *val = *check_cass_value(s, 1);
	const CassDataType *type = cass_value_data_type(val);
	return new_cass_data_type(s, type);
}

static int
lc_cass_value_get_int8(lua_State *s) {
	const CassValue *val = *check_cass_value(s, 1);
	cass_int8_t v;
	CassError e = cass_value_get_int8(val, &v);
	lua_pushinteger(s, e);
	lua_pushinteger(s, v);
	return 2;
}

static int
lc_cass_value_get_int16(lua_State *s) {
	const CassValue *val = *check_cass_value(s, 1);
	cass_int16_t v;
	CassError e = cass_value_get_int16(val, &v);
	lua_pushinteger(s, e);
	lua_pushinteger(s, v);
	return 2;
}

static int
lc_cass_value_get_int32(lua_State *s) {
	const CassValue *val = *check_cass_value(s, 1);
	cass_int32_t v;
	CassError e = cass_value_get_int32(val, &v);
	lua_pushinteger(s, e);
	lua_pushinteger(s, v);
	return 2;
}

static int
lc_cass_value_get_uint32(lua_State *s) {
	const CassValue *val = *check_cass_value(s, 1);
	cass_uint32_t v;
	CassError e = cass_value_get_uint32(val, &v);
	lua_pushinteger(s, e);
	lua_pushinteger(s, v);
	return 2;
}

static int
lc_cass_value_get_int64(lua_State *s) {
	const CassValue *val = *check_cass_value(s, 1);
	cass_int64_t v;
	CassError e = cass_value_get_int64(val, &v);
	lua_pushinteger(s, e);
	lua_pushinteger(s, v);
	return 2;
}

static int
lc_cass_value_get_float(lua_State *s) {
	const CassValue *val = *check_cass_value(s, 1);
	cass_float_t v;
	CassError e = cass_value_get_float(val, &v);
	lua_pushinteger(s, e);
	lua_pushnumber(s, v);
	return 2;
}

static int
lc_cass_value_get_double(lua_State *s) {
	const CassValue *val = *check_cass_value(s, 1);
	cass_double_t v;
	CassError e = cass_value_get_double(val, &v);
	lua_pushinteger(s, e);
	lua_pushnumber(s, v);
	return 2;
}

static int
lc_cass_value_get_bool(lua_State *s) {
	const CassValue *val = *check_cass_value(s, 1);
	cass_bool_t v;
	CassError e = cass_value_get_bool(val, &v);
	lua_pushinteger(s, e);
	lua_pushboolean(s, v);
	return 2;
}

static int
lc_cass_value_get_uuid(lua_State *s) {
	const CassValue *val = *check_cass_value(s, 1);
	CassUuid v;
	CassError e = cass_value_get_uuid(val, &v);
	uuid_t uuid;
	from_cass_uuid(v, uuid);

	lua_pushinteger(s, e);
	lua_pushlstring(s, (const char *)uuid, sizeof(uuid));
	return 2;
}

static int
lc_cass_value_get_inet(lua_State *s) {
	const CassValue *val = *check_cass_value(s, 1);
	char addr[INET6_ADDRSTRLEN];
	CassInet v;
	CassError e = cass_value_get_inet(val, &v);
	from_cass_inet(v, addr, sizeof(addr));
	lua_pushinteger(s, e);
	lua_pushstring(s, addr);
	return 2;
}

static int
lc_cass_value_get_string(lua_State *s) {
	const CassValue *val = *check_cass_value(s, 1);
	size_t len;
	const char *v;
	CassError e = cass_value_get_string(val, &v, &len);
	lua_pushinteger(s, e);
	lua_pushlstring(s, v, len);
	return 2;
}

static int
lc_cass_value_get_bytes(lua_State *s) {
	const CassValue *val = *check_cass_value(s, 1);
	const cass_byte_t *v;
	size_t len;
	CassError e = cass_value_get_bytes(val, &v, &len);
	lua_pushinteger(s, e);
	lua_pushlstring(s, (const char *)v, len);
	return 2;
}

static int
lc_cass_value_get_decimal(lua_State *s) {
	const CassValue *val = *check_cass_value(s, 1);
	const cass_byte_t *v;
	size_t len;
	cass_int32_t scale;
	CassError e = cass_value_get_decimal(val, &v, &len, &scale);
	lua_pushinteger(s, e);
	lua_pushlstring(s, (const char *)v, len);
	lua_pushinteger(s, scale);
	return 3;
}

static int
lc_cass_value_type(lua_State *s) {
	const CassValue *val = *check_cass_value(s, 1);
	CassValueType t = cass_value_type(val);
	lua_pushinteger(s, t);
	return 1;
}

static int
lc_cass_value_is_null(lua_State *s) {
	const CassValue *val = *check_cass_value(s, 1);
	cass_bool_t v = cass_value_is_null(val);
	lua_pushboolean(s, v);
	return 1;
}

static int
lc_cass_value_is_collection(lua_State *s) {
	const CassValue *val = *check_cass_value(s, 1);
	cass_bool_t v = cass_value_is_collection(val);
	lua_pushboolean(s, v);
	return 1;
}

static int
lc_cass_value_item_count(lua_State *s) {
	const CassValue *val = *check_cass_value(s, 1);
	size_t v = cass_value_item_count(val);
	lua_pushinteger(s, v);
	return 1;
}

static int
lc_cass_value_primary_sub_type(lua_State *s) {
	const CassValue *val = *check_cass_value(s, 1);
	CassValueType t = cass_value_primary_sub_type(val);
	lua_pushinteger(s, t);
	return 1;
}

static int
lc_cass_value_secondary_sub_type(lua_State *s) {
	const CassValue *val = *check_cass_value(s, 1);
	CassValueType t = cass_value_secondary_sub_type(val);
	lua_pushinteger(s, t);
	return 1;
}

static int
lc_cass_uuid_gen_new(lua_State *s) {
	CassUuidGen *g = cass_uuid_gen_new();
	return new_cass_uuid_gen(s, g);
}

static int
lc_cass_uuid_gen_new_with_node(lua_State *s) {
	cass_uint64_t node = luaL_checkinteger(s, 1);
	CassUuidGen *g = cass_uuid_gen_new_with_node(node);
	return new_cass_uuid_gen(s, g);
}

static int
lc_cass_uuid_gen_free(lua_State *s) {
	CassUuidGen **g = check_cass_uuid_gen(s, 1);
	cass_uuid_gen_free(*g);
	*g = NULL;
	return 0;
}

static int
lc_cass_uuid_gen_time(lua_State *s) {
	CassUuidGen *g = *check_cass_uuid_gen(s, 1);
	CassUuid v;
	uuid_t uuid;
	cass_uuid_gen_time(g, &v);
	from_cass_uuid(v, uuid);
	lua_pushlstring(s, (const char *)uuid, sizeof(uuid));
	return 1;
}

static int
lc_cass_uuid_gen_random(lua_State *s) {
	CassUuidGen *g = *check_cass_uuid_gen(s, 1);
	CassUuid v;
	uuid_t uuid;
	cass_uuid_gen_random(g, &v);
	from_cass_uuid(v, uuid);
	lua_pushlstring(s, (const char *)uuid, sizeof(uuid));
	return 1;
}

static int
lc_cass_uuid_gen_from_time(lua_State *s) {
	CassUuidGen *g = *check_cass_uuid_gen(s, 1);
	cass_uint64_t timestamp = luaL_checkinteger(s, 2);
	CassUuid v;
	uuid_t uuid;
	cass_uuid_gen_from_time(g, timestamp, &v);
	from_cass_uuid(v, uuid);
	lua_pushlstring(s, (const char *)uuid, sizeof(uuid));
	return 1;
}

static int
lc_cass_uuid_min_from_time(lua_State *s) {
	cass_uint64_t t = luaL_checkinteger(s, 1);
	CassUuid v;
	uuid_t uuid;
	cass_uuid_min_from_time(t, &v);
	from_cass_uuid(v, uuid);
	lua_pushlstring(s, (const char *)uuid, sizeof(uuid));
	return 1;
}

static int
lc_cass_uuid_max_from_time(lua_State *s) {
	cass_uint64_t t = luaL_checkinteger(s, 1);
	CassUuid v;
	uuid_t uuid;
	cass_uuid_max_from_time(t, &v);
	from_cass_uuid(v, uuid);
	lua_pushlstring(s, (const char *)uuid, sizeof(uuid));
	return 1;
}

static int
lc_cass_uuid_timestamp(lua_State *s) {
	CassUuid u = to_cass_uuid(s, 1);
	cass_uint64_t v = cass_uuid_timestamp(u);
	lua_pushinteger(s, v);
	return 1;
}

static int
lc_cass_uuid_version(lua_State *s) {
	CassUuid u = to_cass_uuid(s, 1);
	cass_uint8_t v = cass_uuid_version(u);
	lua_pushinteger(s, v);
	return 1;
}

static int
lc_cass_uuid_string(lua_State *s) {
	CassUuid u = to_cass_uuid(s, 1);
	char v;
	cass_uuid_string(u, &v);
	lua_pushstring(s, &v);
	return 1;
}

static int
lc_cass_uuid_from_string(lua_State *s) {
	size_t len;
	const char *str = luaL_checklstring(s, 1, &len);
	CassUuid u;
	uuid_t uuid;
	CassError e = cass_uuid_from_string(str, &u);
	from_cass_uuid(u, uuid);
	lua_pushinteger(s, e);
	lua_pushlstring(s, (const char *)uuid, sizeof(uuid));
	return 2;
}

static int
lc_cass_timestamp_gen_server_side_new(lua_State *s) {
	CassTimestampGen *g = cass_timestamp_gen_server_side_new();
	return new_cass_timestamp_gen(s, g);
}

static int
lc_cass_timestamp_gen_monotonic_new(lua_State *s) {
	CassTimestampGen *g = cass_timestamp_gen_monotonic_new();
	return new_cass_timestamp_gen(s, g);
}

static int
lc_cass_timestamp_gen_free(lua_State *s) {
	CassTimestampGen **g = check_cass_timestamp_gen(s, 1);
	cass_timestamp_gen_free(*g);
	*g = NULL;
	return 0;
}

static int
lc_cass_retry_policy_default_new(lua_State *s) {
	CassRetryPolicy *pol = cass_retry_policy_default_new();
	return new_cass_retry_policy(s, pol);
}

static int
lc_cass_retry_policy_downgrading_consistency_new(lua_State *s) {
	CassRetryPolicy *pol = cass_retry_policy_downgrading_consistency_new();
	return new_cass_retry_policy(s, pol);
}

static int
lc_cass_retry_policy_fallthrough_new(lua_State *s) {
	CassRetryPolicy *pol = cass_retry_policy_fallthrough_new();
	return new_cass_retry_policy(s, pol);
}

static int
lc_cass_retry_policy_logging_new(lua_State *s) {
	CassRetryPolicy *child = *check_cass_retry_policy(s, 1);
	CassRetryPolicy *pol = cass_retry_policy_logging_new(child);
	return new_cass_retry_policy(s, pol);
}

static int
lc_cass_retry_policy_free(lua_State *s) {
	CassRetryPolicy **pol = check_cass_retry_policy(s, 1);
	cass_retry_policy_free(*pol);

	*pol = NULL;
	return 0;
}

static int
lc_cass_custom_payload_new(lua_State *s) {
	CassCustomPayload *payload = cass_custom_payload_new();
	if (NULL != payload) {
		CassCustomPayload **impl = (CassCustomPayload **)
			lua_newuserdata(s, sizeof(CassCustomPayload *));
		luaL_getmetatable(s, "datastax.cass_custom_payload");
		lua_setmetatable(s, -2);
		*impl = payload;
	} else
		lua_pushnil(s);
	return 1;
}

static int
lc_cass_custom_payload_free(lua_State *s) {
	CassCustomPayload **p = check_cass_custom_payload(s, 1);
	cass_custom_payload_free(*p);
	*p = NULL;
	return 0;
}

static int
lc_cass_custom_payload_set(lua_State *s) {
	CassCustomPayload *p = *check_cass_custom_payload(s, 1);
	size_t len;
	const char *name = luaL_checklstring(s, 2, &len);
	size_t vlen;
	const cass_byte_t *val = (const cass_byte_t *)luaL_checklstring(s, 3, &vlen);
	cass_custom_payload_set(p, name, val, vlen);
	return 0;
}

static int
lc_cass_consistency_string(lua_State *s) {
	CassConsistency c = luaL_checkinteger(s, 1);
	const char *v = cass_consistency_string(c);
	lua_pushstring(s, v);
	return 1;
}

static int
lc_cass_write_type_string(lua_State *s) {
	CassWriteType wt = luaL_checkinteger(s, 1);
	const char *v = cass_write_type_string(wt);
	lua_pushstring(s, v);
	return 1;
}

static int
lc_cass_error_desc(lua_State *s) {
	CassError e = luaL_checkinteger(s, 1);
	const char *err = cass_error_desc(e);
	lua_pushstring(s, err);
	return 1;
}

/* static int */
/* lc_cass_log_cleanup(lua_State *s) { */
/* 	cass_log_cleanup(); */
/* 	return 0; */
/* } */

static int
lc_cass_log_set_level(lua_State *s) {
	CassLogLevel lvl = luaL_checkinteger(s, 1);
	cass_log_set_level(lvl);
	return 0;
}

static int log_cb_ref = LUA_NOREF;
static lua_State *log_thread = NULL;
static int log_thread_ndx = LUA_NOREF;

static void
log_callback(const CassLogMessage *log_msg, void *dummy) {
	if (LUA_NOREF == log_cb_ref)
		return;

	pthread_mutex_lock(&log_mtx);

	/* push the {cb, data} table on the stack */
	lua_rawgeti(log_thread, LUA_REGISTRYINDEX, log_cb_ref);
	/* push the cb on top of the stack */
	lua_pushstring(log_thread, "cb");
	lua_gettable(log_thread, -2);
	/* create the log message */
	lua_newtable(log_thread);
	/* time_ms */
	lua_pushstring(log_thread, "time_ms");
	lua_pushinteger(log_thread, log_msg->time_ms);
	lua_settable(log_thread, -3);
	/* severity */
	lua_pushstring(log_thread, "severity");
	lua_pushinteger(log_thread, log_msg->severity);
	lua_settable(log_thread, -3);
	/* file */
	lua_pushstring(log_thread, "file");
	lua_pushstring(log_thread, log_msg->file);
	lua_settable(log_thread, -3);
	/* line */
	lua_pushstring(log_thread, "line");
	lua_pushinteger(log_thread, log_msg->line);
	lua_settable(log_thread, -3);
	/* function */
	lua_pushstring(log_thread, "cfunction");
	lua_pushstring(log_thread, log_msg->function);
	lua_settable(log_thread, -3);
	/* message */
	lua_pushstring(log_thread, "message");
	lua_pushstring(log_thread, log_msg->message);
	lua_settable(log_thread, -3);
	/* push the opaque data on top of the stack */
	lua_pushstring(log_thread, "data");
	lua_gettable(log_thread, -4);
	/* remove the {cb, future, data} table from the stack */
	lua_remove(log_thread, -4);

	if (0 != lua_pcall(log_thread, 2, 0, 0)) {
		const char *err = lua_tostring(log_thread, -1);
		(void)err;
		lua_pop(log_thread, 1);
	}

	pthread_mutex_unlock(&log_mtx);
}

static int
lc_cass_log_set_callback(lua_State *s) {
	/* lock */
	if (NULL == log_thread) {
		pthread_mutex_lock(&log_mtx);
		if (NULL == log_thread) {
			log_thread = lua_newthread(s);
			log_thread_ndx = luaL_ref(s, LUA_REGISTRYINDEX);
		}
		pthread_mutex_unlock(&log_mtx);
	}
	lua_newtable(s);
	/* add the callback to the table */
	lua_pushstring(s, "cb");
	lua_pushvalue(s, 1);
	lua_settable(s, -3);
	/* add the opaque data to the table */
	lua_pushstring(s, "data");
	lua_pushvalue(s, 2);
	lua_settable(s, -3);
	/* store {cb, data} in the registry */
	if (LUA_NOREF != log_cb_ref)
		luaL_unref(s, LUA_REGISTRYINDEX, log_cb_ref);
	log_cb_ref = luaL_ref(s, LUA_REGISTRYINDEX);
	/* unlock */

	cass_log_set_callback(log_callback, NULL);
	return 0;
}

/* static int */
/* lc_cass_log_set_queue_size(lua_State *s) { */
/* 	size_t sz = luaL_checkinteger(s, 1); */
/* 	cass_log_set_queue_size(sz); */
/* 	return 0; */
/* } */

static int
lc_cass_log_level_string(lua_State *s) {
	CassLogLevel lvl = luaL_checkinteger(s, 1);
	const char *v = cass_log_level_string(lvl);
	lua_pushstring(s, v);
	return 1;
}

/* static int */
/* lc_cass_inet_init_v4(lua_State *s) { */
/* 	const cass_uint8_t *addr = lua_touserdata(s, 1); */
/* 	char addr_str[INET_ADDRSTRLEN]; */
/* 	CassInet v = cass_inet_init_v4(addr); */
/* 	from_cass_inet(v, addr_str, sizeof(addr_str)); */
/* 	lua_pushstring(s, addr_str); */
/* 	return 1; */
/* } */

/* static int */
/* lc_cass_inet_init_v6(lua_State *s) { */
/* 	const cass_uint8_t *addr = lua_touserdata(s, 1); */
/* 	char addr_str[INET6_ADDRSTRLEN]; */
/* 	CassInet v = cass_inet_init_v6(addr); */
/* 	from_cass_inet(v, addr_str, sizeof(addr_str)); */
/* 	lua_pushstring(s, addr_str); */
/* 	return 1; */
/* } */

/* static int */
/* lc_cass_inet_string(lua_State *s) { */
/* 	CassInet inet; */
/* 	void *address = lua_touserdata(s, 1); */
/* 	inet.address_length = (cass_uint8_t)luaL_checkinteger(s, 2); */
/* 	memcpy(inet.address, address, inet.address_length); */
/* 	char v; */
/* 	cass_inet_string(inet, &v); */
/* 	lua_pushstring(s, &v); */
/* 	return 1; */
/* } */

/* static int */
/* lc_cass_inet_from_string(lua_State *s) { */
/* 	size_t len; */
/* 	const char *str = luaL_checklstring(s, 1, &len); */
/* 	CassInet inet; */
/* 	CassError e = cass_inet_from_string(str, &inet); */
/* 	lua_pushinteger(s, e); */
/* 	return 1; */
/* } */

static int
lc_cass_date_from_epoch(lua_State *s) {
	cass_int64_t epoch_secs = luaL_checkinteger(s, 1);
	cass_uint32_t v = cass_date_from_epoch(epoch_secs);
	lua_pushinteger(s, v);
	return 1;
}

static int
lc_cass_time_from_epoch(lua_State *s) {
	cass_int64_t epoch_secs = luaL_checkinteger(s, 1);
	cass_int64_t v = cass_time_from_epoch(epoch_secs);
	lua_pushinteger(s, v);
	return 1;
}

static int
lc_cass_date_time_to_epoch(lua_State *s) {
	cass_uint32_t date = luaL_checkinteger(s, 1);
	cass_int64_t tm = luaL_checkinteger(s, 2);
	cass_int64_t v = cass_date_time_to_epoch(date, tm);
	lua_pushinteger(s, v);
	return 1;
}

static const struct luaL_reg
functions[] = {
	{"cass_cluster_new", lc_cass_cluster_new},
	{"cass_session_new", lc_cass_session_new},
	{"cass_ssl_new", lc_cass_ssl_new},
	{"cass_statement_new", lc_cass_statement_new},
	{"cass_batch_new", lc_cass_batch_new},
	{"cass_data_type_new", lc_cass_data_type_new},
	{"cass_data_type_new_tuple", lc_cass_data_type_new_tuple},
	{"cass_data_type_new_udt", lc_cass_data_type_new_udt},
	{"cass_collection_new", lc_cass_collection_new},
	{"cass_tuple_new", lc_cass_tuple_new},
	{"cass_uuid_gen_new", lc_cass_uuid_gen_new},
	{"cass_uuid_gen_new_with_node", lc_cass_uuid_gen_new_with_node},
	{"cass_timestamp_gen_server_side_new", lc_cass_timestamp_gen_server_side_new},
	{"cass_timestamp_gen_monotonic_new", lc_cass_timestamp_gen_monotonic_new},
	{"cass_retry_policy_default_new", lc_cass_retry_policy_default_new},
	{"cass_retry_policy_downgrading_consistency_new", lc_cass_retry_policy_downgrading_consistency_new},
	{"cass_retry_policy_fallthrough_new", lc_cass_retry_policy_fallthrough_new},
	{"cass_retry_policy_logging_new", lc_cass_retry_policy_logging_new},
	{"cass_custom_payload_new", lc_cass_custom_payload_new},

	/* {"cass_cluster_free", lc_cass_cluster_free}, */
	/* {"cass_session_free", lc_cass_session_free}, */
	/* {"cass_ssl_free", lc_cass_ssl_free}, */
	/* {"cass_statement_free", lc_cass_statement_free}, */
	/* {"cass_batch_free", lc_cass_batch_free}, */
	/* {"cass_data_type_free", lc_cass_data_type_free}, */
	/* {"cass_collection_free", lc_cass_collection_free}, */
	/* {"cass_tuple_free", lc_cass_tuple_free}, */
	/* {"cass_uuid_gen_free", lc_cass_uuid_gen_free}, */
	/* {"cass_timestamp_gen_free", lc_cass_timestamp_gen_free}, */
	/* {"cass_retry_policy_free", lc_cass_retry_policy_free}, */
	/* {"cass_custom_payload_free", lc_cass_custom_payload_free}, */

	/* {"cass_prepared_free", lc_cass_prepared_free}, */
	/* {"cass_future_free", lc_cass_future_free}, */
	/* {"cass_schema_meta_free", lc_cass_schema_meta_free}, */
	/* {"cass_user_type_free", lc_cass_user_type_free}, */
	/* {"cass_result_free", lc_cass_result_free}, */
	/* {"cass_error_result_free", lc_cass_error_result_free}, */
	/* {"cass_iterator_free", lc_cass_iterator_free}, */

	{"cass_consistency_string", lc_cass_consistency_string},
	{"cass_write_type_string", lc_cass_write_type_string},
	{"cass_error_desc", lc_cass_error_desc},
	/* {"cass_log_cleanup", lc_cass_log_cleanup}, */
	{"cass_log_set_level", lc_cass_log_set_level},
	{"cass_log_set_callback", lc_cass_log_set_callback},
	/* {"cass_log_set_queue_size", lc_cass_log_set_queue_size}, */
	{"cass_log_level_string", lc_cass_log_level_string},
	/* {"cass_inet_init_v4", lc_cass_inet_init_v4}, */
	/* {"cass_inet_init_v6", lc_cass_inet_init_v6}, */
	/* {"cass_inet_string", lc_cass_inet_string}, */
	/* {"cass_inet_from_string", lc_cass_inet_from_string}, */
	{"cass_date_from_epoch", lc_cass_date_from_epoch},
	{"cass_time_from_epoch", lc_cass_time_from_epoch},
	{"cass_date_time_to_epoch", lc_cass_date_time_to_epoch},
	{"cass_uuid_min_from_time", lc_cass_uuid_min_from_time},
	{"cass_uuid_max_from_time", lc_cass_uuid_max_from_time},
	{"cass_uuid_timestamp", lc_cass_uuid_timestamp},
	{"cass_uuid_version", lc_cass_uuid_version},
	{"cass_uuid_string", lc_cass_uuid_string},
	{"cass_uuid_from_string", lc_cass_uuid_from_string},
	{NULL, NULL}
};

static const struct luaL_reg
cluster_methods[] = {
	{"set_contact_points", lc_cass_cluster_set_contact_points},
	{"set_port", lc_cass_cluster_set_port},
	{"set_ssl", lc_cass_cluster_set_ssl},
	{"set_protocol_version", lc_cass_cluster_set_protocol_version},
	{"set_num_threads_io", lc_cass_cluster_set_num_threads_io},
	{"set_queue_size_io", lc_cass_cluster_set_queue_size_io},
	{"set_queue_size_event", lc_cass_cluster_set_queue_size_event},
	/* {"set_queue_size_log", lc_cass_cluster_set_queue_size_log}, */
	{"set_core_connections_per_host", lc_cass_cluster_set_core_connections_per_host},
	{"set_max_connections_per_host", lc_cass_cluster_set_max_connections_per_host},
	{"set_reconnect_wait_time", lc_cass_cluster_set_reconnect_wait_time},
	{"set_max_concurrent_creation", lc_cass_cluster_set_max_concurrent_creation},
	{"set_max_concurrent_requests_threshold", lc_cass_cluster_set_max_concurrent_requests_threshold},
	{"set_max_requests_per_flush", lc_cass_cluster_set_max_requests_per_flush},
	{"set_write_bytes_high_water_mark", lc_cass_cluster_set_write_bytes_high_water_mark},
	{"set_write_bytes_low_water_mark", lc_cass_cluster_set_write_bytes_low_water_mark},
	{"set_pending_requests_high_water_mark", lc_cass_cluster_set_pending_requests_high_water_mark},
	{"set_pending_requests_low_water_mark", lc_cass_cluster_set_pending_requests_low_water_mark},
	{"set_connect_timeout", lc_cass_cluster_set_connect_timeout},
	{"set_request_timeout", lc_cass_cluster_set_request_timeout},
	{"set_credentials", lc_cass_cluster_set_credentials},
	{"set_load_balance_round_robin", lc_cass_cluster_set_load_balance_round_robin},
	{"set_load_balance_dc_aware", lc_cass_cluster_set_load_balance_dc_aware},
	{"set_token_aware_routing", lc_cass_cluster_set_token_aware_routing},
	{"set_latency_aware_routing", lc_cass_cluster_set_latency_aware_routing},
	{"set_latency_aware_routing_settings", lc_cass_cluster_set_latency_aware_routing_settings},
	{"set_whitelist_filtering", lc_cass_cluster_set_whitelist_filtering},
	{"set_blacklist_filtering", lc_cass_cluster_set_blacklist_filtering},
	{"set_whitelist_dc+filtering", lc_cass_cluster_set_whitelist_dc_filtering},
	{"set_blacklist_dc_filtering", lc_cass_cluster_set_blacklist_dc_filtering},
	{"set_tcp_nodelay", lc_cass_cluster_set_tcp_nodelay},
	{"set_tcp_keepalive", lc_cass_cluster_set_tcp_keepalive},
	{"set_timestamp_gen", lc_cass_cluster_set_timestamp_gen},
	{"set_connection_heartbeat_interval", lc_cass_cluster_set_connection_heartbeat_interval},
	{"set_connection_idle_timeout", lc_cass_cluster_set_connection_idle_timeout},
	{"set_retry_policy", lc_cass_cluster_set_retry_policy},
	{"set_use_schema", lc_cass_cluster_set_use_schema},
	{NULL, NULL}
};

static const struct luaL_reg
session_methods[] = {
	{"connect", lc_cass_session_connect},
	{"connect_keyspace", lc_cass_session_connect_keyspace},
	{"close", lc_cass_session_close},
	{"prepare", lc_cass_session_prepare},
	{"execute", lc_cass_session_execute},
	{"execute_batch", lc_cass_session_execute_batch},
	{"get_schema_meta", lc_cass_session_get_schema_meta},
	{"get_metrics", lc_cass_session_get_metrics},
	{NULL, NULL}
};

static const struct luaL_reg
schema_meta_methods[] = {
	{"snapshot_version", lc_cass_schema_meta_snapshot_version},
	{"version", lc_cass_schema_meta_version},
	{"keyspace_by_name", lc_cass_schema_meta_keyspace_by_name},
	{"keyspaces_iterator", lc_cass_iterator_keyspaces_from_schema_meta},
	{NULL, NULL}
};

static const struct luaL_reg
keyspace_meta_methods[] = {
	{"name", lc_cass_keyspace_meta_name},
	{"table_by_name", lc_cass_keyspace_meta_table_by_name},
	{"materialized_view_by_name", lc_cass_keyspace_meta_materialized_view_by_name},
	{"user_type_by_name", lc_cass_keyspace_meta_user_type_by_name},
	{"function_by_name", lc_cass_keyspace_meta_function_by_name},
	{"aggregate_by_name", lc_cass_keyspace_meta_aggregate_by_name},
	{"field_by_name", lc_cass_keyspace_meta_field_by_name},
	{"tables_iterator", lc_cass_iterator_tables_from_keyspace_meta},
	{"materialized_views_iterator", lc_cass_iterator_materialized_views_from_keyspace_meta},
	{"user_types_iterator", lc_cass_iterator_user_types_from_keyspace_meta},
	{"functions_iterator", lc_cass_iterator_functions_from_keyspace_meta},
	{"aggregates_iterator", lc_cass_iterator_aggregates_from_keyspace_meta},
	{"fields_iterator", lc_cass_iterator_fields_from_keyspace_meta},
	{NULL, NULL}
};

static const struct luaL_reg
table_meta_methods[] = {
	{"column_by_name", lc_cass_table_meta_column_by_name},
	{"name", lc_cass_table_meta_name},
	{"column_count", lc_cass_table_meta_column_count},
	{"column", lc_cass_table_meta_column},
	{"index_by_name", lc_cass_table_meta_index_by_name},
	{"index_count", lc_cass_table_meta_index_count},
	{"index", lc_cass_table_meta_index},
	{"materialized_view_by_name", lc_cass_table_meta_materialized_view_by_name},
	{"meterialized_view_count", lc_cass_table_meta_meterialized_view_count},
	{"meterialized_view", lc_cass_table_meta_meterialized_view},
	{"partition_key_count", lc_cass_table_meta_partition_key_count},
	{"partition_key", lc_cass_table_meta_partition_key},
	{"clustering_key_count", lc_cass_table_meta_clustering_key_count},
	{"clustering_key", lc_cass_table_meta_clustering_key},
	{"clustering_key_order", lc_cass_table_meta_clustering_key_order},
	{"field_by_name", lc_cass_table_meta_field_by_name},
	{"columns_iterator", lc_cass_iterator_columns_from_table_meta},
	{"fields_iterator", lc_cass_iterator_fields_from_table_meta},
	{"indexes_iterator", lc_cass_iterator_indexes_from_table_meta},
	{"materialized_views_iterator", lc_cass_iterator_materialized_views_from_table_meta},
	{NULL, NULL}
};

static const struct luaL_reg
materialized_view_meta_methods[] = {
	{"column_by_name", lc_cass_materialized_view_meta_column_by_name},
	{"name", lc_cass_materialized_view_meta_name},
	{"base_table", lc_cass_materialized_view_meta_base_table},
	{"column_count", lc_cass_materialized_view_meta_column_count},
	{"column", lc_cass_materialized_view_meta_column},
	{"partition_key_count", lc_cass_materialized_view_meta_partition_key_count},
	{"partition_key", lc_cass_materialized_view_meta_partition_key},
	{"clustering_key_count", lc_cass_materialized_view_meta_clustering_key_count},
	{"clustering_key", lc_cass_materialized_view_meta_clustering_key},
	{"clustering_key_order", lc_cass_materialized_view_meta_clustering_key_order},
	{"field_by_name", lc_cass_materialized_view_meta_field_by_name},
	{"fields_iterator", lc_cass_iterator_fields_from_materialized_view_meta},
	{"columns_iterator", lc_cass_iterator_columns_from_materialized_view_meta},
	{NULL, NULL}
};

static const struct luaL_reg
column_meta_methods[] = {
	{"name", lc_cass_column_meta_name},
	{"type", lc_cass_column_meta_type},
	{"data_type", lc_cass_column_meta_data_type},
	{"field_by_name", lc_cass_column_meta_field_by_name},
	{"fields_iterator", lc_cass_iterator_fields_from_column_meta},
	{NULL, NULL}
};

static const struct luaL_reg
index_meta_methods[] = {
	{"name", lc_cass_index_meta_name},
	{"type", lc_cass_index_meta_type},
	{"target", lc_cass_index_meta_target},
	{"options", lc_cass_index_meta_options},
	{"field_by_name", lc_cass_index_meta_field_by_name},
	{"fields_iterator", lc_cass_iterator_fields_from_index_meta},
	{NULL, NULL}
};

static const struct luaL_reg
function_meta_methods[] = {
	{"name", lc_cass_function_meta_name},
	{"full_name", lc_cass_function_meta_full_name},
	{"body", lc_cass_function_meta_body},
	{"language", lc_cass_function_meta_language},
	{"called_on_null_input", lc_cass_function_meta_called_on_null_input},
	{"argument_count", lc_cass_function_meta_argument_count},
	{"argument", lc_cass_function_meta_argument},
	{"argument_type_by_name", lc_cass_function_meta_argument_type_by_name},
	{"return_type", lc_cass_function_meta_return_type},
	{"field_by_name", lc_cass_function_meta_field_by_name},
	{"fields_iterator", lc_cass_iterator_fields_from_function_meta},
	{NULL, NULL}
};

static const struct luaL_reg
aggregate_meta_methods[] = {
	{"name", lc_cass_aggregate_meta_name},
	{"full_name", lc_cass_aggregate_meta_full_name},
	{"argument_count", lc_cass_aggregate_meta_argument_count},
	{"argument_type", lc_cass_aggregate_meta_argument_type},
	{"return_type", lc_cass_aggregate_meta_return_type},
	{"state_type", lc_cass_aggregate_meta_state_type},
	{"state_func", lc_cass_aggregate_meta_state_func},
	{"final_func", lc_cass_aggregate_meta_final_func},
	{"init_cond", lc_cass_aggregate_meta_init_cond},
	{"field_by_name", lc_cass_aggregate_meta_field_by_name},
	{"fields_iterator", lc_cass_iterator_fields_from_aggregate_meta},
	{NULL, NULL}
};

static const struct luaL_reg
ssl_methods[] = {
	{"add_trusted_cert", lc_cass_ssl_add_trusted_cert},
	{"set_verify_flags", lc_cass_ssl_set_verify_flags},
	{"set_cert", lc_cass_ssl_set_cert},
	{"set_private_key", lc_cass_ssl_set_private_key},
	{NULL, NULL}
};

static const struct luaL_reg
future_methods[] = {
	{"set_callback", lc_cass_future_set_callback},
	{"ready", lc_cass_future_ready},
	{"wait", lc_cass_future_wait},
	{"wait_timed", lc_cass_future_wait_timed},
	{"get_result", lc_cass_future_get_result},
	{"get_error_result", lc_cass_future_get_error_result},
	{"get_prepared", lc_cass_future_get_prepared},
	{"error_code", lc_cass_future_error_code},
	{"error_message", lc_cass_future_error_message},
	{"custom_payload_item_count", lc_cass_future_custom_payload_item_count},
	{"custom_payload_item", lc_cass_future_custom_payload_item},
	{NULL, NULL}
};

static const struct luaL_reg
statement_methods[] = {
	{"add_key_index", lc_cass_statement_add_key_index},
	{"set_keyspace", lc_cass_statement_set_keyspace},
	{"set_consistency", lc_cass_statement_set_consistency},
	{"set_serial_consistency", lc_cass_statement_set_serial_consistency},
	{"set_paging_size", lc_cass_statement_set_paging_size},
	{"set_paging_state", lc_cass_statement_set_paging_state},
	{"set_paging_state_token", lc_cass_statement_set_paging_state_token},
	{"set_timestamp", lc_cass_statement_set_timestamp},
	{"set_retry_policy", lc_cass_statement_set_retry_policy},
	{"set_custom_payload", lc_cass_statement_set_custom_payload},
	{"bind_null", lc_cass_statement_bind_null},
	{"bind_null_by_name", lc_cass_statement_bind_null_by_name},
	{"bind_int8", lc_cass_statement_bind_int8},
	{"bind_int8_by_name", lc_cass_statement_bind_int8_by_name},
	{"bind_int16", lc_cass_statement_bind_int16},
	{"bind_int16_by_name", lc_cass_statement_bind_int16_by_name},
	{"bind_int32", lc_cass_statement_bind_int32},
	{"bind_int32_by_name", lc_cass_statement_bind_int32_by_name},
	{"bind_uint32", lc_cass_statement_bind_uint32},
	{"bind_uint32_by_name", lc_cass_statement_bind_uint32_by_name},
	{"bind_int64", lc_cass_statement_bind_int64},
	{"bind_int64_by_name", lc_cass_statement_bind_int64_by_name},
	{"bind_float", lc_cass_statement_bind_float},
	{"bind_float_by_name", lc_cass_statement_bind_float_by_name},
	{"bind_double", lc_cass_statement_bind_double},
	{"bind_double_by_name", lc_cass_statement_bind_double_by_name},
	{"bind_bool", lc_cass_statement_bind_bool},
	{"bind_bool_by_name", lc_cass_statement_bind_bool_by_name},
	{"bind_string", lc_cass_statement_bind_string},
	{"bind_string_by_name", lc_cass_statement_bind_string_by_name},
	{"bind_bytes", lc_cass_statement_bind_bytes},
	{"bind_bytes_by_name", lc_cass_statement_bind_bytes_by_name},
	{"bind_uuid", lc_cass_statement_bind_uuid},
	{"bind_uuid_by_name", lc_cass_statement_bind_uuid_by_name},
	{"bind_inet", lc_cass_statement_bind_inet},
	{"bind_inet_by_name", lc_cass_statement_bind_inet_by_name},
	{"bind_decimal", lc_cass_statement_bind_decimal},
	{"bind_decimal_by_name", lc_cass_statement_bind_decimal_by_name},
	{"bind_collection", lc_cass_statement_bind_collection},
	{"bind_collection_by_name", lc_cass_statement_bind_collection_by_name},
	{"bind_tuple", lc_cass_statement_bind_tuple},
	{"bind_tuple_by_name", lc_cass_statement_bind_tuple_by_name},
	{"bind_user_type", lc_cass_statement_bind_user_type},
	{"bind_user_type_by_name", lc_cass_statement_bind_user_type_by_name},
	{NULL, NULL}
};

static const struct luaL_reg
prepared_methods[] = {
	{"bind", lc_cass_prepared_bind},
	{"parameter_name", lc_cass_prepared_parameter_name},
	{"parameter_data_type", lc_cass_prepared_parameter_data_type},
	{"parameter_data_type_by_name", lc_cass_prepared_parameter_data_type_by_name},
	{NULL, NULL}
};

static const struct luaL_reg
batch_methods[] = {
	{"set_consistency", lc_cass_batch_set_consistency},
	{"set_serial_consistency", lc_cass_batch_set_serial_consistency},
	{"set_timestamp", lc_cass_batch_set_timestamp},
	{"set_retry_policy", lc_cass_batch_set_retry_policy},
	{"set_custom_payload", lc_cass_batch_set_custom_payload},
	{"add_statement", lc_cass_batch_add_statement},
	{NULL, NULL}
};

static const struct luaL_reg
data_type_methods[] = {
	{"new", lc_cass_data_type_new_from_existing},
	{"type", lc_cass_data_type_type},
	{"type_name", lc_cass_data_type_type_name},
	{"set_type_name", lc_cass_data_type_set_type_name},
	{"keyspace", lc_cass_data_type_keyspace},
	{"set_keyspace", lc_cass_data_type_set_keyspace},
	{"class_name", lc_cass_data_type_class_name},
	{"set_class_name", lc_cass_data_type_set_class_name},
	{"sub_type_count", lc_cass_data_type_sub_type_count},
	/* {"cass_data_sub_type_count", lc_cass_data_sub_type_count}, */
	{"sub_data_type", lc_cass_data_type_sub_data_type},
	{"sub_data_type_by_name", lc_cass_data_type_sub_data_type_by_name},
	{"sub_type_name", lc_cass_data_type_sub_type_name},
	{"add_sub_type", lc_cass_data_type_add_sub_type},
	{"add_sub_type_by_name", lc_cass_data_type_add_sub_type_by_name},
	{"add_sub_value_type", lc_cass_data_type_add_sub_value_type},
	{"add_sub_value_type_by_name", lc_cass_data_type_add_sub_value_type_by_name},
	{"new_collection", lc_cass_collection_new_from_data_type},
	{"new_tuple", lc_cass_tuple_new_from_data_type},
	{"new_user_type", lc_cass_user_type_new_from_data_type},
	{NULL, NULL}
};

static const struct luaL_reg
collection_methods[] = {
	{"data_type", lc_cass_collection_data_type},
	{"append_int8", lc_cass_collection_append_int8},
	{"append_int16", lc_cass_collection_append_int16},
	{"append_int32", lc_cass_collection_append_int32},
	{"append_uint32", lc_cass_collection_append_uint32},
	{"append_int64", lc_cass_collection_append_int64},
	{"append_float", lc_cass_collection_append_float},
	{"append_double", lc_cass_collection_append_double},
	{"append_bool", lc_cass_collection_append_bool},
	{"append_string", lc_cass_collection_append_string},
	{"append_bytes", lc_cass_collection_append_bytes},
	{"append_uuid", lc_cass_collection_append_uuid},
	{"append_inet", lc_cass_collection_append_inet},
	{"append_decimal", lc_cass_collection_append_decimal},
	{"append_collection", lc_cass_collection_append_collection},
	{"append_tuple", lc_cass_collection_append_tuple},
	{"append_user_type", lc_cass_collection_append_user_type},
	{NULL, NULL}
};

static const struct luaL_reg
tuple_methods[] = {
	{"data_type", lc_cass_tuple_data_type},
	{"set_null", lc_cass_tuple_set_null},
	{"set_int8", lc_cass_tuple_set_int8},
	{"set_int16", lc_cass_tuple_set_int16},
	{"set_int32", lc_cass_tuple_set_int32},
	{"set_uint32", lc_cass_tuple_set_uint32},
	{"set_int64", lc_cass_tuple_set_int64},
	{"set_float", lc_cass_tuple_set_float},
	{"set_double", lc_cass_tuple_set_double},
	{"set_bool", lc_cass_tuple_set_bool},
	{"set_string", lc_cass_tuple_set_string},
	{"set_bytes", lc_cass_tuple_set_bytes},
	{"set_uuid", lc_cass_tuple_set_uuid},
	{"set_inet", lc_cass_tuple_set_inet},
	{"set_decimal", lc_cass_tuple_set_decimal},
	{"set_collection", lc_cass_tuple_set_collection},
	{"set_tuple", lc_cass_tuple_set_tuple},
	{"set_user_type", lc_cass_tuple_set_user_type},
	{NULL, NULL}
};

static const struct luaL_reg
user_type_methods[] = {
	{"data_type", lc_cass_user_type_data_type},
	{"set_null", lc_cass_user_type_set_null},
	{"set_null_by_name", lc_cass_user_type_set_null_by_name},
	{"set_int8", lc_cass_user_type_set_int8},
	{"set_int8_by_name", lc_cass_user_type_set_int8_by_name},
	{"set_int16", lc_cass_user_type_set_int16},
	{"set_int16_by_name", lc_cass_user_type_set_int16_by_name},
	{"set_int32", lc_cass_user_type_set_int32},
	{"set_int32_by_name", lc_cass_user_type_set_int32_by_name},
	{"set_uint32", lc_cass_user_type_set_uint32},
	{"set_uint32_by_name", lc_cass_user_type_set_uint32_by_name},
	{"set_int64", lc_cass_user_type_set_int64},
	{"set_int64_by_name", lc_cass_user_type_set_int64_by_name},
	{"set_float", lc_cass_user_type_set_float},
	{"set_float_by_name", lc_cass_user_type_set_float_by_name},
	{"set_double", lc_cass_user_type_set_double},
	{"set_double_by_name", lc_cass_user_type_set_double_by_name},
	{"set_bool", lc_cass_user_type_set_bool},
	{"set_bool_by_name", lc_cass_user_type_set_bool_by_name},
	{"set_string", lc_cass_user_type_set_string},
	{"set_string_by_name", lc_cass_user_type_set_string_by_name},
	{"set_bytes", lc_cass_user_type_set_bytes},
	{"set_bytes_by_name", lc_cass_user_type_set_bytes_by_name},
	{"set_uuid", lc_cass_user_type_set_uuid},
	{"set_uuid_by_name", lc_cass_user_type_set_uuid_by_name},
	{"set_inet", lc_cass_user_type_set_inet},
	{"set_inet_by_name", lc_cass_user_type_set_inet_by_name},
	{"set_decimal", lc_cass_user_type_set_decimal},
	{"set_decimal_by_name", lc_cass_user_type_set_decimal_by_name},
	{"set_collection", lc_cass_user_type_set_collection},
	{"set_collection_by_name", lc_cass_user_type_set_collection_by_name},
	{"set_tuple", lc_cass_user_type_set_tuple},
	{"set_tuple_by_name", lc_cass_user_type_set_tuple_by_name},
	{"set_user_type", lc_cass_user_type_set_user_type},
	{"set_user_type_by_name", lc_cass_user_type_set_user_type_by_name},
	{NULL, NULL}
};

static const struct luaL_reg
result_methods[] = {
	{"row_count", lc_cass_result_row_count},
	{"column_count", lc_cass_result_column_count},
	{"column_name", lc_cass_result_column_name},
	{"column_type", lc_cass_result_column_type},
	{"column_data_type", lc_cass_result_column_data_type},
	{"first_row", lc_cass_result_first_row},
	{"has_more_pages", lc_cass_result_has_more_pages},
	{"paging_state_token", lc_cass_result_paging_state_token},
	{"iterator", lc_cass_iterator_from_result},
	{NULL, NULL}
};

static const struct luaL_reg
error_result_methods[] = {
	{"code", lc_cass_error_result_code},
	{"consistency", lc_cass_error_result_consistency},
	/* {"responses_received", lc_cass_error_result_responses_received}, */
	/* {"responses_required", lc_cass_error_result_responses_required}, */
	{"num_failures", lc_cass_error_result_num_failures},
	{"data_present", lc_cass_error_result_data_present},
	{"write_type", lc_cass_error_result_write_type},
	{"keyspace", lc_cass_error_result_keyspace},
	{"table", lc_cass_error_result_table},
	{"function", lc_cass_error_result_function},
	{"num_arg_types", lc_cass_error_num_arg_types},
	{"arg_type", lc_cass_error_result_arg_type},
	{NULL, NULL}
};

static const struct luaL_reg
iterator_methods[] = {
	{"type", lc_cass_iterator_type},
	{"next", lc_cass_iterator_next},
	{"get_row", lc_cass_iterator_get_row},
	{"get_column", lc_cass_iterator_get_column},
	{"get_value", lc_cass_iterator_get_value},
	{"get_map_key", lc_cass_iterator_get_map_key},
	{"get_map_value", lc_cass_iterator_get_map_value},
	{"get_user_type_field_name", lc_cass_iterator_get_user_type_field_name},
	{"get_user_type_field_value", lc_cass_iterator_get_user_type_field_value},
	{"get_keyspace_meta", lc_cass_iterator_get_keyspace_meta},
	{"get_table_meta", lc_cass_iterator_get_table_meta},
	{"get_materialized_view_meta", lc_cass_iterator_get_materialized_view_meta},
	{"get_user_type", lc_cass_iterator_get_user_type},
	{"get_function_meta", lc_cass_iterator_get_function_meta},
	{"get_aggregate_meta", lc_cass_iterator_get_aggregate_meta},
	{"get_column_meta", lc_cass_iterator_get_column_meta},
	{"get_index_meta", lc_cass_iterator_get_index_meta},
	{"get_meta_field_name", lc_cass_iterator_get_meta_field_name},
	{"get_meta_field_value", lc_cass_iterator_get_meta_field_value},
	{NULL, NULL}
};

static const struct luaL_reg
row_methods[] = {
	{"get_column", lc_cass_row_get_column},
	{"get_column_by_name", lc_cass_row_get_column_by_name},
	{"iterator", lc_cass_iterator_from_row},
	{NULL, NULL}
};

static const struct luaL_reg
value_methods[] = {
	{"data_type", lc_cass_value_data_type},
	{"get_int8", lc_cass_value_get_int8},
	{"get_int16", lc_cass_value_get_int16},
	{"get_int32", lc_cass_value_get_int32},
	{"get_uint32", lc_cass_value_get_uint32},
	{"get_int64", lc_cass_value_get_int64},
	{"get_float", lc_cass_value_get_float},
	{"get_double", lc_cass_value_get_double},
	{"get_bool", lc_cass_value_get_bool},
	{"get_uuid", lc_cass_value_get_uuid},
	{"get_inet", lc_cass_value_get_inet},
	{"get_string", lc_cass_value_get_string},
	{"get_bytes", lc_cass_value_get_bytes},
	{"get_decimal", lc_cass_value_get_decimal},
	{"type", lc_cass_value_type},
	{"is_null", lc_cass_value_is_null},
	{"is_collection", lc_cass_value_is_collection},
	{"item_count", lc_cass_value_item_count},
	{"primary_sub_type", lc_cass_value_primary_sub_type},
	{"secondary_sub_type", lc_cass_value_secondary_sub_type},
	{"iterator_from_collection", lc_cass_iterator_from_collection},
	{"iterator_from_tuple", lc_cass_iterator_from_tuple},
	{"iterator_from_map", lc_cass_iterator_from_map},
	{"iterator_fields_from_user_type", lc_cass_iterator_fields_from_user_type},
	{NULL, NULL}
};

static const struct luaL_reg
uuid_gen_methods[] = {
	{"time", lc_cass_uuid_gen_time},
	{"random", lc_cass_uuid_gen_random},
	{"from_time", lc_cass_uuid_gen_from_time},
	{NULL, NULL}
};

static const struct luaL_reg
custom_payload_methods[] = {
	{"set", lc_cass_custom_payload_set},
	{NULL, NULL}
};

static int
read_only(lua_State *s) {
	return luaL_error(s, "%s\n", "Attempt to set a read-only value.");
}

static int
set_cl_constants(lua_State *s) {
	lua_newtable(s); /* proxy table */
	luaL_newmetatable(s, "datastax.cassandra.CL");

	lua_pushstring(s, "__newindex");
	lua_pushcfunction(s, &read_only);
	lua_settable(s, -3);

	lua_pushstring(s, "__metatable");
	lua_pushnil(s);
	lua_settable(s, -3);

	lua_pushstring(s, "__index");
	lua_newtable(s); /* real table */

	lua_pushstring(s, "UNKNOWN");
	lua_pushinteger(s, CASS_CONSISTENCY_UNKNOWN);
	lua_settable(s, -3);

	lua_pushstring(s, "ANY");
	lua_pushinteger(s, CASS_CONSISTENCY_ANY);
	lua_settable(s, -3);

	lua_pushstring(s, "ONE");
	lua_pushinteger(s, CASS_CONSISTENCY_ONE);
	lua_settable(s, -3);

	lua_pushstring(s, "TWO");
	lua_pushinteger(s, CASS_CONSISTENCY_TWO);
	lua_settable(s, -3);

	lua_pushstring(s, "THREE");
	lua_pushinteger(s, CASS_CONSISTENCY_THREE);
	lua_settable(s, -3);

	lua_pushstring(s, "QUORUM");
	lua_pushinteger(s, CASS_CONSISTENCY_QUORUM);
	lua_settable(s, -3);

	lua_pushstring(s, "ALL");
	lua_pushinteger(s, CASS_CONSISTENCY_ALL);
	lua_settable(s, -3);

	lua_pushstring(s, "LOCAL_QUORUM");
	lua_pushinteger(s, CASS_CONSISTENCY_LOCAL_QUORUM);
	lua_settable(s, -3);

	lua_pushstring(s, "EACH_QUORUM");
	lua_pushinteger(s, CASS_CONSISTENCY_EACH_QUORUM);
	lua_settable(s, -3);

	lua_pushstring(s, "SERIAL");
	lua_pushinteger(s, CASS_CONSISTENCY_SERIAL);
	lua_settable(s, -3);

	lua_pushstring(s, "LOCAL_SERIAL");
	lua_pushinteger(s, CASS_CONSISTENCY_LOCAL_SERIAL);
	lua_settable(s, -3);

	lua_pushstring(s, "LOCAL_ONE");
	lua_pushinteger(s, CASS_CONSISTENCY_LOCAL_ONE);
	lua_settable(s, -3);

	lua_settable(s, -3); /* metatable.__index = real-table */

	lua_setmetatable(s, -2); /* proxy-table.metatable = metatable */

	return 1;
}

static int
set_error_constants(lua_State *s) {
	lua_newtable(s); /* proxy table */
	luaL_newmetatable(s, "datastax.cassandra.ERRORS");

	lua_pushstring(s, "__newindex");
	lua_pushcfunction(s, &read_only);
	lua_settable(s, -3);

	lua_pushstring(s, "__metatable");
	lua_pushnil(s);
	lua_settable(s, -3);

	lua_pushstring(s, "__index");
	lua_newtable(s); /* real table */

	lua_pushstring(s, "LIB_BAD_PARAMS");
	lua_pushinteger(s, CASS_ERROR_LIB_BAD_PARAMS);
	lua_settable(s, -3);

	lua_pushstring(s, "LIB_NO_STREAMS");
	lua_pushinteger(s, CASS_ERROR_LIB_NO_STREAMS);
	lua_settable(s, -3);

	lua_pushstring(s, "LIB_UNABLE_TO_INIT");
	lua_pushinteger(s, CASS_ERROR_LIB_UNABLE_TO_INIT);
	lua_settable(s, -3);

	lua_pushstring(s, "LIB_MESSAGE_ENCODE");
	lua_pushinteger(s, CASS_ERROR_LIB_MESSAGE_ENCODE);
	lua_settable(s, -3);

	lua_pushstring(s, "LIB_HOST_RESOLUTION");
	lua_pushinteger(s, CASS_ERROR_LIB_HOST_RESOLUTION);
	lua_settable(s, -3);

	lua_pushstring(s, "LIB_UNEXPECTED_RESPONSE");
	lua_pushinteger(s, CASS_ERROR_LIB_UNEXPECTED_RESPONSE);
	lua_settable(s, -3);

	lua_pushstring(s, "LIB_REQUEST_QUEUE_FULL");
	lua_pushinteger(s, CASS_ERROR_LIB_REQUEST_QUEUE_FULL);
	lua_settable(s, -3);

	lua_pushstring(s, "LIB_NO_AVAILABLE_IO_THREAD");
	lua_pushinteger(s, CASS_ERROR_LIB_NO_AVAILABLE_IO_THREAD);
	lua_settable(s, -3);

	lua_pushstring(s, "LIB_WRITE_ERROR");
	lua_pushinteger(s, CASS_ERROR_LIB_WRITE_ERROR);
	lua_settable(s, -3);

	lua_pushstring(s, "LIB_NO_HOSTS_AVAILABLE");
	lua_pushinteger(s, CASS_ERROR_LIB_NO_HOSTS_AVAILABLE);
	lua_settable(s, -3);

	lua_pushstring(s, "LIB_INDEX_OUT_OF_BOUNDS");
	lua_pushinteger(s, CASS_ERROR_LIB_INDEX_OUT_OF_BOUNDS);
	lua_settable(s, -3);

	lua_pushstring(s, "LIB_INVALID_ITEM_COUNT");
	lua_pushinteger(s, CASS_ERROR_LIB_INVALID_ITEM_COUNT);
	lua_settable(s, -3);

	lua_pushstring(s, "LIB_INVALID_VALUE_TYPE");
	lua_pushinteger(s, CASS_ERROR_LIB_INVALID_VALUE_TYPE);
	lua_settable(s, -3);

	lua_pushstring(s, "LIB_REQUEST_TIMED_OUT");
	lua_pushinteger(s, CASS_ERROR_LIB_REQUEST_TIMED_OUT);
	lua_settable(s, -3);

	lua_pushstring(s, "LIB_UNABLE_TO_SET_KEYSPACE");
	lua_pushinteger(s, CASS_ERROR_LIB_UNABLE_TO_SET_KEYSPACE);
	lua_settable(s, -3);

	lua_pushstring(s, "LIB_CALLBACK_ALREADY_SET");
	lua_pushinteger(s, CASS_ERROR_LIB_CALLBACK_ALREADY_SET);
	lua_settable(s, -3);

	lua_pushstring(s, "LIB_INVALID_STATEMENT_TYPE");
	lua_pushinteger(s, CASS_ERROR_LIB_INVALID_STATEMENT_TYPE);
	lua_settable(s, -3);

	lua_pushstring(s, "LIB_NAME_DOES_NOT_EXIST");
	lua_pushinteger(s, CASS_ERROR_LIB_NAME_DOES_NOT_EXIST);
	lua_settable(s, -3);

	lua_pushstring(s, "LIB_UNABLE_TO_DETERMINE_PROTOCOL");
	lua_pushinteger(s, CASS_ERROR_LIB_UNABLE_TO_DETERMINE_PROTOCOL);
	lua_settable(s, -3);

	lua_pushstring(s, "LIB_NULL_VALUE");
	lua_pushinteger(s, CASS_ERROR_LIB_NULL_VALUE);
	lua_settable(s, -3);

	lua_pushstring(s, "LIB_NOT_IMPLEMENTED");
	lua_pushinteger(s, CASS_ERROR_LIB_NOT_IMPLEMENTED);
	lua_settable(s, -3);

	lua_pushstring(s, "LIB_UNABLE_TO_CONNECT");
	lua_pushinteger(s, CASS_ERROR_LIB_UNABLE_TO_CONNECT);
	lua_settable(s, -3);

	lua_pushstring(s, "LIB_UNABLE_TO_CLOSE");
	lua_pushinteger(s, CASS_ERROR_LIB_UNABLE_TO_CLOSE);
	lua_settable(s, -3);

	lua_pushstring(s, "LIB_NO_PAGING_STATE");
	lua_pushinteger(s, CASS_ERROR_LIB_NO_PAGING_STATE);
	lua_settable(s, -3);

	lua_pushstring(s, "LIB_PARAMETER_UNSET");
	lua_pushinteger(s, CASS_ERROR_LIB_PARAMETER_UNSET);
	lua_settable(s, -3);

	lua_pushstring(s, "LIB_INVALID_ERROR_RESULT_TYPE");
	lua_pushinteger(s, CASS_ERROR_LIB_INVALID_ERROR_RESULT_TYPE);
	lua_settable(s, -3);

	lua_pushstring(s, "LIB_INVALID_FUTURE_TYPE");
	lua_pushinteger(s, CASS_ERROR_LIB_INVALID_FUTURE_TYPE);
	lua_settable(s, -3);

	lua_pushstring(s, "SERVER_SERVER_ERROR");
	lua_pushinteger(s, CASS_ERROR_SERVER_SERVER_ERROR);
	lua_settable(s, -3);

	lua_pushstring(s, "SERVER_PROTOCOL_ERROR");
	lua_pushinteger(s, CASS_ERROR_SERVER_PROTOCOL_ERROR);
	lua_settable(s, -3);

	lua_pushstring(s, "SERVER_BAD_CREDENTIALS");
	lua_pushinteger(s, CASS_ERROR_SERVER_BAD_CREDENTIALS);
	lua_settable(s, -3);

	lua_pushstring(s, "SERVER_UNAVAILABLE");
	lua_pushinteger(s, CASS_ERROR_SERVER_UNAVAILABLE);
	lua_settable(s, -3);

	lua_pushstring(s, "SERVER_OVERLOADED");
	lua_pushinteger(s, CASS_ERROR_SERVER_OVERLOADED);
	lua_settable(s, -3);

	lua_pushstring(s, "SERVER_IS_BOOTSTRAPPING");
	lua_pushinteger(s, CASS_ERROR_SERVER_IS_BOOTSTRAPPING);
	lua_settable(s, -3);

	lua_pushstring(s, "SERVER_TRUNCATE_ERROR");
	lua_pushinteger(s, CASS_ERROR_SERVER_TRUNCATE_ERROR);
	lua_settable(s, -3);

	lua_pushstring(s, "SERVER_WRITE_TIMEOUT");
	lua_pushinteger(s, CASS_ERROR_SERVER_WRITE_TIMEOUT);
	lua_settable(s, -3);

	lua_pushstring(s, "SERVER_READ_TIMEOUT");
	lua_pushinteger(s, CASS_ERROR_SERVER_READ_TIMEOUT);
	lua_settable(s, -3);

	lua_pushstring(s, "SERVER_READ_FAILURE");
	lua_pushinteger(s, CASS_ERROR_SERVER_READ_FAILURE);
	lua_settable(s, -3);

	lua_pushstring(s, "SERVER_FUNCTION_FAILURE");
	lua_pushinteger(s, CASS_ERROR_SERVER_FUNCTION_FAILURE);
	lua_settable(s, -3);

	lua_pushstring(s, "SERVER_WRITE_FAILURE");
	lua_pushinteger(s, CASS_ERROR_SERVER_WRITE_FAILURE);
	lua_settable(s, -3);

	lua_pushstring(s, "SERVER_SYNTAX_ERROR");
	lua_pushinteger(s, CASS_ERROR_SERVER_SYNTAX_ERROR);
	lua_settable(s, -3);

	lua_pushstring(s, "SERVER_UNAUTHORIZED");
	lua_pushinteger(s, CASS_ERROR_SERVER_UNAUTHORIZED);
	lua_settable(s, -3);

	lua_pushstring(s, "SERVER_INVALID_QUERY");
	lua_pushinteger(s, CASS_ERROR_SERVER_INVALID_QUERY);
	lua_settable(s, -3);

	lua_pushstring(s, "SERVER_CONFIG_ERROR");
	lua_pushinteger(s, CASS_ERROR_SERVER_CONFIG_ERROR);
	lua_settable(s, -3);

	lua_pushstring(s, "SERVER_ALREADY_EXISTS");
	lua_pushinteger(s, CASS_ERROR_SERVER_ALREADY_EXISTS);
	lua_settable(s, -3);

	lua_pushstring(s, "SERVER_UNPREPARED");
	lua_pushinteger(s, CASS_ERROR_SERVER_UNPREPARED);
	lua_settable(s, -3);

	lua_pushstring(s, "SSL_INVALID_CERT");
	lua_pushinteger(s, CASS_ERROR_SSL_INVALID_CERT);
	lua_settable(s, -3);

	lua_pushstring(s, "SSL_INVALID_PRIVATE_KEY");
	lua_pushinteger(s, CASS_ERROR_SSL_INVALID_PRIVATE_KEY);
	lua_settable(s, -3);

	lua_pushstring(s, "SSL_NO_PEER_CERT");
	lua_pushinteger(s, CASS_ERROR_SSL_NO_PEER_CERT);
	lua_settable(s, -3);

	lua_pushstring(s, "SSL_INVALID_PEER_CERT");
	lua_pushinteger(s, CASS_ERROR_SSL_INVALID_PEER_CERT);
	lua_settable(s, -3);

	lua_pushstring(s, "SSL_IDENTITY_MISMATCH");
	lua_pushinteger(s, CASS_ERROR_SSL_IDENTITY_MISMATCH);
	lua_settable(s, -3);

	lua_pushstring(s, "SSL_PROTOCOL_ERROR");
	lua_pushinteger(s, CASS_ERROR_SSL_PROTOCOL_ERROR);
	lua_settable(s, -3);

	lua_settable(s, -3); /* metatable.__index = real-table */

	lua_setmetatable(s, -2); /* proxy-table.metatable = metatable */

	return 1;
}

static int
set_type_constants(lua_State *s) {
	lua_newtable(s); /* proxy table */
	luaL_newmetatable(s, "datastax.cassandra.TYPES");

	lua_pushstring(s, "__newindex");
	lua_pushcfunction(s, &read_only);
	lua_settable(s, -3);

	lua_pushstring(s, "__metatable");
	lua_pushnil(s);
	lua_settable(s, -3);

	lua_pushstring(s, "__index");
	lua_newtable(s); /* real table */

	lua_pushstring(s, "VALUE_TYPE_UNKNOWN");
	lua_pushinteger(s, CASS_VALUE_TYPE_UNKNOWN);
	lua_settable(s, -3);

	lua_pushstring(s, "VALUE_TYPE_CUSTOM");
	lua_pushinteger(s, CASS_VALUE_TYPE_CUSTOM);
	lua_settable(s, -3);

	lua_pushstring(s, "VALUE_TYPE_ASCII");
	lua_pushinteger(s, CASS_VALUE_TYPE_ASCII);
	lua_settable(s, -3);

	lua_pushstring(s, "VALUE_TYPE_BIGINT");
	lua_pushinteger(s, CASS_VALUE_TYPE_BIGINT);
	lua_settable(s, -3);

	lua_pushstring(s, "VALUE_TYPE_BLOB");
	lua_pushinteger(s, CASS_VALUE_TYPE_BLOB);
	lua_settable(s, -3);

	lua_pushstring(s, "VALUE_TYPE_BOOLEAN");
	lua_pushinteger(s, CASS_VALUE_TYPE_BOOLEAN);
	lua_settable(s, -3);

	lua_pushstring(s, "VALUE_TYPE_COUNTER");
	lua_pushinteger(s, CASS_VALUE_TYPE_COUNTER);
	lua_settable(s, -3);

	lua_pushstring(s, "VALUE_TYPE_DECIMAL");
	lua_pushinteger(s, CASS_VALUE_TYPE_DECIMAL);
	lua_settable(s, -3);

	lua_pushstring(s, "VALUE_TYPE_DOUBLE");
	lua_pushinteger(s, CASS_VALUE_TYPE_DOUBLE);
	lua_settable(s, -3);

	lua_pushstring(s, "VALUE_TYPE_FLOAT");
	lua_pushinteger(s, CASS_VALUE_TYPE_FLOAT);
	lua_settable(s, -3);

	lua_pushstring(s, "VALUE_TYPE_INT");
	lua_pushinteger(s, CASS_VALUE_TYPE_INT);
	lua_settable(s, -3);

	lua_pushstring(s, "VALUE_TYPE_TEXT");
	lua_pushinteger(s, CASS_VALUE_TYPE_TEXT);
	lua_settable(s, -3);

	lua_pushstring(s, "VALUE_TYPE_TIMESTAMP");
	lua_pushinteger(s, CASS_VALUE_TYPE_TIMESTAMP);
	lua_settable(s, -3);

	lua_pushstring(s, "VALUE_TYPE_UUID");
	lua_pushinteger(s, CASS_VALUE_TYPE_UUID);
	lua_settable(s, -3);

	lua_pushstring(s, "VALUE_TYPE_VARCHAR");
	lua_pushinteger(s, CASS_VALUE_TYPE_VARCHAR);
	lua_settable(s, -3);

	lua_pushstring(s, "VALUE_TYPE_VARINT");
	lua_pushinteger(s, CASS_VALUE_TYPE_VARINT);
	lua_settable(s, -3);

	lua_pushstring(s, "VALUE_TYPE_TIMEUUID");
	lua_pushinteger(s, CASS_VALUE_TYPE_TIMEUUID);
	lua_settable(s, -3);

	lua_pushstring(s, "VALUE_TYPE_INET");
	lua_pushinteger(s, CASS_VALUE_TYPE_INET);
	lua_settable(s, -3);

	lua_pushstring(s, "VALUE_TYPE_DATE");
	lua_pushinteger(s, CASS_VALUE_TYPE_DATE);
	lua_settable(s, -3);

	lua_pushstring(s, "VALUE_TYPE_TIME");
	lua_pushinteger(s, CASS_VALUE_TYPE_TIME);
	lua_settable(s, -3);

	lua_pushstring(s, "VALUE_TYPE_SMALL_INT");
	lua_pushinteger(s, CASS_VALUE_TYPE_SMALL_INT);
	lua_settable(s, -3);

	lua_pushstring(s, "VALUE_TYPE_TINY_INT");
	lua_pushinteger(s, CASS_VALUE_TYPE_TINY_INT);
	lua_settable(s, -3);

	lua_pushstring(s, "VALUE_TYPE_LIST");
	lua_pushinteger(s, CASS_VALUE_TYPE_LIST);
	lua_settable(s, -3);

	lua_pushstring(s, "VALUE_TYPE_MAP");
	lua_pushinteger(s, CASS_VALUE_TYPE_MAP);
	lua_settable(s, -3);

	lua_pushstring(s, "VALUE_TYPE_SET");
	lua_pushinteger(s, CASS_VALUE_TYPE_SET);
	lua_settable(s, -3);

	lua_pushstring(s, "VALUE_TYPE_UDT");
	lua_pushinteger(s, CASS_VALUE_TYPE_UDT);
	lua_settable(s, -3);

	lua_pushstring(s, "VALUE_TYPE_TUPLE");
	lua_pushinteger(s, CASS_VALUE_TYPE_TUPLE);
	lua_settable(s, -3);

	lua_pushstring(s, "INDEX_TYPE_UNKNOWN");
	lua_pushinteger(s, CASS_INDEX_TYPE_UNKNOWN);
	lua_settable(s, -3);

	lua_pushstring(s, "INDEX_TYPE_KEYS");
	lua_pushinteger(s, CASS_INDEX_TYPE_KEYS);
	lua_settable(s, -3);

	lua_pushstring(s, "INDEX_TYPE_CUSTOM");
	lua_pushinteger(s, CASS_INDEX_TYPE_CUSTOM);
	lua_settable(s, -3);

	lua_pushstring(s, "INDEX_TYPE_COMPOSITES");
	lua_pushinteger(s, CASS_INDEX_TYPE_COMPOSITES);
	lua_settable(s, -3);

	lua_pushstring(s, "COLUMN_TYPE_REGULAR");
	lua_pushinteger(s, CASS_COLUMN_TYPE_REGULAR);
	lua_settable(s, -3);

	lua_pushstring(s, "COLUMN_TYPE_PARTITION_KEY");
	lua_pushinteger(s, CASS_COLUMN_TYPE_PARTITION_KEY);
	lua_settable(s, -3);

	lua_pushstring(s, "COLUMN_TYPE_CLUSTERING_KEY");
	lua_pushinteger(s, CASS_COLUMN_TYPE_CLUSTERING_KEY);
	lua_settable(s, -3);

	lua_pushstring(s, "COLUMN_TYPE_STATIC");
	lua_pushinteger(s, CASS_COLUMN_TYPE_STATIC);
	lua_settable(s, -3);

	lua_pushstring(s, "COLUMN_TYPE_COMPACT_VALUE");
	lua_pushinteger(s, CASS_COLUMN_TYPE_COMPACT_VALUE);
	lua_settable(s, -3);

	lua_pushstring(s, "VALUE_TYPE_LIST");
	lua_pushinteger(s, CASS_VALUE_TYPE_LIST);
	lua_settable(s, -3);

	lua_pushstring(s, "VALUE_TYPE_MAP");
	lua_pushinteger(s, CASS_VALUE_TYPE_MAP);
	lua_settable(s, -3);

	lua_pushstring(s, "VALUE_TYPE_SET");
	lua_pushinteger(s, CASS_VALUE_TYPE_SET);
	lua_settable(s, -3);

	lua_pushstring(s, "BATCH_TYPE_LOGGED");
	lua_pushinteger(s, CASS_BATCH_TYPE_LOGGED);
	lua_settable(s, -3);

	lua_pushstring(s, "BATCH_TYPE_UNLOGGED");
	lua_pushinteger(s, CASS_BATCH_TYPE_UNLOGGED);
	lua_settable(s, -3);

	lua_pushstring(s, "BATCH_TYPE_COUNTER");
	lua_pushinteger(s, CASS_BATCH_TYPE_COUNTER);
	lua_settable(s, -3);

	lua_pushstring(s, "ITERATOR_TYPE_RESULT");
	lua_pushinteger(s, CASS_ITERATOR_TYPE_RESULT);
	lua_settable(s, -3);

	lua_pushstring(s, "ITERATOR_TYPE_ROW");
	lua_pushinteger(s, CASS_ITERATOR_TYPE_ROW);
	lua_settable(s, -3);

	lua_pushstring(s, "ITERATOR_TYPE_COLLECTION");
	lua_pushinteger(s, CASS_ITERATOR_TYPE_COLLECTION);
	lua_settable(s, -3);

	lua_pushstring(s, "ITERATOR_TYPE_MAP");
	lua_pushinteger(s, CASS_ITERATOR_TYPE_MAP);
	lua_settable(s, -3);

	lua_pushstring(s, "ITERATOR_TYPE_TUPLE");
	lua_pushinteger(s, CASS_ITERATOR_TYPE_TUPLE);
	lua_settable(s, -3);

	lua_pushstring(s, "ITERATOR_TYPE_USER_TYPE_FIELD");
	lua_pushinteger(s, CASS_ITERATOR_TYPE_USER_TYPE_FIELD);
	lua_settable(s, -3);

	lua_pushstring(s, "ITERATOR_TYPE_META_FIELD");
	lua_pushinteger(s, CASS_ITERATOR_TYPE_META_FIELD);
	lua_settable(s, -3);

	lua_pushstring(s, "ITERATOR_TYPE_KEYSPACE_META");
	lua_pushinteger(s, CASS_ITERATOR_TYPE_KEYSPACE_META);
	lua_settable(s, -3);

	lua_pushstring(s, "ITERATOR_TYPE_TABLE_META");
	lua_pushinteger(s, CASS_ITERATOR_TYPE_TABLE_META);
	lua_settable(s, -3);

	lua_pushstring(s, "ITERATOR_TYPE_TYPE_META");
	lua_pushinteger(s, CASS_ITERATOR_TYPE_TYPE_META);
	lua_settable(s, -3);

	lua_pushstring(s, "ITERATOR_TYPE_FUNCTION_META");
	lua_pushinteger(s, CASS_ITERATOR_TYPE_FUNCTION_META);
	lua_settable(s, -3);

	lua_pushstring(s, "ITERATOR_TYPE_AGGREGATE_META");
	lua_pushinteger(s, CASS_ITERATOR_TYPE_AGGREGATE_META);
	lua_settable(s, -3);

	lua_pushstring(s, "ITERATOR_TYPE_COLUMN_META");
	lua_pushinteger(s, CASS_ITERATOR_TYPE_COLUMN_META);
	lua_settable(s, -3);

	lua_pushstring(s, "ITERATOR_TYPE_INDEX_META");
	lua_pushinteger(s, CASS_ITERATOR_TYPE_INDEX_META);
	lua_settable(s, -3);

	lua_pushstring(s, "ITERATOR_TYPE_MATERIALIZED_VIEW_META");
	lua_pushinteger(s, CASS_ITERATOR_TYPE_MATERIALIZED_VIEW_META);
	lua_settable(s, -3);

	lua_pushstring(s, "WRITE_TYPE_UKNOWN");
	lua_pushinteger(s, CASS_WRITE_TYPE_UKNOWN);
	lua_settable(s, -3);

	lua_pushstring(s, "WRITE_TYPE_SIMPLE");
	lua_pushinteger(s, CASS_WRITE_TYPE_SIMPLE);
	lua_settable(s, -3);

	lua_pushstring(s, "WRITE_TYPE_BATCH");
	lua_pushinteger(s, CASS_WRITE_TYPE_BATCH);
	lua_settable(s, -3);

	lua_pushstring(s, "WRITE_TYPE_UNLOGGED_BATCH");
	lua_pushinteger(s, CASS_WRITE_TYPE_UNLOGGED_BATCH);
	lua_settable(s, -3);

	lua_pushstring(s, "WRITE_TYPE_COUNTER");
	lua_pushinteger(s, CASS_WRITE_TYPE_COUNTER);
	lua_settable(s, -3);

	lua_pushstring(s, "WRITE_TYPE_BATCH_LOG");
	lua_pushinteger(s, CASS_WRITE_TYPE_BATCH_LOG);
	lua_settable(s, -3);

	lua_pushstring(s, "WRITE_TYPE_CAS");
	lua_pushinteger(s, CASS_WRITE_TYPE_CAS);
	lua_settable(s, -3);

	lua_pushstring(s, "CLUSTERING_ORDER_NONE");
	lua_pushinteger(s, CASS_CLUSTERING_ORDER_NONE);
	lua_settable(s, -3);

	lua_pushstring(s, "CLUSTERING_ORDER_ASC");
	lua_pushinteger(s, CASS_CLUSTERING_ORDER_ASC);
	lua_settable(s, -3);

	lua_pushstring(s, "CLUSTERING_ORDER_DESC");
	lua_pushinteger(s, CASS_CLUSTERING_ORDER_DESC);
	lua_settable(s, -3);

	lua_settable(s, -3); /* metatable.__index = real-table */

	lua_setmetatable(s, -2); /* proxy-table.metatable = metatable */

	return 1;
}

int
luaopen_db_cassandra(lua_State *s) {
	lua_checkstack(s, 100);

	luaL_newmetatable(s, "datastax.cass_batch");
 	lua_pushstring(s, "__gc");
	lua_pushcfunction(s, lc_cass_batch_free);
	lua_settable(s, -3);
	lua_pushstring(s, "__index"); /* mt.__index = mt */
	lua_pushvalue(s, -2);
	lua_settable(s, -3);
	luaL_openlib(s, NULL, batch_methods, 0);

	luaL_newmetatable(s, "datastax.cass_cluster");
 	lua_pushstring(s, "__gc");
	lua_pushcfunction(s, lc_cass_cluster_free);
	lua_settable(s, -3);
	lua_pushstring(s, "__index");
	lua_pushvalue(s, -2);
	lua_settable(s, -3);
	luaL_openlib(s, NULL, cluster_methods, 0);

	luaL_newmetatable(s, "datastax.cass_collection");
 	lua_pushstring(s, "__gc");
	lua_pushcfunction(s, lc_cass_collection_free);
	lua_settable(s, -3);
	lua_pushstring(s, "__index");
	lua_pushvalue(s, -2);
	lua_settable(s, -3);
	luaL_openlib(s, NULL, collection_methods, 0);

	luaL_newmetatable(s, "datastax.cass_custom_payload");
 	lua_pushstring(s, "__gc");
	lua_pushcfunction(s, lc_cass_custom_payload_free);
	lua_settable(s, -3);
	lua_pushstring(s, "__index");
	lua_pushvalue(s, -2);
	lua_settable(s, -3);
	luaL_openlib(s, NULL, custom_payload_methods, 0);

	luaL_newmetatable(s, "datastax.cass_data_type");
 	lua_pushstring(s, "__gc");
	lua_pushcfunction(s, lc_cass_data_type_free);
	lua_settable(s, -3);
	lua_pushstring(s, "__index");
	lua_pushvalue(s, -2);
	lua_settable(s, -3);
	luaL_openlib(s, NULL, data_type_methods, 0);

	luaL_newmetatable(s, "datastax.cass_error_result");
 	lua_pushstring(s, "__gc");
	lua_pushcfunction(s, lc_cass_error_result_free);
	lua_settable(s, -3);
	lua_pushstring(s, "__index");
	lua_pushvalue(s, -2);
	lua_settable(s, -3);
	luaL_openlib(s, NULL, error_result_methods, 0);

	luaL_newmetatable(s, "datastax.cass_future");
	lua_pushstring(s, "__gc");
	lua_pushcfunction(s, lc_cass_future_free);
	lua_settable(s, -3);
	lua_pushstring(s, "__index");
	lua_pushvalue(s, -2);
	lua_settable(s, -3);
	luaL_openlib(s, NULL, future_methods, 0);

	luaL_newmetatable(s, "datastax.cass_iterator");
	lua_pushstring(s, "__gc");
	lua_pushcfunction(s, lc_cass_iterator_free);
	lua_settable(s, -3);
	lua_pushstring(s, "__index");
	lua_pushvalue(s, -2);
	lua_settable(s, -3);
	luaL_openlib(s, NULL, iterator_methods, 0);

	luaL_newmetatable(s, "datastax.cass_prepared");
	lua_pushstring(s, "__gc");
	lua_pushcfunction(s, lc_cass_prepared_free);
	lua_settable(s, -3);
	lua_pushstring(s, "__index");
	lua_pushvalue(s, -2);
	lua_settable(s, -3);
	luaL_openlib(s, NULL, prepared_methods, 0);

	luaL_newmetatable(s, "datastax.cass_result");
 	lua_pushstring(s, "__gc");
	lua_pushcfunction(s, lc_cass_result_free);
	lua_settable(s, -3);
	lua_pushstring(s, "__index");
	lua_pushvalue(s, -2);
	lua_settable(s, -3);
	luaL_openlib(s, NULL, result_methods, 0);

	luaL_newmetatable(s, "datastax.cass_retry_policy");
 	lua_pushstring(s, "__gc");
	lua_pushcfunction(s, lc_cass_retry_policy_free);
	lua_settable(s, -3);

	luaL_newmetatable(s, "datastax.cass_schema_meta");
	lua_pushstring(s, "__gc");
	lua_pushcfunction(s, lc_cass_schema_meta_free);
	lua_settable(s, -3);
	lua_pushstring(s, "__index");
	lua_pushvalue(s, -2);
	lua_settable(s, -3);
	luaL_openlib(s, NULL, schema_meta_methods, 0);

	luaL_newmetatable(s, "datastax.cass_session");
 	lua_pushstring(s, "__gc");
	lua_pushcfunction(s, lc_cass_session_free);
	lua_settable(s, -3);
	lua_pushstring(s, "__index");
	lua_pushvalue(s, -2);
	lua_settable(s, -3);
	luaL_openlib(s, NULL, session_methods, 0);

	luaL_newmetatable(s, "datastax.cass_ssl");
	lua_pushstring(s, "__gc");
	lua_pushcfunction(s, lc_cass_ssl_free);
	lua_settable(s, -3);
	lua_pushstring(s, "__index");
	lua_pushvalue(s, -2);
	lua_settable(s, -3);
	luaL_openlib(s, NULL, ssl_methods, 0);

	luaL_newmetatable(s, "datastax.cass_statement");
	lua_pushstring(s, "__gc");
	lua_pushcfunction(s, lc_cass_statement_free);
	lua_settable(s, -3);
	lua_pushstring(s, "__index");
	lua_pushvalue(s, -2);
	lua_settable(s, -3);
	luaL_openlib(s, NULL, statement_methods, 0);

	luaL_newmetatable(s, "datastax.cass_timestamp_gen");
	lua_pushstring(s, "__gc");
	lua_pushcfunction(s, lc_cass_timestamp_gen_free);
	lua_settable(s, -3);

	luaL_newmetatable(s, "datastax.cass_tuple");
	lua_pushstring(s, "__gc");
	lua_pushcfunction(s, lc_cass_tuple_free);
	lua_settable(s, -3);
	lua_pushstring(s, "__index");
	lua_pushvalue(s, -2);
	lua_settable(s, -3);
	luaL_openlib(s, NULL, tuple_methods, 0);

	luaL_newmetatable(s, "datastax.cass_user_type");
	lua_pushstring(s, "__gc");
	lua_pushcfunction(s, lc_cass_user_type_free);
	lua_settable(s, -3);
	lua_pushstring(s, "__index");
	lua_pushvalue(s, -2);
	lua_settable(s, -3);
	luaL_openlib(s, NULL, user_type_methods, 0);

	luaL_newmetatable(s, "datastax.cass_uuid_gen");
	lua_pushstring(s, "__gc");
	lua_pushcfunction(s, lc_cass_uuid_gen_free);
	lua_settable(s, -3);
	lua_pushstring(s, "__index");
	lua_pushvalue(s, -2);
	lua_settable(s, -3);
	luaL_openlib(s, NULL, uuid_gen_methods, 0);

	luaL_newmetatable(s, "datastax.cass_aggregate_meta");
	lua_pushstring(s, "__index");
	lua_pushvalue(s, -2);
	lua_settable(s, -3);
	luaL_openlib(s, NULL, aggregate_meta_methods, 0);

	luaL_newmetatable(s, "datastax.cass_column_meta");
	lua_pushstring(s, "__index");
	lua_pushvalue(s, -2);
	lua_settable(s, -3);
	luaL_openlib(s, NULL, column_meta_methods, 0);

	luaL_newmetatable(s, "datastax.cass_function_meta");
	lua_pushstring(s, "__index");
	lua_pushvalue(s, -2);
	lua_settable(s, -3);
	luaL_openlib(s, NULL, function_meta_methods, 0);

	luaL_newmetatable(s, "datastax.cass_index_meta");
	lua_pushstring(s, "__index");
	lua_pushvalue(s, -2);
	lua_settable(s, -3);
	luaL_openlib(s, NULL, index_meta_methods, 0);

	luaL_newmetatable(s, "datastax.cass_keyspace_meta");
	lua_pushstring(s, "__index");
	lua_pushvalue(s, -2);
	lua_settable(s, -3);
	luaL_openlib(s, NULL, keyspace_meta_methods, 0);

	luaL_newmetatable(s, "datastax.cass_materialized_view_meta");
	lua_pushstring(s, "__index");
	lua_pushvalue(s, -2);
	lua_settable(s, -3);
	luaL_openlib(s, NULL, materialized_view_meta_methods, 0);

	luaL_newmetatable(s, "datastax.cass_row");
	lua_pushstring(s, "__index");
	lua_pushvalue(s, -2);
	lua_settable(s, -3);
	luaL_openlib(s, NULL, row_methods, 0);

	luaL_newmetatable(s, "datastax.cass_table_meta");
	lua_pushstring(s, "__index");
	lua_pushvalue(s, -2);
	lua_settable(s, -3);
	luaL_openlib(s, NULL, table_meta_methods, 0);

	luaL_newmetatable(s, "datastax.cass_value");
	lua_pushstring(s, "__index");
	lua_pushvalue(s, -2);
	lua_settable(s, -3);
	luaL_openlib(s, NULL, value_methods, 0);

	luaL_openlib(s, "db_cassandra", functions, 0);

	/* constants */

	/* consistency levels */
	lua_pushstring(s, "CL");
	set_cl_constants(s);
	lua_settable(s, -3);

	/* errors */
	lua_pushstring(s, "ERRORS");
	set_error_constants(s);
	lua_settable(s, -3);

	/* types */
	lua_pushstring(s, "TYPES");
	set_type_constants(s);
	lua_settable(s, -3);

	return 1;
}
