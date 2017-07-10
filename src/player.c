#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "player.h"
#include "print.h"

struct _MMPlayer
{
  PortMidiStream *stream;
  int notes[12];
  int nnotes;
};

MMPlayer *
mm_player_new (PmDeviceID device)
{
  MMPlayer *player = NULL;
  PortMidiStream *stream = NULL;
  PmError err;

  err = Pm_OpenOutput (&stream,
                       device,
                       NULL, /* void *outputDriverInfo */
                       1,    /* int32_t bufferSize */
                       NULL, /* PmTimeProcPtr time_proc */
                       NULL, /* void *time_info */
                       0);   /* int32_t latency */
  if (err < pmNoError || stream == NULL)
    {
      MMERR ("MIDI Device " MMCY ("%d") " could not be opened: " MMCY ("%s"),
             device, Pm_GetErrorText (err));
      return NULL;
    }

  player = calloc (1, sizeof (MMPlayer));
  assert (player != NULL);
  player->stream = stream;

  return player;
}

void
mm_player_free (MMPlayer *player)
{
  if (player != NULL)
    {
      Pm_Close (player->stream);
      free (player);
    }
}

bool
mm_player_send (MMPlayer *player, int status, int data1, int data2)
{
  PmError err;
  int message;

  if (player == NULL)
    return false;

  message = Pm_Message (status, data1, data2);
  err = Pm_WriteShort (player->stream, 0, message);
  if (err < pmNoError)
    {
      MMERR ("Message " MMCY ("0x%X") " returned " MMCY ("%s"),
             message, Pm_GetErrorText (err));
      return false;
    }

  return true;
}

void
mm_player_play (MMPlayer *player, const MMChord *chord)
{
  int notes[12];
  int nnotes;

  if (player == NULL || chord == NULL)
    return;

  nnotes = mm_chord_get_notes (chord, notes);

  printf ("PLAYING: " MMCB ("%s") "\n    OFF:",
          mm_chord_get_name (chord));

  for (int o = 0; o < player->nnotes; ++o)
    {
      bool release = true;
      for (int n = 0; n < nnotes; ++n)
        {
          if (notes[n] == player->notes[o])
            {
              release = false;
              break;
            }
        }
      if (release)
        {
          printf (" " MMCY ("%d"), player->notes[o]);
          mm_player_send (player, 0x80, player->notes[o], 0x40);
        }
    }

  printf ("\n     ON:");

  for (int n = 0; n < nnotes; ++n)
    {
      bool press = true;
      for (int o = 0; o < player->nnotes; ++o)
        {
          if (player->notes[o] == notes[n])
            {
              press = false;
              break;
            }
        }
      if (press)
        {
          printf (" " MMCG ("%d"), notes[n]);
          mm_player_send (player, 0x90, notes[n], 0x7F);
        }
    }

  printf ("\n--------\n");

  memcpy (player->notes, notes, sizeof (int) * nnotes);
  player->nnotes = nnotes;
}

bool
mm_player_killall (MMPlayer *player)
{
  if (player == NULL)
    return false;
  player->nnotes = 0;
  memset (player->notes, 0, sizeof (int) * 12);
  return mm_player_send (player, 0xB0, 0x7B, 0x00);
}
