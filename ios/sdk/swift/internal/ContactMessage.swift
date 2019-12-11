import CrossPL

extension ContactInternal {

/* @CrossClass */
open class ContactMessage: CrossBase {
  public enum Kind: Int {
    case Empty       = 0x00000000
    case MsgText     = 0x00000001
    case MsgAudio    = 0x00000002
    case MsgTransfer = 0x00000004
    case MsgImage    = 0x00000008
    case MsgFile     = 0x00000010
    case MsgBinary   = 0x00000020
  }
  
  public class MsgData {
    public func toString() -> String {
      let data = toData()
      return Data.ToString(from: data) ?? "nil"
    }
    public func toData() -> Data? {
      fatalError("\(#function) not implementation.")
    }
    public func fromData(data: Data?) {
      fatalError("\(#function) not implementation.")
    }
  }

  public class TextData: MsgData {
    public init(text: String?) {
      self.text = text;
      super.init()
    }
    
    public override func toData() -> Data? {
      String.ToData(from: self.text)!
    }
    
    public override func fromData(data: Data?) {
      self.text = Data.ToString(from: data)
    }
    
    public private(set) var text: String?
  }
  
  public class BinaryData: MsgData {
    public init(data: Data?) {
      self.data = data;
      super.init()
    }
    
    public override func toString() -> String {
      let encode = try! JSONEncoder().encode(self.data)
      let val = String(data: encode, encoding: .utf8)!
      return val
    }
    
    public override func toData() -> Data? {
      return self.data
    }
    
    public override func fromData(data: Data?) {
      self.data = data
    }
    
    public private(set) var data: Data?
  }

  public class FileData: MsgData, Codable {
    public init(file: URL?) {
      devId = UserInfo.GetCurrDevId()
      if(file != nil) {
        name = file?.lastPathComponent
        let attr = try? FileManager.default.attributesOfItem(atPath: file!.path)
        size = attr![FileAttributeKey.size] as? Int64
        md5 = Utils.getMD5Sum(file: file)
      }
      super.init()
    }
    
    public override func toData() -> Data? {
      let json = try? JSONEncoder().encode(self)
      return json
    }
    
    public override func fromData(data: Data?) {
      let newData = try? JSONDecoder().decode(FileData.self, from: data!)
      self.devId = newData?.devId
      self.name = newData?.name
      self.size = newData?.size
      self.md5 = newData?.md5
     }
    
    public required init(from decoder: Decoder) throws {
      let container = try decoder.container(keyedBy: CodingKeys.self)
      devId = try container.decode(String.self, forKey: .devId)
      name = try container.decode(String.self, forKey: .name)
      size = try container.decode(Int64.self, forKey: .size)
      md5 = try container.decode(String.self, forKey: .md5)
    }
    
    public func encode(to encoder: Encoder) throws {
      var container = encoder.container(keyedBy: CodingKeys.self)
      try container.encode(devId, forKey: .devId)
      try container.encode(name, forKey: .name)
      try container.encode(size, forKey: .size)
      try container.encode(md5, forKey: .md5)
    }
    
    // fix json decode and encode different issue
    public static func ConvertId(id: String) -> String? {
      guard let dataId = id.data(using: .utf8) else {
        print("FileData.ConvertId() 0 Failed to convert \(id)")
        return nil
      }
      
      guard let fileData = try? JSONDecoder().decode(FileData.self, from: dataId) else {
        print("FileData.ConvertId() 1 Failed to convert \(id)")
        return nil
      }
      
      return Data.ToString(from: fileData.toData())
    }
    
    public private(set) var devId: String?
    public private(set) var name: String?
    public private(set) var size: Int64?
    public private(set) var md5: String?
    
    enum CodingKeys : String, CodingKey {
      case devId
      case name
      case size
      case md5
      var rawValue: String {
        get {
          switch self {
            case .devId: return JsonKey.DeviceId
            case .name: return JsonKey.Name
            case .size: return JsonKey.Size
            case .md5: return JsonKey.Md5
          }
        }
      }
    }
  }

  
  public let type: Kind
  public private(set) var data: MsgData
  public let cryptoAlgorithm: String?
  public var timestamp: Int64
  
  public func syncMessageToNative() -> Int {
    let ret = syncMessageToNative(type: type.rawValue,
                                  data: data.toData()!,
                                  cryptoAlgorithm: cryptoAlgorithm,
                                  timestamp: timestamp)
    return ret;
  }


  public init(type: Kind, data: MsgData, cryptoAlgorithm: String?) {
    self.type = type
    self.data = data
    self.cryptoAlgorithm = cryptoAlgorithm ?? ""
    self.timestamp = Int64(Date().timeIntervalSince1970 * 1000)
    
    super.init(className: String(describing: ContactMessage.self))
  }

  public convenience init(text: String, cryptoAlgorithm: String?) {
    self.init(type: Kind.MsgText,
              data: TextData(text: text),
              cryptoAlgorithm: cryptoAlgorithm);
  }

  public convenience init(file: URL, cryptoAlgorithm: String?) {
    self.init(type: Kind.MsgFile,
              data: FileData(file: file),
              cryptoAlgorithm: cryptoAlgorithm);
  }
  
  public convenience init(type: Kind, data: Data, cryptoAlgorithm: String?) {
    self.init(type: type, data: MsgData(), cryptoAlgorithm: cryptoAlgorithm)
    switch (type) {
    case .MsgText:
      self.data = TextData(text: nil)
      break;
    case .MsgBinary:
      self.data = BinaryData(data: nil)
      break;
    case .MsgFile:
      self.data = FileData(file: nil)
      break;
    default:
      print("Unknown message type \(type)")
      return;
    }
    self.data.fromData(data: data)
  }
  
  /* @CrossNativeInterface */
  private func syncMessageToNative(type: Int,
                                   data: Data, cryptoAlgorithm: String?,
                                   timestamp: Int64) -> Int {
    let ret = crosspl_Proxy_ContactMessage_syncMessageToNative(nativeHandle, Int32(type), data, cryptoAlgorithm, timestamp)
    return Int(ret)
  }
}

}
