#ifndef FLUTTER_PLUGIN_FLUTTER_IPC_PLUGIN_H_
#define FLUTTER_PLUGIN_FLUTTER_IPC_PLUGIN_H_

#include <flutter/method_channel.h>
#include <flutter/plugin_registrar_windows.h>

#include <memory>
#include <string>
#include <map>
#include <windows.h>

namespace flutter_ipc {

enum class ServerState {
  CREATED,    // Server created but not listening yet
  LISTENING,  // Waiting for client connections
  CONNECTED,  // Client connected
  CLOSED      // Server closed
};

class NamedPipeServer {
 public:
  NamedPipeServer(const std::string& pipe_name);
  ~NamedPipeServer();

  bool Create();
  bool WaitForConnection();
  bool SendMessage(const std::string& message);
  bool ResetForNewConnection();
  void Close();
  
  bool IsValid() const { return pipe_handle_ != INVALID_HANDLE_VALUE; }
  bool IsListening() const { return state_ == ServerState::LISTENING || state_ == ServerState::CONNECTED; }
  bool IsConnected() const { return is_connected_; }
  ServerState GetState() const { return state_; }
  const std::string& GetPipeName() const { return pipe_name_; }

 private:
  std::string pipe_name_;
  HANDLE pipe_handle_;
  OVERLAPPED overlap_;
  bool is_connected_;
  ServerState state_;
};

class NamedPipeClient {
 public:
  NamedPipeClient(const std::string& pipe_name);
  ~NamedPipeClient();

  bool Connect();
  bool SendMessage(const std::string& message);
  void Disconnect();
  
  bool IsValid() const { return pipe_handle_ != INVALID_HANDLE_VALUE; }
  bool IsConnected() const { return is_connected_; }
  const std::string& GetPipeName() const { return pipe_name_; }

 private:
  std::string pipe_name_;
  HANDLE pipe_handle_;
  bool is_connected_;
};

class FlutterIpcPlugin : public flutter::Plugin {
 public:
  static void RegisterWithRegistrar(flutter::PluginRegistrarWindows *registrar);

  FlutterIpcPlugin();

  virtual ~FlutterIpcPlugin();

  // Disallow copy and assign.
  FlutterIpcPlugin(const FlutterIpcPlugin&) = delete;
  FlutterIpcPlugin& operator=(const FlutterIpcPlugin&) = delete;

  // Called when a method is called on this plugin's channel from Dart.
  void HandleMethodCall(
      const flutter::MethodCall<flutter::EncodableValue> &method_call,
      std::unique_ptr<flutter::MethodResult<flutter::EncodableValue>> result);

 private:
  std::map<std::string, std::unique_ptr<NamedPipeServer>> servers_;
  std::map<std::string, std::unique_ptr<NamedPipeClient>> clients_;
  std::string GenerateServerId();
  std::string GenerateClientId();
};

}  // namespace flutter_ipc

#endif  // FLUTTER_PLUGIN_FLUTTER_IPC_PLUGIN_H_
