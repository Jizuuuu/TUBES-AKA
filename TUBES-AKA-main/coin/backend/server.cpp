#include <iostream>
#include <vector>
#include <chrono>
#include <sstream>
#include <cstring>
#include <winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

// ALGORITMA

// 1. Rekursif (tanpa memoization - sederhana)
int hitungCaraRekursif(int nominal, vector<int>& koin, int index) {
    if (nominal == 0) return 1;
    if (nominal < 0 || index >= koin.size()) return 0;
    
    int include = hitungCaraRekursif(nominal - koin[index], koin, index);
    int exclude = hitungCaraRekursif(nominal, koin, index + 1);
    
    return include + exclude;
}

// 2. Iteratif (Dynamic Programming)
int hitungCaraIteratif(int nominal, vector<int>& koin) {
    vector<int> dp(nominal + 1, 0);
    dp[0] = 1;
    
    for (int coin : koin) {
        for (int i = coin; i <= nominal; i++) {
            dp[i] += dp[i - coin];
        }
    }
    
    return dp[nominal];
}

// SERVER HTTP SEDERHANA 

string buatResponseJSON(int nominal, vector<int>& koin, long long waktuRekursif, long long waktuIteratif, int hasilRek, int hasilIt) {
    stringstream ss;
    ss << "{";
    ss << "\"nominal\":" << nominal << ",";
    ss << "\"koin\":[";
    for (size_t i = 0; i < koin.size(); i++) {
        ss << koin[i];
        if (i < koin.size() - 1) ss << ",";
    }
    ss << "],";
    ss << "\"rekursif\":{";
    ss << "\"hasil\":" << hasilRek << ",";
    ss << "\"waktu_ms\":" << waktuRekursif;
    ss << "},";
    ss << "\"iteratif\":{";
    ss << "\"hasil\":" << hasilIt << ",";
    ss << "\"waktu_ms\":" << waktuIteratif;
    ss << "}";
    ss << "}";
    return ss.str();
}

string handleRequest(const string& request) {
    // Parsing sederhana: cari parameter GET
    string response;
    
    size_t qm = request.find("GET /?");
    if (qm == string::npos) {
        // halaman default (untuk testing)
        response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
        response += "<h1>Server Analisis Koin Berjalan</h1>";
        response += "<p>Gunakan endpoint /?nominal=...&koin=... untuk analisis.</p>";
        return response;
    }
    
    size_t start = request.find("nominal=");
    size_t end = request.find("&", start);
    string strNominal = request.substr(start + 8, end - start - 8);
    int nominal = stoi(strNominal);
    
    start = request.find("koin=");
    end = request.find(" ", start);
    string strKoin = request.substr(start + 5, end - start - 5);
    
    // Parsing koin
    vector<int> koin;
    stringstream ss(strKoin);
    string token;
    while (getline(ss, token, ',')) {
        koin.push_back(stoi(token));
    }
    
    // Eksekusi algoritma
    auto startTime = chrono::high_resolution_clock::now();
    int hasilRek = hitungCaraRekursif(nominal, koin, 0);
    auto endTime = chrono::high_resolution_clock::now();
    long long waktuRekursif = chrono::duration_cast<chrono::microseconds>(endTime - startTime).count();
    
    startTime = chrono::high_resolution_clock::now();
    int hasilIt = hitungCaraIteratif(nominal, koin);
    endTime = chrono::high_resolution_clock::now();
    long long waktuIteratif = chrono::duration_cast<chrono::microseconds>(endTime - startTime).count();
    
    // Buat JSON response
    string json = buatResponseJSON(nominal, koin, waktuRekursif, waktuIteratif, hasilRek, hasilIt);
    
    response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: application/json\r\n";
    response += "Access-Control-Allow-Origin: *\r\n";
    response += "\r\n";
    response += json;
    
    return response;
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "Gagal inisialisasi Winsock" << endl;
        return 1;
    }
    
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        cerr << "Gagal membuat socket" << endl;
        WSACleanup();
        return 1;
    }
    
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8080);
    
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Gagal bind port 8080" << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    
    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        cerr << "Gagal listen" << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    
    cout << "Server berjalan di http://localhost:8080" << endl;
    cout << "Ctrl+C untuk berhenti" << endl;
    
    while (true) {
        sockaddr_in clientAddr;
        int clientSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);
        
        if (clientSocket == INVALID_SOCKET) {
            cerr << "Gagal accept client" << endl;
            continue;
        }
        
        char buffer[4096];
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytesReceived > 0) {
            string request(buffer);
            string response = handleRequest(request);
            send(clientSocket, response.c_str(), response.length(), 0);
        }
        
        closesocket(clientSocket);
    }
    
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}