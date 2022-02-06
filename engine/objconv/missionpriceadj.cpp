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

#include <string>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
using namespace std;

bool feq(double a, double b)
{
    if (fabs(a - b) < .0001) {
        return true;
    }
    return false;
}

bool match(char *item, long len, string mymatch, int &finlen)
{
    if (len <= 0) {
        return false;
    }
    if (isspace(*item)) {
        return false;
    }
    char *final = item;
    double rez = strtod(item, &final);
    if (rez != 0) {
        if (feq(rez, strtod(mymatch.c_str(), NULL))) {
            finlen = final - item;
            return true;
        }
    }
    return false;
}

double bestNumber(vector<double> instances)
{
    std::sort(instances.begin(), instances.end());
    unsigned int bestindex = 0;
    unsigned int bestcount = 1;
    unsigned int bestestindex = 0;
    unsigned int bestestcount = 1;
    double number = 0;
    double bestestnum = 0;
    for (unsigned int i = 1; i < instances.size(); ++i) {
        if (feq(instances[i], instances[i - 1])) {
            bestcount++;
        } else {
            if (bestcount > bestestcount || (bestcount == bestestcount && bestestcount == 1)) {
                bestestcount = bestcount;
                bestestindex = bestindex;
                bestestnum = number;
            }
            bestcount = 1;
            number = instances[i];
            bestindex = i;
        }
    }
    printf("best count is %d for %lf", bestestcount, bestestnum);
    if (bestestindex < instances.size()) {
        return instances[bestestindex];
    }
    return 399218092148029;
}

void replaceAll(FILE *fp, char *content, long len, string myname, string myreplacement)
{
    for (long i = 0; i < len;) {
        int ammt = 0;
        if (match(content + i, len - i, myname, ammt)) {
            fwrite(myreplacement.data(), myreplacement.length(), 1, fp);
            i += ammt;
        } else {
            fwrite(content + i, 1, 1, fp);
            i++;
        }
    }
}

vector<double> findNumbers(FILE *fp)
{
    double mynum = 0;
    char mychar;
    vector<double> retval;
    while (!feof(fp)) {
        if (1 == fscanf(fp, "%lf", &mynum)) {
            if (mynum > 800) {
                retval.push_back(mynum);
            }
        } else {
            fscanf(fp, "%c", &mychar);
        }
    }
    return retval;
}

void findNReplace(char *argv, double adjustment)
{
    FILE *fp = fopen(argv, "rb");
    vector<double> instances = findNumbers(fp);
    double mynum = bestNumber(instances);
    double myreplaceme = mynum * adjustment;
    char myname[100];
    sprintf(myname, "%.2lf", mynum);
    char myreplacement[100];
    sprintf(myreplacement, "%.2lf", myreplaceme);
    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *myfile = new char[len + 1];
    myfile[len] = 0;
    fread(myfile, len, 1, fp);
    fclose(fp);
    fp = fopen(argv, "wb");

    replaceAll(fp, myfile, len, myname, myreplacement);
    fclose(fp);
    delete[] myfile;
}

int main(int argc, char **argv)
{
    if (argc <= 1) {
        return -1;
    }
    double adjustment = strtod(argv[1], NULL);
    for (int i = 2; i < argc; ++i) {
        findNReplace(argv[i], adjustment);
    }
    return 0;
}

