import CrossPL

extension ContactInternal {

/* @CrossClass */
open class ContactBridge: CrossBase {
  public static let TAG = "elastos"
  
  init() {
    super.init(className: String(describing: ContactBridge.self))
  }

  deinit {
    for(_, channel) in mCustomChannelMap {
      channel.unbind()
    }
    mCustomChannelMap.removeAll();
    mListener?.unbind()
    mDataListener?.unbind()
  }
  
  public func appendChannelStrategy(channelStrategy: Contact.ChannelStrategy) -> Int {
    let oldChannelStrategy = mCustomChannelMap[channelStrategy.getChannelId()]
    oldChannelStrategy?.unbind()
    
    channelStrategy.bind()
    let ret = appendChannelStrategy(channelId: channelStrategy.getChannelId(),
                                    channelStrategy: channelStrategy as CrossBase)
    if(ret < 0) {
        channelStrategy.unbind();
        return ret;
    }

    mCustomChannelMap[channelStrategy.getChannelId()] = channelStrategy
    return 0;

  }
  
  public func setListener(listener: Contact.Listener?) {
    if(mListener != nil) {
      mListener!.unbind()
    }
    mListener = listener

    mListener!.bind()
    setListener(listener: mListener as CrossBase?)
  }
  
  public func setDataListener(listener: Contact.DataListener?) {
    if(mDataListener != nil) {
      mDataListener!.unbind()
    }
    mDataListener = listener

    mDataListener!.bind()
    setDataListener(listener: mDataListener as CrossBase?)
  }

  public func setHumanInfo(humanCode: String, item: Contact.HumanInfo.Item, value: String) -> Int {
    return setHumanInfo(humanCode: humanCode, item: item.rawValue, value: value)
  }
  
  public func setUserInfo(item: Contact.UserInfo.Item, value: String) -> Int {
    let ret = setHumanInfo(humanCode: "-user-info-", item: item.rawValue, value: value);
    return ret
  }

  public func setIdentifyCode(type: Contact.UserInfo.Kind, value: String) -> Int {
    let ret = setIdentifyCode(type: type.rawValue, value: value);
    return ret;
  }

  public func getUserInfo() -> Contact.UserInfo? {
    if(mListener == nil) {
      return nil
    }
    
    var sbInfo = String()
    var ret = getHumanInfo(humanCode: "-user-info-", info: &sbInfo)
    if(ret < 0) {
      Log.w(tag: ContactBridge.TAG, msg: "Failed to get user info. ret=\(ret)")
      return nil
    }

    let userInfo = Contact.UserInfo();
    ret = userInfo.fromJson(info: sbInfo);
    if(ret < 0) {
      Log.w(tag: ContactBridge.TAG, msg: "Failed to deserialize user info. ret=\(ret)")
      return nil
    }

    return userInfo;
  }

  public func listFriendInfo() -> [Contact.FriendInfo]? {
    assert(mListener != nil)

    var json = String()
    let ret = getFriendList(info: &json);
    if(ret < 0) {
      Log.w(tag: ContactBridge.TAG, msg: "Failed to list friend info. ret=\(ret)")
      return nil
    }

//    TypeToken<List<Contact.FriendInfo.FriendJson>> friendInfoListType = new TypeToken<List<Contact.FriendInfo.FriendJson>>(){};
//    let listJson = new Gson().fromJson(json.toString(), friendInfoListType.getType());
    let listJson = try! JSONDecoder().decode([Contact.FriendInfo.FriendJson].self, from: json.data(using: .utf8)!)

    var list = [Contact.FriendInfo]()
    for it in listJson {
      let friendInfo = Contact.FriendInfo()
      let ret = friendInfo.fromJson(info: it)
      if(ret < 0) {
        Log.w(tag: ContactBridge.TAG, msg: "Failed to deserialize friend info. ret=\(ret)")
        return nil
      }

      list.append(friendInfo)
    }

    return list
  }

  public func listFriendCode() -> [String] {
    var friendCodeList = [String]()

    let friendList = listFriendInfo()
    if(friendList == nil) {
      return friendCodeList
    }

    for it in friendList! {
      friendCodeList.append(it.humanCode!)
    }

    return friendCodeList
  }

  public func getStatus(humanCode: String) -> Contact.Status? {
    let ret = getHumanStatus(humanCode: humanCode)
    if(ret < 0) {
      return nil
    }

    return Contact.Status(rawValue: ret)
  }

//  public func makeMessage(type: ContactMessage.Kind, data: Data, cryptoAlgorithm: String?) -> Contact.Message {
//    let msg = Contact.Message(type: type, data: data, cryptoAlgorithm: cryptoAlgorithm)
//    return msg
//  }

  public static func MakeTextMessage(text: String, cryptoAlgorithm: String?) -> Contact.Message {
    let msg = Contact.Message(text: text, cryptoAlgorithm: cryptoAlgorithm)
    return msg
  }

  public static func MakeFileMessage(file: URL, cryptoAlgorithm: String?) -> Contact.Message {
    let msg = Contact.Message(file: file,
                              cryptoAlgorithm: cryptoAlgorithm)
    return msg
  }

  public func sendMessage(friendCode: String, channelType: Contact.Channel, message: Contact.Message) -> Int {
    var ret = message.syncMessageToNative()
    if(ret < 0) {
      return ret;
    }

    ret = sendMessage(friendCode: friendCode, channelType: channelType.rawValue, message: message)

    return ret;
  }
  
  public func pullFileAsync(friendCode: String, channelType: Contact.Channel,
                            fileInfo: Contact.Message.FileData) -> Int {
    guard fileInfo.devId != nil else {
      return -1;
    }
    
    let ret = pullData(friendCode: friendCode, channelType: channelType.rawValue,
                       dev: fileInfo.devId!, dataId: fileInfo.toString())

    return ret;
  }

  public func cancelPullFile(friendCode: String, channelType: Contact.Channel,
                             fileInfo: Contact.Message.FileData) -> Int {
    guard fileInfo.devId != nil else {
      return -1;
    }
    
    let ret = cancelPullData(friendCode: friendCode, channelType: channelType.rawValue,
                             dev: fileInfo.devId!, dataId: fileInfo.toString())

    return ret;
  }

  /* @CrossNativeInterface */
  public static func IsDidFriend(friendCode: String) -> Bool {
    let ret = crosspl_Proxy_ContactBridge_IsDidFriend(friendCode)
    return ret
  }
  
  /* @CrossNativeInterface */
  public func start() -> Int {
    let ret = crosspl_Proxy_ContactBridge_start(nativeHandle)
    return Int(ret)
  }
  
  /* @CrossNativeInterface */
  public func stop() -> Int {
    let ret = crosspl_Proxy_ContactBridge_stop(nativeHandle)
    return Int(ret)
  }
  
  /* @CrossNativeInterface */
  public func addFriend(friendCode: String, summary: String) -> Int {
    let ret = crosspl_Proxy_ContactBridge_addFriend(nativeHandle, friendCode, summary)
    return Int(ret)
  }
  
  /* @CrossNativeInterface */
  public func removeFriend(friendCode: String) -> Int {
    let ret = crosspl_Proxy_ContactBridge_removeFriend(nativeHandle, friendCode)
    return Int(ret)
  }
  
  /* @CrossNativeInterface */
  public func acceptFriend(friendCode: String) -> Int {
    let ret = crosspl_Proxy_ContactBridge_acceptFriend(nativeHandle, friendCode)
    return Int(ret)
  }
  
  /* @CrossNativeInterface */
  public func syncInfoDownloadFromDidChain() -> Int {
    let ret = crosspl_Proxy_ContactBridge_syncInfoDownloadFromDidChain(nativeHandle)
    return Int(ret)
  }
  
  /* @CrossNativeInterface */
  public func syncInfoUploadToDidChain() -> Int {
    let ret = crosspl_Proxy_ContactBridge_syncInfoUploadToDidChain(nativeHandle)
    return Int(ret)
  }
  
  /* @CrossNativeInterface */
  public func setWalletAddress(name: String, value: String) -> Int {
    let ret = crosspl_Proxy_ContactBridge_setWalletAddress(nativeHandle, name, value)
    return Int(ret)
  }
  
  /* @CrossNativeInterface */
  private func appendChannelStrategy(channelId: Int, channelStrategy: CrossBase?) -> Int {
    let ret = crosspl_Proxy_ContactBridge_appendChannelStrategy(nativeHandle,
                                                                Int32(channelId),
                                                                channelStrategy)
    return Int(ret)
  }
  
  /* @CrossNativeInterface */
  private func setListener(listener: CrossBase?) {
    crosspl_Proxy_ContactBridge_setListener(nativeHandle, listener)
  }
  
  /* @CrossNativeInterface */
  private func setDataListener(listener: CrossBase?) {
    crosspl_Proxy_ContactBridge_setDataListener(nativeHandle, listener)
  }
  
  /* @CrossNativeInterface */
  private func setHumanInfo(humanCode: String, item: Int, value: String) -> Int {
    let ret = crosspl_Proxy_ContactBridge_setHumanInfo(nativeHandle, humanCode, Int32(item), value)
    return Int(ret)
  }
  
  /* @CrossNativeInterface */
  private func setIdentifyCode(type: Int, value: String) -> Int {
    let ret = crosspl_Proxy_ContactBridge_setIdentifyCode(nativeHandle, Int32(type), value)
    return Int(ret)
  }
  
  /* @CrossNativeInterface */
  public func getHumanInfo(humanCode: String, info: inout String) -> Int {
    var nsinfo = info as NSString?
    
    let ret = crosspl_Proxy_ContactBridge_getHumanInfo(nativeHandle, humanCode, &nsinfo)
    if ret < 0 {
      return Int(ret)
    }
    info = nsinfo! as String
    
    return 0
  }
  
  /* @CrossNativeInterface */
  public func getFriendList(info: inout String) -> Int {
    var nsinfo = info as NSString?
    let ret = crosspl_Proxy_ContactBridge_getFriendList(nativeHandle, &nsinfo)
    if ret < 0 {
      return Int(ret)
    }
    info = nsinfo! as String
    
    return 0
  }
  
  /* @CrossNativeInterface */
  public func getHumanStatus(humanCode: String) -> Int {
    let ret = crosspl_Proxy_ContactBridge_getHumanStatus(nativeHandle, humanCode)
    return Int(ret)
  }
  
  /* @CrossNativeInterface */
  private func sendMessage(friendCode: String, channelType: Int, message: CrossBase) -> Int {
    let ret = crosspl_Proxy_ContactBridge_sendMessage(nativeHandle, friendCode, Int32(channelType), message)
    return Int(ret)
  }
  
  /* @CrossNativeInterface */
  private func pullData(friendCode: String, channelType: Int, dev: String, dataId: String) -> Int {
    let ret = crosspl_Proxy_ContactBridge_pullData(nativeHandle, friendCode, Int32(channelType),
                                                   dev, dataId)
    return Int(ret)
  }
  
  /* @CrossNativeInterface */
  private func cancelPullData(friendCode: String, channelType: Int, dev: String, dataId: String) -> Int {
    let ret = crosspl_Proxy_ContactBridge_cancelPullData(nativeHandle, friendCode, Int32(channelType),
                                                         dev, dataId)
    return Int(ret)
  }
  
  private var mCustomChannelMap = [Int: Contact.ChannelStrategy]()
  private var mListener: Contact.Listener?
  private var mDataListener: Contact.DataListener?
}

}
