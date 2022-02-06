/*
 * Copyright (C) 2001-2022 Daniel Horn, David Ranger, pyramid3d,
 * Stephen G. Tuggy, and other Vega Strike contributors.
 *
 * https://github.com/vegastrike/Vega-Strike-Engine-Source
 *
 * This file is part of Vega Strike.
 *
 * Vega Strike is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Vega Strike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Vega Strike. If not, see <https://www.gnu.org/licenses/>.
 */

/* This include has been designed to act independant of the other modules.
 * This allows it to be used with other programs with minimal changes */

#include "general.h"

int RANDOMIZED = 0;

// Gets the next parameter from the string, sorted by a space
// Sticks a \0 at the space and returns a pointer to the right side.

char *next_parm(char *string)
{
    char *next;
    if (string[0] == '\0') {
        return 0;
    }
    next = string;
    while (next[0] != ' ' && next[0] != '\0') {
        next++;
    }
    if (next[0] == '\0') {
        return next;
    }
    next[0] = '\0';
    next++;
    return next;
}

/*
// Splits the string into an array of char pointers

// Anyone know how to propery return the array?
// Try: char **getbuf() { char **ptrToPtrsToStrings; return ptrToPtrsToStrings; }
// I'll get around to fixing it when I need it

char *split_words(char *string, int max_words) {
	char *array[MAX_ELEM];
	int cur = 0;
	char *parm;

	if (max_words >= MAX_ELEM || max_words <= 0) { max_words = MAX_ELEM; }
	while ((parm = next_parm(string)) != 0) {
		if (cur <= MAX_ELEM) { array[cur] = string; }
		cur++;

		string = parm;
	}
//	printf("Debug: %s - %s - %s\n", array[0], array[1], array[2]);
	return *array;
}
*/

// Allocates memory for a string and returns a pointer to it. Includes a \0 at the end
// Maybe eventually this could be used in memory management

// How efficient is malloc at using recently freed space? Would it be better to allocate memory, and instead
// of freeing it, just mark it for reuse and use an array to keep track of it all? (CPU speed vs Memory speed)

// Might be faster to have malloc handle it since keeping track of the memory used would require extra memory allocated
// everytime memory is allocated to keep track of it. It would only be useful if there's lots of stuff coming and going,
// in which case we just leave flags in the linked lists to say wether or not this link is ready for re-use.

// What about variable clean-up? Keeping track of everything allocated so it can free it up at the end.

// Better yet, use glib

char *ptr_copy(const char *string)
{
#if _XOPEN_SOURCE >= 500 || _POSIX_C_SOURCE >= 200809L
    return strdup(string);
#elif defined (_WINDOWS)
    return _strdup(string);
#else
    size_t buf_size = strlen(string) + 1;
    char *alloc;
    alloc = (char *)malloc(buf_size);
    if (alloc == nullptr)
    {
        fprintf(stderr, "Out of memory\n");
        fflush(stderr);
        exit(-1);
    }
    strncpy(alloc, string, buf_size);
    alloc[buf_size - 1] = '\0';
    return alloc;
#endif
}

/* chomp
 * This function replaces trailing \n (carriage return) with the null character
 */
void chomp(char *line)
{
    int
            current; //[MSVC-Warn] size_t vs (signed) int -- however, cannot convert to size_t without adding additional logic check re: strlen !=0 -- TODO -- fix this in another patch
    for (current = strlen(line) - 1;
         current >= 0 && (line[current] == '\n' || line[current] == ' ' || line[current] == 13);
         current--) //TODO[String Safety] -- strlen assumes null terminated string
    {
        line[current] = '\0';
    }
}

/* pre_chomp
 * This function Removes spaces and = from the front of the line and
 * returns a pointer to the new starting character
 */

char *pre_chomp(char *line)
{
    while (line[0] == '=' || line[0] == ' ' || line[0] == 13 || line[0] == 9) {
        line++;
    }
    /* printf("Debug: (%c) (%d) %s\n", line[0], line[0], line); */
    return line;
}

/* replace
 * This function searches the first parameter for the second parameter
 * If the second parameter is found, it is replaced with the third parameter
 */
/*    --- Seems this function is not used at all.
*char *replace(char *line, char *search, char *replace, int LENGTH) 
*{
*	int  dif, calc;
*	char *ptr_new, *location;
*
*	char *chr_new,*current;
*	chr_new = (char *) malloc (sizeof (char)*LENGTH);
*	current = (char *) malloc (sizeof (char)*LENGTH);
*	calc = strlen(line) - strlen(search) + strlen(replace);
*    if (calc > LENGTH) 
*    {
*        free(chr_new);
*        free(current);
*        return line;
*    }
*	strcpy(current, line);
*	while ((location = strstr(current, search)) != nullptr) 
*        {
*		chr_new[0] = '\0';
*		calc = strlen(current) - strlen(search) + strlen(replace);
*                if (calc > LENGTH) 
*                {
*                    strcpy(line, current);
*                    free(current);
*                    free(chr_new);
*                    return line;
*                }
*		dif = location - current;
*		strncat(chr_new, current, dif);
*		strcat(chr_new, replace);
*		ptr_new = current + dif + strlen(search);
*		strcat(chr_new, ptr_new);
*		strcpy(current, chr_new);
*	}
*	strcpy(line, current);
*	free(chr_new);
*	free(current);
*	return line;
*}
*/
char *strmov(char *to, char *from)
{
    char *end;
    strcpy(to, from); //TODO[String Safety] -- uses naked strcpy and strlen ! //[MSVC-Warn]
    end = to + strlen(to);
    return end;
}

/* lower
 * This function makes sure all characters are in lower case
 */

void lower(char *line)
{
    int current;
    for (current = 0; line[current] != '\0'; current++) {
        if (line[current] >= 65 && line[current] <= 90) {
            line[current] += 32;
        }
    }
}

void strappend(char *dest, char *source, int length)
{
    size_t DoLength;
    DoLength = length - strlen(dest); //TODO[String Safety] -- strlen assumes null terminated string
    strncat(dest, source, DoLength); //[MSVC-Warn]
    return;
}

int randnum(int start, int end)
{
    int random, dif, min, max;
    if (RANDOMIZED == 0) {
        srand((unsigned int) time(NULL));
        RANDOMIZED = 1;
    }
    min = start;
    max = end;
    if (end < start) {
        min = end;
        max = start;
    }
    if (end == start) {
        return start;
    }
    dif = max - min + 1;
    random = rand() % dif;
    random += min;
    return random;
}

void randcode(char *line, int length)
{
    int current, randomA, randomB, test;
    test = 2;
    for (current = 0; current < length; current++) {
        if (test % 5 == 1 && current > 1) {
            line[current] = '-';
            test++;
            continue;
        }
        randomA = randnum(1, 2);
        randomB = 60;
        if (randomA == 1) {
            randomB = randnum(48, 57);
        }
        if (randomA == 2) {
            randomB = randnum(65, 90);
        }
        line[current] = randomB;
        test++;
    }
    line[length] = '\0';
    return;
}

void vs_itoa(char *line, int number, int length)
{
    int current, cur, multiplier, reduce, base;
    base = number;
    cur = 0;
    line[0] = '\0';
    while (1) {
        multiplier = 0;
        current = base;
        while (current / 10 >= 1) {
            current /= 10;
            multiplier++;
            continue;
        }
        if (base == 0) {
            break;
        }
        current /= 10;
        reduce = pwer(1, multiplier);
        current = base / reduce;
        reduce = pwer(current, multiplier);
        current += 48;
        line[cur] = current;
        cur++;
        base -= reduce;
    }
    line[cur] = '\0';
    return;
}

int pwer(int start, int end)
{
    int current, val_return;
    val_return = start;
    for (current = 2; current <= end; current++) {
        val_return *= 10;
    }
    return val_return;
}

int pwr(int start, int end)
{
    int current, val_return;
    val_return = 1;
    for (current = 2; current <= end; ++current) {
        val_return *= start;
    }
    return val_return;
}

#ifdef __cplusplus

double pwer(double start, long end)
{
    double current, val_return;
    val_return = start;
    for (current = 2; current <= end; current++) {
        val_return *= start;
    }
    return val_return;
}

#endif

void btoa(char *dest, char *string)
{
    int max, cur, pos, new_val;
    char *ptr_char;
    char *new_string =
            (char *) malloc(strlen(string) + 1); //TODO[String Safety] -- uses strlen, ergo assumes terminated string
    if (!new_string) {
        fprintf(stderr, "Failed to malloc a small string :-(");
        exit(-1);
    }
    max = 7;
    cur = 7;
    pos = 0;
    ptr_char = string;
    new_val = 0;
    while (ptr_char[0] != '\0') {
        if (ptr_char[0] == '1') {
            new_val += pwr(2, cur);
        }
        cur--;
        if (cur < 0) {
            cur = max;
            new_string[pos] = new_val;
            new_val = 0;
            pos++;
        }
        ptr_char++;
    }
    new_string[pos] = '\0';
    strcpy(dest, new_string); //TODO[String Safety] -- uses naked strcpy //[MSVC-Warn]
    free(new_string);
    return;
}

#ifdef GLIB

// Some handy wrappers for glib that help error handling which prevent segfaults
char *GetString(GString *line) 
{
    if (line == 0)
        {
            return "";
        }
    return line->str;
}

void SetString(GString **ptr, char *line)
{
    if (ptr <= 0)
        {
            return;
        }
    if (*ptr <= 0)
        {
        *ptr = g_string_new(line);
        return;
    }
    g_string_sprintf(*ptr, line);
}

#endif

char *NewString(const char *line)
{
    char *new_str;
    size_t inLength = strlen(line); //TODO[String Safety] -- uses strlen, ergo assumes null terminated string
    new_str = (char *) malloc(inLength + 1);
    if (new_str == 0) {
        fprintf(stderr, "Out of Memory\n");
        exit(-1);
    }
    strncpy(new_str, line, inLength); //[MSVC-Warn]
    new_str[inLength] = '\0';
    return new_str;
}

// Checks for a <!-- and returns a pointer to the next character
// A \0 is placed at the < so that anything in front can be kept
char *xml_pre_chomp_comment(char *string)
{
    int length, cur;
    if (string[0] == '<' && string[1] == '!') {
        string[0] = '\0';
        return string += 5;
    }
    length = strlen(string)
            - 5; //TODO[String Safety] -- strlen assumes null terminated string //[MSVC-Warn] -- size_t vs (signed) int, but need to rewrite code to check for strlen <5 -- fix in another patch
    for (cur = 0; cur <= length; cur++) {
        if (string[cur] == '<' && string[cur + 1] == '!') {
            string[cur] = '\0';
            return &string[cur + 5];
        }
    }
    return &string[length + 5];
}

char *xml_chomp_comment(char *string)
{
    int len, cur;
    if (string[0] == '\0') {
        return string;
    }
    len = strlen(string)
            - 1; //TODO[String Safety] -- strlen assumes null terminated string //[MSVC-Warn] -- should be a size_t -- sanity-check there are no negatives generated by the below and fix in another patch
    if (len <= 3) {
        return &string[len + 1];
    }
    if (string[len] == '>' && string[len - 1] == '-' && string[len - 2] == '-') {
        if (string[len - 3] == ' ') {
            string[len - 3] = '\0';
        }
        string[len - 2] = '\0';
        return &string[len - 2];
    }
    len -= 3;
    for (cur = 0; cur <= len; cur++) {
        if (string[cur] == '-' && string[cur + 2] == '>') {
            if (string[cur - 1] == ' ') {
                string[cur - 1] = '\0';
            }
            string[cur] = '\0';
            string[cur + 2] = '\0';
            return &string[cur + 3];
        }
    }
    return &string[len + 4];
}
