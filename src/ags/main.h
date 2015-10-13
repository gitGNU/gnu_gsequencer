/* GSequencer - Advanced GTK Sequencer
 * Copyright (C) 2005-2015 Joël Krähemann
 *
 * This file is part of GSequencer.
 *
 * GSequencer is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GSequencer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GSequencer.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __AGS_MAIN_H__
#define __AGS_MAIN_H__

#define _GNU_SOURCE
#define ALSA_PCM_NEW_HW_PARAMS_API

#ifndef PACKAGE_VERSION
#define AGS_VERSION "0.4.2\0"
#else
#define AGS_VERSION PACKAGE_VERSION
#endif

#define AGS_BUILD_ID "CEST 13-10-2015 04:33\0"
#define AGS_DEFAULT_DIRECTORY ".gsequencer\0"
#define AGS_DEFAULT_CONFIG "ags.conf\0"
#define AGS_PRIORITY (5)
#define AGS_RT_PRIORITY (49)

#endif /*__AGS_MAIN_H__*/
