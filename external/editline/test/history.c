/* History file round-trip.  read_history()/write_history() must preserve
 * every entry byte-for-byte across a range of history sizes:
 *   - lines of any length (a fixed read buffer used to split long lines and
 *     chop a byte mid-glyph for multibyte input),
 *   - a file filled to capacity -- issue #78 dropped the most recent entry
 *     because read_history() read one fewer than write_history() wrote, and
 *   - a runtime change to el_hist_size, which now (re)sizes the scrollback
 *     buffer (grow or shrink) instead of overflowing a fixed allocation.
 */
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "editline.h"

#define IN  "history-in.tmp"
#define OUT "history-out.tmp"

static int files_equal(const char *a, const char *b)
{
	FILE *fa = fopen(a, "rb");
	FILE *fb = fopen(b, "rb");
	int ca, cb, eq = 1;

	if (!fa || !fb) {
		if (fa) fclose(fa);
		if (fb) fclose(fb);
		return 0;
	}
	do {
		ca = getc(fa);
		cb = getc(fb);
		if (ca != cb) {
			eq = 0;
			break;
		}
	} while (ca != EOF);
	fclose(fa);
	fclose(fb);

	return eq;
}

/* Fill a file to capacity for the given history size (one entry more than
 * el_hist_size, the most write_history() emits), with distinct lines and one
 * 300-byte multibyte line, then require a byte-identical round-trip.
 * Returns 1 on pass, 0 on failure, -1 to skip. */
static int roundtrip(int hsize)
{
	FILE *fp;
	int i, n, ok;

	el_hist_size = hsize;
	n = el_hist_size + 1;

	fp = fopen(IN, "w");
	if (!fp)
		return -1;		/* SKIP: cannot create scratch file */
	for (i = 0; i < n; i++) {
		if (i == n / 2) {
			int k;

			for (k = 0; k < 150; k++)
				fputs("\303\251", fp);
			fputc('\n', fp);
		} else {
			fprintf(fp, "history entry %d\n", i);
		}
	}
	fclose(fp);

	read_history(IN);
	write_history(OUT);
	ok = files_equal(IN, OUT);

	unlink(IN);
	unlink(OUT);

	return ok;
}

int main(void)
{
	/* Default, then shrink, then grow -- exercises the buffer realloc as
	 * well as the fill-to-capacity and long-line cases. */
	int sizes[] = { 64, 8, 200 };
	size_t i, n = sizeof(sizes) / sizeof(sizes[0]);
	int fail = 0;

	for (i = 0; i < n; i++) {
		int rc = roundtrip(sizes[i]);

		if (rc < 0)
			return 77;	/* SKIP */
		if (rc) {
			printf("PASS history-roundtrip  [%d entries]\n", sizes[i] + 1);
		} else {
			fprintf(stderr, "FAIL history-roundtrip  %d-entry file not preserved\n",
				sizes[i] + 1);
			fail++;
		}
	}

	printf("\nhistory: %zu tests, %d failures\n", n, fail);
	return fail ? 1 : 0;
}
