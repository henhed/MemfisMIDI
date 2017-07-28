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

#ifndef MM_PLAYER_H
#define MM_PLAYER_H 1

#include <stdbool.h>
#include <math.h>

#include <portmidi.h>

#include "chord.h"

typedef struct _MMPlayer MMPlayer;

typedef struct
{
  int i;
  double f;
} MMBeat;

MMPlayer *mm_player_new (PmDeviceID);
void mm_player_free (MMPlayer *);
bool mm_player_send (MMPlayer *, int, int, int, int);
void mm_player_play (MMPlayer *, const MMChord *);
bool mm_player_killall (MMPlayer *);
void mm_player_set_bpm (MMPlayer *, double);
void mm_player_sync_clock (MMPlayer *);
bool mm_player_get_beat (const MMPlayer *, MMBeat *);
int mm_player_get_time_to_beat (const MMPlayer *, MMBeat *);

static inline void
mm_beat_addf (MMBeat *beat, double addition)
{
  double i;
  beat->f += modf (addition, &i);
  beat->i += (int) i;
  for (; beat->f >= 1.; beat->i += 1, beat->f -= 1.);
}

#endif /* ! MM_PLAYER_H */
