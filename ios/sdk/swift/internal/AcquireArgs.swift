
extension ContactInternal {

public class AcquireArgs {
  public enum Kind: Int {
    case PublicKey = 201
    case EncryptData = 202
    case DecryptData = 203
    case DidPropAppId = 204
    case DidAgentAuthHeader = 205
    case SignData = 206
  }
  
  public init(type: Int, pubKey: String?, data: Data?, extra: String?) {
    self.type = Kind(rawValue: type)!
    self.publicKey = pubKey
    self.data = data
    self.extra = extra
  }

  public func toString() -> String {
    return  "AcquireArgs"
          + "[type=\(type)"
          + ",publicKey=\(String(describing: publicKey))"
          + ",data=\(String(describing: data))"
          + ",extra=\(String(describing: extra))"
          + "]"
  }

  public let type: Kind
  public let publicKey: String?
  public let data: Data?
  public let extra: String?
}
  
}
