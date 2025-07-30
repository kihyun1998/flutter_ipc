#include "flutter_ipc_plugin.h"

// This must be included before many other Windows headers.
#include <windows.h>

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>
#include <flutter/standard_method_codec.h>

#include <memory>
#include <sstream>
#include <string>
#include <chrono>

namespace flutter_ipc {

// NamedPipeServer Implementation
NamedPipeServer::NamedPipeServer(const std::string& pipe_name)
    : pipe_name_(pipe_name), pipe_handle_(INVALID_HANDLE_VALUE), is_connected_(false) {
  ZeroMemory(&overlap_, sizeof(OVERLAPPED));
}

NamedPipeServer::~NamedPipeServer() {
  Close();
}

bool NamedPipeServer::Create() {
  std::string full_pipe_name = "\\\\.\\pipe\\" + pipe_name_;
  
  pipe_handle_ = CreateNamedPipeA(
    full_pipe_name.c_str(),
    PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
    PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
    1, // Maximum instances
    4096, // Output buffer size
    4096, // Input buffer size
    0, // Default timeout
    NULL // Security attributes
  );
  
  return pipe_handle_ != INVALID_HANDLE_VALUE;
}

bool NamedPipeServer::WaitForConnection() {
  if (pipe_handle_ == INVALID_HANDLE_VALUE) {
    return false;
  }
  
  BOOL connected = ConnectNamedPipe(pipe_handle_, &overlap_);
  
  if (connected) {
    is_connected_ = true;
    return true;
  }
  
  DWORD error = GetLastError();
  if (error == ERROR_PIPE_CONNECTED) {
    is_connected_ = true;
    return true;
  } else if (error == ERROR_IO_PENDING) {
    // Asynchronous operation - would need event handling for full implementation
    // For now, return true to indicate operation started
    return true;
  }
  
  return false;
}

void NamedPipeServer::Close() {
  if (pipe_handle_ != INVALID_HANDLE_VALUE) {
    if (is_connected_) {
      DisconnectNamedPipe(pipe_handle_);
      is_connected_ = false;
    }
    CloseHandle(pipe_handle_);
    pipe_handle_ = INVALID_HANDLE_VALUE;
  }
}

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
