/*	$OpenBSD: helper.c,v 1.9 2010/01/08 13:30:21 oga Exp $	*/

/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <phk@login.dkuug.dk> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Poul-Henning Kamp
 * ----------------------------------------------------------------------------
 */

#include <config.h>

#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <sha2.h>

#ifndef MIN
# define MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif

/* ARGSUSED */
char *
SHA384End(SHA2_CTX *ctx, char *buf)
{
	int i;
	uint8_t digest[SHA384_DIGEST_LENGTH];
	static const char hex[] = "0123456789abcdef";

	if (buf == NULL && (buf = malloc(SHA384_DIGEST_STRING_LENGTH)) == NULL)
		return (NULL);

	SHA384Final(digest, ctx);
	for (i = 0; i < SHA384_DIGEST_LENGTH; i++) {
		buf[i + i] = hex[digest[i] >> 4];
		buf[i + i + 1] = hex[digest[i] & 0x0f];
	}
	buf[i + i] = '\0';
	memset(digest, 0, sizeof(digest));
	return (buf);
}

char *
SHA384FileChunk(const char *filename, char *buf, off_t off, off_t len)
{
	struct stat sb;
	uint8_t buffer[BUFSIZ];
	SHA2_CTX ctx;
	int fd, save_errno;
	ssize_t nr;

	SHA384Init(&ctx);

	if ((fd = open(filename, O_RDONLY)) < 0)
		return (NULL);
	if (len == 0) {
		if (fstat(fd, &sb) == -1) {
			close(fd);
			return (NULL);
		}
		len = sb.st_size;
	}
	if (off > 0 && lseek(fd, off, SEEK_SET) < 0) {
		close(fd);
		return (NULL);
	}

	while ((nr = read(fd, buffer, MIN((off_t)sizeof(buffer), len))) > 0) {
		SHA384Update(&ctx, buffer, (size_t)nr);
		if (len > 0 && (len -= nr) == 0)
			break;
	}

	save_errno = errno;
	close(fd);
	errno = save_errno;
	return (nr < 0 ? NULL : SHA384End(&ctx, buf));
}

char *
SHA384File(const char *filename, char *buf)
{
	return (SHA384FileChunk(filename, buf, (off_t)0, (off_t)0));
}

char *
SHA384Data(const uint8_t *data, size_t len, char *buf)
{
	SHA2_CTX ctx;

	SHA384Init(&ctx);
	SHA384Update(&ctx, data, len);
	return (SHA384End(&ctx, buf));
}
