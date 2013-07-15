#include "kore.h"
#include "http.h"

#include "static.h"

int		serve_index(struct http_request *);
int 	serve_ip(struct http_request *req);
int		serve_qr(struct http_request *);

int serve_index(struct http_request *req) {
	int ret;

	http_response_header_add(req, "content-type", "text/html");
	ret = http_response(req, 200, static_html_index, static_len_html_index);

	return (ret);
}

int serve_ip(struct http_request *req) {
	int ret;
	char *json = NULL;

	http_response_header_add(req, "content-type", "application/json");
	struct in_addr addr = req->owner->sin.sin_addr;
	char *ip = inet_ntoa(addr);
	size_t jsonsize = strlen(ip) + strlen(req->agent) + strlen("{ip:,agent:}") + 1;
	json = (char *) malloc(sizeof(char) * jsonsize);
	snprintf(json, jsonsize, "{ip:%s,agent:%s}", ip, req->agent);
	ret = http_response(req, 200, (u_int8_t *)json, jsonsize -1 );

	return (ret);
}

int serve_qr(struct http_request *req) {
	int		ret;

	http_response_header_add(req, "content-type", "image/jpg");
	ret = http_response(req, 200, static_png_qr, static_len_png_qr);

	return (ret);
}
