# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [v0.2.0] - 2026-11-05

### Added
- **Nissan Leaf ZE1 Infotainment Nag Screen Dismissal (`nag_dismiss`)**:
  - Automatic dismissal of the infotainment startup confirmation ("OK") screen on AV-CAN (configurable via `nag_dismiss` in CLI, disabled by default).
  - Reverse gear detection on EV-CAN (`0x421`) to defer or pause nag screen dismissal while the reverse camera is active, resuming once a non-reverse gear is engaged.

### Fixed
- **CAN Bus Throughput & Logging Reliability**:
  - Prevented CAN reception and loop stalls under high bus load by bounding per-poll frame reads across TWAI and MCP2515 interfaces, increasing the TWAI RX queue length, and using non-blocking buffered Serial TX writes for CAN logs.

## [v0.1.0] - 2026-11-04

### Added
- **Nissan Leaf ZE1 CAN Bus Control & Telemetry**:
  - Asynchronous, non-blocking CAN command sequence state machine for waking up the vehicle and sending control commands.
  - Remote climate control (HVAC start and stop) with configurable target temperature setpoint.
  - Remote charging start to override the vehicle's charge timer.
  - Remote door lock and unlock control (opt-in via `locking_enabled` configuration, disabled by default).
  - Real-time CAN telemetry parsing for State of Charge (SOC %), cabin temperature, charging status and state (`Idle`, `Charging`, `Finished`, `Interrupted`, `Waiting`), HVAC status, heating/cooling mode indicators, HVAC target temperature setpoint, fan speed (%), ventilation airflow mode, individual door open/closed states (front left, front right, rear left, rear right, trunk), and door lock status.
- **Hardware & Multi-Board Support**:
  - Compile-time board selection (`simppeliTCU/hardware.h`) supporting **LilyGO T-2Can** and **Waveshare ESP32-S3-RS485-CAN**.
  - Dual CAN bus support on LilyGO T-2Can combining the ESP32 built-in TWAI controller (primary CAN) and the onboard MCP2515 SPI controller (secondary CAN).
- **Web Interface**:
  - Built-in mobile-friendly HTTP web UI displaying live vehicle telemetry, HVAC operating state, and remote action buttons.
  - Automatic page refresh while a CAN command sequence is in progress.
  - Firmware version and git commit hash indicator in the page footer.
- **Wi-Fi & Local Networking**:
  - Support for Wi-Fi Station (`STA`), Access Point (`AP`), and simultaneous `AP+STA` operating modes with automatic reconnection and AP password validation.
  - mDNS (`<hostName>.local`) and NetBIOS name resolution for easy local discovery.
- **MQTT & Home Automation Integration**:
  - OVMS v3-compatible MQTT topic hierarchy for publishing telemetry and receiving remote commands (compatible with OVMS Connect and IoT MQTT Panel).
  - Configurable TLS (`mqtt_tls`) support for encrypted or unencrypted MQTT broker connections.
  - Home Assistant MQTT Auto-Discovery (`hass_discovery`) for automatic registration of vehicle sensors, binary sensors, switches, and buttons.
- **Configuration & Serial CLI**:
  - Persistent configuration storage in ESP32 non-volatile memory (`Preferences`).
  - Interactive USB Serial CLI (`list`, `get`, `set`, `version`, `reboot`, `factory-reset`) at 115200 baud.
- **Testing & CI/CD**:
  - GoogleTest unit test suite for Leaf ZE1 CAN message parsing and command sequences with code coverage reporting.
  - GitHub Actions workflow for automated unit testing, per-board firmware compilation, automatic git tagging, and GitHub Releases.
