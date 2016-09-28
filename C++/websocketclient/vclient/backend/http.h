#ifndef HTTP_H
#define HTTP_H

#ifdef __cplusplus
extern "C" {
#endif

	 int HttpPost(const char *url, const char *input, char *output, int *outputLength);
	 int HttpPut(const char *url, const char *input, char *output, int *outputLength);
	 int HttpGet(const char *url, const char *input, char *output, int *outputLength);
	 int HttpDelete(const char *url, const char *input, char *output, int *outputLength);

#ifdef __cplusplus
}
#endif

#endif // HTTP_H
