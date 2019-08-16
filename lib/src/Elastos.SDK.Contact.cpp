//
//  Elastos.SDK.Contact.cpp
//
//  Created by mengxk on 19/03/16.
//  Copyright © 2016 mengxk. All rights reserved.
//

#include <Elastos.SDK.Contact.hpp>

#include "ChannelImplCarrier.hpp"
#include "CompatibleFileSystem.hpp"
#include "DateTime.hpp"
#include "DidChnClient.hpp"
#include "DidChnDataListener.hpp"
#include "Log.hpp"
#include "Platform.hpp"
#include "SafePtr.hpp"


namespace elastos {

/***********************************************/
/***** static variables initialize *************/
/***********************************************/
std::string Contact::Factory::sLocalDataDir;

/***********************************************/
/***** static function implement ***************/
/***********************************************/

void Contact::Factory::SetLogLevel(int level)
{
    Log::D(Log::TAG, "set log level: %d", level);
    Log::SetLevel(static_cast<Log::Level>(level));
}

int Contact::Factory::SetLocalDataDir(const std::string& dir)
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

std::shared_ptr<Contact> Contact::Factory::Create()
{
    struct Impl: Contact {
    };

    return std::make_shared<Impl>();
}

/***********************************************/
/***** class public function implement  ********/
/***********************************************/
void Contact::setListener(std::shared_ptr<SecurityManager::SecurityListener> sectyListener,
                          std::shared_ptr<UserManager::UserListener> userListener,
                          std::shared_ptr<FriendManager::FriendListener> friendListener,
                          std::shared_ptr<MessageManager::MessageListener> msgListener)
{
    mSecurityManager->setSecurityListener(sectyListener);
    mUserManager->setUserListener(userListener);
    mFriendManager->setFriendListener(friendListener);
    mMessageManager->setMessageListener(msgListener);
}

int Contact::start()
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

    ret = monitorDidChainData();
    CHECK_ERROR(ret)
    auto dcClient = DidChnClient::GetInstance();
    ret = dcClient->startMonitor();
    CHECK_ERROR(ret)

    mStarted = true;
    return 0;
}

bool Contact::isStarted()
{
    return mStarted;
}

int Contact::syncInfoDownloadFromDidChain()
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

int Contact::syncInfoUploadToDidChain()
{
    auto dcClient = DidChnClient::GetInstance();
    if(dcClient.get() == nullptr) {
        return ErrCode::NotReadyError;
    }

    int ret = dcClient->uploadCachedDidProp();
    CHECK_ERROR(ret)

    return 0;
}

std::weak_ptr<SecurityManager> Contact::getSecurityManager()
{
    return mSecurityManager;
}

std::weak_ptr<UserManager> Contact::getUserManager()
{
    return mUserManager;
}

std::weak_ptr<FriendManager> Contact::getFriendManager()
{
    return mFriendManager;
}

std::weak_ptr<MessageManager> Contact::getMessageManager()
{
    return mMessageManager;
}

/***********************************************/
/***** class protected function implement  *****/
/***********************************************/


/***********************************************/
/***** class private function implement  *******/
/***********************************************/
Contact::Contact()
    : mSecurityManager(std::make_shared<SecurityManager>())
    , mUserManager(std::make_shared<UserManager>(mSecurityManager))
    , mFriendManager(std::make_shared<FriendManager>(mSecurityManager))
    , mMessageManager(std::make_shared<MessageManager>(mSecurityManager, mUserManager, mFriendManager))
    , mConfig()
    , mStarted(false)
{
}

Contact::~Contact()
{
}


int Contact::getUserDataDir(std::string& dir)
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

int Contact::initGlobal()
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

    ret = DidChnClient::InitInstance(mConfig, mSecurityManager);
    CHECK_ERROR(ret)

    ret = DidChnDataListener::InitInstance(mUserManager, mFriendManager, mMessageManager);
    CHECK_ERROR(ret)

    return 0;
}

int Contact::monitorDidChainData()
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
            Log::W(Log::TAG, "Contact::monitorDidChainData() Failed to get friend did.");
            continue;
        }

        ret = dcClient->appendMoniter(did, listener, false);
        CHECK_ERROR(ret)
    }

    return 0;
}

} // namespace elastos
