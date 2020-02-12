//
//  ViewController.swift
//  test
//
//  Created by mengxk on 2019/9/20.
//  Copyright © 2019 Elastos. All rights reserved.
//

import UIKit
import ContactSDK

class ViewController: UIViewController {

  override func viewDidLoad() {
    super.viewDidLoad()
    // Do any additional setup after loading the view.
    
    let devId = getDeviceId()
    Log.i(tag: ViewController.TAG, msg: "Device ID:" + devId)
    
    mSavedMnemonic = UserDefaults.standard.string(forKey: ViewController.SavedMnemonicKey)
    if mSavedMnemonic == nil {
      var mnem = String()
      let ret = Contact.Debug.Keypair.GenerateMnemonic(language: ViewController.KeypairLanguage,
                                                       words: ViewController.KeypairWords,
                                                       mnem:&mnem)
      if(ret < 0) {
        showMessage(ViewController.ErrorPrefix + "Failed to call Contact.Debug.Keypair.GenerateMnemonic()")
        return
      }
      _ = newOrLoadMnemonic(mnem)
    }

    showMessage("Mnemonic:\(mSavedMnemonic ?? "nil")\n")
  }

  @IBAction func onOptionsMenuTapped(_ sender: UIView) {
//    optionsMenu.isHidden = !optionsMenu.isHidden
    if(mPopMenu == nil) {
      let screenSize = UIScreen.main.bounds.size
      let menuRect = CGRect(x:screenSize.width - 10 - ViewController.MENU_WIDTH, y:100,
                            width:ViewController.MENU_WIDTH, height:screenSize.height - 100)
      mPopMenu = SwiftPopMenu(frame: menuRect)
      var popData = [(icon:String,title:String)]()
      for (key,_) in ViewController.MENU_ITEM {
        popData.append((icon: "", title: key))
      }
      mPopMenu!.popData = popData
      mPopMenu!.didSelectMenuBlock = self.onOptionsItemSelected
    }
    mPopMenu!.show()
  }
  
  private func onOptionsItemSelected(_ index: Int) {
    mPopMenu!.dismiss()
    let (_,value) = Array(ViewController.MENU_ITEM)[index]
    guard value != nil else {
      self.showToast("Unimplement!")
      return
    }
    let callback = value!(self)
    let message = callback()
    guard ((message as? String) != nil) else {
      return
    }
    
    showMessage(message as! String)
  }

  private func getStarted() {
    var help = ""
    help += "Step1: [New Contact]\n"
    help += "Step2: [Start Contact]\n"
    help += "Step3: [User Info] can show you info\n"
    help += "Step4: After online, [Add friend] can add friend\n"
    help += "Step5: After friend online, [Send Message] can send message\n"
    
    clearEvent()
    showEvent(help)
  }
  
  private func clearEvent() {
    DispatchQueue.main.async { [weak self] in
      self?.mEventLog.text = ""
    }
  }

  private func newAndSaveMnemonic() -> String {
    return newOrLoadMnemonic(nil) ?? ""
  }
  
  private func newAndStartContact() -> String {
    var message = testNewContact()
    message += testStartContact()
    return message
  }
  
  private func stopAndDelContact() -> String {
    var message = testStopContact()
    message += testDelContact()
    return message
  }
  
  private func recreateContact() -> String {
    var message = testStopContact()
    message += testDelContact()
    message += testNewContact()
    message += testStartContact()
    return message
  }
  
  private func restartContact() -> String {
    var message = testStopContact()
    message += testStartContact()
    return message
  }
  
  private func newOrLoadMnemonic(_ newMnemonic: String?) -> String? {
    mSavedMnemonic = newMnemonic
    if mSavedMnemonic == nil {
      var mnem = String()
      let ret = Contact.Debug.Keypair.GenerateMnemonic(language: ViewController.KeypairLanguage,
                                                       words: ViewController.KeypairWords,
                                                       mnem:&mnem)
      if(ret < 0) {
        showMessage(ViewController.ErrorPrefix + "Failed to call Contact.Debug.Keypair.GenerateMnemonic()")
        return nil
      }
      mSavedMnemonic = mnem
    }
  
    UserDefaults.standard.set(mSavedMnemonic, forKey: ViewController.SavedMnemonicKey)
    
    if mContact == nil { // noneed to restart
      return "Success to save mnemonic: \(String(describing: mSavedMnemonic))"
    }

    let dialog = UIAlertController(title: nil, message: nil, preferredStyle: .alert)
    dialog.message = "New mnemonic can only be valid after restart,\ndo you want restart app?"
    dialog.addAction(UIAlertAction(title: "OK", style: .default, handler: { _ in
      exit(0)
    }))
    dialog.addAction(UIAlertAction(title: "Cancel", style: .cancel, handler: nil))
    
    self.present(dialog, animated: false, completion: nil)

    return ("Cancel to save mnemonic: \(newMnemonic ?? "nil")\n")
  }
  
  private func importMnemonic() -> String {
    Helper.showImportMnemonic(view: self, listener: { result in
        if self.isEnglishWords(result) == false {
          self.showMessage(ViewController.ErrorPrefix + "Only english mnemonic is supported.")
          return
        }
        let privKey = self.getPrivateKey()
        if privKey.isEmpty {
          self.showMessage(ViewController.ErrorPrefix + "Bad mnemonic.")
          return
        }

        let message = self.newOrLoadMnemonic(result)
        self.showMessage(message!)
    })

    return "Success to show import mnemonic dialog."
  }

  
  private func testNewContact() -> String {
    Contact.Factory.SetLogLevel(level: 7)

    Contact.Factory.SetDeviceId(devId: getDeviceId())

    mCacheDir = FileManager.default.urls(for: .cachesDirectory, in: .userDomainMask).first
    let ret = Contact.Factory.SetLocalDataDir(dir: mCacheDir!.path)
    if ret < 0 {
      return "Failed to call Contact.Factory.SetLocalDataDir() ret=\(ret)"
    }

    mContact = Contact.Factory.Create()
    if mContact == nil {
      return "Failed to call Contact.Factory.Create()"
    }

    mCustomChannelStrategy = {
      class Impl: Contact.ChannelStrategy {
        init() {
          super.init(channelId: Contact.Channel.CustomId.rawValue, name: "LoopMessage")
        }
        
        override func onSendMessage(humanCode: String, data: Data?) -> Int {
          self.receivedMessage(humanCode: humanCode, data: data)
        }
      }
      return Impl()
    }()
    let cret = mContact!.appendChannelStrategy(channelStrategy: mCustomChannelStrategy!)
    if cret < 0 {
      return "Failed to call Contact.appendChannelStrategy() ret=\(ret)"
    }
    
    mContactListener = {
      class Impl: Contact.Listener {
        init(_ vc: ViewController) {
          viewCtrl = vc
          super.init()
        }
        
        override func onAcquire(request: AcquireArgs) -> Data? {
          let ret = viewCtrl.processAcquire(request: request)
          
          var msg = "onAcquire(): req=\(request.toString())\n"
          msg += "onAcquire(): resp=\(String(describing: ret))\n"
          viewCtrl.showEvent(msg)
          
          return ret
        }
        
        override func onEvent(event: EventArgs) {
          viewCtrl.processEvent(event: event)
          
          let msg = "onEvent(): ev=\(event.toString())\n"
          viewCtrl.showEvent(msg)
        }
        
        override func onReceivedMessage(humanCode: String, channelType: Contact.Channel, message: Contact.Message) {
          var msg = "onRcvdMsg(): data=\(message.data.toString())\n"
          msg += "onRcvdMsg(): type=\(message.type)\n"
          msg += "onRcvdMsg(): crypto=" + (message.cryptoAlgorithm ?? "nil") + "\n"
          msg += "onRcvdMsg(): nanoTime=\(message.nanoTime / Contact.Message.TimeOffset)\n"
          msg += "onRcvdMsg(): replyTo=\(message.replyToNanoTime)\n"
          viewCtrl.showEvent(msg)
          
          if(message.type == Contact.Message.Kind.MsgFile) {
            viewCtrl.mContactRecvFileMap[humanCode] = message.data as! Contact.Message.FileData
          }
        }
        
        override func onError(errCode: Int32, errStr: String, ext: String?) {
          var msg = "\(errCode): \(errStr)"
          msg += "\n\(String(describing: ext))"
          viewCtrl.showError(msg)
        }
        
        private let viewCtrl: ViewController
      }
      
      return Impl(self)
    }()
    mContact!.setListener(listener: mContactListener)
  
    mContactDataListener = {
      class Impl: Contact.DataListener {
        init(_ vc: ViewController) {
          viewCtrl = vc
          super.init()
        }
      
        override func onNotify(humanCode: String, channelType: Contact.Channel, dataId: String,
                               status: Status) {
          let msg = "onNotify(): dataId=\(dataId), status=\(status)\n";
          viewCtrl.showEvent(msg);

        }
        
        override func onReadData(humanCode: String, channelType: Contact.Channel, dataId: String,
                                 offset: Int64, data: inout Data?) -> Int {

          guard let filepath = viewCtrl.mContactSendFileMap[dataId] else {
            let msg = "onReadData(): dataId=\(dataId) File not cached.\n"
            viewCtrl.showEvent(ViewController.ErrorPrefix + msg)
            return -1
          }
          
          guard FileManager.default.fileExists(atPath: filepath) else {
            let msg = "onReadData(): dataId=\(dataId) File not exists.\n"
            viewCtrl.showEvent(ViewController.ErrorPrefix + msg)
            return -1
          }
          
          var fileAttr = try! FileManager.default.attributesOfItem(atPath: filepath)
          let fileSize = Int64(fileAttr[FileAttributeKey.size] as! UInt64)

          let msg = "onReadData(): dataId=\(dataId), offset=\(offset)\n";
          if(offset == 0 || offset >= fileSize) {
            viewCtrl.showEvent(msg);
          } else {
            print("\(msg)");
          }
          
          if(offset >= fileSize) { // finished
            return 0
          }
          
          guard let fileHandle = FileHandle(forReadingAtPath: filepath) else {
            let msg = "onReadData(): dataId=\(dataId) Failed to open file.\n"
            viewCtrl.showEvent(ViewController.ErrorPrefix + msg)
            return -1
          }
           
          var size = data!.count
          if(offset + Int64(size) > fileSize) {
            size = Int(fileSize - offset)
          }

          try? fileHandle.seek(toOffset: UInt64(offset))
          data = fileHandle.readData(ofLength: size)
          fileHandle.closeFile()
         
          return size
        }
        
        override func onWriteData(humanCode: String, channelType: Contact.Channel, dataId: String,
                                  offset: Int64, data: Data?) -> Int {
          var msg = "onWriteData(): dataId=\(dataId), offset=\(offset)\n";
          if(offset == 0 || data!.count == 0) {
            viewCtrl.showEvent(msg);
          } else {
            print("\(msg)");
          }
          
          guard let fileInfo = viewCtrl.mContactRecvFileMap[humanCode] else {
            let msg = "onWriteData(): dataId=\(dataId) File not cached.\n"
            viewCtrl.showEvent(ViewController.ErrorPrefix + msg)
            return -1
          }
          
          let filepath = viewCtrl.mCacheDir!.appendingPathComponent(fileInfo.name!,
                                                                    isDirectory: false)
          if (FileManager.default.fileExists(atPath: filepath.path) == false) {
            viewCtrl.showEvent("Write data to \(filepath.path)")
            FileManager.default.createFile(atPath: filepath.path, contents: nil, attributes: nil)
          }
          guard let fileHandle = FileHandle(forWritingAtPath: filepath.path) else {
            let msg = "onWriteData(): dataId=\(dataId) Failed to open file.\n"
            viewCtrl.showEvent(ViewController.ErrorPrefix + msg)
            return -1
          }
          if(data!.count == 0) {
              let md5 = Utils.getMD5Sum(file: filepath)
              if(md5 == fileInfo.md5!)  {
                  msg = "onWriteData(): Success to pull dataId=" + dataId + "\n";
              } else {
                  msg = "onWriteData(): Failed to pull dataId=" + dataId + "\n";
              }
              viewCtrl.showEvent(msg);
              return 0;
          }
          
          fileHandle.seekToEndOfFile()
          fileHandle.write(data!)
          fileHandle.closeFile()
          
          return data!.count
        }
        
        private let viewCtrl: ViewController
      }
        
      return Impl(self)
    }()
    mContact!.setDataListener(listener: mContactDataListener)
    
    return "Success to create a contact instance."
  }

  private func testStartContact() -> String {
    if mContact == nil {
      return ViewController.ErrorPrefix + "Contact is null."
    }
  
    let ret = mContact!.start()
    if ret < 0 {
      return "Failed to start contact instance. ret=\(ret)"
    }

//    var recvKey: String
//    var recvDataId: String
//    var sendDataId: String
//    var sendFilePath: String
//    if (UserInfo.GetCurrDevId() == "AC200C58-6375-48E8-85D7-E27255D94C41") {
//      recvKey = "igh7qBS5BYLLG9PqfF6gY1ytjnwvAKRcEx";
//      recvDataId = "{\"DevId\":\"9fdb3e667aec0e60\",\"Md5\":\"de57b4c20b3d7cffed47ba42d1f0f0ad\",\"Name\":\"P91025-131156.jpg\",\"Size\":3217685}";
//     sendDataId = "{\"DevId\":\"AC200C58-6375-48E8-85D7-E27255D94C41\",\"Md5\":\"6345517ead2f2776f680ec598e4df559\",\"Name\":\"3CDA6326-1CDB-47E7-A055-763589AFBFEA.jpeg\",\"Size\":1896240}";
//      sendFilePath = "/Users/mengxk/Library/Developer/CoreSimulator/Devices/5F2D3CE5-702A-4D73-B1F3-D7648E13768B/data/Containers/Data/Application/9EC7A10E-234E-4E24-A8D7-724DDEBC31AC/tmp/0E89B578-C2A9-4FED-8915-F891913ABDDA.jpeg";
//    } else {
//      recvKey = ""
//      recvDataId = ""
//      sendDataId = ""
//      sendFilePath = ""
//    }

//    let fileMsg = Contact.Message(type: Contact.Message.Kind.MsgFile,
//                                  data: recvDataId.data(using: String.Encoding.utf8),
//                                  cryptoAlgorithm: nil)
//    mContactRecvFileMap[recvKey] = fileMsg.data as? Contact.Message.FileData
//    mContactSendFileMap[sendDataId] = sendFilePath
    
    return "Success to start contact instance."
  }
  
  private func testStopContact() -> String {
    if mContact == nil {
      return ViewController.ErrorPrefix + "Contact is null."
    }

    let ret = mContact!.stop()
    if ret < 0 {
      return "Failed to stop contact instance. ret=\(ret)"
    }

    return "Success to stop contact instance."
  }

  private func testDelContact() -> String {
      if mContact == nil {
        return ViewController.ErrorPrefix + "Contact is null."
      }

      mContact = nil
      return "Success to delete a contact instance."
  }

  private func showGetUserInfo() -> String {
    if mContact == nil {
      return ViewController.ErrorPrefix + "Contact is null."
    }

    let info = mContact!.getUserInfo()
    if info == nil {
      return ViewController.ErrorPrefix + "Failed to get user info."
    }

    let humanCode: Helper.OrderedDictionary = [
        ("Did", info!.did),
        ("Ela", info!.elaAddress),
        ("Carrier", info!.getCurrDevCarrierAddr())
    ]
    let ext = info!.getCurrDevCarrierId()
    Helper.showAddress(view: self,
                       listener: { _ in
                         Helper.showDetails(view: self, msg: info!.toJson()!)
                       },
                       humanCode: humanCode, presentDevId: getDeviceId(), ext: ext)

    return info!.toString()
  }
  
  private func showSetUserIdentifyCode() -> String {
    if mContact == nil {
      return ViewController.ErrorPrefix + "Contact is null."
    }

    let checkList = [
        "\(Contact.UserInfo.Kind.PhoneNumber)",
        "\(Contact.UserInfo.Kind.EmailAddress)",
        "\(Contact.UserInfo.Kind.WechatId)"
    ]
    let separator = ":-:-:"

    Helper.showSetDetails(view: self,
                          checkList: checkList, separator: separator,
                          listener: { result in
        let details = [
          "\(Contact.UserInfo.Kind.PhoneNumber)": Contact.UserInfo.Kind.PhoneNumber,
          "\(Contact.UserInfo.Kind.EmailAddress)": Contact.UserInfo.Kind.EmailAddress,
          "\(Contact.UserInfo.Kind.WechatId)": Contact.UserInfo.Kind.WechatId
        ]

        let keyValue = result!.components(separatedBy: separator)
        let type = details[keyValue[0]]!
        let value = keyValue[1]

        let ret = self.mContact!.setIdentifyCode(type: type, value: value)
        if(ret < 0) {
          self.showMessage(ViewController.ErrorPrefix + "Failed to set \(result!). ret=\(ret)")
        }
    })

    let info = mContact!.getUserInfo()
    if info == nil {
      return ViewController.ErrorPrefix + "Failed to get user info."
    }

    return info.toString()
  }

  private func showSetUserDetails() -> String {
    if mContact == nil {
      return ViewController.ErrorPrefix + "Contact is null."
    }

    let checkList = [
      "\(Contact.UserInfo.Item.Nickname)",
      "\(Contact.UserInfo.Item.Avatar)",
      "\(Contact.UserInfo.Item.Gender)",
      "\(Contact.UserInfo.Item.Description)"
    ]
    let separator = ":-:-:"
    
    
    Helper.showSetDetails(view: self,
                          checkList: checkList, separator: separator,
                          listener: { result in
          let details = [
                  "\(Contact.UserInfo.Item.Nickname)": Contact.UserInfo.Item.Nickname,
                  "\(Contact.UserInfo.Item.Avatar)": Contact.UserInfo.Item.Avatar,
                  "\(Contact.UserInfo.Item.Gender)": Contact.UserInfo.Item.Gender,
                  "\(Contact.UserInfo.Item.Description)": Contact.UserInfo.Item.Description
          ]
                              
          let keyValue = result!.components(separatedBy: separator)
          let item = details[keyValue[0]]!
          let value = keyValue[1]

          let ret = self.mContact!.setUserInfo(item: item, value: value)
          if(ret < 0) {
            self.showMessage(ViewController.ErrorPrefix + "Failed to set \(result!). ret=\(ret)")
          }
      })

      let info = mContact!.getUserInfo()
      if info == nil {
        return ViewController.ErrorPrefix + "Failed to get user info."
      }

      return info.toString()
  }

  private func showSetWalletAddress() -> String {
    if mContact == nil {
      return ViewController.ErrorPrefix + "Contact is null."
    }

    let checkList = [
            "ELA",
            "BTC",
            "ETH"
    ]
    let separator = ":-:-:"

    Helper.showSetDetails(view: self,
                          checkList: checkList, separator: separator,
                          listener: { result in
    
          let keyValue = result!.components(separatedBy: separator)
          let name = keyValue[0]
          let value = keyValue[1]
                            
          let ret = self.mContact!.setWalletAddress(name: name, value: value)
          if(ret < 0) {
            self.showMessage(ViewController.ErrorPrefix + "Failed to set \(result!). ret=\(ret)")
          }
      })

      let info = mContact!.getUserInfo()
      if info == nil {
        return ViewController.ErrorPrefix + "Failed to get user info."
      }

      return info.toString()
  }

  private func testSyncUpload() -> String {
    if mContact == nil {
      return ViewController.ErrorPrefix + "Contact is null."
    }

    let ret = mContact!.syncInfoUploadToDidChain()
    if(ret < 0) {
      return ViewController.ErrorPrefix + "Failed to call syncInfoUploadToDidChain() ret=\(ret)"
    }

    return "Success to syncInfoUploadToDidChain."
  }

  private func testSyncDownload() -> String {
    if mContact == nil {
      return ViewController.ErrorPrefix + "Contact is null."
    }

    let ret = mContact!.syncInfoDownloadFromDidChain()
    if(ret < 0) {
      return ViewController.ErrorPrefix + "Failed to call syncInfoDownloadFromDidChain() ret=\(ret)"
    }

    return "Success to syncInfoDownloadFromDidChain ."
  }

  private func testLoopMessage() -> String {
    if mContact == nil {
      return ViewController.ErrorPrefix + "Contact is null."
    }
    let info = mContact!.getUserInfo()
    if info == nil {
      return ViewController.ErrorPrefix + "Failed to get user info."
    }
    
    let msgInfo = Contact.MakeTextMessage(text: "test loop message", cryptoAlgorithm: nil)
    let ret = mContact!.sendMessage(friendCode: info!.humanCode!,
                                    channelType: mCustomChannelStrategy!.getChannel(),
                                    message: msgInfo)
    if(ret < 0) {
      return ViewController.ErrorPrefix + "Failed to call testLoopMessage() ret=\(ret)"
    }

    return "Success to testLoopMessage."
  }
  
  private func listFriendInfo() -> String {
    if mContact == nil {
      return ViewController.ErrorPrefix + "Contact is null."
    }

    let friendList = mContact!.listFriendInfo()
    if(friendList == nil) {
      return ViewController.ErrorPrefix + "Failed to list friend info."
    }
    let friendCodeList = mContact!.listFriendCode()

    Helper.showFriendList(view: self, friendList: friendCodeList, listener:  { friendCode in
      var friendInfo: Contact.FriendInfo?
      for info in friendList! {
        if(info.humanCode == friendCode) {
            friendInfo = info
            break
        }
      }
      Helper.showDetails(view: self, msg: friendInfo!.toJson()!)
    })
    return "Success to list friend info."
  }

  private func scanUserInfo() -> String {
      if mContact == nil {
        return ViewController.ErrorPrefix + "Contact is null."
      }

      Helper.scanAddress(view: self, listener: { result in
        guard result != nil else {
          self.showMessage(ViewController.ErrorPrefix + "Failed to scan QRCode.")
          return
        }
        
        self.showMessage(result!)

        Helper.showAddFriend(view: self, friendCode: result!, listener: { summary in
          let ret = self.mContact!.addFriend(friendCode: result!, summary: summary!)
            if(ret < 0) {
              self.showMessage(ViewController.ErrorPrefix + "Failed to add friend. ret=\(ret)")
            }
          })
      })

      return ""
  }
  
  private func removeFriend() -> String {
    if mContact == nil {
      return ViewController.ErrorPrefix + "Contact is null."
    }
    let info = mContact!.getUserInfo()
    if info == nil {
      return ViewController.ErrorPrefix + "Failed to get user info."
    }

    if (info!.status != Contact.Status.Online) {
      return ViewController.ErrorPrefix + "Contact is not online."
    }

    let separator = " "
    var friendCodeList = [String]()
    let friendList = mContact!.listFriendInfo()
    for friendInfo in friendList! {
        friendCodeList.append(friendInfo.humanCode! + separator + "[\(friendInfo.status)]")
    }
    
    Helper.showFriendList(view: self, friendList: friendCodeList, listener:  { result in
      let friendCode = result!.components(separatedBy: separator)[0]
      let dialog = UIAlertController(title: nil, message: nil, preferredStyle: .alert)
      dialog.message = "Are you sure to remove friend: " + friendCode
      dialog.addAction(UIAlertAction(title: "Accept", style: .default, handler: { _ in
        let ret = self.mContact!.removeFriend(friendCode: friendCode)
        if(ret < 0) {
          self.showMessage(ViewController.ErrorPrefix + "Failed to delete friend. ret=\(ret)")
          return;
        }
      }))
      dialog.addAction(UIAlertAction(title: "Cancel", style: .cancel))
      Helper.showDialog(self, dialog)
    })
    
    return "Success to send message.";
  }
  
  private func sendTextMessage() -> String {
    if mContact == nil {
      return ViewController.ErrorPrefix + "Contact is null."
    }
    let info = mContact!.getUserInfo()
    if info == nil {
      return ViewController.ErrorPrefix + "Failed to get user info."
    }

    if (info!.status != Contact.Status.Online) {
      return ViewController.ErrorPrefix + "Contact is not online."
    }

    let friendCodeList = mContact!.listFriendCode()
    Helper.showFriendList(view: self, friendList: friendCodeList, listener:  { friendCode in
      let separator = ":-:-:"
      Helper.showTextSendMessage(view: self, friendCode: friendCode!, separator: separator, listener:  { result in
        let keyValue = result!.components(separatedBy: separator)
        let message = keyValue[0]
        var replyTo: Int64 = 0
        if(keyValue.count > 1) {
          replyTo = Int64(keyValue[1]) ?? -1
          if(replyTo < 0) {
            self.showError("Failed to parse reply to nanotime.")
          }
        }
        
        let msgInfo = Contact.MakeTextMessage(text: message, cryptoAlgorithm: nil)
        if(replyTo != 0) {
          msgInfo.replyTo(toNanoTime: replyTo);
        }

        let status = self.mContact!.getStatus(humanCode: friendCode!)
        if(status != Contact.Status.Online) {
          self.showMessage(ViewController.ErrorPrefix + "Friend is not online.")
          return
        }

        let ret = self.mContact!.sendMessage(friendCode: friendCode!,
                                             channelType: Contact.Channel.Carrier,
                                             message: msgInfo)
        if(ret < 0) {
          self.showMessage(ViewController.ErrorPrefix + "Failed to send message to " + friendCode!)
        }
      })
    })
    
      return "Success to send text message.";
  }
  
  private func sendFileMessage() -> String {
    if mContact == nil {
      return ViewController.ErrorPrefix + "Contact is null."
    }
    let info = mContact!.getUserInfo()
    if info == nil {
      return ViewController.ErrorPrefix + "Failed to get user info."
    }

    if (info!.status != Contact.Status.Online) {
      return ViewController.ErrorPrefix + "Contact is not online."
    }

    let friendCodeList = mContact!.listFriendCode()
    Helper.showFriendList(view: self, friendList: friendCodeList, listener:  { friendCode in
      Helper.showFileSendMessage(view: self, friendCode: friendCode!, listener:  { filepath in
        guard filepath != nil else {
          self.showMessage(ViewController.ErrorPrefix + "File not selected.")
          return
        }
        
        let msgInfo = Contact.MakeFileMessage(file: URL(string: filepath!)!, cryptoAlgorithm: nil)

        let status = self.mContact!.getStatus(humanCode: friendCode!)
        if(status != Contact.Status.Online) {
          self.showMessage(ViewController.ErrorPrefix + "Friend is not online.")
          return
        }

        let ret = self.mContact!.sendMessage(friendCode: friendCode!,
                                             channelType: Contact.Channel.Carrier,
                                             message: msgInfo)
        if(ret < 0) {
          self.showMessage(ViewController.ErrorPrefix + "Failed to send message to " + friendCode!)
          return
        }
        
        self.mContactSendFileMap[msgInfo.data.toString()] = filepath!
      })
    })
    
      return "Success to send file message.";
  }

  private func pullFile() -> String {
    if mContact == nil {
      return ViewController.ErrorPrefix + "Contact is null."
    }
    let info = mContact!.getUserInfo()
    if info == nil {
      return ViewController.ErrorPrefix + "Failed to get user info."
    }

    if (info!.status != Contact.Status.Online) {
      return ViewController.ErrorPrefix + "Contact is not online."
    }

    let friendCodeList = mContact!.listFriendCode()
    Helper.showFriendList(view: self, friendList: friendCodeList, listener:  { friendCode in
      Helper.dismissDialog()
      
      let status = self.mContact!.getStatus(humanCode: friendCode!)
      if(status != Contact.Status.Online) {
        self.showMessage(ViewController.ErrorPrefix + "Friend is not online.")
        return
      }

      guard let fileData = self.mContactRecvFileMap[friendCode!] else {
        self.showMessage(ViewController.ErrorPrefix + "Failed to find pull file from \(friendCode!)")
        return
      }
      let ret = self.mContact!.pullFileAsync(friendCode: friendCode!,
                                             channelType: Contact.Channel.Carrier,
                                             fileInfo: fileData)
      if(ret < 0) {
        self.showMessage(ViewController.ErrorPrefix + "Failed to pull file from  \(friendCode!)")
      }
    })
    
      return "Success to start pull file.";
  }

  private func cancelPullFile() -> String {
    if mContact == nil {
      return ViewController.ErrorPrefix + "Contact is null."
    }
    let info = mContact!.getUserInfo()
    if info == nil {
      return ViewController.ErrorPrefix + "Failed to get user info."
    }

    if (info!.status != Contact.Status.Online) {
      return ViewController.ErrorPrefix + "Contact is not online."
    }

    let friendCodeList = mContact!.listFriendCode()
    Helper.showFriendList(view: self, friendList: friendCodeList, listener:  { friendCode in
      Helper.dismissDialog()
      
      let status = self.mContact!.getStatus(humanCode: friendCode!)
      if(status != Contact.Status.Online) {
        self.showMessage(ViewController.ErrorPrefix + "Friend is not online.")
        return
      }

      guard let fileData = self.mContactRecvFileMap[friendCode!] else {
        self.showMessage(ViewController.ErrorPrefix + "Failed to find pull file from \(friendCode!)")
        return
      }
      let ret = self.mContact!.cancelPullFile(friendCode: friendCode!,
                                              channelType: Contact.Channel.Carrier,
                                              fileInfo: fileData)
      if(ret < 0) {
        self.showMessage(ViewController.ErrorPrefix + "Failed to cancel pull file from  \(friendCode!)")
      }
    })
    
    return "Success to cancel pull file.";
  }

  private func getCachedDidProp() -> String {
    if mContact == nil {
      return ViewController.ErrorPrefix + "Contact is null."
    }

    var cachedDidProp = String()
    Contact.Debug.GetCachedDidProp(value: &cachedDidProp)
    
    Helper.showDetails(view: self, msg:  cachedDidProp)
    
    return "Success to get cached didprop.";
  }
  
  
  private func processAcquire(request: Contact.Listener.AcquireArgs) -> Data? {
    var response: Data?
  
    switch (request.type) {
      case .PublicKey:
        response = getPublicKey().data(using: .utf8)
        break
      case .EncryptData:
        response = request.data // plaintext
        break
      case .DecryptData:
        response = request.data // plaintext
        break
      case .DidPropAppId:
        let appId = "DC92DEC59082610D1D4698F42965381EBBC4EF7DBDA08E4B3894D530608A64AAA65BB82A170FBE16F04B2AF7B25D88350F86F58A7C1F55CC29993B4C4C29E405"
        response = appId.data(using: .utf8)
        break
      case .DidAgentAuthHeader:
        response = getAgentAuthHeader()
        break
      case .SignData:
        response = signData(data: request.data)
        break
    }
  
    return response
  }

  private func processEvent(event: Contact.Listener.EventArgs) {
    switch (event.type) {
      case .StatusChanged:
        break
      case .FriendRequest:
        let requestEvent = event as! Contact.Listener.RequestEvent
        Helper.showFriendRequest(view: self,
                                 humanCode: requestEvent.humanCode, summary: requestEvent.summary,
                                 listener: { _ in
          let ret = self.mContact!.acceptFriend(friendCode: requestEvent.humanCode)
          if(ret < 0) {
            self.showMessage(ViewController.ErrorPrefix + "Failed to acceptFriend \(requestEvent.humanCode). ret=\(ret)")
          }
        })
        break
    case .HumanInfoChanged:
      let infoEvent = event as! Contact.Listener.InfoEvent
      let msg = event.humanCode + " info changed: " + infoEvent.toString()
      showEvent(msg)
      break
    }
  }
  
  private func getPublicKey() -> String {
    var seed = Data()
    var ret = Contact.Debug.Keypair.GetSeedFromMnemonic(mnemonic: mSavedMnemonic!,
                                                        mnemonicPassword: "",
                                                        seed: &seed)
    if(ret < 0) {
      showMessage(ViewController.ErrorPrefix + "Failed to call Contact.Debug.Keypair.GetSeedFromMnemonic()")
    }
    var pubKey = String()
    ret = Contact.Debug.Keypair.GetSinglePublicKey(seed: seed, pubKey: &pubKey)
    if(ret < 0) {
      showMessage(ViewController.ErrorPrefix + "Failed to call Contact.Debug.Keypair.GetSinglePublicKey()")
    }

    return pubKey
  }

  private func getPrivateKey() -> String {
    var seed = Data()
    var ret = Contact.Debug.Keypair.GetSeedFromMnemonic(mnemonic: mSavedMnemonic!,
                                                        mnemonicPassword: "",
                                                        seed: &seed)
    if(ret < 0) {
      showMessage(ViewController.ErrorPrefix + "Failed to call Contact.Debug.Keypair.GetSeedFromMnemonic()")
    }
    var privKey = String()
    ret = Contact.Debug.Keypair.GetSinglePrivateKey(seed: seed, privKey: &privKey)
    if(ret < 0) {
      showMessage(ViewController.ErrorPrefix + "Failed to call Contact.Debug.Keypair.GetSinglePrivateKey()")
    }

    return privKey
  }
  
  private func getAgentAuthHeader() -> Data {
    let appid = "org.elastos.debug.didplugin"
    //let appkey = "b2gvzUM79yLhCbbGNWCuhSsGdqYhA7sS"
    let timestamp = Int64(Date().timeIntervalSince1970 * 1000)
    let auth = Utils.getMD5Sum(str: "appkey\(timestamp)")
    let headerValue = "id=\(appid)time=\(timestamp)auth=\(auth)"
    Log.i(tag: ViewController.TAG, msg: "getAgentAuthHeader() headerValue=" + headerValue)
  
    return headerValue.data(using: .utf8)!
  }
  
  private func signData(data: Data?) -> Data? {
    if data == nil {
      return nil
    }
    
    let privKey = getPrivateKey()

    var signedData = Data()
    let ret = Contact.Debug.Keypair.Sign(privateKey: privKey, data: data!, signedData: &signedData)
    if(ret < 0) {
      showMessage(ViewController.ErrorPrefix + "Failed to call Contact.Debug.Keypair.Sign()")
      return nil
    }
  
    return signedData
  }
  
  private func getDeviceId() -> String {
    let devId = UIDevice.current.identifierForVendor?.uuidString
    return devId!
  }

  
  private func showMessage(_ msg: String) {
    Log.i(tag: ViewController.TAG, msg: "\(msg)")
    
    DispatchQueue.main.async { [weak self] in
      self?.mMsgLog.text = msg
    }
    
    if msg.hasPrefix(ViewController.ErrorPrefix) {
      showToast(msg)
    }
  }
  
  private func showEvent(_ newMsg: String) {
    print("\(newMsg)")
    DispatchQueue.main.async { [weak self] in
      self?.mEventLog.text += "\n"
      self?.mEventLog.text += newMsg
    }
  }
  
  private func showError(_ newErr: String) {
    Log.e(tag: ViewController.TAG, msg: newErr)

    DispatchQueue.main.async { [weak self] in
      self?.mErrLog.text = newErr
    }
  }

  private func showToast(_ message : String) {
    let alert = UIAlertController(title: nil, message: message, preferredStyle: .alert)
    alert.view.backgroundColor = UIColor.black
    alert.view.alpha = 0.6
    alert.view.layer.cornerRadius = 15
    
    DispatchQueue.main.async { [weak self] in
      self?.present(alert, animated: false)
    }
    
    DispatchQueue.main.asyncAfter(deadline: DispatchTime.now() + 1) {
      alert.dismiss(animated: true)
    }
  }
  
  private func isEnglishWords(_ words: String?) -> Bool {
    guard (words?.count ?? -1) > 0 else {
      return false
    }
    
    let isEnglish = (words!.range(of: "[^a-zA-Z ]", options: .regularExpression) == nil)
    return isEnglish
  }
  
  private var mPopMenu: SwiftPopMenu?
  
  @IBOutlet weak var mErrLog: UITextView!
  @IBOutlet weak var mMsgLog: UITextView!
  @IBOutlet weak var mEventLog: UITextView!
  
  private var mCacheDir: URL?
  private var mSavedMnemonic: String?
  private var mContact: Contact?
  private var mCustomChannelStrategy: Contact.ChannelStrategy?
  private var mContactListener: Contact.Listener?
  private var mContactDataListener: Contact.DataListener?
  
  private var mContactRecvFileMap = [String: Contact.Message.FileData]()
  private var mContactSendFileMap = [String: String]()
  
  private static let KeypairLanguage = "english"
  private static let KeypairWords = ""
  private static let SavedMnemonicKey = "mnemonic"
  private static let ErrorPrefix = "Error: "
  private static let TAG = "ContactTest"
  private static let MENU_WIDTH: CGFloat = 280
  private static let MENU_ITEM: KeyValuePairs = [
    "Get Started": getStarted,
    "Log": nil,
    "   `- Clear Event": clearEvent,
    "Contact": nil,
    "   |- New and Start Contact": newAndStartContact,
    "   |- Stop and Del Contact": stopAndDelContact,
    "   |- Recreate Contact": recreateContact,
    "   `- Restart Contact": restartContact,
    "User": nil,
    "   |- Get User Info": showGetUserInfo,
    "   |- Set User IdentifyCode": showSetUserIdentifyCode,
    "   |- Set User Details": showSetUserDetails,
    "   |- Set Wallet Address": showSetWalletAddress,
    "   |- Sync Upload": testSyncUpload,
    "   `- Sync Download": testSyncDownload,
    "   `- Send Loop Message": testLoopMessage,
    "Friend": nil,
    "   |- Friend Info": listFriendInfo,
    "   |- Add Friend": scanUserInfo,
    "   |- Del Friend": removeFriend,
    "   |- Set Friend Details": nil,
    "   |- Send Text Message": sendTextMessage,
    "   |- Send Binary Message": nil,
    "   |- Send File Message": sendFileMessage,
    "   |- Pull File": pullFile,
    "   `- Cancel Pull File": cancelPullFile,
    "Debug": nil,
    "   `- Show Cached DidProp": getCachedDidProp,
    "Mnemonic": nil,
    "   |- New And Save Mnemonic": newAndSaveMnemonic,
    "   `- Import Mnemonic": importMnemonic,
  ]
}

