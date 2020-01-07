#ifndef _CROSSPL_PROXY_CrossBase_HPP_
#define _CROSSPL_PROXY_CrossBase_HPP_

#ifdef __cplusplus
extern "C" {
#endif

int64_t crosspl_Proxy_CrossBase_CreateNativeObject(NSString* ocvar0);
void crosspl_Proxy_CrossBase_DestroyNativeObject(NSString* ocvar0, int64_t ocvar1);
void crosspl_Proxy_CrossBase_bindPlatformHandle(int64_t nativeHandle, NSObject* ocvar0);
void crosspl_Proxy_CrossBase_unbindPlatformHandle(int64_t nativeHandle, NSObject* ocvar0);


#ifdef __cplusplus
} // extern "C"
#endif

#ifdef ENABLE_PLATFORM_FUNCTION



#endif // ENABLE_PLATFORM_FUNCTION

#endif /* _CROSSPL_PROXY_CrossBase_HPP_ */
