/*
 * Copyright (c) 2016 Thomas Pornin <pornin@bolet.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be 
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include "brssl.h"
#include "bearssl.h"

static void
usage(void)
{
	fprintf(stderr, "usage: brssl command [ options ]\n");
	fprintf(stderr, "available commands:\n");
	fprintf(stderr, "   client       run SSL client\n");
	fprintf(stderr, "   server       run SSL server\n");
	fprintf(stderr, "   verify       verify certificate chain\n");
	fprintf(stderr, "   skey         decode private key\n");
	fprintf(stderr, "   ta           decode trust anchors\n");
	fprintf(stderr, "   chain        make C code for certificate chains\n");
}

int
main(int argc, char *argv[])
{
	char *cmd;

	if (argc < 2) {
		usage();
		return EXIT_FAILURE;
	}
	cmd = argv[1];
	if (eqstr(cmd, "client")) {
		if (do_client(argc - 2, argv + 2) < 0) {
			return EXIT_FAILURE;
		}
	} else if (eqstr(cmd, "server")) {
		if (do_server(argc - 2, argv + 2) < 0) {
			return EXIT_FAILURE;
		}
	} else if (eqstr(cmd, "verify")) {
		if (do_verify(argc - 2, argv + 2) < 0) {
			return EXIT_FAILURE;
		}
	} else if (eqstr(cmd, "skey")) {
		if (do_skey(argc - 2, argv + 2) < 0) {
			return EXIT_FAILURE;
		}
	} else if (eqstr(cmd, "ta")) {
		if (do_ta(argc - 2, argv + 2) < 0) {
			return EXIT_FAILURE;
		}
	} else if (eqstr(cmd, "chain")) {
		if (do_chain(argc - 2, argv + 2) < 0) {
			return EXIT_FAILURE;
		}
	} else {
		fprintf(stderr, "unknown command: '%s'\n", cmd);
		usage();
		return EXIT_FAILURE;
	}
	return 0;
}
