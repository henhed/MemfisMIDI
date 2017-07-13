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

#ifndef MM_INPUT_H
#define MM_INPUT_H 1

#include <stdbool.h>

typedef struct _MMInput MMInput;

enum {
  MM_BTN_TRIGGER = 0x120,
  MM_BTN_THUMB,
  MM_BTN_THUMB2,
  MM_BTN_TOP,
  MM_BTN_TOP2,
  MM_BTN_PINKIE,
  MM_BTN_BASE,
  MM_BTN_BASE2,
  MM_BTN_BASE3,
  MM_BTN_BASE4,
  MM_BTN_BASE5,
  MM_BTN_BASE6,

  MM_BTN_DEAD = 0x12F,
  MM_BTN_A,
  MM_BTN_B,
  MM_BTN_C,
  MM_BTN_X,
  MM_BTN_Y,
  MM_BTN_Z,
  MM_BTN_TL,
  MM_BTN_TR,
  MM_BTN_TL2,
  MM_BTN_TR2,
  MM_BTN_SELECT,
  MM_BTN_START,
  MM_BTN_MODE,
  MM_BTN_THUMBL,
  MM_BTN_THUMBR,

  MM_BTN_DPAD_UP = 0x220,
  MM_BTN_DPAD_DOWN,
  MM_BTN_DPAD_LEFT,
  MM_BTN_DPAD_RIGTH,

  MM_BTN_DPAD_LEFT_ALT = 0x2C0,
  MM_BTN_DPAD_RIGHT_ALT,
  MM_BTN_DPAD_UP_ALT,
  MM_BTN_DPAD_DOWN_ALT
};

MMInput *mm_input_new ();
void mm_input_free (MMInput *);
bool mm_input_connect (MMInput *, const char *);
void mm_input_disconnect (MMInput *);
int mm_input_read (const MMInput *);

static inline const char *
mm_btn_name (int id)
{
  switch (id)
    {
    case MM_BTN_TRIGGER:
      return "TRIGGER";
    case MM_BTN_THUMB:
      return "THUMB";
    case MM_BTN_THUMB2:
      return "THUMB2";
    case MM_BTN_TOP:
      return "TOP";
    case MM_BTN_TOP2:
      return "TOP2";
    case MM_BTN_PINKIE:
      return "PINKIE";
    case MM_BTN_BASE:
      return "BASE";
    case MM_BTN_BASE2:
      return "BASE2";
    case MM_BTN_BASE3:
      return "BASE3";
    case MM_BTN_BASE4:
      return "BASE4";
    case MM_BTN_BASE5:
      return "BASE5";
    case MM_BTN_BASE6:
      return "BASE6";
    case MM_BTN_DEAD:
      return "DEAD";
    case MM_BTN_A:
      return "A";
    case MM_BTN_B:
      return "B";
    case MM_BTN_C:
      return "C";
    case MM_BTN_X:
      return "X";
    case MM_BTN_Y:
      return "Y";
    case MM_BTN_Z:
      return "Z";
    case MM_BTN_TL:
      return "TL";
    case MM_BTN_TR:
      return "TR";
    case MM_BTN_TL2:
      return "TL2";
    case MM_BTN_TR2:
      return "TR2";
    case MM_BTN_SELECT:
      return "SELECT";
    case MM_BTN_START:
      return "START";
    case MM_BTN_MODE:
      return "MODE";
    case MM_BTN_THUMBL:
      return "THUMBL";
    case MM_BTN_THUMBR:
      return "THUMBR";
    case MM_BTN_DPAD_UP:
      return "DPAD_UP";
    case MM_BTN_DPAD_DOWN:
      return "DPAD_DOWN";
    case MM_BTN_DPAD_LEFT:
      return "DPAD_LEFT";
    case MM_BTN_DPAD_RIGTH:
      return "DPAD_RIGTH";
    case MM_BTN_DPAD_LEFT_ALT:
      return "DPAD_LEFT_ALT";
    case MM_BTN_DPAD_RIGHT_ALT:
      return "DPAD_RIGHT_ALT";
    case MM_BTN_DPAD_UP_ALT:
      return "DPAD_UP_ALT";
    case MM_BTN_DPAD_DOWN_ALT:
      return "DPAD_DOWN_ALT";
    default:
      return "UNKNOWN";
    }
}

#endif /* ! MM_INPUT_H */
