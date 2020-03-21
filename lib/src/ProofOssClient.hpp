#ifndef _PROOF_OSS_CLIENT_HPP_
#define _PROOF_OSS_CLIENT_HPP_


#include <set>
#include <Config.hpp>
#include <SecurityManager.hpp>
#include <UserInfo.hpp>
#include "RemoteStorageManager.hpp"

namespace elastos {

namespace sdk {
class CloudPartition;
};
class HttpClient;

class ProofOssClient : public RemoteStorageManager::RemoteStorageClient {
public:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/
    explicit ProofOssClient(std::weak_ptr<Config> config, std::weak_ptr<SecurityManager> sectyMgr);
    virtual ~ProofOssClient();

    virtual int uploadProperties(const std::map<std::string, std::string>& changedPropMap,
                                 const std::map<std::string, std::shared_ptr<std::iostream>>& totalPropMap) override;
    virtual int downloadProperties(std::map<std::string, std::string>& changedPropMap,
                                   std::map<std::string, std::shared_ptr<std::iostream>>& totalPropMap) override;

protected:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/

private:
    /*** type define ***/
    struct OssInfo {
        std::string mUser;
        std::string mPassword;
        std::string mToken;
        std::string mDisk;
        std::string mPartition;
        std::string mPath;
    };

    /*** static function and variable ***/
    static constexpr const char* DataFileName = "oss-cacheddata.dat";

    /*** class function and variable ***/
    int getOssInfo(OssInfo& ossInfo);
    int getVerifyCode(std::shared_ptr<HttpClient> httpClient, std::string& verifyCode);
    int getOssInfo(std::shared_ptr<HttpClient> httpClient, const std::string signedVerifyCode,
                   OssInfo& ossInfo);
    int ossLogin();
    int ossList(std::vector<std::string>& pathList);
    int ossWrite(const std::string& path, std::shared_ptr<std::iostream> content);
    int ossRead(const std::string& path, std::shared_ptr<std::iostream> content);

//    int loadLocalData();
//    int saveLocalData();

    std::weak_ptr<Config> mConfig;
    std::weak_ptr<SecurityManager> mSecurityManager;
    std::set<std::string> mFileCache;
    std::shared_ptr<elastos::sdk::CloudPartition> mOssPartition;
    int64_t mExpiredTime = 0;
};

/***********************************************/
/***** class template function implement *******/
/***********************************************/

/***********************************************/
/***** macro definition ************************/
/***********************************************/

} // namespace elastos

#endif /* _PROOF_OSS_CLIENT_HPP_ */

