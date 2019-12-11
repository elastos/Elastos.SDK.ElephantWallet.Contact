import CrossPL

extension ContactInternal {

/* @CrossClass */
public class KeypairWrapper: CrossBase {

  /* @CrossNativeInterface */
  public static func GetSinglePublicKey(seed: Data, pubKey: inout String) -> Int {
    var nsPubKey = pubKey as NSString?
    let ret = crosspl_Proxy_KeypairWrapper_GetSinglePublicKey(seed, &nsPubKey)
    if ret < 0 {
      return Int(ret)
    }
    pubKey = nsPubKey! as String
    
    return 0
  }
  
  /* @CrossNativeInterface */
  public static func GetSinglePrivateKey(seed: Data, privKey: inout String) -> Int {
    var nsPrivKey = privKey as NSString?
    let ret = crosspl_Proxy_KeypairWrapper_GetSinglePrivateKey(seed, &nsPrivKey)
    if ret < 0 {
      return Int(ret)
    }
    privKey = nsPrivKey! as String
    
    return 0
  }
  
  /* @CrossNativeInterface */
  public static func GenerateMnemonic(language: String, words: String?, mnem: inout String) -> Int {
    var nsMnem = mnem as NSString?
    let ret = crosspl_Proxy_KeypairWrapper_GenerateMnemonic(language, words, &nsMnem)
    if ret < 0 {
      return Int(ret)
    }
    mnem = nsMnem! as String
    
    return 0  }
  
  /* @CrossNativeInterface */
  public static func GetSeedFromMnemonic(mnemonic: String,
                                         mnemonicPassword: String,
                                         seed: inout Data) -> Int {
    var nsseed = seed as NSData?
    let ret = crosspl_Proxy_KeypairWrapper_GetSeedFromMnemonic(mnemonic, mnemonicPassword, &nsseed)
    if ret < 0 {
      return Int(ret)
    }
    seed = nsseed! as Data
    
    return Int(ret)
  }
  
  /* @CrossNativeInterface */
  public static func Sign(privateKey: String?, data: Data, signedData: inout Data) -> Int {
    var nsSignedData = signedData as NSData?
    let ret = crosspl_Proxy_KeypairWrapper_Sign(privateKey, data, &nsSignedData)
    if ret < 0 {
      return Int(ret)
    }
    signedData = nsSignedData! as Data
    
    return Int(ret)
  }
  
  init() {
    super.init(className: String(describing: ContactDebug.self))
  }
} // class Factory

}
