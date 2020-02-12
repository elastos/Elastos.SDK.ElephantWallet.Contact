//
//  ContactV1.cpp
//
//  Created by mengxk on 19/03/16.
//  Copyright © 2016 mengxk. All rights reserved.
//

#include <Contact.V1.hpp>
#include <sstream>

#include "ChannelImplCarrier.hpp"
#include "CompatibleFileSystem.hpp"
#include "DateTime.hpp"
#include "DidChnClient.hpp"
#include "DidChnDataListener.hpp"
#include "ElaChnClient.hpp"
#include "Log.hpp"
#include "Platform.hpp"
#include "SafePtr.hpp"
#include "JsonDefine.hpp"


namespace elastos {

/***********************************************/
/***** static variables initialize *************/
/***********************************************/
std::string ContactV1::Factory::sLocalDataDir;

/***********************************************/
/***** static function implement ***************/
/***********************************************/

void ContactV1::Factory::SetLogLevel(int level)
{
    Log::D(Log::TAG, "set log level: %d", level);
    Log::SetLevel(static_cast<Log::Level>(level));
}

int ContactV1::Factory::SetLocalDataDir(const std::string& dir)
{
    if(dir.empty()) {
        return ErrCode::InvalidArgument;
    }

    std::error_code stdErrCode;
    bool ret = elastos::filesystem::create_directories(dir, stdErrCode);
    if(ret == false
    || stdErrCode.value() != 0) {
        int errCode = ErrCode::StdSystemErrorIndex - stdErrCode.value();
        auto errMsg = ErrCode::ToString(errCode);
        Log::D(Log::TAG, "Failed to set local data dir, errcode: %s", errMsg.c_str());
        return errCode;
    }

    sLocalDataDir = dir;
    Log::D(Log::TAG, "set local data dir: %s", sLocalDataDir.c_str());

    return 0;
}

std::shared_ptr<ContactV1> ContactV1::Factory::Create()
{
    struct Impl: ContactV1 {
    };

    return std::make_shared<Impl>();
}

/***********************************************/
/***** class public function implement  ********/
/***********************************************/
void ContactV1::setListener(std::shared_ptr<SecurityManager::SecurityListener> sectyListener,
                          std::shared_ptr<UserManager::UserListener> userListener,
                          std::shared_ptr<FriendManager::FriendListener> friendListener,
                          std::shared_ptr<MessageManager::MessageListener> msgListener)
{
    mSecurityManager->setSecurityListener(sectyListener);
    mUserManager->setUserListener(userListener);
    mFriendManager->setFriendListener(friendListener);
    mMessageManager->setMessageListener(msgListener);
    mHasListener = true;
}

int ContactV1::start()
{
    if(mStarted == true) {
        return ErrCode::RepeatOperationError;
    }

    int ret = initGlobal();
    CHECK_ERROR(ret)

    ret = mUserManager->restoreUserInfo();
    CHECK_ERROR(ret)

    ret = mMessageManager->presetChannels(mConfig);
    CHECK_ERROR(ret)

    ret = mUserManager->ensureUserCarrierInfo();
    CHECK_ERROR(ret)

    ret = mFriendManager->restoreFriendsInfo();
    CHECK_ERROR(ret)

    ret = mMessageManager->openChannels();
    CHECK_ERROR(ret)

//    ret = mUserManager->monitorDidChainData();
//    CHECK_ERROR(ret)
//
//    ret = mFriendManager->monitorDidChainData();
//    CHECK_ERROR(ret)

//    auto dcClient = DidChnClient::GetInstance();
//    ret = dcClient->startMonitor();
//    CHECK_ERROR(ret)

//    ret = monitorDidChainData();
//    CHECK_ERROR(ret)

    mStarted = true;
    return 0;
}

int ContactV1::stop()
{
    if(mStarted == false) {
        return 0;
    }

    int ret = mMessageManager->closeChannels();
    CHECK_ERROR(ret)

    mConfig.reset();

    auto dcClient = DidChnClient::GetInstance();
    ret = dcClient->stopMonitor();
    CHECK_ERROR(ret)

    mStarted = false;
    return 0;
}

bool ContactV1::isStarted()
{
    return mStarted;
}

int ContactV1::syncInfoDownloadFromDidChain()
{
    std::string did;
    int ret = mSecurityManager->getDid(did);
    CHECK_ERROR(ret)

    auto dcClient = DidChnClient::GetInstance();

    std::map<std::string, std::vector<std::string>> didProps;
    ret = dcClient->downloadDidProp(did, false, didProps);
    CHECK_ERROR(ret);

    auto listener = DidChnDataListener::GetInstance();
    for(auto& [key, value]: didProps) {
        ret = listener->onChanged(did, key, value);
        CHECK_ERROR(ret);
    }

    return 0;
}

int ContactV1::syncInfoUploadToDidChain()
{
    auto dcClient = DidChnClient::GetInstance();
    if(dcClient.get() == nullptr) {
        return ErrCode::NotReadyError;
    }

    int ret = dcClient->uploadCachedDidProp();
    CHECK_ERROR(ret)

    return 0;
}

int ContactV1::exportUserData(const std::string& toFile)
{
    if(mHasListener == false) {
        return ErrCode::NoSecurityListener;
    }

    std::string userDataDir;
    int ret = getUserDataDir(userDataDir);
    CHECK_ERROR(ret);

    std::vector<uint8_t> originData;
    auto userDataFilePath = elastos::filesystem::path(userDataDir) / UserManager::DataFileName;
    ret = mSecurityManager->loadCryptoFile(userDataFilePath, originData);
    if(ret == ErrCode::FileNotExistsError) {
        Log::W(Log::TAG, "Ignore to export user, data is not exists.");
    } else {
        CHECK_ERROR(ret)
    }
    std::string userData {originData.begin(), originData.end()};

    originData.clear();
    auto friendDataFilePath = elastos::filesystem::path(userDataDir) / FriendManager::DataFileName;
    ret = mSecurityManager->loadCryptoFile(friendDataFilePath, originData);
    if(ret == ErrCode::FileNotExistsError) {
        Log::W(Log::TAG, "Ignore to export friend, data is not exists.");
    } else {
        CHECK_ERROR(ret)
    }
    std::string friendData {originData.begin(), originData.end()};

    std::string bundledData;
    try {
        Json jsonCache;
        jsonCache[JsonKey::UserData] = userData;
        jsonCache[JsonKey::FriendData] = friendData;
        bundledData = jsonCache.dump();
    } catch(const std::exception& ex) {
        Log::E(Log::TAG, "Failed to export local data to: %s.\nex=%s", toFile.c_str(), ex.what());
        return ErrCode::JsonParseException;
    }

    originData = std::vector<uint8_t> {bundledData.begin(), bundledData.end()};
    ret = mSecurityManager->saveCryptoFile(toFile, originData);
    CHECK_ERROR(ret)

    Log::D(Log::TAG, "Success to export local data to: %s", toFile.c_str());
    return 0;
}


int ContactV1::importUserData(const std::string& fromFile)
{
    if(mHasListener == false) {
        return ErrCode::NoSecurityListener;
    }
    if(mStarted == true) {
        return ErrCode::ExpectedBeforeStartedError;
    }

    std::vector<uint8_t> originData;
    int ret = mSecurityManager->loadCryptoFile(fromFile, originData);
    CHECK_ERROR(ret)
    std::string bundledData {originData.begin(), originData.end()};

    std::string userData;
    std::string friendData;
    try {
        Json jsonCache = Json::parse(bundledData);

        userData = jsonCache[JsonKey::UserData];
        friendData = jsonCache[JsonKey::FriendData];
    } catch(const std::exception& ex) {
        Log::E(Log::TAG, "Failed to import local data from: %s.\nex=%s", fromFile.c_str(), ex.what());
        return ErrCode::JsonParseException;
    }

    std::string userDataDir;
    ret = getUserDataDir(userDataDir);
    CHECK_ERROR(ret);

    auto userDataFilePath = elastos::filesystem::path(userDataDir) / UserManager::DataFileName;
    elastos::filesystem::remove(userDataFilePath);
    if(userData.empty() == false) {
        originData = std::vector<uint8_t>{userData.begin(), userData.end()};
        ret = mSecurityManager->saveCryptoFile(userDataFilePath, originData);
        CHECK_ERROR(ret)
    }

    auto friendDataFilePath = elastos::filesystem::path(userDataDir) / FriendManager::DataFileName;
    elastos::filesystem::remove(friendDataFilePath);
    if(friendData.empty() == false) {
        originData = std::vector<uint8_t>{friendData.begin(), friendData.end()};
        ret = mSecurityManager->saveCryptoFile(friendDataFilePath, originData);
        CHECK_ERROR(ret)
    }

    Log::D(Log::TAG, "Success to import local data from: %s", fromFile.c_str());
    return 0;
}

std::weak_ptr<SecurityManager> ContactV1::getSecurityManager()
{
    return mSecurityManager;
}

std::weak_ptr<UserManager> ContactV1::getUserManager()
{
    return mUserManager;
}

std::weak_ptr<FriendManager> ContactV1::getFriendManager()
{
    return mFriendManager;
}

std::weak_ptr<MessageManager> ContactV1::getMessageManager()
{
    return mMessageManager;
}

/***********************************************/
/***** class protected function implement  *****/
/***********************************************/


/***********************************************/
/***** class private function implement  *******/
/***********************************************/
ContactV1::ContactV1()
    : mSecurityManager(std::make_shared<SecurityManager>())
    , mUserManager(std::make_shared<UserManager>(mSecurityManager))
    , mFriendManager(std::make_shared<FriendManager>(mSecurityManager))
    , mMessageManager(std::make_shared<MessageManager>(mSecurityManager, mUserManager, mFriendManager))
    , mConfig()
    , mHasListener(false)
    , mStarted(false)
{
}

ContactV1::~ContactV1()
{
    auto dcClient = DidChnClient::GetInstance();
    dcClient->stopMonitor();
}


int ContactV1::getUserDataDir(std::string& dir)
{
    if(Factory::sLocalDataDir.empty()) {
        return ErrCode::InvalidLocalDataDir;
    }
    bool isDir = elastos::filesystem::is_directory(Factory::sLocalDataDir);
    if(isDir == false) {
        return ErrCode::InvalidLocalDataDir;
    }
    auto perms = elastos::filesystem::status(Factory::sLocalDataDir).permissions();
    if((perms & elastos::filesystem::perms::owner_read) == elastos::filesystem::perms::none
    || (perms & elastos::filesystem::perms::owner_write) == elastos::filesystem::perms::none) {
        return ErrCode::InvalidLocalDataDir;
    }

    std::string did;
    int ret = mSecurityManager->getDid(did);
    CHECK_ERROR(ret)

    auto userDataDir = elastos::filesystem::path(Factory::sLocalDataDir) / did;
    std::error_code stdErrCode;
    bool bret = elastos::filesystem::create_directories(userDataDir, stdErrCode);
    if(bret == false
    || stdErrCode.value() != 0) {
        int errCode = ErrCode::StdSystemErrorIndex - stdErrCode.value();
        auto errMsg = ErrCode::ToString(errCode);
        Log::D(Log::TAG, "Failed to set user data dir, errcode: %s", errMsg.c_str());
        return errCode;
    }

    dir = userDataDir.string();

    return 0;
}

int ContactV1::initGlobal()
{
    int ret;

    std::string userDataDir;
    ret = getUserDataDir(userDataDir);
    CHECK_ERROR(ret)
    Log::D(Log::TAG, "%s userdatadir:%s", __PRETTY_FUNCTION__, userDataDir.c_str());

    mConfig = std::make_shared<Config>(userDataDir);
    ret = mConfig->load();
    CHECK_ERROR(ret)

    mUserManager->setConfig(mConfig, mMessageManager);
    mFriendManager->setConfig(mConfig, mMessageManager);

    ret = ElaChnClient::InitInstance(mConfig, mSecurityManager);
    CHECK_ERROR(ret)

    ret = DidChnClient::InitInstance(mConfig, mSecurityManager);
    CHECK_ERROR(ret)

    ret = DidChnDataListener::InitInstance(mUserManager, mFriendManager, mMessageManager);
    CHECK_ERROR(ret)

    return 0;
}

int ContactV1::monitorDidChainData()
{
    auto listener = DidChnDataListener::GetInstance();
    auto dcClient = DidChnClient::GetInstance();

    std::string did;
    int ret = mSecurityManager->getDid(did);
    CHECK_ERROR(ret)

    ret = dcClient->appendMoniter(did, listener, false);
    CHECK_ERROR(ret)

    std::vector<std::shared_ptr<FriendInfo>> friendList;
    ret = mFriendManager->getFriendInfoList(friendList);
    CHECK_ERROR(ret)

    for(const auto& it: friendList) {
        ret = it->getHumanInfo(HumanInfo::Item::Did, did);
        if(ret < 0) {
            Log::W(Log::TAG, "ContactV1::monitorDidChainData() Failed to get friend did.");
            continue;
        }

        ret = dcClient->appendMoniter(did, listener, false);
        CHECK_ERROR(ret)
    }

    return 0;
}

} // namespace elastos
