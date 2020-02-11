import CrossPL

/* @CrossClass */
@objc open class ContactChannelStrategy: CrossBase {
  public init() {
    super.init(className: String(describing: ContactChannelStrategy.self))
  }
  
  public func getChannel() -> Contact.Channel {
      return mChannel!;
  }
  
  public func getChannelId() -> Int {
      return mChannel!.rawValue;
  }

  open func onSendMessage(humanCode: String, data: Data?) -> Int {
    fatalError("\(#function) not implementation.")
  }

  public func receivedMessage(humanCode: String, data: Data?) -> Int {
    let ret = receivedMessage(humanCode, mChannel!.rawValue, data);
    return ret;
  }

  /* @CrossPlatformInterface */
  @objc open func onOpen() -> Int {
    return 0
  }
  
  /* @CrossPlatformInterface */
  @objc open func onClose() -> Int {
    return 0
  }
  
  /* @CrossPlatformInterface */
  @objc internal func onSendMessage(_ humanCode: String, _ channelType: Int, _ data: Data?) -> Int {
    let ret = onSendMessage(humanCode: humanCode, data: data)
    return ret
  }
  
  /* @CrossNativeInterface */
  private func receivedMessage(_ humanCode: String, _ channelType: Int, _ data: Data?) -> Int {
    let ret = crosspl_Proxy_ContactChannelStrategy_receivedMessage(nativeHandle,
                                                                   humanCode, Int32(channelType), data)
    return Int(ret)
  }
  
  private var mChannel: Contact.Channel? = nil;
}
