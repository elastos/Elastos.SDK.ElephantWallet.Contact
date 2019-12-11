
extension ContactInternal {
  
public enum ContactStatus: Int, Codable {
  case Invalid = 0x0
  case WaitForAccept = 0x1
  case Offline = 0x2
  case Online = 0x4
  case Removed = 0x8
  
  public init(from decoder: Decoder) throws {
    let container = try decoder.singleValueContainer()
    let x = try container.decode(String.self)
    switch x {
    case "Invalid":
      self = .Invalid
      break
    case "WaitForAccept":
      self = .WaitForAccept
      break
    case "Offline":
      self = .Offline
      break
    case "Online":
      self = .Online
      break
    case "Removed":
      self = .Removed
      break
    default:
      throw DecodingError.typeMismatch(ContactStatus.self,
                                       DecodingError.Context(codingPath: decoder.codingPath,
                                                             debugDescription: "Wrong type for InnerItem"))
    }
  }

  public func encode(to encoder: Encoder) throws {
    var container = encoder.singleValueContainer()
    switch self {
    case .Invalid:
      try container.encode("Invalid")
      break
      case .WaitForAccept:
        try container.encode("WaitForAccept")
        break
      case .Offline:
        try container.encode("Offline")
        break
      case .Online:
        try container.encode("Online")
        break
      case .Removed:
        try container.encode("Removed")
        break
    }
  }
}

}
