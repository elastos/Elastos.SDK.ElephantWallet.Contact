/**
 * @file	Contact.hpp
 * @brief	Contact
 * @details	
 *
 * @author	xxx
 * @author	<xxx@xxx.com>
 * @copyright	(c) 2012 xxx All rights reserved.
 **/

#ifndef _ELASTOS_SDK_CONTACT_HPP_
#define _ELASTOS_SDK_CONTACT_HPP_

#include <Elastos.SDK.Contact.hpp>
#include <ContactBridge.hpp>
#include <ContactDataListener.hpp>
#include <ContactFactory.hpp>
#include <ContactListener.hpp>
#include <ContactMessage.hpp>
#include <memory>


class ElaphantContact : public ContactBridge {
public:
    /*** type define ***/
    class Factory final: public ContactFactory {
       public:
        static std::shared_ptr<ElaphantContact> Create() {
            struct Impl : ElaphantContact {
            };

            return std::make_shared<Impl>();
        }

        // void SetDeviceId(const std::string& devId) {
        //     ContactFactory.SetDeviceId(devId);
        //     // UserInfo.SetCurrDevId(devId);
        // }

        private:
         explicit Factory() = default;
         virtual ~Factory() = default;
    }; // class Factory

    class Listener: public ContactListener {
    public:
    }; // class Listener

    class DataListener: public ContactDataListener {
    }; // class DataListener

    using UserInfo = elastos::UserInfo;

    class Message: public ContactMessage {
        // public Message(String text, String cryptoAlgorithm) {
        //     super(text, cryptoAlgorithm);
        // }

        // public Message(byte[] binary, String cryptoAlgorithm) {
        //     super(binary, cryptoAlgorithm);
        // }

        // public Message(File file, String cryptoAlgorithm) {
        //     super(file, cryptoAlgorithm);
        // }

        // public Message(Type type, byte[] data, String cryptoAlgorithm) {
        //     super(type, data, cryptoAlgorithm);
        // }

    }; // class Message


    /*** static function and variable ***/

    /*** class function and variable ***/
    std::shared_ptr<ElaphantContact::UserInfo> getUserInfo();


private:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/
    explicit ElaphantContact() = default;
    virtual ~ElaphantContact() = default;


}; // class Contact

#ifdef WITH_CROSSPL
} //namespace native
} //namespace crosspl
#endif // WITH_CROSSPL

#endif /* _ELASTOS_SDK_CONTACT_HPP_ */
