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
#include <signal.h>

#include "input.h"
#include "print.h"

#define MAX_NUM_BACKENDS 8

static size_t _nbackends = 0;
static const MMInputBackend *_backends[MAX_NUM_BACKENDS];

static bool mm_quit = false;
static void mm_sa_handler (int);

struct _MMInput
{
  MMInputDevice device;
  const MMInputBackend *backend;
  void *connection;
};

MMInput *
mm_input_new (const MMInputDevice *device)
{
  MMInput *input;
  const MMInputBackend *backend = NULL;
  void *connection;
  struct sigaction sa;

  if (device == NULL || device->type == NULL)
    return NULL;

  for (unsigned i = 0; i < _nbackends; ++i)
    {
      if (strcmp (_backends[i]->name, device->type) == 0)
        {
          backend = _backends[i];
          break;
        }
    }

  if (backend == NULL)
    {
      MMERR ("Input backend " MMCY ("%s") " not found", device->type);
      return NULL;
    }

  connection = backend->connect (device);
  if (connection == NULL)
    return NULL;

  input = calloc (1, sizeof (MMInput));
  assert (input != NULL);
  memcpy (&input->device, device, sizeof (MMInputDevice));
  input->backend = backend;
  input->connection = connection;

  sigaction (SIGINT, NULL, &sa);
  if (sa.sa_handler != mm_sa_handler)
    {
      sa.sa_handler = mm_sa_handler;
      sigaction (SIGINT, &sa, NULL);
    }

  return input;
}

void
mm_input_free (MMInput *input)
{
  if (input != NULL)
    {
      input->backend->disconnect (input->connection);
      free (input);
    }
}

int
mm_input_read (const MMInput *input, MMInputEvent *event)
{
  if (mm_quit == true && event != NULL)
    {
      mm_quit = false;
      event->type = MMIE_QUIT;
      return 1;
    }
  else if (input != NULL && event != NULL)
    return input->backend->read (input->connection, event);

  return -1;
}

bool
mm_input_register_backend (const MMInputBackend *backend)
{
  if (backend == NULL || backend->name == NULL || backend->connect == NULL
      || backend->disconnect == NULL || backend->read == NULL
      || backend->probe == NULL)
    {
      MMERR ("Invalid input backend");
      return false;
    }

  for (unsigned i = 0; i < _nbackends; ++i)
    {
      if (_backends[i] == backend
          || strcmp (_backends[i]->name, backend->name) == 0)
        {
          MMERR ("Input backend " MMCY ("%s") " already registered",
                 backend->name);
          return false;
        }
    }

  if (_nbackends == MAX_NUM_BACKENDS)
    {
      MMERR ("Maximum of " MMCY ("%d") " input backends reached",
             MAX_NUM_BACKENDS);
      return false;
    }

  _backends[_nbackends++] = backend;

  return true;
}

size_t
mm_input_list_devices (MMInputDevice *devices, size_t ndevices)
{
  size_t found = 0;

  if (devices == NULL || ndevices == 0)
    return 0;

  for (unsigned i = 0; i < _nbackends && found < ndevices; ++i)
    found += _backends[i]->probe (&devices[found], ndevices - found);

  return found;
}

static void
mm_sa_handler (int sig)
{
  signal (sig, mm_sa_handler);
  mm_quit = true;
}
