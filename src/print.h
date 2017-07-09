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

#define MMCB(string) \
  "\e[1;34m" string "\e[0m"
#define MMCG(string) \
  "\e[1;32m" string "\e[0m"
#define MMCR(string) \
  "\e[0;31m" string "\e[0m"
#define MMCY(string) \
  "\e[0;33m" string "\e[0m"

#define MMERR(format, ...)                                    \
  fprintf (stderr, MMCR ("ERROR") " in %s(%d): " format "\n", \
           __FILE__, __LINE__, ##__VA_ARGS__)

#endif /* ! MM_PRINT_H */
