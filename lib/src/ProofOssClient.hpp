#ifndef _PROOF_OSS_CLIENT_HPP_
#define _PROOF_OSS_CLIENT_HPP_


#include <set>
#include <Config.hpp>
#include <RemoteStorageManager.hpp>
#include <SecurityManager.hpp>

namespace elastos {

namespace sdk {
class CloudPartition;
};
class HttpClient;

class ProofOssClient : public RemoteStorageManager::RemoteStorageClient {
public:
    /*** type define ***/
    struct OssAuth {
        std::string user;
        std::string password;
        std::string token;
        std::string disk;
        std::string partition;
        std::string rootdir;

        int64_t expiredTime = 0;
        bool isDefaultOss = false;
    };

    /*** static function and variable ***/
    std::string CalculateAuthHash(const std::shared_ptr<OssAuth> ossAuth);

    /*** class function and variable ***/
    explicit ProofOssClient(std::weak_ptr<Config> config, std::weak_ptr<SecurityManager> sectyMgr);
    virtual ~ProofOssClient();

    virtual int genAuthHash() override;
    virtual std::string getAuthHash() override;
    virtual void setAuthHash(const std::string& authHash) override;

    virtual int uploadProperties(const std::multimap<std::string, std::string>& changedPropMap,
                                 const std::map<std::string, std::shared_ptr<std::iostream>>& totalPropMap) override;
    virtual int downloadProperties(const std::string& fromDid,
                                   std::multimap<std::string, std::string>& savedPropMap,
                                   std::map<std::string, std::shared_ptr<std::iostream>>& totalPropMap) override;

    virtual int migrateTo(const std::shared_ptr<OssAuth> to);
    virtual int restoreOssAuth(const std::shared_ptr<OssAuth> ossAuth);
    virtual int getOssAuth(std::shared_ptr<OssAuth>& ossAuth);

protected:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/

private:
    /*** type define ***/

    /*** static function and variable ***/
    static constexpr const char* DataFileName = "oss-cacheddata.dat";

    /*** class function and variable ***/
    int getDefaultOssAuth(std::shared_ptr<OssAuth>& ossAuth);
    int getDefaultVerifyCode(std::shared_ptr<HttpClient> httpClient, std::string& verifyCode);
    int getDefaultOssAuth(std::shared_ptr<HttpClient> httpClient, const std::string signedVerifyCode,
                            std::shared_ptr<OssAuth>& ossAuth);
    int ossLogin();
    int ossList(std::vector<std::string>& pathList);
    int ossWrite(const std::string& path, std::shared_ptr<std::iostream> content);
    int ossRead(const std::string& path, std::shared_ptr<std::iostream> content);
    int ossDelete(const std::string& path);

//    int loadLocalData();
//    int saveLocalData();

    std::weak_ptr<Config> mConfig;
    std::weak_ptr<SecurityManager> mSecurityManager;
    std::string mAuthHash;
    std::shared_ptr<OssAuth> mOssAuth;
    std::shared_ptr<elastos::sdk::CloudPartition> mOssPartition;
};

/***********************************************/
/***** class template function implement *******/
/***********************************************/

/***********************************************/
/***** macro definition ************************/
/***********************************************/

} // namespace elastos

#endif /* _PROOF_OSS_CLIENT_HPP_ */

