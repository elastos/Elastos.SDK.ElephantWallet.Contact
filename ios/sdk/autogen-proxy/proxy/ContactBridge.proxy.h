#ifndef _CROSSPL_PROXY_ContactBridge_HPP_
#define _CROSSPL_PROXY_ContactBridge_HPP_

#ifdef ENABLE_NATIVE_FUNCTION

#ifdef __cplusplus
extern "C" {
#endif

bool crosspl_Proxy_ContactBridge_IsDidFriend(NSString* ocvar0);
int crosspl_Proxy_ContactBridge_start(int64_t nativeHandle);
int crosspl_Proxy_ContactBridge_stop(int64_t nativeHandle);
int crosspl_Proxy_ContactBridge_addFriend(int64_t nativeHandle, NSString* ocvar0, NSString* ocvar1);
int crosspl_Proxy_ContactBridge_removeFriend(int64_t nativeHandle, NSString* ocvar0);
int crosspl_Proxy_ContactBridge_acceptFriend(int64_t nativeHandle, NSString* ocvar0);
int crosspl_Proxy_ContactBridge_syncInfoDownloadFromDidChain(int64_t nativeHandle);
int crosspl_Proxy_ContactBridge_syncInfoUploadToDidChain(int64_t nativeHandle);
int crosspl_Proxy_ContactBridge_setWalletAddress(int64_t nativeHandle, NSString* ocvar0, NSString* ocvar1);
int crosspl_Proxy_ContactBridge_appendChannelStrategy(int64_t nativeHandle, int ocvar0, NSObject* ocvar1);
void crosspl_Proxy_ContactBridge_setListener(int64_t nativeHandle, NSObject* ocvar0);
void crosspl_Proxy_ContactBridge_setDataListener(int64_t nativeHandle, NSObject* ocvar0);
int crosspl_Proxy_ContactBridge_setHumanInfo(int64_t nativeHandle, NSString* ocvar0, int ocvar1, NSString* ocvar2);
int crosspl_Proxy_ContactBridge_setIdentifyCode(int64_t nativeHandle, int ocvar0, NSString* ocvar1);
int crosspl_Proxy_ContactBridge_getHumanInfo(int64_t nativeHandle, NSString* ocvar0, NSString** ocvar1);
int crosspl_Proxy_ContactBridge_getFriendList(int64_t nativeHandle, NSString** ocvar0);
int crosspl_Proxy_ContactBridge_getHumanStatus(int64_t nativeHandle, NSString* ocvar0);
int crosspl_Proxy_ContactBridge_sendMessage(int64_t nativeHandle, NSString* ocvar0, int ocvar1, NSObject* ocvar2);
int crosspl_Proxy_ContactBridge_pullData(int64_t nativeHandle, NSString* ocvar0, int ocvar1, NSString* ocvar2, NSString* ocvar3);
int crosspl_Proxy_ContactBridge_cancelPullData(int64_t nativeHandle, NSString* ocvar0, int ocvar1, NSString* ocvar2, NSString* ocvar3);


#ifdef __cplusplus
} // extern "C"
#endif

#endif // ENABLE_NATIVE_FUNCTION

#ifdef ENABLE_PLATFORM_FUNCTION



#endif // ENABLE_PLATFORM_FUNCTION

#endif /* _CROSSPL_PROXY_ContactBridge_HPP_ */
