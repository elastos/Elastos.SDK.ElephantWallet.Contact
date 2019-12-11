//
//  ContactFactory.swift
//  sdk
//
//  Created by mengxk on 2019/9/19.
//  Copyright © 2019 Elastos. All rights reserved.
//

import CrossPL

extension ContactInternal {

/* @CrossClass */
public class ContactFactory: CrossBase {
  /* @CrossNativeInterface */
  public static func SetLogLevel(level: Int32) {
    crosspl_Proxy_ContactFactory_SetLogLevel(level)
  }
  
  /* @CrossNativeInterface */
  public class func SetDeviceId(devId: String) {
    crosspl_Proxy_ContactFactory_SetDeviceId(devId)
  }
  
  /* @CrossNativeInterface */
  public static func SetLocalDataDir(dir: String) -> Int32 {
    return crosspl_Proxy_ContactFactory_SetLocalDataDir(dir)
  }
  
  init() {
    super.init(className: String(describing: ContactFactory.self))
  }
}
}
