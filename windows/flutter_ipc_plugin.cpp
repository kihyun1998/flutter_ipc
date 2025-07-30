#include "flutter_ipc_plugin.h"

// This must be included before many other Windows headers.
#include <windows.h>

// For getPlatformVersion; remove unless needed for your plugin implementation.
#include <VersionHelpers.h>

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <memory>
#include <sstream>

namespace flutter_ipc {

// static
void FlutterIpcPlugin::RegisterWithRegistrar(
    flutter::PluginRegistrarWindows *registrar) {
  auto channel =
      std::make_unique<flutter::MethodChannel<flutter::EncodableValue>>(
          registrar->messenger(), "flutter_ipc",
          &flutter::StandardMethodCodec::GetInstance());

  auto plugin = std::make_unique<FlutterIpcPlugin>();

  channel->SetMethodCallHandler(
      [plugin_pointer = plugin.get()](const auto &call, auto result) {
        plugin_pointer->HandleMethodCall(call, std::move(result));
      });

  registrar->AddPlugin(std::move(plugin));
}

FlutterIpcPlugin::FlutterIpcPlugin() {}

FlutterIpcPlugin::~FlutterIpcPlugin() {}

void FlutterIpcPlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue> &method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  
  const std::string& method = method_call.method_name();
  
  if (method == "createServer") {
    result->Error("NOT_IMPLEMENTED", "createServer not implemented yet");
  } 
  else if (method == "connect") {
    result->Error("NOT_IMPLEMENTED", "connect not implemented yet");
  }
  else if (method == "listen") {
    result->Error("NOT_IMPLEMENTED", "listen not implemented yet");
  }
  else if (method == "sendMessageFromServer") {
    result->Error("NOT_IMPLEMENTED", "sendMessageFromServer not implemented yet");
  }
  else if (method == "sendMessageFromClient") {
    result->Error("NOT_IMPLEMENTED", "sendMessageFromClient not implemented yet");
  }
  else if (method == "closeServer") {
    result->Error("NOT_IMPLEMENTED", "closeServer not implemented yet");
  }
  else if (method == "disconnect") {
    result->Error("NOT_IMPLEMENTED", "disconnect not implemented yet");
  }
  else {
    result->NotImplemented();
  }
}

}  // namespace flutter_ipc
