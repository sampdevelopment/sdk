/*  Date/time module for the Pawn Abstract Machine
 *
 *  Copyright (c) ITB CompuPhase, 2001-2005
 *
 *  Version: $Id: amxtime.c,v 1.2 2006/03/26 16:56:19 spookie Exp $
 */

#include <time.h>
#include <assert.h>
#include "amx.h"

#if defined(__WIN32__) || defined(_WIN32)
  #include <windows.h>
  #include <mmsystem.h>
#endif

/* Ensure CLOCKS_PER_SEC is defined for Linux/Unix */
#if !defined(CLOCKS_PER_SEC)
  #define CLOCKS_PER_SEC 1000000
#endif

#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32)
static int timerset = 0;
/* timeGetTime() is more accurate on Windows NT if timeBeginPeriod(1) is set */
#define INIT_TIMER()       \
  if (!timerset) {         \
    timeBeginPeriod(1);    \
    timerset = 1;          \
  }
#else
#define INIT_TIMER()
#endif

static unsigned long timestamp;
static unsigned long timelimit;
static int timerepeat;

static unsigned long gettimestamp(void)
{
    unsigned long value;

#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32)
    value = timeGetTime(); /* milliseconds */
#else
    value = (unsigned long)clock();
    /* Convert to milliseconds */
    value = (unsigned long)((1000UL * (value + CLOCKS_PER_SEC / 2)) / CLOCKS_PER_SEC);
#endif
    return value;
}

#ifdef AMX_USE_TIME_DATE_SET
static cell AMX_NATIVE_CALL n_settime(AMX *amx, cell *params)
{
#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32)
    SYSTEMTIME systim;
    GetLocalTime(&systim);
    if (params[1] >= 0) systim.wHour   = (WORD)params[1];
    if (params[2] >= 0) systim.wMinute = (WORD)params[2];
    if (params[3] >= 0) systim.wSecond = (WORD)params[3];
    SetLocalTime(&systim);
#else
    time_t sec1970;
    struct tm gtm;
    (void)amx;
    time(&sec1970);
    gtm = *localtime(&sec1970);
    if (params[1] >= 0) gtm.tm_hour = params[1];
    if (params[2] >= 0) gtm.tm_min  = params[2];
    if (params[3] >= 0) gtm.tm_sec  = params[3];
    sec1970 = mktime(&gtm);
    stime(&sec1970);
#endif
    (void)amx;
    return 0;
}

static cell AMX_NATIVE_CALL n_setdate(AMX *amx, cell *params)
{
#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32)
    SYSTEMTIME systim;
    GetLocalTime(&systim);
    if (params[1] != 0) systim.wYear  = (WORD)params[1];
    if (params[2] != 0) systim.wMonth = (WORD)params[2];
    if (params[3] != 0) systim.wDay   = (WORD)params[3];
    SetLocalTime(&systim);
#else
    time_t sec1970;
    struct tm gtm;
    (void)amx;
    time(&sec1970);
    gtm = *localtime(&sec1970);
    if (params[1] != 0) gtm.tm_year = params[1] - 1900;
    if (params[2] != 0) gtm.tm_mon  = params[2] - 1;
    if (params[3] != 0) gtm.tm_mday = params[3];
    sec1970 = mktime(&gtm);
    stime(&sec1970);
#endif
    (void)amx;
    return 0;
}
#endif

static cell AMX_NATIVE_CALL n_gettime(AMX *amx, cell *params)
{
    time_t sec1970;
    struct tm gtm;
    cell *cptr;

    assert(params[0] == (int)(3 * sizeof(cell)));

    time(&sec1970);
    gtm = *localtime(&sec1970);

    if (amx_GetAddr(amx, params[1], &cptr) == AMX_ERR_NONE) *cptr = gtm.tm_hour;
    if (amx_GetAddr(amx, params[2], &cptr) == AMX_ERR_NONE) *cptr = gtm.tm_min;
    if (amx_GetAddr(amx, params[3], &cptr) == AMX_ERR_NONE) *cptr = gtm.tm_sec;

    return (cell)sec1970;
}

static cell AMX_NATIVE_CALL n_getdate(AMX *amx, cell *params)
{
    time_t sec1970;
    struct tm gtm;
    cell *cptr;

    assert(params[0] == (int)(3 * sizeof(cell)));

    time(&sec1970);
    gtm = *localtime(&sec1970);

    if (amx_GetAddr(amx, params[1], &cptr) == AMX_ERR_NONE) *cptr = gtm.tm_year + 1900;
    if (amx_GetAddr(amx, params[2], &cptr) == AMX_ERR_NONE) *cptr = gtm.tm_mon + 1;
    if (amx_GetAddr(amx, params[3], &cptr) == AMX_ERR_NONE) *cptr = gtm.tm_mday;

    return gtm.tm_yday + 1;
}

static cell AMX_NATIVE_CALL n_tickcount(AMX *amx, cell *params)
{
    cell *cptr;
    assert(params[0] == (int)sizeof(cell));

    INIT_TIMER();
#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32)
    if (amx_GetAddr(amx, params[1], &cptr) == AMX_ERR_NONE) *cptr = 1000; /* 1ms granularity */
#else
    if (amx_GetAddr(amx, params[1], &cptr) == AMX_ERR_NONE) *cptr = (cell)CLOCKS_PER_SEC;
#endif
    return gettimestamp() & 0x7fffffff;
}

static cell AMX_NATIVE_CALL n_settimer(AMX *amx, cell *params)
{
    (void)amx;
    assert(params[0] == (int)(2 * sizeof(cell)));
    timestamp = gettimestamp();
    timelimit = params[1];
    timerepeat = (int)(params[2] == 0);
    return 0;
}

#if !defined(AMXTIME_NOIDLE)
static AMX_DEBUG PrevIdle = NULL;
static int idxTimer = -1;

static int AMXAPI amx_TimeIdle(AMX *amx)
{
    int err = 0;

    assert(idxTimer >= 0);

    if (PrevIdle != NULL) PrevIdle(amx);

    if (timelimit > 0 && (gettimestamp() - timestamp) >= timelimit)
    {
        if (timerepeat)
            timestamp += timelimit;
        else
            timelimit = 0;
        err = amx_Exec(amx, NULL, idxTimer);
        while (err == AMX_ERR_SLEEP)
            err = amx_Exec(amx, NULL, AMX_EXEC_CONT);
    }

    return err;
}
#endif

#if defined(__cplusplus)
extern "C"
#endif
const AMX_NATIVE_INFO time_Natives[] = {
    { "gettime",   n_gettime },
    { "getdate",   n_getdate },
#ifdef AMX_USE_TIME_DATE_SET
    { "settime",   n_settime },
    { "setdate",   n_setdate },
#endif
    { "tickcount", n_tickcount },
    { "settimer",  n_settimer },
    { NULL, NULL }
};

int AMXEXPORT amx_TimeInit(AMX *amx)
{
#if !defined(AMXTIME_NOIDLE)
    if (amx_FindPublic(amx, "@timer", &idxTimer) == AMX_ERR_NONE)
    {
        if (amx_GetUserData(amx, AMX_USERTAG('I','d','l','e'), (void**)&PrevIdle) != AMX_ERR_NONE)
            PrevIdle = NULL;
        amx_SetUserData(amx, AMX_USERTAG('I','d','l','e'), amx_TimeIdle);
    }
#endif
    return amx_Register(amx, time_Natives, -1);
}

int AMXEXPORT amx_TimeCleanup(AMX *amx)
{
    (void)amx;
#if !defined(AMXTIME_NOIDLE)
    PrevIdle = NULL;
#endif
    return AMX_ERR_NONE;
}
