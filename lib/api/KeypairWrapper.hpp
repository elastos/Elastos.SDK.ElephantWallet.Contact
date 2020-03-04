/**
 * @file	KeypairWrapper.hpp
 * @brief	Contact
 * @details
 *
 * @author	xxx
 * @author	<xxx@xxx.com>
 * @copyright	(c) 2012 xxx All rights reserved.
 **/

#ifndef _ELASTOS_SDK_JNI_KEYPAIR_WRAPPER_HPP_
#define _ELASTOS_SDK_JNI_KEYPAIR_WRAPPER_HPP_

#include "experimental-span.hpp"
#include <sstream>
#include <vector>

namespace crosspl {
namespace native {
    
class KeypairWrapper {
public:
    /*** type define ***/

    /*** static function and variable ***/
    static int GetSinglePublicKey(const std::span<uint8_t>* seed, std::stringstream* pubKey);
    static int GetSinglePrivateKey(const std::span<uint8_t>* seed, std::stringstream* priviey);
    static int GenerateMnemonic(const char* language, const char* words, std::stringstream* mnem);
    static int GetSeedFromMnemonic(const char* mnemonic, const char* mnemonicPassword, std::vector<uint8_t>* seed);
    static int Sign(const char* privateKey, const std::span<uint8_t>* data, std::vector<uint8_t>* signedData);
    static int EciesEncrypt(const char* publicKey, const std::span<uint8_t>* plainData,
                            std::vector<uint8_t>* cipherData);
    static int EciesDecrypt(const char* privateKey, const std::span<uint8_t>* cipherData,
                            std::vector<uint8_t>* plainData);

    /*** class function and variable ***/
    explicit KeypairWrapper() = default;
    virtual ~KeypairWrapper() = default;

private:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/
}; // class Contact

} //namespace native
} //namespace crosspl

#endif /* _ELASTOS_SDK_JNI_KEYPAIR_WRAPPER_HPP_ */

