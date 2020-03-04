#ifndef _CROSSPL_PROXY_KeypairWrapper_HPP_
#define _CROSSPL_PROXY_KeypairWrapper_HPP_

#ifdef ENABLE_NATIVE_FUNCTION

#ifdef __cplusplus
extern "C" {
#endif

int crosspl_Proxy_KeypairWrapper_GetSinglePublicKey(NSData* ocvar0, NSString** ocvar1);
int crosspl_Proxy_KeypairWrapper_GetSinglePrivateKey(NSData* ocvar0, NSString** ocvar1);
int crosspl_Proxy_KeypairWrapper_GenerateMnemonic(NSString* ocvar0, NSString* ocvar1, NSString** ocvar2);
int crosspl_Proxy_KeypairWrapper_GetSeedFromMnemonic(NSString* ocvar0, NSString* ocvar1, NSData** ocvar2);
int crosspl_Proxy_KeypairWrapper_Sign(NSString* ocvar0, NSData* ocvar1, NSData** ocvar2);
int crosspl_Proxy_KeypairWrapper_EciesEncrypt(NSString* ocvar0, NSData* ocvar1, NSData** ocvar2);
int crosspl_Proxy_KeypairWrapper_EciesDecrypt(NSString* ocvar0, NSData* ocvar1, NSData** ocvar2);


#ifdef __cplusplus
} // extern "C"
#endif

#endif // ENABLE_NATIVE_FUNCTION

#ifdef ENABLE_PLATFORM_FUNCTION



#endif // ENABLE_PLATFORM_FUNCTION

#endif /* _CROSSPL_PROXY_KeypairWrapper_HPP_ */
