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

  Future<String?> getPlatformVersion() {
    throw UnimplementedError('platformVersion() has not been implemented.');
  }
}
