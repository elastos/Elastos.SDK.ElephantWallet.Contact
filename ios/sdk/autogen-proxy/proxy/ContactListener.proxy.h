#ifndef _CROSSPL_PROXY_ContactListener_HPP_
#define _CROSSPL_PROXY_ContactListener_HPP_

#ifdef ENABLE_NATIVE_FUNCTION

#ifdef __cplusplus
extern "C" {
#endif



#ifdef __cplusplus
} // extern "C"
#endif

#endif // ENABLE_NATIVE_FUNCTION

#ifdef ENABLE_PLATFORM_FUNCTION

std::shared_ptr<std::span<uint8_t>> crosspl_Proxy_ContactListener_onAcquire(int64_t platformHandle, int var0, const char* var1, const std::span<uint8_t>* var2);
void crosspl_Proxy_ContactListener_onEvent(int64_t platformHandle, int var0, const char* var1, int var2, const std::span<uint8_t>* var3);
void crosspl_Proxy_ContactListener_onReceivedMessage(int64_t platformHandle, const char* var0, int var1, int var2, const std::span<uint8_t>* var3, const char* var4, int64_t var5, int64_t var6);
void crosspl_Proxy_ContactListener_onError(int64_t platformHandle, int32_t var0, const char* var1, const char* var2);


#endif // ENABLE_PLATFORM_FUNCTION

#endif /* _CROSSPL_PROXY_ContactListener_HPP_ */
