#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

const int PORT = 8080;
const string CSV_FILE = "transactions.csv";

size_t find_case_insensitive(const string& data, string toSearch) {
    string lowerData = data;
    transform(lowerData.begin(), lowerData.end(), lowerData.begin(), ::tolower);
    transform(toSearch.begin(), toSearch.end(), toSearch.begin(), ::tolower);
    return lowerData.find(toSearch);
}

string readFile(string path) {
    ifstream file(path, ios::binary);
    if (!file) return "";
    return string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
}

string csvToJson() {
    ifstream file(CSV_FILE);
    if (!file) return "[]";
    string line, header;
    getline(file, header);
    string json = "[";
    bool first = true;
    while (getline(file, line)) {
        if (line.empty() || line.find_first_not_of(" \t\n\r") == string::npos) continue;
        stringstream ss(line);
        string id, date, name, type, qty, price, total;
        if (getline(ss, id, ',') && getline(ss, date, ',') && getline(ss, name, ',') && 
            getline(ss, type, ',') && getline(ss, qty, ',') && getline(ss, price, ',') && 
            getline(ss, total)) {
            total.erase(remove(total.begin(), total.end(), '\r'), total.end());
            total.erase(remove(total.begin(), total.end(), '\n'), total.end());
            if (!first) json += ",";
            json += "{\"id\":" + id + ",\"date\":\"" + date + "\",\"name\":\"" + name + "\",\"type\":\"" + type + "\",\"qty\":" + qty + ",\"price\":" + price + ",\"total\":" + total + "}";
            first = false;
        }
    }
    json += "]";
    return json;
}

void resetCsv() {
    ofstream file(CSV_FILE);
    file << "ID,Date,Stock Name,Type,Quantity,Unit Price,Total Price\n";
    file.close();
}

int getNextId() {
    ifstream file(CSV_FILE);
    string line, header;
    getline(file, header);
    int maxId = 0;
    while (getline(file, line)) {
        if (line.empty() || line.find_first_not_of(" \t\n\r") == string::npos) continue;
        stringstream ss(line);
        string idStr;
        if (getline(ss, idStr, ',')) {
            try { int id = stoi(idStr); if (id > maxId) maxId = id; } catch (...) {}
        }
    }
    return maxId + 1;
}

void updateCsv(int targetId, string newData = "", bool isDelete = false) {
    ifstream file(CSV_FILE);
    vector<string> lines;
    string line, header;
    getline(file, header);
    while (getline(file, line)) {
        if (line.empty() || line.find_first_not_of(" \t\n\r") == string::npos) continue;
        stringstream ss(line);
        string idStr;
        getline(ss, idStr, ',');
        try {
            if (stoi(idStr) != targetId) lines.push_back(line);
            else if (!isDelete) lines.push_back(newData);
        } catch (...) { lines.push_back(line); }
    }
    file.close();
    ofstream outFile(CSV_FILE);
    outFile << header << "\n";
    for (const auto& l : lines) outFile << l << "\n";
    outFile.close();
}

void appendToCsv(string data) {
    data.erase(0, data.find_first_not_of(" \t\n\r"));
    data.erase(data.find_last_not_of(" \t\n\r") + 1);
    if (data.empty()) return;
    size_t firstComma = data.find(",");
    if (firstComma != string::npos) {
        int newId = getNextId();
        data = to_string(newId) + data.substr(firstComma);
    }
    ofstream file(CSV_FILE, ios::app);
    file << data << "\n";
    file.close();
}

void handleClient(SOCKET clientSocket) {
    char buffer[16384] = {0};
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead <= 0) { closesocket(clientSocket); return; }
    string request(buffer, bytesRead);
    string originalRequest = request;
    stringstream ss(request);
    string method, path;
    ss >> method >> path;
    size_t qPos = path.find_first_of("?#");
    if (qPos != string::npos) path = path.substr(0, qPos);
    string response = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
    if (method == "GET") {
        if (path == "/" || path == "/index.html") {
            string content = readFile("index.html");
            response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + to_string(content.length()) + "\r\nConnection: close\r\n\r\n" + content;
        } else if (path == "/style.css") {
            string content = readFile("style.css");
            response = "HTTP/1.1 200 OK\r\nContent-Type: text/css\r\nContent-Length: " + to_string(content.length()) + "\r\nConnection: close\r\n\r\n" + content;
        } else if (path == "/script.js") {
            string content = readFile("script.js");
            response = "HTTP/1.1 200 OK\r\nContent-Type: application/javascript\r\nContent-Length: " + to_string(content.length()) + "\r\nConnection: close\r\n\r\n" + content;
        } else if (path == "/api/transactions") {
            string content = csvToJson();
            response = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " + to_string(content.length()) + "\r\nAccess-Control-Allow-Origin: *\r\nConnection: close\r\n\r\n" + content;
        } else if (path == "/api/export") {
            string content = readFile(CSV_FILE);
            response = "HTTP/1.1 200 OK\r\nContent-Type: text/csv\r\nContent-Disposition: attachment; filename=\"transactions.csv\"\r\nContent-Length: " + to_string(content.length()) + "\r\nAccess-Control-Allow-Origin: *\r\nConnection: close\r\n\r\n" + content;
        }
    } else if (method == "POST") {
        if (path == "/api/transactions") {
            size_t bodyPos = request.find("\r\n\r\n");
            if (bodyPos != string::npos) {
                string body = request.substr(bodyPos + 4);
                size_t clPos = find_case_insensitive(request, "content-length: ");
                if (clPos != string::npos) {
                    try {
                        int expectedLen = stoi(request.substr(clPos + 16, request.find("\r\n", clPos + 16) - (clPos + 16)));
                        while (body.length() < (size_t)expectedLen) {
                            char bBuffer[4096] = {0};
                            int more = recv(clientSocket, bBuffer, sizeof(bBuffer)-1, 0);
                            if (more <= 0) break;
                            body += string(bBuffer, more);
                        }
                    } catch(...) {}
                }
                appendToCsv(body);
                response = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\nAccess-Control-Allow-Origin: *\r\nConnection: close\r\n\r\n";
            }
        } else if (path == "/api/reset") {
            resetCsv();
            response = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\nAccess-Control-Allow-Origin: *\r\nConnection: close\r\n\r\n";
        } else if (path == "/api/import") {
            size_t bodyPos = request.find("\r\n\r\n");
            if (bodyPos != string::npos) {
                string body = request.substr(bodyPos + 4);
                size_t clPos = find_case_insensitive(request, "content-length: ");
                if (clPos != string::npos) {
                    try {
                        int expectedLen = stoi(request.substr(clPos + 16, request.find("\r\n", clPos + 16) - (clPos + 16)));
                        while (body.length() < (size_t)expectedLen) {
                            char bBuffer[4096] = {0};
                            int more = recv(clientSocket, bBuffer, sizeof(bBuffer)-1, 0);
                            if (more <= 0) break;
                            body += string(bBuffer, more);
                        }
                    } catch(...) {}
                }
                ofstream file(CSV_FILE);
                file << body;
                file.close();
                response = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\nAccess-Control-Allow-Origin: *\r\nConnection: close\r\n\r\n";
            }
        }
    } else if (method == "DELETE" && path == "/api/transactions") {
        size_t idQueryPos = originalRequest.find("id=");
        if (idQueryPos != string::npos) {
            size_t idEnd = originalRequest.find_first_of(" \r\n&", idQueryPos);
            try {
                int id = stoi(originalRequest.substr(idQueryPos + 3, idEnd - (idQueryPos + 3)));
                updateCsv(id, "", true);
                response = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\nAccess-Control-Allow-Origin: *\r\nConnection: close\r\n\r\n";
            } catch (...) {}
        }
    }
    send(clientSocket, response.c_str(), response.length(), 0);
    closesocket(clientSocket);
}

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);
    bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(listenSocket, SOMAXCONN);
    cout << "========================================" << endl;
    cout << " StockPro C++ Engine - Data Management" << endl;
    cout << " Dashboard: http://localhost:" << PORT << endl;
    cout << "========================================" << endl;
    while (true) {
        SOCKET clientSocket = accept(listenSocket, NULL, NULL);
        if (clientSocket != INVALID_SOCKET) handleClient(clientSocket);
    }
    closesocket(listenSocket);
    WSACleanup();
    return 0;
}
