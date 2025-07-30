
import 'flutter_ipc_platform_interface.dart';

class FlutterIpc {
  static Future<IpcServer> createServer(String pipeName) async {
    final serverId = await FlutterIpcPlatform.instance.createServer(pipeName);
    return IpcServer._(serverId);
  }

  static Future<IpcClient> connect(String pipeName) async {
    final clientId = await FlutterIpcPlatform.instance.connect(pipeName);
    return IpcClient._(clientId);
  }
}

class IpcServer {
  final String _serverId;

  IpcServer._(this._serverId);

  Future<void> listen() async {
    return FlutterIpcPlatform.instance.listen(_serverId);
  }

  Future<void> sendMessage(String message) async {
    return FlutterIpcPlatform.instance.sendMessageFromServer(_serverId, message);
  }

  Stream<String> get messageStream {
    return FlutterIpcPlatform.instance.getServerMessageStream(_serverId);
  }

  Future<void> close() async {
    return FlutterIpcPlatform.instance.closeServer(_serverId);
  }
}

class IpcClient {
  final String _clientId;

  IpcClient._(this._clientId);

  Future<void> sendMessage(String message) async {
    return FlutterIpcPlatform.instance.sendMessageFromClient(_clientId, message);
  }

  Stream<String> get messageStream {
    return FlutterIpcPlatform.instance.getClientMessageStream(_clientId);
  }

  Future<void> disconnect() async {
    return FlutterIpcPlatform.instance.disconnect(_clientId);
  }
}
