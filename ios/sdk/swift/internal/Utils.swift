import CommonCrypto

public class Utils {
  private init() {}
  
  public static func getMD5Sum(str: String) -> String {
    let length = Int(CommonCrypto.CC_MD5_DIGEST_LENGTH)
    let messageData = str.data(using: .utf8)!
    var digestData = Data(count: length)
    
    _ = digestData.withUnsafeMutableBytes { digestBytes -> UInt8 in
      messageData.withUnsafeBytes { messageBytes -> UInt8 in
        if let messageBytesBaseAddress = messageBytes.baseAddress, let digestBytesBlindMemory = digestBytes.bindMemory(to: UInt8.self).baseAddress {
          let messageLength = CC_LONG(messageData.count)
          CC_MD5(messageBytesBaseAddress, messageLength, digestBytesBlindMemory)
        }
        return 0
      }
    }
    return digestData.map { String(format: "%02hhx", $0) }.joined()
  }

  public static func getMD5Sum(file: URL?) -> String? {
    guard file != nil else {
      return nil
    }
    guard let file = try? FileHandle(forReadingFrom: file!) else {
      return nil
    }
    defer {
        file.closeFile()
    }
    
    var context = CC_MD5_CTX()
    CC_MD5_Init(&context)
    
    let bufferSize = 1024 * 1024
    while autoreleasepool(invoking: {
        let data = file.readData(ofLength: bufferSize)
        if data.count > 0 {
            data.withUnsafeBytes {
                _ = CC_MD5_Update(&context, $0.baseAddress, numericCast(data.count))
            }
            return true // Continue
        } else {
            return false // End of file
        }
    }) { }

    // Compute the MD5 digest:
    var digest: [UInt8] = Array(repeating: 0, count: Int(CC_MD5_DIGEST_LENGTH))
    _ = CC_MD5_Final(&digest, &context)
    
    let hexDigest = digest.map { String(format: "%02hhx", $0) }.joined()
    return hexDigest
  }
}
