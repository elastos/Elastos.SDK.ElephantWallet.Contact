import Foundation

class CrossClassInfo {
  static func Parse(sourcePath: String, className: String,
                    sourceLines: [String], classIndex: Int,
                    productName: String, bundleId: String) -> CrossClassInfo {
    CrossClassInfo.ProductName = productName
    
    let classInfo = CrossClassInfo()
    classInfo.sourcePath = sourcePath
    classInfo.cppInfo.className = className
    classInfo.swiftInfo.className = className
    classInfo.swiftInfo.classPath = bundleId + "." + className
    
    var nativeMethodList = [String]()
    var platformMethodList = [String]()
    var layer = 0;
    for idx in classIndex..<sourceLines.count {
      let line = sourceLines[idx]
      if line.contains("{") {
        layer += 1
      }
      if line.contains("}") {
        layer -= 1
        if(layer == 0) {
          break;
        }
      }
      if layer != 1 { // only recode top layer function
        continue
      }
      
      if line.contains(Annotation.CrossNativeInterface) {
        let methodLine = getMethodLine(sourceLines: sourceLines, from: idx)
        nativeMethodList.append(methodLine)
      } else if line.contains(Annotation.CrossPlatformInterface) {
        let methodLine = getMethodLine(sourceLines: sourceLines, from: idx)
        platformMethodList.append(methodLine)
      }
    }
//    print("nativeMethodList=\(nativeMethodList)")
//    print("platformMethodList=\(platformMethodList)")
    
    let ast = printAst(filePath: sourcePath)
    let classLines = ast.components(separatedBy: .newlines)
    var classLayer = -1
    layer = 0
    for idx in classLines.indices {
      let line = classLines[idx];
      if line.contains("{") {
        layer += 1
      }
      if line.contains("}") {
        layer -= 1
      }
      if line.contains(" \(className) ") {
        classLayer = layer
      }
    
      if layer == classLayer
      && line.contains(" func ") {
        let methodName = line.replace(".*func (\\w*)\\(.*") { "\($0[1])" }
        var isCrossInterface = false
        var isNative = false
        var argTypes: [String]? = nil
        
        let nativeMethodLines = nativeMethodList.filter() { $0.contains("func \(methodName)(") }
        if (nativeMethodLines.count != 0) {
          isCrossInterface = true
          isNative = true
          argTypes = getMethodArgTypes(methodLine: nativeMethodLines.first!)
        }
        let platformMethodLines = platformMethodList.filter() { $0.contains("func \(methodName)(") }
        if (platformMethodLines.count != 0
        && line.contains(" @objc ")) {
          isCrossInterface = true
          isNative = false
          argTypes = getMethodArgTypes(methodLine: platformMethodLines.first!)
        }
       
        if isCrossInterface == true {
          let methodInfo = CrossMethodInfo.Parse(sourceContent: line,
                                                 methodName: methodName, argTypes: argTypes!,
                                                 isNative: isNative)
          print("  \(methodInfo?.toString() ?? "xxxxxxxxxxxxx: Failed to parse \(line)")")
          
          if methodInfo != nil {
            classInfo.methodInfoList.append(methodInfo!)
          }
        }
      }
    }
    
    return classInfo
  }
  
  static func getMethodLine(sourceLines: [String], from: Int) -> String {
    var methodLine = String()
    var next = 1
    repeat {
      methodLine += sourceLines[from + next]
      next += 1
    } while methodLine.contains(")") == false
    
    return methodLine
  }
  
  static func getMethodArgTypes(methodLine: String) -> [String] {
    var argTypes = [String]()
    
    let argsLine = methodLine.replace(".*\\((.*)\\).*") { "\($0[1])" }
    if argsLine.isEmpty == true {
      return argTypes
    }
    
    let argsList = argsLine.components(separatedBy: ",")
    for arg in argsList {
      let split = arg.components(separatedBy: ":")
      var type = split[1].replacingOccurrences(of: "?", with: "") // remove '?'
      type = type.trimmingCharacters(in: .whitespacesAndNewlines) // trim
      
      argTypes.append(type)
    }
    
    return argTypes
  }
  
  func toString() -> String {
    var dump = String()
    dump += "ClassInfo{cppInfo=\(cppInfo.toString()),"
    dump += " swiftInfo=\(swiftInfo.toString()),"
    dump += " methodInfoList={"
    for it in methodInfoList {
      dump += it.toString() + ","
    }
    dump += "}}"
    
    return dump
  }

  
  class CppInfo {
    var className: String?
    
    func toString() -> String {
      return  "CppInfo{className=\(className ?? "nil")"
    }
  }
  
  class SwiftInfo {
    var className: String?
    var classPath: String?
    
    func toString() -> String {
      return "SwiftInfo{className=\(className ?? "nil"), classPath=\(classPath ?? "nil")"
    }
  }

  private static func printAst(filePath: String) -> String {
    let task = Process()
    task.environment = nil
    task.launchPath = "/bin/bash"
    task.arguments = ["-c", "swiftc -print-ast \(filePath) 2>/dev/null"]
    
    let outputPipe = Pipe()
    let errorPipe = Pipe()
    task.standardOutput = outputPipe
    task.standardError = errorPipe
    task.launch()
    var data = outputPipe.fileHandleForReading.readDataToEndOfFile()
    let output = String(data: data, encoding: .utf8)!
    
    data = errorPipe.fileHandleForReading.readDataToEndOfFile()
    let error = String(data: data, encoding: .utf8)!
    
    task.waitUntilExit()
    print("print-ast error: \(filePath):\n\(error)")
    print("print-ast: \(filePath):\n\(output)")
    
    return output
  }
  
  func makeProxyDeclare(tmpl: String) -> String {
    var nativeFuncList = ""
    var platformFuncList = ""
    methodInfoList.forEach { (it) in
      let functionDeclare = it.makeProxyDeclare(cppClassName: cppInfo.className!)
      
      if it.isNative! {
        nativeFuncList += "\(functionDeclare);\n"
      } else {
        platformFuncList += "\(functionDeclare);\n"
      }
    }
  
    let content = tmpl
      .replacingOccurrences(of: CrossClassInfo.TmplKeyClassName, with: cppInfo.className!)
      .replacingOccurrences(of: CrossClassInfo.TmplKeyPlatformFunction, with: platformFuncList)
      .replacingOccurrences(of: CrossClassInfo.TmplKeyNativeFunction, with: nativeFuncList)
  
    return content
  }

  func makeProxySource(tmpl: String) -> String {
    var nativeFuncList = ""
    var platformFuncList = ""
    var objcNativeMethodList = ""
    methodInfoList.forEach { (it) in
      let funcSource = it.makeProxySource(cppClassName: cppInfo.className!, javaClassPath: swiftInfo.classPath!)
      if it.isNative! {
        nativeFuncList += "\(funcSource)"
  
        let methodContent = makeObjcNativeMethod(methodInfo: it)
        objcNativeMethodList += "\(CrossTmplUtils.TabSpace)\(CrossTmplUtils.TabSpace)\(methodContent),\n"
      } else {
        platformFuncList += "\(funcSource)"
      }
    }
  
    let content = tmpl
      .replacingOccurrences(of: CrossClassInfo.TmplKeyProductName, with: CrossClassInfo.ProductName!)
      .replacingOccurrences(of: CrossClassInfo.TmplKeyClassName, with: cppInfo.className!)
      .replacingOccurrences(of: CrossClassInfo.TmplKeyPlatformFunction, with: platformFuncList)
      .replacingOccurrences(of: CrossClassInfo.TmplKeyNativeFunction, with: nativeFuncList)
      .replacingOccurrences(of: CrossClassInfo.TmplKeyObjcNativeMethods, with: objcNativeMethodList)
      .replacingOccurrences(of: CrossClassInfo.TmplKeyObjcSwiftClass, with: swiftInfo.classPath!)
  
    return content
  }

  var sourcePath: String? = nil
  var cppInfo = CppInfo()
  var swiftInfo = SwiftInfo()
  private var methodInfoList = [CrossMethodInfo]()
  
  private func makeObjcNativeMethod(methodInfo: CrossMethodInfo) -> String {
    var funcType = "("
    methodInfo.paramsType.forEach { (it) in
      funcType += it.toSwiftString()
    }
    funcType += ")"
    funcType += methodInfo.returnType!.toSwiftString()
  
    let methodContent = "{\"\(methodInfo.methodName!)\", \"\(funcType)\", (void*)\(methodInfo.methodName!)}"
  
    return methodContent
  }

  private static var ProductName: String?
  
  private static let CrossBaseClass = "org.elastos.tools.crosspl.CrossBase"
  
  private static let TmplKeyProductName: String = "%ProductName%"
  private static let TmplKeyClassName: String = "%ClassName%"
  private static let TmplKeyPlatformFunction = "%PlatformFunction%"
  private static let TmplKeyNativeFunction = "%NativeFunction%"
  
  private static let TmplKeyObjcSwiftClass = "%ObjcSwiftClass%"
  private static let TmplKeyObjcNativeMethods = "%ObjcNativeMethods%"
}
