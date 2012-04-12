#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <dispatch/dispatch.h>
#include "hiredis.h"
#include "async.h"
#include "adapters/libdispatch.h"

static dispatch_semaphore_t dispatch_semaphore = NULL;

void getCallback(redisAsyncContext *c, void *r, void *privdata) {
    redisReply *reply = r;
    if (reply == NULL) return;
    printf("argv[%s]: %s\n", (char*)privdata, reply->str);

    /* Disconnect after receiving the reply to GET */
    redisAsyncDisconnect(c);
}

void connectCallback(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        printf("Error: %s\n", c->errstr);
        return;
    }
    printf("Connected...\n");
}

void disconnectCallback(const redisAsyncContext *c, int status) {
    if (status != REDIS_OK) {
        printf("Error: %s\n", c->errstr);
        return;
    }
    printf("Disconnected...\n");
    dispatch_semaphore_signal(dispatch_semaphore);
}

int main (int argc, char **argv) {
    redisAsyncContext *c = redisAsyncConnect("127.0.0.1", 6379);
    if (c->err) {
        /* Let *c leak for now... */
        printf("Error: %s\n", c->errstr);
        return 1;
    }

    dispatch_queue_t queue = NULL;
    queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
    if (!queue) {
        printf("Error: cannot get global dispatch queue\n");
        return 1;
    }

    if (dispatch_semaphore == NULL)
      dispatch_semaphore = dispatch_semaphore_create(0);

    redisLibdispatchAttach(c, queue);
    redisAsyncSetConnectCallback(c, connectCallback);
    redisAsyncSetDisconnectCallback(c, disconnectCallback);
    redisAsyncCommand(c, NULL, NULL, "SET key %b", argv[argc-1],
                      strlen(argv[argc-1]));
    redisAsyncCommand(c, getCallback, (char*)"end-1", "GET key");

    dispatch_semaphore_wait(dispatch_semaphore, DISPATCH_TIME_FOREVER);
    return 0;
}
