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

#define MM_TICKTIME 8

typedef void (*MMAppEventHandler) (MMApp *, MMProgram *);

struct _MMApp
{
  bool quit;
  MMInput *input;
  MMPlayer *player;
  MMTimer *timer;
  MMBeat beat;
  MMBeat *trigger;
  MMAppEventHandler event_handlers[MMIE_NUM_TYPES];
};

static void on_tick (MMApp *, MMProgram *);
static void on_quit (MMApp *, MMProgram *);
static void on_killall (MMApp *, MMProgram *);
static void on_next_step (MMApp *, MMProgram *);
static void on_prev_seq (MMApp *, MMProgram *);
static void on_next_seq (MMApp *, MMProgram *);
static void on_tap (MMApp *, MMProgram *);

static int get_event (MMApp *, MMInputEvent *);
static void start_sequence (MMApp *, MMSequence *);

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
  app->trigger = NULL;

  app->event_handlers[MMIE_QUIT] = on_quit;
  app->event_handlers[MMIE_KILLALL] = on_killall;
  app->event_handlers[MMIE_NEXT_STEP] = on_next_step;
  app->event_handlers[MMIE_PREV_SEQ] = on_prev_seq;
  app->event_handlers[MMIE_NEXT_SEQ] = on_next_seq;
  app->event_handlers[MMIE_TAP] = on_tap;

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

void
mm_app_run (MMApp *app, MMProgram *program)
{
  MMTimer *timer;

  if (app == NULL || program == NULL)
    return;

  app->quit = false;

  timer = mm_timer_new ();

  on_next_seq (app, program);

  while (!app->quit)
    {
      unsigned int ticktime;
      mm_timer_reset (timer);

      on_tick (app, program);

      ticktime = mm_timer_get_age (timer);
      if (ticktime < MM_TICKTIME)
        mm_sleep (MM_TICKTIME - ticktime);
    }

  mm_timer_free (timer);
}

static inline void
on_tick (MMApp *app, MMProgram *program)
{
  MMInputEvent event;

  mm_player_sync_clock (app->player);

  while (get_event (app, &event) > 0)
    {
      if (event.type < MMIE_NUM_TYPES
          && app->event_handlers[event.type] != NULL)
        app->event_handlers[event.type] (app, program);
      else
        MMERR ("Unhandled input event " MMCY ("%d"), event.type);
    }
}

static void
on_quit (MMApp *app, MMProgram *prg)
{
  app->quit = true;
  on_killall (app, prg);
}

static void
on_killall (MMApp *app, MMProgram *prg)
{
  (void) prg;
  mm_player_killall (app->player);
}

static void
on_next_step (MMApp *app, MMProgram *prg)
{
  MMSequence *seq = mm_program_current (prg);
  MMChord *chord = mm_sequence_next (seq);

  if (chord != NULL)
    {
      double duration = mm_chord_get_duration (chord);
      if (duration > 0.)
        {
          app->trigger = &app->beat;
          mm_player_get_beat (app->player, app->trigger);
          mm_beat_addf (app->trigger, duration);
        }
      else
        app->trigger = NULL;

      if (mm_sequence_get_tap (seq))
        on_tap (app, prg);

      mm_player_play (app->player, chord);
    }
  else
    on_next_seq (app, prg);
}

static void
on_prev_seq (MMApp *app, MMProgram *prg)
{
  MMSequence *seq = mm_program_current (prg);

  if (seq != NULL && mm_sequence_is_reset (seq))
    seq = mm_program_previous (prg);

  if (seq != NULL)
    {
      mm_sequence_reset (seq);
      start_sequence (app, seq);
    }
}

static void
on_next_seq (MMApp *app, MMProgram *prg)
{
  MMSequence *seq = mm_program_next (prg);
  if (seq != NULL)
    start_sequence (app, seq);
  else
    on_quit (app, prg);
}

static void
on_tap (MMApp *app, MMProgram *prg)
{
  double bpm;
  (void) prg;
  mm_timer_tap (app->timer);
  bpm = mm_timer_get_bpm (app->timer);
  if (bpm > 0.)
    mm_player_set_bpm (app->player, bpm);
}

static inline int
get_event (MMApp *app, MMInputEvent *event)
{
  if (app->quit)
    return 0;

  if (app->trigger != NULL)
    {
      int timeout = mm_player_get_time_to_beat (app->player, app->trigger);
      if (timeout < MM_TICKTIME)
        {
          if (timeout > 0)
            mm_sleep (timeout);
          event->type = MMIE_NEXT_STEP;
          app->trigger = NULL;
          return 1;
        }
    }

  return mm_input_read (app->input, event);
}

static void
start_sequence (MMApp *app, MMSequence *seq)
{
  int midiprg = mm_sequence_get_midiprg (seq);
  char midiprgname[5] = "None";
  double bpm = mm_sequence_get_bpm (seq);

  if (midiprg >= 0)
    {
      midiprg &= 0x7F;
      mm_player_send (app->player, 0xC0, midiprg, 0, 0);
      snprintf (midiprgname, 5, "0x%.2X", midiprg);
    }

  mm_printf_subtitle ("%15.15s: " MMCB ("%-15.15s") "\n"
                      "%15.15s: " MMCY ("%-15.15s") "\n"
                      "%15.15s: " MMCY ("x%-14u") "\n"
                      "%15.15s: " MMCY ("%-15.15s"),
                      "SEQUENCE", mm_sequence_get_name (seq),
                      "PROGRAM", midiprgname,
                      "LOOP", mm_sequence_get_loop (seq),
                      "TAP", mm_sequence_get_tap (seq) ? "Yes" : "No");

  if (bpm > 0.)
    {
      mm_player_set_bpm (app->player, bpm);
      mm_timer_reset_tap (app->timer);
    }

  app->trigger = NULL;
}
