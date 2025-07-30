import Cocoa
import FlutterMacOS

public class FlutterIpcPlugin: NSObject, FlutterPlugin {
  public static func register(with registrar: FlutterPluginRegistrar) {
    let channel = FlutterMethodChannel(name: "flutter_ipc", binaryMessenger: registrar.messenger)
    let instance = FlutterIpcPlugin()
    registrar.addMethodCallDelegate(instance, channel: channel)
  }

  public func handle(_ call: FlutterMethodCall, result: @escaping FlutterResult) {
    switch call.method {
    case "createServer":
      result(FlutterError(code: "NOT_IMPLEMENTED", message: "createServer not implemented yet", details: nil))
    case "connect":
      result(FlutterError(code: "NOT_IMPLEMENTED", message: "connect not implemented yet", details: nil))
    case "listen":
      result(FlutterError(code: "NOT_IMPLEMENTED", message: "listen not implemented yet", details: nil))
    case "sendMessageFromServer":
      result(FlutterError(code: "NOT_IMPLEMENTED", message: "sendMessageFromServer not implemented yet", details: nil))
    case "sendMessageFromClient":
      result(FlutterError(code: "NOT_IMPLEMENTED", message: "sendMessageFromClient not implemented yet", details: nil))
    case "closeServer":
      result(FlutterError(code: "NOT_IMPLEMENTED", message: "closeServer not implemented yet", details: nil))
    case "disconnect":
      result(FlutterError(code: "NOT_IMPLEMENTED", message: "disconnect not implemented yet", details: nil))
    default:
      result(FlutterMethodNotImplemented)
    }
  }
}
