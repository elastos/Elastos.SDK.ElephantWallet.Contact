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
    auto dataInfo = message->data->toData();
    std::span<uint8_t> data(dataInfo.data(), dataInfo.size());
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
    switch (type) {
        case Type::MsgText:
            this->data = std::make_shared<TextData>(std::string(data.begin(), data.end()));
            break;
        case Type::MsgBinary:
            this->data = std::make_shared<BinaryData>(data);
            break;
        default:
            Log::E(Log::TAG, "Unprocessed Message Type");
            break;
    }
}

std::string ElaphantContact::Message::TextData::toString()
{
    return this->text;
}

std::vector<uint8_t> ElaphantContact::Message::TextData::toData()
{
    return std::vector<uint8_t>(this->text.begin(), this->text.end());
}
void ElaphantContact::Message::TextData::fromData(const std::vector<uint8_t>& data)
{
    this->text = std::string(data.begin(), data.end());
}

std::string ElaphantContact::Message::BinaryData::toString()
{
    auto jsonInfo = elastos::Json(this->binary);
    return jsonInfo.dump();
}

std::vector<uint8_t> ElaphantContact::Message::BinaryData::toData()
{
    return this->binary;
}
void ElaphantContact::Message::BinaryData::fromData(const std::vector<uint8_t>& data)
{
    this->binary = data;
}

/***********************************************/
/***** class protected function implement  *****/
/***********************************************/


/***********************************************/
/***** class private function implement  *******/
/***********************************************/

#endif // WITH_CROSSPL
