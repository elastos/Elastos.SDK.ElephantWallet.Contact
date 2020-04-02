#ifndef _REMOTE_STORAGE_MANAGER_HPP_
#define _REMOTE_STORAGE_MANAGER_HPP_

#include <map>
#include <mutex>
#include <set>
#include <string>
#include <Config.hpp>
#include <SecurityManager.hpp>

namespace elastos {

class HumanInfo;
class FriendInfo;
class UserInfo;

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
        virtual int genAuthHash() { return 0; };
        virtual std::string getAuthHash() { return "DefaultAuthHash"; };
        virtual void setAuthHash(const std::string& authHash) {};

        virtual int uploadProperties(const std::multimap<std::string, std::string>& changedPropMap,
                                     const std::map<std::string, std::shared_ptr<std::iostream>>& totalPropMap) = 0;
        virtual int downloadProperties(const std::string& fromDid,
                                       std::multimap<std::string, std::string>& savedPropMap,
                                       std::map<std::string, std::shared_ptr<std::iostream>>& totalPropMap) = 0;
    protected:
        explicit RemoteStorageClient() = default;
        virtual ~RemoteStorageClient() = default;

        using PropKey = RemoteStorageManager::PropKey;
    };

    /*** static function and variable ***/

    /*** class function and variable ***/
    explicit RemoteStorageManager() = default;
    virtual ~RemoteStorageManager() = default;

    int setConfig(std::weak_ptr<Config> config,
                  std::weak_ptr<SecurityManager> sectyMgr);
    int ensureRemoteStorageHash();

    void addClient(ClientType type, std::shared_ptr<RemoteStorageClient> client);
    int getClient(ClientType type, std::shared_ptr<RemoteStorageClient>& client);

    int cacheProperty(const std::string& did, const char* key);
    int uploadData(const std::vector<ClientType>& toClientList,
                   const std::shared_ptr<UserInfo> userInfo,
                   const std::vector<std::shared_ptr<FriendInfo>>& friendInfoList,
                   const std::shared_ptr<std::fstream> carrierData);
    int downloadData(const std::vector<ClientType>& fromClientList,
                     std::shared_ptr<UserInfo>& userInfo,
                     std::vector<std::shared_ptr<FriendInfo>>& friendInfoList,
                     std::shared_ptr<std::fstream>& carrierData);

    int unpackHumanSegment(const std::string& segment,
                           const std::string& propKey,
                           std::shared_ptr<HumanInfo>& humanInfo);

private:
    /*** type define ***/

    /*** static function and variable ***/
    static constexpr const char* DataFileName = "remotestorage-cacheddata.dat";

    /*** class function and variable ***/
    int loadLocalData();
    int saveLocalData();

    int downloadData(ClientType fromClient,
                     std::shared_ptr<UserInfo>& userInfo,
                     std::vector<std::shared_ptr<FriendInfo>>& friendInfoList,
                     std::shared_ptr<std::iostream>& carrierData);
    int downloadData(ClientType fromClient,
                     const std::vector<std::string>& propKeyList,
                     const std::vector<std::string>& propFileList,
                     std::shared_ptr<UserInfo>& userInfo,
                     std::vector<std::shared_ptr<FriendInfo>>& friendInfoList,
                     std::shared_ptr<std::iostream>& carrierData);

    int packUserSegment(const std::shared_ptr<UserInfo> userInfo,
                        const std::string& propKey,
                        std::string& segment);
    int unpackUserSegment(const std::string& segment,
                          const std::string& propKey,
                          std::shared_ptr<UserInfo>& userInfo);
    int packFriendSegment(const std::string& friendCode,
                          const std::vector<std::shared_ptr<FriendInfo>>& friendInfoList,
                          std::string& segment);
    int unpackFriendSegment(const std::string& segment,
                            std::vector<std::shared_ptr<FriendInfo>>& friendInfoList);

    int unpackUserData(const std::string& data,
                       std::shared_ptr<UserInfo>& userInfo);
    int unpackFriendData(const std::string& data,
                         std::vector<std::shared_ptr<FriendInfo>>& friendInfoList);

    int mergeUserInfo(const std::shared_ptr<UserInfo>& from, const std::shared_ptr<UserInfo>& to);

    std::recursive_mutex mMutex;
    std::weak_ptr<Config> mConfig;
    std::weak_ptr<SecurityManager> mSecurityManager;
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

