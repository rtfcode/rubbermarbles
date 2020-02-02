/*
 * Rubber Marbles - K Sheldrake
 * rb-vis.c
 *
 * This file is part of rubbermarbles.
 * 
 * Copyright (C) 2016 Kevin Sheldrake <rtfcode at gmail.com>
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar. See the COPYING file or
 * http://www.wtfpl.net/for more details.
 * 
 * Provides functions to load visualisers.
 */

#include "rb-vis.h"

unsigned int visualiser_count = 0;
struct visualiser *visualisers = NULL;


/* load_vis loads a visualiser name and exe from a single line.
 * format for this is:
 * "name" exe
*/
int load_vis(char *str)
{
    char *ptr = str;
    char *ptr2 = NULL;

    if (!str) {
        FAIL_MSG("load_vis: invalid params\n");
        return 1;
    }


    if (visualiser_count >= MAXVISLOAD) {
        FAIL_MSG("load_vis: maximum number of visualisers reached\n");
        return 2;
    }


    if (*str == 0x00) {
        /* blank line */
        return 0;
    }

    /* skip blank space */
    while ((*ptr == ' ') || (*ptr == '\t')) {
        ptr++;
        if (*ptr == 0x00) {
            /* blank line */
            return 0;
        }
    }

    if (*ptr != '"') {
        FAIL_MSG("load_vis: invalid input\n");
        return 3;
    }


    /* find name string */
    ptr++;

    ptr2 = strchr(ptr, '"');
    if (!ptr2) {
        FAIL_MSG("load_vis: invalid input\n");
        return 4;
    }


    if ((ptr2 - ptr) > 255) {
        FAIL_MSG("load_vis: name too long\n");
        return 5;
    }


    /* copy name string to visualiser */
    strncpy(visualisers[visualiser_count].name, ptr, ptr2 - ptr);
    visualisers[visualiser_count].name[ptr2 - ptr] = 0x00;
    /* skip blank space */
    ptr = ptr2 + 1;

    while ((*ptr == ' ') || (*ptr == '\t')) {
        ptr++;
        if (*ptr == 0x00) {
            FAIL_MSG("load_vis: no exe\n");
            return 6;
        }

    }

    /* find string end */
    ptr2 = strchr(ptr, '\n');
    if (!ptr2) {
        FAIL_MSG("load_vis: invalid input\n");
        return 7;
    }


    if ((ptr2 - ptr) > PATH_MAX) {
        FAIL_MSG("load_vis: name too long\n");
        return 8;
    }


    /* copy exe string to visualiser */
    strncpy(visualisers[visualiser_count].exe, ptr, ptr2 - ptr);
    visualisers[visualiser_count].exe[ptr2 - ptr] = 0x00;
    visualiser_count++;

    return 0;
}


/* load_visfile loads the visualiser names and exes from the named config file. */
int load_visfile(char *confpath)
{
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    int nread = 0;

    if (!confpath) {
        FAIL_MSG("load_visfile: invalid params\n");
        return 1;
    }


    /* open file */
    fp = fopen(confpath, "r");
    if (!fp) {
        return -1;
    }

    /* read line by line and load each one */
    while ((nread = getline(&line, &len, fp)) != -1) {
		if (strncmp(line, "vis:", 4) == 0) {
			if (load_vis(line+4) != 0) {
				FAIL_MSG("load_visfile: load_vis() failed\n");
				return 2;
			}
		}
    }

    fclose(fp);
    if (line) {
        free(line);
    }

    return 0;
}


/* load_visualisers loads the visualiser names and exes from one of the config files. */
int load_visualisers(char *homepath)
{
    int ret;
    char userconf[PATH_MAX];

    if (!homepath) {
        FAIL_MSG("load_visualisers: invalid params\n");
        return 1;
    }


    /* initialise the visualisers */
    visualiser_count = 0;
    visualisers = calloc(MAXVISLOAD, sizeof(struct visualiser));

    if (!visualisers) {
        FAIL_MSG("load_visualisers: calloc() failed\n");
        return 2;
    }


    /* attempt to read system wide config file first */
    ret = load_visfile("/etc/rb-vis.conf");

    if (ret > 0) {
        fprintf(stderr, "load_visualisers: load_visfile() failed: %d\n",
                ret);
        return 3;
    }

    /* attempt to read user's own config file */
    snprintf(userconf, PATH_MAX, "%s/.rb-vis.conf", homepath);
    ret = load_visfile(userconf);

    if (ret > 0) {
        fprintf(stderr, "load_visualisers: load_visfile() failed: %d\n",
                ret);
        return 4;
    }

    if (visualiser_count == 0) {
        FAIL_MSG("load_visualisers: no visualisers loaded\n");
        return 5;
    }


    visualisers =
        realloc(visualisers, visualiser_count * sizeof(struct visualiser));

    if (!visualisers) {
        FAIL_MSG("load_visualisers: realloc() failed\n");
        return 6;
    }


    return 0;
}
