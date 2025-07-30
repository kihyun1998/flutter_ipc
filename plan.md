# Flutter IPC 간단 점진적 구현 계획

## 🎯 목표: 작은 단계로 완전한 기능 구현

## Phase 1: 프로젝트 기본 구조 (30분)

### 목표
Flutter 플러그인 기본 구조 및 Method Channel 설정

### 작업 내용
1. **Flutter 플러그인 생성**
   ```bash
   flutter create --template=plugin --platforms=windows flutter_ipc
   ```

2. **파일 구조 확인 및 기본 설정**
   - `lib/flutter_ipc.dart` - 메인 API
   - `lib/flutter_ipc_platform_interface.dart` - 플랫폼 인터페이스
   - `lib/flutter_ipc_method_channel.dart` - Method Channel 구현
   - `windows/flutter_ipc_plugin.cpp` - Windows 네이티브 코드

### 수정/추가 파일
- ✏️ `lib/flutter_ipc.dart` - 기본 API 정의
- ✏️ `lib/flutter_ipc_platform_interface.dart` - 인터페이스 수정
- ✏️ `lib/flutter_ipc_method_channel.dart` - Method Channel 설정

### 완료 기준
- 플러그인 프로젝트 생성 완료
- 예제 앱에서 플러그인 import 가능
- Method Channel 연결 테스트 성공

---

## Phase 2: Windows 기본 서버 구현 (2시간)

### 목표
Windows Named Pipe 서버 생성 및 클라이언트 연결 대기

### 작업 내용
1. **Dart API 확장**
   ```dart
   // lib/flutter_ipc.dart
   class FlutterIpc {
     static Future<IpcServer> createServer(String pipeName) async { ... }
   }
   
   class IpcServer {
     Future<void> listen() async { ... }
     Future<void> close() async { ... }
   }
   ```

2. **Windows C++ 서버 구현**
   ```cpp
   // windows/flutter_ipc_plugin.cpp
   class NamedPipeServer {
     HANDLE CreateServer(const std::string& pipeName);
     bool WaitForConnection(); // ConnectNamedPipe()
   };
   ```

### 수정/추가 파일
- ✏️ `lib/flutter_ipc.dart` - IpcServer 클래스 추가
- ✏️ `lib/flutter_ipc_platform_interface.dart` - createServer 메서드 추가
- ✏️ `lib/flutter_ipc_method_channel.dart` - createServer 구현
- ✏️ `windows/flutter_ipc_plugin.cpp` - NamedPipeServer 클래스 추가
- ✏️ `windows/flutter_ipc_plugin.h` - 헤더 파일 수정

### 완료 기준
- Named Pipe 서버 생성 성공
- 클라이언트 연결 대기 가능
- 예제 앱에서 서버 실행 확인

---

## Phase 3: Windows 기본 클라이언트 구현 (1시간)

### 목표
Windows Named Pipe 클라이언트 연결 구현

### 작업 내용
1. **Dart API 확장**
   ```dart
   // lib/flutter_ipc.dart
   class FlutterIpc {
     static Future<IpcClient> connect(String pipeName) async { ... }
   }
   
   class IpcClient {
     Future<void> disconnect() async { ... }
   }
   ```

2. **Windows C++ 클라이언트 구현**
   ```cpp
   // windows/flutter_ipc_plugin.cpp
   class NamedPipeClient {
     HANDLE ConnectToServer(const std::string& pipeName); // CreateFile()
   };
   ```

### 수정/추가 파일
- ✏️ `lib/flutter_ipc.dart` - IpcClient 클래스 추가
- ✏️ `lib/flutter_ipc_platform_interface.dart` - connect 메서드 추가
- ✏️ `lib/flutter_ipc_method_channel.dart` - connect 구현
- ✏️ `windows/flutter_ipc_plugin.cpp` - NamedPipeClient 클래스 추가

### 완료 기준
- Named Pipe 클라이언트 연결 성공
- 서버-클라이언트 연결 확인
- 예제 앱에서 연결 테스트 성공

---

## Phase 4: 기본 메시지 송수신 (2시간)

### 목표
문자열 메시지 1:1 양방향 송수신

### 작업 내용
1. **Dart API 메시지 기능**
   ```dart
   class IpcServer {
     Future<void> sendMessage(String message) async { ... }
     Stream<String> get messageStream { ... }
   }
   
   class IpcClient {
     Future<void> sendMessage(String message) async { ... }
     Stream<String> get messageStream { ... }
   }
   ```

2. **Windows C++ 메시지 처리**
   ```cpp
   // ReadFile, WriteFile을 이용한 메시지 송수신
   bool SendMessage(HANDLE pipe, const std::string& message);
   std::string ReceiveMessage(HANDLE pipe);
   ```

### 수정/추가 파일
- ✏️ `lib/flutter_ipc.dart` - sendMessage, messageStream 추가
- ✏️ `lib/flutter_ipc_platform_interface.dart` - 메시지 메서드 추가
- ✏️ `lib/flutter_ipc_method_channel.dart` - Event Channel 설정
- ✏️ `windows/flutter_ipc_plugin.cpp` - 메시지 송수신 구현

### 완료 기준
- 서버 → 클라이언트 메시지 전송 성공
- 클라이언트 → 서버 메시지 전송 성공
- Stream으로 실시간 메시지 수신 가능

---

## Phase 5: 예제 앱 및 테스트 (1시간)

### 목표
완전한 동작 예제 및 자체 테스트

### 작업 내용
1. **예제 앱 UI**
   - 서버/클라이언트 모드 선택 버튼
   - 메시지 입력/전송 UI
   - 수신 메시지 표시

2. **자체 테스트 (같은 앱 내 서버+클라이언트)**
   ```dart
   // 하나의 앱에서 서버와 클라이언트 동시 실행
   final server = await FlutterIpc.createServer('test_pipe');
   final client = await FlutterIpc.connect('test_pipe');
   ```

### 수정/추가 파일
- ✏️ `example/lib/main.dart` - 완전한 예제 UI
- ➕ `example/lib/server_page.dart` - 서버 모드 페이지
- ➕ `example/lib/client_page.dart` - 클라이언트 모드 페이지
- ➕ `test/flutter_ipc_test.dart` - 기본 테스트 코드

### 완료 기준
- 예제 앱에서 서버/클라이언트 모두 동작
- 같은 앱 내에서 자기 자신과 통신 성공
- 기본 테스트 통과

---

## Phase 6: 에러 처리 및 안정성 (1시간)

### 목표
연결 실패, 통신 오류 등 예외 상황 처리

### 작업 내용
1. **Dart 예외 처리**
   ```dart
   try {
     await FlutterIpc.createServer('test');
   } catch (e) {
     print('서버 생성 실패: $e');
   }
   ```

2. **Windows 에러 코드 변환**
   ```cpp
   // GetLastError()를 Dart 예외로 변환
   void HandleWindowsError(const std::string& operation);
   ```

### 수정/추가 파일
- ✏️ `lib/flutter_ipc.dart` - 예외 클래스 정의
- ✏️ `windows/flutter_ipc_plugin.cpp` - 에러 처리 추가

### 완료 기준
- 연결 실패 시 명확한 에러 메시지
- 잘못된 파이프 이름 처리
- 앱 종료 시 안전한 정리

---

## 🎉 Phase 7: 문서 작성 및 배포 준비 (30분)

### 목표
README 작성 및 pub.dev 배포 준비

### 작업 내용
- README.md 작성 (설치, 사용법, 예제)
- CHANGELOG.md 작성
- pubspec.yaml 버전 및 설명 정리

### 수정/추가 파일
- ✏️ `README.md` - 완전한 문서
- ✏️ `CHANGELOG.md` - 버전 히스토리
- ✏️ `pubspec.yaml` - 메타데이터 정리

---

## 📋 각 Phase별 예상 소요 시간

| Phase | 작업 내용 | 예상 시간 | 누적 시간 |
|-------|-----------|-----------|-----------|
| 1 | 프로젝트 구조 | 30분 | 30분 |
| 2 | Windows 서버 | 2시간 | 2.5시간 |
| 3 | Windows 클라이언트 | 1시간 | 3.5시간 |
| 4 | 메시지 송수신 | 2시간 | 5.5시간 |
| 5 | 예제 앱 | 1시간 | 6.5시간 |
| 6 | 에러 처리 | 1시간 | 7.5시간 |
| 7 | 문서화 | 30분 | 8시간 |

**총 예상 시간: 8시간** (1-2일 작업)

---

## 🚀 확장 계획 (Phase 8+)

### 나중에 추가할 기능들
- JSON 메시지 지원
- macOS XPC 구현
- 멀티 클라이언트 지원
- 바이너리 데이터 전송
- 보안 및 권한 관리

각 Phase가 완료되면 **완전히 동작하는 기능**을 제공하여 점진적으로 발전시킬 수 있습니다!