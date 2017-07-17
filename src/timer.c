/* Copyright (C) 2017 Henrik Hedelund.

   This file is part of MemfisMIDI.

   MemfisMIDI is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   MemfisMIDI is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with MemfisMIDI.  If not, see <http://www.gnu.org/licenses/>. */

#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <errno.h>
#include <string.h>

#include "timer.h"

#ifdef CLOCK_MONOTONIC_RAW
# define MM_CLOCK_ID CLOCK_MONOTONIC_RAW
#else
# define MM_CLOCK_ID CLOCK_MONOTONIC
#endif

#define MM_TIMER_NUM_TAPS 4

struct _MMTimer
{
  struct timespec ts;
  unsigned int taps[MM_TIMER_NUM_TAPS];
  unsigned int curr_tap;
  unsigned int last_tap_age;
};

MMTimer *
mm_timer_new ()
{
  MMTimer *timer = calloc (1, sizeof (MMTimer));
  assert (timer != NULL);
  assert (mm_timer_reset (timer));
  return timer;
}

void
mm_timer_free (MMTimer *timer)
{
  if (timer != NULL)
    free (timer);
}

bool
mm_timer_reset (MMTimer *timer)
{
  if (timer == NULL)
    return false;
  memset (timer->taps, 0, sizeof (unsigned int) * MM_TIMER_NUM_TAPS);
  timer->curr_tap = MM_TIMER_NUM_TAPS - 1;
  timer->last_tap_age = 0;
  return clock_gettime (MM_CLOCK_ID, &timer->ts) == 0 ? true : false;
}

unsigned int
mm_timer_get_age (const MMTimer *timer)
{
  struct timespec now, diff;

  if (timer == NULL)
    return 0;

  clock_gettime (MM_CLOCK_ID, &now);
  diff.tv_sec = now.tv_sec - timer->ts.tv_sec;
  diff.tv_nsec = now.tv_nsec - timer->ts.tv_nsec;

  return (diff.tv_sec * 1000) + (diff.tv_nsec / 1000000);
}

void
mm_timer_tap (MMTimer *timer)
{
  unsigned int tap, age;

  if (timer == NULL)
    return;

  age = mm_timer_get_age (timer);
  tap = age - timer->last_tap_age;

  if (timer->last_tap_age > 0 && tap < 2000) /* > 30 bpm */
    {
      timer->curr_tap = (timer->curr_tap + 1) % MM_TIMER_NUM_TAPS;
      timer->taps[timer->curr_tap] = tap;
    }

  timer->last_tap_age = age;
}

static int
cmp_uint (const void *a, const void *b)
{
  unsigned int ia = *(int *) a;
  unsigned int ib = *(int *) b;
  return (ia > ib) - (ia < ib);
}

double
mm_timer_get_bpm (const MMTimer *timer)
{
  double median;
  int ntaps = 0;
  unsigned int taps[MM_TIMER_NUM_TAPS];

  if (timer == NULL)
    return 0.;

  for (int i = 0; i < MM_TIMER_NUM_TAPS; ++i)
    {
      if (timer->taps[i] > 0)
        taps[ntaps++] = timer->taps[i];
    }

  if (ntaps == 0)
    return 0.;

  qsort (taps, ntaps, sizeof (unsigned int), cmp_uint);

  median = (double) taps[ntaps / 2];
  if ((ntaps % 2) == 0)
    median = (median + (double) taps[(ntaps / 2) - 1]) / 2.;

  return 60000. / median;
}

void
mm_sleep (unsigned int ms)
{
  int err;
  struct timespec req, rem;
  rem.tv_sec = ms / 1000;
  rem.tv_nsec = (ms % 1000) * 1000000;
  do
    {
      err = 0;
      req.tv_sec = rem.tv_sec;
      req.tv_nsec = rem.tv_nsec;
      err = nanosleep (&req, &rem);
    }
  while (err && (errno == EINTR));
}
