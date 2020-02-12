#ifndef _CROSSPL_PROXY_ContactChannelStrategy_HPP_
#define _CROSSPL_PROXY_ContactChannelStrategy_HPP_

#ifdef ENABLE_NATIVE_FUNCTION

#ifdef __cplusplus
extern "C" {
#endif

int crosspl_Proxy_ContactChannelStrategy_receivedMessage(int64_t nativeHandle, NSString* ocvar0, int ocvar1, NSData* ocvar2);
int crosspl_Proxy_ContactChannelStrategy_syncChannelToNative(int64_t nativeHandle, int ocvar0, NSString* ocvar1);


#ifdef __cplusplus
} // extern "C"
#endif

#endif // ENABLE_NATIVE_FUNCTION

#ifdef ENABLE_PLATFORM_FUNCTION

int crosspl_Proxy_ContactChannelStrategy_onOpen(int64_t platformHandle);
int crosspl_Proxy_ContactChannelStrategy_onClose(int64_t platformHandle);
int crosspl_Proxy_ContactChannelStrategy_onSendMessage(int64_t platformHandle, const char* var0, int var1, const std::span<uint8_t>* var2);


#endif // ENABLE_PLATFORM_FUNCTION

#endif /* _CROSSPL_PROXY_ContactChannelStrategy_HPP_ */
