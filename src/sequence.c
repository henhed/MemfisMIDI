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
  unsigned int loop;
  bool tap;
  int midiprg;
  double bpm;
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
  sequence->loop = 0;
  sequence->tap = false;
  sequence->midiprg = -1;
  sequence->bpm = -1.;
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

unsigned int
mm_sequence_get_loop (const MMSequence *sequence)
{
  return (sequence != NULL) ? sequence->loop : 0;
}

void
mm_sequence_set_loop (MMSequence *sequence, unsigned int loop)
{
  if (sequence != NULL)
    sequence->loop = loop;
}

bool
mm_sequence_get_tap (const MMSequence *sequence)
{
  return (sequence != NULL) ? sequence->tap : 0;
}

void
mm_sequence_set_tap (MMSequence *sequence, bool tap)
{
  if (sequence != NULL)
    sequence->tap = tap;
}

int
mm_sequence_get_midiprg (const MMSequence *sequence)
{
  return (sequence != NULL) ? sequence->midiprg : -1;
}

void
mm_sequence_set_midiprg (MMSequence *sequence, int midiprg)
{
  if (sequence != NULL)
    sequence->midiprg = midiprg;
}

double
mm_sequence_get_bpm (const MMSequence *sequence)
{
  return (sequence != NULL) ? sequence->bpm : -1.;
}

void
mm_sequence_set_bpm (MMSequence *sequence, double bpm)
{
  if (sequence != NULL)
    sequence->bpm = bpm;
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
  if (sequence == NULL || sequence->nchords <= 0)
    return NULL;

  if (sequence->current >= sequence->nchords - 1)
    {
      if (sequence->loop == 0)
        return NULL;
      else
        --sequence->loop;
    }

  sequence->current = (sequence->current + 1) % sequence->nchords;
  return sequence->chords[sequence->current];
}

void
mm_sequence_reset (MMSequence *sequence)
{
  if (sequence != NULL)
    sequence->current = -1;
}

bool
mm_sequence_is_reset (const MMSequence *sequence)
{
  return (sequence != NULL && sequence->current == -1) ? true : false;
}
