#ifndef _ELACHN_CLIENT_HPP_
#define _ELACHN_CLIENT_HPP_

#include <Config.hpp>
#include <SecurityManager.hpp>

namespace elastos {

class HttpClient;

class ElaChnClient {
public:
    /*** type define ***/

    /*** static function and variable ***/
    static int InitInstance(std::weak_ptr<Config> config, std::weak_ptr<SecurityManager> sectyMgr);
    static std::shared_ptr<ElaChnClient> GetInstance();

    /*** class function and variable ***/
    int setConnectTimeout(uint32_t milliSecond);

    int downloadPublicKey(const std::string& elaAddr, std::string& pubKey);

protected:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/

private:
    /*** type define ***/
    struct ElaProtocol {
    };

    /*** static function and variable ***/
    static std::shared_ptr<ElaChnClient> gElaChnClient;

    /*** class function and variable ***/
    explicit ElaChnClient(std::weak_ptr<Config> config, std::weak_ptr<SecurityManager> sectyMgr);
    virtual ~ElaChnClient();

    int downloadElaChnData(std::shared_ptr<HttpClient>& httpClient,
                           const std::string& path, std::string& result);

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

#endif /* _ELACHN_CLIENT_HPP_ */

