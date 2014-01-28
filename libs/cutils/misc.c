
#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>

#include <cutils/misc.h>

#if 0
size_t
strlcpy(char *dst, const char *src, size_t siz)
{
	char *d = dst;
	const char *s = src;
	size_t n = siz;

	/* Copy as many bytes as will fit */
	if (n != 0 && --n != 0) {
		do {
			if ((*d++ = *s++) == 0)
				break;
		} while (--n != 0);
	}

	/* Not enough room in dst, add NUL and traverse rest of src */
	if (n == 0) {
		if (siz != 0)
			*d = '\0';		/* NUL-terminate dst */
		while (*s++)
			;
	}

	return(s - src - 1);	/* count does not include NUL */
}
#endif
int vfdprintf(int fd, const char * format, va_list ap)
{
        char *buf=0;
        int ret;
        ret = vasprintf(&buf, format, ap);
        if (ret < 0)
                goto end;

        ret = write(fd, buf, ret);
        free(buf);
end:
        return ret;
}

int fdprintf(int fd, const char * format, ...)
{
        va_list ap;
        int ret;

        va_start(ap, format);
        ret = vfdprintf(fd, format, ap);
        va_end(ap);

        return ret;
}
