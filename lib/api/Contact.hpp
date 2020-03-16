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

#include <Contact.V1.hpp>
#include <ContactBridge.hpp>
#include <ContactChannelStrategy.hpp>
#include <ContactDataListener.hpp>
#include <ContactDebug.hpp>
#include <ContactFactory.hpp>
#include <ContactListener.hpp>
#include <ContactMessage.hpp>
#include <memory>

#ifndef WITH_CROSSPL

namespace elastos {
namespace sdk {

class Contact : public crosspl::native::ContactBridge {
public:
    /*** type define ***/
    class Factory final: public crosspl::native::ContactFactory {
    public:
        static std::shared_ptr<Contact> Create();
        // void SetDeviceId(const std::string& devId) {

    private:
        explicit Factory() = delete;
        virtual ~Factory() = default;
    }; // class Factory

    using Channel = crosspl::native::ChannelType;
    using Status = crosspl::native::StatusType;
    using HumanInfo = elastos::HumanInfo;
    using UserInfo = elastos::UserInfo;
    using FriendInfo = elastos::FriendInfo;

    class Message: public crosspl::native::ContactMessage {
    public:
        class MsgData {
        public:
            virtual std::string toString() = 0;
            virtual std::vector<uint8_t> toData() = 0;
            virtual void fromData(const std::vector<uint8_t>& data) = 0;
        };

        class TextData: public  MsgData {
        public:
            explicit TextData(const std::string& text);
            explicit TextData() = default;
            virtual ~TextData() = default;
            virtual std::string toString() override;
            virtual std::vector<uint8_t> toData() override;
            virtual void fromData(const std::vector<uint8_t>& data) override;

            std::string text;
        };

        class BinaryData: public MsgData {
        public:
            explicit BinaryData(const std::vector<uint8_t>& binary);
            explicit BinaryData() = default;
            virtual ~BinaryData() = default;
            virtual std::string toString() override;
            virtual std::vector<uint8_t> toData() override;
            virtual void fromData(const std::vector<uint8_t>& data) override;

            std::vector<uint8_t> binary;
        };

        class FileData: public MsgData {
        public:
//            // fix json decode and encode different issue
//            static std::string ConvertId(const std::string& id) {

            explicit FileData(const std::string& filepath);
            explicit FileData() = default;
            virtual ~FileData() = default;
            virtual std::string toString() override;
            virtual std::vector<uint8_t> toData() override;
            virtual void fromData(const std::vector<uint8_t>& data) override;

            std::string devId;
            std::string name;
            int64_t size;
            std::string md5;
        };

        explicit Message(Type type, std::shared_ptr<MsgData> data,
                         const std::string& cryptoAlgorithm, const std::string& memo);
        explicit Message(Type type, const std::vector<uint8_t>& data,
                         const std::string& cryptoAlgorithm, const std::string& memo,
                         int64_t nanoTime, int64_t replyToNanoTime);
        virtual ~Message();

        void replyTo(int64_t toNanoTime);

        const Type type;
        std::shared_ptr<MsgData> data;
        const std::string cryptoAlgorithm;
        const std::string memo;
        const int64_t nanoTime;
        int64_t replyToNanoTime;
    }; // class Message


    class ChannelStrategy: public crosspl::native::ContactChannelStrategy {
    public:
        explicit ChannelStrategy(int channelId, const std::string& name);
        virtual ~ChannelStrategy() = default;
    };

    class Listener: public crosspl::native::ContactListener {
    public:
        virtual void onReceivedMessage(const std::string& humanCode, Channel channelType,
                                       std::shared_ptr<Message> msgInfo) = 0;

    private:
        void onReceivedMessage(const std::string& humanCode, Channel channelType,
                                       std::shared_ptr<elastos::MessageManager::MessageInfo> msgInfo) override;
    }; // class Listener

    class DataListener: public crosspl::native::ContactDataListener {
    }; // class DataListener

    class Debug: public crosspl::native::ContactDebug {
    }; // class ContactDebug

    /*** static function and variable ***/
    static std::shared_ptr<Message> MakeTextMessage(const std::string& text, const std::string& cryptoAlgorithm = "", const std::string& memo = "");
    static std::shared_ptr<Message> MakeBinaryMessage(const std::vector<uint8_t>& binary, const std::string& cryptoAlgorithm = "", const std::string& memo = "");
    static std::shared_ptr<Message> MakeFileMessage(const std::string& filepath, const std::string& cryptoAlgorithm = "", const std::string& memo = "");

    /*** class function and variable ***/
    std::shared_ptr<Contact::UserInfo> getUserInfo();
    int getUserBrief(std::string& brief);
    std::vector<std::shared_ptr<Contact::FriendInfo>> listFriendInfo();

    int sendMessage(const std::string& friendCode, Channel chType, std::shared_ptr<Message> message);

private:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/
    explicit Contact() = default;
    virtual ~Contact() = default;


}; // class Contact

} // namespace sdk
} // namespace elastos

using ElaphantContact = elastos::sdk::Contact;

#endif // WITH_CROSSPL

#endif /* _ELASTOS_SDK_CONTACT_HPP_ */
