/**
 * @file	Contact.hpp
 * @brief	Contact
 * @details	
 *
 * @author	xxx
 * @author	<xxx@xxx.com>
 * @copyright	(c) 2012 xxx All rights reserved.
 **/

#ifndef _CONTACT_TEST_HPP_
#define _CONTACT_TEST_HPP_

#include <Contact.hpp>

class ContactTest {
public:
    /*** type define ***/

    /*** static function and variable ***/
    static std::shared_ptr<ContactTest> GetInstance();

    /*** class function and variable ***/
    int init();
    int newAndSaveMnemonic(const std::string& newMnemonic = "");
    int testNewContact();
    int testStartContact();
    int testStopContact();
    int testDelContact();

    int showGetUserInfo();

   private:
    /*** type define ***/

    /*** static function and variable ***/
    static constexpr const char* KeypairLanguage = "english";
    static constexpr const char* KeypairWords = "";
    static constexpr const char* MnemonicFileName = "mnemonic";
    static const std::string gCacheDir;
    static std::shared_ptr<ContactTest> gContactTest;
    static std::string GetMd5Sum(const std::string& data);
    static void ShowEvent(const std::string& msg);
    static void ShowError(const std::string& msg);

    /*** class function and variable ***/
    explicit ContactTest() = default;
    virtual ~ContactTest() = default;
    std::string getPublicKey();
    std::string getPrivateKey();
    std::string getAgentAuthHeader();
    std::string getMd5Sum(const std::string& data);
    std::shared_ptr<std::vector<uint8_t>> processAcquire(ElaphantContact::Listener::AcquireType type,
                                                         const std::string& pubKey,
                                                         const std::vector<uint8_t>& data);
    void processEvent(ElaphantContact::Listener::EventType event, const std::string& humanCode,
                      ElaphantContact::Listener::ContactChannel channelType, const std::vector<uint8_t>& data);

    std::string mSavedMnemonic;
    std::shared_ptr<ElaphantContact> mContact;
    std::shared_ptr<ElaphantContact::Listener> mContactListener;
    std::shared_ptr<ElaphantContact::DataListener> mContactDataListener;
}; // class Contact

#ifdef WITH_CROSSPL
} //namespace native
} //namespace crosspl
#endif // WITH_CROSSPL

#endif /* _CONTACT_TEST_HPP_ */
