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
#include <ctype.h>

#include "chord.h"

enum
{
  MM_DOM = 0,
  MM_MAJ = 1 << 0,
  MM_MIN = 1 << 1,
  MM_DIM = 1 << 2,
  MM_AUG = 1 << 3,
  MM_SUS = 1 << 4
};

struct _MMChord
{
  char *name;
  int root;
  int octave;
  int quality;
  int notes[12];
  bool lift;
  double delay;
  double broken;
};

static int dom_scale[7] = {0, 2, 4, 5, 7, 9, 10};

static int parse_root (const char *, char **);
static int parse_quality (const char *, char **);
static int parse_extension (const char *, char **);
static void set_quality (MMChord *, const char *, char **);
static void set_extension (MMChord *, const char *, char **);
static void add_alteration (MMChord *, const char *, char **);
static void set_bass (MMChord *, const char *, char **);

MMChord *
mm_chord_new (const char *name)
{
  MMChord *chord;
  char *suffix, *endptr = NULL;

  assert (name != NULL);
  chord = calloc (1, sizeof (MMChord));
  assert (chord != NULL);
  chord->name = strndup (name, 32);
  assert (chord->name != NULL);

  chord->root = parse_root (chord->name, &endptr);
  if (endptr == NULL)
    {
      mm_chord_free (chord);
      return NULL;
    }

  chord->octave = 5;
  chord->lift = false;
  chord->delay = 0.;
  chord->broken = 0.;

  suffix = endptr;
  set_quality (chord, suffix, &endptr);
  suffix = endptr;
  set_extension (chord, suffix, &endptr);

  do
    {
      suffix = endptr;
      add_alteration (chord, suffix, &endptr);
    }
  while (suffix != endptr);

  suffix = endptr;
  set_bass (chord, suffix, &endptr);

  return chord;
}

void
mm_chord_free (MMChord *chord)
{
  if (chord != NULL)
    {
      if (chord->name != NULL)
        free (chord->name);
      free (chord);
    }
}

const char *
mm_chord_get_name (const MMChord *chord)
{
  if (chord == NULL)
    return NULL;
  return chord->name;
}

static int
cmp_int (const void *a, const void *b)
{
  int ia = *(int *) a;
  int ib = *(int *) b;
  return (ia > ib) - (ia < ib);
}

int
mm_chord_get_notes (const MMChord *chord, int *notes)
{
  int root, nnotes = 0;

  if (chord == NULL || notes == NULL)
    return nnotes;

  root = (12 * chord->octave) + chord->root;

  for (int i = 0; i < 12; ++i)
    {
      int offset = chord->notes[i];

      if (offset == 0)
        continue;
      else if (offset > 0)
        offset -= 1;

      notes[nnotes++] = root + i + (offset * 12);
    }

  if (nnotes > 1)
    qsort (notes, nnotes, sizeof (int), cmp_int);

  return nnotes;
}

bool
mm_chord_get_lift (const MMChord *chord)
{
  return (chord != NULL && chord->lift == true) ? true : false;
}

void
mm_chord_set_lift (MMChord *chord, bool lift)
{
  if (chord != NULL)
    chord->lift = lift;
}

void
mm_chord_shift_octave (MMChord *chord, int octave)
{
  if (chord == NULL)
    return;

  chord->octave += octave;
  if (chord->octave < 0)
    chord->octave = 0;
  else if (chord->octave > 10)
    chord->octave = 10;
}

void
mm_chord_shift_note_octave (MMChord *chord, int note, int octave)
{
  if (chord == NULL || note < 0 || note > 11 || octave == 0
      || chord->notes[note] == 0)
    return;

  if (chord->notes[note] > 0)
    chord->notes[note] -= 1;

  chord->notes[note] += octave;

  if (chord->notes[note] >= 0)
    chord->notes[note] += 1;
}

double
mm_chord_get_delay (const MMChord *chord)
{
  return (chord != NULL) ? chord->delay : 0.;
}

void
mm_chord_set_delay (MMChord *chord, double delay)
{
  if (chord != NULL && delay >= 0.)
    chord->delay = delay;
}

double
mm_chord_get_broken (const MMChord *chord)
{
  return (chord != NULL) ? chord->broken : 0.;
}

void
mm_chord_set_broken (MMChord *chord, double broken)
{
  if (chord != NULL)
    chord->broken = broken;
}

static int
parse_root (const char *note, char **endptr)
{
  int n = -1;
  char *c = (char *) note;

  switch (tolower (note[0]))
    {
    case 'c':
      n = 0;
      break;
    case 'd':
      n = 2;
      break;
    case 'e':
      n = 4;
      break;
    case 'f':
      n = 5;
      break;
    case 'g':
      n = 7;
      break;
    case 'a':
      n = 9;
      break;
    case 'b':
    case 'h':
      n = 11;
      break;
    default:
      /* Set ENDPTR to NULL to indicate error as -1 could mean Cb, Dbbb etc.  */
      *endptr = NULL;
      return n;
      break;
    }

  for (++c;;++c)
    {
      if (*c == '#')
        ++n;
      else if (tolower (*c) == 'b')
        --n;
      else
        break;
    }

  *endptr = c;

  return n;
}

static int
parse_quality (const char *quality, char **endptr)
{
  if (strncmp (quality, "maj", 3) == 0)
    {
      *endptr = (char *) quality + 3;
      return MM_MAJ;
    }
  else if (strncmp (quality, "mMaj", 4) == 0)
    {
      *endptr = (char *) quality + 4;
      return MM_MIN | MM_MAJ;
    }
  else if (strncmp (quality, "dim", 3) == 0)
    {
      *endptr = (char *) quality + 3;
      return MM_DIM;
    }
  else if (strncmp (quality, "aug", 3) == 0)
    {
      *endptr = (char *) quality + 3;
      return MM_AUG;
    }
  else if (strncmp (quality, "sus", 3) == 0)
    {
      *endptr = (char *) quality + 3;
      return MM_SUS;
    }
  else if (strncmp (quality, "m", 1) == 0)
    {
      *endptr = (char *) quality + 1;
      return MM_MIN;
    }

  *endptr = (char *) quality;
  return MM_DOM;
}

static int
parse_extension (const char *extension, char **endptr)
{
  int i;
  switch (extension[0])
    {
    case '2':
    case '4':
    case '5':
    case '6':
    case '7':
    case '9':
      i = extension[0] - '0';
      break;
    case '1':
      switch (extension[1])
        {
        case '1':
          i = 11;
          break;
        case '3':
          i = 13;
          break;
        default:
          i = 0;
          break;
        }
      break;
    default:
      i = 0;
      break;
    }

  *endptr = (char *) extension + (i > 9 ? 2 : (i > 0 ? 1 : 0));

  return i;
}

static void
set_quality (MMChord *chord, const char *quality, char **endptr)
{
  chord->quality = parse_quality (quality, endptr);
  chord->notes[0] = 1;
  chord->notes[4] = 1;
  chord->notes[7] = 1;

  if (chord->quality & (MM_MIN | MM_DIM | MM_SUS))
    chord->notes[4] = 0;
  if (chord->quality & (MM_MIN | MM_DIM))
    chord->notes[3] = 1;
  if (chord->quality & MM_MAJ)
    chord->notes[11] = 1;
  if (chord->quality & (MM_AUG | MM_DIM))
    chord->notes[7] = 0;
  if (chord->quality & MM_AUG)
    chord->notes[8] = 1;
  if (chord->quality & MM_DIM)
    chord->notes[6] = 1;
}

static void
set_extension (MMChord *chord, const char *extension, char **endptr)
{
  int ext = parse_extension (extension, endptr);
  switch (ext)
    {
    case 13:
      chord->notes[9] = 2;
    case 11:
      chord->notes[5] = 2;
    case 9:
      chord->notes[2] = 2;
    case 7:
      if ((chord->quality & MM_DIM) && ext == 7)
        chord->notes[9] = 1;
      else if (~chord->quality & MM_MAJ)
        chord->notes[10] = 1;
      break;
    case 6:
      chord->notes[9] = 1;
      break;
    case 5:
      if (~chord->quality & MM_AUG)
        chord->notes[4] = 0;
      break;
    case 4:
      if (chord->quality & MM_SUS)
        chord->notes[5] = 1;
      break;
    case 2:
      chord->notes[2] = 1;
      break;
    default:
      if (chord->quality & MM_SUS)
        chord->notes[5] = 1;
      break;
    }
}

static void
add_alteration (MMChord *chord, const char *alt, char **endptr)
{
  char *c = (char *) alt;
  int d, note, offset = 0;
  bool add = false, omit = false;

  *endptr = (char *) alt;

  if (strncmp (c, "add", 3) == 0)
    {
      add = true;
      c += 3;
    }
  else if (strncmp (c, "no", 2) == 0)
    {
      omit = true;
      c += 2;
    }

  while (*c == 'b' || *c == '#')
    {
      offset += (*c == 'b') ? -1 : 1;
      ++c;
    }

  if (!add && !omit && offset == 0)
    return;

  if (*c < '1' || *c > '9')
    return;

  d = *c - '0';
  ++c;
  if (d == 1 && (*c == '1' || *c == '3'))
    {
      d = 10 + (*c - '0');
      ++c;
    }

  note = dom_scale[(d - 1) % 7];
  if (note == 4 && (chord->quality & (MM_MIN | MM_DIM)))
    --note;
  else if (note == 10 && (chord->quality & MM_DIM))
    --note;
  else if (note == 10 && (chord->quality & MM_MAJ))
    ++note;

  if (!add)
    chord->notes[note] = 0;

  d += offset;
  note += offset;
  while (note < 0)
    note += 12;

  if (!omit)
    chord->notes[note % 12] = d > 7 ? 2 : 1;

  *endptr = c;
}

static void
set_bass (MMChord *chord, const char *bass, char **endptr)
{
  char *c = (char *) bass;
  int note;

  *endptr = c;
  if (*c != '/')
    return;

  ++c;
  note = parse_root (c, endptr) - chord->root;
  if (*endptr == NULL)
    {
      *endptr = c;
      return;
    }

  while (note < 0)
    note += 12;

  chord->notes[note % 12] = -1;
}
