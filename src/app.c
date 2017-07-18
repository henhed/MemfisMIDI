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
#include <stdbool.h>

#include "app.h"
#include "timer.h"
#include "print.h"

struct _MMApp
{
  bool quit;
  MMInput *input;
  MMPlayer *player;
  MMTimer *timer;
};

MMApp *
mm_app_new (MMInput *input, MMPlayer *player)
{
  MMApp *app;

  assert (input != NULL);
  assert (player != NULL);

  app = calloc (1, sizeof (MMApp));
  assert (app != NULL);

  app->quit = false;
  app->input = input;
  app->player = player;
  app->timer = mm_timer_new ();

  return app;
}

void
mm_app_free (MMApp *app)
{
  if (app != NULL)
    {
      mm_input_free (app->input);
      mm_player_free (app->player);
      mm_timer_free (app->timer);
      free (app);
    }
}

static void
mm_app_tap (MMApp *app)
{
  double bpm;
  mm_timer_tap (app->timer);
  bpm = mm_timer_get_bpm (app->timer);
  if (bpm > 0.)
    mm_player_set_bpm (app->player, bpm);
}

static MMSequence *
mm_app_get_sequence (MMApp *app, MMProgram *program, bool progress)
{
  MMSequence *seq = mm_program_current (program);
  if (seq == NULL || progress == true)
    {
      int midiprg;
      char midiprgname[5] = "None";

      seq = mm_program_next (program);
      if (seq == NULL)
        {
          app->quit = true;
          mm_player_killall (app->player);
          return NULL;
        }

      midiprg = mm_sequence_get_midiprg (seq);
      if (midiprg >= 0)
        {
          midiprg &= 0x7F;
          mm_player_send (app->player, 0xC0, midiprg, 0, 0);
          snprintf (midiprgname, 5, "0x%.2X", midiprg);
        }

      mm_printf_subtitle ("%20.20s: " MMCB ("%-20.20s") "\n"
                          "%20.20s: " MMCY ("%-20.20s") "\n"
                          "%20.20s: " MMCY ("x%-19u") "\n"
                          "%20.20s: " MMCY ("%-20.20s"),
                          "SEQUENCE", mm_sequence_get_name (seq),
                          "PROGRAM", midiprgname,
                          "LOOP", mm_sequence_get_loop (seq),
                          "TAP", mm_sequence_get_tap (seq) ? "Yes" : "No");

      mm_player_set_bpm (app->player, mm_sequence_get_bpm (seq));
    }
  return seq;
}

static inline void
mm_app_tick (MMApp *app, MMProgram *program)
{
  int event;
  MMChord *chord;
  MMSequence *seq = mm_app_get_sequence (app, program, false);

  while ((event = mm_input_read (app->input)) >= 0)
    {
      switch (event)
        {
        case MM_BTN_SELECT:
          app->quit = true;
          /* break intentionally omitted.  */
        case MM_BTN_TL:
          mm_player_killall (app->player);
          break;
        case MM_BTN_TR:
          chord = mm_sequence_next (seq);
          if (chord != NULL)
            {
              if (mm_sequence_get_tap (seq))
                mm_app_tap (app);
              mm_player_play (app->player, chord);
            }
          else
            seq = mm_app_get_sequence (app, program, true);
          break;
        case MM_BTN_Y:
          mm_app_tap (app);
          break;
        default:
          MMERR ("Unhandled input event " MMCY ("%s"), mm_btn_name (event));
          break;
        }
    }
}

void
mm_app_run (MMApp *app, MMProgram *program)
{
  MMTimer *timer;

  if (app == NULL || program == NULL)
    return;

  app->quit = false;

  timer = mm_timer_new ();

  while (!app->quit)
    {
      unsigned int ticktime;
      mm_timer_reset (timer);

      mm_app_tick (app, program);

      ticktime = mm_timer_get_age (timer);
      if (ticktime < 10)
        mm_sleep (10 - ticktime);
    }

  mm_timer_free (timer);
}
