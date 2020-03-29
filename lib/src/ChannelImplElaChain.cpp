//
//  ChannelImplElaChain.cpp
//
//  Created by mengxk on 19/03/16.
//  Copyright © 2016 mengxk. All rights reserved.
//

#include <ChannelImplElaChain.hpp>

#include <Log.hpp>

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
ChannelImplElaChain::ChannelImplElaChain(uint32_t chType,
                                         std::weak_ptr<Config> config,
                                         std::weak_ptr<SecurityManager> sectyMgr)
    : MessageChannelStrategy(chType)
{
}

ChannelImplElaChain::~ChannelImplElaChain()
{
}

int ChannelImplElaChain::preset(const std::string& profile,
                                std::shared_ptr<ChannelListener> chListener,
                                std::shared_ptr<ChannelDataListener> dataListener)
{
    throw std::runtime_error(std::string(FORMAT_METHOD) + " Unimplemented!!!");
}

int ChannelImplElaChain::open()
{
    throw std::runtime_error(std::string(FORMAT_METHOD) + " Unimplemented!!!");
}

int ChannelImplElaChain::close()
{
    throw std::runtime_error(std::string(FORMAT_METHOD) + " Unimplemented!!!");
}

int ChannelImplElaChain::getAddress(std::string& address)
{
    throw std::runtime_error(std::string(FORMAT_METHOD) + " Unimplemented!!!");
}

bool ChannelImplElaChain::isReady()
{
    throw std::runtime_error(std::string(FORMAT_METHOD) + " Unimplemented!!!");
}

int ChannelImplElaChain::requestFriend(const std::string& friendAddr,
                                       const std::string& summary,
                                       bool remoteRequest,
                                       bool forceRequest)
{
    throw std::runtime_error(std::string(FORMAT_METHOD) + " Unimplemented!!!");
}

int ChannelImplElaChain::removeFriend(const std::string& friendAddr)
{
    throw std::runtime_error(std::string(FORMAT_METHOD) + " Unimplemented!!!");
}

int ChannelImplElaChain::sendMessage(const std::string& friendCode,
                                     const std::vector<uint8_t>& msgContent,
                                     bool ignorePackData)
{
    throw std::runtime_error(std::string(FORMAT_METHOD) + " Unimplemented!!!");
}

/***********************************************/
/***** class protected function implement  *****/
/***********************************************/


/***********************************************/
/***** class private function implement  *******/
/***********************************************/

} // namespace elastos
