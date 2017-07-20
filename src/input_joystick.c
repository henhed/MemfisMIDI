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
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <unistd.h>
#include <errno.h>

#include <fcntl.h>
#include <linux/joystick.h>

#include "input_joystick.h"
#include "print.h"

#define MAP_LENGTH (KEY_MAX - BTN_MISC + 1)

typedef struct
{
  int fd;
  uint8_t nbuttons;
  uint16_t map[MAP_LENGTH];
} MMInputJoystick;

enum {
  MMJS_BTN_TRIGGER = 0x120,
  MMJS_BTN_THUMB,
  MMJS_BTN_THUMB2,
  MMJS_BTN_TOP,
  MMJS_BTN_TOP2,
  MMJS_BTN_PINKIE,
  MMJS_BTN_BASE,
  MMJS_BTN_BASE2,
  MMJS_BTN_BASE3,
  MMJS_BTN_BASE4,
  MMJS_BTN_BASE5,
  MMJS_BTN_BASE6,

  MMJS_BTN_DEAD = 0x12F,
  MMJS_BTN_A,
  MMJS_BTN_B,
  MMJS_BTN_C,
  MMJS_BTN_X,
  MMJS_BTN_Y,
  MMJS_BTN_Z,
  MMJS_BTN_TL,
  MMJS_BTN_TR,
  MMJS_BTN_TL2,
  MMJS_BTN_TR2,
  MMJS_BTN_SELECT,
  MMJS_BTN_START,
  MMJS_BTN_MODE,
  MMJS_BTN_THUMBL,
  MMJS_BTN_THUMBR,

  MMJS_BTN_DPAD_UP = 0x220,
  MMJS_BTN_DPAD_DOWN,
  MMJS_BTN_DPAD_LEFT,
  MMJS_BTN_DPAD_RIGTH,

  MMJS_BTN_DPAD_LEFT_ALT = 0x2C0,
  MMJS_BTN_DPAD_RIGHT_ALT,
  MMJS_BTN_DPAD_UP_ALT,
  MMJS_BTN_DPAD_DOWN_ALT
};

static const char *mm_js_btn_name (int);

static int
mm_input_joystick_open (int id)
{
  int fd;
  char path[15];
  if (id < 0 || id > 9)
    return -1;

  sprintf (path, "/dev/input/js%d", id);
  fd = open (path, O_RDONLY);
  if (fd >= 0)
    fcntl (fd, F_SETFL, O_NONBLOCK);

  return fd;
}

static void *
mm_input_joystick_connect (const MMInputDevice *device)
{
  MMInputJoystick *input;
  int fd;

  if (device == NULL)
    return NULL;

  fd = mm_input_joystick_open (device->id);
  if (fd < 0)
    {
      MMERR ("Could not open joystick " MMCY ("%d"), device->id);
      return NULL;
    }

  input = calloc (1, sizeof (MMInputJoystick));
  assert (input != NULL);
  input->fd = fd;

  if (ioctl (input->fd, JSIOCGBUTTONS, &input->nbuttons) < 0)
    MMERR ("Could not get button count: ERRNO " MMCY ("%d"), errno);

  if (ioctl (input->fd, JSIOCGBTNMAP, input->map) < 0)
    MMERR ("Could not get button map: ERRNO " MMCY ("%d"), errno);

  return input;
}

static void
mm_input_joystick_disconnect (void *connection)
{
  MMInputJoystick *input = (MMInputJoystick *) connection;
  if (input == NULL)
    return;

  if (input->fd >= 0)
    close (input->fd);

  free (input);
}

static int
mm_input_joystick_read (void *connection, MMInputEvent *event)
{
  MMInputJoystick *input = (MMInputJoystick *) connection;
  struct js_event e;

  if (input == NULL || input->fd < 0 || event == NULL)
    return -1;

  while (read (input->fd, &e, sizeof (e)) > 0)
    {
      if (e.type & JS_EVENT_INIT)
        continue;
      if ((e.type & JS_EVENT_BUTTON)
          && e.value == 1
          && e.number < input->nbuttons)
        {
          switch (input->map[e.number])
            {
            case MMJS_BTN_SELECT:
              event->type = MMIE_QUIT;
              break;
            case MMJS_BTN_TL:
              event->type = MMIE_KILLALL;
              break;
            case MMJS_BTN_TR:
              event->type = MMIE_NEXT_STEP;
              break;
            case MMJS_BTN_Y:
              event->type = MMIE_TAP;
              break;
            default:
              MMERR ("Unhandled input event " MMCY ("%s"),
                     mm_js_btn_name (input->map[e.number]));
              continue;
            }
          event->timestamp = e.time;
          return 1;
        }
    }

  return 0;
}

static size_t
mm_input_joystick_probe (MMInputDevice *devices, size_t ndevices)
{
  size_t found = 0;

  if (devices == NULL || ndevices == 0)
    return 0;

  for (int id = 0; id < 10 && found < ndevices; ++id)
    {
      int fd = mm_input_joystick_open (id);
      if (fd >= 0)
        {
          MMInputDevice *device = &devices[found++];
          device->type = mm_input_joystick_backend->name;
          device->id = id;
          if (ioctl (fd, JSIOCGNAME (sizeof (device->name)), device->name) < 0)
            MMERR ("Could not get joystick name: ERRNO " MMCY ("%d"), errno);
          close (fd);
        }
    }

  return found;
}

static const MMInputBackend _mm_input_joystick_backend = {
  "JOYSTICK",
  mm_input_joystick_connect,
  mm_input_joystick_disconnect,
  mm_input_joystick_read,
  mm_input_joystick_probe
};

const MMInputBackend *mm_input_joystick_backend = &_mm_input_joystick_backend;

static const char *
mm_js_btn_name (int id)
{
  switch (id)
    {
    case MMJS_BTN_TRIGGER:
      return "TRIGGER";
    case MMJS_BTN_THUMB:
      return "THUMB";
    case MMJS_BTN_THUMB2:
      return "THUMB2";
    case MMJS_BTN_TOP:
      return "TOP";
    case MMJS_BTN_TOP2:
      return "TOP2";
    case MMJS_BTN_PINKIE:
      return "PINKIE";
    case MMJS_BTN_BASE:
      return "BASE";
    case MMJS_BTN_BASE2:
      return "BASE2";
    case MMJS_BTN_BASE3:
      return "BASE3";
    case MMJS_BTN_BASE4:
      return "BASE4";
    case MMJS_BTN_BASE5:
      return "BASE5";
    case MMJS_BTN_BASE6:
      return "BASE6";
    case MMJS_BTN_DEAD:
      return "DEAD";
    case MMJS_BTN_A:
      return "A";
    case MMJS_BTN_B:
      return "B";
    case MMJS_BTN_C:
      return "C";
    case MMJS_BTN_X:
      return "X";
    case MMJS_BTN_Y:
      return "Y";
    case MMJS_BTN_Z:
      return "Z";
    case MMJS_BTN_TL:
      return "TL";
    case MMJS_BTN_TR:
      return "TR";
    case MMJS_BTN_TL2:
      return "TL2";
    case MMJS_BTN_TR2:
      return "TR2";
    case MMJS_BTN_SELECT:
      return "SELECT";
    case MMJS_BTN_START:
      return "START";
    case MMJS_BTN_MODE:
      return "MODE";
    case MMJS_BTN_THUMBL:
      return "THUMBL";
    case MMJS_BTN_THUMBR:
      return "THUMBR";
    case MMJS_BTN_DPAD_UP:
      return "DPAD_UP";
    case MMJS_BTN_DPAD_DOWN:
      return "DPAD_DOWN";
    case MMJS_BTN_DPAD_LEFT:
      return "DPAD_LEFT";
    case MMJS_BTN_DPAD_RIGTH:
      return "DPAD_RIGTH";
    case MMJS_BTN_DPAD_LEFT_ALT:
      return "DPAD_LEFT_ALT";
    case MMJS_BTN_DPAD_RIGHT_ALT:
      return "DPAD_RIGHT_ALT";
    case MMJS_BTN_DPAD_UP_ALT:
      return "DPAD_UP_ALT";
    case MMJS_BTN_DPAD_DOWN_ALT:
      return "DPAD_DOWN_ALT";
    default:
      return "UNKNOWN";
    }
}
