#ifndef _CROSSPL_PROXY_ContactDataListener_HPP_
#define _CROSSPL_PROXY_ContactDataListener_HPP_

#ifdef ENABLE_NATIVE_FUNCTION

#ifdef __cplusplus
extern "C" {
#endif



#ifdef __cplusplus
} // extern "C"
#endif

#endif // ENABLE_NATIVE_FUNCTION

#ifdef ENABLE_PLATFORM_FUNCTION

void crosspl_Proxy_ContactDataListener_onNotify(int64_t platformHandle, const char* var0, int var1, const char* var2, int var3);
std::shared_ptr<std::span<uint8_t>> crosspl_Proxy_ContactDataListener_onReadData(int64_t platformHandle, const char* var0, int var1, const char* var2, int64_t var3);
int crosspl_Proxy_ContactDataListener_onWriteData(int64_t platformHandle, const char* var0, int var1, const char* var2, int64_t var3, const std::span<uint8_t>* var4);


#endif // ENABLE_PLATFORM_FUNCTION

#endif /* _CROSSPL_PROXY_ContactDataListener_HPP_ */
