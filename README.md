# Linux Networking Tools 🚀

A collection of low-level networking utilities built in **C on Linux** to understand and debug network communication. Ideal for learning socket programming, packet parsing, and traffic monitoring.

## Features
- **TCP Client & Server** (multi-client supported)
- **UDP Client & Server**
- **Packet Sniffer** (parses Ethernet/IP/TCP/UDP headers using raw sockets)
- **Traffic Logger** (monitors bandwidth, packet counts, latency)
- Fully compatible with Linux (tested on Ubuntu 20.04)

## Installation & Usage
```bash
# Clone repo
git clone https://github.com/YourGitHubUsername/Linux-Networking-Tools.git
cd Linux-Networking-Tools

# Compile all programs
make

# Run TCP Server (port 8080)
./tcp_server

# Run TCP Client
./tcp_client 127.0.0.1 8080

# Run UDP Server
./udp_server

# Run UDP Client
./udp_client 127.0.0.1 9090

# Run Packet Sniffer (requires root)
sudo ./packet_sniffer

# Run Traffic Logger (requires root)
sudo ./traffic_logger
