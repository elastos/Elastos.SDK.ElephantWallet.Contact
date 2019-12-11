
open class Contact: ContactInternal.ContactBridge {
  public class Factory: ContactInternal.ContactFactory {
    public static func Create() -> Contact {
      return Contact()
    }
  
    public class override func SetDeviceId(devId: String) {
      ContactInternal.ContactFactory.SetDeviceId(devId: devId)
      UserInfo.setCurrDevId(devId: devId)
    }
    
    private override init() {
      super.init()
    }
  } // class Factory

  open class Listener: ContactListener {
  } // class Listener
  
  open class DataListener: ContactDataListener {
  } // class Listener

  public typealias Channel = ContactInternal.ContactChannel
  
  public typealias Status = ContactInternal.ContactStatus
  
  public typealias HumanInfo = ContactInternal.HumanInfo
  public typealias UserInfo = ContactInternal.UserInfo
  public typealias FriendInfo = ContactInternal.FriendInfo
  
  public typealias Message = ContactInternal.ContactMessage

  public typealias Debug = ContactInternal.ContactDebug
}
