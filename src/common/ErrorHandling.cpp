/**
 * @file ErrorHandling.cpp
 * @brief Implementation of error handling utilities
 */

#include "ErrorHandling.h"
#include "pluginterfaces/base/ftypes.h"
#include <unordered_map>

namespace GranularPlunderphonics {

//------------------------------------------------------------------------
// GranularPlunderphonicsException implementation
//------------------------------------------------------------------------
GranularPlunderphonicsException::GranularPlunderphonicsException(int errorCode, const std::string& message)
: std::runtime_error(message), mErrorCode(errorCode)
{
}

//------------------------------------------------------------------------
int GranularPlunderphonicsException::getErrorCode() const
{
    return mErrorCode;
}

//------------------------------------------------------------------------
// ErrorHandler implementation
//------------------------------------------------------------------------
ErrorHandler::ErrorHandler()
: mCurrentErrorCode(ErrorCodes::kNoError), mCurrentErrorInfo("")
{
}

//------------------------------------------------------------------------
void ErrorHandler::setError(int errorCode, const std::string& additionalInfo)
{
    mCurrentErrorCode = errorCode;
    
    if (!additionalInfo.empty()) {
        mCurrentErrorInfo = additionalInfo;
    } else {
        mCurrentErrorInfo = getErrorDescription(errorCode);
    }
}

//------------------------------------------------------------------------
bool ErrorHandler::hasError() const
{
    return mCurrentErrorCode != ErrorCodes::kNoError;
}

//------------------------------------------------------------------------
int ErrorHandler::getErrorCode() const
{
    return mCurrentErrorCode;
}

//------------------------------------------------------------------------
std::string ErrorHandler::getErrorMessage() const
{
    if (!hasError()) {
        return "";
    }
    
    std::string baseMsg = getErrorDescription(mCurrentErrorCode);
    if (mCurrentErrorInfo.empty() || mCurrentErrorInfo == baseMsg) {
        return baseMsg;
    }
    
    return baseMsg + ": " + mCurrentErrorInfo;
}

//------------------------------------------------------------------------
void ErrorHandler::clearError()
{
    mCurrentErrorCode = ErrorCodes::kNoError;
    mCurrentErrorInfo.clear();
}

//------------------------------------------------------------------------
std::string ErrorHandler::getErrorDescription(int errorCode)
{
    static const std::unordered_map<int, std::string> errorMessages = {
        {ErrorCodes::kNoError, "No error"},
        {ErrorCodes::kInitializationError, "Failed to initialize plugin"},
        {ErrorCodes::kProcessingError, "Audio processing error"},
        {ErrorCodes::kMemoryError, "Memory allocation error"},
        {ErrorCodes::kInvalidParameter, "Invalid parameter value or ID"}
        // Additional error codes will be added here
    };
    
    auto it = errorMessages.find(errorCode);
    if (it != errorMessages.end()) {
        return it->second;
    }
    
    return "Unknown error (code " + std::to_string(errorCode) + ")";
}

//------------------------------------------------------------------------
Steinberg::tresult ErrorHandler::toVstResult(int errorCode)
{
    switch (errorCode) {
        case ErrorCodes::kNoError:
            return Steinberg::kResultOk;
            
        case ErrorCodes::kInitializationError:
            return Steinberg::kNotInitialized;
            
        case ErrorCodes::kInvalidParameter:
            return Steinberg::kInvalidArgument;
            
        case ErrorCodes::kMemoryError:
            return Steinberg::kOutOfMemory;
            
        case ErrorCodes::kProcessingError:
        default:
            return Steinberg::kInternalError;
    }
}

} // namespace GranularPlunderphonics