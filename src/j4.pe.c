#include <mkdio.h>

#include "kore.h"
#include "http.h"
#include "static.h"

int	serve_index(struct http_request *);
int 	serve_ip(struct http_request *req);
int	serve_qr(struct http_request *);
int 	serve_blog(struct http_request *req);
int 	serve_mkd(struct http_request *req);

static int	page_serve(struct http_request *, u_int8_t *, u_int32_t);
static void	page_replace(struct kore_buf *, char *, u_int8_t *, size_t);

static struct kore_buf	*build_common(struct http_request *, u_int8_t *, u_int32_t);

int serve_index(struct http_request *req) {
	int ret;

	http_response_header_add(req, "content-type", "text/html");
	ret = http_response(req, 200, static_html_index, static_len_html_index);

	return (ret);
}

int serve_blog(struct http_request *req) {
	return (page_serve(req, static_mkd_blog, static_len_mkd_blog));
}

int serve_ip(struct http_request *req) {
	int ret;
	struct in_addr addr;
	size_t jsonsize;
	char *json, *ip;

	http_response_header_add(req, "content-type", "application/json");
	addr = req->owner->sin.sin_addr;
	ip = inet_ntoa(addr);
	if(!ip)
		return (http_response(req, 500, NULL, 0));

	jsonsize = strlen(ip) + strlen(req->agent) + strlen("{ip:,agent:}") + 1;
	json = (char *) malloc(sizeof(char) * jsonsize);
	if(!json)
		return (http_response(req, 500, NULL, 0));

	snprintf(json, jsonsize, "{ip:%s,agent:%s}", ip, req->agent);
	ret = http_response(req, 200, (u_int8_t *)json, jsonsize -1 );

	return (ret);
}

int serve_mkd(struct http_request *req) {
	int ret;
	http_response_header_add(req, "content-type", "text/plain");
	ret = http_response(req, 200, static_mkd_blog, static_len_mkd_blog);
	return (ret);
}

int serve_qr(struct http_request *req) {
	int ret;

	http_response_header_add(req, "content-type", "image/jpg");
	ret = http_response(req, 200, static_png_qr, static_len_png_qr);

	return (ret);
}

static int
page_serve(struct http_request *req, u_int8_t *c, u_int32_t len)
{
	u_int8_t		*p;
	int			ret;
	u_int32_t		plen;
	struct kore_buf		*page;

	page = build_common(req, c, len);
	p = kore_buf_release(page, &plen);

	http_response_header_add(req, "content-type", "text/html");
	ret = http_response(req, 200, p, plen);
	kore_mem_free(p);

	return (ret);
}

static void
page_replace(struct kore_buf *b, char *src, u_int8_t *dst, size_t len)
{
	u_int32_t	blen, off, off2;
	size_t		nlen, klen;
	char		*key, *end, *tmp, *p;

	off = 0;
	klen = strlen(src);
	for (;;) {
		blen = b->offset;
		nlen = blen + len;
		p = (char *)b->data;

		if ((key = strstr((p + off), src)) == NULL)
			break;

		end = key + klen;
		off = key - p;
		off2 = ((char *)(b->data + b->offset) - end);

		tmp = (char *)kore_malloc(nlen);
		memcpy(tmp, p, off);
		memcpy((tmp + off), dst, len);
		memcpy((tmp + off + len), end, off2);

		kore_mem_free(b->data);
		b->data = (u_int8_t *)tmp;
		b->offset = off + len + off2;
		b->length = nlen;

		off = off + len;
	}
}

static struct kore_buf *
build_common(struct http_request *req, u_int8_t *c, u_int32_t len)
{
	struct kore_buf	*page;

	char *html;
	int sizemkd;
	MMIOT *mkd;

	page = kore_buf_create(4096);
	kore_buf_append(page, static_html_default, static_len_html_default);

	// traitement mkd
	mkd = mkd_string((char *) c, len, 0);
	if(!mkd) {
		html = strdup("error: can't mkd_string");
		page_replace(page, "$mkd$", (u_int8_t *) html, strlen(html));
		page_replace(page, "$build_date$", (u_int8_t *)MODULE_BUILD_DATE, strlen(MODULE_BUILD_DATE));
		return (page);	
	}
			
		
	if(!mkd_compile(mkd, 0)) {
		html = strdup("error: can't mkd_compile");
		page_replace(page, "$mkd$", (u_int8_t *) html, strlen(html));
		page_replace(page, "$build_date$", (u_int8_t *)MODULE_BUILD_DATE, strlen(MODULE_BUILD_DATE));
		return (page);	
	}

	if(-1 == (sizemkd = mkd_document(mkd, &html))) {
		html = strdup("error: can't mkd_document");
		page_replace(page, "$mkd$", (u_int8_t *) html, strlen(html));
		page_replace(page, "$build_date$", (u_int8_t *)MODULE_BUILD_DATE, strlen(MODULE_BUILD_DATE));
		return (page);	
	}

	page_replace(page, "$mkd$", (u_int8_t *) html, sizemkd);
	page_replace(page, "$build_date$", (u_int8_t *)MODULE_BUILD_DATE, strlen(MODULE_BUILD_DATE));

	return (page);
}
