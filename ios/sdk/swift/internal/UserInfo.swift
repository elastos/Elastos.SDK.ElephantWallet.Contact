
extension ContactInternal {

public class UserInfo: HumanInfo {
  public typealias Kind = IdentifyCode.Kind
  
  public class UserJson: HumanInfo.HumanJson {
    var identifyCode: IdentifyCode? = nil
    
    enum CodingKeys : String, CodingKey {
      case identifyCode
      
      var rawValue: String {
        get {
          switch self {
            case .identifyCode: return JsonKey.IdentifyCode
          }
        }
      }
    }
  }

  public class func setCurrDevId(devId: String) {
    mCurrDevId = devId
  }

  public class func GetCurrDevId() -> String? {
    return mCurrDevId
  }
  
  public func getCurrDevCarrierAddr() -> String? {
    if UserInfo.mCurrDevId == nil
    || boundCarrierArray == nil {
      return nil
    }
    
    for info in boundCarrierArray! {
      if info.devInfo.devId == UserInfo.mCurrDevId {
        return info.usrAddr
      }
    }
    return nil
  }

  public func getCurrDevCarrierId() -> String? {
    if UserInfo.mCurrDevId == nil
    || boundCarrierArray == nil {
      return nil
    }

    for info in boundCarrierArray! {
      if info.devInfo.devId == UserInfo.mCurrDevId {
        return info.usrId
      }
    }
    return nil
  }

  public func fromJson(info: String) -> Int {
    let decode = try! JSONDecoder().decode(UserJson.self, from: info.data(using: .utf8)!)
  
    var ret = super.fromJsonObj(info: decode)
    if ret < 0 {
        return ret
    }

    if decode.identifyCode != nil {
      ret = identifyCode.fromJsonObj(json: decode.identifyCode!)
      if ret < 0 {
        return ret
      }
    }

    return 0
  }

  public func toJson() -> String? {
    let json = UserJson()
    json.identifyCode = IdentifyCode()

    var ret = super.toJsonObj(json: json)
    if ret < 0  {
      return nil
    }

    if json.identifyCode != nil {
      ret = self.identifyCode.toJsonObj(json: json.identifyCode!)
      if ret < 0 {
        return nil
      }
    }
    
    let encoder = JSONEncoder()
    encoder.outputFormatting = .prettyPrinted
    let encode = try! encoder.encode(json)
    let val = String(data: encode, encoding: .utf8)!
    
    return val
  }

  public let identifyCode = IdentifyCode()

  private static var mCurrDevId: String?
} // class UserInfo

}
