
extension ContactInternal {

public class IdentifyCode: Codable {
  public enum Kind: Int {
       case PhoneNumber = 1
       case EmailAddress = 2
       case WechatId = 3
  }
  
  var idCodeMap: IntStrDictionary?
  public private(set) var updateTime: Int64 = 0

  public private(set) var phoneNumber: String? = nil
  public private(set) var emailAddress: String? = nil
  public private(set) var wechatId: String? = nil

  enum CodingKeys : String, CodingKey {
    case idCodeMap
    case updateTime
    
    var rawValue: String {
      get {
        switch self {
          case .idCodeMap: return JsonKey.IdCodeMap
          case .updateTime: return JsonKey.UpdateTime
        }
      }
    }
  }

  func fromJsonObj(json: IdentifyCode) -> Int {
    self.idCodeMap = nil
    self.updateTime = json.updateTime

    self.phoneNumber = json.phoneNumber
    self.emailAddress = json.emailAddress
    self.wechatId = json.wechatId

    if json.idCodeMap != nil {
      self.phoneNumber = json.idCodeMap?[Kind.PhoneNumber.rawValue]
      self.emailAddress = json.idCodeMap?[Kind.EmailAddress.rawValue]
      self.wechatId = json.idCodeMap?[Kind.WechatId.rawValue]
    }

    return 0
  }

  func toJsonObj(json: IdentifyCode) -> Int {
    json.idCodeMap = nil
    json.updateTime = self.updateTime

    json.phoneNumber = self.phoneNumber
    json.emailAddress = self.emailAddress
    json.wechatId = self.wechatId

    if self.idCodeMap != nil {
      json.phoneNumber = self.idCodeMap?[Kind.PhoneNumber.rawValue]
      json.emailAddress = self.idCodeMap?[Kind.EmailAddress.rawValue]
      json.wechatId = self.idCodeMap?[Kind.WechatId.rawValue]
    }

    return 0
  }
} // class Factory

}
