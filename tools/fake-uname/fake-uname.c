#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static size_t strlcpy(char *dst, const char *src, size_t dsize);
static void override(char *buf, size_t bufsize, const char *ident);

int uname(struct utsname *buf)
{
    int err = syscall(SYS_uname, buf);
    if (err) {
        return err;
    }

    override(buf->release, sizeof(buf->release), "RELEASE");
    override(buf->sysname, sizeof(buf->sysname), "SYSNAME");
    override(buf->version, sizeof(buf->version), "VERSION");
    override(buf->machine, sizeof(buf->machine), "MACHINE");

    return 0;
}


static
void
override(char *buf, size_t bufsize, const char *ident)
{
    char namebuf[256] = { '\0' };
    
    snprintf(namebuf, sizeof(namebuf), "FAKE_UNAME_%s_FILE", ident);
    const char *fpath = secure_getenv(namebuf);
    if (fpath != NULL) {
        int fd = open(fpath, O_RDONLY);
        if (fd != -1) {
            memset(buf, 0, bufsize);
            read(fd, buf, bufsize-1);
            close(fd);
        }
    }

    snprintf(namebuf, sizeof(namebuf), "FAKE_UNAME_%s", ident);
    const char *value = secure_getenv(namebuf);
    if (value != NULL) {
        memset(buf, 0, bufsize); // mostly over caution
        strlcpy(buf, value, bufsize);
    }
}


/*	$OpenBSD: strlcpy.c,v 1.16 2019/01/25 00:19:25 millert Exp $	*/

/*
 * Copyright (c) 1998, 2015 Todd C. Miller <millert@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

//#include <sys/types.h>
//#include <string.h>

/*
 * Copy string src to buffer dst of size dsize.  At most dsize-1
 * chars will be copied.  Always NUL terminates (unless dsize == 0).
 * Returns strlen(src); if retval >= dsize, truncation occurred.
 */
static
size_t
strlcpy(char *dst, const char *src, size_t dsize)
{
	const char *osrc = src;
	size_t nleft = dsize;

	/* Copy as many bytes as will fit. */
	if (nleft != 0) {
		while (--nleft != 0) {
			if ((*dst++ = *src++) == '\0')
				break;
		}
	}

	/* Not enough room in dst, add NUL and traverse rest of src. */
	if (nleft == 0) {
		if (dsize != 0)
			*dst = '\0';		/* NUL-terminate dst */
		while (*src++)
			;
	}

	return(src - osrc - 1);	/* count does not include NUL */
}
// DEF_WEAK(strlcpy);
