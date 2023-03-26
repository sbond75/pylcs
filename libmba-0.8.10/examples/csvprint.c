#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <errno.h>
#include <mba/msgno.h>
#include <mba/csv.h>

#define ROW_MAX 100

int
run(const char *filename, const char *format, int filter, int sep)
{
	FILE *in;
	unsigned char buf[1024], *row[ROW_MAX];
	int n;

	if ((in = fopen(filename, "r")) == NULL) {
		PMNF(errno, ": %s", filename);
		return -1;
	}

	while ((n = csv_row_fread(in, buf, 1024, row, ROW_MAX, sep, CSV_TRIM | CSV_QUOTES)) > 0) {
		const char *fmt = format;
		char outbuf[1024];
		char *out = outbuf;

		if (filter && strcmp(row[0], "0") == 0) {
			continue;
		}

		while (*fmt) {
			if (*fmt == '\\') {
				fmt++;
				switch (*fmt) {
					case '\0':
						return -1;
					case 'n':
						*out++ = '\n';
						break;
					case 't':
						*out++ = '\t';
						break;
					case 'r':
						*out++ = '\r';
						break;
					default:
						*out++ = *fmt;
				}
				fmt++;
			} else if (*fmt == '%') {
				unsigned long i;
				char *endptr;
				fmt++;
				if ((i = strtoul(fmt, &endptr, 10)) == ULONG_MAX) {
					PMNF(errno, ": %s", fmt);
					return -1;
				}
				fmt = endptr;
				if (i < ROW_MAX) {
					const char *s = row[i];
					if (s) {
						while (*s) {
							*out++ = *s++;
						}
					} else {
						*out++ = '-';
					}
				}
			} else {
				*out++ = *fmt++;
			}
		}
		*out = '\0';

		fputs(outbuf, stdout);
	}
	if (n == -1) {
		AMSG("");
		return -1;
	}

	fclose(in);

	return 0;
}

int
main(int argc, char *argv[])
{
	char **args;
	char *filename = NULL;
	char *format = NULL;
	int filter = 0;
	int sep = ',';

	if (argc < 3) {
usage:
		fprintf(stderr, "usage: %s [-f] [-s <sep>] <filename> <format>\n", argv[0]);
		return EXIT_FAILURE;
	}

	errno = 0;

	args = argv;
	args++; argc--;

	while (argc) {
		if (strcmp(*args, "-f") == 0) {
			filter = 1;
		} else if (strcmp(*args, "-s") == 0) {
			args++; argc--;
			sep = **args;
		} else if (filename) {
			if (format) {
				fprintf(stderr, "invalid argument: %s\n", *args);
				goto usage;
			}
			format = *args;
		} else {
			filename = *args;
		}
		args++; argc--;
	}

	if (run(filename, format, filter, sep) == -1) {
		MSG("Run failed.");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
