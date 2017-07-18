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

#ifndef MM_SEQUENCE_H
#define MM_SEQUENCE_H 1

#include <stdbool.h>
#include "chord.h"

typedef struct _MMSequence MMSequence;

MMSequence *mm_sequence_new (const char *);
void mm_sequence_free (MMSequence *);
const char *mm_sequence_get_name (const MMSequence *);
unsigned int mm_sequence_get_loop (const MMSequence *);
void mm_sequence_set_loop (MMSequence *, unsigned int);
bool mm_sequence_get_tap (const MMSequence *);
void mm_sequence_set_tap (MMSequence *, bool);
int mm_sequence_get_midiprg (const MMSequence *);
void mm_sequence_set_midiprg (MMSequence *, int);
double mm_sequence_get_bpm (const MMSequence *);
void mm_sequence_set_bpm (MMSequence *, double);
MMChord *mm_sequence_add (MMSequence *, MMChord *);
MMChord *mm_sequence_next (MMSequence *);

#endif /* ! MM_SEQUENCE_H */
