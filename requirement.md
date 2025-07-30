# Flutter IPC 플러그인 요구사항 명세서

## 1. 프로젝트 개요
**프로젝트명**: flutter_ipc  
**목적**: Windows와 macOS에서 IPC(Inter-Process Communication) 통신을 Dart 코드로 쉽게 사용할 수 있도록 하는 Flutter 플러그인  
**대상 플랫폼**: Windows, macOS  

## 2. 기능 요구사항

### 2.1 핵심 기능
- **Named Pipe 통신 지원**
  - Windows: Windows Named Pipes API 활용
  - macOS: Unix Domain Sockets 활용
- **메시지 송수신**
  - 문자열 데이터 송수신
  - 바이너리 데이터 송수신
- **서버/클라이언트 모드 지원**
  - IPC 서버 생성 및 관리
  - IPC 클라이언트 연결 및 통신

### 2.2 Dart API 설계
```dart
// 기본 사용 예시 - 서버/클라이언트 모두 지원
class FlutterIpc {
  // 서버 생성 (다른 앱이 연결할 수 있도록)
  static Future<IpcServer> createServer(String pipeName);
  
  // 클라이언트 연결 (다른 앱의 서버에 연결)
  static Future<IpcClient> connect(String pipeName);
}

class IpcServer {
  // 클라이언트 연결 대기
  Future<void> listen();
  
  // 연결된 클라이언트에게 메시지 송신
  Future<void> sendMessage(String message);
  
  // 클라이언트로부터 메시지 수신 스트림
  Stream<String> get messageStream;
  
  // 서버 종료
  Future<void> close();
}

class IpcClient {
  // 서버에게 메시지 송신
  Future<void> sendMessage(String message);
  
  // 서버로부터 메시지 수신 스트림  
  Stream<String> get messageStream;
  
  // 연결 해제
  Future<void> disconnect();
}
```

### 2.3 서버/클라이언트 모드 지원
- **서버 모드**: Named Pipe 생성하여 다른 앱의 연결 대기
- **클라이언트 모드**: 기존 Named Pipe에 연결하여 통신
- **양방향 통신**: 서버↔클라이언트 모두 메시지 송수신 가능
- **같은 앱 내 통신**: 하나의 앱에서 서버와 클라이언트 동시 실행 가능 (테스트 및 내부 컴포넌트 통신용)
- **연결 상태 관리**
  - 연결/해제 상태 모니터링
  - 재연결 기능
- **에러 핸들링**
  - 연결 실패 처리
  - 통신 오류 처리
- **멀티 클라이언트 지원**
  - 하나의 서버에 여러 클라이언트 연결 지원

## 3. 비기능 요구사항

### 3.1 성능
- 메시지 전송 지연시간 최소화
- 대용량 데이터 전송 지원 (최대 1MB)

### 3.2 안정성
- 프로세스 종료 시 안전한 정리
- 메모리 누수 방지

### 3.3 사용성
- 간단하고 직관적인 API 제공
- 충분한 문서화 및 예제 제공

## 4. 기술 스택

### 4.1 Flutter 플러그인 구조
- **Dart 레이어**: Flutter 앱과의 인터페이스
- **플랫폼 채널**: Method Channel 사용
- **네이티브 레이어**:
  - Windows: C++ 구현 (Named Pipes)
  - macOS: Objective-C/Swift 구현 (XPC)

### 4.2 IPC 구현 방식
- **Windows**: Named Pipes (CreateNamedPipe, ConnectNamedPipe)
- **macOS**: XPC (Cross-Process Communication)

## 5. 제약사항
- Flutter 2.0 이상 지원
- Windows 10 이상, macOS 10.14 이상 지원
- 동일 머신 내 프로세스 간 통신만 지원 (네트워크 통신 제외)

## 6. 마일스톤

### Phase 1: 기본 구조 구축
- Flutter 플러그인 프로젝트 생성
- 기본 Method Channel 설정
- Windows/macOS 플랫폼 코드 구조 생성

### Phase 2: Windows 구현
- Windows Named Pipes 구현
- 서버/클라이언트 기본 통신 구현
- 문자열 메시지 송수신 구현

### Phase 3: macOS 구현  
- XPC (Cross-Process Communication) 구현
- 서버/클라이언트 기본 통신 구현
- 문자열 메시지 송수신 구현

### Phase 4: 고급 기능
- 에러 핸들링 강화
- 멀티 클라이언트 지원
- 바이너리 데이터 지원

### Phase 5: 완성 및 배포
- 테스트 코드 작성
- 문서화
- pub.dev 배포

## 7. 성공 기준
- Windows와 macOS에서 안정적인 IPC 통신 동작
- 간단한 API로 쉬운 사용성 제공
- 메모리 누수 없는 안정적인 동작
- 완전한 문서화 및 예제 제공