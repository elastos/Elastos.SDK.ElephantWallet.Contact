#ifndef _REMOTE_STORAGE_MANAGER_HPP_
#define _REMOTE_STORAGE_MANAGER_HPP_

#include <map>
#include <string>

namespace elastos {

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
        virtual int init() { return 0; }
        virtual int cacheProperty(const char* key, const std::string& value,
                                  const std::string& savedAt, const std::string& extra) = 0;
        virtual int uploadCachedProp() = 0;
    protected:
        explicit RemoteStorageClient() = default;
        virtual ~RemoteStorageClient() = default;
    };

    /*** static function and variable ***/

    /*** class function and variable ***/
    explicit RemoteStorageManager() = default;
    virtual ~RemoteStorageManager() = default;

    void addClient(ClientType type, std::shared_ptr<RemoteStorageClient> client);

    int cacheProperty(const char* key, const std::string& value,
                      const std::string& savedAt, const std::string& extra = "");
    int uploadCachedProp();

private:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/

    std::map<ClientType, std::shared_ptr<RemoteStorageClient>> mRemoteStorageClientMap;
};

/***********************************************/
/***** class template function implement *******/
/***********************************************/

/***********************************************/
/***** macro definition ************************/
/***********************************************/

} // namespace elastos

#endif /* _REMOTE_STORAGE_MANAGER_HPP_ */

