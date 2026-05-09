# StockPro | C++ Full-Stack Portfolio Manager

StockPro is a high-performance, professional-grade stock portfolio management suite. It combines a robust **Native C++ Backend** with a modern **Glassmorphic Web Frontend** to provide real-time financial insights, risk analysis, and data mobility.

---

## 🏗️ System Architecture & Pipeline

StockPro operates on a custom **RESTful API Pipeline** built from the ground up without heavy external frameworks.

### 1. The C++ Engine (`server.cpp`)
The heart of the application is a native Windows C++ server utilizing the **Winsock2 (Windows Sockets)** library.
- **Protocol**: HTTP/1.1 over TCP/IP.
- **Data Handling**: The server acts as a File System Manager, reading and writing to `transactions.csv` with zero-latency overhead.
- **Endpoints**:
    - `GET /`: Serves the UI (`index.html`).
    - `GET /api/transactions`: Parses the CSV and returns a JSON array to the frontend.
    - `POST /api/transactions`: Receives new transaction strings and appends them to the database.
    - `POST /api/import`: Replaces the entire local database with an uploaded CSV stream.
    - `GET /api/export`: Streams the local `transactions.csv` directly to the browser for download.

### 2. The Communication Pipeline
When you perform an action in the browser (e.g., adding a stock):
1. **Frontend**: `script.js` captures your input and formats it into a CSV-compliant string.
2. **Request**: A `fetch()` request is sent to `http://localhost:8080`.
3. **Backend**: The C++ server intercepts the packet, parses the HTTP headers, extracts the payload, and performs an atomic write to `transactions.csv`.
4. **Synchronization**: The frontend then re-fetches the updated data to ensure the UI perfectly matches the server's state.

---

## 📊 Data Management Features

### 🔄 Undo & Redo (Snapshot System)
Every time a change is made, the frontend captures a "Snapshot" of the entire database before the edit.
- **Undo**: Reverts the local state to the previous snapshot and re-syncs it with the C++ server.
- **Redo**: Allows you to jump forward if you reversed a change by mistake.

### 📈 Stock Split Engine
Unlike simple trackers, StockPro handles corporate actions.
- **Logic**: If a "Split" transaction is detected (e.g., 2-for-1), the system chronologically multiplies existing share counts and divides average costs, ensuring historical profit/loss accuracy remains intact.

### 🛡️ Risk Intelligence (Portfolio Beta)
The dashboard calculates a weighted **Portfolio Beta** using a built-in market intelligence dictionary. 
- **Calculation**: $\sum (Weight_i \times Beta_i)$
- It tells you if your portfolio is more volatile (>1.0) or safer (<1.0) than the broader market.

---

## 📂 File Structure
- `server.cpp`: The C++ source code (Winsock2 server logic).
- `index.html`: The glassmorphic UI structure.
- `style.css`: Premium theme-aware design system.
- `script.js`: Frontend logic, calculation engine, and API synchronization.
- `transactions.csv`: The persistent flat-file database.

---

## 🛠️ Developer Setup & Compilation

### Prerequisites
- **Compiler**: GCC/G++ (MinGW-w64 recommended for Windows).
- **Libraries**: `ws2_32` (Windows Sockets 2).

### Compilation Command
```bash
g++ server.cpp -o server.exe -lws2_32
```

### Running the App
1. Execute `./server.exe`.
2. Open your browser to `http://localhost:8080`.

---

## 🔒 Security & Data Integrity
- **Local-First**: Your financial data never leaves your machine.
- **Atomic Operations**: CSV writes are handled sequentially to prevent data corruption.
- **Import Validation**: The backend validates the 7-column schema during data restoration.

---
**StockPro** - Built with Performance and Privacy in mind.
