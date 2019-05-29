/**
 * @file	ChannelImplCarrier.hpp
 * @brief	ChannelImplCarrier
 * @details	
 *
 * @author	xxx
 * @author	<xxx@xxx.com>
 * @copyright	(c) 2012 xxx All rights reserved.
 **/

#ifndef _ELASTOS_CHANNEL_IMPL_CARRIER_HPP_
#define _ELASTOS_CHANNEL_IMPL_CARRIER_HPP_

#include <Config.hpp>
#include <ela_carrier.h>
#include <ela_session.h>
#include <MessageChannelStrategy.hpp>
#include <ThreadPool.hpp>

namespace elastos {

class ChannelImplCarrier : public MessageChannelStrategy,
                                  std::enable_shared_from_this<ChannelImplCarrier> {
public:
    /*** type define ***/

    /*** static function and variable ***/
    static bool IsValidCarrierAddress(const std::string& address);
    static bool IsValidCarrierUsrId(const std::string& usrId);
    static int GetCarrierUsrIdByAddress(const std::string& address, std::string& usrId);

    /*** class function and variable ***/
    explicit ChannelImplCarrier(uint32_t chType,
                                std::shared_ptr<ChannelListener> listener,
                                std::weak_ptr<Config> config);
    virtual ~ChannelImplCarrier();

    virtual int preset() override;
    virtual int open() override;
    virtual int close() override;

    virtual int getAddress(std::string& address) override;

    virtual bool isReady() override;

    virtual int requestFriend(const std::string& friendAddr,
                              const std::string& summary,
                              bool remoteRequest = true) override;

    virtual int sendMessage(FriendInfo friendInfo,
                            uint32_t msgType, std::string msgContent) override;
    virtual int sendMessage(FriendInfo friendInfo,
                            uint32_t msgType, std::vector<int8_t> msgContent) override;

protected:
    /*** type define ***/

    /*** static function and variable ***/
    static void OnCarrierConnection(ElaCarrier *carrier,
                                    ElaConnectionStatus status, void *context);
    static void OnCarrierFriendRequest(ElaCarrier *carrier,
                                       const char *friendid, const ElaUserInfo *info,
                                       const char *hello, void *context);
    static void OnCarrierFriendConnection(ElaCarrier *carrier,const char *friendid,
                                          ElaConnectionStatus status, void *context);

    /*** class function and variable ***/
    void runCarrier();

    std::weak_ptr<Config> mConfig;
    std::unique_ptr<ElaCarrier, std::function<void(ElaCarrier*)>> mCarrier;
    std::unique_ptr<ThreadPool> mTaskThread;
    ChannelListener::ChannelStatus mChannelStatus;

}; // class ChannelImplCarrier

} // namespace elastos

#endif /* _ELASTOS_CHANNEL_IMPL_CARRIER_HPP_ */
