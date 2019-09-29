import CrossPL

/* @CrossClass */
@objc open class ContactListener: CrossBase {
  open func onError(errCode: Int32, errStr: String, ext: String?) {
    fatalError("\(#function) not implementation.")
  }
  open func onAcquire(request: AcquireArgs) -> Data? {
    fatalError("\(#function) not implementation.")
  }
  open func onEvent(event: EventArgs) {
    fatalError("\(#function) not implementation.")
  }
  open func onReceivedMessage(humanCode: String, channelType: Int, message: Contact.Message) {
    fatalError("\(#function) not implementation.")
  }
  
  public class EventArgs: ContactSDK.EventArgs {
  }

  public class StatusEvent: EventArgs {
    public override init(type: Int, humanCode: String, channelType: Int, data: Data?) {
      status = ContactStatus(rawValue: Int(data![0]))!
      super.init(type: type, humanCode: humanCode, channelType: channelType, data: data)
    }
   
    public override func toString() -> String {
      return  "StatusEvent"
            + "[type=\(type)"
            + ",humanCode=\(humanCode)"
            + ",channelType=\(channelType)"
            + ",status=\(status)"
            + "]"
    }

    public let status: ContactStatus
  }

  public class RequestEvent: EventArgs {
    public override init(type: Int, humanCode: String, channelType: Int, data: Data?) {
      summary = Data.ToString(from: data) ?? ""
      super.init(type: type, humanCode: humanCode, channelType: channelType, data: data)
    }
    
    public override func toString() -> String {
    return  "StatusEvent"
          + "[type=\(type)"
          + ",humanCode=\(humanCode)"
          + ",channelType=\(channelType)"
          + ",summary=\(summary)"
          + "]"
    }

    public let summary: String
  }

  public class InfoEvent: EventArgs {
    public override init(type: Int, humanCode: String, channelType: Int, data: Data?) {
      let info =  Data.ToString(from: data) ?? ""
      if info.contains(JsonKey.IsMyself) == true {
        let userInfo = UserInfo()
        _ = userInfo.fromJson(info: info)
        humanInfo = userInfo
      } else if info.contains(JsonKey.IsFriend) == true {
        let friendInfo = FriendInfo()
        _ = friendInfo.fromJson(info: info)
        humanInfo = friendInfo
      } else {
        Log.w(tag: Contact.TAG, msg: "InfoEvent: Failed to parse human data.");
      }
      
      humanInfo = HumanInfo()
      super.init(type: type, humanCode: humanCode, channelType: channelType, data: data)
    }

    public override func toString() -> String {
      return  "StatusEvent"
            + "[type=\(type)"
            + ",humanCode=\(humanCode)"
            + ",channelType=\(channelType)"
            + ",humanInfo=\(humanInfo.toString())"
            + "]"
    }
    
    public private(set) var humanInfo: HumanInfo
  }

  public init() {
    super.init(className: String(describing: ContactListener.self))
  }

  /* @CrossPlatformInterface */
  @objc internal func onAcquire(_ reqType: Int, _ pubKey: String?, _ data: Data?) -> Data? {
    Log.i(tag: Contact.TAG, msg: "ContactListener.onAcquire()")
//
    let args = AcquireArgs(type: reqType, pubKey: pubKey, data: data)
    let ret = onAcquire(request: args);

    return ret;
  }

  /* @CrossPlatformInterface */
  @objc internal func onEvent(_ eventType: Int, _ humanCode: String, _ channelType: Int, _ data: Data) {

    var args: EventArgs

    let type = EventArgs.Kind(rawValue: eventType)
    switch (type) {
    case .StatusChanged:
      args = StatusEvent(type: eventType, humanCode: humanCode, channelType: channelType, data: data)
      break;
    case .FriendRequest:
      args = RequestEvent(type: eventType, humanCode: humanCode, channelType: channelType, data: data)
      break;
    case .HumanInfoChanged:
      args = InfoEvent(type: eventType, humanCode: humanCode, channelType: channelType, data: data)
      break;
    default:
      fatalError("Unimplemented type: \(String(describing: type))");
    }

    Log.i(tag: Contact.TAG, msg: "ContactListener.onEvent() args=\(args.toString())");
    onEvent(event: args);
    return;
  }

  /* @CrossPlatformInterface */
  @objc internal func onReceivedMessage(_ humanCode: String, _ channelType: Int,
                                       _ type: Int, _ data: Data,
                                       _ cryptoAlgorithm: String, _ timestamp: Int64) {
    let message = Contact.Message(type: ContactMessage.Kind(rawValue: type)!,
                                  data: data,
                                  cryptoAlgorithm: cryptoAlgorithm)
    message.timestamp = timestamp

    onReceivedMessage(humanCode: humanCode, channelType: channelType, message: message);
    return;
  }
  
  /* @CrossPlatformInterface */
  @objc internal func onError(_ errCode: Int32, _ errStr: String, _ ext: String?) {
    onError(errCode: errCode, errStr: errStr, ext: ext)
  }
}
