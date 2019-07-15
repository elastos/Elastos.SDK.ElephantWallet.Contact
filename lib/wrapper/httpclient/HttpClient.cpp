#include <HttpClient.hpp>

#include <iostream>
#include <cstring>
#include <algorithm>

#include <curl/curl.h>
#include <ErrCode.hpp>
#include <Log.hpp>

#define CHECK_CURL(curl_code) \
	if(curl_code != CURLE_OK) { \
		int errcode = (ErrCode::CurlBaseCode + (-curl_code)); \
		Log::E(Log::TAG, "Failed to call %s, return %d.", __PRETTY_FUNCTION__, errcode); \
		return errcode; \
	}

namespace elastos {

/* =========================================== */
/* === static variables initialize =========== */
/* =========================================== */
std::mutex HttpClient::gMutex {};
bool HttpClient::gIsGlobalInitialized = false;

/* =========================================== */
/* === static function implement ============= */
/* =========================================== */
int HttpClient::InitGlobal()
{
	std::unique_lock<std::mutex> lock(gMutex);
    if(gIsGlobalInitialized == true) {
        return 0;
    }

	CURLcode curle;
	curle = curl_global_init(CURL_GLOBAL_ALL);
	CHECK_CURL(curle);

	//curl_global_cleanup(); // never called
    gIsGlobalInitialized = true;
	return 0;
}

/* =========================================== */
/* === class public function implement  ====== */
/* =========================================== */
HttpClient::HttpClient()
	: mUrl()
	, mConnectTimeoutMS(10000)
	, mReqHeaders()
	, mRespStatus(-1)
	, mRespReason()
	, mRespHeaders()
	, mRespBody()
{
}

HttpClient::~HttpClient()
{
}

int HttpClient::url(const std::string& url)
{
	if(url.empty() == true) {
		return ErrCode::NullArgument;
	}

	if(url.compare(0, std::strlen(SCHEME_HTTP), SCHEME_HTTP) != 0
	&& url.compare(0, std::strlen(SCHEME_HTTPS), SCHEME_HTTPS) != 0) {
		return ErrCode::BadArgument;
	}

	mUrl = url;

	return 0;
}

int HttpClient::addHeader(const std::string& name, const std::string& value)
{
	int ret = 0;

	ret = addHeader(mReqHeaders, name, value);

	return ret;
}

int HttpClient::setHeader(const std::string& name, const std::string& value)
{
	int ret = 0;
	if(name.empty() == true) {
		return ErrCode::NullArgument;
	}

	auto found = mReqHeaders.find(name);
	if(found != mReqHeaders.end()) {
		found->second.clear();
	}

	ret = addHeader(name, value);

	return ret;
}

int HttpClient::setConnectTimeout(unsigned long milliSecond)
{
	mConnectTimeoutMS = milliSecond;

	return 0;
}

int HttpClient::syncGet()
{
	int ret;
	CURLcode curle;
	std::shared_ptr<CURL> curlHandlePtr;
	std::shared_ptr<struct curl_slist> curlHeadersPtr;

	ret = makeCurl(curlHandlePtr, curlHeadersPtr);
	CHECK_ERROR(ret);

	mRespStatus = -1;
	mRespReason.clear();
	mRespHeaders.clear();
	mRespBody = std::stringstream();

	curle = curl_easy_perform(curlHandlePtr.get());
	CHECK_CURL(curle);

	curle = curl_easy_getinfo(curlHandlePtr.get(), CURLINFO_RESPONSE_CODE, &mRespStatus);
	CHECK_CURL(curle);

	return 0;
}

int HttpClient::syncPost(const int8_t* body, int size)
{
	int ret;
	CURLcode curle;
	std::shared_ptr<CURL> curlHandlePtr;
	std::shared_ptr<struct curl_slist> curlHeadersPtr;

	ret = makeCurl(curlHandlePtr, curlHeadersPtr);
	CHECK_ERROR(ret);

	mRespStatus = -1;
	mRespReason.clear();
	mRespHeaders.clear();
	mRespBody = std::stringstream();

	curle = curl_easy_setopt(curlHandlePtr.get(), CURLOPT_POSTFIELDS, body);
	CHECK_CURL(curle);

	curle = curl_easy_setopt(curlHandlePtr.get(), CURLOPT_POSTFIELDSIZE, size);
	CHECK_CURL(curle);

	curle = curl_easy_perform(curlHandlePtr.get());
	CHECK_CURL(curle);

	curle = curl_easy_getinfo(curlHandlePtr.get(), CURLINFO_RESPONSE_CODE, &mRespStatus);
	CHECK_CURL(curle);

	return 0;
}

int HttpClient::syncPost(const std::string& body)
{
	int ret = 0;

	ret = syncPost(reinterpret_cast<const int8_t*>(body.data()), body.size());

	return ret;
}

int HttpClient::getResponseStatus() const
{
	return mRespStatus;
}

int HttpClient::getResponseHeaders(HeaderMap& headers) const
{
	headers.clear();
	if((200 <= mRespStatus && mRespStatus < 300) == false) {
		return ErrCode::NetFailed;
	}

	headers = mRespHeaders;

	return mRespHeaders.size();
}

int HttpClient::getResponseHeader(const std::string& name, HeaderValue& value) const
{
	value.clear();
	if((200 <= mRespStatus && mRespStatus < 300) == false) {
		return ErrCode::NetFailed;
	}
	auto found = mRespHeaders.find(name);
	if(found == mRespHeaders.end()) {
		return ErrCode::NotFound;
	}

	value = found->second;
	int size = found->second.size();

	return size;
}

int HttpClient::getResponseBody(std::shared_ptr<int8_t>& body)
{
	body.reset();
	if((200 <= mRespStatus && mRespStatus < 300) == false) {
		return ErrCode::NetFailed;
	}

	mRespBody.seekg(0, mRespBody.end);
	int size = mRespBody.tellg();
	mRespBody.seekg(0, mRespBody.beg);

	body = std::shared_ptr<int8_t>(new int8_t[size], std::default_delete<int8_t[]>());
	mRespBody.read(reinterpret_cast<char*>(body.get()), size);

	return size;
}

int HttpClient::getResponseBody(std::string& body)
{
	body.clear();
	if((200 <= mRespStatus && mRespStatus < 300) == false) {
		return ErrCode::NetFailed;
	}

	body = mRespBody.str();

	mRespBody.seekg(0, mRespBody.end);
	int size = mRespBody.tellg();
	mRespBody.seekg(0, mRespBody.beg);

	return size;
}

/* =========================================== */
/* === class protected function implement  === */
/* =========================================== */


/* =========================================== */
/* === class private function implement  ===== */
/* =========================================== */
std::string trim(const std::string &s)
{
   auto wsfront=std::find_if_not(s.begin(),s.end(),[](int c){return std::isspace(c);});
   auto wsback=std::find_if_not(s.rbegin(),s.rend(),[](int c){return std::isspace(c);}).base();
   return (wsback<=wsfront ? std::string() : std::string(wsfront,wsback));
}

size_t HttpClient::CurlHeaderCallback(char* buffer, size_t size, size_t nitems, void* userdata)
{
	HttpClient* httpClient = static_cast<HttpClient*>(userdata);
	std::string header(buffer);
	size_t length = size * nitems;

	auto pos = header.find(':');
	if(pos != std::string::npos) {
		std::string name = header.substr(0, pos);
		std::string value = header.substr(pos + 1);
		name = trim(name);
		value = trim(value);

		httpClient->addHeader(httpClient->mRespHeaders, name, value);
	}

	return length;
}

size_t HttpClient::CurlWriteCallback(char* buffer, size_t size, size_t nitems, void* userdata)
{
	HttpClient* httpClient = static_cast<HttpClient*>(userdata);
	size_t length = size * nitems;

	httpClient->mRespBody.write(buffer, length);

	return length;
}

size_t HttpClient::CurlReadCallback(char* buffer, size_t size, size_t nitems, void* userdata)
{

	return -1;
}

int HttpClient::makeCurl(std::shared_ptr<void>& curlHandlePtr, std::shared_ptr<struct curl_slist>& curlHeadersPtr) const
{
	CURLcode curle;

	curlHandlePtr.reset();
	curlHeadersPtr.reset();

	CURL*curlHandle = curl_easy_init();
	if(curlHandle == nullptr) {
		return (ErrCode::CurlBaseCode);
	}
	auto curlHandleDeleter = [](CURL* ptr) -> void {
		curl_easy_cleanup(ptr);
	};
	curlHandlePtr = std::shared_ptr<CURL>(curlHandle, curlHandleDeleter);

	curle = curl_easy_setopt(curlHandlePtr.get(), CURLOPT_NOSIGNAL, true);
	CHECK_CURL(curle);

	if(mUrl.empty() == true) {
		return ErrCode::UrlNotExists;
	}
    Log::I(Log::TAG, "HttpClient::makeCurl() url=%s", mUrl.c_str());
	curle = curl_easy_setopt(curlHandlePtr.get(), CURLOPT_URL, mUrl.c_str());
	CHECK_CURL(curle);

	struct curl_slist* curlHeaders = nullptr;
	bool hasAcceptEncoding = false;
	for (auto& [key, val]: mReqHeaders) {
		auto& name = key;
		for(auto& value: val) {
			std::string header = (name + ": " + value);
            Log::I(Log::TAG, "HttpClient::makeCurl() header=%s", header.c_str());
			curlHeaders = curl_slist_append(curlHeaders, header.c_str());
		}

		if(key == "Accept-Encoding") {
			hasAcceptEncoding = true;
		}
	}
	if(hasAcceptEncoding == false) {
		curlHeaders = curl_slist_append(curlHeaders, "Accept-Encoding: identity");
	}
	auto curlHeadersDeleter = [](struct curl_slist* ptr) -> void {
		curl_slist_free_all(ptr);
	};
	curlHeadersPtr = std::shared_ptr<struct curl_slist>(curlHeaders, curlHeadersDeleter);

	curle = curl_easy_setopt(curlHandlePtr.get(), CURLOPT_SSL_VERIFYHOST, 0L);
	CHECK_CURL(curle);

    curle = curl_easy_setopt(curlHandlePtr.get(), CURLOPT_SSL_VERIFYPEER, 0L);
	CHECK_CURL(curle);

	curle = curl_easy_setopt(curlHandlePtr.get(), CURLOPT_HTTPHEADER, curlHeadersPtr.get());
	CHECK_CURL(curle);

	curle = curl_easy_setopt(curlHandlePtr.get(), CURLOPT_CONNECTTIMEOUT_MS, mConnectTimeoutMS);
	CHECK_CURL(curle);

	curle = curl_easy_setopt(curlHandlePtr.get(), CURLOPT_HEADERFUNCTION, CurlHeaderCallback);
	CHECK_CURL(curle);

	curle = curl_easy_setopt(curlHandlePtr.get(), CURLOPT_HEADERDATA, this);
	CHECK_CURL(curle);

	curle = curl_easy_setopt(curlHandlePtr.get(), CURLOPT_WRITEFUNCTION, CurlWriteCallback);
	CHECK_CURL(curle);

	curle = curl_easy_setopt(curlHandlePtr.get(), CURLOPT_WRITEDATA, this);
	CHECK_CURL(curle);

	curle = curl_easy_setopt(curlHandlePtr.get(), CURLOPT_READFUNCTION, CurlReadCallback);
	CHECK_CURL(curle);

	curle = curl_easy_setopt(curlHandlePtr.get(), CURLOPT_READDATA, this);
	CHECK_CURL(curle);

	return 0;
}

int HttpClient::addHeader(HeaderMap& headers, const std::string& name, const std::string& value) const
{
	if(name.empty() == true) {
		return ErrCode::NullArgument;
	}

	auto found = headers.find(name);
	if(found == headers.end()) {
		headers[name] = HeaderValue();
		found = headers.find(name);
	}

	found->second.push_back(value);

	return 0;
}

} // namespace elastos
