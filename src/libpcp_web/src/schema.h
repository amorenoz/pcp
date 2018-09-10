/*
 * Copyright (c) 2017-2018 Red Hat.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */
#ifndef SERIES_SCHEMA_H
#define SERIES_SCHEMA_H

#include "load.h"
#include "redis.h"
#include "private.h"
#include "slots.h"
#include "query.h"

#define COMMAND		"COMMAND"
#define COMMAND_LEN	(sizeof(COMMAND)-1)
#define CLUSTER		"CLUSTER"
#define CLUSTER_LEN	(sizeof(CLUSTER)-1)
#define EVALSHA		"EVALSHA"
#define EVALSHA_LEN	(sizeof(EVALSHA)-1)
#define GEOADD		"GEOADD"
#define GEOADD_LEN	(sizeof(GEOADD)-1)
#define GETS		"GET"
#define GETS_LEN	(sizeof(GETS)-1)
#define HGET		"HGET"
#define HGET_LEN	(sizeof(HGET)-1)
#define HGETALL		"HGETALL"
#define HGETALL_LEN	(sizeof(HGETALL)-1)
#define HKEYS		"HKEYS"
#define HKEYS_LEN	(sizeof(HKEYS)-1)
#define HMGET		"HMGET"
#define HMGET_LEN	(sizeof(HMGET)-1)
#define HMSET		"HMSET"
#define HMSET_LEN	(sizeof(HMSET)-1)
#define HSCAN		"HSCAN"
#define HSCAN_LEN	(sizeof(HSCAN)-1)
#define PUBLISH		"PUBLISH"
#define PUBLISH_LEN	(sizeof(PUBLISH)-1)
#define SADD		"SADD"
#define SADD_LEN	(sizeof(SADD)-1)
#define SETS		"SET"
#define SETS_LEN	(sizeof(SETS)-1)
#define SMEMBERS	"SMEMBERS"
#define SMEMBERS_LEN	(sizeof(SMEMBERS)-1)
#define XADD		"XADD"
#define XADD_LEN	(sizeof(XADD)-1)
#define XRANGE		"XRANGE"
#define XRANGE_LEN	(sizeof(XRANGE)-1)

/* create a Redis protocol command (e.g. XADD, SMEMBER) */
static inline sds
redis_command(unsigned int count)
{
    return sdscatfmt(sdsempty(), "*%u\r\n", count);
}

/* append a string parameter to a Redis protocol command */
static inline sds
redis_param_str(sds cmd, const char *param, unsigned int length)
{
    return sdscatfmt(cmd, "$%u\r\n%s\r\n", length, param);
}

/* append a SHA1 hash parameter to a Redis protocol command */
static inline sds
redis_param_sha(sds cmd, const unsigned char *sha)
{
    int offset = sdslen(cmd);
    cmd = sdscatfmt(cmd, "$20\r\n01234567890123456789\r\n");
    memcpy(cmd + offset + 5, sha, 20);
    return cmd;
}

/* append an sds parameter to a Redis protocol command */
static inline sds
redis_param_sds(sds cmd, sds param)
{
    return sdscatfmt(cmd, "$%u\r\n%S\r\n", sdslen(param), param);
}

/* directly append (wire-format) to Redis protocol command */
static inline sds
redis_param_raw(sds cmd, sds param)
{
    return sdscatfmt(cmd, "%S\r\n", param);
}

extern void redis_series_source(redisSlots *, void *);
extern void redis_series_mark(redisSlots *, sds, int, void *);
extern void redis_series_metric(redisSlots *, metric_t *, sds, int, int, void *);

/*
 * Asynchronous schema load baton structures
 */
#define LOAD_PHASES	5

typedef struct seriesGetContext {
    seriesBatonMagic	header;		/* MAGIC_CONTEXT */

    context_t		context;
    unsigned long long	count;		/* number of samples processed */
    pmResult		*result;	/* currently active sample data */
    int			error;		/* PMAPI error code from fetch */

    redisDoneCallBack	done;

    void		*baton;
} seriesGetContext;

typedef struct seriesLoadBaton {
    seriesBatonMagic	header;		/* MAGIC_LOAD */

    seriesBatonPhase	*current;
    seriesBatonPhase	phases[LOAD_PHASES];

    seriesGetContext	pmapi;		/* PMAPI context info */
    redisSlots		*slots;		/* Redis server slots */

    pmSeriesSettings	*settings;
    void		*userdata;
    timing_t		timing;
    pmflags		flags;

    dict		*clusters;
    dict		*domains;
    dict		*indoms;
    dict		*pmids;

    dict		*errors;	/* PMIDs where errors observed */
    dict		*wanted;	/* PMIDs from query whitelist */

    int			error;
    void		*arg;
} seriesLoadBaton;

#endif	/* SERIES_SCHEMA_H */
