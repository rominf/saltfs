#include "string.h"

#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/string.h>

char *vstrcat(char const *first, ...)
{
	size_t len;
	char *retbuf;
	va_list argp;
	char *p;

	if(first == NULL)
		return NULL;

	len = strlen(first);

	va_start(argp, first);

	while((p = va_arg(argp, char *)) != NULL)
		len += strlen(p);

	va_end(argp);

	pr_debug("vstrcat: len=%zu", len);

	retbuf = kmalloc(len + 1, GFP_KERNEL);

	if (retbuf == NULL)
		return NULL;		/* error */

	(void)strcpy(retbuf, first);

	va_start(argp, first);		/* restart; 2nd scan */

	while((p = va_arg(argp, char *)) != NULL)
		(void)strcat(retbuf, p);

	va_end(argp);

	pr_debug("vstrcat: result=%s", retbuf);

	return retbuf;
}
