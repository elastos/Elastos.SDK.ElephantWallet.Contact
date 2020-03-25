#ifndef _DID_CHAIN_CLIENT_HPP_
#define _DID_CHAIN_CLIENT_HPP_


#include <map>
#include <Config.hpp>
#include <RemoteStorageManager.hpp>
#include <SecurityManager.hpp>

namespace elastos {

class HttpClient;

class DidChainClient : public RemoteStorageManager::RemoteStorageClient {
public:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/
    explicit DidChainClient(std::weak_ptr<Config> config, std::weak_ptr<SecurityManager> sectyMgr);
    virtual ~DidChainClient();

    virtual int uploadProperties(const std::multimap<std::string, std::string>& changedPropMap,
                                 const std::map<std::string, std::shared_ptr<std::iostream>>& totalPropMap) override;
    virtual int downloadProperties(const std::string& fromDid,
                                   std::multimap<std::string, std::string>& savedPropMap,
                                   std::map<std::string, std::shared_ptr<std::iostream>>& totalPropMap) override;

protected:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/

private:
    /*** type define ***/
    struct DidProtocol {
        struct Name {
            static constexpr const char* Tag = "Tag";
            static constexpr const char* Ver = "Ver";
            static constexpr const char* Properties = "Properties";
            static constexpr const char* Key = "Key";
            static constexpr const char* Value = "Value";
            static constexpr const char* Status = "Status";
        };

        struct Value {
            static constexpr const char* Tag = "DID Property";
            static constexpr const char* Ver = "1.0";
            struct Status {
                static constexpr const char* Normal = "Normal";
                static constexpr const char* Deprecated = "Deprecated";
            };
        };
    };

    /*** static function and variable ***/
    static const std::map<std::string, bool> mDidPropHistoryMap;

    /*** class function and variable ***/
    int uploadDidPropsByAgent(const std::string& didAgentData);
    int downloadDidPropsByAgent(const std::string& did, const std::string& key,
                                std::shared_ptr<std::vector<std::string>>& values);
    int downloadDidChnData(const std::string& path, std::string& result);
    int serializeDidProps(const std::multimap<std::string, std::string>& changedPropMap,
                          std::string& result);
    int makeDidAgentData(const std::string& didProtocolData, std::string& result);
    int getDidPropPath(const std::string& did, const std::string& key, bool withHistory, std::string& path, bool useCache);
    int getPropKeyPath(const std::string& key, std::string& keyPath);

    std::weak_ptr<Config> mConfig;
    std::weak_ptr<SecurityManager> mSecurityManager;
    std::string mPropKeyPathPrefix;
};

/***********************************************/
/***** class template function implement *******/
/***********************************************/

/***********************************************/
/***** macro definition ************************/
/***********************************************/

} // namespace elastos

#endif /* _DID_CHAIN_CLIENT_HPP_ */

