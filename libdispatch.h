#ifndef __HIREDIS_LIBDISPATCH_H__
#define __HIREDIS_LIBDISPATCH_H__

#include <dispatch/dispatch.h>

#include "../hiredis.h"
#include "../async.h"


typedef struct redisLibdispatchEvents {
    redisAsyncContext *context;
    int reading, writing;
    dispatch_source_t rev, wev;
} redisLibdispatchEvents;


static void redisLibdispatchAddRead(void *privdata) {
    redisLibdispatchEvents *e = (redisLibdispatchEvents*)privdata;
    if (!e->reading) {
        e->reading = 1;
        dispatch_resume(e->rev);
    }
}

static void redisLibdispatchDelRead(void *privdata) {
    redisLibdispatchEvents *e = (redisLibdispatchEvents*)privdata;
    if (!e->reading) {
        e->reading = 0;
        dispatch_suspend(e->rev);
    }
}

static void redisLibdispatchAddWrite(void *privdata) {
    redisLibdispatchEvents *e = (redisLibdispatchEvents*)privdata;
    if (!e->writing) {
        e->writing = 1;
        dispatch_resume(e->wev);
    }
}

static void redisLibdispatchDelWrite(void *privdata) {
    redisLibdispatchEvents *e = (redisLibdispatchEvents*)privdata;
    if (e->writing) {
        e->writing = 0;
        dispatch_suspend(e->wev);
    }
}

static void redisLibdispatchCleanup(void *privdata) {
  redisLibdispatchEvents *e = (redisLibdispatchEvents*)privdata;

  if (e->rev != NULL && dispatch_source_testcancel(e->rev) == 0) {
    redisLibdispatchAddRead(privdata);
    dispatch_source_cancel(e->rev);
    dispatch_release(e->rev);
  }

  if (e->wev != NULL && dispatch_source_testcancel(e->wev) == 0) {
    redisLibdispatchAddWrite(privdata);
    dispatch_source_cancel(e->wev);
    dispatch_release(e->wev);
  }

  free(e);
}

static int redisLibdispatchAttach(redisAsyncContext *ac, dispatch_queue_t queue) {
    redisContext *c = &(ac->c);
    redisLibdispatchEvents *e;

    // Nothing should be attached when something is already attached
    if (ac->ev.data != NULL)
        return REDIS_ERR;

    // Initialize and install read/write events
    dispatch_source_t rev = dispatch_source_create(DISPATCH_SOURCE_TYPE_READ,
                                                   c->fd, 0, queue);
    if (rev == NULL)
        return REDIS_ERR_IO;
    dispatch_source_set_event_handler(rev, ^{
        redisAsyncHandleRead(ac);
    });

    dispatch_source_t wev = dispatch_source_create(DISPATCH_SOURCE_TYPE_WRITE,
                                                   c->fd, 0, queue);
    if (wev == NULL)
        return REDIS_ERR_IO;
    dispatch_source_set_event_handler(wev, ^{
        redisAsyncHandleWrite(ac);
    });

    // Create container for context and r/w events
    e = (redisLibdispatchEvents*)malloc(sizeof(*e));
    e->context = ac;
    e->reading = e->writing = 0;

    // Register functions to start/stop listening for events
    ac->ev.addRead = redisLibdispatchAddRead;
    ac->ev.delRead = redisLibdispatchDelRead;
    ac->ev.addWrite = redisLibdispatchAddWrite;
    ac->ev.delWrite = redisLibdispatchDelWrite;
    ac->ev.cleanup = redisLibdispatchCleanup;
    ac->ev.data = e;
    e->rev = rev;
    e->wev = wev;

    return REDIS_OK;
}

#endif  // __HIREDIS_LIBDISPATCH_H__
