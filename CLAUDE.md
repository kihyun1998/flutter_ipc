# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a Flutter plugin project called `flutter_ipc` that implements Inter-Process Communication (IPC) for Windows and macOS platforms. The plugin allows Flutter applications to communicate with other processes using:
- **Windows**: Named Pipes API
- **macOS**: XPC (Cross-Process Communication) / Unix Domain Sockets

## Development Commands

### Essential Commands
```bash
# Get dependencies
flutter pub get

# Run the example app (Windows only currently)
cd example && flutter run -d windows

# Run tests
flutter test

# Analyze code and check linting
flutter analyze
dart analyze

# Build for specific platforms
flutter build windows
flutter build macos
```

### Testing Commands
```bash
# Run unit tests for Dart layer
flutter test

# Run integration tests (when available)
cd example && flutter test integration_test/

# Test single file
flutter test test/flutter_ipc_test.dart
```

## Architecture Overview

### Current Implementation Status
The project is in **Phase 2** development - Windows Named Pipe server implementation is partially complete. The basic plugin structure exists with:

- ✅ **Phase 1**: Flutter plugin structure and Method Channel setup
- 🚧 **Phase 2**: Windows Named Pipe server (partial - NamedPipeServer class exists but methods not fully implemented)
- ⏳ **Phase 3+**: Client implementation, message passing, error handling pending

### Plugin Architecture
Uses Flutter's federated plugin architecture with three main layers:

- **`lib/flutter_ipc.dart`** - Public API with IpcServer and IpcClient classes
- **`lib/flutter_ipc_platform_interface.dart`** - Abstract platform interface 
- **`lib/flutter_ipc_method_channel.dart`** - Method channel bridge to native code

### Native Implementation Structure

#### Windows (C++)
- **Location**: `windows/flutter_ipc_plugin.cpp` and `windows/flutter_ipc_plugin.h`
- **Key Classes**:
  - `NamedPipeServer` - Handles Named Pipe creation and connection management
  - `FlutterIpcPlugin` - Main plugin class handling method calls from Dart
- **Current State**: Basic NamedPipeServer structure exists, but method handlers return "NOT_IMPLEMENTED" errors
- **Named Pipe Path**: Uses `\\\\.\\pipe\\{pipeName}` format

#### macOS (Swift)
- **Location**: `macos/Classes/FlutterIpcPlugin.swift`
- **Status**: Contains only boilerplate code, XPC implementation pending

### API Design (Already Implemented)
The Dart API follows the planned design from requirements:

```dart
// Main entry points
class FlutterIpc {
  static Future<IpcServer> createServer(String pipeName);
  static Future<IpcClient> connect(String pipeName);
}

// Server instance (uses internal _serverId for tracking)
class IpcServer {
  Future<void> listen();
  Future<void> sendMessage(String message);
  Stream<String> get messageStream;
  Future<void> close();
}

// Client instance (uses internal _clientId for tracking)  
class IpcClient {
  Future<void> sendMessage(String message);
  Stream<String> get messageStream;
  Future<void> disconnect();
}
```

## Implementation Phases (from plan.md)

**Current Status**: Phase 2 in progress

1. **Phase 1**: Basic plugin structure ✅
2. **Phase 2**: Windows Named Pipe server 🚧 (NamedPipeServer class exists)
3. **Phase 3**: Windows Named Pipe client ⏳
4. **Phase 4**: Message sending/receiving ⏳
5. **Phase 5**: Example app and testing ⏳
6. **Phase 6**: Error handling and stability ⏳
7. **Phase 7**: Documentation and deployment ⏳

## Key Development Context

### Current Development State
- **Windows plugin structure**: Complete but methods return "NOT_IMPLEMENTED"
- **Example app**: Complete UI with test buttons for server/client operations
- **Native pipe creation**: NamedPipeServer.Create() method exists and handles `CreateNamedPipeA` API
- **Connection handling**: NamedPipeServer.WaitForConnection() partially implemented with overlapped I/O structure

### Next Implementation Steps
1. Complete Windows C++ method handlers in HandleMethodCall()
2. Implement server-client ID tracking system  
3. Add Event Channel for message streams
4. Implement ReadFile/WriteFile for message passing

### Critical Files for Development
- **`requirement.md`** - Korean specification with detailed API requirements
- **`plan.md`** - 8-hour phased implementation timeline
- **`example/lib/main.dart`** - Complete test UI ready for functionality
- **`windows/flutter_ipc_plugin.cpp`** - Core C++ implementation (methods need completion)

### Development Notes
- Plugin uses `plugin_platform_interface` pattern for cross-platform abstraction
- Internal ID system (_serverId, _clientId) tracks multiple server/client instances
- Event channels will be required for Stream<String> messageStream implementation
- Focus on local machine IPC only - no network communication
- Current Flutter version: 3.32.7, Dart 3.8.1

### Development Guidelines
- **Comments**: Always write comments in English to avoid encoding issues with MSVC compiler
- **Encoding**: Use UTF-8 encoding for all source files but avoid non-ASCII characters in comments
- **Platform compatibility**: Consider Windows CP949 encoding limitations when writing C++ code