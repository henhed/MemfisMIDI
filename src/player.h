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

#include <portmidi.h>

#include "chord.h"

typedef struct _MMPlayer MMPlayer;

MMPlayer *mm_player_new (PmDeviceID);
void mm_player_free (MMPlayer *);
bool mm_player_send (MMPlayer *, int, int, int, int);
void mm_player_play (MMPlayer *, const MMChord *);
bool mm_player_killall (MMPlayer *);

#endif /* ! MM_PLAYER_H */
