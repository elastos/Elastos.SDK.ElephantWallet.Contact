//
//  Config.cpp
//
//  Created by mengxk on 19/03/16.
//  Copyright © 2016 mengxk. All rights reserved.
//

#include <Config.hpp>

#include <CompatibleFileSystem.hpp>

namespace elastos {

/***********************************************/
/***** static variables initialize *************/
/***********************************************/


/***********************************************/
/***** static function implement ***************/
/***********************************************/

/***********************************************/
/***** class public function implement  ********/
/***********************************************/
Config::Config(const std::string& userDataDir)
    : mUserDataDir(userDataDir)
    , mCarrierConfig()
    , mElaChainConfig()
    , mDidChainConfig()
{
}

Config::~Config()
{
}

int Config::load()
{
    auto cfgFilePath = elastos::filesystem::path(mUserDataDir) / ConfigFileName;

    int ret = loadDefaultValues();
    return ret;
}

int Config::save()
{
    throw std::runtime_error(std::string(__PRETTY_FUNCTION__) + " Unimplemented!!!");
}

int Config::loadDefaultValues()
{
    mCarrierConfig = std::make_unique<CarrierConfig>();
    mCarrierConfig->mLogLevel = 4;
    mCarrierConfig->mEnableUdp = true;
    mCarrierConfig->mBootstrapNodes.push_back({ "13.58.208.50",  "33445", "89vny8MrKdDKs7Uta9RdVmspPjnRMdwMmaiEW27pZ7gh" });
    mCarrierConfig->mBootstrapNodes.push_back({ "18.216.6.197",  "33445", "H8sqhRrQuJZ6iLtP2wanxt4LzdNrN2NNFnpPdq1uJ9n2" });
    mCarrierConfig->mBootstrapNodes.push_back({ "18.216.102.47", "33445", "G5z8MqiNDFTadFUPfMdYsYtkUDbX5mNCMVHMZtsCnFeb" });
    mCarrierConfig->mBootstrapNodes.push_back({ "52.83.127.85",  "33445", "CDkze7mJpSuFAUq6byoLmteyGYMeJ6taXxWoVvDMexWC" });
    mCarrierConfig->mBootstrapNodes.push_back({ "52.83.127.216", "33445", "4sL3ZEriqW7pdoqHSoYXfkc1NMNpiMz7irHMMrMjp9CM" });
    mCarrierConfig->mBootstrapNodes.push_back({ "52.83.171.135", "33445", "5tuHgK1Q4CYf4K5PutsEPK5E3Z7cbtEBdx7LwmdzqXHL" });
    mCarrierConfig->mHiveNodes.push_back({ "18.217.147.205", "9094" });
    mCarrierConfig->mHiveNodes.push_back({ "18.219.53.133", "9094" });

    mElaChainConfig = std::make_unique<ElaChainConfig>();
    mElaChainConfig->mUrl = "https://api-wallet-ela.elastos.org/api/v1";
//    mElaChainConfig->mUrl = "https://api-wallet-ela-testnet.elastos.org/api/1";
    mElaChainConfig->mApi.mGetBalance = "/balance/"; //{address}
    mElaChainConfig->mApi.mCreateTx = "/createTx";
    mElaChainConfig->mApi.mCreateCrossTx = "/createCrossTx";
    mElaChainConfig->mApi.mSendRawTx = "/sendRawTx";
    mElaChainConfig->mApi.mGetTx = "/tx/"; //{address}
    mElaChainConfig->mApi.mGetAllTxs = "/getAllTxs";
    mElaChainConfig->mApi.mGetHistory = "/history/"; //{address}
    mElaChainConfig->mApi.mGetPubKey = "/pubkey/"; //{address}

    mDidChainConfig = std::make_unique<DidChainConfig>();
    mDidChainConfig->mUrl = "https://api-wallet-did.elastos.org/api/1";
    //mDidChainConfig->mUrl = "https://api-wallet-did-testnet.elastos.org/api/1";
    mDidChainConfig->mApi = mElaChainConfig->mApi;
    mDidChainConfig->mAgentApi.mGetDidProps = "/didexplorer/did/"; //{did}
    mDidChainConfig->mAgentApi.mUploadDidProps = "/blockagent/upchain/data";
    mDidChainConfig->mAgentApi.mDidPropHistory = "/property_history?key="; //{key}
    mDidChainConfig->mAgentApi.mDidProp = "/property?key="; //{key}

    return 0;
}

/***********************************************/
/***** class protected function implement  *****/
/***********************************************/


/***********************************************/
/***** class private function implement  *******/
/***********************************************/

} // namespace elastos
