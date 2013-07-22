#include <mkdio.h>

#include "kore.h"
#include "http.h"
#include "static.h"

#define BLOG_MKD "/tmp/blog.mkd"

int	serve_index(struct http_request *);
int 	serve_ip(struct http_request *req);
int	serve_qr(struct http_request *);

int serve_index(struct http_request *req) {
	int ret;

	http_response_header_add(req, "content-type", "text/html");
	ret = http_response(req, 200, static_html_index, static_len_html_index);

	return (ret);
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
		return http_response(req, 500, NULL, 0);

	jsonsize = strlen(ip) + strlen(req->agent) + strlen("{ip:,agent:}") + 1;
	json = (char *) malloc(sizeof(char) * jsonsize);
	if(!json)
		return http_response(req, 500, NULL, 0);

	snprintf(json, jsonsize, "{ip:%s,agent:%s}", ip, req->agent);
	ret = http_response(req, 200, (u_int8_t *)json, jsonsize -1 );

	return (ret);
}

int serve_qr(struct http_request *req) {
	int ret;

	http_response_header_add(req, "content-type", "image/jpg");
	ret = http_response(req, 200, static_png_qr, static_len_png_qr);

	return (ret);
}
