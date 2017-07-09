#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "player.h"

#define C_BLUE "\e[1;34m"
#define C_GREEN "\e[1;32m"
#define C_RED "\e[0;31m"
#define C_YELLOW "\e[0;33m"
#define C_RESET "\e[0m"

struct _MMPlayer
{
  int notes[12];
  int nnotes;
};

MMPlayer *
mm_player_new ()
{
  MMPlayer *player = calloc (1, sizeof (MMPlayer));
  assert (player != NULL);
  return player;
}

void
mm_player_free (MMPlayer *player)
{
  if (player != NULL)
    free (player);
}

void
mm_player_send (MMPlayer *player, int status, int data1, int data2)
{
  if (player == NULL)
    return;

  fprintf (stderr,
           C_RED
           __FILE__
           ": `mm_player_send' (%X, %X, %X) not implemented"
           C_RESET
           "\n",
           status, data1, data2);
}

void
mm_player_play (MMPlayer *player, const MMChord *chord)
{
  int notes[12];
  int nnotes;

  if (player == NULL || chord == NULL)
    return;

  nnotes = mm_chord_get_notes (chord, notes);

  printf ("PLAYING: " C_BLUE "%s" C_RESET "\n    OFF:",
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
          printf (" " C_YELLOW "%d" C_RESET, player->notes[o]);
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
          printf (" " C_GREEN "%d" C_RESET, notes[n]);
          mm_player_send (player, 0x90, notes[n], 0x7F);
        }
    }

  printf ("\n--------\n");

  memcpy (player->notes, notes, sizeof (int) * nnotes);
  player->nnotes = nnotes;
}

void
mm_player_killall (MMPlayer *player)
{
  mm_player_send (player, 0xB0, 0x7B, 0x00);
}
