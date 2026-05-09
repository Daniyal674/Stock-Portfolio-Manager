# Stock Portfolio Manager

A robust C++ command-line application designed to track and manage stock market investments using a specialized doubly linked list architecture.

## 1. Introduction

### 1.1 Problem Statement
Maintaining an accurate history of stock transactions is a complex task. Standard flat files or simple arrays often struggle with:
- **Historical Integrity**: Tracking the exact sequence of buy and sell orders.
- **Holdings Calculation**: Dynamically determining current positions by traversing full transaction histories.
- **Corporate Actions**: Adjusting historical data for events like stock splits without losing original transaction context.
- **Performance Analysis**: Calculating realized profit or loss over specific, user-defined date ranges.

The **Stock Portfolio Manager** addresses these challenges by implementing a custom Doubly Linked List (DLL), where each node represents a unique transaction, allowing for efficient traversal, modification, and reporting.

### 1.2 Operational Objective
The primary goal of this system is to provide a reliable, persistent, and user-friendly interface for investment tracking. It aims to:
- **Ensure Data Persistence**: Save and load all transaction data to a local CSV database.
- **Automate Calculations**: Instantly compute net holdings and profit/loss metrics.
- **Support Flexibility**: Allow users to edit or delete past transactions while maintaining list integrity.
- **Handle Scalability**: Efficiently manage a growing list of transactions through optimized DLL operations.

### 1.3 Brief Overview of the System
The system is built on a custom `PortfolioManager` class that manages a doubly linked list of `Node` structures. 
- **Data Structure**: Each `Node` contains metadata such as Transaction ID, Date, Stock Name, Type (Buy/Sell), Quantity, and Unit Price.
- **Persistence Layer**: All actions are mirrored in a `transactions.csv` file, ensuring no data loss between sessions.
- **Core Functionality**:
  - **Transaction Management**: Full CRUD (Create, Read, Update, Delete) capabilities.
  - **Portfolio Insights**: Real-time calculation of current holdings and P/L reports.
  - **Stock Split Utility**: A specialized function to adjust quantities and prices across historical transactions based on split ratios.

---

## 2. Header Files Explanation

The application utilizes several standard C++ libraries to provide its functionality:

| Header | Purpose in the System |
| :--- | :--- |
| `<iostream>` | Handles standard input and output (cin/cout) for the command-line interface. |
| `<string>` | Provides the `std::string` class for managing stock names, dates, and transaction types. |
| `<map>` | Used to efficiently aggregate and calculate current holdings by mapping stock names to their net quantities. |
| `<iomanip>` | Enables precise output formatting (e.g., `setw`, `fixed`, `setprecision`) for tabular history views and financial reports. |
| `<fstream>` | Facilitates file stream operations for saving and loading the `transactions.csv` database. |
| `<sstream>` | Used for string stream parsing when reading CSV data and splitting comma-separated values into individual fields. |
| `<limits>` | Provides `std::numeric_limits`, used to robustly clear the input buffer and handle invalid user input without crashing. |

---

## 3. Core Features

### 🟢 Transaction Management
- **Add**: Record new Buy or Sell orders with automatic ID generation.
- **Edit**: Modify any field of an existing transaction using its unique ID.
- **Delete**: Remove transactions from history; the system automatically repairs the DLL links.

### 📊 Portfolio Analysis
- **Current Holdings**: Scans the entire transaction history to display the net shares currently held for every stock in the portfolio.
- **Profit/Loss Reporting**: Generates a detailed report for any specific date range, calculating total buy value, total sell value, and net profit/loss status.

### ✂️ Stock Split Adjustment
A sophisticated tool to handle stock splits (e.g., 2:1, 5:1). It adjusts:
1. **Quantity**: Multiplied by the split ratio.
2. **Unit Price**: Divided by the split ratio.
This can be applied to all transactions for a stock or filtered by type (Buy/Sell).

---

## 4. Technical Architecture

### The Doubly Linked List (DLL)
The core of the system is a DLL. Unlike a standard array, a DLL allows for:
- **O(1) Deletion**: Once the node is found, pointers are remapped instantly.
- **Bi-directional Traversal**: Useful for sorting or complex reporting (though currently utilized for linear scanning).

```cpp
struct Node {
    int id;
    string date;
    string stockName;
    string type;
    int quantity;
    double unitPrice;
    Node *prev; // Pointer to previous transaction
    Node *next; // Pointer to next transaction
};
```

---

## 5. Getting Started

### Prerequisites
- A C++ compiler (GCC, Clang, or MSVC).

### Compilation
```bash
g++ main.cpp -o PortfolioManager
```

### Execution
```bash
./PortfolioManager
```

The system will automatically attempt to load `transactions.csv` upon startup.
