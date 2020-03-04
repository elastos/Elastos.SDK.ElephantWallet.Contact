//
//  Elastos.SDK.Contact.cpp
//
//  Created by mengxk on 19/03/16.
//  Copyright © 2016 mengxk. All rights reserved.
//

#include <KeypairWrapper.hpp>

#include "Log.hpp"
#include <ErrCode.hpp>
#include <Elastos.SDK.Keypair.C/Elastos.Wallet.Utility.h>

namespace crosspl {
namespace native {
    
/***********************************************/
/***** static variables initialize *************/
/***********************************************/

/***********************************************/
/***** static function implement ***************/
/***********************************************/

int KeypairWrapper::GetSinglePublicKey(const std::span<uint8_t>* seed, std::stringstream* pubKey)
{
  pubKey->str("");

  auto ret = ::getSinglePublicKey(seed->data(), (int)seed->size());
  if(ret == nullptr) {
    return -1;
  }
  
  (*pubKey) << ret;
  freeBuf(ret);

  return 0;
}

int KeypairWrapper::GetSinglePrivateKey(const std::span<uint8_t>* seed, std::stringstream* privKey)
{
  privKey->str("");

  auto ret = ::getSinglePrivateKey(seed->data(), (int)seed->size());
  if(ret == nullptr) {
    return -1;
  }
  
  (*privKey) << ret;
  freeBuf(ret);

  return 0;
}

int KeypairWrapper::GenerateMnemonic(const char* language, const char* words, std::stringstream* mnem)
{
  if(language == nullptr || mnem == nullptr) {
    return elastos::ErrCode::InvalidArgument;
  }
  if(words == nullptr) {
    words = "";
  }

  mnem->str("");
  auto ret = ::generateMnemonic(language, words);
  if(ret == nullptr) {
    return -1;
  }
  
  (*mnem) << ret;
  freeBuf(ret);

  return 0;
}

int KeypairWrapper::GetSeedFromMnemonic(const char* mnemonic, const char* mnemonicPassword, std::vector<uint8_t>* seed)
{
  if(mnemonic == nullptr || seed == nullptr) {
    return elastos::ErrCode::InvalidArgument;
  }

  if(mnemonicPassword == nullptr) {
    mnemonicPassword = "";
  }

  seed->clear();
  void* seedData = nullptr;
  int seedSize = ::getSeedFromMnemonic(&seedData, mnemonic, mnemonicPassword);
  if(seedData == nullptr || seedSize <= 0) {
    return -1;
  }

  *seed = std::vector<uint8_t>((uint8_t*)seedData, (uint8_t*)seedData + seedSize);

  freeBuf(seedData);

  return 0;
}

int KeypairWrapper::Sign(const char* privateKey, const std::span<uint8_t>* data, std::vector<uint8_t>* signedData)
{
  if(privateKey == nullptr || data == nullptr || signedData == nullptr) {
    return elastos::ErrCode::InvalidArgument;
  }

  signedData->clear();
  void* keypairSignedData = nullptr;
  int keypairSignedSize = ::sign(privateKey, data->data(), (int)data->size(), &keypairSignedData);
  if(keypairSignedData == nullptr || keypairSignedSize <= 0) {
    return -1;
  }

  *signedData = std::vector<uint8_t>((uint8_t*)keypairSignedData, (uint8_t*)keypairSignedData + keypairSignedSize);

  freeBuf(keypairSignedData);

  return keypairSignedSize;
}

int KeypairWrapper::EciesEncrypt(const char* publicKey, const std::span<uint8_t>* plainData,
                                 std::vector<uint8_t>* cipherData)
{
  cipherData->clear();

  char* keypairCipherData = ::eciesEncrypt(publicKey, plainData->data(), (int)plainData->size());
  if(keypairCipherData == nullptr) {
    return -1;
  }

  auto keypairCipherLen = strlen(keypairCipherData) + 1;
  *cipherData = std::vector<uint8_t>((uint8_t*)keypairCipherData, (uint8_t*)keypairCipherData + keypairCipherLen);

  freeBuf(keypairCipherData);

  return (int)keypairCipherLen;
}

int KeypairWrapper::EciesDecrypt(const char* privateKey, const std::span<uint8_t>* cipherData,
                                 std::vector<uint8_t>* plainData)
{
  plainData->clear();

  int keypairPlainLen = -1;
  unsigned char* keypairPlainData = ::eciesDecrypt(privateKey, (char*)cipherData->data(), &keypairPlainLen);
  if(keypairPlainData == nullptr) {
    return -1;
  }

  *plainData = std::vector<uint8_t>((uint8_t*)keypairPlainData, (uint8_t*)keypairPlainData + keypairPlainLen);
  
  freeBuf(keypairPlainData);

  return keypairPlainLen;
}

/***********************************************/
/***** class public function implement  ********/
/***********************************************/

/***********************************************/
/***** class protected function implement  *****/
/***********************************************/


/***********************************************/
/***** class private function implement  *******/
/***********************************************/


} //namespace native
} //namespace crosspl
