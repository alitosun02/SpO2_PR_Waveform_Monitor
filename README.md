# SpO₂ PR Waveform Monitor — Qt/QML Medical Device Interface

**Educational/research project. Not a medical device and not for clinical use.**

A cross-platform SpO₂ and pulse rate monitoring application built with Qt/QML that interfaces with Biolight medical sensor modules. The system provides real-time vital sign monitoring, waveform visualization, patient management, and PDF report generation with 20-second historical data buffering.

## 🔍 Key Features

### 💻 **Real-time Monitoring**
- 📊 Live SpO₂ and Pulse Rate (PR) measurements
- 📈 Real-time plethysmography waveform visualization
- ⏸️ Freeze/unfreeze waveform display functionality
- 🎯 Response time configuration (4, 8, 16 seconds)

### 🏥 **Patient Management**
- 👥 Multi-patient database with SQLite
- 📝 Patient registration and selection
- 💾 Automatic measurement recording every 10 seconds
- 🔍 Advanced filtering by SpO₂ and PR ranges

### 📄 **Reporting & Export**
- 🖨️ PDF export with 20-second waveform snapshots
- 📊 Professional medical report formatting
- 🕐 Timestamp-based data visualization
- 📈 Historical waveform data preservation

### 🔌 **Device Integration**
- ⚙️ Biolight SpO₂ module protocol support (COM port communication)
- 🔄 Custom serial packet parsing (AA55 protocol)
- 📡 Configurable baud rate (375000) with odd parity
- 🛡️ Robust error handling and data validation

## 🧰 Tech Stack

| Layer | Technologies & Tools | Purpose |
|-------|---------------------|---------|
| **Hardware** | Biolight SpO₂ Module, Serial/USB Interface | Medical sensor data acquisition |
| **Desktop App** | Qt 6, QML, C++17 | Real-time UI, waveform rendering, patient management |
| **Database** | SQLite | Multi-patient data storage and historical records |
| **Communication** | QtSerialPort, Custom Protocol Parser | Device-to-application data transmission |
| **Reporting** | Qt PrintSupport, QPdfWriter | PDF generation and document export |
| **Threading** | QThread, DatabaseWorker | Asynchronous database operations |
| **UI Framework** | QML, Qt Quick Controls 2 | Modern, responsive user interface |

## 🏗️ System Architecture

The SpO₂ PR Waveform Monitor follows a modular, multi-threaded architecture designed for reliable medical data acquisition and management.

### Data Flow Overview

```
┌─────────────────┐ Serial Data  ┌──────────────────┐
│ Biolight SpO₂   │ ──────────────→ │ Reader Class     │
│ Module          │               │ (Packet Parser)  │
└─────────────────┘               └──────────────────┘
                                            │
                                            ▼
                                  ┌──────────────────┐
                                  │ QML UI Layer     │
                                  │ (Waveform Canvas)│
                                  └──────────────────┘
                                            │
                         ┌──────────────────┼──────────────────┐
                         ▼                  ▼                  ▼
              ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐
              │ MeasurementList │  │ DatabaseWorker  │  │ PdfExporter     │
              │ Model           │  │ (QThread)       │  │                 │
              └─────────────────┘  └─────────────────┘  └─────────────────┘
                         │                  │                  │
                         ▼                  ▼                  ▼
              ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐
              │ Patient         │  │ SQLite Database │  │ PDF Reports     │
              │ Management      │  │ (patients.db)   │  │ (Desktop)       │
              └─────────────────┘  └─────────────────┘  └─────────────────┘
```

### Key Components

1. **Reader** – Serial communication manager with freeze/unfreeze capability
2. **MeasurementListModel** – Qt model for patient data management
3. **DatabaseWorker** – Thread-safe database operations
4. **PdfExporter** – Professional medical report generation
5. **QML UI** – Modern, touch-friendly interface with real-time canvas rendering

## ⚠️ Medical Device Protocol

### Biolight SpO₂ Module Communication
- **Protocol**: Custom AA55 packet structure
- **Baud Rate**: 375,000 bps
- **Parity**: Odd
- **Data Format**: 8-bit data, 1 stop bit
- **Flow Control**: None

### Packet Structure
```
┌──────┬──────┬─────┬──────┬────────┬──────────┐
│ AA55 │ LEN  │CODE │ DATA │  ...   │CHECKSUM  │
├──────┼──────┼─────┼──────┼────────┼──────────┤
│ 2B   │ 1B   │ 1B  │ nB   │  ...   │   1B     │
└──────┴──────┴─────┴──────┴────────┴──────────┘
```

## 📁 Project Structure

```
SpO2_PR_Waveform_Monitor/
├── main.cpp                    # Application entry point
├── main.qml                    # Root QML interface
├── SpO2_PR_Waveform_Monitor.pro # Qt project configuration
│
├── reader.h / .cpp             # Serial communication & data parsing
├── databasemanager.h / .cpp    # Database coordination layer
├── databaseworker.h / .cpp     # Threaded database operations
├── measurementlistmodel.h / .cpp # Qt model for patient data
├── pdfexporter.h / .cpp        # PDF report generation
│
├── patients.db                 # SQLite database (auto-created)
└── build/                      # Build artifacts (ignored)
```

## 🚀 Quick Start

### Prerequisites

- Qt 6.x with QML support
- C++17 compatible compiler
- SQLite development libraries
- Biolight SpO₂ module with serial interface

### Building the Application

```bash
# Clone the repository
git clone https://github.com/alitosun02/SpO2_PR_Waveform_Monitor.git
cd SpO2_PR_Waveform_Monitor

# Configure Qt environment
qmake SpO2_PR_Waveform_Monitor.pro

# Build the application
make

# Run the application
./SpO2_PR_Waveform_Monitor
```

### Device Setup

1. Connect your Biolight SpO₂ module to a serial port (e.g., COM8 on Windows)
2. Update the port name in `main.cpp` if needed:
   ```cpp
   Reader *r = new Reader("COM8", &app);
   ```
3. Ensure proper baud rate configuration (375,000 bps, odd parity)

## 📊 Usage Guide

### Patient Management
1. **Add Patient**: Enter first name and last name, click "Hasta Kaydet"
2. **Auto Recording**: Measurements automatically save every 10 seconds when a patient is active
3. **View History**: Click "Veri Tablosunu Aç" to see all recorded measurements

### Waveform Monitoring
- **Real-time Display**: Waveform updates continuously with incoming data
- **Freeze Function**: Click "🔒 Freeze" to pause waveform display
- **Response Time**: Use settings menu to configure sensor response time

### PDF Export
1. Ensure a patient is selected and active
2. Click "Waveform'u PDF Olarak Kaydet"
3. PDF will be saved to desktop with patient name and timestamp

### Data Filtering
- Navigate to data table view
- Set SpO₂ and PR range filters
- Click "Filtrele" to apply, "Temizle" to reset

## 🔧 Configuration Options

### Serial Port Settings
```cpp
// In reader.cpp constructor
serial.setBaudRate(375000);
serial.setDataBits(QSerialPort::Data8);
serial.setParity(QSerialPort::OddParity);
serial.setStopBits(QSerialPort::OneStop);
```

### Response Time Options
- 4 seconds (fast response)
- 8 seconds (medium response)  
- 16 seconds (slow response, more stable)

### Database Schema
```sql
-- Patients table
CREATE TABLE patients (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    first_name TEXT NOT NULL,
    last_name TEXT NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- Measurements table
CREATE TABLE measurements (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    patient_id INTEGER NOT NULL,
    spo2 INTEGER NOT NULL,
    pr INTEGER NOT NULL,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY(patient_id) REFERENCES patients(id)
);
```

## 🛡️ Safety & Limitations

⚠️ **IMPORTANT DISCLAIMERS**

- This software is for **educational and research purposes only**
- **NOT approved for clinical or diagnostic use**
- **NOT a substitute for professional medical devices**
- Always use FDA-approved medical devices for patient care
- Developers assume no responsibility for medical decisions based on this software

## 🤝 Contributing

We welcome contributions! Please follow these guidelines:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

### Development Guidelines
- Follow Qt coding conventions
- Add unit tests for new functionality
- Update documentation for API changes
- Ensure thread safety for database operations

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Qt Project for the excellent framework
- Biolight for medical sensor technology
- SQLite for reliable embedded database
- Medical device protocol documentation contributors

---

**⚡ Built with Qt/QML | 🩺 Educational Use Only | 💻 Cross-Platform Compatible**
