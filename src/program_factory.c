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
static void load_chords (MMSequence *, yaml_document_t *, yaml_node_t *);
static yaml_node_t *get_node_by_key (yaml_document_t *, yaml_node_t *,
                                     const char *key);

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
      if (cnode == NULL || cnode->type != YAML_SCALAR_NODE)
        {
          MMERR ("Chord is is not a scalar");
          continue;
        }

      chord = mm_chord_new ((const char *) cnode->data.scalar.value);
      if (chord != NULL)
        mm_sequence_add (sequence, chord);
      else
        {
          MMERR ("Could not parse chord " MMCY ("%s"),
                 (const char *) cnode->data.scalar.value);
        }
    }
}

static yaml_node_t *
get_node_by_key (yaml_document_t *doc, yaml_node_t *parent,
                 const char *key)
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

      if (strncmp ((const char *) knode->data.scalar.value,
                   key,
                   knode->data.scalar.length) == 0)
        return yaml_document_get_node (doc, pair->value);
    }

  return NULL;
}
