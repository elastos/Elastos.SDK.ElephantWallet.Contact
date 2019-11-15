//
//  Elastos.SDK.Contact.cpp
//
//  Created by mengxk on 19/03/16.
//  Copyright © 2016 mengxk. All rights reserved.
//

#include <Contact.hpp>

#include <SafePtr.hpp>

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

/***********************************************/
/***** class protected function implement  *****/
/***********************************************/


/***********************************************/
/***** class private function implement  *******/
/***********************************************/
