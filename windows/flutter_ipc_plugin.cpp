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
    : pipe_name_(pipe_name), pipe_handle_(INVALID_HANDLE_VALUE), is_connected_(false), state_(ServerState::CREATED) {
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

bool NamedPipeClient::SendMessage(const std::string& message) {
  if (pipe_handle_ == INVALID_HANDLE_VALUE || !is_connected_) {
    return false;
  }
  
  // Send message length first (4 bytes)
  DWORD message_length = static_cast<DWORD>(message.length());
  DWORD bytes_written = 0;
  
  BOOL success = WriteFile(
    pipe_handle_,
    &message_length,
    sizeof(message_length),
    &bytes_written,
    NULL
  );
  
  if (!success || bytes_written != sizeof(message_length)) {
    return false;
  }
  
  // Send message content
  success = WriteFile(
    pipe_handle_,
    message.c_str(),
    message_length,
    &bytes_written,
    NULL
  );
  
  return success && bytes_written == message_length;
}

bool NamedPipeServer::Create() {
  std::string full_pipe_name = "\\\\.\\pipe\\" + pipe_name_;
  
  // Try to create the pipe
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
  
  // If creation failed due to pipe busy, wait a bit and try again
  if (pipe_handle_ == INVALID_HANDLE_VALUE && GetLastError() == ERROR_PIPE_BUSY) {
    Sleep(100); // Wait 100ms for any cleanup to complete
    
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
  }
  
  return pipe_handle_ != INVALID_HANDLE_VALUE;
}

bool NamedPipeServer::WaitForConnection() {
  if (pipe_handle_ == INVALID_HANDLE_VALUE || state_ != ServerState::CREATED) {
    return false;
  }
  
  state_ = ServerState::LISTENING;
  
  BOOL connected = ConnectNamedPipe(pipe_handle_, &overlap_);
  
  if (connected) {
    is_connected_ = true;
    state_ = ServerState::CONNECTED;
    return true;
  }
  
  DWORD error = GetLastError();
  if (error == ERROR_PIPE_CONNECTED) {
    is_connected_ = true;
    state_ = ServerState::CONNECTED;
    return true;
  } else if (error == ERROR_IO_PENDING) {
    // Asynchronous operation - would need event handling for full implementation
    // For now, return true to indicate operation started
    return true;
  }
  
  state_ = ServerState::CREATED; // Restore to original state on failure
  return false;
}

bool NamedPipeServer::SendMessage(const std::string& message) {
  if (pipe_handle_ == INVALID_HANDLE_VALUE || !is_connected_) {
    return false;
  }
  
  // Send message length first (4 bytes)
  DWORD message_length = static_cast<DWORD>(message.length());
  DWORD bytes_written = 0;
  
  BOOL success = WriteFile(
    pipe_handle_,
    &message_length,
    sizeof(message_length),
    &bytes_written,
    NULL
  );
  
  if (!success || bytes_written != sizeof(message_length)) {
    return false;
  }
  
  // Send message content
  success = WriteFile(
    pipe_handle_,
    message.c_str(),
    message_length,
    &bytes_written,
    NULL
  );
  
  return success && bytes_written == message_length;
}

bool NamedPipeServer::ResetForNewConnection() {
  if (is_connected_) {
    // Flush any pending writes first
    FlushFileBuffers(pipe_handle_);
    
    // Disconnect existing client
    if (!DisconnectNamedPipe(pipe_handle_)) {
      DWORD error = GetLastError();
      // If no client was connected, that's fine - continue
      if (error != ERROR_NO_DATA) {
        return false;
      }
    }
    is_connected_ = false;
  }
  
  // Wait a brief moment for Windows to clean up the pipe state
  Sleep(50);
  
  // Reset state to allow new connections
  state_ = ServerState::CREATED;
  return true;
}

void NamedPipeServer::Close() {
  if (pipe_handle_ != INVALID_HANDLE_VALUE) {
    // Force disconnect if connected
    if (is_connected_) {
      DisconnectNamedPipe(pipe_handle_);
      is_connected_ = false;
    }
    
    // Cancel any pending I/O operations
    CancelIo(pipe_handle_);
    
    // Close the handle
    CloseHandle(pipe_handle_);
    pipe_handle_ = INVALID_HANDLE_VALUE;
  }
  state_ = ServerState::CLOSED;
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
  LPWSTR message_buffer = nullptr;
  size_t size = FormatMessageW(
    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL, error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    (LPWSTR)&message_buffer, 0, NULL
  );
  
  if (size == 0 || message_buffer == nullptr) {
    return "Unknown error";
  }
  
  // Convert from UTF-16 to UTF-8
  int utf8_length = WideCharToMultiByte(CP_UTF8, 0, message_buffer, -1, NULL, 0, NULL, NULL);
  if (utf8_length == 0) {
    LocalFree(message_buffer);
    return "Error message conversion failed";
  }
  
  std::string message(utf8_length - 1, '\0'); // -1 to exclude null terminator
  WideCharToMultiByte(CP_UTF8, 0, message_buffer, -1, &message[0], utf8_length, NULL, NULL);
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
      // Check if a server with the same pipe name already exists and close it
      for (auto it = servers_.begin(); it != servers_.end();) {
        if (it->second->GetPipeName() == *pipe_name) {
          it->second->Close();
          it = servers_.erase(it);
        } else {
          ++it;
        }
      }
      
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
      // Wait a brief moment to allow any previous disconnection to complete
      Sleep(100);
      
      // Try to find and reset any existing server with the same pipe name
      for (auto& server_pair : servers_) {
        if (server_pair.second->GetPipeName() == *pipe_name) {
          if (!server_pair.second->ResetForNewConnection()) {
            result->Error("SERVER_RESET_FAILED", "Failed to reset server for new connection");
            return;
          }
          break;
        }
      }
      
      std::string client_id = GenerateClientId();
      auto client = std::make_unique<NamedPipeClient>(*pipe_name);
      
      if (!client->Connect()) {
        DWORD error = GetLastError();
        std::string error_msg;
        
        if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND) {
          error_msg = "Named pipe '" + *pipe_name + "' does not exist. Make sure the server is created and listening.";
        } else if (error == ERROR_PIPE_BUSY) {
          error_msg = "Named pipe '" + *pipe_name + "' is busy. Another client may already be connected.";
        } else {
          error_msg = "Failed to connect to named pipe '" + *pipe_name + "': " + GetWindowsErrorMessage(error) + " (Code: " + std::to_string(error) + ")";
        }
        
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
    const auto* arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!arguments) {
      result->Error("INVALID_ARGUMENTS", "Missing arguments for sendMessageFromServer");
      return;
    }
    
    auto server_id_it = arguments->find(flutter::EncodableValue("serverId"));
    if (server_id_it == arguments->end()) {
      result->Error("INVALID_ARGUMENTS", "Missing serverId argument");
      return;
    }
    
    auto message_it = arguments->find(flutter::EncodableValue("message"));
    if (message_it == arguments->end()) {
      result->Error("INVALID_ARGUMENTS", "Missing message argument");
      return;
    }
    
    const auto* server_id = std::get_if<std::string>(&server_id_it->second);
    const auto* message = std::get_if<std::string>(&message_it->second);
    
    if (!server_id || !message) {
      result->Error("INVALID_ARGUMENTS", "serverId and message must be strings");
      return;
    }
    
    auto server_it = servers_.find(*server_id);
    if (server_it == servers_.end()) {
      result->Error("SERVER_NOT_FOUND", "Server with given ID not found");
      return;
    }
    
    if (!server_it->second->IsConnected()) {
      result->Error("SERVER_NOT_CONNECTED", "Server is not connected to any client");
      return;
    }
    
    try {
      if (!server_it->second->SendMessage(*message)) {
        DWORD error = GetLastError();
        std::string error_msg = "Failed to send message from server '" + *server_id + "': " + GetWindowsErrorMessage(error) + " (Code: " + std::to_string(error) + ")";
        result->Error("SEND_MESSAGE_FAILED", error_msg);
        return;
      }
      
      result->Success(flutter::EncodableValue(true));
    } catch (const std::exception& e) {
      result->Error("SEND_MESSAGE_FAILED", e.what());
    }
  }
  else if (method == "sendMessageFromClient") {
    const auto* arguments = std::get_if<flutter::EncodableMap>(method_call.arguments());
    if (!arguments) {
      result->Error("INVALID_ARGUMENTS", "Missing arguments for sendMessageFromClient");
      return;
    }
    
    auto client_id_it = arguments->find(flutter::EncodableValue("clientId"));
    if (client_id_it == arguments->end()) {
      result->Error("INVALID_ARGUMENTS", "Missing clientId argument");
      return;
    }
    
    auto message_it = arguments->find(flutter::EncodableValue("message"));
    if (message_it == arguments->end()) {
      result->Error("INVALID_ARGUMENTS", "Missing message argument");
      return;
    }
    
    const auto* client_id = std::get_if<std::string>(&client_id_it->second);
    const auto* message = std::get_if<std::string>(&message_it->second);
    
    if (!client_id || !message) {
      result->Error("INVALID_ARGUMENTS", "clientId and message must be strings");
      return;
    }
    
    auto client_it = clients_.find(*client_id);
    if (client_it == clients_.end()) {
      result->Error("CLIENT_NOT_FOUND", "Client with given ID not found");
      return;
    }
    
    if (!client_it->second->IsConnected()) {
      result->Error("CLIENT_NOT_CONNECTED", "Client is not connected to server");
      return;
    }
    
    try {
      if (!client_it->second->SendMessage(*message)) {
        DWORD error = GetLastError();
        std::string error_msg = "Failed to send message from client '" + *client_id + "': " + GetWindowsErrorMessage(error) + " (Code: " + std::to_string(error) + ")";
        result->Error("SEND_MESSAGE_FAILED", error_msg);
        return;
      }
      
      result->Success(flutter::EncodableValue(true));
    } catch (const std::exception& e) {
      result->Error("SEND_MESSAGE_FAILED", e.what());
    }
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
