/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * bsp_thread.c
 * Copyright (C) 2015 Dr.NP <np@bsgroup.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of Unknown nor the name of any other
 *    contributor may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Unknown AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL Unknown OR ANY OTHER
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * OS threading model
 *
 * @package bsp::blacktail
 * @author Dr.NP <np@bsgroup.org>
 * @update 02/26/2015
 * @changelog
 *      [02/26/2015] - Creation
 */

#include "bsp-private.h"
#include "bsp.h"

BSP_PRIVATE(const char *) _tag_ = "Thread";
void * _process(void *arg)
{
    BSP_THREAD *me = (BSP_THREAD *) arg;
    if (!me)
    {
        return NULL;
    }

    int nfds, i;
    BSP_EVENT ev;

    if (me->hook_former)
    {
        (me->hook_former)(me);
    }

    while (me->has_loop)
    {
        nfds = bsp_wait_event(me->event_container);
        for (i = 0; i < nfds; i ++)
        {
            bsp_get_active_event(me->event_container, &ev, i);
            if (ev.events & BSP_EVENT_ACCEPT)
            {
                // Do TCP accept
            }

            if (ev.events & BSP_EVENT_SIGNAL)
            {
                // Signal
            }

            if (ev.events & BSP_EVENT_TIMER)
            {
                // Timer
                bsp_trace_message(BSP_TRACE_DEBUG, _tag_, "Timer event triggered");
            }

            if (ev.events & BSP_EVENT_READ)
            {
                // Data can read
            }

            if (ev.events & BSP_EVENT_WRITE)
            {
                // IO writable
            }

            if (ev.events & BSP_EVENT_LOCAL_HUP)
            {
                // Local hup
            }

            if (ev.events & BSP_EVENT_REMOTE_HUP)
            {
                // Remote hup
            }

            if (ev.events & BSP_EVENT_ERROR)
            {
                // General error
            }
        }
    }

    if (me->hook_latter)
    {
        (me->hook_latter)(me);
    }

    return NULL;
}

// Generate an OS thread
BSP_DECLARE(BSP_THREAD *) bsp_new_thread(BSP_THREAD_TYPE type, void (*hook_former)(BSP_THREAD *), void (*hook_latter)(BSP_THREAD *))
{
    BSP_THREAD *t = bsp_calloc(1, sizeof(BSP_THREAD));
    if (!t)
    {
        return NULL;
    }

    t->type = type;
    t->has_loop = BSP_FALSE;

    if (type != BSP_THREAD_NORMAL)
    {
        // Normal thread has no event loop
        // Create event loop
        t->event_container = bsp_new_event_container();
        if (t->event_container)
        {
            t->has_loop = BSP_TRUE;
        }
        else
        {
            bsp_free(t);
            bsp_trace_message(BSP_TRACE_CRITICAL, _tag_, "Cannot create thread, event container alloc error");

            return NULL;
        }
    }

    t->hook_former = hook_former;
    t->hook_latter = hook_latter;

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    if (BSP_THREAD_ACCEPTOR == t->type || 
        BSP_THREAD_IO == t->type || 
        BSP_THREAD_WORKER == t->type)
    {
        // Detach them
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    }
    else
    {
        // Boss and normal thread were joinable
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    }

    if (0 == pthread_create(&t->pid, &attr, _process, (void *) t))
    {
        bsp_trace_message(BSP_TRACE_DEBUG, _tag_, "Create thread %llu", (uint64_t) t->pid);
    }
    else
    {
        bsp_del_event_container(t->event_container);
        bsp_free(t);
        bsp_trace_message(BSP_TRACE_ALERT, _tag_, "Cannot create thread, pthread_create failed!");
        t = NULL;
    }

    pthread_attr_destroy(&attr);

    return t;
}

// Cancellation an OS thread
BSP_DECLARE(int) bsp_del_thread(BSP_THREAD *t)
{
    if (t)
    {
        // Try cancel the thread
        int ret = pthread_cancel(t->pid);
        if (0 == ret)
        {
            bsp_trace_message(BSP_TRACE_DEBUG, _tag_, "Thread %llu cancelled", (uint64_t) t->pid);

            return BSP_RTN_SUCCESS;
        }
        else
        {
            // No thread
            bsp_trace_message(BSP_TRACE_ERROR, _tag_, "Cannot cancel thread %llu, thread not found", (uint64_t) t->pid);

            return BSP_RTN_ERR_THREAD;
        }

        bsp_free(t);
    }

    return BSP_RTN_INVALID;
}

// Wait (try join) thread, this operation will be blocked until the target thread exited
BSP_DECLARE(int) bsp_wait_thread(BSP_THREAD *t)
{
    if (t)
    {
        int ret = pthread_join(t->pid, NULL);
        if (0 == ret)
        {
            bsp_trace_message(BSP_TRACE_NOTICE, _tag_, "Thread %llu exited normally", (uint64_t) t->pid);

            return BSP_RTN_SUCCESS;
        }
        else
        {
            switch (ret)
            {
                case EDEADLK : 
                    bsp_trace_message(BSP_TRACE_CRITICAL, _tag_, "Deadlock detected");
                    break;
                case EINVAL : 
                    bsp_trace_message(BSP_TRACE_ERROR, _tag_, "Try to wait a non-joinable thread");
                    break;
                case ESRCH : 
                    bsp_trace_message(BSP_TRACE_ERROR, _tag_, "Try to wait a non-existent thread");
                    break;
                default : 
                    break;
            }

            return BSP_RTN_ERR_THREAD;
        }
    }

    return BSP_RTN_INVALID;
}