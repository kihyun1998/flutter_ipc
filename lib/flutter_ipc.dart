
import 'flutter_ipc_platform_interface.dart';

class FlutterIpc {
  Future<String?> getPlatformVersion() {
    return FlutterIpcPlatform.instance.getPlatformVersion();
  }
}
