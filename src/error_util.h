/* 
 * Vega Strike 
 * Copyright (C) 2001-2002 Daniel Horn
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * This code taken from Tux Racer by Jasmin F. Patry www.tuxracer.com
 */

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef _ERROR_UTIL_H_
#define _ERROR_UTIL_H_

/* Warning levels -- these categories are very vague */
#define CRITICAL_WARNING        1
#define IMPORTANT_WARNING       10
#define MISSING_FILE_WARNING    10
#define TCL_WARNING             20
#define DEPRECATION_WARNING     30
#define CONFIGURATION_WARNING   50
#define PEDANTIC_WARNING        100

void print_warning( int warning_level, char *fmt, ... );
void handle_error( int exit_code, char *fmt, ... );
void handle_system_error( int exit_code, char *fmt, ... );

#endif /* _ERROR_UTIL_H_ */

#ifdef __cplusplus
} /* extern "C" */
#endif
