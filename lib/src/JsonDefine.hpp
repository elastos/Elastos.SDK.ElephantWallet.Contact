#ifndef _JSON_DEFINE_HPP_
#define _JSON_DEFINE_HPP_

#include <Json.hpp>

#include <HumanInfo.hpp>
#include <MessageManager.hpp>

namespace elastos {

/***********************************************/
/***** macro definition ************************/
/***********************************************/
struct JsonKey {
    static constexpr const char* UpdateTime = "UpdateTime";

    // HumanInfo
    static constexpr const char* HumanInfo          = "HumanInfo";
    static constexpr const char* BoundCarrierArray  = "BoundCarrierArray";
    static constexpr const char* BoundCarrierStatus = "BoundCarrierStatus";
    static constexpr const char* CommonInfoMap      = "CommonInfoMap";
    static constexpr const char* WalletAddressMap   = "WalletAddressMap";
    static constexpr const char* StatusMap          = "StatusMap";
    static constexpr const char* Status             = "Status";
    static constexpr const char* HumanCode          = "HumanCode";
    static constexpr const char* IsMyself          = "IsMyself";
    static constexpr const char* IsFriend          = "IsFriend";

    static constexpr const char* DeviceId   = "DevId";
    static constexpr const char* DeviceName = "DevName";

    static constexpr const char* CarrierAddr = "CarrierAddr";
    static constexpr const char* CarrierId   = "CarrierId";
    static constexpr const char* DeviceInfo  = "DeviceInfo";

    // FriendInfo
    static constexpr const char* FriendCode = "FriendCode";
    static constexpr const char* Alias = "Alias";

    // IdentifyCode
    static constexpr const char* IdentifyCode        = "IdentifyCode";
    static constexpr const char* IdCodeMap           = "IdCodeMap";

    // MessageInfo
    static constexpr const char* Did         = "Did";
    static constexpr const char* Summary     = "Summary";
    static constexpr const char* MessageData = "MessageData";


    static constexpr const char* Type            = "Type";
    static constexpr const char* PlainContent    = "PlainContent";
    static constexpr const char* CryptoAlgorithm = "CryptoAlgorithm";
    static constexpr const char* TimeStamp       = "TimeStamp";

    static constexpr const char* Text            = "Text";
    static constexpr const char* Binary          = "Binary";
    static constexpr const char* Name            = "Name";
    static constexpr const char* Size            = "Size";
    static constexpr const char* Md5             = "Md5";

    static constexpr const char* DidPropCache       = "DidPropCache";
};

// HumanInfo
NLOHMANN_JSON_SERIALIZE_ENUM(
    HumanInfo::Status,
    {
        {HumanInfo::Status::Invalid,       "Invalid"},
        {HumanInfo::Status::WaitForAccept, "WaitForAccept"},
        {HumanInfo::Status::Offline,       "Offline"},
        {HumanInfo::Status::Online,        "Online"},
        {HumanInfo::Status::Removed,       "Removed"},
    }
);

inline void to_json(Json& j, const HumanInfo::CarrierInfo::DeviceInfo& info) {
    j = Json {
        {JsonKey::DeviceId, info.mDevId},
        {JsonKey::DeviceName, info.mDevName},
    };
}

inline void from_json(const Json& j, HumanInfo::CarrierInfo::DeviceInfo& info) {
    info.mDevId = j[JsonKey::DeviceId];
    info.mDevName = j[JsonKey::DeviceName];
}

inline void to_json(Json& j, const HumanInfo::CarrierInfo& info) {
    j = Json {
        {JsonKey::CarrierAddr, info.mUsrAddr},
        {JsonKey::CarrierId, info.mUsrId},
        {JsonKey::DeviceInfo, info.mDevInfo},
        {JsonKey::UpdateTime, info.mUpdateTime},
    };
}

inline void from_json(const Json& j, HumanInfo::CarrierInfo& info) {
    info.mUsrAddr = j[JsonKey::CarrierAddr];
    info.mUsrId = j[JsonKey::CarrierId];
    info.mDevInfo = j[JsonKey::DeviceInfo];
    info.mUpdateTime = j[JsonKey::UpdateTime];
}

// MessageInfo
NLOHMANN_JSON_SERIALIZE_ENUM(
    MessageManager::MessageType,
    {
        {MessageManager::MessageType::MsgText, "MsgText"},
        {MessageManager::MessageType::MsgAudio, "MsgAudio"},
        {MessageManager::MessageType::MsgTransfer, "MsgTransfer"},
        {MessageManager::MessageType::MsgImage, "MsgImage"},
        {MessageManager::MessageType::MsgFile, "MsgFile"},
        {MessageManager::MessageType::MsgBinary, "MsgBinary"},

        {MessageManager::MessageType::CtrlSyncDesc, "CtrlSyncDesc"},
        {MessageManager::MessageType::CtrlPullData, "CtrlPullData"},
        {MessageManager::MessageType::CtrlPullDataAck, "CtrlPullDataAck"},
    }
);

inline void to_json(Json& j, const std::shared_ptr<MessageManager::MessageInfo>& info) {
    j = Json {
        {JsonKey::Type, info->mType},
        {JsonKey::PlainContent, info->mPlainContent},
        {JsonKey::CryptoAlgorithm, info->mCryptoAlgorithm},
        {JsonKey::TimeStamp, info->mTimeStamp},
    };
}

inline void from_json(const Json& j, std::shared_ptr<MessageManager::MessageInfo>& info) {
    info = MessageManager::MakeEmptyMessage();
    info->mType = j[JsonKey::Type];
    info->mPlainContent = j[JsonKey::PlainContent].get<std::vector<uint8_t>>();
    info->mCryptoAlgorithm = j[JsonKey::CryptoAlgorithm];
    info->mTimeStamp = j[JsonKey::TimeStamp];
}

inline void to_json(Json& j, const std::shared_ptr<MessageManager::FileInfo>& info) {
    j = Json {
        {JsonKey::DeviceId, info->mDevId},
        {JsonKey::Name, info->mName},
        {JsonKey::Size, info->mSize},
        {JsonKey::Md5, info->mMd5},
    };
}

inline void from_json(const Json& j, std::shared_ptr<MessageManager::FileInfo>& info) {
    info = MessageManager::MakeEmptyFileInfo();
    info->mDevId = j[JsonKey::DeviceId];
    info->mName = j[JsonKey::Name];
    info->mSize = j[JsonKey::Size];
    info->mMd5 = j[JsonKey::Md5];
}


//    struct DataInfo {

} //namespace elastos

#endif /* _JSON_DEFINE_HPP_ */

