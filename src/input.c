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
#include <signal.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include <unistd.h>

#include <fcntl.h>
#include <linux/joystick.h>

#include "input.h"
#include "print.h"

#define MAP_LENGTH (KEY_MAX - BTN_MISC + 1)

static bool mm_quit = false;
static void mm_sa_handler (int);

struct _MMInput
{
  int fd;
  uint8_t nbuttons;
  uint16_t map[MAP_LENGTH];
};

MMInput *
mm_input_new ()
{
  MMInput *input = calloc (1, sizeof (MMInput));
  struct sigaction sa;

  assert (input != NULL);
  input->fd = -1;

  sigaction (SIGINT, NULL, &sa);
  if (sa.sa_handler != mm_sa_handler)
    {
      sa.sa_handler = mm_sa_handler;
      sigaction (SIGINT, &sa, NULL);
    }

  return input;
}

void
mm_input_free (MMInput *input)
{
  if (input != NULL)
    {
      mm_input_disconnect (input);
      free (input);
    }
}

bool
mm_input_connect (MMInput *input, const char *pathname)
{
  if (input == NULL || pathname == NULL)
    return false;

  mm_input_disconnect (input);

  input->fd = open (pathname, O_RDONLY);
  if (input->fd < 0)
    return false;

  fcntl (input->fd, F_SETFL, O_NONBLOCK);

  if (ioctl (input->fd, JSIOCGBUTTONS, &input->nbuttons) < 0)
    MMERR ("Could not get button count: ERRNO " MMCY ("%d"), errno);

  if (ioctl (input->fd, JSIOCGBTNMAP, input->map) < 0)
    MMERR ("Could not get button map: ERRNO " MMCY ("%d"), errno);

  return true;
}

void
mm_input_disconnect (MMInput *input)
{
  if (input != NULL && input->fd >= 0)
    {
      close (input->fd);
      input->fd = -1;
      input->nbuttons = 0;
      memset (input->map, 0, sizeof (uint16_t) * MAP_LENGTH);
    }
}

int
mm_input_read (const MMInput *input)
{
  struct js_event e;

  if (mm_quit == true)
    {
      mm_quit = false;
      return MM_BTN_SELECT;
    }

  if (input == NULL || input->fd < 0)
    return -1;

  while (read (input->fd, &e, sizeof (e)) > 0)
    {
      if (e.type & JS_EVENT_INIT)
        continue;
      if ((e.type & JS_EVENT_BUTTON)
          && e.value == 1
          && e.number < input->nbuttons)
        {
          return (int) input->map[e.number];
        }
    }

  return -1;
}

static void
mm_sa_handler (int sig)
{
  signal (sig, mm_sa_handler);
  mm_quit = true;
}
