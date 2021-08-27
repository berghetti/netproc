
/*
 *  Copyright (C) 2020-2021 Mayco S. Berghetti
 *
 *  This file is part of Netproc.
 *
 *  Netproc is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SUFIX_H
#define SUFIX_H

enum sufix_types
{
  IEC_BYTE = 0,
  IEC_BIT,
  SI_BYTE,
  SI_BIT,
  IEC_BYTE_TOT,
  IEC_BIT_TOT,
  SI_BYTE_TOT,
  SI_BIT_TOT,
  TOT_SUFIX_SCHEME
};

#define TOT_ELEMENTS_SUFIX 6

static const char *const sufix_schemes[TOT_SUFIX_SCHEME][TOT_ELEMENTS_SUFIX] = {
        [IEC_BYTE] = { "B/s", "KiB/s", "MiB/s", "GiB/s", "TiB/s", "PiB/s" },
        [IEC_BIT] = { "b/s", "Kib/s", "Mib/s", "Gib/s", "Tib/s", "Pib/s" },
        [SI_BYTE] = { "B/s", "KB/s", "MB/s", "GB/s", "TB/s", "PB/s" },
        [SI_BIT] = { "b/s", "Kb/s", "Mb/s", "Gb/s", "Tb/s", "Pb/s" },
        [IEC_BYTE_TOT] = { "B", "KiB", "MiB", "GiB", "TiB", "PiB" },
        [IEC_BIT_TOT] = { "b", "Kib", "Mib", "Gib", "Tib", "Pib" },
        [SI_BYTE_TOT] = { "B", "KB", "MB", "GB", "TB", "PB" },
        [SI_BIT_TOT] = { "b", "Kb", "Mb", "Gb", "Tb", "Pb" } };

#endif  // SUFIX_H
