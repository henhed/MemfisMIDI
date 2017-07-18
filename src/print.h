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

#ifndef MM_PRINT_H
#define MM_PRINT_H 1

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define MM_VLINE 10

#define MMCB(string) \
  "\e[1;34m" string "\e[21;39m"
#define MMCG(string) \
  "\e[1;32m" string "\e[21;39m"
#define MMCR(string) \
  "\e[0;31m" string "\e[39m"
#define MMCY(string) \
  "\e[0;33m" string "\e[39m"

#define MMERR(format, ...)                                    \
  fprintf (stderr, MMCR ("ERROR") " in %s(%d): " format "\n", \
           __FILE__, __LINE__, ##__VA_ARGS__)

static inline int
mm_screen_width ()
{
  struct winsize size;
  ioctl (STDOUT_FILENO, TIOCGWINSZ, &size);
  return size.ws_col;
}

static inline size_t
mm_string_width (const char *str)
{
  size_t len = 0;
  const char *c;

  if (str == NULL)
    return 0;

  for (c = str; *c != '\0'; ++c)
    {
      /* Skip control sequence.  */
      if (*c == '\e' && *(c + 1) == '[')
        {
          for (c += 2; *c != 'm' && *c != '\0'; ++c);
          if (*c == '\0' || *(c + 1) == '\0')
            break;
          else
            ++c;
        }

      if (isprint (*c) == 0)
        {
          if ((*c & 0xE0) == 0xC0
              && *(c + 1) != '\0')
            {
              c += 1;
              ++len;
            }
          else if ((*c & 0xF0) == 0xE0
                   && *(c + 1) != '\0'
                   && *(c + 2) != '\0')
            {
              c += 2;
              ++len;
            }
          else if ((*c & 0xF8) == 0xF0
                   && *(c + 1) != '\0'
                   && *(c + 2) != '\0'
                   && *(c + 3) != '\0')
            {
              c += 3;
              ++len;
            }
          /* else Non-UTF8.  */
        }
      else /* ASCII.  */
        ++len;
    }

  return len;
}

static inline int
mm_vstrlenf (const char *format, va_list ap)
{
  char c;
  int length;
  va_list aq;

  va_copy (aq, ap);
  length = vsnprintf (&c, 1, format, aq);
  va_end (aq);

  return length;
}

static inline void
mm_print_break (const char *beg, const char *ver, const char *mid,
                const char *end, unsigned int pad)
{
  unsigned int pos, width, blen, vlen, mlen, elen;

  pos = 0;
  width = mm_screen_width ();
  blen = mm_string_width (beg);
  vlen = mm_string_width (ver);
  mlen = mm_string_width (mid);
  elen = mm_string_width (end);

  if (pad * 2 >= width - blen - elen)
    pad = (width / 2) - (blen > elen ? blen : elen);

  for (; pos < pad; ++pos)
    printf (" ");

  printf ("%s", beg);
  pos += blen;

  for (; pos < MM_VLINE; pos += mlen)
    printf ("%s", mid);

  if (pos == MM_VLINE)
    {
      printf ("%s", ver);
      pos += vlen;
    }

  for (; pos < width - elen - pad; pos += mlen)
    printf ("%s", mid);

  printf ("%s\n", end);
}

static inline void
mm_print_centered (const char *str, const char *beg, const char *end,
                   unsigned int pad)
{
  unsigned int pos, width, slen, blen, elen;

  pos = 0;
  width = mm_screen_width ();
  slen = mm_string_width (str);
  blen = mm_string_width (beg);
  elen = mm_string_width (end);

  if (pad * 2 >= width - blen - elen - slen)
    pad = (width / 2) - (slen / 2) - (blen > elen ? blen : elen);

  for (; pos < pad; ++pos)
    printf (" ");

  printf ("%s", beg);
  pos += blen;

  for (; pos < (width / 2) - (slen / 2); ++pos)
    printf (" ");

  printf ("%s", str);
  pos += slen;

  for (; pos < width - elen - pad; ++pos)
    printf (" ");

  printf ("%s\n", end);
}

static inline void
mm_vprintf_box (const char *format, va_list ap, unsigned int pad,
                const char *tl, const char *tv, const char *tm, const char *tr,
                const char *ml, const char *mr,
                const char *bl, const char *bv, const char *bm, const char *br)
{
  int len;
  va_list aq;

  mm_print_break (tl, tv, tm, tr, pad);

  va_copy (aq, ap);
  len = mm_vstrlenf (format, aq);
  if (len < 0)
    {
      va_end (aq);
      mm_print_break (bl, bv, bm, br, pad);
      return;
    }

  char str[len + 1];
  vsprintf (str, format, aq);
  va_end (aq);

  for (char *line = str, *lb = NULL;; line = lb + 1)
    {
      lb = strchr (line, '\n');
      if (lb != NULL)
        *lb = '\0';
      mm_print_centered (line, ml, mr, pad);
      if (lb == NULL)
        break;
    }

  mm_print_break (bl, bv, bm, br, pad);
}

static inline void
mm_printf_title (const char *format, ...)
{
  va_list ap;
  va_start (ap, format);
  mm_vprintf_box (format, ap, 1,
                  "┏", "┷", "━", "┓",
                  "┃",           "┃",
                  "┗", "┯", "━", "┛");
  va_end (ap);
}

static inline void
mm_printf_subtitle (const char *format, ...)
{
  va_list ap;
  va_start (ap, format);
  mm_vprintf_box (format, ap, MM_VLINE,
                  "╰", "", "─", "╮",
                  " ",          "│",
                  "╭", "", "─", "╯");
  va_end (ap);
}

static inline void
mm_print_cmd_end ()
{
  for (int i = 0; i < MM_VLINE; ++i)
    printf ("─");
  printf ("┤\n");
}

static inline void
mm_print_cmd (const char *cmd, bool arg)
{
  printf (arg ? "%*.*s " : MMCY ("%*.*s "), MM_VLINE - 1, MM_VLINE - 1, cmd);
  if (arg)
    printf ("├ ");
  else
    {
      printf ("│\n");
      mm_print_cmd_end ();
    }
}

#endif /* ! MM_PRINT_H */
