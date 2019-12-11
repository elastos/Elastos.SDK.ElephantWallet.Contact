import CrossPL

extension ContactInternal {
  
/* @CrossClass */
public class ContactDebug: CrossBase {

  /* @CrossNativeInterface */
  public static func GetCachedDidProp(value: inout String) -> Int {
    var nsVal = value as NSString?
    
    let ret = crosspl_Proxy_ContactDebug_GetCachedDidProp(&nsVal)
    if ret < 0 {
      return Int(ret)
    }
    value = nsVal! as String
    
    return 0
  }
  
  public class Keypair: KeypairWrapper {
  } // class Listener
  
  init() {
    super.init(className: String(describing: ContactDebug.self))
  }
} // class Factory

}
