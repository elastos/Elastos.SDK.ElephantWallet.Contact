//
//  MessageChannelStrategy.cpp
//
//  Created by mengxk on 19/03/16.
//  Copyright © 2016 mengxk. All rights reserved.
//

#include <MessageChannelStrategy.hpp>

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
MessageChannelStrategy::MessageChannelStrategy(uint32_t chType)
    : mChannelType(chType)
    , mChannelListener()
    , mChannelDataListener()
{
}

MessageChannelStrategy::~MessageChannelStrategy()
{
}

int MessageChannelStrategy::preset(const std::string& profile,
                                   std::shared_ptr<ChannelListener> chListener,
                                   std::shared_ptr<ChannelDataListener> dataListener)
{
    mChannelListener = chListener;
    mChannelDataListener = dataListener;

    return 0;
}

std::shared_ptr<MessageChannelStrategy::ChannelListener> MessageChannelStrategy::getChannelListener()
{
    return mChannelListener;
}

std::shared_ptr<MessageChannelStrategy::ChannelDataListener> MessageChannelStrategy::getChannelDataListener()
{
    return mChannelDataListener;
}

uint32_t MessageChannelStrategy::getChannelType()
{
    return mChannelType;
}

/***********************************************/
/***** class protected function implement  *****/
/***********************************************/


/***********************************************/
/***** class private function implement  *******/
/***********************************************/

} // namespace elastos
