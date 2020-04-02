#ifndef _DIDCHN_DATA_LISTENER_HPP_
#define _DIDCHN_DATA_LISTENER_HPP_

#include "DidChnClient.hpp"
#include <UserManager.hpp>
#include <FriendManager.hpp>

namespace elastos {

class DidChnDataListener : public DidChnClient::MonitorCallback {
public:
    /*** type define ***/

    /*** static function and variable ***/
    static int InitInstance(std::weak_ptr<UserManager> userMgr,
                            std::weak_ptr<FriendManager> friendMgr,
                            std::weak_ptr<MessageManager> msgMgr);
    static std::shared_ptr<DidChnDataListener> GetInstance();

    /*** class function and variable ***/
    int mergeHumanInfo(std::shared_ptr<HumanInfo> humanInfo,
                       const std::string& key, const std::vector<std::string>& didProps);

    virtual void onError(const std::string& did, const std::string& key, int errcode) override;
    virtual int onChanged(const std::string& did, const std::string& key, const std::vector<std::string>& didProps) override;

protected:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/

private:
    /*** type define ***/

    /*** static function and variable ***/
    static std::shared_ptr<DidChnDataListener> gDidChnDataListener;

    /*** class function and variable ***/
    explicit DidChnDataListener(std::weak_ptr<UserManager> userMgr,
                                std::weak_ptr<FriendManager> friendMgr,
                                std::weak_ptr<MessageManager> msgMgr);
    virtual ~DidChnDataListener();

    int processPublicKeyChanged(std::shared_ptr<HumanInfo> humanInfo, const std::vector<std::string>& didProps);
    int processCarrierKeyChanged(std::shared_ptr<HumanInfo> humanInfo, const std::vector<std::string>& didProps);
    int processDetailKeyChanged(std::shared_ptr<HumanInfo> humanInfo, const std::vector<std::string>& didProps);
    int processIdentifyKeyChanged(std::shared_ptr<HumanInfo> humanInfo, const std::vector<std::string>& didProps);
    int processFriendKeyChanged(std::shared_ptr<HumanInfo> humanInfo, const std::vector<std::string>& didProps);

    std::weak_ptr<UserManager> mUserManager;
    std::weak_ptr<FriendManager> mFriendManager;
    std::weak_ptr<MessageManager> mMessageManager;
};

/***********************************************/
/***** class template function implement *******/
/***********************************************/

/***********************************************/
/***** macro definition ************************/
/***********************************************/

} // namespace elastos

#endif /* _DIDCHN_DATA_LISTENER_HPP_ */

