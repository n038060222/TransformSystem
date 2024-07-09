
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>
#include <netinet/in.h>
#include <jsoncpp/json/json.h>
#include <curl/curl.h> // For libcurl
#include "AppEnums.h"

#define SERVER_PORT 12345
#define APPUI_PORT 8765
#define MATRIX_SIZE 64

// Structure to hold a full image matrix
struct FullImageMatrix {
    int matrix[MATRIX_SIZE][MATRIX_SIZE];
};

// Structure to hold a mini image matrix
struct MiniImageMatrix {
    int matrix[16][16];
};

// Function to convert AppStatus enum to string
const char* appStatusToString(AppStatus status) {
    switch (status) {
        case AppStatus::SUCCESS_SEND_MATRIX: return "SUCCESS_SEND_MATRIX";
        case AppStatus::ERROR_SEND_MATRIX: return "ERROR_SEND_MATRIX";
        case AppStatus::SUCCESS_SEND_TIMESTAMP: return "SUCCESS_SEND_TIMESTAMP";
        case AppStatus::ERROR_SEND_TIMESTAMP: return "ERROR_SEND_TIMESTAMP";
        case AppStatus::TCP_CONNECT_SUCCESS: return "TCP_CONNECT_SUCCESS";
        case AppStatus::TCP_CONNECT_FAILURE: return "TCP_CONNECT_FAILURE";
        default: return "UNKNOWN_STATUS";
    }
}

// Function to convert AppError enum to string
const char* appErrorToString(AppError error) {
    switch (error) {
        case AppError::SOCKET_CREATION_FAILED: return "SOCKET_CREATION_FAILED";
        case AppError::INVALID_ADDRESS: return "INVALID_ADDRESS";
        case AppError::CONNECTION_FAILED: return "CONNECTION_FAILED";
        case AppError::MATRIX_SEND_FAILED: return "MATRIX_SEND_FAILED";
        case AppError::TIMESTAMP_SEND_FAILED: return "TIMESTAMP_SEND_FAILED";
        default: return "UNKNOWN_ERROR";
    }
}

// Function to convert AppSuccess enum to string
const char* appSuccessToString(AppSuccess success) {
    switch (success) {
        case AppSuccess::MATRIX_SENT_TO_APPY: return "MATRIX_SENT_TO_APPY";
        case AppSuccess::TIMESTAMP_SENT_TO_APPUI: return "TIMESTAMP_SENT_TO_APPUI";
        default: return "UNKNOWN_SUCCESS";
    }
}

/*
[in parm] - const FullImageMatrix& matrix, const std::string& logFile
[out] - none
[action] - Logs the full image matrix data to the specified log file.
*/
void logMatrix(const FullImageMatrix& matrix, const std::string& logFile) {
    std::ofstream logStream(logFile, std::ios::app);
    if (!logStream.is_open()) {
        std::cerr << "Failed to open log file " << logFile << std::endl;
        return;
    }

    logStream << "Matrix values:" << std::endl;
    for (int i = 0; i < MATRIX_SIZE; ++i) {
        for (int j = 0; j < MATRIX_SIZE; ++j) {
            logStream << matrix.matrix[i][j] << " ";
        }
        logStream << std::endl;
    }
    logStream << std::endl;

    logStream.close();
}

/*
[in parm] - const MiniImageMatrix& matrix, const std::string& logFile
[out] - none
[action] - Logs the mini image matrix data to the specified log file.
*/
void logMatrix(const MiniImageMatrix& matrix, const std::string& logFile) {
    std::ofstream logStream(logFile, std::ios::app);
    if (!logStream.is_open()) {
        std::cerr << "Failed to open log file " << logFile << std::endl;
        return;
    }

    logStream << "Matrix values:" << std::endl;
    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 16; ++j) {
            logStream << matrix.matrix[i][j] << " ";
        }
        logStream << std::endl;
    }
    logStream << std::endl;

    logStream.close();
}

/*
[in parm] - const std::string& jsonStr
[out] - none
[action] - Sends the JSON formatted matrix data to AppUI using HTTP POST request.
*/
AppStatus sendMatrixToAppUI(const std::string& jsonStr) {
    CURL *curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl) {
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8765/receive_matrix");
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonStr.c_str());

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return AppStatus::ERROR_SEND_MATRIX;
        }

        curl_easy_cleanup(curl);
    } else {
        curl_global_cleanup();
        return AppStatus::ERROR_SEND_MATRIX;
    }

    curl_global_cleanup();
    return AppStatus::SUCCESS_SEND_MATRIX;
}

/*
[in parm] - const FullImageMatrix& fullMatrix, const MiniImageMatrix& miniMatrix, const std::string& logFile
[out] - none
[action] - Transforms the full image matrix into a mini image matrix, logs both matrices, and sends them to AppUI.
*/
void transformMatrix(const FullImageMatrix& fullMatrix, MiniImageMatrix& miniMatrix, const std::string& logFile) {
    // Initialize mini matrix to zero
    memset(miniMatrix.matrix, 0, sizeof(miniMatrix.matrix));

    // Transform full matrix to mini matrix
    for (int i = 0; i < MATRIX_SIZE; ++i) {
        for (int j = 0; j < MATRIX_SIZE; ++j) {
            miniMatrix.matrix[i / 4][j / 4] += fullMatrix.matrix[i][j] / 16;
        }
    }

    // Average out values in mini matrix
    for (int i = 0; i < 16; ++i) {
        for (int j = 0; j < 16; ++j) {
            miniMatrix.matrix[i][j] /= 16;
        }
    }

    // Log mini matrix
    logMatrix(miniMatrix, logFile);

    // Send matrices to AppUI
    Json::Value root;
    Json::Value originalMatrix;
    Json::Value transformedMatrix;

    // Populate original matrix in JSON
    for (int i = 0; i < MATRIX_SIZE; ++i) {
        Json::Value row;
        for (int j = 0; j < MATRIX_SIZE; ++j) {
            row.append(fullMatrix.matrix[i][j]);
        }
        originalMatrix.append(row);
    }

    // Populate transformed matrix in JSON
    for (int i = 0; i < 16; ++i) {
        Json::Value row;
        for (int j = 0; j < 16; ++j) {
            row.append(miniMatrix.matrix[i][j]);
        }
        transformedMatrix.append(row);
    }

    root["originalMatrix"] = originalMatrix;
    root["transformedMatrix"] = transformedMatrix;

    // Convert JSON to string and send to AppUI
    std::string jsonStr = root.toStyledString();
    AppStatus status = sendMatrixToAppUI(jsonStr);

    // Print the status
    std::cout << appStatusToString(status) << std::endl;
}

/*
[in parm] - int socket_fd, const std::string& logFile, int& matrixCount
[out] - none
[action] - Receives a full image matrix from AppX via socket, logs it, transforms it into a mini image matrix, and sends both matrices to AppUI.
*/
void receiveMatrixFromAppX(int socket_fd, const std::string& logFile, int& matrixCount) {
    FullImageMatrix receivedMatrix;
    int bytes_received = recv(socket_fd, &receivedMatrix, sizeof(receivedMatrix), 0);

    if (bytes_received != sizeof(receivedMatrix)) {
        std::cerr << appErrorToString(AppError::MATRIX_SEND_FAILED) << std::endl;
        return;
    }

    if (matrixCount % 2 == 1) {
        matrixCount++;
        return;
    }

    logMatrix(receivedMatrix, logFile);
    MiniImageMatrix miniMatrix = {}; // Initialize to zero
    transformMatrix(receivedMatrix, miniMatrix, logFile);

    matrixCount++;
}

/*
[out] - none
[action] - Sets up a socket server to listen for connections from AppX, receives matrices, and processes them.
*/
void matrixReceiver(const std::string& logFile) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    int matrixCount = 0;

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << appErrorToString(AppError::SOCKET_CREATION_FAILED) << std::endl;
        return;
    }

    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        std::cerr << "setsockopt failed" << std::endl;
        close(server_fd);
        return;
    }

    // Assign address and port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(SERVER_PORT);

    // Bind the socket to the address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << appErrorToString(AppError::INVALID_ADDRESS) << std::endl;
        close(server_fd);
        return;
    }

    // Start listening for incoming connections
    if (listen(server_fd, 3) < 0) {
        std::cerr << "Listen failed" << std::endl;
        close(server_fd);
        return;
    }

    std::cout << "AppY listening on port " << SERVER_PORT << "..." << std::endl;

    // Accept incoming connections and handle matrices
    while (true) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            std::cerr << appErrorToString(AppError::CONNECTION_FAILED) << std::endl;
            continue;
        }

        std::cout << appStatusToString(AppStatus::TCP_CONNECT_SUCCESS) << std::endl;

        // Receive matrix from AppX
        receiveMatrixFromAppX(new_socket, logFile, matrixCount);

        close(new_socket);

        // Add delay to reduce CPU load
        usleep(20000);
    }

    close(server_fd);
}

/*
[out] - int
[action] - Entry point of the application. Starts the matrix receiver.
*/
int main() {
    std::string logFile = "matrix_log.txt";
    matrixReceiver(logFile);
    return 0;
}