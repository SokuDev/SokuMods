#pragma once
#include <curl/curl.h>
#include <iostream>
#include <stdio.h>
using namespace std;

// Most of this was shamelessly yoinked from various tutorials/StackOverflow pages.
namespace WebHandler {
namespace {
struct MemoryStruct {
	char *memory;
	size_t size;
};

CURL *request_handle;
CURL *put_handle;
struct MemoryStruct chunk;
curl_slist *headers;
} // namespace

size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)userp;

	char *ptr = (char *)realloc(mem->memory, mem->size + realsize + 1);
	if (ptr == NULL) {
		throw "not enough memory (realloc returned NULL)\n";
	}

	mem->memory = ptr;
	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}

void Init() {
	chunk.memory = 0;
	chunk.size = 0;

	curl_global_init(CURL_GLOBAL_ALL);

	request_handle = curl_easy_init();
	curl_easy_setopt(request_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(request_handle, CURLOPT_WRITEDATA, (void *)&chunk);
	curl_easy_setopt(request_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

	put_handle = curl_easy_init();
	curl_easy_setopt(put_handle, CURLOPT_CUSTOMREQUEST, "PUT");
	headers = curl_slist_append(headers, "Content-Type: application/json");
	curl_easy_setopt(put_handle, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(request_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
}

string Request(const string &url) {
	string response;
	int response_code;
	chunk.memory = (char *)malloc(1);
	chunk.size = 0;

	curl_easy_setopt(request_handle, CURLOPT_URL, &url[0]);
	CURLcode res = curl_easy_perform(request_handle);

	if (res != CURLE_OK) {
		free(chunk.memory);
		throw curl_easy_strerror(res);
	}

	curl_easy_getinfo(request_handle, CURLINFO_RESPONSE_CODE, &response_code);

	if (response_code != 200) {
		free(chunk.memory);
		throw response_code;
	}

	response = chunk.memory;

	free(chunk.memory);
	return response;
}

CURLcode Put(const string &url, const string &data) {
	curl_easy_setopt(put_handle, CURLOPT_URL, &url[0]);
	curl_easy_setopt(put_handle, CURLOPT_POSTFIELDS, &data[0]);

	return curl_easy_perform(put_handle);
}

void Cleanup() {
	curl_slist_free_all(headers);
	curl_easy_cleanup(put_handle);
	curl_easy_cleanup(request_handle);
	curl_global_cleanup();
}
}; // namespace WebHandler
