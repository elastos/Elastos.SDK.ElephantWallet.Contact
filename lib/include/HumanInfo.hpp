#ifndef _ELASTOS_HUMAN_INFO_HPP_
#define _ELASTOS_HUMAN_INFO_HPP_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <ErrCode.hpp>
#include <Json.hpp>

namespace elastos {

class HumanInfo {
public:
    /*** type define ***/
    enum class Item: int {
        ChainPubKey = 1,
        Did,
        ElaAddress,
        Nickname,
        Avatar,
        Gender,
        Description,
    };

    enum class HumanKind: int {
        Did = 1,
        Ela,
        Carrier,
    };

    enum class Status: uint8_t {
        Invalid = 0x0,
        WaitForAccept = 0x1,
        Offline = 0x2,
        Online = 0x4,
        Removed = 0x8,
    };

    struct CarrierInfo {
        struct DeviceInfo {
            std::string mDevId;
            std::string mDevName;
            int64_t mUpdateTime;
        };

        std::string mUsrAddr;
        std::string mUsrId;
        DeviceInfo mDevInfo;
    };

    /*** static function and variable ***/
    static HumanKind AnalyzeHumanKind(const std::string& code);

    /*** class function and variable ***/
    explicit HumanInfo();
    virtual ~HumanInfo();

    bool contains(const std::string& humanCode);
    bool contains(const std::shared_ptr<HumanInfo>& humanInfo);

    int getHumanCode(std::string& humanCode) const;

    virtual int addCarrierInfo(const CarrierInfo& info, const Status status = Status::Invalid);
    virtual int getCarrierInfoByUsrId(const std::string& usrId, CarrierInfo& info) const;
    virtual int getCarrierInfoByDevId(const std::string& devId, CarrierInfo& info) const;
    virtual int getAllCarrierInfo(std::vector<CarrierInfo>& infoArray) const;
    virtual int setCarrierStatus(const std::string& usrId, const Status status);
    virtual int getCarrierStatus(const std::string& usrId, Status& status) const;
    virtual int serialize(const CarrierInfo& info, std::string& value) const;
    virtual int deserialize(const std::string& value, CarrierInfo& info) const;

    virtual int setHumanInfo(Item item, const std::string& value);
    virtual int getHumanInfo(Item item, std::string& value) const;
    virtual int mergeHumanInfo(const HumanInfo& value, const Status status);

    virtual int setHumanStatus(HumanKind kind, const Status status);
    virtual int getHumanStatus(HumanKind kind, Status& status);
    virtual int setHumanStatus(const Status from, const Status to);
    virtual Status getHumanStatus() const;

    virtual int serialize(std::string& value, bool summaryOnly = false) const;
    virtual int deserialize(const std::string& value, bool summaryOnly = false);
    virtual int toJson(std::shared_ptr<Json>& value) const;
protected:
    /*** type define ***/

    /*** static function and variable ***/

    /*** class function and variable ***/
    std::vector<CarrierInfo> mBoundCarrierArray;
    std::vector<Status> mBoundCarrierStatus;
    std::map<Item, std::string> mCommonInfoMap;
    std::map<HumanKind, Status> mStatusMap;

}; // class HumanInfo

} // namespace elastos

#endif /* _ELASTOS_HUMAN_INFO_HPP_ */
