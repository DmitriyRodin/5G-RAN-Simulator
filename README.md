# 5G RAN Simulator

This project is a high-level 5G RAN network simulator designed to demonstrate C++ system programming skills and an understanding of telecommunications protocol architecture.

Designed a 5G simulator using C++17 and Qt6, configurable as a monolith or as distributed micro-services with independent UE, gNB, and RadioHub/GUI-monitor binaries. Modeled 3GPP Control Plane signaling, including the RRC state machine. Developed pluggable serialization layers (streams, Protobuf) with runtime selection, alongside a live node monitoring dashboard with GUI. Maintained software quality using GTest/GMock and automated CI/CD pipelines via GitHub Actions. Using asynchronous event-driven engine via Qt Event Loop for concurrency-free network event processing

Technology stack
C++17: gNB and UE core logic
Protobuf

## Basic components

* **gNB (Next Generation NodeB):** Manages Radio Resource Control (RRC)
* **RadioHub:** Acts as a transparent proxy between gNB and UE. RadioHub simulates the transmission of messages over radio.
* **UE (User Equipment):** Simulates the mobile device's protocol stack and state transitions.

##  Protocol Sequence (Initial Access & Registration)

The following diagram illustrates the implemented signaling flow, from Cell Selection to successful Network Registration:

![5G Registration Sequence Diagram](./doc/diagrams/5G-RAN-Simulator-Sequence-Diagram.jpg)

## USER VIEW

This allows the user to see base stations (GNBs) with their coverage radius on the map, as well as user equipments (UEs) connected to specific GNB and UEs not connected to any GNB. Tables with additional information are also available to the right of the map.

![User View](./doc/user-view/5G-RAN-Simulator-user-view.JPG)

## Deployment Modes

The simulator now supports two distinct deployment modes, configured via the `config.yaml` file under the `simulation` section:

1. Monolithic Mode
How it works: The SimulationController automatically spins up and manages all network nodes (GnbLogic and UeLogic) internally as local objects within the same process.

Use case: Ideal for quick local testing, debugging core simulation logic, and lightweight scenarios without network orchestration overhead.

2. Distributed Mode
How it works: Nodes are deployed independently as separate, standalone services.

The simulator now supports two types os serializer (include Protobuf), configured via 'config.yaml' file.

Use case: Designed for realistic telecom network emulation, scalability testing, and simulating real-world distributed environments.

## Testing

1. Unit tests: bash ./check_before_push.sh
2. Integration test: python3 -m pytest tests/integration/test_gnb.py -v

## Status
   in progress
