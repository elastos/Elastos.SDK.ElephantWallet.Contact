//
//  Elastos.SDK.Contact.cpp
//
//  Created by mengxk on 19/03/16.
//  Copyright © 2016 mengxk. All rights reserved.
//

#ifndef WITH_CROSSPL

#include <Contact.hpp>

#include <SafePtr.hpp>
#include <JsonDefine.hpp>

/***********************************************/
/***** static variables initialize *************/
/***********************************************/

/***********************************************/
/***** static function implement ***************/
/***********************************************/


/***********************************************/
/***** class public function implement  ********/
/***********************************************/
std::shared_ptr<ElaphantContact::UserInfo> ElaphantContact::getUserInfo()
{
    if(mContactImpl->isStarted() == false) {
        return nullptr;
    }

    auto weakUserMgr = mContactImpl->getUserManager();
    auto userMgr =  SAFE_GET_PTR_DEF_RETVAL(weakUserMgr, nullptr);

    std::shared_ptr<ElaphantContact::UserInfo> userInfo;
    int ret = userMgr->getUserInfo(userInfo);
    CHECK_AND_RETDEF(ret, nullptr);

    return userInfo;
}

int ElaphantContact::sendMessage(const std::string& friendCode, ContactChannel chType, std::shared_ptr<Message> message)
{
    auto str = message->data->toString();
    std::span<uint8_t> data(reinterpret_cast<uint8_t*>(str.data()), str.size());
    auto ret = message->syncMessageToNative(static_cast<int>(message->type), &data,
                                            message->cryptoAlgorithm.c_str(),
                                            message->timestamp);
    CHECK_ERROR(ret);

    ret = ContactBridge::sendMessage(friendCode.c_str(), static_cast<int>(chType), message.get());
    CHECK_ERROR(ret);

    return 0;
}

ElaphantContact::Message::Message(Type type, const std::vector<uint8_t>& data, std::string cryptoAlgorithm, int64_t timestamp)
        : type(type)
        , data()
        , cryptoAlgorithm(cryptoAlgorithm)
        , timestamp(timestamp)
{
    auto msgData = std::string(data.begin(), data.end());
    auto jsonInfo = elastos::Json::parse(msgData);
    switch (type) {
        case Type::MsgText:
            this->data = std::make_shared<TextData>(jsonInfo[elastos::JsonKey::Text]);
            break;
        case Type::MsgBinary:
            this->data = std::make_shared<BinaryData>(jsonInfo[elastos::JsonKey::Binary]);
            break;
        default:
            Log::E(Log::TAG, "Unprocessed Message Type");
            break;
    }
}

std::string ElaphantContact::Message::TextData::toString()
{
    auto jsonInfo = elastos::Json::object();
    jsonInfo[elastos::JsonKey::Text] = text;
    return jsonInfo.dump();
}

std::string ElaphantContact::Message::BinaryData::toString()
{
    auto jsonInfo = elastos::Json::object();
    jsonInfo[elastos::JsonKey::Binary] = binary;
    return jsonInfo.dump();
}


/***********************************************/
/***** class protected function implement  *****/
/***********************************************/


/***********************************************/
/***** class private function implement  *******/
/***********************************************/

#endif // WITH_CROSSPL
