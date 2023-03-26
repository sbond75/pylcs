/* domnode - simple XML DOM-like interface
 * Copyright (c) 2002 Michael B. Allen <mba2000 ioplex.com>
 *
 * The MIT License
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "mba/msgno.h"
#include "mba/iterator.h"
#include "mba/stack.h"
#include "mba/linkedlist.h"
#include "mba/text.h"
#include "mba/allocator.h"
#include "mba/domnode.h"
#include "defines.h"

#if HAVE_LANGINFO > 0
  #include <langinfo.h>
#endif

#if HAVE_EXPAT > 130
  #include <expat.h>
#elif HAVE_EXPAT > 0
  #include <xmlparse.h>
#endif

#if HAVE_ENCDEC > 0
  #include <encdec.h>
#endif

struct user_data {
	tchar *buf;
	unsigned int buflen;
	struct stack stk;
	int err;
	struct allocator *al;
};

size_t utf8tods(const char *src, size_t sn, struct user_data *ud);

static int
_domnode_del(void *context, void *obj)
{
	int ret = 0;

	if (obj) {
		struct allocator *al = context;
		struct domnode *dn = obj;

		ret += domnode_destroy(dn);
		ret += allocator_free(al, dn);
		if (ret != 0) {
			AMSG("");
		}
	}

	return ret ? -1 : 0;
}
int
domnode_create(struct domnode *dn,
	const tchar *name,
	const tchar *value,
	struct allocator *al)
{
	if (dn == NULL) {
		errno = EINVAL;
		PMNO(errno);
		return -1;
	}

	memset(dn, 0, sizeof *dn);

	dn->al = al;

	if (name && text_copy_new(name, name + 255, (tchar **)&dn->name, 255, dn->al) < 1) {
		PMNO(errno);
		return -1;
	}
	if (value && text_copy_new(value, value + 255, (tchar **)&dn->value, 255, dn->al) < 1) {
		PMNO(errno);
		allocator_free(dn->al, (tchar *)dn->name);
		return -1;
	}

	if (value == NULL &&
			((dn->children = allocator_alloc(dn->al, sizeof *dn->children, 0)) == NULL ||
			linkedlist_init(dn->children, 0, dn->al) == -1 ||
			(dn->attrs = allocator_alloc(al, sizeof *dn->attrs, 0)) == NULL ||
			linkedlist_init(dn->attrs, 0, al) == -1)) {
		AMSG("");
		domnode_destroy(dn);
		return -1;
	}

	return 0;
}
int
domnode_destroy(struct domnode *dn)
{
	int ret = 0;

	if (dn) {
		if (dn->children) {
			ret += linkedlist_deinit(dn->children, _domnode_del, dn->al);
			ret += linkedlist_deinit(dn->attrs, _domnode_del, dn->al);
			ret += allocator_free(dn->al, dn->children);
			ret += allocator_free(dn->al, dn->attrs);
		} else {
			ret += allocator_free(dn->al, (tchar *)dn->value);
		}
		ret += allocator_free(dn->al, (tchar *)dn->name);
		if (ret != 0) {
			AMSG("");
		}
	}

	return ret ? -1 : 0;
}
struct domnode *
domnode_new(const tchar *name, const tchar *value, struct allocator *al)
{
	struct domnode *dn;

	if ((dn = allocator_alloc(al, sizeof *dn, 0)) == NULL ||
			domnode_create(dn, name, value, al) == -1) {
		AMSG("");
		return NULL;
	}

	return dn;
}
int
domnode_del(struct domnode *dn)
{
	int ret = 0;

	if (dn && (ret = _domnode_del(dn->al, dn)) != 0) {
		AMSG("");
	}

	return ret;
}

#if USE_WCHAR

size_t
utf8tods(const char *src, size_t sn, struct user_data *ud)
{
	const char *slim;
	wchar_t *dst, *dlim;
	size_t n;

	if (sn > 0xFFFF) {
		sn = 0xFFFF;
	}
	slim = src + sn;
	dst = ud->buf;
	dlim = ud->buf + ud->buflen;

	while (*src) {
		if (dst == dlim) {
			n = sn + 1;
			ud->buflen = n > (2 * ud->buflen) ? n : (2 * ud->buflen);
			if ((ud->buf = allocator_realloc(ud->al, ud->buf, ud->buflen * sizeof *dst)) == NULL) {
				ud->err = errno;
				PMNO(errno);
				return -1;
			}
			dlim = ud->buf + ud->buflen;
			if (dst == NULL) {
				dst = ud->buf;
			}
		}
		if ((n = utf8towc(src, slim, dst)) == (size_t)-1) {
			ud->err = errno;
			PMNO(errno);
			return -1;
		}
		if (n == 0) {
			break;
		}
		src += n;
		dst++;
	}

	*dst = _T('\0');

	return dst - ud->buf;
}

#elif HAVE_ENCDEC > 0

size_t
utf8tods(const char *src, size_t sn, struct user_data *ud)
{
	size_t n;
	char *s;

	s = (char *)src;
	if ((n = dec_mbsncpy(&s, sn, NULL, -1, -1, "UTF-8")) == (size_t)-1) {
		ud->err = errno;
		PMNO(errno);
		return -1;
	}
	if (n > ud->buflen) {
		ud->buflen = n > (2 * ud->buflen) ? n : (2 * ud->buflen);
		if ((ud->buf = allocator_realloc(ud->al, ud->buf, ud->buflen)) == NULL) {
			ud->err = errno;
			PMNO(errno);
			return -1;
		}
	}
	s = (char *)src;
	if ((n = dec_mbsncpy(&s, sn, ud->buf, ud->buflen, -1, "UTF-8")) == (size_t)-1) {
		ud->err = errno;
		PMNO(errno);
		return -1;
	}
	return n;
}

#else

size_t
utf8tods(const char *src, size_t sn, struct user_data *ud)
{
	size_t n;
	n = strnlen(src, sn) + 1;
	if (n > ud->buflen) {
		ud->buflen = n > (2 * ud->buflen) ? n : (2 * ud->buflen);
		if ((ud->buf = allocator_realloc(ud->al, ud->buf, ud->buflen)) == NULL) {
			ud->err = errno;
			PMNO(errno);
			return -1;
		}
	}
	strncpy(ud->buf, src, n - 1);
	ud->buf[n - 1] = '\0';
	return n;
}

#endif

static void
start_fn(void *userData, const XML_Char *name, const XML_Char **atts)
{
	struct user_data *ud = userData;
	struct domnode *parent, *child, *attr;
	tchar *a;
	int i;

	if (ud->err) {
		return;
	}
	if (ud == NULL || name == NULL || atts == NULL) {
		ud->err = errno = EINVAL;
		PMNO(errno);
		return;
	}

	parent = stack_peek(&ud->stk);
	if (parent == NULL) {
		ud->err = errno = EIO;
		PMNO(errno);
		return;
	}
	if (utf8tods(name, -1, ud) == (size_t)-1) {
		AMSG("");
		return;
	}
	if ((child = domnode_new(ud->buf, NULL, ud->al)) == NULL) {
		ud->err = errno;
		AMNO(EIO);
		return;
	}
	if (linkedlist_add(parent->children, child) == -1) {
		ud->err = errno;
		AMNO(EIO);
		_domnode_del(ud->al, child);
		return;
	}
	for (i = 0; atts[i]; i += 2) {
		if (utf8tods(atts[i], -1, ud) == (size_t)-1) {
			AMSG("");
			return;
		}
		if (text_copy_new(ud->buf, ud->buf + ud->buflen, &a, ud->buflen, ud->al) == -1 || a == NULL) {
			ud->err = EINVAL;
			PMNO(ud->err);
			return;
		}
		if (utf8tods(atts[i + 1], -1, ud) == (size_t)-1) {
			AMSG("");
			allocator_free(ud->al, a);
			return;
		}
		if ((attr = domnode_new(a, ud->buf, ud->al)) == NULL) {
			ud->err = errno;
			AMNO(EIO);
			allocator_free(ud->al, a);
			return;
		}
		allocator_free(ud->al, a);
		if (linkedlist_add(child->attrs, attr) == -1) {
			ud->err = errno;
			_domnode_del(ud->al, attr);
			AMNO(EIO);
			return;
		}
	}
	if (stack_push(&ud->stk, child) == -1) {
		ud->err = errno;
		AMNO(EIO);
		return;
	}
}

static void
end_fn(void *userData, const XML_Char *name)
{
	struct user_data *ud = userData;
	name = NULL;
	stack_pop(&ud->stk);
}
static void
_data_fn(void *userData, const tchar *name, const XML_Char *s, int len)
{
	struct user_data *ud = userData;
	const XML_Char *str;
	struct domnode *parent, *tex;

	if (ud->err) {
		return;
	}
	if (ud == NULL || s == NULL || len == 0) {
		ud->err = errno = EINVAL;
		PMNO(errno);
		return;
	}

	str = s + len;
	while (s < str && isspace(*s)) {
		s++;
		len--;
	}
	if (s == str) {
		return;
	}
	str = s + len - 1;
	while (str > s && isspace(*str)) {
		str--;
		len--;
	}

	parent = stack_peek(&ud->stk);
	if (parent == NULL) {
		ud->err = errno = EIO;
		PMNO(errno);
		return;
	}
	if (utf8tods(s, len, ud) == (size_t)-1) {
		AMSG("");
		return;
	}
	if ((tex = domnode_new(name, ud->buf, ud->al)) == NULL) {
		ud->err = errno;
		AMNO(EIO);
		return;
	}
	if (linkedlist_add(parent->children, tex) == -1) {
		ud->err = errno;
		_domnode_del(ud->al, tex);
		AMNO(EIO);
	}
}
static void
chardata_fn(void *userData, const XML_Char *s, int len)
{
	_data_fn(userData, _T("#text"), s, len);
}
static void
comment_fn(void *userData, const XML_Char *s)
{
	_data_fn(userData, _T("#comment"), s, strlen(s));
}
size_t
domnode_fread(struct domnode *dn, FILE *stream)
{
	XML_Parser p;
	struct user_data ud;
	size_t ret, n;
	int si;
	void *buf;
	int done;
	struct domnode dummy_node, *root;

	if (dn == NULL || stream == NULL) {
		errno = EINVAL;
		PMNF(errno, ": dn=%p,stream=%p", dn, stream);
		return -1;
	}

	if ((p = XML_ParserCreate(NULL)) == NULL) {
		errno = EIO;
		PMNO(errno);
		return -1;
	}

	if (domnode_create(&dummy_node, NULL, NULL, dn->al) == -1) {
		AMNO(EIO);
		XML_ParserFree(p);
		return -1;
	}

	ud.buf = NULL;
	ud.buflen = 0;
	ud.err = 0;
	ud.al = dn->al;

	if ((si = stack_init(&ud.stk, 500, dn->al)) == -1 || stack_push(&ud.stk, &dummy_node) == -1) {
		AMNO(EIO);
		if (si == 0) {
			stack_deinit(&ud.stk, NULL, NULL);
		}
		domnode_destroy(&dummy_node);
		XML_ParserFree(p);
		return -1;
	}

	XML_SetElementHandler(p, start_fn, end_fn);
	XML_SetCharacterDataHandler(p, chardata_fn);
	XML_SetCommentHandler(p, comment_fn);
	XML_SetUserData(p, &ud);

	ret = 0;
	for ( ;; ) {
		if ((buf = XML_GetBuffer(p, BUFSIZ)) == NULL) {
			errno = EIO;
			PMNF(errno, ": buf=NULL");
			ret = -1;
			break;
		}
		if ((n = fread(buf, 1, BUFSIZ, stream)) == 0 && ferror(stream)) {
			errno = EIO;
			PMNO(errno);
			ret = -1;
			break;
		}
		ret += n;
		if (XML_ParseBuffer(p, n, (done = feof(stream))) == 0 || ud.err) {
			if (ud.err == 0) {
				errno = EIO;
				PMNF(errno, ": %s: line %u",
							XML_ErrorString(XML_GetErrorCode(p)),
							XML_GetCurrentLineNumber(p));
			} else {
				AMNO(EIO);
			}
			ret = -1;
			break;
		}
		if (done) {
			break;
		}
	}

	free(ud.buf);
	stack_deinit(&ud.stk, NULL, NULL);
	XML_ParserFree(p);

	root = linkedlist_remove(dummy_node.children, 0);
	if (root) {
		domnode_destroy(dn);
		dn->name = root->name;
		dn->value = NULL;
		dn->children = root->children;
		dn->attrs = root->attrs;
		dn->al = root->al;

		memset(root, 0, sizeof *root);
		allocator_free(root->al, root);
	}

	domnode_destroy(&dummy_node);

	return ret;
}
int
domnode_load(struct domnode *dn, const char *filename)
{
	FILE *fd;

	if (dn == NULL || filename == NULL) {
		errno = EINVAL;
		PMNF(errno, ": dn=%p", dn);
		return -1;
	}

	fd = fopen(filename, "r");
	if (fd == NULL) {
		PMNF(errno, ": %s", filename);
		return -1;
	}

	if (domnode_fread(dn, fd) == (size_t)-1) {
		AMSG("");
		fclose(fd);
		return -1;
	}
	fclose(fd);

	return 0;
}

static size_t
_domnode_fwrite(struct domnode *dn, FILE *stream, int indent)
{
	struct domnode *node;
	size_t ret = 0;
	int i;

    if (dn == NULL || stream == NULL) {
		errno = EINVAL;
		PMNF(errno, ": dn=%p,stream=%p", dn, stream);
		return -1;
    }

	if (errno) {
		AMNO(EIO);
		return -1;
	}

	fputc('\n', stream);
	for (i = 0; i < indent; i++) {
		fputs("    ", stream);
	}

	if (tcscmp(dn->name, _T("#comment")) == 0) {
		fputs("<!--", stream);
		fputts(dn->value, stream);
		fputs("-->", stream);
	} else if (tcscmp(dn->name, _T("#text")) == 0) {
		fputts(dn->value, stream);
	} else {
		iter_t iter;

		fputc('<', stream);
		fputts(dn->name, stream);
		linkedlist_iterate(dn->attrs, &iter);
		while ((node = linkedlist_next(dn->attrs, &iter)) != NULL) {
			fputc(' ', stream);
			fputts(node->name, stream);
			fputs("=\"", stream);
			fputts(node->value, stream);
			fputc('"', stream);
		}
		if (linkedlist_is_empty(dn->children) == 0) {
			fputc('>', stream);
			linkedlist_iterate(dn->children, &iter);
			while ((node = linkedlist_next(dn->children, &iter)) != NULL) {
				if (_domnode_fwrite(node, stream, indent + 1) == (size_t)-1) {
					return -1;
				}
			}
			fputc('\n', stream);
			for (i = 0; i < indent; i++) {
				fputs("    ", stream);
			}
			fputs("</", stream);
			fputts(dn->name, stream);
			fputc('>', stream);
		} else {
			fputs("/>", stream);
		}
	}

	return ret;
}
size_t
domnode_fwrite(struct domnode *dn, FILE *stream)
{
	size_t n;

	fputs("<?xml version=\"1.0", stream);
#ifdef USE_CODESET
	fputs("\" encoding=\"", stream);
	fputs(nl_langinfo(CODESET), stream);
#endif
	fputs("\"?>\n\n", stream);
	if ((n = _domnode_fwrite(dn, stream, 0)) == (size_t)-1) {
		AMSG("");
		return -1;
	}
	fputc('\n', stream);

	return n;
}
int
domnode_store(struct domnode *dn, const char *filename)
{
	FILE *fd;

	if (dn == NULL || filename == NULL) {
		errno = EINVAL;
		PMNF(errno, ": dn=%p", dn);
		return -1;
	}

	if ((fd = fopen(filename, "w")) == NULL) {
		PMNF(errno, ": %s", filename);
		return -1;
	}
	if (domnode_fwrite(dn, fd) == (size_t)-1) {
		fclose(fd);
		AMSG("");
		return -1;
	}
	fclose(fd);

	return 0;
}

struct domnode *
domnode_attrs_get(struct linkedlist *attrs, const tchar *name)
{
	struct domnode *node;
	iter_t iter;

	if (attrs == NULL || name == NULL || *name == _T('\0')) {
		errno = EINVAL;
		PMNF(errno, ": atts=%p,name=%p", attrs, name);
		return NULL;
	}

	linkedlist_iterate(attrs, &iter);
	while ((node = linkedlist_next(attrs, &iter))) {
		if (tcscoll(node->name, name) == 0) {
			return node;
		}
	}

	return NULL;
}
struct domnode *
domnode_search(struct domnode *dn, const tchar *name)
{
	struct domnode *node;
	iter_t iter;

	if (dn->children) {
		linkedlist_iterate(dn->children, &iter);
		while ((node = linkedlist_next(dn->children, &iter))) {
			if (tcscoll(node->name, name) == 0) {
				return node;
			}
		}
	}
	if (dn->attrs) {
		linkedlist_iterate(dn->attrs, &iter);
		while ((node = linkedlist_next(dn->attrs, &iter))) {
			if (tcscoll(node->name, name) == 0) {
				return node;
			}
		}
	}
	if (dn->children) {
		linkedlist_iterate(dn->children, &iter);
		while ((node = linkedlist_next(dn->children, &iter))) {
			if ((node = domnode_search(node, name)) != NULL) {
				return node;
			}
		}
	}

	return NULL;
}
struct domnode *
domnode_attrs_remove(struct linkedlist *attrs, const tchar *name)
{
	struct domnode *node;
	unsigned int idx;
	iter_t iter;

	if (attrs == NULL || name == NULL || *name == _T('\0')) {
		errno = EINVAL;
		PMNF(errno, ": atts=%p,name=%p", attrs, name);
		return NULL;
	}

	linkedlist_iterate(attrs, &iter);
	for (idx = 0; (node = linkedlist_next(attrs, &iter)); idx++) {
		if (tcscoll(node->name, name) == 0) {
			return linkedlist_remove(attrs, idx);
		}
	}

	return NULL;
}
int
domnode_attrs_put(struct linkedlist *attrs, struct domnode *attr)
{
	struct domnode *node;
	unsigned int idx;
	iter_t iter;

	if (attrs == NULL || attr == NULL || attr->value == NULL || attrs->al != attr->al) {
		errno = EINVAL;
		PMNF(errno, ": attrs=%p,attr=%p", attrs, attr);
		return -1;
	}

	linkedlist_iterate(attrs, &iter);
	for (idx = 0; (node = linkedlist_next(attrs, &iter)); idx++) {
		if (tcscoll(node->name, attr->name) == 0) {
			domnode_del(linkedlist_remove(attrs, idx));
			return linkedlist_insert(attrs, idx, attr);
		}
	}
	return linkedlist_add(attrs, attr);
}

