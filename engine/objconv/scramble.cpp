/*
 * Copyright (C) 2001-2022 Daniel Horn, pyramid3d, Stephen G. Tuggy,
 * and other Vega Strike contributors.
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

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("require arg");
        return 0;
    }
    printf("are you sure you wish to nail %s\n", argv[1]);
    char c;
    scanf("%c", &c);
    if (c != 'y' && c != 'Y') {
        return 0;
    }
    int fp = open(argv[1], O_RDWR);
    struct stat s;
    fstat(fp, &s);
    size_t size = s.st_size;
    char *mem = (char *) mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fp, 0);
    int sum = 0;
    FILE *rand = fopen("/dev/random", "r");
    for (size_t i = 0; i < size; ++i) {
        sum += mem[i];
        mem[i] = getc(rand);
    }
    fclose(rand);
    munmap(mem, size);
    close(fp);
    return 0;
}

