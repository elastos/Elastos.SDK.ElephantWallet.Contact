//
//  IdentifyCode.cpp
//
//  Created by mengxk on 19/03/16.
//  Copyright © 2016 mengxk. All rights reserved.
//

#include <IdentifyCode.hpp>

#include <DateTime.hpp>
#include <JsonDefine.hpp>
#include <Log.hpp>
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
IdentifyCode::IdentifyCode()
    : mIdCodeMap()
    , mUpdateTime()
{
}

IdentifyCode::~IdentifyCode()
{
}

int IdentifyCode::setIdentifyCode(Type type, const std::string& value)
{
    const auto& it = mIdCodeMap.find(type);
    if (it != mIdCodeMap.end()
    && it->second == value) {
        return 0;
    }

    mIdCodeMap[type] = value;
    mUpdateTime = DateTime::CurrentMS();

    return static_cast<int>(type);
}

int IdentifyCode::getIdentifyCode(Type type, std::string& value) const
{
    const auto& it = mIdCodeMap.find(type);
    if(it == mIdCodeMap.end()) {
        return ErrCode::NotFoundError;
    }
    value = it->second;

    return 0;
}

int IdentifyCode::mergeIdentifyCode(const IdentifyCode& value)
{
    if(this->mUpdateTime >= value.mUpdateTime) {
        return ErrCode::IgnoreMergeOldInfo;
    }

    for(const auto& it: value.mIdCodeMap) {
        int ret = IdentifyCode::setIdentifyCode(it.first, it.second);
        if(ret < 0) {
            Log::W(Log::TAG, "IdentifyCode::mergeIdentifyCode() Failed to merge %d:%s", it.first, it.second.c_str());
        }
    }

    return 0;
}

int IdentifyCode::serialize(std::string& value) const
{
    Json jsonInfo = Json::object();

    jsonInfo[JsonKey::IdCodeMap] = mIdCodeMap;
    jsonInfo[JsonKey::UpdateTime] = mUpdateTime;

    value = jsonInfo.dump();

    return 0;
}

int IdentifyCode::deserialize(const std::string& value)
{
    Json jsonInfo;
    try {
        jsonInfo= Json::parse(value);
    } catch(Json::parse_error) {
        return ErrCode::JsonParseException;
    }

    if(jsonInfo.find(JsonKey::IdCodeMap) != jsonInfo.end()) {
        mIdCodeMap = jsonInfo[JsonKey::IdCodeMap].get<std::map<Type, std::string>>();
    }

    mUpdateTime = jsonInfo[JsonKey::UpdateTime];

    return 0;
}

int IdentifyCode::toJson(std::shared_ptr<Json>& value) const
{
    if(value.get() == nullptr) {
        return ErrCode::InvalidArgument;
    }

    Json jsonInfo;
    jsonInfo[JsonKey::IdCodeMap] = mIdCodeMap;

    (*value)[JsonKey::IdentifyCode] = jsonInfo;

    return 0;
}

/***********************************************/
/***** class protected function implement  *****/
/***********************************************/


/***********************************************/
/***** class private function implement  *******/
/***********************************************/

} // namespace elastos
