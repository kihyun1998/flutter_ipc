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
                onPressed: _createServer,
                child: const Text('Create Server'),
              ),
              const SizedBox(height: 10),
              ElevatedButton(
                onPressed: _connectClient,
                child: const Text('Connect Client'),
              ),
              const SizedBox(height: 10),
              ElevatedButton(
                onPressed: _server != null ? _startListening : null,
                child: const Text('Start Server Listening'),
              ),
              const SizedBox(height: 10),
              ElevatedButton(
                onPressed: _client != null ? _sendMessage : null,
                child: const Text('Send Message from Client'),
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
      });
    } catch (e) {
      setState(() {
        _status = 'Server creation failed: $e';
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
      });
    } catch (e) {
      setState(() {
        _status = 'Server listen failed: $e';
      });
    }
  }

  Future<void> _sendMessage() async {
    try {
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
      });
    } catch (e) {
      setState(() {
        _status = 'Cleanup failed: $e';
      });
    }
  }
}
