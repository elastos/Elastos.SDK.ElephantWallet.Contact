
extension ContactInternal {

open class Log {
  private init() {}
  
  public static func v(tag: String, msg: String) {
    log(LogPriority.LOG_VERBOSE, tag, msg)
  }
  
  public static func d(tag: String, msg: String) {
    log(LogPriority.LOG_DEBUG, tag, msg)
  }
  
  public static func i(tag: String, msg: String) {
    log(LogPriority.LOG_INFO, tag, msg)
  }
  
  public static func w(tag: String, msg: String) {
    log(LogPriority.LOG_WARN, tag, msg)
  }
  
  public static func e(tag: String, msg: String) {
    log(LogPriority.LOG_ERROR, tag, msg)
  }
  
  private static func log(_ prio: LogPriority, _ tag: String, _ msg: String) {
    let prioMap = [
      LogPriority.LOG_VERBOSE: "V",
      LogPriority.LOG_DEBUG: "D",
      LogPriority.LOG_INFO: "I",
      LogPriority.LOG_WARN: "W",
      LogPriority.LOG_ERROR: "E",
    ]
    let prioStr = prioMap[prio]!
    
    print("\(prioStr) \(tag): \(msg)")
  }
  
  private enum LogPriority: Int {
    case LOG_VERBOSE
    case LOG_DEBUG
    case LOG_INFO
    case LOG_WARN
    case LOG_ERROR
  }
}

}
