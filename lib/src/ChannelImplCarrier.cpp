//
//  ChannelImplCarrier.cpp
//
//  Created by mengxk on 19/03/16.
//  Copyright © 2016 mengxk. All rights reserved.
//

#include <ChannelImplCarrier.hpp>

#include <ela_session.h>
#include <cstring>
#include <Log.hpp>
#include <SafePtr.hpp>
#include <Platform.hpp>
#include <DateTime.hpp>

namespace elastos {

/***********************************************/
/***** static variables initialize *************/
/***********************************************/


/***********************************************/
/***** static function implement ***************/
/***********************************************/
bool ChannelImplCarrier::IsValidCarrierAddress(const std::string& address)
{
    bool valid = ela_address_is_valid(address.c_str());
    return valid;
}

bool ChannelImplCarrier::IsValidCarrierUsrId(const std::string& usrId)
{
    bool valid = ela_id_is_valid(usrId.c_str());
    return valid;
}

int ChannelImplCarrier::GetCarrierUsrIdByAddress(const std::string& address, std::string& usrId)
{
    char buf[ELA_MAX_ID_LEN + 1] = {0};
    auto ret = ela_get_id_by_address(address.c_str(), buf, sizeof(buf));
    if(ret == nullptr) {
        int err = ela_get_error();
        char strerr_buf[512] = {0};
        ela_get_strerror(err, strerr_buf, sizeof(strerr_buf));
        Log::E(Log::TAG, "Failed to get carrier usr id from:%s! ret=%s(0x%x)", address.c_str(), strerr_buf, err);
        return ErrCode::ChannelFailedCarrier;
    }

    usrId = buf;
    return 0;
}


/***********************************************/
/***** class public function implement  ********/
/***********************************************/
ChannelImplCarrier::ChannelImplCarrier(uint32_t chType,
                                       std::weak_ptr<Config> config)
    : MessageChannelStrategy(chType)
    , mConfig(config)
    , mCarrier()
    , mTaskThread()
    , mChannelStatus(ChannelListener::ChannelStatus::Pending)
    , mRecvDataCache()
    , mCarrierDataTrans()
{
#if defined(__ANDROID__)
    Platform::CallOnload(ela_session_jni_onload);
#endif // defined(__ANDROID__)
}

ChannelImplCarrier::~ChannelImplCarrier()
{
    close();
}

int ChannelImplCarrier::preset(const std::string& profile,
                               std::shared_ptr<ChannelListener> chListener,
                               std::shared_ptr<ChannelDataListener> dataListener)
{
    int ret = MessageChannelStrategy::preset(profile, chListener, dataListener);
    CHECK_ERROR(ret)

    if(mCarrier != nullptr) {
        return ErrCode::ChannelFailedMultiOpen;
    }

    ret = initCarrier();
    CHECK_ERROR(ret)

    ret = ela_set_self_nospam(mCarrier.get(), 0);
    if(ret != 0) {
        int err = ela_get_error();
        char strerr_buf[512] = {0};
        ela_get_strerror(err, strerr_buf, sizeof(strerr_buf));
        Log::E(Log::TAG, "Failed to set carrier nospam! ret=%s(0x%x)", strerr_buf, err);
        return ErrCode::ChannelFailedCarrier;
    }


    std::string address;
    ret = ChannelImplCarrier::getAddress(address);
    CHECK_ERROR(ret)
    Log::I(Log::TAG, "ChannelImplCarrier::preset() Success preset carrier on address: %s.", address.c_str());



    return 0;
}

int ChannelImplCarrier::open()
{
    if(mTaskThread == nullptr) {
        mTaskThread = std::make_unique<ThreadPool>("carrier-channel");
    }
    mTaskThread->post(std::bind(&ChannelImplCarrier::runCarrier, this));

    return 0;
}

int ChannelImplCarrier::close()
{
    if (mCarrier == nullptr) {
        ela_session_cleanup(mCarrier.get());
        ela_kill(mCarrier.get());
    }
    mCarrier = nullptr;

    return 0;
}

int ChannelImplCarrier::getAddress(std::string& address)
{
    char addr[ELA_MAX_ADDRESS_LEN + 1] = {0};
    auto ret = ela_get_address(mCarrier.get(), addr, sizeof(addr));
    if(ret == nullptr) {
        int err = ela_get_error();
        char strerr_buf[512] = {0};
        ela_get_strerror(err, strerr_buf, sizeof(strerr_buf));
        Log::E(Log::TAG, "Failed to get address! ret=%s(0x%x)", strerr_buf, err);
        return ErrCode::ChannelFailedCarrier;
    }

    address = addr;
    return 0;
}

bool ChannelImplCarrier::isReady()
{
    return (mChannelStatus == ChannelListener::ChannelStatus::Online);
}

int ChannelImplCarrier::requestFriend(const std::string& friendCode,
                                      const std::string& summary,
                                      bool remoteRequest,
                                      bool forceRequest)
{
    std::function<bool(const char*)> validCheckFun = (remoteRequest ? ela_address_is_valid : ela_id_is_valid);
    bool valid = validCheckFun(friendCode.c_str());
    if(valid != true) {
        return ErrCode::InvalidArgument;
    }

    std::string selfAddr, selfUsrId;
    getAddress(selfAddr);
    GetCarrierUsrIdByAddress(selfAddr, selfUsrId);
    if(friendCode == selfAddr
    || friendCode == selfUsrId) {
        return ErrCode::ChannelFailedFriendSelf;
    }

    int ret = ErrCode::UnknownError;
    if(remoteRequest == true) {
        std::string usrId;
        ret = GetCarrierUsrIdByAddress(friendCode, usrId);
        CHECK_ERROR(ret)

        const char* hello = (summary.empty() ? " " : summary.c_str());
        bool isAdded = ela_is_friend(mCarrier.get(), usrId.c_str());
        if(isAdded == true) {
            if(forceRequest == false) {
                Log::I(Log::TAG, "ChannelImplCarrier::requestFriend() Friend is already exists. friendCode=%s summary=%s", friendCode.c_str(), hello);
                return ErrCode::ChannelFailedFriendExists;
            }
            ela_remove_friend(mCarrier.get(), usrId.c_str());
        }

        Log::I(Log::TAG, "ChannelImplCarrier::requestFriend() friendCode=%s summary=%s", friendCode.c_str(), hello);
        ret = ela_add_friend(mCarrier.get(), friendCode.c_str(), hello);
    } else {
        Log::I(Log::TAG, "ChannelImplCarrier::requestFriend() friendCode=%s", friendCode.c_str());
        ret = ela_accept_friend(mCarrier.get(), friendCode.c_str());
    }
    if(ret != 0) {
        int err = ela_get_error();
        if(err == ELA_GENERAL_ERROR(ELAERR_ALREADY_EXIST)) {
            return ErrCode::ChannelFailedFriendExists;
        }

        char strerr_buf[512] = {0};
        ela_get_strerror(err, strerr_buf, sizeof(strerr_buf));
        Log::E(Log::TAG, "Failed to add friend! ret=%s(0x%x)", strerr_buf, err);
        return ErrCode::ChannelFailedCarrier;
    }

    return 0;
}

int ChannelImplCarrier::removeFriend(const std::string& friendCode)
{
    std::string usrId;
    bool valid = IsValidCarrierUsrId(friendCode);
    if(valid == true) {
        usrId = friendCode;
    } else {
        bool valid = IsValidCarrierAddress(friendCode);
        if (valid == true) {
            GetCarrierUsrIdByAddress(friendCode, usrId);
        }
    }
    if(usrId.empty() == true) {
        return ErrCode::InvalidArgument;
    }

    bool isFriend = ela_is_friend(mCarrier.get(), usrId.c_str());
    if(isFriend == false) {
        Log::W(Log::TAG, "Ignore to remove a friend: %s, it's not exists.", usrId.c_str());
        return 0;
    }

    int ret = ela_remove_friend(mCarrier.get(), usrId.c_str());
    if(ret != 0) {
        int err = ela_get_error();
        if(err == ELA_GENERAL_ERROR(ELAERR_ALREADY_EXIST)) {
            return ErrCode::ChannelFailedFriendExists;
        }

        char strerr_buf[512] = {0};
        ela_get_strerror(err, strerr_buf, sizeof(strerr_buf));
        Log::E(Log::TAG, "Failed to remove friend! ret=%s(0x%x)", strerr_buf, err);
        return ErrCode::ChannelFailedCarrier;
    }

    return 0;
}

int ChannelImplCarrier::sendMessage(const std::string& friendCode,
                                    const std::vector<uint8_t>& msgContent,
                                    bool ignorePackData)
{
    if(ignorePackData == true) {
        bool offlineMsg;
        int ret = ela_send_friend_message(mCarrier.get(), friendCode.c_str(), msgContent.data(), msgContent.size(), &offlineMsg);
        if(ret != 0) {
            int err = ela_get_error();
            char strerr_buf[512] = {0};
            ela_get_strerror(err, strerr_buf, sizeof(strerr_buf));
            Log::E(Log::TAG, "Failed to send signal message! ret=%s(0x%x)", strerr_buf, err);
            return ErrCode::ChannelNotSendMessage;
        }
        Log::D(Log::TAG, "ChannelImplCarrier::sendMessage() success to send %d bytes.", msgContent.size());
        return 0;
    }

    uint64_t pkgCount = msgContent.size() / MaxPkgSize + 1;
    Log::D(Log::TAG, "ChannelImplCarrier::sendMessage() size=%d count=%lld", msgContent.size(), pkgCount);
    if(pkgCount > MaxPkgCount) {
        return ErrCode::ChannelDataTooLarge;
    }

    uint64_t pkgTimestamp = DateTime::CurrentMS();
    for(uint64_t pkgIdx = 0; pkgIdx < pkgCount; pkgIdx++) {
        std::vector<uint8_t> data {std::begin(PkgMagic), std::end(PkgMagic)};
        uint16_t pkgVal = static_cast<uint16_t>(pkgIdx);
        data[PkgMagicDataIdx] = (pkgVal >> 8 ) & 0xFF;
        data[PkgMagicDataIdx + 1] = pkgVal & 0xFF;
        pkgVal = static_cast<uint16_t>(pkgCount);
        data[PkgMagicDataCnt] = (pkgVal >> 8 ) & 0xFF;
        data[PkgMagicDataCnt + 1] = pkgVal & 0xFF;
        uint64_t timestamp = pkgTimestamp;
        for(auto idx = 0; idx < 8; idx++) {
            data[PkgMagicTimestamp + idx] = timestamp & 0xFF;
            timestamp = (timestamp >> 8);
        }

        auto dataBegin = msgContent.begin() + pkgIdx * MaxPkgSize;
        auto dataRemains = msgContent.end() - dataBegin;
        auto dataEnd = (MaxPkgSize < dataRemains ? dataBegin + MaxPkgSize : dataBegin + dataRemains);
        data.insert(data.end(), dataBegin, dataEnd);

        bool offlineMsg;
        int ret = ela_send_friend_message(mCarrier.get(), friendCode.c_str(), data.data(), data.size(), &offlineMsg);
        if(ret != 0) {
            int err = ela_get_error();
            char strerr_buf[512] = {0};
            ela_get_strerror(err, strerr_buf, sizeof(strerr_buf));
            Log::E(Log::TAG, "Failed to send message! ret=%s(0x%x)", strerr_buf, err);
            return ErrCode::ChannelNotSendMessage;
        }
        Log::D(Log::TAG, "ChannelImplCarrier::sendMessage PkgMagicData Idx/Cnt=0x%04llx[%02x,%02x]/0x%04llx[%02x,%02x] Timestamp=0x%016llx[%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x]",
               pkgIdx, data[PkgMagicDataIdx], data[PkgMagicDataIdx + 1], pkgCount, data[PkgMagicDataCnt], data[PkgMagicDataCnt + 1],
               pkgTimestamp, data[PkgMagicTimestamp + 0], data[PkgMagicTimestamp + 1], data[PkgMagicTimestamp + 2], data[PkgMagicTimestamp + 3],
                             data[PkgMagicTimestamp + 4], data[PkgMagicTimestamp + 5], data[PkgMagicTimestamp + 6], data[PkgMagicTimestamp + 7]);
    }

    return 0;
}

int ChannelImplCarrier::sendData(const std::string& friendCode,
                                 const std::string& dataId)
{
    Log::I(Log::TAG, "%s friendCode=%s dataId=%s", __PRETTY_FUNCTION__, friendCode.c_str(), dataId.c_str());
    mCarrierDataTrans = std::make_unique<ChannelImplCarrierDataTrans>(mChannelType, mCarrier, mChannelDataListener);
    int ret = mCarrierDataTrans->start(ChannelImplCarrierDataTrans::Direction::Sender, friendCode, dataId);
    CHECK_ERROR(ret);

    return 0;
}

int ChannelImplCarrier::cancelSendData(const std::string& friendCode,
                                       const std::string& dataId)
{
    Log::I(Log::TAG, "%s friendCode=%s dataId=%s", __PRETTY_FUNCTION__, friendCode.c_str(), dataId.c_str());
    mCarrierDataTrans.reset();
    return 0;
}

/***********************************************/
/***** class protected function implement  *****/
/***********************************************/
int ChannelImplCarrier::initCarrier()
{
    auto config = SAFE_GET_PTR(mConfig);

    ElaOptions carrierOpts;
    ElaCallbacks carrierCallbacks;

    memset(&carrierOpts, 0, sizeof(carrierOpts));
    memset(&carrierCallbacks, 0, sizeof(carrierCallbacks));
    carrierCallbacks.connection_status = OnCarrierConnection;
    carrierCallbacks.friend_request = OnCarrierFriendRequest;
    carrierCallbacks.friend_connection = OnCarrierFriendConnection;
    carrierCallbacks.friend_message = OnCarrierFriendMessage;

    carrierOpts.udp_enabled = config->mCarrierConfig->mEnableUdp;
    carrierOpts.persistent_location = config->mUserDataDir.c_str();
//    Log::W(Log::TAG, "========= Recover carrier by secret key: %s", profile.c_str());
//    if(profile.empty() == false) {
//        carrierOpts.secret_key = profile.c_str();
//    }

    // set BootstrapNode
    size_t carrierNodeSize = config->mCarrierConfig->mBootstrapNodes.size();
    BootstrapNode carrierNodeArray[carrierNodeSize];
    memset (carrierNodeArray, 0, sizeof(carrierNodeArray));
    for(int idx = 0; idx < carrierNodeSize; idx++) {
        const auto& node = config->mCarrierConfig->mBootstrapNodes[idx];
        carrierNodeArray[idx].ipv4 = node.mIpv4.c_str();
        carrierNodeArray[idx].port = node.mPort.c_str();
        carrierNodeArray[idx].public_key = node.mPublicKey.c_str();
    }
    carrierOpts.bootstraps_size = carrierNodeSize;
    carrierOpts.bootstraps = carrierNodeArray;

    // set HiveBootstrapNode
    size_t hiveNodeSize = config->mCarrierConfig->mHiveNodes.size();
    HiveBootstrapNode hiveNodeArray[hiveNodeSize];
    memset (hiveNodeArray, 0, sizeof(hiveNodeArray));
    for(int idx = 0; idx < hiveNodeSize; idx++) {
        const auto& node = config->mCarrierConfig->mHiveNodes[idx];
        hiveNodeArray[idx].ipv4 = node.mIpv4.c_str();
        hiveNodeArray[idx].port = node.mPort.c_str();
    }
    //carrierOpts.hive_bootstraps_size = hiveNodeSize;
    //carrierOpts.hive_bootstraps = hiveNodeArray;

    carrierOpts.log_level = static_cast<ElaLogLevel>(config->mCarrierConfig->mLogLevel);

    auto creater = [&]() -> ElaCarrier* {
        auto ptr = ela_new(&carrierOpts, &carrierCallbacks, this);
        return ptr;
    };
    auto deleter = [=](ElaCarrier* ptr) -> void {
        if(ptr != nullptr) {
            ela_filetransfer_cleanup(ptr);
            ela_session_cleanup(ptr);
            ela_kill(ptr);
        }
    };
    mCarrier = std::shared_ptr<ElaCarrier>(creater(), deleter);
    if (mCarrier.get() == nullptr) {
        Log::E(Log::TAG, "Failed to new carrier!");
        return ErrCode::ChannelFailedCarrier;
    }

    int ret = ela_filetransfer_init(mCarrier.get(), OnCarrierFileTransConnect, this);
    if (ret < 0) {
        Log::E(Log::TAG, "Failed to init filetransfer!");
        return ErrCode::ChannelFailedCarrier;
    }

    return 0;
}

void ChannelImplCarrier::runCarrier()
{
    int ret = ela_run(mCarrier.get(), 500);
    if(ret < 0) {
        ela_kill(mCarrier.get());
        Log::E(Log::TAG, "Failed to run carrier!");
        return;
    }
}

void ChannelImplCarrier::OnCarrierConnection(ElaCarrier *carrier,
                                             ElaConnectionStatus status, void *context)
{
    auto thiz = reinterpret_cast<ChannelImplCarrier*>(context);

    std::string carrierAddr, carrierUsrId;
    thiz->getAddress(carrierAddr);
    GetCarrierUsrIdByAddress(carrierAddr, carrierUsrId);

    thiz->mChannelStatus = ( status == ElaConnectionStatus_Connected
                              ? ChannelListener::ChannelStatus::Online
                              : ChannelListener::ChannelStatus::Offline);
    Log::D(Log::TAG, "ChannelImplCarrier::OnCarrierConnection status: %d", thiz->mChannelStatus);
    if(thiz->mChannelListener.get() != nullptr) {
        thiz->mChannelListener->onStatusChanged(carrierUsrId, thiz->mChannelType, thiz->mChannelStatus);
    }

    if(status == ElaConnectionStatus_Disconnected) {
        thiz->cancelSendData("", "");
    }
}

void ChannelImplCarrier::OnCarrierFriendRequest(ElaCarrier *carrier, const char *friendid,
                                                const ElaUserInfo *info,
                                                const char *hello, void *context)
{
    Log::D(Log::TAG, "ChannelImplCarrier::OnCarrierFriendRequest from: %s", friendid);
    auto thiz = reinterpret_cast<ChannelImplCarrier*>(context);

    if(thiz->mChannelListener.get() != nullptr) {
        thiz->mChannelListener->onFriendRequest(friendid, thiz->mChannelType, hello);
    }
}

void ChannelImplCarrier::OnCarrierFriendConnection(ElaCarrier *carrier,const char *friendid,
                                                   ElaConnectionStatus status, void *context)
{
    Log::D(Log::TAG, "=-=-=-=-= ChannelImplCarrier::OnCarrierFriendConnection from: %s %d", friendid, status);
    auto thiz = reinterpret_cast<ChannelImplCarrier*>(context);

    if(thiz->mChannelListener.get() != nullptr) {
        auto chStatus = ( status == ElaConnectionStatus_Connected
                        ? ChannelListener::ChannelStatus::Online
                        : ChannelListener::ChannelStatus::Offline);
        thiz->mChannelListener->onFriendStatusChanged(friendid, thiz->mChannelType, chStatus);
    }

    if(status == ElaConnectionStatus_Disconnected) {
        thiz->cancelSendData(friendid, "");
    }
}

void ChannelImplCarrier::OnCarrierFriendMessage(ElaCarrier *carrier, const char *from,
                                                const void *msg, size_t len,
                                                bool offline, void *context)
{
    Log::D(Log::TAG, "ChannelImplCarrier::OnCarrierFriendMessage from: %s len=%d", from, len);

    auto thiz = reinterpret_cast<ChannelImplCarrier*>(context);
    if(thiz->mChannelListener.get() == nullptr) {
        Log::W(Log::TAG, "ChannelListener is not set. ignore to process");
        return;
    }

    auto data = reinterpret_cast<const uint8_t*>(msg);

    int32_t dataOffset = 0;
    bool dataComplete = true;
    bool isPkgData = true;
    for(auto idx = 0; idx < PkgMagicHeadSize; idx++) {
        if(data[idx] != PkgMagic[idx]) {
            isPkgData = false;
            break;
        }
        dataOffset = PkgMagicSize;
    }

    auto dataSection = std::vector<uint8_t>(data + dataOffset, data + len);
    if(isPkgData == true) {
        uint64_t pkgIdx = data[PkgMagicDataIdx];
        pkgIdx = (pkgIdx << 8) + data[PkgMagicDataIdx + 1];
        uint64_t pkgCount = data[PkgMagicDataCnt];
        pkgCount = (pkgCount << 8) + data[PkgMagicDataCnt + 1];
        uint64_t pkgTimestamp = 0;
        for(auto idx = 0; idx < 8; idx++) {
            uint64_t tmp = data[PkgMagicTimestamp + idx];
            pkgTimestamp += (tmp << (8 * idx));
        }

//        dataComplete = (pkgIdx == pkgCount - 1 ? true : false);
        Log::D(Log::TAG, "ChannelImplCarrier::OnCarrierFriendMessage PkgMagicData Idx/Cnt=0x%04llx[%02x,%02x]/0x%04llx[%02x,%02x] Timestamp=0x%016llx[%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x]",
               pkgIdx, data[PkgMagicDataIdx], data[PkgMagicDataIdx + 1], pkgCount, data[PkgMagicDataCnt], data[PkgMagicDataCnt + 1],
               pkgTimestamp, data[PkgMagicTimestamp + 0], data[PkgMagicTimestamp + 1], data[PkgMagicTimestamp + 2], data[PkgMagicTimestamp + 3],
                             data[PkgMagicTimestamp + 4], data[PkgMagicTimestamp + 5], data[PkgMagicTimestamp + 6], data[PkgMagicTimestamp + 7]);

        auto& dataCache = thiz->mRecvDataCache[from];
        if(dataCache.timestamp > pkgTimestamp) { // if new message is too old, ignore it.
            Log::W(Log::TAG, "Ignore to process received carrier old message!");
            return;
        } else if(dataCache.timestamp < pkgTimestamp) { // if new message is too new, clear cached
            Log::W(Log::TAG, "Ignore to process cached carrier old message!");
            dataCache.dataMap.clear();
            dataCache.timestamp = pkgTimestamp;
        }
        dataCache.dataMap[pkgIdx] = std::move(dataSection);

        if(dataCache.dataMap.size() == pkgCount) {
            std::vector<uint8_t> dataPkg;
            for(int idx = 0; idx < dataCache.dataMap.size(); idx++) {
                dataPkg.insert(dataPkg.end(), dataCache.dataMap[idx].begin(), dataCache.dataMap[idx].end());
            }

            thiz->mChannelListener->onReceivedMessage(from, thiz->mChannelType, dataPkg);
            dataCache.dataMap.clear();
        }
    } else {
        thiz->mChannelListener->onReceivedMessage(from, thiz->mChannelType, dataSection);
    }

}

void ChannelImplCarrier::OnCarrierFileTransConnect(ElaCarrier *carrier,
                                                   const char *from,
                                                   const ElaFileTransferInfo *fileinfo,
                                                   void *context)
{
    Log::I(Log::TAG, "%s from=%s fileinfo=%p", __PRETTY_FUNCTION__, from, fileinfo);
    if(fileinfo != nullptr) {
        Log::I(Log::TAG, "filename=%s", fileinfo->filename);
    }
    auto thiz = reinterpret_cast<ChannelImplCarrier*>(context);

    thiz->mCarrierDataTrans = std::make_unique<ChannelImplCarrierDataTrans>(thiz->mChannelType, thiz->mCarrier, thiz->mChannelDataListener);
    int ret = thiz->mCarrierDataTrans->start(ChannelImplCarrierDataTrans::Direction::Receiver, from);
    CHECK_RETVAL(ret);
}

/***********************************************/
/***** class private function implement  *******/
/***********************************************/

} // namespace elastos
