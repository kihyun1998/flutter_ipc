import 'dart:async';

import 'package:flutter/material.dart';
import 'package:flutter_ipc/flutter_ipc.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  String _status = 'Ready to test IPC';
  IpcServer? _server;
  IpcClient? _client;
  bool _serverCreated = false;
  bool _isListening = false;

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(title: const Text('Flutter IPC Example')),
        body: Padding(
          padding: const EdgeInsets.all(16.0),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              Text(
                'Status: $_status',
                style: const TextStyle(
                  fontSize: 16,
                  fontWeight: FontWeight.bold,
                ),
              ),
              const SizedBox(height: 20),
              ElevatedButton(
                onPressed: !_serverCreated ? _createServer : null,
                child: const Text('Create Server'),
              ),
              const SizedBox(height: 10),
              ElevatedButton(
                onPressed: _connectClient,
                child: const Text('Connect Client'),
              ),
              const SizedBox(height: 10),
              ElevatedButton(
                onPressed: _startListening,
                child: const Text('Start Server Listening'),
              ),
              const SizedBox(height: 10),
              ElevatedButton(
                onPressed: _sendMessage,
                child: const Text('Send Message from Client'),
              ),
              const SizedBox(height: 10),
              ElevatedButton(
                onPressed: _client != null ? _disconnectClient : null,
                child: const Text('Disconnect Client'),
              ),
              const SizedBox(height: 10),
              ElevatedButton(onPressed: _cleanup, child: const Text('Cleanup')),
            ],
          ),
        ),
      ),
    );
  }

  Future<void> _createServer() async {
    try {
      _server = await FlutterIpc.createServer('test_pipe');
      setState(() {
        _status = 'Server created successfully';
        _serverCreated = true;
      });
    } catch (e) {
      setState(() {
        _status = 'Server creation failed: $e';
        _serverCreated = false;
      });
    }
  }

  Future<void> _connectClient() async {
    try {
      _client = await FlutterIpc.connect('test_pipe');
      setState(() {
        _status = 'Client connected successfully';
      });
    } catch (e) {
      setState(() {
        _status = 'Client connection failed: $e';
      });
    }
  }

  Future<void> _startListening() async {
    try {
      await _server!.listen();
      setState(() {
        _status = 'Server is listening';
        _isListening = true;
      });
    } catch (e) {
      setState(() {
        _status = 'Server listen failed: $e';
        _isListening = false;
      });
    }
  }

  Future<void> _sendMessage() async {
    try {
      if (_client == null) {
        setState(() {
          _status = 'No client connected - please connect client first';
        });
        return;
      }
      
      await _client!.sendMessage('Hello from client!');
      setState(() {
        _status = 'Message sent from client';
      });
    } catch (e) {
      setState(() {
        _status = 'Send message failed: $e';
      });
    }
  }

  Future<void> _disconnectClient() async {
    try {
      if (_client != null) {
        await _client!.disconnect();
        _client = null;
      }
      setState(() {
        _status = 'Client disconnected successfully';
      });
    } catch (e) {
      setState(() {
        _status = 'Client disconnect failed: $e';
      });
    }
  }

  Future<void> _cleanup() async {
    try {
      if (_server != null) {
        await _server!.close();
        _server = null;
      }
      if (_client != null) {
        await _client!.disconnect();
        _client = null;
      }
      setState(() {
        _status = 'Cleaned up successfully';
        _serverCreated = false;
        _isListening = false;
      });
    } catch (e) {
      setState(() {
        _status = 'Cleanup failed: $e';
      });
    }
  }
}
