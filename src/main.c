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

#include <portmidi.h>

#include "app.h"
#include "input.h"
#include "input_joystick.h"
#include "input_midi.h"
#include "player.h"
#include "program.h"
#include "program_factory.h"
#include "print.h"

static PmDeviceID
mm_get_output_device_id (const char *input_name)
{
  for (PmDeviceID id = Pm_CountDevices () - 1; id >= 0; --id)
    {
      const PmDeviceInfo *dev = Pm_GetDeviceInfo (id);
      if ((dev->output == 1) && (strcmp (dev->name, input_name) != 0))
        {
          mm_printf_subtitle ("INPUT / OUTPUT\n"
                              MMCB ("%.32s") "\n" MMCB ("%.32s"),
                              input_name, dev->name);
          return id;
        }
    }
  return pmNoDevice;
}

int
main (int argc, char **argv)
{
  MMApp *app;
  MMInput *input;
  MMPlayer *player;

  PmError err;
  PmDeviceID device;

  if (argc <= 1)
    {
      MMERR ("No input file");
      return EXIT_FAILURE;
    }

  err = Pm_Initialize ();
  if (err < pmNoError)
    {
      MMERR ("Failed to initialize: " MMCY ("%s"), Pm_GetErrorText (err));
      return EXIT_FAILURE;
    }

  mm_clear_screen ();
  mm_printf_subtitle ("Detecting input..");
  mm_input_register_backend (mm_input_joystick_backend);
  mm_input_register_backend (mm_input_midi_backend);
  input = mm_input_autodetect ();
  if (input == NULL)
    {
      MMERR ("No input device found");
      Pm_Terminate ();
      return EXIT_FAILURE;
    }
  mm_clear_screen ();

  device = mm_get_output_device_id (mm_input_get_name (input));
  if (device == pmNoDevice)
    {
      MMERR ("No output device found");
      mm_input_free (input);
      Pm_Terminate ();
      return EXIT_FAILURE;
    }

  player = mm_player_new (device);
  if (player == NULL)
    {
      mm_input_free (input);
      Pm_Terminate ();
      return EXIT_FAILURE;
    }

  app = mm_app_new (input, player);

  for (int arg = 1; arg < argc; ++arg)
    {
      MMProgram *program = mm_program_factory (argv[arg]);
      if (program == NULL)
          continue;

      mm_printf_title ("\n" MMCG ("%s") "\n", argv[arg]);

      mm_app_run (app, program);
      mm_program_free (program);
      mm_clear_screen ();
    }

  mm_app_free (app);

  Pm_Terminate ();

  return EXIT_SUCCESS;
}
