/**
 * @file	Contact.V1.hpp
 * @brief	Contact
 * @details	
 *
 * @author	xxx
 * @author	<xxx@xxx.com>
 * @copyright	(c) 2012 xxx All rights reserved.
 **/

#ifndef _ELASTOS_SDK_CONTACT_V1_HPP_
#define _ELASTOS_SDK_CONTACT_V1_HPP_

#include "Config.hpp"
#include "FriendManager.hpp"
#include "MessageManager.hpp"
#include "SecurityManager.hpp"
#include "UserManager.hpp"
#include "RemoteStorageManager.hpp"

namespace elastos {

class ContactV1 {
public:
    /*** type define ***/
    class Factory {
    public:
        static void SetLogLevel(int level);
        static int SetLocalDataDir(const std::string& dir);
        static std::shared_ptr<ContactV1> Create();

    private:
        static std::string sLocalDataDir;
        friend class ContactV1;
    }; // class Factory

    class Listener { // TODO
    public:
        virtual void onRequest() = 0;
        virtual void onEvent() = 0;
        virtual void onError(int errCode) = 0;
    protected:
        explicit Listener() = default;
        virtual ~Listener() = default;
    }; // class ContactListener

    struct SyncInfoLocation {
        static constexpr const int DidChain = 1;
        static constexpr const int Oss = 2;
    };

    /*** static function and variable ***/

    /*** class function and variable ***/
    void setListener(std::shared_ptr<SecurityManager::SecurityListener> sectyListener,
                     std::shared_ptr<UserManager::UserListener> userListener = nullptr,
                     std::shared_ptr<FriendManager::FriendListener> friendListener = nullptr,
                     std::shared_ptr<MessageManager::MessageListener> msgListener = nullptr);

    std::weak_ptr<SecurityManager> getSecurityManager();
    std::weak_ptr<UserManager> getUserManager();
    std::weak_ptr<FriendManager> getFriendManager();
    std::weak_ptr<MessageManager> getMessageManager();

    int start();
    int stop();
    bool isStarted();

    int syncInfoDownloadFromDidChain();
    int syncInfoUploadToDidChain();

    int syncInfoAuthOss(const std::string& user, const std::string& password, const std::string& token,
                        const std::string& disk, const std::string& partition, const std::string& path);
    int syncInfoDownload(int fromLocation);
    int syncInfoUpload(int toLocation);

    int exportUserData(const std::string& toFile);
    int importUserData(const std::string& fromFile);

private:
    /*** type define ***/

    /*** static function and variable ***/
    static constexpr const char* UserDataSeparator = "--------------------------";

    /*** class function and variable ***/
    explicit ContactV1();
    virtual ~ContactV1();
    int getUserDataDir(std::string& dir);
    int initGlobal();
    int monitorDidChainData();

    std::shared_ptr<SecurityManager> mSecurityManager;
    std::shared_ptr<UserManager> mUserManager;
    std::shared_ptr<FriendManager> mFriendManager;
    std::shared_ptr<MessageManager> mMessageManager;
    std::shared_ptr<RemoteStorageManager> mRemoteStorageManager;
    std::shared_ptr<Config> mConfig;
    bool mHasListener;
    bool mGlobalInited;
    bool mStarted;
}; // class ContactV1

} // namespace elastos

#endif /* _ELASTOS_SDK_CONTACT_V1_HPP_ */
