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

// NamedPipeClient Implementation
NamedPipeClient::NamedPipeClient(const std::string& pipe_name)
    : pipe_name_(pipe_name), pipe_handle_(INVALID_HANDLE_VALUE), is_connected_(false) {
}

NamedPipeClient::~NamedPipeClient() {
  Disconnect();
}

bool NamedPipeClient::Connect() {
  if (is_connected_) {
    return true; // Already connected
  }
  
  std::string full_pipe_name = "\\\\.\\pipe\\" + pipe_name_;
  
  // Try to connect to the named pipe
  pipe_handle_ = CreateFileA(
    full_pipe_name.c_str(),
    GENERIC_READ | GENERIC_WRITE,
    0,              // No sharing
    NULL,           // Default security attributes
    OPEN_EXISTING,  // Opens existing pipe
    0,              // Default attributes
    NULL            // No template file
  );
  
  if (pipe_handle_ == INVALID_HANDLE_VALUE) {
    return false;
  }
  
  // Set pipe mode to message mode
  DWORD mode = PIPE_READMODE_MESSAGE;
  BOOL success = SetNamedPipeHandleState(
    pipe_handle_,
    &mode,
    NULL,     // Don't set maximum bytes
    NULL      // Don't set maximum time
  );
  
  if (!success) {
    CloseHandle(pipe_handle_);
    pipe_handle_ = INVALID_HANDLE_VALUE;
    return false;
  }
  
  is_connected_ = true;
  return true;
}

void NamedPipeClient::Disconnect() {
  if (pipe_handle_ != INVALID_HANDLE_VALUE) {
    CloseHandle(pipe_handle_);
    pipe_handle_ = INVALID_HANDLE_VALUE;
  }
  is_connected_ = false;
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

std::string FlutterIpcPlugin::GenerateServerId() {
  static int counter = 0;
  return "server_" + std::to_string(++counter);
}

std::string FlutterIpcPlugin::GenerateClientId() {
  static int counter = 0;
  return "client_" + std::to_string(++counter);
}

std::string GetWindowsErrorMessage(DWORD error_code) {
  LPSTR message_buffer = nullptr;
  size_t size = FormatMessageA(
    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL, error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    (LPSTR)&message_buffer, 0, NULL
  );
  
  std::string message(message_buffer, size);
  LocalFree(message_buffer);
  
  // Remove trailing newline characters
  while (!message.empty() && (message.back() == '\n' || message.back() == '\r')) {
    message.pop_back();
  }
  
  return message;
}

void FlutterIpcPlugin::HandleMethodCall(
    const flutter::MethodCall<flutter::EncodableValue> &method_call,
    std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result) {
  
  const std::string& method = method_call.method_name();
  
  if (method == "createServer") {
    const auto* arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!arguments) {
      result->Error("INVALID_ARGUMENTS", "Missing arguments for createServer");
      return;
    }
    
    auto pipe_name_it = arguments->find(flutter::EncodableValue("pipeName"));
    if (pipe_name_it == arguments->end()) {
      result->Error("INVALID_ARGUMENTS", "Missing pipeName argument");
      return;
    }
    
    const auto* pipe_name = std::get_if<std::string>(&pipe_name_it->second);
    if (!pipe_name) {
      result->Error("INVALID_ARGUMENTS", "pipeName must be a string");
      return;
    }
    
    try {
      std::string server_id = GenerateServerId();
      auto server = std::make_unique<NamedPipeServer>(*pipe_name);
      
      if (!server->Create()) {
        DWORD error = GetLastError();
        std::string error_msg = "Failed to create named pipe '" + *pipe_name + "': " + GetWindowsErrorMessage(error) + " (Code: " + std::to_string(error) + ")";
        result->Error("PIPE_CREATION_FAILED", error_msg);
        return;
      }
      
      servers_[server_id] = std::move(server);
      result->Success(flutter::EncodableValue(server_id));
    } catch (const std::exception& e) {
      result->Error("CREATE_SERVER_FAILED", e.what());
    }
  } 
  else if (method == "connect") {
    const auto* arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!arguments) {
      result->Error("INVALID_ARGUMENTS", "Missing arguments for connect");
      return;
    }
    
    auto pipe_name_it = arguments->find(flutter::EncodableValue("pipeName"));
    if (pipe_name_it == arguments->end()) {
      result->Error("INVALID_ARGUMENTS", "Missing pipeName argument");
      return;
    }
    
    const auto* pipe_name = std::get_if<std::string>(&pipe_name_it->second);
    if (!pipe_name) {
      result->Error("INVALID_ARGUMENTS", "pipeName must be a string");
      return;
    }
    
    try {
      std::string client_id = GenerateClientId();
      auto client = std::make_unique<NamedPipeClient>(*pipe_name);
      
      if (!client->Connect()) {
        DWORD error = GetLastError();
        std::string error_msg = "Failed to connect to named pipe '" + *pipe_name + "': " + GetWindowsErrorMessage(error) + " (Code: " + std::to_string(error) + ")";
        result->Error("PIPE_CONNECTION_FAILED", error_msg);
        return;
      }
      
      clients_[client_id] = std::move(client);
      result->Success(flutter::EncodableValue(client_id));
    } catch (const std::exception& e) {
      result->Error("CONNECT_FAILED", e.what());
    }
  }
  else if (method == "listen") {
    const auto* arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!arguments) {
      result->Error("INVALID_ARGUMENTS", "Missing arguments for listen");
      return;
    }
    
    auto server_id_it = arguments->find(flutter::EncodableValue("serverId"));
    if (server_id_it == arguments->end()) {
      result->Error("INVALID_ARGUMENTS", "Missing serverId argument");
      return;
    }
    
    const auto* server_id = std::get_if<std::string>(&server_id_it->second);
    if (!server_id) {
      result->Error("INVALID_ARGUMENTS", "serverId must be a string");
      return;
    }
    
    auto server_it = servers_.find(*server_id);
    if (server_it == servers_.end()) {
      result->Error("SERVER_NOT_FOUND", "Server with given ID not found");
      return;
    }
    
    try {
      if (!server_it->second->WaitForConnection()) {
        DWORD error = GetLastError();
        std::string error_msg = "Failed to wait for connection on server '" + *server_id + "': " + GetWindowsErrorMessage(error) + " (Code: " + std::to_string(error) + ")";
        result->Error("LISTEN_FAILED", error_msg);
        return;
      }
      
      result->Success(flutter::EncodableValue(true));
    } catch (const std::exception& e) {
      result->Error("LISTEN_FAILED", e.what());
    }
  }
  else if (method == "sendMessageFromServer") {
    result->Error("NOT_IMPLEMENTED", "sendMessageFromServer not implemented yet");
  }
  else if (method == "sendMessageFromClient") {
    result->Error("NOT_IMPLEMENTED", "sendMessageFromClient not implemented yet");
  }
  else if (method == "closeServer") {
    const auto* arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!arguments) {
      result->Error("INVALID_ARGUMENTS", "Missing arguments for closeServer");
      return;
    }
    
    auto server_id_it = arguments->find(flutter::EncodableValue("serverId"));
    if (server_id_it == arguments->end()) {
      result->Error("INVALID_ARGUMENTS", "Missing serverId argument");
      return;
    }
    
    const auto* server_id = std::get_if<std::string>(&server_id_it->second);
    if (!server_id) {
      result->Error("INVALID_ARGUMENTS", "serverId must be a string");
      return;
    }
    
    auto server_it = servers_.find(*server_id);
    if (server_it == servers_.end()) {
      result->Error("SERVER_NOT_FOUND", "Server with given ID not found");
      return;
    }
    
    try {
      server_it->second->Close();
      servers_.erase(server_it);
      result->Success(flutter::EncodableValue(true));
    } catch (const std::exception& e) {
      result->Error("CLOSE_SERVER_FAILED", e.what());
    }
  }
  else if (method == "disconnect") {
    const auto* arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!arguments) {
      result->Error("INVALID_ARGUMENTS", "Missing arguments for disconnect");
      return;
    }
    
    auto client_id_it = arguments->find(flutter::EncodableValue("clientId"));
    if (client_id_it == arguments->end()) {
      result->Error("INVALID_ARGUMENTS", "Missing clientId argument");
      return;
    }
    
    const auto* client_id = std::get_if<std::string>(&client_id_it->second);
    if (!client_id) {
      result->Error("INVALID_ARGUMENTS", "clientId must be a string");
      return;
    }
    
    auto client_it = clients_.find(*client_id);
    if (client_it == clients_.end()) {
      result->Error("CLIENT_NOT_FOUND", "Client with given ID not found");
      return;
    }
    
    try {
      client_it->second->Disconnect();
      clients_.erase(client_it);
      result->Success(flutter::EncodableValue(true));
    } catch (const std::exception& e) {
      result->Error("DISCONNECT_FAILED", e.what());
    }
  }
  else {
    result->NotImplemented();
  }
}

}  // namespace flutter_ipc
