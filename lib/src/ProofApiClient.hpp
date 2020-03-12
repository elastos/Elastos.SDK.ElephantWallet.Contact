#ifndef _PROOF_API_CLIENT_HPP_
#define _PROOF_API_CLIENT_HPP_


#include <Config.hpp>
#include <SecurityManager.hpp>

namespace elastos {

class HttpClient;

class ProofApiClient {
public:
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
    static int InitInstance(std::weak_ptr<Config> config, std::weak_ptr<SecurityManager> sectyMgr);
    static std::shared_ptr<ProofApiClient> GetInstance();

    /*** class function and variable ***/
    int setConnectTimeout(uint32_t milliSecond);
    int getOssInfo(OssInfo& ossInfo);

protected:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/

private:
    /*** type define ***/

    /*** static function and variable ***/
    static std::shared_ptr<ProofApiClient> gProofApiClient;

    /*** class function and variable ***/
    explicit ProofApiClient(std::weak_ptr<Config> config, std::weak_ptr<SecurityManager> sectyMgr);
    virtual ~ProofApiClient();

    int getVerifyCode(std::shared_ptr<HttpClient> httpClient, std::string& verifyCode);
    int getOssInfo(std::shared_ptr<HttpClient> httpClient, const std::string signedVerifyCode,
                   OssInfo& ossInfo);

    std::weak_ptr<Config> mConfig;
    std::weak_ptr<SecurityManager> mSecurityManager;
    uint32_t mConnectTimeoutMS;
};

/***********************************************/
/***** class template function implement *******/
/***********************************************/

/***********************************************/
/***** macro definition ************************/
/***********************************************/

} // namespace elastos

#endif /* _PROOF_API_CLIENT_HPP_ */

