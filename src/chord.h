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

#ifndef MM_CHORD_H
#define MM_CHORD_H 1

#include <stdbool.h>

typedef struct _MMChord MMChord;

MMChord *mm_chord_new (const char *);
void mm_chord_free (MMChord *);
const char *mm_chord_get_name (const MMChord *);
int mm_chord_get_notes (const MMChord *, int *);
bool mm_chord_get_lift (const MMChord *);
void mm_chord_set_lift (MMChord *, bool);
void mm_chord_shift_octave (MMChord *, int);
void mm_chord_shift_note_octave (MMChord *, int, int);

#endif /* ! MM_CHORD_H */
