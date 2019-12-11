extension ContactInternal {

public class HumanInfo: Codable {
  public enum Item: Int {
    case ChainPubKey = 1
    case Did = 2
    case ElaAddress = 3
    case Nickname = 4
    case Avatar = 5
    case Gender = 6
    case Description = 7
  }
  
  public class HumanJson: Codable {
    public var humanInfo: HumanInfo? = nil
    
    enum CodingKeys : String, CodingKey {
      case humanInfo
      
      var rawValue: String {
        get {
          switch self {
            case .humanInfo: return JsonKey.HumanInfo
          }
        }
      }
    }
  }
  
  public class CarrierInfo: Codable {
    public class DeviceInfo: Codable {
      public let devId: String
      public let devName: String
      
      enum CodingKeys : String, CodingKey {
        case devId
        case devName
        
        var rawValue: String {
          get {
            switch self {
              case .devId: return JsonKey.DeviceId
              case .devName: return JsonKey.DeviceName
            }
          }
        }
      }
    }
    
    public let usrAddr: String
    public let usrId: String
    public let devInfo: DeviceInfo
    public let updateTime: Int64
    
    enum CodingKeys : String, CodingKey {
      case usrAddr
      case usrId
      case devInfo
      case updateTime
      
      var rawValue: String {
        get {
          switch self {
            case .usrAddr: return JsonKey.CarrierAddr
            case .usrId: return JsonKey.CarrierId
            case .devInfo: return JsonKey.DeviceInfo
            case .updateTime: return JsonKey.UpdateTime
          }
        }
      }
    }
  }
  
  private(set) var boundCarrierArray: [CarrierInfo]? = nil
  public var commonInfoMap: IntStrDictionary? = nil
  private var walletAddressMap: [String: String]? = nil
  public private(set) var  status: Contact.Status = .Invalid
  public private(set) var humanCode: String? = nil
  public private(set) var updateTime: Int64 = 0

  enum CodingKeys : String, CodingKey {
    case boundCarrierArray
    case commonInfoMap
    case walletAddressMap
    case status
    case humanCode
    case updateTime

    case chainPubKey
    case did
    case elaAddress
    case nickname
    case avatar
    case gender
    case description
    
    var rawValue: String {
      get {
        switch self {
          case .boundCarrierArray: return JsonKey.BoundCarrierArray
          case .commonInfoMap:     return JsonKey.CommonInfoMap
          case .walletAddressMap:  return JsonKey.WalletAddressMap
          case .status:            return JsonKey.Status
          case .humanCode:         return JsonKey.HumanCode
          case .updateTime:        return JsonKey.UpdateTime
          case .chainPubKey:       return "chainPubKey"
          case .did:               return "did"
          case .elaAddress:        return "elaAddress"
          case .nickname:          return "nickname"
          case .avatar:            return "avatar"
          case .gender:            return "gender"
          case .description:       return "description"
        }
      }
    }
  }
  
  public private(set) var chainPubKey: String? = nil
  public private(set) var did: String? = nil
  public private(set) var elaAddress: String? = nil
  public private(set) var nickname: String? = nil
  public private(set) var avatar: String? = nil
  public private(set) var gender: String? = nil
  public private(set) var description: String? = nil
  
  func fromJsonObj(info: HumanJson) -> Int {
    self.commonInfoMap = nil
    self.walletAddressMap = info.humanInfo?.walletAddressMap
    self.boundCarrierArray = info.humanInfo?.boundCarrierArray
    self.status = info.humanInfo?.status ?? .Invalid
    self.humanCode = info.humanInfo?.humanCode
    self.updateTime = info.humanInfo?.updateTime ?? 0

    self.chainPubKey = info.humanInfo?.chainPubKey
    self.did = info.humanInfo?.did
    self.elaAddress = info.humanInfo?.elaAddress
    self.nickname = info.humanInfo?.nickname
    self.avatar = info.humanInfo?.avatar
    self.gender = info.humanInfo?.gender
    self.description = info.humanInfo?.description

    if info.humanInfo?.commonInfoMap != nil {
      self.chainPubKey = info.humanInfo?.commonInfoMap?[Item.ChainPubKey.rawValue]
      self.did = info.humanInfo?.commonInfoMap?[Item.Did.rawValue]
      self.elaAddress = info.humanInfo?.commonInfoMap?[Item.ElaAddress.rawValue]
      self.nickname = info.humanInfo?.commonInfoMap?[Item.Nickname.rawValue]
      self.avatar = info.humanInfo?.commonInfoMap?[Item.Avatar.rawValue]
      self.gender = info.humanInfo?.commonInfoMap?[Item.Gender.rawValue]
      self.description = info.humanInfo?.commonInfoMap?[Item.Description.rawValue]
    }

    return 0
  }

  func toJsonObj(json: HumanJson) -> Int {
    json.humanInfo = HumanInfo()

    json.humanInfo?.commonInfoMap = nil
    json.humanInfo?.walletAddressMap = self.walletAddressMap
    json.humanInfo?.boundCarrierArray = self.boundCarrierArray
    json.humanInfo?.status = self.status
    json.humanInfo?.humanCode = self.humanCode
    json.humanInfo?.updateTime = self.updateTime

    json.humanInfo?.chainPubKey = self.chainPubKey
    json.humanInfo?.did = self.did
    json.humanInfo?.elaAddress = self.elaAddress
    json.humanInfo?.nickname = self.nickname
    json.humanInfo?.avatar = self.avatar
    json.humanInfo?.gender = self.gender
    json.humanInfo?.description = self.description

    if self.commonInfoMap != nil {
      json.humanInfo?.chainPubKey = self.commonInfoMap?[Item.ChainPubKey.rawValue]
      json.humanInfo?.did = self.commonInfoMap?[Item.Did.rawValue]
      json.humanInfo?.elaAddress = self.commonInfoMap?[Item.ElaAddress.rawValue]
      json.humanInfo?.nickname = self.commonInfoMap?[Item.Nickname.rawValue]
      json.humanInfo?.avatar = self.commonInfoMap?[Item.Avatar.rawValue]
      json.humanInfo?.gender = self.commonInfoMap?[Item.Gender.rawValue]
      json.humanInfo?.description = self.commonInfoMap?[Item.Description.rawValue]
    }

    return 0
  }
  
  init() {
  }
} // class HumanInfo

}
