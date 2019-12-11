import CrossPL

/* @CrossClass */
@objc open class ContactDataListener: CrossBase {
  public enum Status: Int {
    case Unknown             = 0x0100
    case Initialized         = 0x0101
    case Destroyed           = 0x0102
    case Connecting          = 0x0104
    case Connected           = 0x0108
    case Transmitting        = 0x0110
    case Closed              = 0x0120
    case Failed              = 0x0140
  
    case PeerInitialized     = 0x1001
    case PeerFailed          = 0x1040
  }

  open func onNotify(humanCode: String, channelType: Contact.Channel, dataId: String,
                     status: Status) {
    fatalError("\(#function) not implementation.")
  }
  open func onReadData(humanCode: String, channelType: Contact.Channel, dataId: String,
                       offset: Int64, data: inout Data?) -> Int {
    fatalError("\(#function) not implementation.")
  }
  open func onWriteData(humanCode: String, channelType: Contact.Channel, dataId: String,
                        offset: Int64, data: Data?) -> Int {
    fatalError("\(#function) not implementation.")
  }

  public init() {
    super.init(className: String(describing: ContactDataListener.self))
  }
  
  /* @CrossPlatformInterface */
  @objc internal func onNotify(_ humanCode: String, _ channelType: Int, _ dataId: String,
                               _ status: Int) {
    var id = Contact.Message.FileData.ConvertId(id: dataId)
    if id == nil {
      id = dataId
    }
    
    onNotify(humanCode: humanCode, channelType: Contact.Channel(rawValue: channelType)!,
             dataId: id!, status: Status(rawValue: status)!)
  }
  
  /* @CrossPlatformInterface */
  @objc internal func onReadData(_ humanCode: String, _ channelType: Int, _ dataId: String,
                                 _ offset: Int64) -> Data? {
    var id = Contact.Message.FileData.ConvertId(id: dataId)
    if id == nil {
      id = dataId
    }
    
    var data: Data? = Data(count: 1024)
    let ret = onReadData(humanCode: humanCode, channelType: Contact.Channel(rawValue: channelType)!,
                         dataId: id!, offset: offset, data: &data)
    if(ret < 0) {
        return nil;
    }
    
    let retData = data?.subdata(in: 0..<ret)
    return retData
  }

  /* @CrossPlatformInterface */
  @objc internal func onWriteData(_ humanCode: String, _ channelType: Int, _ dataId: String,
                                  _ offset: Int64, _ data: Data?) -> Int {
    var id = Contact.Message.FileData.ConvertId(id: dataId)
    if id == nil {
      id = dataId
    }
    
    let ret = onWriteData(humanCode: humanCode, channelType: Contact.Channel(rawValue: channelType)!,
                          dataId: id!, offset: offset, data: data)
    return ret
  }
}
