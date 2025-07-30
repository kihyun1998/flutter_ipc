import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

import 'flutter_ipc_platform_interface.dart';

/// An implementation of [FlutterIpcPlatform] that uses method channels.
class MethodChannelFlutterIpc extends FlutterIpcPlatform {
  /// The method channel used to interact with the native platform.
  @visibleForTesting
  final methodChannel = const MethodChannel('flutter_ipc');

  /// Event channels for message streams
  final Map<String, EventChannel> _eventChannels = {};

  @override
  Future<String> createServer(String pipeName) async {
    final serverId = await methodChannel.invokeMethod<String>('createServer', {
      'pipeName': pipeName,
    });
    return serverId!;
  }

  @override
  Future<String> connect(String pipeName) async {
    final clientId = await methodChannel.invokeMethod<String>('connect', {
      'pipeName': pipeName,
    });
    return clientId!;
  }

  @override
  Future<void> listen(String serverId) async {
    return methodChannel.invokeMethod<void>('listen', {
      'serverId': serverId,
    });
  }

  @override
  Future<void> sendMessageFromServer(String serverId, String message) async {
    return methodChannel.invokeMethod<void>('sendMessageFromServer', {
      'serverId': serverId,
      'message': message,
    });
  }

  @override
  Future<void> sendMessageFromClient(String clientId, String message) async {
    return methodChannel.invokeMethod<void>('sendMessageFromClient', {
      'clientId': clientId,
      'message': message,
    });
  }

  @override
  Stream<String> getServerMessageStream(String serverId) {
    final eventChannel = _getOrCreateEventChannel('server_$serverId');
    return eventChannel.receiveBroadcastStream().cast<String>();
  }

  @override
  Stream<String> getClientMessageStream(String clientId) {
    final eventChannel = _getOrCreateEventChannel('client_$clientId');
    return eventChannel.receiveBroadcastStream().cast<String>();
  }

  @override
  Future<void> closeServer(String serverId) async {
    return methodChannel.invokeMethod<void>('closeServer', {
      'serverId': serverId,
    });
  }

  @override
  Future<void> disconnect(String clientId) async {
    return methodChannel.invokeMethod<void>('disconnect', {
      'clientId': clientId,
    });
  }

  EventChannel _getOrCreateEventChannel(String channelName) {
    return _eventChannels.putIfAbsent(
      channelName,
      () => EventChannel('flutter_ipc_stream_$channelName'),
    );
  }
}
