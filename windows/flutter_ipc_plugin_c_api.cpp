#include "include/flutter_ipc/flutter_ipc_plugin_c_api.h"

#include <flutter/plugin_registrar_windows.h>

#include "flutter_ipc_plugin.h"

void FlutterIpcPluginCApiRegisterWithRegistrar(
    FlutterDesktopPluginRegistrarRef registrar) {
  flutter_ipc::FlutterIpcPlugin::RegisterWithRegistrar(
      flutter::PluginRegistrarManager::GetInstance()
          ->GetRegistrar<flutter::PluginRegistrarWindows>(registrar));
}
