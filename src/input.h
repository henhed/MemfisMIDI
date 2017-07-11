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

enum
{
  MM_INPUT_PREV_SEQUENCE = 5,
  MM_INPUT_NEXT_SEQUENCE = 7,
  MM_INPUT_PREV_CHORD = 4,
  MM_INPUT_NEXT_CHORD = 6,
  MM_INPUT_KILL_ALL = 2,
  MM_INPUT_QUIT = 9
};

MMInput *mm_input_new ();
void mm_input_free (MMInput *);
bool mm_input_connect (MMInput *, const char *);
void mm_input_disconnect (MMInput *);
int mm_input_read (const MMInput *);

#endif /* ! MM_INPUT_H */
