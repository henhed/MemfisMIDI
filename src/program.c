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

#include "program.h"
#include "sequence.h"
#include "print.h"

#define MAX_NUM_SEQUENCES 64

struct _MMProgram
{
  MMSequence *sequences[MAX_NUM_SEQUENCES];
  int nsequences;
  int current;
};

MMProgram *
mm_program_new ()
{
  MMProgram *program = calloc (1, sizeof (MMProgram));
  assert (program != NULL);
  program->current = -1;
  return program;
}

void
mm_program_free (MMProgram *program)
{
  if (program != NULL)
    {
      for (int i = 0; i < program->nsequences; ++i)
        mm_sequence_free (program->sequences[i]);
      free (program);
    }
}

MMSequence *
mm_program_add (MMProgram *program, MMSequence *sequence)
{
  if (program == NULL || sequence == NULL)
    return NULL;

  if (program->nsequences == MAX_NUM_SEQUENCES)
    {
      MMERR ("Maximum of " MMCY ("%d") " sequences reached", MAX_NUM_SEQUENCES);
      return NULL;
    }

  program->sequences[program->nsequences++] = sequence;

  return sequence;
}

MMSequence *
mm_program_current (const MMProgram *program)
{
  if (program == NULL || program->current < 0
      || program->current >= program->nsequences)
    return NULL;
  return program->sequences[program->current];
}

MMSequence *
mm_program_next (MMProgram *program)
{
  if (program == NULL || program->current < -1
      || program->current >= (program->nsequences - 1))
    return NULL;

  return program->sequences[++program->current];
}

MMSequence *
mm_program_previous (MMProgram *program)
{
  if (program != NULL && program->current > 0)
    --program->current;

  return mm_program_current (program);
}
