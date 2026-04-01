/*
 * Copyright (c) 2026 dresden elektronik ingenieurtechnik gmbh.
 * All rights reserved.
 *
 * The software in this package is published under the terms of the BSD
 * style license a copy of which has been included with this distribution in
 * the LICENSE.txt file.
 *
 */

#include "deconz/u_assert.h"
#include "deconz/n_downloader.h"
#include "deconz/n_proxy.h"
#include "deconz/u_threads.h"
#include "deconz/u_memory.h"
#include "deconz/n_ssl.h"
#include "deconz/u_sstream.h"

enum ndl_entry_state
{
    N_DL_ENTRY_INITIAL,
    N_DL_ENTRY_TCP_CONNECTED,
    N_DL_ENTRY_SSL_HANDSHAKE,
    N_DL_ENTRY_SSL_CONNECTED,
    N_DL_ENTRY_WAIT_REPLY,
    N_DL_ENTRY_DONE,
};

typedef struct N_DownloadEntry_s
{
    struct N_DownloadEntry_s *next;
    int handle;
    int ssl;
    N_DownloadStatus status;
    enum ndl_entry_state state;
    N_DownloadCallback callback;
    char host[256];
    char *url;
    char *urlpath;
    unsigned hdr_length;
    long content_length;
    unsigned char *buf;
    unsigned bufpos;
    unsigned bufsize;
    void *user;
} N_DownloadEntry;

enum ndl_state
{
    N_DL_INITIAL = 0,
    N_DL_SHUTDOWN = 1,

    N_DL_RUNNING = 2, /* anything above this means: operational */
    N_DL_IDLE = 3
};

typedef struct
{
    int handle;
    enum ndl_state state;
    U_Thread th;
    U_Mutex mtx;
    N_SslSocket ssl;

    N_DownloadEntry *busy_list;
    N_DownloadEntry *done_list;
    N_Proxy proxy;
} N_dl;

static N_dl ndl_;

static void ndl_connect(N_dl *dl, N_DownloadEntry *entry)
{
    U_SStream ss;
    unsigned hostlen;
    unsigned short port;

    U_memset(entry->host, 0, sizeof(entry->host));
    U_sstream_init(&ss, entry->url, U_strlen(entry->url));

    if (U_sstream_starts_with(&ss, "https://"))
    {
        U_sstream_seek(&ss, U_strlen("https://"));
        port = 443;
        entry->ssl = 1;
    }
    else if (U_sstream_starts_with(&ss, "http://"))
    {
        U_sstream_seek(&ss, U_strlen("http://"));
        port = 80;
        entry->ssl = 0;
    }
    else
    {
        entry->status = N_DownloadError;
        entry->state = N_DL_ENTRY_DONE;
        return;
    }

    for (hostlen = ss.pos; hostlen < ss.len && ss.str[hostlen] != '/'; hostlen++)
    {
        entry->host[hostlen - ss.pos] = ss.str[hostlen];
    }

    entry->urlpath = &entry->url[hostlen];

    if (entry->ssl)
    {
        if (N_SslClientInit(&dl->ssl, entry->host, port, &dl->proxy))
        {
            entry->status = N_DownloadStarting;
            entry->state = N_DL_ENTRY_SSL_HANDSHAKE;
            return;
        }
    }
    else
    {
        if (N_TcpInit(&dl->ssl.tcp, N_AF_IPV4))
        {
            if (N_TcpConnect(&dl->ssl.tcp, entry->host, port))
            {
                entry->status = N_DownloadStarting;
                entry->state = N_DL_ENTRY_TCP_CONNECTED;
                return;
            }

            N_TcpClose(&dl->ssl.tcp);
        }
    }

    entry->status = N_DownloadError;
    entry->state = N_DL_ENTRY_DONE;
    return;
}

static void ndl_ssl_handhsake(N_dl *dl, N_DownloadEntry *entry)
{
    U_ASSERT(entry->state = N_DL_ENTRY_SSL_HANDSHAKE);

    int handShake = N_SslHandshake(&dl->ssl);

    if (handShake < 0) /* error */
    {
        entry->status = N_DownloadErrorHandshake;
        entry->state = N_DL_ENTRY_DONE;
    }
    else if (handShake == 0)
    {
        /* in progress */
    }
    else
    {
        entry->state = N_DL_ENTRY_SSL_CONNECTED;
    }
}

static void ndl_send_request(N_dl *dl, N_DownloadEntry *entry)
{
    int ok = 0;
    U_SStream ss;
    char buf[1024];

    U_sstream_init(&ss, buf, sizeof(buf));

    U_sstream_put_str(&ss, "GET ");
    U_sstream_put_str(&ss, entry->urlpath);
    U_sstream_put_str(&ss, " HTTP/1.1\r\n");
    U_sstream_put_str(&ss, "Host: ");
    U_sstream_put_str(&ss, entry->host);
    U_sstream_put_str(&ss, "\r\n");
    U_sstream_put_str(&ss, "Accept: *.*\r\n");
    U_sstream_put_str(&ss, "Connection: close\r\n");
    U_sstream_put_str(&ss, "\r\n\r\n");

    if (entry->state == N_DL_ENTRY_TCP_CONNECTED)
    {
        if (N_TcpWrite(&dl->ssl.tcp, buf, ss.pos))
            ok = 1;
    }
    else if (entry->state == N_DL_ENTRY_SSL_CONNECTED)
    {
        if (N_SslWrite(&dl->ssl, buf, ss.pos))
            ok = 1;
    }

    if (ok)
    {
        entry->hdr_length = 0;
        U_memset(entry->buf, 0, entry->bufsize);
        entry->state = N_DL_ENTRY_WAIT_REPLY;
        entry->status = N_DownloadRunning;
    }
    else
    {
        entry->status = N_DownloadErrorSendRequest;
        entry->state = N_DL_ENTRY_DONE;
    }
}

static void ndl_read_response(N_dl *dl, N_DownloadEntry *entry)
{
    int didread = 0;
    int nread = 0;
    unsigned i;

    if (entry->ssl)
    {
        if (N_SslCanRead(&dl->ssl))
        {
            nread = N_SslRead(&dl->ssl, &entry->buf[entry->bufpos], entry->bufsize - entry->bufpos);
            if (nread >= 0)
                didread = 1;
        }
    }
    else
    {
        if (N_TcpCanRead(&dl->ssl.tcp))
        {
            nread = N_TcpRead(&dl->ssl.tcp, &entry->buf[entry->bufpos], entry->bufsize - entry->bufpos);
            didread = 1;
        }
    }

    if (didread)
    {
        if (nread > 0)
        {

            U_ASSERT(nread <= entry->bufsize - entry->bufpos);
            entry->bufpos += (unsigned)nread;

            if (entry->hdr_length == 0 && 16 < entry->bufpos)
            {
                for (i = 0; i < entry->bufpos - 4; i++)
                {
                    if (U_memcmp(&entry->buf[i], "\r\n\r\n", 4) == 0)
                    {
                        entry->buf[i] = '\0';
                        entry->hdr_length = i + 4;
                        entry->content_length = 0;

                        U_SStream ss;
                        U_sstream_init(&ss, entry->buf, entry->hdr_length);
                        U_sstream_find(&ss, " ");

                        long httpStatus = U_sstream_get_long(&ss);
                        if (httpStatus != 200)
                        {
                            if (httpStatus == 404)
                                entry->status = N_DownloadErrorNotFound;
                            else
                                entry->status = N_DownloadError;
                        }
                        else if (U_sstream_find(&ss, "Content-Length:"))
                        {
                            U_sstream_find(&ss, ":");
                            ss.pos++;
                            entry->content_length = U_sstream_get_long(&ss);

                            if (entry->bufsize < entry->hdr_length + entry->content_length)
                            {
                                entry->status = N_DownloadErrorTooLarge;
                            }
                        }

                        break;
                    }
                }
            }
        }
        else if (nread <= 0)
        {
            U_ASSERT(entry->bufpos != 0);
            entry->state = N_DL_ENTRY_DONE;

            if (entry->status == N_DownloadRunning)
            {
                if (entry->hdr_length + entry->content_length == entry->bufpos)
                {
                    entry->status = N_DownloadDone;
                }
                else
                {
                    entry->status = N_DownloadErrorSizeMismatch;
                }
            }
        }
    }
}

static void ndl_thread(void *arg)
{
    int running = 1;
    N_dl *dl = arg;
    N_DownloadEntry *entry;

    U_thread_mutex_lock(&dl->mtx);
    U_thread_set_name(&dl->th, "n_downloader");
    U_thread_mutex_unlock(&dl->mtx);

    for (;running;)
    {
        unsigned long sleepms = 100;
        U_thread_mutex_lock(&dl->mtx);
        if (dl->state == N_DL_SHUTDOWN)
        {
            running = 0;
        }
        else if (dl->busy_list)
        {
            sleepms = 5;
            entry = dl->busy_list;
            if (entry->state == N_DL_ENTRY_INITIAL)
            {
                ndl_connect(dl, entry);
            }
            else if (entry->state == N_DL_ENTRY_TCP_CONNECTED || entry->state == N_DL_ENTRY_SSL_CONNECTED)
            {
                ndl_send_request(dl, entry);
            }
            else if (entry->state == N_DL_ENTRY_WAIT_REPLY)
            {
                ndl_read_response(dl, entry);
            }
            else if (entry->state == N_DL_ENTRY_SSL_HANDSHAKE)
            {
                ndl_ssl_handhsake(dl, entry);
            }
            else if (entry->state == N_DL_ENTRY_DONE)
            {
                if (entry->ssl)
                {
                    N_SslClose(&dl->ssl);
                }
                else
                {
                    N_TcpClose(&dl->ssl.tcp);
                }

                dl->busy_list = entry->next;
                entry->next = dl->done_list;
                dl->done_list = entry;
            }
        }
        U_thread_mutex_unlock(&dl->mtx);
        U_thread_msleep(sleepms);
    }

    U_thread_exit(0);
}

void N_DownloadInit(void)
{
    if (ndl_.state == N_DL_INITIAL)
    {
        N_SslInit();

        U_thread_mutex_init(&ndl_.mtx);
        U_thread_mutex_lock(&ndl_.mtx);
        ndl_.handle = 1;
        ndl_.state = N_DL_IDLE;
        U_thread_create(&ndl_.th, ndl_thread, &ndl_);
        U_thread_mutex_unlock(&ndl_.mtx);
    }
}

void N_DownloadShutDown(void)
{
    if (ndl_.state == N_DL_INITIAL)
        return;

    U_thread_mutex_lock(&ndl_.mtx);
    if (ndl_.state > N_DL_RUNNING)
        ndl_.state = N_DL_SHUTDOWN;
    U_thread_mutex_unlock(&ndl_.mtx);

    U_thread_join(&ndl_.th);
    U_thread_mutex_destroy(&ndl_.mtx);
    ndl_.state = N_DL_INITIAL;
}

void N_DownloadStep(void)
{
    N_DownloadData dldata;
    N_DownloadEntry *entry = 0;
    if (ndl_.state == N_DL_INITIAL)
        return;

    U_thread_mutex_lock(&ndl_.mtx);
    if (ndl_.done_list)
    {
        entry = ndl_.done_list;
        ndl_.done_list = entry->next;
        entry->next = 0;
    }
    U_thread_mutex_unlock(&ndl_.mtx);

    if (entry)
    {
        if (entry->callback)
        {
            U_memset(&dldata, 0, sizeof(dldata));
            dldata.handle = entry->handle;
            dldata.status = entry->status;
            dldata.user = entry->user;
            if (entry->hdr_length)
            {
                dldata.hdr_size = entry->hdr_length;
                dldata.hdr = (const char*)&entry->buf[0];
            }

            if (dldata.status == N_DownloadDone && entry->content_length)
            {
                dldata.data = &entry->buf[entry->hdr_length];
                dldata.data_size = entry->content_length;
            }

            entry->callback(&dldata);
        }

        U_Free(entry);
    }
}

int N_Download(const char *url, unsigned maxsize, N_DownloadCallback cb, void *user)
{
    int result = 0;
    unsigned urllen = 0;
    unsigned size = 0;
    N_DownloadEntry *entry;
    if (ndl_.state < N_DL_RUNNING)
        return 0;

    size = sizeof(*entry) + maxsize;

    urllen = U_strlen(url);
    size += urllen + 1; /* '\0' */

    unsigned mul = 8;
    for (;mul * 128 < size; mul++)
    {}
    size = mul * 128;

    U_thread_mutex_lock(&ndl_.mtx);

    N_GetProxy(&ndl_.proxy);

    entry = U_Alloc(size);
    if (entry)
    {
        U_memset(entry, 0, size);
        if (ndl_.handle <= 0)
            ndl_.handle = 1;
        entry->handle = ndl_.handle++;
        result = entry->handle;

        entry->url = (char*)entry + sizeof(*entry);
        U_memcpy(entry->url, url, urllen + 1);
        entry->buf = (unsigned char*)&entry->url[urllen + 1];
        entry->bufsize = maxsize;
        entry->callback = cb;
        entry->status = N_DownloadStarting;
        entry->state = N_DL_ENTRY_INITIAL;
        entry->user = user;
    }

    N_DownloadEntry **pp = &ndl_.busy_list;
    while (*pp != 0)
        pp = &((*pp)->next);

    *pp = entry;

    U_thread_mutex_unlock(&ndl_.mtx);

    return result;
}

