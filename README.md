# KVStore — Concurrent In-Memory Key–Value Store

A concurrent, in-memory key–value store implemented in C, featuring a multi-threaded server, TCP-based client–server communication, and carefully synchronized shared state — engineered to production-quality standards.

---

## System Architecture

```
+-------------+
|  dbclient   |
| (TCP client)|
+------+------+
       |
       | TCP
       v
+----------------------------------+
|            dbserver              |
|                                  |
|  +----------------------------+  |
|  |   Console / Main Thread    |  |
|  |  - stats                   |  |
|  |  - list                    |  |
|  |  - quit                    |  |
|  +-------------+--------------+  |
|                |                 |
|  +-------------v--------------+  |
|  | Network Listener Thread    |  |
|  |  - accepts connections     |  |
|  |  - enqueues requests       |  |
|  +-------------+--------------+  |
|                |                 |
|  +-------------v--------------+  |
|  |  Worker Thread Pool (N)    |  |
|  |  - read / write / delete   |  |
|  |  - update stats            |  |
|  +----------------------------+  |
+----------------------------------+
```

---

## Request Lifecycle

```
Client sends command
        │
        ▼
Listener thread accepts TCP connection
        │
        ▼
Request enqueued into thread-safe task queue
        │
        ▼
Worker thread dequeues request
        │
     ┌──┴──────────────┐
     ▼                 ▼                 ▼
   SET              GET              DELETE
  (write)          (read)           (remove)
     │                 │                 │
     └──────────┬──────┘─────────────────┘
                ▼
   Response written back to client
                │
                ▼
       Stats updated (thread-safe)
```

---

## Core Features

### Concurrent Request Processing
The listener thread accepts incoming TCP connections and places requests into a bounded, thread-safe task queue. A configurable pool of worker threads continuously dequeues and processes operations in parallel, maximising throughput under load.

### Thread-Safe Design via Monitors
All shared components are implemented as monitors — combining a mutex with condition variables to eliminate busy-waiting and prevent data races. Monitored state includes the task queue, the key–value store itself, and global operation statistics.

### Key–Value Store Semantics
Keys and values are arbitrary strings. The store supports three core operations — `SET`, `GET`, and `DELETE` — with bounded capacity and deterministic, thread-safe behaviour under concurrency.

---

## Example Usage

**Start the server**
```bash
./dbserver
```

**Run clients**
```bash
./dbclient --set=KEY0 ABC
./dbclient --get=KEY0
./dbclient --delete=KEY0
```

**Server console commands**
```
stats    # show operation statistics
list     # dump current key-value pairs
quit     # graceful shutdown
```

---

## Technology Stack

| Category | Technology |
|---|---|
| Language | C (systems-level) |
| Threading | POSIX threads (pthreads) |
| Networking | TCP sockets |
| Synchronisation | Mutexes & condition variables |
| Concurrency patterns | Producer–consumer queue, monitor pattern |
| Statistics | Thread-safe aggregation |
| Build system |  CMake |
| IDE | CLion |

---

## Design Highlights

**No busy waiting** — worker threads block on condition variables when the queue is empty, consuming no CPU until work arrives.

**Bounded capacity** — the task queue has a configurable maximum depth, providing natural backpressure when clients outpace worker throughput.

**Graceful shutdown** — the `quit` command signals all threads to drain in-flight work and exit cleanly before the process terminates.

**Zero data races** — every access to shared state passes through a monitor; lock discipline is enforced at the API boundary, not by convention.
