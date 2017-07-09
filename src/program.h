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

#ifndef MM_PROGRAM_H
#define MM_PROGRAM_H 1

#include "sequence.h"

typedef struct _MMProgram MMProgram;

MMProgram *mm_program_new ();
void mm_program_free (MMProgram *);
MMSequence *mm_program_add (MMProgram *, MMSequence *);
MMSequence *mm_program_current (MMProgram *);

#endif /* ! MM_PROGRAM_H */
