
3GPP logic development (start)
| ID  |  Group          | Technical Scope                                                                                                                                                                                                                               | status   |
|:----|:----------------|:----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|:---------|
| 001 | Base entity     | Creating a pure virtual base class using QUdpSocket and its methods. Encapsulating common logic: reading datagrams and node ID.                                                                                                               | Done     |
| 002 | gNB basic logic | Implementation of base station logic. Maintaining a registry of connected UEs (UeContext) and processing registration requests. Message Types. Sending broadcast info. Processes incoming input and dispatches it to the appropriate handlers | Done     |
| 003 | UE basic logic  | Implementation of UE logic. Cell Search, Attach Request and Measurement Reports (RSSI).                                                                                                                                                       | Done     |

Tech Design
| ID  | Group           | Technical Scope                                         | status      |
|:----|:----------------|:--------------------------------------------------------|:------------|
| 004 | Roadmap         | Development road map.                                   | Done        |
| 005 | Docs            | Create project documentation and accompanying diagrams. | Not started |

Infrastructure & Observability
| ID  |  Group                                      | Technical Scope                                                                                                                                                                      | status      |
|:----|:--------------------------------------------|:-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|:------------|
| 006 | Radio Environment Simulation                | Implementation of so called RadioHub that simulates the physical environment. It calculates RSSI based on distance and delivers packets only to those within the coverage area.      | Not started |
| 007 | Centralized Signaling Tracing               | Implementation of a mechanism for capturing and recording all messages passing through RadioHub                                                                                      | Not started |
| 008 | Cloud-Native RAN Orchestration              | Docker-based containerization and deployment of nodes (gNB, UE, RadioHub) in a local Kubernetes cluster (Minikube/K3s) to emulate a distributed RAN cloud infrastructure.            | Not started |
| 009 | Unit Testing Framework Implementation       | Deploying a testing environment. Unit testing covers key algorithms: Path Loss (RSSI) calculation, JSON message parsing, and state transitions (State Machine).                      | Not started |
| 010 | Integration & Protocol Flow Verification    | Automated node interaction testing via RadioHub. End-to-end  testing: from Cell Search to successful Attach and Handover.                                                            | Not started |
| 011 | CI/CD Pipeline Automation                   | Setting up GitHub Actions. Automatically build the project and run tests with every push, and build Docker images.                                                                   | Not started |

Visualization
| ID  |  Group                    | Technical Scope                                                                                                                                              | status      |
|:----|:--------------------------|:-------------------------------------------------------------------------------------------------------------------------------------------------------------|:------------|
| 012 | Topology Live View        | A graphical window based on QGraphicsView or QML displays a map: gBNs with their radii and moving smartphone icons. Visualization of communication "beams."  | Not started |
| 013 | UE Connectivity Dashboard | An analytical dashboard with real-time data updates: smartphone ID, which gNB it is connected to, current signal strength, and status (Attached/Idle).       | Not started |

3GPP logic development (continued)
| ID  |  Group                    | Technical Scope                                                                                                                                                | status      |
|:----|:--------------------------|:---------------------------------------------------------------------------------------------------------------------------------------------------------------|:------------|
| 014 | RACH Procedure Simulation                   | Implementation of a 4-way (or 2-way for HO) synchronization protocol. Addition of Msg2 wait states and Timing Advance time adjustments.      | Not started |
| 015 | 5QI Prioritization                          | Injection of QoS labels into data packets. Logic for separating traffic into GBR (voice) and Non-GBR (internet) for the scheduler.           | Not started |
| 016 | PRB Allocation                              | Bandwidth quantization. Converting available PRBs into bytes based on the signal strength (MCS) and allocating them to specific users.       | Not started |
| 017 | MAC Layer Scheduler                         | Resource allocation algorithm (e.g., Proportional Fair). Decides which UE will receive the right to transmit in the current time slice (TTI).| Not started |
| 018 | ASN.1 Serialization                         | Replacement of QJsonDocument with binary serialization. Simulation of real 3GPP encoding, which is critical for conserving air resources.    | Not started |
| 019 | Xn Interface (Backhaul Transport)           | Implementation of direct communication between GnbLogic objects via a dedicated UDP port (simulating a landline cable).                      | Not started |
| 020 | RRC Mobility Management (Handover Decision) | Event A3 event processing logic. RSSI comparison of the current and neighboring cells, taking into account hysteresis and Time-to-Trigger.   | Not started |
