/**
 * @file ParameterManager.h
 * @brief Parameter management system for the GranularPlunderphonics plugin
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <functional>
#include <mutex>
#include <unordered_map>
#include <cmath>
#include "../common/Logger.h"
#include "../common/ErrorHandling.h"

//------------------------------------------------------------------------
// Forward declarations for VST3 types (outside our namespace)
//------------------------------------------------------------------------
namespace Steinberg {
    typedef int tresult;
    typedef int int32;

    namespace Vst {
        typedef int32 ParamValue;
        typedef int32 ParamID;
    }
}

namespace GranularPlunderphonics {
//------------------------------------------------------------------------
// Forward references to ::Steinberg namespace
//------------------------------------------------------------------------
using ParamID = ::Steinberg::Vst::ParamID;
using ParamValue = ::Steinberg::Vst::ParamValue;

/**
 * @enum ParameterType
 * @brief Supported parameter types
 */
enum class ParameterType {
    Float,      // Continuous floating point parameter
    Integer,    // Integer parameter with step values
    Boolean,    // Boolean parameter (on/off)
    Enum        // Enumerated parameter with named options
};

/**
 * @enum ParameterFlags
 * @brief Flags that modify parameter behavior
 */
enum class ParameterFlags {
    NoFlags         = 0,
    IsReadOnly      = 1 << 0,  // Parameter cannot be modified by user
    IsHidden        = 1 << 1,  // Parameter is not visible in the UI
    IsProgramChange = 1 << 2,  // Parameter represents a program change
    IsBypass        = 1 << 3,  // Parameter controls bypass state
    IsLogarithmic   = 1 << 4,  // Parameter uses logarithmic scaling
    IsStepInteger   = 1 << 5   // Parameter is stepped as integer values
};

// Helper for combining flags
inline ParameterFlags operator|(ParameterFlags a, ParameterFlags b) {
    return static_cast<ParameterFlags>(
        static_cast<int>(a) | static_cast<int>(b)
    );
}

inline bool hasFlag(ParameterFlags flags, ParameterFlags flag) {
    return (static_cast<int>(flags) & static_cast<int>(flag)) != 0;
}

/**
 * @struct EnumValue
 * @brief Represents a single option in an enumerated parameter
 */
struct EnumValue {
    int value;              // Integer value of the enum option
    std::string name;       // Display name of the enum option
    std::string shortName;  // Short name (for limited displays)
};

/**
 * @class ParameterValue
 * @brief Thread-safe parameter value container with interpolation
 */
class ParameterValue {
public:
    ParameterValue(float initialValue = 0.0f, float smoothingTimeMs = 20.0f);

    /**
     * @brief Set the target value (will be smoothly interpolated to)
     * @param newValue The new target value
     */
    void setTarget(float newValue);

    /**
     * @brief Get the current smoothed value
     * @param sampleRate Current sample rate for time-based calculations
     * @return Current interpolated value
     */
    float getSmoothed(float sampleRate);

    /**
     * @brief Get the current target value (not smoothed)
     * @return Target value
     */
    float getTarget() const;

    /**
     * @brief Set smoothing time in milliseconds
     * @param timeMs Smoothing time in milliseconds
     */
    void setSmoothingTime(float timeMs);

    /**
     * @brief Reset smoothing (make current value immediately match target)
     */
    void resetSmoothing();

private:
    std::atomic<float> mTargetValue;     // Target parameter value
    std::atomic<float> mCurrentValue;    // Current interpolated value
    std::atomic<float> mSmoothingTime;   // Smoothing time in milliseconds
    std::atomic<bool> mNeedsSmoothing;   // Flag to indicate if smoothing is needed
};

/**
 * @class Parameter
 * @brief Base class for all parameter types
 */
class Parameter {
public:
    /**
     * @brief Constructor for Parameter
     * @param id Unique parameter ID
     * @param name Parameter display name
     * @param shortName Short parameter name
     * @param type Parameter type (float, int, bool, enum)
     * @param flags Parameter behavior flags
     * @param smoothingTimeMs Time in milliseconds for parameter smoothing
     */
    Parameter(ParamID id,
              const std::string& name,
              const std::string& shortName,
              ParameterType type,
              ParameterFlags flags = ParameterFlags::NoFlags,
              float smoothingTimeMs = 20.0f);

    /**
     * @brief Virtual destructor
     */
    virtual ~Parameter() = default;

    /**
     * @brief Get parameter ID
     * @return Parameter ID
     */
    ParamID getId() const;

    /**
     * @brief Get parameter name
     * @return Parameter name
     */
    const std::string& getName() const;

    /**
     * @brief Get short parameter name
     * @return Short parameter name
     */
    const std::string& getShortName() const;

    /**
     * @brief Get parameter type
     * @return Parameter type
     */
    ParameterType getType() const;

    /**
     * @brief Get parameter flags
     * @return Parameter flags
     */
    ParameterFlags getFlags() const;

    /**
     * @brief Check if parameter has a specific flag
     * @param flag Flag to check
     * @return True if parameter has the flag
     */
    bool hasFlag(ParameterFlags flag) const;

    /**
     * @brief Convert normalized value [0,1] to real value
     * @param normalizedValue Value in normalized range [0,1]
     * @return Real parameter value
     */
    virtual float denormalize(float normalizedValue) const = 0;

    /**
     * @brief Convert real value to normalized value [0,1]
     * @param realValue Real parameter value
     * @return Normalized value in range [0,1]
     */
    virtual float normalize(float realValue) const = 0;

    /**
     * @brief Get normalized default value
     * @return Default value in normalized range [0,1]
     */
    float getDefaultNormalizedValue() const;

    /**
     * @brief Get parameter value as string
     * @param normalizedValue Value in normalized range [0,1]
     * @return String representation of the value
     */
    virtual std::string toString(float normalizedValue) const = 0;

    /**
     * @brief Try to set parameter from string value
     * @param string String representation of value
     * @param normalizedValue [out] Normalized value if successful
     * @return True if string was successfully parsed
     */
    virtual bool fromString(const std::string& string, float& normalizedValue) const = 0;

    /**
     * @brief Set normalized value
     * @param normalizedValue Value in normalized range [0,1]
     */
    void setNormalized(float normalizedValue);

    /**
     * @brief Get normalized value (target, not smoothed)
     * @return Target normalized value
     */
    float getNormalized() const;

    /**
     * @brief Get smoothed normalized value
     * @param sampleRate Current sample rate for time calculations
     * @return Current smoothed normalized value
     */
    float getSmoothedNormalized(float sampleRate);

    /**
     * @brief Set real value (will be normalized internally)
     * @param realValue Real parameter value
     */
    void setReal(float realValue);

    /**
     * @brief Get real value (denormalized from target)
     * @return Real parameter value
     */
    float getReal() const;

    /**
     * @brief Get smoothed real value
     * @param sampleRate Current sample rate for time calculations
     * @return Current smoothed real value
     */
    float getSmoothedReal(float sampleRate);

    /**
     * @brief Reset smoothing (make current value immediately match target)
     */
    void resetSmoothing();

    /**
     * @brief Set parameter change callback
     * @param callback Function to call when parameter changes
     */
    void setChangeCallback(std::function<void(float)> callback);

    /**
     * @brief Prepare parameter info
     * This is a placeholder that will be implemented when the VST3 SDK is available
     */
    virtual void fillParameterInfo() const;

protected:
    ParamID mId;                  // Parameter ID
    std::string mName;            // Parameter name
    std::string mShortName;       // Short parameter name
    ParameterType mType;          // Parameter type
    ParameterFlags mFlags;        // Parameter flags
    ParameterValue mValue;        // Parameter value with smoothing
    float mDefaultNormalizedValue;// Default value in normalized range
    std::function<void(float)> mChangeCallback;  // Callback when value changes
};

/**
 * @class FloatParameter
 * @brief Parameter with continuous floating point value
 */
class FloatParameter : public Parameter {
public:
    /**
     * @brief Constructor for float parameter
     * @param id Unique parameter ID
     * @param name Parameter display name
     * @param shortName Short parameter name
     * @param minValue Minimum real value
     * @param maxValue Maximum real value
     * @param defaultValue Default real value
     * @param flags Parameter behavior flags
     * @param units Optional units string (e.g., "dB", "Hz")
     * @param smoothingTimeMs Time in milliseconds for parameter smoothing
     */
    FloatParameter(ParamID id,
                   const std::string& name,
                   const std::string& shortName,
                   float minValue,
                   float maxValue,
                   float defaultValue,
                   ParameterFlags flags = ParameterFlags::NoFlags,
                   const std::string& units = "",
                   float smoothingTimeMs = 20.0f);

    float denormalize(float normalizedValue) const override;
    float normalize(float realValue) const override;
    std::string toString(float normalizedValue) const override;
    bool fromString(const std::string& string, float& normalizedValue) const override;

    // Parameter-specific implementation (placeholder)
    void fillParameterInfo() const override;

    /**
     * @brief Get minimum value
     * @return Minimum real value
     */
    float getMin() const;

    /**
     * @brief Get maximum value
     * @return Maximum real value
     */
    float getMax() const;

    /**
     * @brief Get units string
     * @return Units string
     */
    const std::string& getUnits() const;

private:
    float mMinValue;        // Minimum real value
    float mMaxValue;        // Maximum real value
    std::string mUnits;     // Units string
};

/**
 * @class IntParameter
 * @brief Parameter with integer values
 */
class IntParameter : public Parameter {
public:
    /**
     * @brief Constructor for integer parameter
     * @param id Unique parameter ID
     * @param name Parameter display name
     * @param shortName Short parameter name
     * @param minValue Minimum integer value
     * @param maxValue Maximum integer value
     * @param defaultValue Default integer value
     * @param flags Parameter behavior flags
     * @param units Optional units string
     * @param smoothingTimeMs Time in milliseconds for parameter smoothing
     */
    IntParameter(ParamID id,
                 const std::string& name,
                 const std::string& shortName,
                 int minValue,
                 int maxValue,
                 int defaultValue,
                 ParameterFlags flags = ParameterFlags::IsStepInteger,
                 const std::string& units = "",
                 float smoothingTimeMs = 20.0f);

    float denormalize(float normalizedValue) const override;
    float normalize(float realValue) const override;
    std::string toString(float normalizedValue) const override;
    bool fromString(const std::string& string, float& normalizedValue) const override;

    // IntParameter-specific implementation (placeholder)
    void fillParameterInfo() const override;

    /**
     * @brief Get integer value
     * @return Current integer value
     */
    int getIntValue() const;

    /**
     * @brief Get smoothed integer value
     * @param sampleRate Current sample rate for time calculations
     * @return Current smoothed integer value
     */
    int getSmoothedIntValue(float sampleRate);

    /**
     * @brief Set integer value
     * @param value Integer value to set
     */
    void setIntValue(int value);

    /**
     * @brief Get minimum integer value
     * @return Minimum integer value
     */
    int getMinInt() const;

    /**
     * @brief Get maximum integer value
     * @return Maximum integer value
     */
    int getMaxInt() const;

private:
    int mMinValue;          // Minimum integer value
    int mMaxValue;          // Maximum integer value
    std::string mUnits;     // Units string
};

/**
 * @class BoolParameter
 * @brief Parameter with boolean value (on/off)
 */
class BoolParameter : public Parameter {
public:
    /**
     * @brief Constructor for boolean parameter
     * @param id Unique parameter ID
     * @param name Parameter display name
     * @param shortName Short parameter name
     * @param defaultValue Default boolean value
     * @param flags Parameter behavior flags
     */
    BoolParameter(ParamID id,
                  const std::string& name,
                  const std::string& shortName,
                  bool defaultValue,
                  ParameterFlags flags = ParameterFlags::NoFlags);

    float denormalize(float normalizedValue) const override;
    float normalize(float realValue) const override;
    std::string toString(float normalizedValue) const override;
    bool fromString(const std::string& string, float& normalizedValue) const override;

    // BoolParameter-specific implementation (placeholder)
    void fillParameterInfo() const override;

    /**
     * @brief Get boolean value
     * @return Current boolean value
     */
    bool getBoolValue() const;

    /**
     * @brief Set boolean value
     * @param value Boolean value to set
     */
    void setBoolValue(bool value);
};

/**
 * @class EnumParameter
 * @brief Parameter with enumerated values
 */
class EnumParameter : public Parameter {
public:
    /**
     * @brief Constructor for enum parameter
     * @param id Unique parameter ID
     * @param name Parameter display name
     * @param shortName Short parameter name
     * @param options Available enum options
     * @param defaultValue Default option index
     * @param flags Parameter behavior flags
     */
    EnumParameter(ParamID id,
                  const std::string& name,
                  const std::string& shortName,
                  const std::vector<EnumValue>& options,
                  int defaultValue,
                  ParameterFlags flags = ParameterFlags::NoFlags);

    float denormalize(float normalizedValue) const override;
    float normalize(float realValue) const override;
    std::string toString(float normalizedValue) const override;
    bool fromString(const std::string& string, float& normalizedValue) const override;

    // EnumParameter-specific implementation (placeholder)
    void fillParameterInfo() const override;

    /**
     * @brief Get current enum value
     * @return Current enum value
     */
    int getEnumValue() const;

    /**
     * @brief Set current enum value
     * @param value Enum value to set
     */
    void setEnumValue(int value);

    /**
     * @brief Get current enum name
     * @return Current enum name
     */
    std::string getEnumName() const;

    /**
     * @brief Get all enum options
     * @return Vector of enum options
     */
    const std::vector<EnumValue>& getEnumOptions() const;

private:
    std::vector<EnumValue> mOptions;   // Available enum options
};

/**
 * @class ParameterManager
 * @brief Manages all parameters for the plugin
 */
class ParameterManager {
public:
    /**
     * @brief Constructor
     */
    ParameterManager();

    /**
     * @brief Destructor
     */
    ~ParameterManager();

    /**
     * @brief Register a new parameter
     * @param parameter Parameter to register
     * @return True if registration succeeded
     */
    bool registerParameter(std::shared_ptr<Parameter> parameter);

    /**
     * @brief Get parameter by ID
     * @param id Parameter ID to find
     * @return Shared pointer to parameter, or nullptr if not found
     */
    std::shared_ptr<Parameter> getParameter(ParamID id);

    /**
     * @brief Get parameter by ID (const version)
     * @param id Parameter ID to find
     * @return Shared pointer to parameter, or nullptr if not found
     */
    std::shared_ptr<const Parameter> getParameter(ParamID id) const;

    /**
     * @brief Set normalized parameter value
     * @param id Parameter ID
     * @param value Normalized value [0,1]
     * @return True if parameter was found and set
     */
    bool setParameterNormalized(ParamID id, float value);

    /**
     * @brief Get normalized parameter value
     * @param id Parameter ID
     * @param defaultValue Value to return if parameter not found
     * @return Normalized parameter value, or defaultValue if not found
     */
    float getParameterNormalized(ParamID id, float defaultValue = 0.0f) const;

    /**
     * @brief Get all parameters
     * @return Vector of all parameters
     */
    std::vector<std::shared_ptr<Parameter>> getAllParameters() const;

    /**
     * @brief Get parameter count
     * @return Number of registered parameters
     */
    size_t getParameterCount() const;

    /**
     * @brief Reset all parameters to default values
     */
    void resetToDefaults();

    /**
     * @brief Process parameter changes for the current audio block
     * @param sampleRate Current sample rate
     */
    void processParameterChanges(float sampleRate);

    /**
     * @brief Save parameter state to stream
     * @param stream Output stream
     * @return True if successful
     */
    bool saveState(std::ostream& stream) const;

    /**
     * @brief Load parameter state from stream
     * @param stream Input stream
     * @return True if successful
     */
    bool loadState(std::istream& stream);

private:
    std::unordered_map<ParamID, std::shared_ptr<Parameter>> mParameters;
    mutable std::mutex mParameterMutex;  // Mutex for thread-safe parameter access
    Logger mLogger;                      // Logger instance for parameter operations
};

} // namespace GranularPlunderphonics