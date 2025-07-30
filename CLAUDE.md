# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a Flutter plugin project called `flutter_ipc` that implements Inter-Process Communication (IPC) for Windows and macOS platforms. The plugin allows Flutter applications to communicate with other processes using:
- **Windows**: Named Pipes API
- **macOS**: XPC (Cross-Process Communication) / Unix Domain Sockets

## Development Commands

### Flutter Commands
```bash
# Get dependencies
flutter pub get

# Run the example app
cd example && flutter run

# Run tests
flutter test

# Analyze code
flutter analyze

# Check for linting issues
dart analyze
```

### Platform-Specific Building
```bash
# Build for Windows
flutter build windows

# Build for macOS  
flutter build macos

# Test integration tests
cd example && flutter test integration_test/
```

## Architecture Overview

### Plugin Structure
The project follows Flutter's federated plugin architecture:

- **`lib/flutter_ipc.dart`** - Main public API that developers will use
- **`lib/flutter_ipc_platform_interface.dart`** - Abstract platform interface defining the contract
- **`lib/flutter_ipc_method_channel.dart`** - Method channel implementation for platform communication

### Platform Implementations
- **Windows**: `windows/flutter_ipc_plugin.cpp` - C++ implementation using Windows Named Pipes
- **macOS**: `macos/Classes/FlutterIpcPlugin.swift` - Swift implementation using XPC

### Planned API Design (from requirements)
```dart
// Server/Client API structure
class FlutterIpc {
  static Future<IpcServer> createServer(String pipeName);
  static Future<IpcClient> connect(String pipeName);
}

class IpcServer {
  Future<void> listen();
  Future<void> sendMessage(String message);
  Stream<String> get messageStream;
  Future<void> close();
}

class IpcClient {
  Future<void> sendMessage(String message);
  Stream<String> get messageStream;
  Future<void> disconnect();
}
```

## Implementation Phases

The project follows a phased implementation approach defined in `plan.md`:

1. **Phase 1**: Basic plugin structure and Method Channel setup (✅ Complete)
2. **Phase 2**: Windows Named Pipe server implementation 
3. **Phase 3**: Windows Named Pipe client implementation
4. **Phase 4**: Message sending/receiving functionality
5. **Phase 5**: Example app and testing
6. **Phase 6**: Error handling and stability
7. **Phase 7**: Documentation and deployment

## Key Files and Their Purpose

- **`requirement.md`** - Korean language specification detailing all requirements and features
- **`plan.md`** - Detailed implementation plan with time estimates for each phase
- **`example/`** - Example Flutter app demonstrating plugin usage
- **`test/`** - Unit tests for the Dart layer
- **`integration_test/`** - Platform integration tests

## Native Code Integration

### Windows (C++)
- Uses Windows Named Pipes API (`CreateNamedPipe`, `ConnectNamedPipe`, `ReadFile`, `WriteFile`)
- Located in `windows/flutter_ipc_plugin.cpp`
- Handles both server and client modes

### macOS (Swift)  
- Will use XPC for cross-process communication
- Located in `macos/Classes/FlutterIpcPlugin.swift`
- Currently contains only boilerplate code

## Target Features

- Bidirectional message communication between processes
- Support for both server and client modes in the same app
- String and binary data transmission
- Multi-client server support
- Error handling and connection state management
- Cross-platform abstraction (Windows and macOS)

## Development Notes

- This is currently in early development - only basic plugin structure exists
- The plugin uses `plugin_platform_interface` for platform abstraction
- Method channels are used for Dart ↔ Native communication
- Event channels will be needed for stream-based message receiving
- Focus is on defensive IPC implementation, not network communication