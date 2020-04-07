/***************************************************************************
 *                           general.c  -  description
 *                           ----------------------------
 *                           begin                : December 28, 2001
 *                           copyright            : (C) 2001 by David Ranger
 *                           email                : reliant@canshell.com
 **************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   any later version.                                                    *
 *                                                                         *
 **************************************************************************/

/* This include has been designed to act independant of the other modules. 
 * This allows it to be used with other programs with minimal changes */

#include "general.h"

int RANDOMIZED = 0;

// Gets the next parameter from the string, sorted by a space
// Sticks a \0 at the space and returns a pointer to the right side.

char *next_parm(char *string) {
	char *next;
	if (string[0] == '\0') { return 0; }
	next = string;
	while (next[0] != ' ' && next[0] != '\0') {
		next++;
	}
	if (next[0] == '\0') { return next; }
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

char *ptr_copy(char *string) {
	char *alloc;
	alloc = (char *)malloc(strlen(string)+1);
	if (alloc == 0) { fprintf(stderr, "Out of memory\n"); exit(-1); }
	strncpy(alloc, string, strlen(string));
	alloc[strlen(string)] = '\0';
	return alloc;
}

/* chomp
 * This function replaces trailing \n (carriage return) with the null character
 */
void chomp(char *line) {
	int current;
	for (current = strlen(line) -1; current>=0&&(line[current] == '\n' || line[current] == ' ' || line[current] == 13); current--) {
		line[current] = '\0';
	}
}
/* pre_chomp
 * This function Removes spaces and = from the front of the line and
 * returns a pointer to the new starting character
 */

char *pre_chomp(char *line) {
	while (line[0] == '=' || line[0] == ' ' || line[0] == 13 || line[0] == 9) { line++; }
	/* printf("Debug: (%c) (%d) %s\n", line[0], line[0], line); */
	return line;
}

/* replace
 * This function searches the first parameter for the second parameter
 * If the second parameter is found, it is replaced with the third parameter
 */

char *replace(char *line, char *search, char *replace, int LENGTH) {
	int  dif, calc;
	char *ptr_new, *location;

	char *chr_new,*current;
	chr_new = (char *) malloc (sizeof (char)*LENGTH);
	current = (char *) malloc (sizeof (char)*LENGTH);
	calc = strlen(line) - strlen(search) + strlen(replace);
	if (calc > LENGTH) { return line; }
	strcpy(current, line);
	while ((location = strstr(current, search)) != NULL) {
		chr_new[0] = '\0';
		calc = strlen(current) - strlen(search) + strlen(replace);
		if (calc > LENGTH) { strcpy(line, current); free(current); free(chr_new); return line; }
		dif = location - current;
		strncat(chr_new, current, dif);
		strncat(chr_new, replace, strlen(replace));
		ptr_new = current + dif + strlen(search);
		strncat(chr_new, ptr_new, strlen(ptr_new));
		strcpy(current, chr_new); 
	}
	strcpy(line, current);
	free(chr_new);
	free(current);
	return line;
}

char *strmov(char *to, char *from) {
	char *end;
	strcpy(to, from);
	end = to + strlen(to);
	return end;
}

/* lower
 * This function makes sure all characters are in lower case
 */

void lower(char *line) {
	int current;
	for (current = 0; line[current] != '\0'; current++) {
		if (line[current] >= 65 && line[current] <= 90) {
			line[current] += 32;
		}
	}
}

void strappend(char *dest, char *source, int length) {
	int DoLength;
	DoLength = length - strlen(dest);
	strncat(dest, source, DoLength);
	return;
}

int randnum(int start, int end) {
	int random, dif, min, max;
	if (RANDOMIZED == 0) { srand(time(NULL)); RANDOMIZED = 1; }
	min = start;
	max = end;
	if (end < start) { min = end; max = start; }
	if (end == start) { return start; }
	dif = max - min + 1;
	random = rand() % dif;
	random += min;
	return random;
}

void randcode(char *line, int length) {
	int current, randomA, randomB, test;
	test = 2;
	for (current = 0; current < length; current++) {
		if (test % 5 == 1 && current > 1) { line[current] = '-'; test++; continue; }
		randomA = randnum(1,2);
		randomB = 60;
		if (randomA == 1) { randomB = randnum(48,57); }
		if (randomA == 2) { randomB = randnum(65,90); }
		line[current] = randomB;
		test++;
	}
	line[length] = '\0';
	return;
}

void vs_itoa(char *line, int number, int length) {
	int current, cur, multiplier, reduce, base;
	base = number;
	cur = 0;
	line[0] = '\0';
	while (1) {
		multiplier = 0;
		current = base;
		while (current / 10 >= 1) { current /= 10; multiplier++; continue; }
		if (base == 0) { break; }
		current /= 10;
		reduce = pwer(1,multiplier);
		current = base / reduce;
		reduce = pwer(current,multiplier);
		current += 48;
		line[cur] = current;
		cur++;
		base -= reduce;
	}
	line[cur] = '\0';
	return;
}

int pwer(int start, int end) {
	int current, val_return;
	val_return = start;
	for (current = 2; current <= end; current++) {
		val_return *= 10;
	}
	return val_return;
}

int pwr(int start, int end) {
	int current, val_return;
	val_return = 1;
	for (current = 2; current <= end; current++) {
		val_return *= start;
	}
	return val_return;
}

#ifdef __cplusplus

double pwer(double start, long end) {
	double current, val_return;
	val_return = start;
	for (current = 2; current <= end; current++) {
		val_return *= start;
	}
	return val_return;
}

#endif

void btoa(char *dest, char *string) {
	int max, cur, pos, new_val;
	char *ptr_char;
	char *new_string= (char *)malloc(strlen(string)+1);
	max = 7;
	cur = 7;
	pos = 0;
	ptr_char = string;
	new_val = 0;
	while (ptr_char[0] != '\0') {
		if (ptr_char[0] == '1') { new_val += pwr(2,cur); }
		cur--;
		if (cur < 0) { cur = max; new_string[pos] = new_val; new_val = 0; pos++; }
		ptr_char++;
	}
	new_string[pos] = '\0';
	strcpy(dest, new_string);
	free (new_string);
	return;
}

#ifdef GLIB

// Some handy wrappers for glib that help error handling which prevent segfaults
char *GetString(GString *line) {
	if (line == 0) { return ""; }
	return line->str;
}

void SetString(GString **ptr, char *line) {
	if (ptr <= 0) { return; }
	if (*ptr <= 0) {
		*ptr = g_string_new(line);
		return;
	}
	g_string_sprintf(*ptr, line);
}

#endif

char *NewString(const char *line) {
	char *new_str;
	new_str = (char *)malloc(strlen(line)+1);
	if (new_str == 0) { fprintf(stderr, "Out of Memory\n"); exit(-1); }
	strcpy(new_str, line);
	new_str[strlen(line)] = '\0';
	return new_str;
}


// Checks for a <!-- and returns a pointer to the next character
// A \0 is placed at the < so that anything in front can be kept
char *xml_pre_chomp_comment(char *string) {
	int length, cur;
	if (string[0] == '<' && string[1] == '!') { string[0] = '\0'; return string += 5; }
	length = strlen(string) - 5;
	for (cur = 0; cur <= length; cur++) {
		if (string[cur] == '<' && string[cur+1] == '!') { string[cur] = '\0'; return &string[cur+5]; }
	}
	return &string[length+5];
}
char *xml_chomp_comment(char *string) {
	int len, cur;
	if (string[0] == '\0') { return string; }
	len = strlen(string) - 1;
	if (len <= 3) { return &string[len+1]; }
	if (string[len] == '>' && string[len-1] == '-' && string[len-2] == '-') {
		if (string[len-3] == ' ') { string[len-3] = '\0'; }
		string[len-2] = '\0';
		return &string[len-2];
	}
	len -= 3;
	for (cur = 0; cur <= len; cur++) {
		if (string[cur] == '-' && string[cur+2] == '>') {
			if (string[cur-1] == ' ') { string[cur-1] = '\0'; }
			string[cur] = '\0';
			string[cur+2] = '\0';
			return &string[cur+3];
		}
	}
	return &string[len+4];
}
