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
#include <string.h>

#include "sequence.h"
#include "print.h"

#define MAX_NUM_CHORDS 64

struct _MMSequence
{
  char *name;
  MMChord *chords[MAX_NUM_CHORDS];
  int nchords;
  int current;
};

MMSequence *
mm_sequence_new (const char *name)
{
  MMSequence *sequence;
  assert (name != NULL);
  sequence = calloc (1, sizeof (MMSequence));
  assert (sequence != NULL);
  sequence->name = strndup (name, 32);
  assert (sequence->name != NULL);
  sequence->current = -1;
  return sequence;
}

void
mm_sequence_free (MMSequence *sequence)
{
  if (sequence != NULL)
    {
      if (sequence->name != NULL)
        free (sequence->name);
      for (int i = 0; i < sequence->nchords; ++i)
        mm_chord_free (sequence->chords[i]);
      free (sequence);
    }
}

const char *
mm_sequence_get_name (const MMSequence *sequence)
{
  if (sequence == NULL)
    return NULL;
  return sequence->name;
}

MMChord *
mm_sequence_add (MMSequence *sequence, MMChord *chord)
{
  if (sequence == NULL || chord == NULL)
    return NULL;

  if (sequence->nchords == MAX_NUM_CHORDS)
    {
      MMERR ("Maximum of " MMCY ("%d") " chords reached", MAX_NUM_CHORDS);
      return NULL;
    }

  sequence->chords[sequence->nchords++] = chord;

  return chord;
}

MMChord *
mm_sequence_next (MMSequence *sequence)
{
  if (sequence == NULL || sequence->nchords == 0)
    return NULL;

  sequence->current = (sequence->current + 1) % sequence->nchords;
  return sequence->chords[sequence->current];
}
