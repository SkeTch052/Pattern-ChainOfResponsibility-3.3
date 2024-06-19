#include <iostream>
#include <string>
#include <fstream>
#include <stdexcept>

enum class Type {
    WARNING,
    ERROR,
    FATAL_ERROR,
    UNKNOWN
};


class LogMessage {
public:
    LogMessage(Type type, const std::string& msg) : type_(type), msg_(msg) {}

    Type type() const {
        return type_;
    }

    const std::string& message() const {
        return msg_;
    }
private:
    Type type_;
    std::string msg_;
};


class LogHandler {
public:
    virtual ~LogHandler() = default;

    void setNext(LogHandler* nextHandler) {
        this->nextHandler = nextHandler;
    }

    void handle(const LogMessage& logMessage) {
        if (!process(logMessage) && nextHandler) {
            nextHandler->handle(logMessage);
        }
    }
protected:
    virtual bool process(const LogMessage& logMessage) = 0;
private:
    LogHandler* nextHandler = nullptr;
};


class WarningHandler : public LogHandler {
protected:
    bool process(const LogMessage& logMessage) override {
        if (logMessage.type() == Type::WARNING) {
            std::cout << logMessage.message() << std::endl;
            return true;
        }
        return false;
    }
};


class ErrorHandler : public LogHandler {
public:
    ErrorHandler(const std::string& filePath) : logFilePath(filePath) {}

protected:
    bool process(const LogMessage& logMessage) override {
        if (logMessage.type() == Type::ERROR) {
            std::ofstream file(logFilePath, std::ios::app);

            if (file.is_open()) {
                file << logMessage.message() << std::endl;
            }

            return true;
        }
        return false;
    }

private:
    std::string logFilePath;
};


class FatalErrorHandler : public LogHandler {
protected:
    bool process(const LogMessage& logMessage) override {
        if (logMessage.type() == Type::FATAL_ERROR) {
            throw std::runtime_error(logMessage.message());
        }
        return false;
    }
};


class UnknownHandler : public LogHandler {
protected:
    bool process(const LogMessage& logMessage) override {
        if (logMessage.type() == Type::UNKNOWN) {
            throw std::runtime_error("Unhandled message (" + logMessage.message() + ")");
        }
        return false;
    }
};


int main()
{
    FatalErrorHandler fatalHandler;
    ErrorHandler errorHandler("logs.txt");
    WarningHandler warningHandler;
    UnknownHandler unknownHandler;

    fatalHandler.setNext(&errorHandler);
    errorHandler.setNext(&warningHandler);
    warningHandler.setNext(&unknownHandler);

    LogMessage warningMessage(Type::WARNING, "Warning message");
    LogMessage errorMessage(Type::ERROR, "Error message");
    LogMessage fatalErrorMessage(Type::FATAL_ERROR, "Fatal error message");
    LogMessage unknownMessage(Type::UNKNOWN, "UNKNOWN");

    try {
        warningHandler.handle(warningMessage);
        errorHandler.handle(errorMessage);
        fatalHandler.handle(fatalErrorMessage);
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
    }
    try {
        unknownHandler.handle(unknownMessage);
    }
    catch (const std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
    }
    return 0;
}