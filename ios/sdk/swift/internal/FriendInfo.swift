
extension ContactInternal {
  
public class FriendInfo: HumanInfo {
  public class FriendJson: HumanInfo.HumanJson {
    var alias: String? = nil
    
    enum CodingKeys : String, CodingKey {
      case alias
      
      var rawValue: String {
        get {
          switch self {
            case .alias: return JsonKey.Alias
          }
        }
      }
    }
  }

  public var alias: String?

  public func fromJson(info: HumanJson) -> Int {
    if info is FriendJson == false {
      return -1
    }

    let ret = super.fromJsonObj(info: info)
    if ret < 0 {
        return ret
    }

    self.alias = (info as! FriendJson).alias

    return 0
  }

  public func fromJson(info: String) -> Int {
    let decode = try! JSONDecoder().decode(FriendJson.self, from: info.data(using: .utf8)!)
        
    let ret = fromJson(info: decode)
    if ret < 0 {
      return ret
    }

    return 0
  }

  public func toJson() -> String? {
    let json = FriendJson()

    let ret = super.toJsonObj(json: json)
    if(ret < 0) {
      return nil
    }

    json.alias = self.alias

    let encoder = JSONEncoder()
    encoder.outputFormatting = .prettyPrinted
    let encode = try! encoder.encode(json)
    let val = String(data: encode, encoding: .utf8)!
    return val
  }
} // class FriendInfo

}
