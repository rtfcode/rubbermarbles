/*
 * Rubber Marbles - K Sheldrake
 * rb-conf.c
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

#include "rb-conf.h"


/* get_conf finds the value for the param from the named config file. */
int get_conf(char *confpath, char *param, char *value)
{
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    int nread = 0;
	char p[PATH_MAX];
	int plen;
	int dlen;
	
    if (!confpath || !param || !value || (strlen(param) >= PATH_MAX - 2)) {
        FAIL_MSG("get_conf: invalid params\n");
        return 1;
    }

	/* create param */
	snprintf(p, PATH_MAX, "%s:", param);
	plen = strlen(p);
	
    /* open file */
    fp = fopen(confpath, "r");
    if (!fp) {
        return -1;
    }

    /* read line by line and load each one */
    while ((nread = getline(&line, &len, fp)) != -1) {
		if (strncmp(line, p, plen) == 0) {
			strncpy(value, line+plen, PATH_MAX-1);
			value[PATH_MAX-1] = 0x00;
			
			dlen = strlen(value);
			while ((dlen > 0) && ((value[dlen-1] == 0x0a) || (value[dlen-1] == 0x0d) || (value[dlen-1] == 0x20) || (value[dlen-1] == 0x09))) {
				value[dlen-1] = 0x00;
				dlen--;
			}
			
			return 0;
		}
    }

    fclose(fp);
    if (line) {
        free(line);
    }

	value[0] = 0x00;
    return -2;
}


/* configuration finds the value for a param from one of the config files. */
int configuration(char *homepath, char *param, char *value)
{
    int ret;
    char userconf[PATH_MAX];

    if (!homepath || !param || !value) {
        FAIL_MSG("configuration: invalid params\n");
        return 1;
    }

	value[0] = 0x00;
	
    /* attempt to read system wide config file first */
    ret = get_conf("/etc/rb-vis.conf", param, value);

    if (ret > 0) {
        fprintf(stderr, "configuration: get_conf() failed: %d\n",
                ret);
        return 2;
    }

    /* attempt to read user's own config file */
    snprintf(userconf, PATH_MAX, "%s/.rb-vis.conf", homepath);
    ret = get_conf(userconf, param, value);

    if (ret > 0) {
        fprintf(stderr, "configuration: get_conf() failed: %d\n",
                ret);
        return 3;
    }

    if (value[0] == 0x00) {
        FAIL_MSG("configuration: param not found\n");
        return 4;
    }

    return 0;
}
