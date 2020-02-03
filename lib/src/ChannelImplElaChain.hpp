/**
 * @file	ChannelImplElaChain.hpp
 * @brief	ChannelImplElaChain
 * @details	
 *
 * @author	xxx
 * @author	<xxx@xxx.com>
 * @copyright	(c) 2012 xxx All rights reserved.
 **/

#ifndef _ELASTOS_CHANNEL_IMPL_ELACHAIN_HPP_
#define _ELASTOS_CHANNEL_IMPL_ELACHAIN_HPP_

#include <Config.hpp>
#include <MessageChannelStrategy.hpp>
#include <SecurityManager.hpp>
#include <ThreadPool.hpp>

namespace elastos {

class ChannelImplElaChain : public MessageChannelStrategy {
public:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/
    explicit ChannelImplElaChain(uint32_t chType,
                                 std::shared_ptr<ChannelListener> chListener,
                                 std::shared_ptr<ChannelDataListener> dataListener,
                                 std::weak_ptr<Config> config,
                                 std::weak_ptr<SecurityManager> sectyMgr);
    virtual ~ChannelImplElaChain();

    virtual int preset(const std::string& profile) override;
    virtual int open() override;
    virtual int close() override;

    virtual int getAddress(std::string& address) override;

    virtual bool isReady() override;

    virtual int requestFriend(const std::string& friendAddr,
                              const std::string& summary,
                              bool remoteRequest = true,
                              bool forceRequest = false) override;

    virtual int removeFriend(const std::string& friendAddr) override;

    virtual int sendMessage(const std::string& friendCode,
                            std::vector<uint8_t> msgContent,
                            bool ignorePackData = false) override;

private:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/

}; // class ChannelImplElaChain

} // namespace elastos

#endif /* _ELASTOS_CHANNEL_IMPL_ELACHAIN_HPP_ */
