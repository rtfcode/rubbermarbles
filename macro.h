/*
 * Rubber Marbles - K Sheldrake
 * macro.h
 *
 * This file is part of rubbermarbles.
 * 
 * Copyright (C) 2016 Kevin Sheldrake <rtfcode at gmail.com>
 * This work is free. You can redistribute it and/or modify it under the
 * terms of the Do What The Fuck You Want To Public License, Version 2,
 * as published by Sam Hocevar. See the COPYING file or
 * http://www.wtfpl.net/for more details.
 * 
 * Provides macros to make error checking easier (to read).
 */


#include <stdio.h>

#ifndef _MACRO_H
#define _MACRO_H


/* macros to print error messages.
 * FAIL_MSG(msg) prints the message to stderr.
 * FAIL_ERR(msg) prints the message using perror.
 * Both also add the line number, function name and filename.
 * NOTE: Be careful with FAIL_ERR as it is a multi-statement replacement;
 * e.g. don't use FAIL_ERR() as a single statement to an if (that is, if you
 * want it to work.)
 */
#define FAIL_MSG(MSG) fprintf(stderr, "%s. %s: %s: %d\n", MSG, __FILE__, __func__, __LINE__);
#define FAIL_ERR(MSG) fprintf(stderr, "%s: %s: %d\n", __FILE__, __func__, __LINE__); perror(MSG);


#endif
