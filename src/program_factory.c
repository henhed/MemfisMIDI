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

#include <stdbool.h>
#include <string.h>
#include <yaml.h>

#include "program_factory.h"
#include "sequence.h"
#include "print.h"

static bool load_sequence (MMProgram *, yaml_document_t *);
const char *get_sequence_name (yaml_document_t *, yaml_node_t *);
static void load_sequence_properties (MMSequence *, yaml_document_t *,
                                      yaml_node_t *);
static void load_chords (MMSequence *, yaml_document_t *, yaml_node_t *);
static void load_chord_properties (MMChord *, yaml_document_t *, yaml_node_t *);
static bool node_to_string (yaml_node_t *, const char **);
static bool node_to_int (yaml_node_t *, int *);
static bool node_to_float (yaml_node_t *, double *);
static bool node_to_bool (yaml_node_t *, bool *);
static yaml_node_t *get_node_by_key (yaml_document_t *, yaml_node_t *,
                                     const char *);

MMProgram *
mm_program_factory (const char *filename)
{
  MMProgram *program;
  FILE *file;
  yaml_parser_t parser;

  file = fopen (filename, "rb");
  if (file == NULL)
    {
      MMERR ("Failed to open " MMCY ("%s"), filename);
      return NULL;
    }

  if (yaml_parser_initialize (&parser) == 0)
    {
      MMERR ("Failed to initialize YAML parser");
      fclose (file);
      return NULL;
    }

  yaml_parser_set_input_file (&parser, file);

  program = mm_program_new ();

  for (bool eof = false; !eof;) 
    {
      yaml_document_t document;
      if (!yaml_parser_load (&parser, &document))
        {
          MMERR ("Invalid YAML file: " MMCY ("%s"), filename);
          mm_program_free (program);
          yaml_document_delete (&document);
          yaml_parser_delete (&parser);
          fclose (file);
          return NULL;
        }
      eof = !load_sequence (program, &document);
      yaml_document_delete (&document);
    }

  yaml_parser_delete (&parser);
  fclose (file);

  return program;
}

static bool
load_sequence (MMProgram *program, yaml_document_t *doc)
{
  MMSequence *sequence;
  yaml_node_t *root = yaml_document_get_root_node (doc);

  if (root == NULL)
    return false;

  if (root->type != YAML_MAPPING_NODE)
    {
      MMERR ("Root node must be a map (" MMCY ("%d") ") "
             "found type " MMCY ("%d"),
             YAML_MAPPING_NODE, root->type);
      return true;
    }

  sequence = mm_sequence_new (get_sequence_name (doc, root));
  load_sequence_properties (sequence, doc, root);
  mm_program_add (program, sequence);
  load_chords (sequence, doc, root);

  return true;
}

const char *
get_sequence_name (yaml_document_t *doc, yaml_node_t *root)
{
  yaml_node_t *node = get_node_by_key (doc, root, "name");
  if (node == NULL || node->type != YAML_SCALAR_NODE)
    {
      MMERR ("Sequence name missing or not scalar");
      return "Untitled";
    }

  return (const char *) node->data.scalar.value;
}

static void
load_sequence_properties (MMSequence *sequence, yaml_document_t *doc,
                          yaml_node_t *node)
{
  int loop;
  bool tap;
  int prg;
  double bpm;

  if (node_to_int (get_node_by_key (doc, node, "loop"), &loop) && loop >= 0)
    mm_sequence_set_loop (sequence, (unsigned int) loop);

  if (node_to_bool (get_node_by_key (doc, node, "tap"), &tap) && tap == true)
    mm_sequence_set_tap (sequence, tap);

  if (node_to_int (get_node_by_key (doc, node, "program"), &prg) && prg >= 0)
    mm_sequence_set_midiprg (sequence, prg);

  if (node_to_float (get_node_by_key (doc, node, "bpm"), &bpm) && bpm > 0.)
    mm_sequence_set_bpm (sequence, bpm);
}

static void
load_chords (MMSequence *sequence, yaml_document_t *doc, yaml_node_t *root)
{
  yaml_node_t *node = get_node_by_key (doc, root, "chords");
  if (node == NULL || node->type != YAML_SEQUENCE_NODE)
    {
      MMERR ("Chord node is missing or is not a sequence");
      return;
    }

  for (yaml_node_item_t *c = node->data.sequence.items.start;
       c < node->data.sequence.items.top;
       ++c)
    {
      MMChord *chord;
      yaml_node_t *cnode = yaml_document_get_node (doc, *c);
      yaml_node_t *name = cnode;
      if (cnode == NULL)
        continue;

      if (name->type != YAML_SCALAR_NODE)
        name = get_node_by_key (doc, cnode, "name");

      if (name == NULL || name->type != YAML_SCALAR_NODE)
        {
          MMERR ("No scalar chord name found");
          continue;
        }

      chord = mm_chord_new ((const char *) name->data.scalar.value);
      if (chord != NULL)
        {
          if (name != cnode) /* chord node is a map.  */
            load_chord_properties (chord, doc, cnode);
          mm_sequence_add (sequence, chord);
        }
      else
        {
          MMERR ("Could not parse chord " MMCY ("%s"),
                 (const char *) cnode->data.scalar.value);
        }
    }
}

static void
load_chord_voicing (MMChord *chord, yaml_document_t *doc, yaml_node_t *node,
                    bool replace)
{
  if (node != NULL && node->type == YAML_MAPPING_NODE)
    {
      for (yaml_node_pair_t *pair = node->data.mapping.pairs.start;
           pair < node->data.mapping.pairs.top;
           ++pair)
        {
          int note;
          int offset;
          yaml_node_t *knode = yaml_document_get_node (doc, pair->key);
          yaml_node_t *vnode = yaml_document_get_node (doc, pair->value);
          if (node_to_int (knode, &note) && node_to_int (vnode, &offset))
            mm_chord_shift_note_octave (chord, note, offset, replace);
        }
    }
}

static void
load_chord_properties (MMChord *chord, yaml_document_t *doc, yaml_node_t *node)
{
  bool lift;
  int octave;
  double delay;
  double broken;

  if (node_to_bool (get_node_by_key (doc, node, "lift"), &lift) && lift == true)
    mm_chord_set_lift (chord, lift);

  if (node_to_int (get_node_by_key (doc, node, "octave"), &octave))
    mm_chord_shift_octave (chord, octave);

  if (node_to_float (get_node_by_key (doc, node, "delay"), &delay))
    mm_chord_set_delay (chord, delay);

  if (node_to_float (get_node_by_key (doc, node, "break"), &broken))
    mm_chord_set_broken (chord, broken);

  load_chord_voicing (chord, doc, get_node_by_key (doc, node, "voice"), true);
  load_chord_voicing (chord, doc, get_node_by_key (doc, node, "double"), false);
}

static bool
node_to_string (yaml_node_t *node, const char **value)
{
  if (node == NULL || node->type != YAML_SCALAR_NODE)
    return false;
  *value = (const char *) node->data.scalar.value;
  return true;
}

static bool
node_to_bool (yaml_node_t *node, bool *value)
{
  const char *strval = NULL;
  if (!node_to_string (node, &strval))
    return false;

  if (strcmp (strval, "1") == 0
      || strcasecmp (strval, "true") == 0
      || strcasecmp (strval, "yes") == 0
      || strcasecmp (strval, "on") == 0)
    *value = true;
  else if (strcmp (strval, "0") == 0
           || strcasecmp (strval, "false") == 0
           || strcasecmp (strval, "no") == 0
           || strcasecmp (strval, "off") == 0)
    *value = false;
  else
    {
      MMERR ("Invalid boolean expression " MMCY ("%s"), strval);
      return false;
    }

  return true;
}

static bool
node_to_int (yaml_node_t *node, int *value)
{
  long int intval;
  char *endptr;
  const char *strval = NULL;

  if (!node_to_string (node, &strval))
    return false;

  intval = strtol (strval, &endptr, 0);
  if (*endptr == '\0')
    {
      *value = (int) intval;
      return true;
    }
  else
    {
      MMERR ("Invalid integer expression " MMCY ("%s"), strval);
      return false;
    }
}

static bool
node_to_float (yaml_node_t *node, double *value)
{
  double floatval;
  char *endptr;
  const char *strval = NULL;

  if (!node_to_string (node, &strval))
    return false;

  floatval = strtod (strval, &endptr);
  if (*endptr == '\0')
    {
      *value = floatval;
      return true;
    }
  else
    {
      MMERR ("Invalid float expression " MMCY ("%s"), strval);
      return false;
    }
}

static yaml_node_t *
get_node_by_key (yaml_document_t *doc, yaml_node_t *parent, const char *key)
{
  if (doc == NULL || parent == NULL || key == NULL
      || parent->type != YAML_MAPPING_NODE)
    return NULL;

  for (yaml_node_pair_t *pair = parent->data.mapping.pairs.start;
       pair < parent->data.mapping.pairs.top;
       ++pair)
    {
      yaml_node_t *knode = yaml_document_get_node (doc, pair->key);
      if (knode->type != YAML_SCALAR_NODE
          || knode->data.scalar.length != strlen (key))
        continue;

      if (strcmp ((const char *) knode->data.scalar.value, key) == 0)
        return yaml_document_get_node (doc, pair->value);
    }

  return NULL;
}
