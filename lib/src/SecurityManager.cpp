//
//  SecurityManager.cpp
//
//  Created by mengxk on 19/03/16.
//  Copyright © 2016 mengxk. All rights reserved.
//

#include <SecurityManager.hpp>

#include <Elastos.SDK.Keypair.C/Elastos.Wallet.Utility.h>
#include <fstream>
#include <Log.hpp>
#include <MD5.hpp>

namespace elastos {

/***********************************************/
/***** static variables initialize *************/
/***********************************************/


/***********************************************/
/***** static function implement ***************/
/***********************************************/
int SecurityManager::GetElaAddress(const std::string& pubKey, std::string& elaAddr)
{
    auto keypairAddr = ::getAddress(pubKey.c_str());
    if(keypairAddr == nullptr) {
        return ErrCode::KeypairError;
    }

    elaAddr = keypairAddr;
    ::freeBuf(keypairAddr);

    return 0;
}

int SecurityManager::GetDid(const std::string& pubKey, std::string& did)
{
    auto keypairDid = ::getDid(pubKey.c_str());
    if(keypairDid == nullptr) {
        return ErrCode::KeypairError;
    }

    did = keypairDid;
    ::freeBuf(keypairDid);

    return 0;
}

bool SecurityManager::IsValidElaAddress(const std::string& code)
{
    bool valid = ::isAddressValid(code.c_str());
    return valid;
}

bool SecurityManager::IsValidDid(const std::string& code)
{
    // TODO
    //bool valid = ::isAddressValid(code.c_str());
    //return valid;

    return (code.length() == 34 && code[0] == 'i');
}

/***********************************************/
/***** class public function implement  ********/
/***********************************************/
void SecurityManager::setSecurityListener(std::shared_ptr<SecurityListener> listener)
{
    mSecurityListener = listener;
}

int SecurityManager::getPublicKey(std::string& value)
{
    if(mPublicKey.empty() == false) {
        value = mPublicKey;
        return 0;
    }

    if(mSecurityListener == nullptr) {
        return ErrCode::NoSecurityListener;
    }

    mPublicKey = mSecurityListener->onAcquirePublicKey();
    if(mPublicKey.empty() == true) {
        return ErrCode::BadSecurityValue;
    }

    value = mPublicKey;
    return 0;
}

int SecurityManager::getElaAddress(std::string& value)
{
    std::string pubKey;
    int ret = getPublicKey(pubKey);
    CHECK_ERROR(ret);

    ret = GetElaAddress(pubKey, value);
    CHECK_ERROR(ret);

    return 0;
}

int SecurityManager::getDid(std::string& value)
{
    if(mDid.empty() == false) {
        value = mDid;
        return 0;
    }

    std::string pubKey;
    int ret = getPublicKey(pubKey);
    CHECK_ERROR(ret);

    ret = GetDid(pubKey, mDid);
    CHECK_ERROR(ret);

    value = mDid;
    return 0;
}


int SecurityManager::encryptData(const std::string& pubKey, const std::string& cryptoAlgorithm, const std::vector<uint8_t>& src, std::vector<uint8_t>& dest)
{
    if(mSecurityListener == nullptr) {
        return ErrCode::NoSecurityListener;
    }

    dest = mSecurityListener->onEncryptData(pubKey, cryptoAlgorithm, src);
    if(dest.empty() == true) {
        return ErrCode::BadSecurityValue;
    }

    return 0;
}

int SecurityManager::decryptData(const std::string& cryptoAlgorithm, const std::vector<uint8_t>& src, std::vector<uint8_t>& dest)
{
    if(mSecurityListener == nullptr) {
        return ErrCode::NoSecurityListener;
    }

    if(src.empty()) {
        dest.clear();
        return 0;
    }

    dest = mSecurityListener->onDecryptData(cryptoAlgorithm, src);
    if(dest.empty() == true) {
        return ErrCode::BadSecurityValue;
    }

    return 0;
}

int SecurityManager::encryptString(const std::string& pubKey, const std::string& cryptoAlgorithm,
                                   const std::string& src, std::string& dest)
{
    std::vector<uint8_t> originData {src.begin(), src.end()};
    std::vector<uint8_t> cryptedData;

    int ret = encryptData(pubKey, cryptoAlgorithm, originData, cryptedData);
    CHECK_ERROR(ret);

    dest = {cryptedData.begin(), cryptedData.end()};

    return 0;
}

int SecurityManager::decryptString(const std::string& cryptoAlgorithm,
                                   const std::string& src, std::string& dest)
{
    std::vector<uint8_t> cryptedData {src.begin(), src.end()};
    std::vector<uint8_t> originData;

    int ret = decryptData(cryptoAlgorithm, cryptedData, originData);
    CHECK_ERROR(ret);

    dest = {originData.begin(), originData.end()};

    return 0;
}

int SecurityManager::saveCryptoFile(const std::string& filePath, const std::vector<uint8_t>& originData)
{
    std::string pubKey;
    int ret = getPublicKey(pubKey);
    CHECK_ERROR(ret);

    std::vector<uint8_t> encryptedData;
    ret = encryptData(pubKey, DefaultCryptoAlgorithm, originData, encryptedData);
    CHECK_ERROR(ret);

    std::ofstream cryptoFile;

    cryptoFile.open(filePath, std::ios::out | std::ios::binary);

    cryptoFile.write(reinterpret_cast<char*>(encryptedData.data()), encryptedData.size ());

    cryptoFile.close();

    return 0;
}

int SecurityManager::loadCryptoFile(const std::string& filePath, std::vector<uint8_t>& originData)
{
    std::ifstream cryptoFile;

    cryptoFile.open(filePath, std::ios::in | std::ios::binary);

    cryptoFile.seekg (0, cryptoFile.end);
    auto dataLen = cryptoFile.tellg();
    cryptoFile.seekg (0, cryptoFile.beg);
    if(dataLen < 0) {
        cryptoFile.close();
        return ErrCode::FileNotExistsError;
    }

    std::vector<uint8_t> encryptedData;
    encryptedData.resize(dataLen);

    cryptoFile.read(reinterpret_cast<char*>(encryptedData.data()), encryptedData.size ());

    cryptoFile.close();

    int ret = decryptData(DefaultCryptoAlgorithm, encryptedData, originData);
    CHECK_ERROR(ret);

    return 0;
}

int SecurityManager::signData(const std::vector<uint8_t>& src, std::vector<uint8_t>& dest)
{
    if(mSecurityListener == nullptr) {
        return ErrCode::NoSecurityListener;
    }

    dest = mSecurityListener->onSignData(src);
    if(dest.empty() == true) {
        return ErrCode::BadSecurityValue;
    }

    return 0;
}

int SecurityManager::signDataSelfVerifiable(const std::vector<uint8_t>& src, std::string& dest)
{
    std::vector<uint8_t> signedBytes;
    int ret = signData(src, signedBytes);
    CHECK_ERROR(ret);

    std::string pubKey;
    ret = getPublicKey(pubKey);
    CHECK_ERROR(ret);

    std::string msgStr = MD5::MakeHexString(src);
    std::string sigStr = MD5::MakeHexString(signedBytes);

    // did prop key, sign, make {msg, sig, pub}
    dest = std::string("{")
         + "\"pub\":\"" + pubKey + "\","
         + "\"msg\":\"" + msgStr + "\","
         + "\"sig\":\"" + sigStr + "\""
         + "}";

    return 0;
}

    int SecurityManager::getDidPropAppId(std::string& appId)
{
    if(mSecurityListener == nullptr) {
        return ErrCode::NoSecurityListener;
    }

    appId = mSecurityListener->onAcquireDidPropAppId();
    if(appId.empty() == true) {
        return ErrCode::EmptyInfoError;
    }

    return 0;
}

int SecurityManager::getDidAgentAuthHeader(std::string& authHeader)
{
    if(mSecurityListener == nullptr) {
        return ErrCode::NoSecurityListener;
    }

    authHeader = mSecurityListener->onAcquireDidAgentAuthHeader();
    if(authHeader.empty() == true) {
        return ErrCode::BadSecurityValue;
    }

    return 0;
}

/***********************************************/
/***** class protected function implement  *****/
/***********************************************/


/***********************************************/
/***** class private function implement  *******/
/***********************************************/

} // namespace elastos
