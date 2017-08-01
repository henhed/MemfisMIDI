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

#include <portmidi.h>

#include "input_midi.h"
#include "print.h"

typedef struct {
  PortMidiStream *stream;
  int last_ts;
} MMInputMidi;

static void *
mm_input_midi_connect (const MMInputDevice *device)
{
  MMInputMidi *input;
  PmError err;
  PortMidiStream *stream;

  if (device == NULL)
    return NULL;

  err = Pm_OpenInput (&stream, device->id, NULL, 32, NULL, NULL);
  if (err < pmNoError || stream == NULL)
    {
      MMERR ("MIDI Device " MMCY ("%d") " could not be opened: " MMCY ("%s"),
             device->id, Pm_GetErrorText (err));
      return NULL;
    }

  /* Filter everything except PM_FILT_PROGRAM.  */
  Pm_SetFilter (stream, PM_FILT_ACTIVE | PM_FILT_SYSEX | PM_FILT_CLOCK
                | PM_FILT_PLAY | PM_FILT_TICK | PM_FILT_FD | PM_FILT_UNDEFINED
                | PM_FILT_FD | PM_FILT_RESET | PM_FILT_NOTE
                | PM_FILT_CHANNEL_AFTERTOUCH | PM_FILT_POLY_AFTERTOUCH
                | PM_FILT_CONTROL | PM_FILT_PITCHBEND | PM_FILT_MTC
                | PM_FILT_SONG_POSITION | PM_FILT_SONG_SELECT | PM_FILT_TUNE);

  input = calloc (1, sizeof (MMInputMidi));
  assert (input != NULL);
  input->stream = stream;
  input->last_ts = -1;

  return input;
}

static void
mm_input_midi_disconnect (void *connection)
{
  MMInputMidi *input = (MMInputMidi *) connection;
  if (input == NULL)
    return;

  if (input->stream != NULL)
    Pm_Close (input->stream);

  free (input);
}

static int
mm_input_midi_read (void *connection, MMInputEvent *event)
{
  MMInputMidi *input = (MMInputMidi *) connection;
  PmEvent e;
  int nread;

  if (input == NULL || input->stream == NULL || event == NULL)
    return -1;

  while ((nread = Pm_Read (input->stream, &e, 1)) > 0)
    {
      switch (Pm_MessageStatus (e.message))
        {
        case 0xC0:

          if (e.timestamp < input->last_ts + 10)
            /* Stop feedback loops and double taps.  */
            continue;

          switch (Pm_MessageData1 (e.message))
            {
            case 0x00:
              event->type = MMIE_KILLALL;
              break;
            case 0x02:
              event->type = MMIE_TAP;
              break;
            case 0x03:
              event->type = MMIE_NEXT_STEP;
              break;
            case 0x04:
              event->type = MMIE_QUIT;
              break;
            case 0x06:
              event->type = MMIE_PREV_SEQ;
              break;
            case 0x07:
              event->type = MMIE_NEXT_SEQ;
              break;
            default:
              MMERR ("Unhandled program change " MMCY ("0x%X"),
                     Pm_MessageData1 (e.message));
              continue;
            }

          input->last_ts = e.timestamp;
          event->timestamp = (unsigned int) e.timestamp;
          return 1;

        default:
          MMERR ("Unhandled message status " MMCY ("0x%X, 0x%X, 0x%X"),
                 Pm_MessageStatus (e.message),
                 Pm_MessageData1 (e.message),
                 Pm_MessageData2 (e.message));
          continue;
        }
    }

  if (nread == pmBufferOverflow)
    MMERR ("Input buffer overflow");

  return 0;
}

static size_t
mm_input_midi_probe (MMInputDevice *devices, size_t ndevices)
{
  size_t found = 0;

  if (devices == NULL || ndevices == 0)
    return 0;

  for (int id = 0; id < Pm_CountDevices () && found < ndevices; ++id)
    {
      const PmDeviceInfo *pmdev = Pm_GetDeviceInfo (id);
      if (pmdev != NULL && pmdev->input == 1)
        {
          MMInputDevice *device = &devices[found++];
          device->type = mm_input_midi_backend->name;
          device->id = id;
          strncpy (device->name, pmdev->name, sizeof (device->name));
        }
    }

  return found;
}

static const MMInputBackend _mm_input_midi_backend = {
  "MIDI",
  mm_input_midi_connect,
  mm_input_midi_disconnect,
  mm_input_midi_read,
  mm_input_midi_probe
};

const MMInputBackend *mm_input_midi_backend = &_mm_input_midi_backend;
