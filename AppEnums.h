// AppEnums.h

#ifndef APP_ENUMS_H
#define APP_ENUMS_H


enum class AppStatus {
    SUCCESS_SEND_MATRIX,
    ERROR_SEND_MATRIX,
    SUCCESS_SEND_TIMESTAMP,
    ERROR_SEND_TIMESTAMP,
    TCP_CONNECT_SUCCESS,
    TCP_CONNECT_FAILURE
};
enum class AppError {
    SOCKET_CREATION_FAILED,
    INVALID_ADDRESS,
    CONNECTION_FAILED,
    MATRIX_SEND_FAILED,
    TIMESTAMP_SEND_FAILED
};

enum class AppSuccess {
    MATRIX_SENT_TO_APPY,
    TIMESTAMP_SENT_TO_APPUI
};

enum class TCPConnectStatus {
    SUCCESS,
    FAILURE
};

#endif // APP_ENUMS_H
