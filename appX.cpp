#include <iostream>
#include <thread>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <chrono>
#include <fstream>
#include <atomic>
#include <jsoncpp/json/json.h>
#include <curl/curl.h>
#include "AppEnums.h"

// Server IP and ports
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT_APPY 12345 // Port for AppY communication
#define SERVER_PORT_APPUI 8765 // Port for AppUI communication

// Matrix properties
#define MATRIX_SIZE 64   // Size of the matrix (64x64)
#define FREQUENCY 50     // Frequency of matrix sending in Hz
#define NUM_THREADS 4    // Number of threads generating matrices

std::atomic<int> matrix_counter(0);  // Global matrix counter

// Structure to hold a full image matrix
struct FullImageMatrix {
    int matrix[MATRIX_SIZE][MATRIX_SIZE];
};

/*
[in parm] - FullImageMatrix& matrix
[out] - none
[action] - Generates a random matrix filled with integer values between 0 and 256. The generated matrix is stored in the provided FullImageMatrix reference.
*/
void generateRandomMatrix(FullImageMatrix& matrix) {
    for (int i = 0; i < MATRIX_SIZE; ++i) {
        for (int j = 0; j < MATRIX_SIZE; ++j) {
            matrix.matrix[i][j] = rand() % 257;
        }
    }
}

/*
[in parm] - int socket_fd, const FullImageMatrix& matrix
[out] - AppStatus
[action] - Sends the provided matrix over the socket connection specified by socket_fd to AppY. Returns AppStatus::SUCCESS_SEND_MATRIX if the matrix is sent successfully, AppStatus::ERROR_SEND_MATRIX otherwise.
*/
AppStatus sendMatrixToAppY(int socket_fd, const FullImageMatrix& matrix) {
    int bytes_sent = send(socket_fd, &matrix, sizeof(matrix), 0);
    if (bytes_sent != sizeof(matrix)) {
        return AppStatus::ERROR_SEND_MATRIX;
    }
    return AppStatus::SUCCESS_SEND_MATRIX;
}

/*
[in parm] - const std::string& timestamp
[out] - AppStatus
[action] - Sends the provided timestamp to AppUI using HTTP POST request. Returns AppStatus::SUCCESS_SEND_TIMESTAMP if the timestamp is sent successfully, AppStatus::ERROR_SEND_TIMESTAMP otherwise.
*/
AppStatus sendTimestampToAppUI(const std::string& timestamp) {
    CURL *curl;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl) {
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        Json::Value timestamp_json;
        timestamp_json["timestamp"] = timestamp;
        std::string jsonStr = Json::FastWriter().write(timestamp_json);

        curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:8765/receive_timestamp");
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonStr.c_str());

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return AppStatus::ERROR_SEND_TIMESTAMP;
        }

        curl_easy_cleanup(curl);
        curl_global_cleanup();
        return AppStatus::SUCCESS_SEND_TIMESTAMP;
    }

    curl_global_cleanup();
    return AppStatus::ERROR_SEND_TIMESTAMP;
}

/*
[in parm] - int thread_id
[out] - none
[action] - Establishes a socket connection to AppY, sends randomly generated matrices at a frequency of 50Hz, and sends timestamps to AppUI. Logs matrix data and connection status.
*/
void matrixSender(int thread_id) {
    int socket_fd_appy;

    struct sockaddr_in server_addr_appy;

    // Socket creation and connection for AppY
    if ((socket_fd_appy = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Failed to create socket for AppY" << std::endl;
        return;
    }

    server_addr_appy.sin_family = AF_INET;
    server_addr_appy.sin_port = htons(SERVER_PORT_APPY); // Define SERVER_PORT_APPY for AppY
    if (inet_pton(AF_INET, SERVER_IP, &server_addr_appy.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        return;
    }

    if (connect(socket_fd_appy, (struct sockaddr *)&server_addr_appy, sizeof(server_addr_appy)) < 0) {
        std::cerr << "Failed to connect to AppY" << std::endl;
        close(socket_fd_appy);
        return;
    }

    std::cout << "Thread " << thread_id << " connected to AppY on " << SERVER_IP << ":" << SERVER_PORT_APPY << std::endl;

    for (int i = 0; i < 50; ++i) { // 50 iterations for 50Hz frequency
        FullImageMatrix matrix;
        generateRandomMatrix(matrix);

        AppStatus status = sendMatrixToAppY(socket_fd_appy, matrix);
        if (status != AppStatus::SUCCESS_SEND_MATRIX) {
            std::cerr << "Failed to send matrix to AppY. Thread ID: " << thread_id << std::endl;
            break;
        }

        // Increment global matrix counter
        ++matrix_counter;

        // Send timestamp to AppUI
        auto now = std::chrono::system_clock::now();
        auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        auto timer = std::chrono::system_clock::to_time_t(now);
        std::tm bt = *std::localtime(&timer);

        char buffer[80];
        snprintf(buffer, 80, "%02d:%02d:%02d.%03d", bt.tm_hour, bt.tm_min, bt.tm_sec, static_cast<int>(now_ms.count()));

        std::string timestamp(buffer);

        status = sendTimestampToAppUI(timestamp);
        if (status != AppStatus::SUCCESS_SEND_TIMESTAMP) {
            std::cerr << "Failed to send timestamp to AppUI. Thread ID: " << thread_id << std::endl;
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / FREQUENCY));
    }

    close(socket_fd_appy);

    std::cout << "Thread " << thread_id << " - Closing Socket " << SERVER_IP << ":" << SERVER_PORT_APPY << std::endl;
}

/*
[out] - int
[action] - Entry point of the application. Initializes random seed, creates threads to execute matrixSender function, and waits for threads to complete.
*/
int main() {
    std::srand(std::time(nullptr)); // Initialize random seed
    std::vector<std::thread> threads;

    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.push_back(std::thread(matrixSender, i));
    }

    for (auto& thread : threads) {
        thread.join();
    }

    return 0;
}
