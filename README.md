# kadena-client-api

Kadena blockchain client API for microcontroller and IoT devices. Created by [Crankk.io](https://crankk.io) for interfacing with the Kadena blockchain.

## Overview

This library provides a lightweight client implementation for interacting with the Kadena blockchain from microcontrollers and IoT devices. It handles:

- Secure communication with Kadena nodes
- Payload encryption and hashing
- Basic node synchronization
- Command execution

## Features

- Minimal dependencies for embedded systems
- Secure encryption using mbedTLS
- Support for RSA and AES encryption
- Base64 encoding/decoding
- JSON parsing with ArduinoJson
- Configurable blockchain endpoints
- Basic wallet key validation
- Command execution interface

## Requirements

- C++17 capable compiler
- Arduino-compatible device
- mbedTLS library
- ArduinoJson library
- Base64 library

## Usage

Add this repository to your PlatformIO project's `platformio.ini` selecting the desired tag:

```ini
lib_deps =
    https://github.com/crankk/kadena-client-api.git#1.0.0
```

## Testing

The library includes a test suite that can be run using PlatformIO's test runner.

### Prerequisites
- Python 3.x
- PlatformIO CLI
- Build essentials
- mbedTLS development libraries

### Running Tests

1. Install system dependencies (platform specific):

Linux:
```bash
sudo apt-get update
sudo apt-get install -y build-essential
sudo apt-get install -y libmbedtls-dev
```

macOS:
```bash
brew install mbedtls
```

Windows:
```bash
# Using vcpkg:
vcpkg install mbedtls:x64-windows
# Or download from: https://github.com/Mbed-TLS/mbedtls/releases
```

2. Install PlatformIO:
```bash
python -m pip install --upgrade pip
pip install platformio
```

3. Run the tests:
```bash
pio test -e native -v
```

## Credits

Created and maintained by [Crankk.io](https://crankk.io)

For more information about Kadena blockchain, visit [kadena.io](https://kadena.io)
