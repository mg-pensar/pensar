# CommandQueue Worker System - Phased Implementation Plan

## Overview
Transform CommandQueue from a simple command executor into a distributed publish/subscribe system using ASIO, supporting command execution across same-process, inter-process, LAN, and remote workers.

## Architecture Principles

### Core Concepts
- **Worker**: Command executor that can run in different execution contexts
- **CommandQueue**: Server-like component managing command distribution and worker subscriptions
- **Dispatch Mechanism**: How commands are sent to workers (template parameter, defaults to same-process)
- **Subscription**: Workers subscribe to all commands or specific command types (variadic template)

### File Organization
```
cpp/src/
  command.hpp              # Public API header
  command/
    worker.hpp             # Worker base class and types
    dispatch.hpp           # Dispatch mechanism types/traits
    queue_impl.hpp         # CommandQueue implementation
    local_worker.hpp       # Same-process worker (Phase 1)
    ipc_worker.hpp         # Inter-process worker (Phase 2)
    lan_worker.hpp         # LAN worker (Phase 3)
    remote_worker.hpp      # Remote worker (Phase 4)
    network_common.hpp     # Common networking utilities (Phase 3+)
```

---

## Phase 1: Same-Process Workers

### Objectives
- Establish core Worker and CommandQueue architecture
- Implement ASIO-based same-process async execution
- Create type-safe subscription mechanism
- Support subscribe to all commands or specific command types

### Implementation Tasks

#### 1.1 Core Types & Interfaces
- [ ] Define `WorkerBase` abstract class
  - `virtual void handle_command(Command::Ptr cmd) = 0`
  - `virtual bool can_handle(const Command& cmd) const = 0`
  - Worker ID and state management
- [ ] Define `DispatchMechanism` enum class
  ```cpp
  enum class DispatchMechanism {
      SameProcess,
      InterProcess,
      LAN,
      Remote
  };
  ```
- [ ] Create `Subscription` concept/trait to validate command types

#### 1.2 CommandQueue Core (ASIO-based)
- [ ] Add ASIO `io_context` member to CommandQueue
- [ ] Add worker thread pool (configurable size)
- [ ] Implement worker registry: `std::map<WorkerId, WorkerBase::Ptr>`
- [ ] Add subscription map: `std::map<TypeId, std::vector<WorkerId>>`
- [ ] Implement `run()` method to start io_context processing
- [ ] Implement `stop()` method for graceful shutdown

#### 1.3 Subscription API
- [ ] `subscribe(Worker::Ptr worker)` - subscribe to all commands
  ```cpp
  template<typename W>
  requires std::derived_from<W, WorkerBase>
  WorkerId subscribe(std::shared_ptr<W> worker);
  ```
- [ ] `subscribe<Cmd1, Cmd2, ...>(Worker::Ptr worker)` - type-specific subscription
  ```cpp
  template<typename W, typename... CmdTypes>
  requires std::derived_from<W, WorkerBase> && (std::derived_from<CmdTypes, Command> && ...)
  WorkerId subscribe(std::shared_ptr<W> worker);
  ```
- [ ] `unsubscribe(WorkerId id)` - remove worker subscription

#### 1.4 Command Dispatch
- [ ] Implement `enqueue(Command::Ptr cmd)` using `io_context.post()`
  - Find matching workers from subscription map
  - Post execution to io_context for each worker
  - Thread-safe command queueing
- [ ] Add command routing logic based on command type
- [ ] Implement priority queue support (optional)

#### 1.5 LocalWorker Implementation
- [ ] Create `LocalWorker` template class
  ```cpp
  template<typename... CmdTypes>
  class LocalWorker : public WorkerBase {
      std::function<void(Command::Ptr)> handler_;
  public:
      LocalWorker(std::function<void(Command::Ptr)> handler);
      void handle_command(Command::Ptr cmd) override;
      bool can_handle(const Command& cmd) const override;
  };
  ```
- [ ] Implement type checking using RTTI or type registry
- [ ] Add error handling and exception safety

#### 1.6 Testing & Validation
- [ ] Unit tests for subscription mechanism
- [ ] Test multiple workers handling same command type
- [ ] Test type-specific subscriptions
- [ ] Concurrent execution tests
- [ ] Benchmark same-process worker performance

### Deliverables (Phase 1)
- Working CommandQueue with ASIO-based async execution
- LocalWorker for same-process execution
- Type-safe subscription API
- Comprehensive test suite
- Performance baseline metrics

---

## Phase 2: Inter-Process Workers

### Objectives
- Enable command execution across processes on same machine
- Choose efficient IPC mechanism (shared memory, named pipes, domain sockets)
- Maintain type safety across process boundaries
- Implement serialization/deserialization for commands

### Implementation Tasks

#### 2.1 IPC Mechanism Selection
- [ ] Evaluate IPC options:
  - Boost.Interprocess (shared memory + synchronization)
  - Named pipes (Windows) / Unix domain sockets (Unix/Linux/macOS)
  - Memory-mapped files
- [ ] **Decision**: Use Unix domain sockets (cross-platform via Boost.ASIO)
  - Low latency, efficient, ASIO support
  - Fallback to named pipes on Windows if needed

#### 2.2 Command Serialization
- [ ] Extend Command with serialization interface
  ```cpp
  virtual void serialize(std::ostream& os) const = 0;
  virtual void deserialize(std::istream& is) = 0;
  ```
- [ ] Use existing `binary_write`/`binary_read` from Object hierarchy
- [ ] Implement command factory for deserialization by type
- [ ] Add command type registry for dynamic instantiation

#### 2.3 IPC Transport Layer
- [ ] Create `IPCChannel` class using ASIO local sockets
  ```cpp
  class IPCChannel {
      asio::local::stream_protocol::socket socket_;
  public:
      void send_command(const Command& cmd);
      Command::Ptr receive_command();
  };
  ```
- [ ] Implement message framing (length prefix + payload)
- [ ] Add error handling for disconnections

#### 2.4 IPCWorker Implementation
- [ ] Create `IPCWorker` class
  - Runs in separate process
  - Connects to CommandQueue via IPC socket
  - Receives serialized commands, executes, optionally sends results back
- [ ] Implement worker process management
  - Spawn/fork worker processes
  - Monitor worker health (heartbeat)
  - Restart crashed workers

#### 2.5 CommandQueue IPC Server
- [ ] Add ASIO local acceptor to CommandQueue
- [ ] Listen on well-known socket path (e.g., `/tmp/cpplib_cmdqueue.sock`)
- [ ] Accept worker connections
- [ ] Dispatch commands to IPC workers via serialization
- [ ] Handle worker disconnections gracefully

#### 2.6 Testing & Validation
- [ ] Multi-process integration tests
- [ ] Serialization round-trip tests
- [ ] Worker crash/recovery tests
- [ ] Performance comparison vs same-process
- [ ] Memory leak detection across processes

### Deliverables (Phase 2)
- Working IPC worker implementation
- Command serialization framework
- Multi-process test harness
- Process management utilities
- Updated performance metrics

---

## Phase 3: LAN Workers

### Objectives
- Enable command execution across machines on local network
- Implement port range management and discovery
- Use TCP for LAN communication
- Maintain low latency for LAN scenarios

### Implementation Tasks

#### 3.1 Network Configuration
- [ ] Define port range configuration
  ```cpp
  struct NetworkConfig {
      uint16_t port_range_start = 50000;
      uint16_t port_range_end = 50100;
      std::chrono::seconds heartbeat_interval{5};
      std::chrono::seconds worker_timeout{15};
  };
  ```
- [ ] Implement port availability testing
  ```cpp
  std::vector<uint16_t> test_available_ports(const NetworkConfig& cfg);
  ```
- [ ] Listen on all available ports in range

#### 3.2 TCP Transport Layer
- [ ] Create `TCPChannel` class using ASIO TCP sockets
  ```cpp
  class TCPChannel {
      asio::ip::tcp::socket socket_;
  public:
      void send_command(const Command& cmd);
      Command::Ptr receive_command();
      void send_heartbeat();
  };
  ```
- [ ] Implement message framing with checksums (detect corruption)
- [ ] Add compression for large commands (optional)

#### 3.3 Worker Discovery & Registration
- [ ] Implement UDP-based discovery protocol (optional)
  - Workers broadcast presence on LAN
  - CommandQueue responds with available ports
- [ ] Alternative: Static configuration with IP:port pairs
- [ ] Worker registration handshake
  - Worker connects to CommandQueue port
  - Sends capabilities (supported command types)
  - Receives worker ID

#### 3.4 LANWorker Implementation
- [ ] Create `LANWorker` class
  - Discovers CommandQueue on network
  - Connects via TCP to assigned port
  - Receives commands, executes, sends results
- [ ] Implement reconnection logic (network interruptions)
- [ ] Add bandwidth throttling (optional)

#### 3.5 CommandQueue LAN Server
- [ ] Add ASIO TCP acceptors for port range
- [ ] Manage multiple TCP connections concurrently
- [ ] Implement worker heartbeat monitoring
  - Detect and remove dead workers
  - Requeue commands from failed workers
- [ ] Load balancing across LAN workers
  - Round-robin or least-loaded strategy

#### 3.6 Security (Basic)
- [ ] Implement simple authentication token
- [ ] Command signing/verification (HMAC)
- [ ] IP whitelist/blacklist
- [ ] Note: Full encryption deferred to Phase 4

#### 3.7 Testing & Validation
- [ ] Multi-machine integration tests
- [ ] Network failure simulation (disconnect, latency)
- [ ] Load balancing tests
- [ ] Port exhaustion scenarios
- [ ] Latency/throughput benchmarks

### Deliverables (Phase 3)
- Working LAN worker implementation
- Port management and discovery
- Basic security measures
- Network resilience features
- LAN performance benchmarks

---

## Phase 4: Remote Workers

### Objectives
- Enable secure command execution over public internet
- Implement efficient binary protocol (first choice)
- Fallback to HTTPS/WebSocket for firewall traversal
- Add encryption and strong authentication

### Implementation Tasks

#### 4.1 Protocol Selection
- [ ] **Primary**: ASIO-based custom binary protocol over TLS
  - Uses ASIO SSL support
  - Efficient binary serialization
  - Lower overhead than HTTP
- [ ] **Fallback**: HTTPS + WebSockets
  - For environments with restrictive firewalls
  - Uses standard port 443
  - Integrates with existing web infrastructure

#### 4.2 TLS/SSL Integration
- [ ] Add ASIO SSL context to CommandQueue
  ```cpp
  asio::ssl::context ssl_ctx_{asio::ssl::context::tlsv13};
  ```
- [ ] Implement certificate management
  - Server certificate + private key
  - Client certificate verification (mutual TLS)
  - Certificate revocation checking
- [ ] Configure cipher suites (strong encryption only)

#### 4.3 Binary Protocol Implementation
- [ ] Create `SecureChannel` class
  ```cpp
  class SecureChannel {
      asio::ssl::stream<asio::ip::tcp::socket> ssl_socket_;
  public:
      void send_command(const Command& cmd);
      Command::Ptr receive_command();
      bool verify_peer();
  };
  ```
- [ ] Add protocol version negotiation
- [ ] Implement session resumption for performance

#### 4.4 HTTPS/WebSocket Fallback
- [ ] Integrate WebSocket library (e.g., Boost.Beast)
- [ ] Create `WebSocketChannel` adapter
- [ ] Implement command tunneling over WebSocket frames
- [ ] Add automatic protocol detection/upgrade

#### 4.5 Authentication & Authorization
- [ ] Implement OAuth2-like token system
  - Worker authenticates with credentials
  - Receives time-limited access token
  - Token included in command headers
- [ ] Role-based access control
  - Workers have specific command permissions
  - CommandQueue enforces authorization
- [ ] Add audit logging for remote executions

#### 4.6 RemoteWorker Implementation
- [ ] Create `RemoteWorker` class
  - Connects via TLS or WebSocket
  - Handles authentication flow
  - Supports automatic reconnection with backoff
- [ ] Implement connection pooling
- [ ] Add request/response correlation (for async results)

#### 4.7 CommandQueue Remote Server
- [ ] Add ASIO SSL acceptor
- [ ] Implement rate limiting per worker
- [ ] Add DDoS protection measures
  - Connection limits
  - Request throttling
  - Blacklist/greylist
- [ ] Monitoring and metrics
  - Worker statistics
  - Command processing latency
  - Error rates

#### 4.8 NAT Traversal & Discovery
- [ ] Support reverse tunnels (worker initiates connection)
- [ ] Optional: Integrate STUN/TURN for NAT traversal
- [ ] Service discovery (DNS-SD, Consul, etc.)

#### 4.9 Testing & Validation
- [ ] Cross-internet integration tests
- [ ] Security penetration testing
- [ ] Certificate expiry/rotation tests
- [ ] Fallback protocol switching tests
- [ ] Latency measurement across geographies
- [ ] Compliance validation (encryption standards)

### Deliverables (Phase 4)
- Production-ready remote worker system
- TLS-secured binary protocol
- HTTPS/WebSocket fallback
- Comprehensive security features
- Full test coverage
- Deployment documentation

---

## Cross-Cutting Concerns

### Error Handling
- Define error taxonomy (network, serialization, execution, authentication)
- Implement retry policies with exponential backoff
- Command failure notifications
- Dead letter queue for undeliverable commands

### Observability
- Structured logging (command lifecycle, worker events)
- Metrics collection (Prometheus-compatible)
- Distributed tracing (optional, OpenTelemetry)
- Health check endpoints

### Performance
- Zero-copy serialization where possible
- Connection pooling and reuse
- Batching for high-throughput scenarios
- Back-pressure handling

### Configuration
- Unified configuration file (YAML/JSON)
- Runtime reconfiguration (e.g., port ranges, worker limits)
- Environment-specific overrides

### Documentation
- API reference for each phase
- Deployment guides (same-process, IPC, LAN, remote)
- Security best practices
- Troubleshooting guide

---

## Implementation Timeline (Estimated)

### Phase 1: Same-Process Workers
- **Duration**: 2-3 weeks
- **Complexity**: Medium
- **Risks**: ASIO learning curve, subscription API design

### Phase 2: Inter-Process Workers
- **Duration**: 2-3 weeks
- **Complexity**: Medium-High
- **Risks**: Serialization compatibility, process management

### Phase 3: LAN Workers
- **Duration**: 3-4 weeks
- **Complexity**: High
- **Risks**: Network reliability, port management, discovery

### Phase 4: Remote Workers
- **Duration**: 4-6 weeks
- **Complexity**: Very High
- **Risks**: Security vulnerabilities, protocol complexity, firewall issues

**Total Estimated Timeline**: 11-16 weeks

---

## Success Criteria

### Phase 1
✅ Multiple workers execute commands concurrently in same process
✅ Type-safe subscription to specific command types works correctly
✅ Performance is acceptable (< 1ms overhead per command)

### Phase 2
✅ Commands execute correctly across process boundaries
✅ Serialization round-trip preserves command state
✅ Worker crashes don't affect CommandQueue stability

### Phase 3
✅ Commands execute on different machines on LAN
✅ Port management handles exhaustion gracefully
✅ Network interruptions recover automatically
✅ Latency < 10ms for typical LAN

### Phase 4
✅ Secure communication over public internet
✅ Authentication prevents unauthorized access
✅ Fallback to HTTPS works in restrictive networks
✅ No known security vulnerabilities
✅ Production-ready monitoring and logging

---

## Next Steps

1. **Review and approve** this plan
2. **Set up development environment** with Boost.ASIO
3. **Create feature branch** for Phase 1
4. **Begin implementation** of Phase 1, Task 1.1

