#ifndef _REMOTE_STORAGE_MANAGER_HPP_
#define _REMOTE_STORAGE_MANAGER_HPP_

#include <map>
#include <mutex>
#include <set>
#include <string>
#include <Config.hpp>
#include <SecurityManager.hpp>

namespace elastos {

class FriendInfo;
class FriendManager;
class UserInfo;
class UserManager;

class RemoteStorageManager final {
public:
    /*** type define ***/
    enum ClientType {
        DidChain = 1,
        Oss = 2
    };

    struct PropKey {
        static constexpr const char* PublicKey     = "PublicKey";
        static constexpr const char* CarrierInfo   = "CarrierInfo";
        static constexpr const char* DetailKey     = "DetailKey";
        static constexpr const char* IdentifyKey   = "IdentifyKey";
        static constexpr const char* FriendKey     = "FriendKey";
    };

    class RemoteStorageClient {
    public:
        virtual int uploadProperties(const std::map<std::string, std::string>& changedPropMap,
                                     const std::map<std::string, std::shared_ptr<std::iostream>>& totalPropMap) = 0;
        virtual int downloadProperties(std::map<std::string, std::string>& changedPropMap,
                                       std::map<std::string, std::shared_ptr<std::iostream>>& totalPropMap) = 0;
    protected:
        explicit RemoteStorageClient() = default;
        virtual ~RemoteStorageClient() = default;
    };

    /*** static function and variable ***/

    /*** class function and variable ***/
    explicit RemoteStorageManager() = default;
    virtual ~RemoteStorageManager() = default;

    int setConfig(std::weak_ptr<Config> config,
                  std::weak_ptr<SecurityManager> sectyMgr,
                  std::weak_ptr<UserManager> userMgr,
                  std::weak_ptr<FriendManager> friendMgr);

    void addClient(ClientType type, std::shared_ptr<RemoteStorageClient> client);

    int cacheProperty(const std::string& did, const char* key);
    int uploadData(const std::shared_ptr<UserInfo> userInfo,
                   const std::vector<std::shared_ptr<FriendInfo>>& friendInfoList);
    int downloadData(std::shared_ptr<UserInfo>& userInfo,
                     std::vector<std::shared_ptr<FriendInfo>>& friendInfoList);

private:
    /*** type define ***/

    /*** static function and variable ***/
    static constexpr const char* DataFileName = "remotestorage-cacheddata.dat";

    /*** class function and variable ***/
    int loadLocalData();
    int saveLocalData();

    int packUserSegment(const std::shared_ptr<UserInfo> userInfo,
                        const std::string& propKey,
                        std::string& segment);
    int packFriendSegment(const std::vector<std::shared_ptr<FriendInfo>>& friendInfoList,
                          const std::string& friendCode,
                          std::string& segment);

    int unpackUserData(const std::string& data,
                       std::shared_ptr<UserInfo>& userInfo);
    int unpackFriendData(const std::string& data,
                         std::vector<std::shared_ptr<FriendInfo>>& friendInfoList);

    std::recursive_mutex mMutex;
    std::weak_ptr<Config> mConfig;
    std::weak_ptr<SecurityManager> mSecurityManager;
    std::weak_ptr<UserManager> mUserManager;
    std::weak_ptr<FriendManager> mFriendManager;
    std::map<ClientType, std::shared_ptr<RemoteStorageClient>> mRemoteStorageClientMap;
    std::map<std::string, std::string> mPropCache;
};

/***********************************************/
/***** class template function implement *******/
/***********************************************/

/***********************************************/
/***** macro definition ************************/
/***********************************************/

} // namespace elastos

#endif /* _REMOTE_STORAGE_MANAGER_HPP_ */

