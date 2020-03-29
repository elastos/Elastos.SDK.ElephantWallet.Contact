//
//  ChannelImplCarrier.cpp
//
//  Created by mengxk on 19/03/16.
//  Copyright © 2016 mengxk. All rights reserved.
//

#include <ChannelImplCarrierDataTrans.hpp>

//#include <ela_session.h>
#include <cstring>
#include <Log.hpp>
#include <ErrCode.hpp>
#include <SafePtr.hpp>
#include <Platform.hpp>

namespace elastos {

/***********************************************/
/***** static variables initialize *************/
/***********************************************/


/***********************************************/
/***** static function implement ***************/
/***********************************************/

/***********************************************/
/***** class public function implement  ********/
/***********************************************/
ChannelImplCarrierDataTrans::ChannelImplCarrierDataTrans(uint32_t channelType,
                                                         std::shared_ptr<ElaCarrier> carrier,
                                                         std::shared_ptr<MessageChannelStrategy::ChannelDataListener> listener)
        : mChannelType(channelType)
        , mCarrier(carrier)
        , mChannelDataListener(listener)
        , mDataTransMutex()
        , mDataTransThread("carrier-channel-datatrans")
        , mCarrierFileTrans(nullptr)
        , mDataTransDirection(Invalid)
        , mDataTransStatus(MessageChannelStrategy::ChannelDataListener::Status::Unknown)
        , mDataId()
        , mDataRecvOffset(0)
{
}

ChannelImplCarrierDataTrans::~ChannelImplCarrierDataTrans()
{
    stop();
}

int ChannelImplCarrierDataTrans::start(Direction direction,
                                        const std::string& friendCode,
                                        const std::string& dataId)
{
    if(dataId.length() > ELA_MAX_FILE_NAME_LEN) {
        return ErrCode::ChannelInvalidDataId;
    }

    std::unique_lock<std::recursive_mutex> lock(mDataTransMutex);
    if(mCarrierFileTrans != nullptr) {
        return ErrCode::ChannelFileTransBusy;
    }

    ElaFileTransferCallbacks ftCallbacks = {};
    ftCallbacks.state_changed = DataTransListener::OnStateChanged;
//    ftCallbacks.cancel        = DataTransListener::OnCancel;
    ftCallbacks.pull          = DataSendListener::OnPull;
    ftCallbacks.file          = DataRecvListener::OnFile;
    ftCallbacks.data          = DataRecvListener::OnData;

    Log::I(Log::TAG, "%s ela_filetransfer_new()", FORMAT_METHOD);
    mCarrierFileTrans = ela_filetransfer_new(mCarrier.get(), friendCode.c_str(), nullptr,
                                             &ftCallbacks, this);
    if (mCarrierFileTrans == nullptr) {
        int err = ela_get_error();
        char strerr_buf[512] = {0};
        ela_get_strerror(err, strerr_buf, sizeof(strerr_buf));
        Log::E(Log::TAG, "Failed to make file transfer! ret=%s(0x%x)", strerr_buf, err);
        return ErrCode::ChannelFailedCarrier;
    }

    mDataRecvOffset = 0;
    mDataTransDirection = direction;
    mFriendId = friendCode;
    mDataId = dataId;

    setDataTransStatus(MessageChannelStrategy::ChannelDataListener::Status::Initialized);

    int ret = -1;
    if(mDataTransDirection == Sender) {
        ret = ela_filetransfer_connect(mCarrierFileTrans);
    } else if(mDataTransDirection == Receiver) {
        ret = ela_filetransfer_accept_connect(mCarrierFileTrans);
    }
    if (ret < 0) {
        int err = ela_get_error();
        char strerr_buf[512] = {0};
        ela_get_strerror(err, strerr_buf, sizeof(strerr_buf));
        Log::E(Log::TAG, "Failed to start data transfer! ret=%s(0x%x)", strerr_buf, err);
        return ErrCode::ChannelFailedCarrier;
    }

    return 0;
}

int ChannelImplCarrierDataTrans::stop()
{
    std::unique_lock<std::recursive_mutex> lock(mDataTransMutex);
    if(mCarrierFileTrans != nullptr) {
        ela_filetransfer_close(mCarrierFileTrans);
        mCarrierFileTrans = nullptr;
    }

    setDataTransStatus(MessageChannelStrategy::ChannelDataListener::Status::Destroyed);
    mDataRecvOffset = 0;
    mFriendId = "";
    mDataTransDirection = Invalid;

    return 0;
}

/***********************************************/
/***** class protected function implement  *****/
/***********************************************/
void ChannelImplCarrierDataTrans::runDataTransfer(const std::string fileid, uint64_t offset)
{
    mDataRecvOffset = offset;
    std::vector<uint8_t> data;
    int closeReason;

    setDataTransStatus(MessageChannelStrategy::ChannelDataListener::Status::Transmitting);
    for(;;) {
        std::unique_lock<std::recursive_mutex> lock(mDataTransMutex);
        Log::I(Log::TAG, "%s %d", FORMAT_METHOD, __LINE__);
        if(mCarrierFileTrans == nullptr) { // data transfer closed.
            Log::W(Log::TAG, "Failed to send data. user closed.");
            break;
        }

        data.clear();
        int ret = mChannelDataListener->onReadData(mFriendId, mChannelType,
                                               mDataId, mDataRecvOffset,
                                               data);
        if(ret > 0) {
            ret = ela_filetransfer_send(mCarrierFileTrans, fileid.c_str(), data.data(), data.size());
            if(ret > 0) {
                mDataRecvOffset += ret;
                continue;
            }

            int err = ela_get_error();
            char strerr_buf[512] = {0};
            ela_get_strerror(err, strerr_buf, sizeof(strerr_buf));
            Log::E(Log::TAG, "Failed to send data ret=%s(0x%x)", strerr_buf, err);
            closeReason = err;
            // close transfer
        } else if(ret == 0) { // finished
            ret = ela_filetransfer_send(mCarrierFileTrans, fileid.c_str(), nullptr, 0);
            if(ret < 0) {
                int err = ela_get_error();
                char strerr_buf[512] = {0};
                ela_get_strerror(err, strerr_buf, sizeof(strerr_buf));
                Log::E(Log::TAG, "Failed to cancel send data err=%s(0x%x)", strerr_buf, err);
            }
            Log::I(Log::TAG, "Finish to send data.");
            closeReason = 0;
        } else {
            closeReason = ret;
        }

        break;
    }

    Log::I(Log::TAG, "%s end.", FORMAT_METHOD);

    std::unique_lock<std::recursive_mutex> lock(mDataTransMutex);
    stop();
}

void ChannelImplCarrierDataTrans::setDataTransStatus(MessageChannelStrategy::ChannelDataListener::Status status)
{
    if(mDataTransStatus != status) {
        mChannelDataListener->onNotify(mFriendId, mChannelType, mDataId, status);
    }

    mDataTransStatus = status;
}

void ChannelImplCarrierDataTrans::DataTransListener::OnStateChanged(ElaFileTransfer *filetransfer,
                                                                    FileTransferConnection state,
                                                                    void *context)
{
    Log::I(Log::TAG, "%s state=%d", FORMAT_METHOD, state);
    auto thiz = reinterpret_cast<ChannelImplCarrierDataTrans*>(context);
    std::unique_lock<std::recursive_mutex> lock(thiz->mDataTransMutex);

    auto dataStatus = MessageChannelStrategy::ChannelDataListener::Status::Unknown;

    switch (state) {
        case FileTransferConnection_connecting:
            dataStatus = MessageChannelStrategy::ChannelDataListener::Status::Connecting;
            break;
        case FileTransferConnection_connected:
            dataStatus = MessageChannelStrategy::ChannelDataListener::Status::Connected;
            if(thiz->mDataTransDirection == Sender) { // sender
                ElaFileTransferInfo fileInfo = {};

                strncpy(fileInfo.filename, "carrier bug workaround", ELA_MAX_FILE_NAME_LEN);
                fileInfo.size = -1; // not used
                ela_filetransfer_add(filetransfer, &fileInfo);

                strcpy(fileInfo.filename, thiz->mDataId.c_str());
                fileInfo.size = -1; // not used
                int ret = ela_filetransfer_add(filetransfer, &fileInfo);
                Log::I(Log::TAG, "%s add ret=%d", FORMAT_METHOD, ret);
                CHECK_RETVAL(ret);
            }
            break;
        case FileTransferConnection_closed:
        case FileTransferConnection_failed:
            if(state == FileTransferConnection_closed) {
                dataStatus = MessageChannelStrategy::ChannelDataListener::Status::Closed;
            } else {
                dataStatus = MessageChannelStrategy::ChannelDataListener::Status::Failed;
            }
            thiz->mDataTransThread.post([=]() {
                thiz->stop();
            });
            break;
        default:
            break;
    }

    thiz->setDataTransStatus(dataStatus);

}

void ChannelImplCarrierDataTrans::DataSendListener::OnPull(ElaFileTransfer *filetransfer,
                                                  const char *fileid,
                                                  uint64_t offset,
                                                  void *context)
{
    Log::I(Log::TAG, "%s fileid=%s size=%llu", FORMAT_METHOD, fileid, offset);
    auto thiz = reinterpret_cast<ChannelImplCarrierDataTrans*>(context);
    std::unique_lock<std::recursive_mutex> lock(thiz->mDataTransMutex);

    thiz->mDataTransThread.post(std::bind(&ChannelImplCarrierDataTrans::runDataTransfer, thiz,
                                          std::string(fileid), offset));
}

void ChannelImplCarrierDataTrans::DataRecvListener::OnFile(ElaFileTransfer *filetransfer,
                                                  const char *fileid,
                                                  const char *filename, uint64_t size,
                                                  void *context)
{
    Log::I(Log::TAG, "%s fileid=%s name=%s size=%llu", FORMAT_METHOD, fileid, filename, size);
    auto thiz = reinterpret_cast<ChannelImplCarrierDataTrans*>(context);
    std::unique_lock<std::recursive_mutex> lock(thiz->mDataTransMutex);

    thiz->mDataId = filename;

    int ret = ela_filetransfer_pull(filetransfer, fileid, 0);
    CHECK_RETVAL(ret);

//    throw std::runtime_error(std::string(FORMAT_METHOD) + " Unimplemented!!!");
}

bool ChannelImplCarrierDataTrans::DataRecvListener::OnData(ElaFileTransfer *filetransfer,
                                                  const char *fileid,
                                                  const uint8_t *data, size_t length,
                                                  void *context) {
    Log::I(Log::TAG, "%s fileid=%s data=%p size=%d", FORMAT_METHOD, fileid, data, length);
    auto thiz = reinterpret_cast<ChannelImplCarrierDataTrans*>(context);
    std::unique_lock<std::recursive_mutex> lock(thiz->mDataTransMutex);

    thiz->setDataTransStatus(MessageChannelStrategy::ChannelDataListener::Status::Transmitting);

    std::vector<uint8_t> dataRecv;
    if (length > 0) {
        dataRecv = std::vector<uint8_t>(data, data + length);
    }

    int ret = thiz->mChannelDataListener->onWriteData(thiz->mFriendId, thiz->mChannelType,
                                                      thiz->mDataId, thiz->mDataRecvOffset,
                                                      dataRecv);
    if(ret < 0) {
        return false;
    }

    thiz->mDataRecvOffset += length;

    Log::I(Log::TAG, "%s fileid=%s end offset=%llu", FORMAT_METHOD, fileid, thiz->mDataRecvOffset);

    if(length == 0) {
        thiz->stop();
    }

    return true;
}

/***********************************************/
/***** class private function implement  *******/
/***********************************************/

} // namespace elastos
