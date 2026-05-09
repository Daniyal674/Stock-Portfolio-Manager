#include <iostream>
#include <string>
#include <map>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <limits>

using namespace std;

struct Node
{
    int id;
    string date;
    string stockName;
    string type;
    int quantity;
    double unitPrice;

    Node *prev;
    Node *next;
};

class PortfolioManager
{
private:
    Node *head;
    Node *tail;
    int nextId;
    const string filename = "transactions.csv";

    void saveToFile()
    {
        ofstream file(filename);
        if (file.is_open())
        {
            file << "ID,Date,Stock Name,Type,Quantity,Unit Price,Total Price\n";

            Node *current = head;
            while (current)
            {
                double total = current->quantity * current->unitPrice;

                file << current->id << ","
                     << current->date << ","
                     << current->stockName << ","
                     << current->type << ","
                     << current->quantity << ","
                     << current->unitPrice << ","
                     << total << "\n";
                current = current->next;
            }
            file.close();
        }
        else
        {
            cout << "Error: Unable to save to database.\n";
        }
    }

public:
    PortfolioManager()
    {
        head = nullptr;
        tail = nullptr;
        nextId = 1;
    }

    void addTransaction(string date, string name, string type, int qty, double uPrice, int existingId = -1)
    {
        Node *newNode = new Node;

        if (existingId == -1)
        {
            newNode->id = nextId++;
        }
        else
        {
            newNode->id = existingId;
            if (existingId >= nextId)
            {
                nextId = existingId + 1;
            }
        }

        newNode->date = date;
        newNode->stockName = name;
        newNode->type = type;
        newNode->quantity = qty;
        newNode->unitPrice = uPrice;
        newNode->prev = nullptr;
        newNode->next = nullptr;

        if (!head)
        {
            head = tail = newNode;
        }
        else
        {
            tail->next = newNode;
            newNode->prev = tail;
            tail = newNode;
        }

        if (existingId == -1)
        {
            saveToFile();
            cout << "Transaction added successfully.\n";
            cout << "Total Value: Rs. " << (qty * uPrice) << " (Saved to ID: " << newNode->id << ")\n";
        }
    }

    void editTransaction(int targetId)
    {
        Node *current = head;
        bool found = false;

        while (current)
        {
            if (current->id == targetId)
            {
                found = true;
                cout << "\n--- Editing Transaction ID: " << targetId << " ---\n";

                cin.ignore(numeric_limits<streamsize>::max(), '\n');

                cout << "Enter New Date (YYYY-MM-DD): ";
                getline(cin, current->date);

                cout << "Enter New Stock Name: ";
                getline(cin, current->stockName);

                cout << "Enter New Type (Buy/Sell): ";
                getline(cin, current->type);

                cout << "Enter New Quantity: ";
                while (!(cin >> current->quantity))
                {
                    cout << "Invalid input. Enter a number: ";
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                }

                cout << "Enter New Unit Price: ";
                while (!(cin >> current->unitPrice))
                {
                    cout << "Invalid input. Enter a number: ";
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                }

                cout << "Transaction updated successfully.\n";
                cout << "New Total Value: Rs. " << (current->quantity * current->unitPrice) << "\n";
                saveToFile();
                break;
            }
            current = current->next;
        }

        if (!found)
            cout << "Transaction with ID " << targetId << " not found.\n";
    }

    void deleteTransaction(int targetId)
    {
        Node *current = head;
        bool found = false;

        while (current)
        {
            if (current->id == targetId)
            {
                found = true;

                if (current->prev)
                    current->prev->next = current->next;
                if (current->next)
                    current->next->prev = current->prev;
                if (current == head)
                    head = current->next;
                if (current == tail)
                    tail = current->prev;

                delete current;
                cout << "Transaction ID " << targetId << " deleted successfully.\n";
                saveToFile();
                break;
            }
            current = current->next;
        }

        if (!found)
            cout << "Transaction with ID " << targetId << " not found.\n";
    }

    void loadTransactions()
    {
        ifstream file(filename);
        if (!file.is_open())
            return;

        string line;
        while (getline(file, line))
        {
            stringstream ss(line);
            string idStr, date, name, type, qtyStr, priceStr;

            if (getline(ss, idStr, ',') &&
                getline(ss, date, ',') &&
                getline(ss, name, ',') &&
                getline(ss, type, ',') &&
                getline(ss, qtyStr, ',') &&
                getline(ss, priceStr, ','))
            {
                try
                {
                    int id = stoi(idStr);
                    int qty = stoi(qtyStr);
                    double uPrice = stod(priceStr);

                    addTransaction(date, name, type, qty, uPrice, id);
                }
                catch (...)
                {
                    continue;
                }
            }
        }
        file.close();
    }

    void viewHistory()
    {
        if (!head)
        {
            cout << "No transactions found.\n";
            return;
        }

        cout << "\n--- Transaction History ---\n";
        cout << left << setw(5) << "ID"
             << setw(15) << "Date"
             << setw(15) << "Stock"
             << setw(8) << "Type"
             << setw(10) << "Qty"
             << setw(12) << "Unit Price"
             << setw(12) << "Total Price" << endl;
        cout << "------------------------------------------------------------------------------\n";

        Node *current = head;
        while (current)
        {
            double total = current->quantity * current->unitPrice;
            cout << left << setw(5) << current->id
                 << setw(15) << current->date
                 << setw(15) << current->stockName
                 << setw(8) << current->type
                 << setw(10) << current->quantity
                 << setw(12) << current->unitPrice
                 << setw(12) << total << endl;
            current = current->next;
        }
        cout << endl;
    }

    void showCurrentHoldings()
    {
        if (!head)
        {
            cout << "No holdings to display.\n";
            return;
        }

        map<string, int> holdings;
        Node *current = head;

        while (current)
        {
            if (current->type == "Buy")
                holdings[current->stockName] += current->quantity;
            else if (current->type == "Sell")
                holdings[current->stockName] -= current->quantity;

            current = current->next;
        }

        cout << "\n--- Current Holdings ---\n";
        for (auto const &[name, qty] : holdings)
        {
            cout << "Stock: " << name << " | Net Shares: " << qty << endl;
        }
        cout << endl;
    }

    void generateProfitLossReport(string startDate, string endDate)
    {
        if (!head)
        {
            cout << "\nError: Database is empty.\n";
            return;
        }

        double totalBuyVal = 0;
        double totalSellVal = 0;
        Node *current = head;
        bool dataFound = false;

        cout << "\n--- P/L Report (" << startDate << " to " << endDate << ") ---\n";

        while (current)
        {
            if (current->date >= startDate && current->date <= endDate)
            {
                double value = current->unitPrice * current->quantity;
                dataFound = true;

                if (current->type == "Sell")
                    totalSellVal += value;
                else if (current->type == "Buy")
                    totalBuyVal += value;
            }
            current = current->next;
        }

        if (!dataFound)
        {
            cout << "No transactions found in this date range.\n";
            return;
        }

        double pnl = totalSellVal - totalBuyVal;

        cout << "Total Sell Value: Rs. " << totalSellVal << endl;
        cout << "Total Buy Value:  Rs. " << totalBuyVal << endl;
        cout << "Net Profit/Loss:  Rs. " << pnl << endl;

        cout << "Status: ";
        if (pnl > 0)
            cout << "Great, you are in profit." << endl;
        else
            cout << "You have to work more to go in profit." << endl;
        cout << endl;
    }

    // UPDATED FUNCTION: Now accepts targetType
    void applyStockSplit(string targetStock, double ratio, string targetType)
    {
        Node *current = head;
        bool found = false;
        int count = 0;

        while (current)
        {
            if (current->stockName == targetStock)
            {
                // Logic: If user entered "0", apply to ALL types.
                // Otherwise, only apply if the node's type matches the input type.
                if (targetType == "0" || current->type == targetType)
                {
                    current->quantity = (int)(current->quantity * ratio);
                    current->unitPrice = current->unitPrice / ratio;
                    found = true;
                    count++;
                }
            }
            current = current->next;
        }

        if (found)
        {
            cout << "Stock split applied to " << targetStock << " (" << count << " transactions updated).\n";
            if (targetType != "0")
            {
                cout << "Filtered by type: " << targetType << "\n";
            }
            cout << "Split Ratio: " << ratio << ":1\n";
            saveToFile();
        }
        else
        {
            cout << "Stock not found or no transactions matched the criteria.\n";
        }
    }

    ~PortfolioManager()
    {
        Node *current = head;
        while (current)
        {
            Node *nextNode = current->next;
            delete current;
            current = nextNode;
        }
    }
};

void clearInputBuffer()
{
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

int main()
{
    PortfolioManager pm;
    int choice = 0;

    pm.loadTransactions();

    do
    {
        cout << "=== Stock Portfolio Manager ===\n";
        cout << "1. Add Transaction\n";
        cout << "2. View History\n";
        cout << "3. Edit Transaction\n";
        cout << "4. Delete Transaction\n";
        cout << "5. Current Holdings\n";
        cout << "6. Profit/Loss Report\n";
        cout << "7. Apply Stock Split\n";
        cout << "8. Exit\n";
        cout << "Enter choice: ";

        if (!(cin >> choice))
        {
            cout << "Invalid input. Please enter a number.\n";
            clearInputBuffer();
            continue;
        }

        switch (choice)
        {
        case 1:
        {
            string date, name, type;
            int qty;
            double uPrice;

            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            cout << "Enter Date (YYYY-MM-DD): ";
            getline(cin, date);

            cout << "Enter Stock Name: ";
            getline(cin, name);

            cout << "Enter Type (Buy/Sell): ";
            getline(cin, type);

            cout << "Enter Quantity: ";
            while (!(cin >> qty))
            {
                cout << "Invalid quantity. Enter a number: ";
                clearInputBuffer();
            }

            cout << "Enter Unit Price (Rs): ";
            while (!(cin >> uPrice))
            {
                cout << "Invalid price. Enter a number: ";
                clearInputBuffer();
            }

            pm.addTransaction(date, name, type, qty, uPrice);
            break;
        }

        case 2:
            pm.viewHistory();
            break;

        case 3:
        {
            int id;
            cout << "Enter Transaction ID to Edit: ";
            while (!(cin >> id))
            {
                cout << "Invalid ID. Enter a number: ";
                clearInputBuffer();
            }
            pm.editTransaction(id);
            break;
        }

        case 4:
        {
            int id;
            cout << "Enter Transaction ID to Delete: ";
            while (!(cin >> id))
            {
                cout << "Invalid ID. Enter a number: ";
                clearInputBuffer();
            }
            pm.deleteTransaction(id);
            break;
        }

        case 5:
            pm.showCurrentHoldings();
            break;

        case 6:
        {
            string start, end;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            cout << "Enter Start Date (YYYY-MM-DD): ";
            getline(cin, start);

            cout << "Enter End Date (YYYY-MM-DD): ";
            getline(cin, end);

            pm.generateProfitLossReport(start, end);
            break;
        }

        case 7:
        {
            string name, splitType;
            double ratio;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            cout << "Enter Stock Name: ";
            getline(cin, name);

            // UPDATED INPUT: Ask for Type or 0
            cout << "Enter Type to split (Buy/Sell) or enter 0 to split ALL: ";
            getline(cin, splitType);

            cout << "Enter Split Ratio (e.g., 2 for 2:1): ";
            while (!(cin >> ratio))
            {
                cout << "Invalid ratio. Enter a number: ";
                clearInputBuffer();
            }
            pm.applyStockSplit(name, ratio, splitType);
            break;
        }

        case 8:
            cout << "Exiting...\n";
            break;

        default:
            cout << "Invalid choice.\n";
            break;
        }

    } while (choice != 8);

    return 0;
}