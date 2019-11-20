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
#include <ela_filetransfer.h>
#include <MessageChannelStrategy.hpp>
#include <ThreadPool.hpp>
#include "ChannelImplCarrierDataTrans.hpp"

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
                                std::shared_ptr<ChannelListener> chListener,
                                std::shared_ptr<ChannelDataListener> dataListener,
                                std::weak_ptr<Config> config);
    virtual ~ChannelImplCarrier();

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
                            std::vector<uint8_t> msgContent) override;

    virtual int sendData(const std::string& friendCode,
                         const std::string& dataId) override;

    virtual int cancelSendData(const std::string& friendCode,
                               const std::string& dataId) override;

protected:
    /*** type define ***/
    struct DataCache {
        uint64_t timestamp = 0;
        std::map<int, std::vector<uint8_t>> dataMap;
    };

    /*** static function and variable ***/
    static void OnCarrierConnection(ElaCarrier *carrier,
                                    ElaConnectionStatus status, void *context);
    static void OnCarrierFriendRequest(ElaCarrier *carrier,
                                       const char *friendid, const ElaUserInfo *info,
                                       const char *hello, void *context);
    static void OnCarrierFriendConnection(ElaCarrier *carrier,const char *friendid,
                                          ElaConnectionStatus status, void *context);
    static void OnCarrierFriendMessage(ElaCarrier *carrier, const char *from,
                                                const void *msg, size_t len,
                                                bool offline, void *context);

    static void OnCarrierFileTransConnect(ElaCarrier *carrier, const char *from,
                                          const ElaFileTransferInfo *fileinfo,
                                          void *context);


    static constexpr int32_t MaxPkgSize = 896;
    static constexpr uint8_t PkgMagic[] = { 0xA5, 0xA5, 0x5A, 0x5A,
                                            0x00/*index*/, 0x00/*index*/,
                                            0x00/*count*/, 0x00/*count*/,
                                            0x00, 0x00, 0x00, 0x00, /* timestamp */
                                            0x00, 0x00, 0x00, 0x00, /* timestamp */
                                          };
    static constexpr int32_t PkgMagicSize = 16;
    static constexpr int32_t PkgMagicHeadSize = 4;
    static constexpr int32_t PkgMagicDataIdx = 4;
    static constexpr int32_t PkgMagicDataCnt = 6;
    static constexpr int32_t PkgMagicTimestamp = 8;
    static constexpr int32_t MaxPkgCount = 65535; // sizeof uint16

    /*** class function and variable ***/
    int initCarrier();
    void runCarrier();

    std::weak_ptr<Config> mConfig;
    std::shared_ptr<ElaCarrier> mCarrier;
    std::unique_ptr<ThreadPool> mTaskThread;
    ChannelListener::ChannelStatus mChannelStatus;
    std::map<std::string, DataCache> mRecvDataCache;
    std::unique_ptr<ChannelImplCarrierDataTrans> mCarrierDataTrans;
}; // class ChannelImplCarrier

} // namespace elastos

#endif /* _ELASTOS_CHANNEL_IMPL_CARRIER_HPP_ */
