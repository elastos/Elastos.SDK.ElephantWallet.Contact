extension ContactInternal {

public class EventArgs {
  public enum Kind : Int {
    case StatusChanged = 101
    //        ReceivedMessage = 102
    //        SentMessage = 103
    case FriendRequest = 104
    //        FriendStatusChanged(105),
    case HumanInfoChanged = 106
    case MessageAck = 107
  }
  
  public init(type: Int, humanCode: String, channelType: Int, data: Data?) {
    self.type = Kind(rawValue: type)!
    self.humanCode = humanCode
    self.channelType = Contact.Channel(rawValue: channelType)!
    self.data = data
  }
  
  public func toString() -> String {
    return  "EventArgs"
          + "[type=\(type)"
          + ",humanCode=\(humanCode)"
          + ",channelType=\(channelType)"
          + ",data=\(String(describing: data))"
          + "]"
  }
  
  public let type: Kind
  public let humanCode: String
  public let channelType: Contact.Channel
  public let data: Data?
}

}
