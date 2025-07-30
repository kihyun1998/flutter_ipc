import 'package:plugin_platform_interface/plugin_platform_interface.dart';

import 'flutter_ipc_method_channel.dart';

abstract class FlutterIpcPlatform extends PlatformInterface {
  /// Constructs a FlutterIpcPlatform.
  FlutterIpcPlatform() : super(token: _token);

  static final Object _token = Object();

  static FlutterIpcPlatform _instance = MethodChannelFlutterIpc();

  /// The default instance of [FlutterIpcPlatform] to use.
  ///
  /// Defaults to [MethodChannelFlutterIpc].
  static FlutterIpcPlatform get instance => _instance;

  /// Platform-specific implementations should set this with their own
  /// platform-specific class that extends [FlutterIpcPlatform] when
  /// they register themselves.
  static set instance(FlutterIpcPlatform instance) {
    PlatformInterface.verifyToken(instance, _token);
    _instance = instance;
  }

  Future<String> createServer(String pipeName) {
    throw UnimplementedError('createServer() has not been implemented.');
  }

  Future<String> connect(String pipeName) {
    throw UnimplementedError('connect() has not been implemented.');
  }

  Future<void> listen(String serverId) {
    throw UnimplementedError('listen() has not been implemented.');
  }

  Future<void> sendMessageFromServer(String serverId, String message) {
    throw UnimplementedError('sendMessageFromServer() has not been implemented.');
  }

  Future<void> sendMessageFromClient(String clientId, String message) {
    throw UnimplementedError('sendMessageFromClient() has not been implemented.');
  }

  Stream<String> getServerMessageStream(String serverId) {
    throw UnimplementedError('getServerMessageStream() has not been implemented.');
  }

  Stream<String> getClientMessageStream(String clientId) {
    throw UnimplementedError('getClientMessageStream() has not been implemented.');
  }

  Future<void> closeServer(String serverId) {
    throw UnimplementedError('closeServer() has not been implemented.');
  }

  Future<void> disconnect(String clientId) {
    throw UnimplementedError('disconnect() has not been implemented.');
  }
}
