#include "CModuleInput.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

typedef struct {
    pthread_t thread;
    int done;

    struct InputModuleInfo info;
} input_t;

static void *
input_thread (void * arg)
{
    input_t * input = arg;
    int i = 0;

    while (!input->done) {
        int loops;
        struct Event event;

        for (loops = 0; loops < 20; loops++) {
            usleep (10000);
            if (input->done || (loops > 2 && (i & 1)))
                loops = 20;
        }

        memset (&event, 0, sizeof (event));
        event.type = EVENT_ACTIVE;
        event.flags = 0;
        event.active = (i++) & 1;
        input->info.postevent (input->info.manager, &event);
    }

    return NULL;
}

void* InputModuleOpen (struct InputModuleInfo * info)
{
    input_t * input;
    int ret;

    input = malloc (sizeof (input_t));
    if (!input)
        return NULL;

    input->done = 0;
    input->info = *info;
    ret = pthread_create (&input->thread, NULL, input_thread, input);
    if (ret) {
        free (input);
        return NULL;
    }

    return input;
}

void InputModuleClose (void * handle)
{
    input_t * input = handle;

    if (!input)
        return;

    input->done = 1;
    pthread_join (input->thread, NULL);
    free (input);
}
