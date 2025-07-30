import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

import 'flutter_ipc_platform_interface.dart';

/// An implementation of [FlutterIpcPlatform] that uses method channels.
class MethodChannelFlutterIpc extends FlutterIpcPlatform {
  /// The method channel used to interact with the native platform.
  @visibleForTesting
  final methodChannel = const MethodChannel('flutter_ipc');

  @override
  Future<String?> getPlatformVersion() async {
    final version = await methodChannel.invokeMethod<String>('getPlatformVersion');
    return version;
  }
}
