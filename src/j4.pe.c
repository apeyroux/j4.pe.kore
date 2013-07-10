#include "kore.h"
#include "http.h"

#include "static.h"

int		serve_index(struct http_request *);
int		serve_qr(struct http_request *);

int
serve_index(struct http_request *req)
{
	int ret;

	http_response_header_add(req, "content-type", "text/html");
	ret = http_response(req, 200, static_html_index, static_len_html_index);

	return (ret);
}

int
serve_qr(struct http_request *req)
{
	int		ret;

	http_response_header_add(req, "content-type", "image/jpg");
	ret = http_response(req, 200, static_png_qr, static_len_png_qr);

	return (ret);
}
