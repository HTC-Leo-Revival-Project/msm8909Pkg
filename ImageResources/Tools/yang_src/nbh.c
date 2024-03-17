/* 
 * HTCFlasher - http://htc-flasher.googlecode.com
 *
 * Copyright (C) 2007-2008 Pau Oliva Fora - pof @ <eslack.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * opinion) any later version. See <http://www.gnu.org/licenses/gpl.html>
 *
 */

#include <stdio.h>
#include <stdlib.h>

/* bufferedReadWrite - read from input file, write on output file */
int bufferedReadWrite(FILE *input, FILE *output, unsigned long length)
{
	unsigned char data[2048];
	unsigned long nread;

	while (length > 0) {
		nread = length;
		if (nread > sizeof(data))
			nread = sizeof(data);
		nread = fread(data, 1, nread, input);
		if (!nread)
			return 0;
		if (fwrite(data, 1, nread, output) != nread)
			return 0;
		length -= nread;
	}
	return 1;
}