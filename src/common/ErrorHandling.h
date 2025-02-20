/**
 * @file ErrorHandling.h
 * @brief Error handling utilities for the GranularPlunderphonics plugin
 */

#pragma once

#include <string>
#include <exception>
#include <stdexcept>

namespace GranularPlunderphonics {

// Forward declare error codes here until we have the VST3 SDK
namespace ErrorCodes {
    enum ErrorCode {
        kNoError = 0,
        kInitializationError = -1000,
        kProcessingError = -1001,
        kMemoryError = -1002,
        kInvalidParameter = -1003,
        // Add more error codes as needed
    };
}

// Forward declaration for Steinberg types used when the SDK is available
namespace Steinberg {
    typedef int tresult;
    const tresult kResultOk = 0;
    const tresult kResultFalse = 1;
    const tresult kInvalidArgument = 2;
    const tresult kNotImplemented = 3;
    const tresult kInternalError = 4;
    const tresult kNotInitialized = 5;
    const tresult kOutOfMemory = 6;
}

/**
 * @class GranularPlunderphonicsException
 * @brief Custom exception class for plugin-specific errors
 */
class GranularPlunderphonicsException : public std::runtime_error {
public:
    /**
     * @brief Constructor with error code and message
     * @param errorCode The error code from ErrorCodes enum
     * @param message Detailed error message
     */
    explicit GranularPlunderphonicsException(int errorCode, const std::string& message);

    /**
     * @brief Get the associated error code
     * @return The error code
     */
    int getErrorCode() const;

private:
    int mErrorCode;  // Error code from ErrorCodes enum
};

/**
 * @class ErrorHandler
 * @brief Central error handling facility for the plugin
 *
 * Provides methods for setting, checking, and clearing errors,
 * as well as converting error codes to messages.
 */
class ErrorHandler {
public:
    /**
     * @brief Default constructor
     */
    ErrorHandler();

    /**
     * @brief Destructor
     */
    ~ErrorHandler() = default;

    /**
     * @brief Set the current error
     * @param errorCode Error code from ErrorCodes enum
     * @param additionalInfo Optional additional context information
     */
    void setError(int errorCode, const std::string& additionalInfo = "");

    /**
     * @brief Check if an error is currently set
     * @return true if an error is set, false otherwise
     */
    bool hasError() const;

    /**
     * @brief Get the current error code
     * @return Current error code, or kNoError if none is set
     */
    int getErrorCode() const;

    /**
     * @brief Get the current error message
     * @return Current error message, or empty string if no error
     */
    std::string getErrorMessage() const;

    /**
     * @brief Clear the current error state
     */
    void clearError();

    /**
     * @brief Get a descriptive message for an error code
     * @param errorCode Error code from ErrorCodes enum
     * @return Human-readable error description
     */
    static std::string getErrorDescription(int errorCode);

    /**
     * @brief Translate error codes to VST3 result codes
     * @param errorCode Error code from ErrorCodes enum
     * @return Equivalent Steinberg::tresult value
     */
    static Steinberg::tresult toVstResult(int errorCode);

private:
    int mCurrentErrorCode;           // Current error code
    std::string mCurrentErrorInfo;   // Additional context for the current error
};

} // namespace GranularPlunderphonics