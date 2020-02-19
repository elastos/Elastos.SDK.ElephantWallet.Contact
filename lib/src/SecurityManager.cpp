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

bool SecurityManager::IsValidPublicKey(const std::string& code)
{
    bool valid = false;
    if(code.length() == 66 && code[0] == '0'
    && (code[1] == '2' || code[1] == '3')) {
        valid = true;
    }
    Log::V(Log::TAG, " %s %d", __PRETTY_FUNCTION__, valid);

    return valid;
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
SecurityManager::SecurityManager()
    : mSecurityListener()
{
}

SecurityManager::~SecurityManager()
{
}

void SecurityManager::setSecurityListener(std::shared_ptr<SecurityListener> listener)
{
    mSecurityListener = listener;
}

int SecurityManager::getPublicKey(std::string& value)
{
    if(mSecurityListener == nullptr) {
        return ErrCode::NoSecurityListener;
    }

    value = mSecurityListener->onAcquirePublicKey();
    if(value.empty() == true) {
        return ErrCode::BadSecurityValue;
    }

    return 0;
}

int SecurityManager::getElaAddress(std::string& value)
{
    std::string pubKey;
    int ret = getPublicKey(pubKey);
    CHECK_ERROR(ret)

    ret = GetElaAddress(pubKey, value);
    CHECK_ERROR(ret)

    return 0;
}

int SecurityManager::getDid(std::string& value)
{
    std::string pubKey;
    int ret = getPublicKey(pubKey);
    CHECK_ERROR(ret)

    ret = GetDid(pubKey, value);
    CHECK_ERROR(ret)

    return 0;
}


int SecurityManager::encryptData(const std::string& key, const std::vector<uint8_t>& src, std::vector<uint8_t>& dest)
{
    if(mSecurityListener == nullptr) {
        return ErrCode::NoSecurityListener;
    }

    dest = mSecurityListener->onEncryptData(key, src);
    if(dest.empty() == true) {
        return ErrCode::BadSecurityValue;
    }

    return 0;
}

int SecurityManager::decryptData(const std::vector<uint8_t>& src, std::vector<uint8_t>& dest)
{
    if(mSecurityListener == nullptr) {
        return ErrCode::NoSecurityListener;
    }

    if(src.empty()) {
        dest.clear();
        return 0;
    }

    dest = mSecurityListener->onDecryptData(src);
    if(dest.empty() == true) {
        return ErrCode::BadSecurityValue;
    }

    return 0;
}

int SecurityManager::saveCryptoFile(const std::string& filePath, const std::vector<uint8_t>& originData)
{
    std::string pubKey;
    int ret = getPublicKey(pubKey);
    CHECK_ERROR(ret)

    std::vector<uint8_t> encryptedData;
    ret = encryptData(pubKey, originData, encryptedData);
    CHECK_ERROR(ret)

    std::ofstream cryptoFile;

    cryptoFile.open(filePath, std::ios::out | std::ios::binary);

    cryptoFile.write(reinterpret_cast<char*>(encryptedData.data()), encryptedData.size ());

    cryptoFile.flush();
    cryptoFile.close();

    Log::I(Log::TAG, "Success to save crypto file to %s", filePath.c_str());

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

    int ret = decryptData(encryptedData, originData);
    CHECK_ERROR(ret)

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
