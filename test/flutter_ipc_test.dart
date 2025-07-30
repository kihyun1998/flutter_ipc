import 'package:flutter_test/flutter_test.dart';
import 'package:flutter_ipc/flutter_ipc.dart';
import 'package:flutter_ipc/flutter_ipc_platform_interface.dart';
import 'package:flutter_ipc/flutter_ipc_method_channel.dart';
import 'package:plugin_platform_interface/plugin_platform_interface.dart';

class MockFlutterIpcPlatform
    with MockPlatformInterfaceMixin
    implements FlutterIpcPlatform {

  @override
  Future<String?> getPlatformVersion() => Future.value('42');
}

void main() {
  final FlutterIpcPlatform initialPlatform = FlutterIpcPlatform.instance;

  test('$MethodChannelFlutterIpc is the default instance', () {
    expect(initialPlatform, isInstanceOf<MethodChannelFlutterIpc>());
  });

  test('getPlatformVersion', () async {
    FlutterIpc flutterIpcPlugin = FlutterIpc();
    MockFlutterIpcPlatform fakePlatform = MockFlutterIpcPlatform();
    FlutterIpcPlatform.instance = fakePlatform;

    expect(await flutterIpcPlugin.getPlatformVersion(), '42');
  });
}
