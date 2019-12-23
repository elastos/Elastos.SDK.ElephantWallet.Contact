//
//  Elastos.SDK.Contact.cpp
//
//  Created by mengxk on 19/03/16.
//  Copyright © 2016 mengxk. All rights reserved.
//

#include <ContactFactory.hpp>

#ifdef WITH_CROSSPL
#ifdef __ANDROID__
#include "CrossPLUtils.hpp"
#endif // __ANDROID__
#endif // WITH_CROSSPL
#include <Elastos.SDK.Contact.hpp>
#include <Platform.hpp>
#include "Log.hpp"

namespace crosspl {
namespace native {

/***********************************************/
/***** static variables initialize *************/
/***********************************************/

/***********************************************/
/***** static function implement ***************/
/***********************************************/

void ContactFactory::SetLogLevel(int level)
{
    elastos::Contact::Factory::SetLogLevel(level);

    return;
}

void ContactFactory::SetDeviceId(const std::string& devId)
{
#ifdef WITH_CROSSPL
#ifdef __ANDROID__
    auto jvm = crosspl::CrossPLUtils::GetJavaVM();
    elastos::Platform::SetJavaVM(jvm);
#endif // __ANDROID__
#endif // WITH_CROSSPL

    elastos::Platform::SetCurrentDevId(devId);
}

int ContactFactory::SetLocalDataDir(const std::string& dir)
{
    int ret = elastos::Contact::Factory::SetLocalDataDir(dir);

    return ret;
}

/***********************************************/
/***** class public function implement  ********/
/***********************************************/

/***********************************************/
/***** class protected function implement  *****/
/***********************************************/


/***********************************************/
/***** class private function implement  *******/
/***********************************************/

} //namespace native
} //namespace crosspl
