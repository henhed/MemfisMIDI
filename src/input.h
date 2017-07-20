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

typedef enum
{
  MMIE_QUIT,
  MMIE_KILLALL,
  MMIE_NEXT_STEP,
  MMIE_TAP
} MMInputEventType;

typedef struct
{
  MMInputEventType type;
  unsigned int timestamp;
} MMInputEvent;

typedef struct
{
  const char *type;
  int id;
  char name[128];
} MMInputDevice;

typedef struct
{
  const char *name;
  void * (*connect) (const MMInputDevice *);
  void (*disconnect) (void *);
  int (*read) (void *, MMInputEvent *);
  size_t (*probe) (MMInputDevice *, size_t);
} MMInputBackend;

MMInput *mm_input_new (const MMInputDevice *);
void mm_input_free (MMInput *);
int mm_input_read (const MMInput *, MMInputEvent *);
bool mm_input_register_backend (const MMInputBackend *);
size_t mm_input_list_devices (MMInputDevice *, size_t);

#endif /* ! MM_INPUT_H */
