/**
 * @file ParameterManager.cpp
 * @brief Implementation of parameter management system for GranularPlunderphonics plugin
 */

#include "ParameterManager.h"
#include <iomanip>
#include <sstream>
#include <cstring>
#include <algorithm>
#include <limits>

namespace GranularPlunderphonics {

//------------------------------------------------------------------------
// ParameterValue implementation
//------------------------------------------------------------------------
ParameterValue::ParameterValue(float initialValue, float smoothingTimeMs)
    : mTargetValue(initialValue)
    , mCurrentValue(initialValue)
    , mSmoothingTime(smoothingTimeMs)
    , mNeedsSmoothing(false)
{
}

void ParameterValue::setTarget(float newValue) {
    // Only set as needing smoothing if the value actually changes
    if (mTargetValue.load() != newValue) {
        mTargetValue.store(newValue);
        mNeedsSmoothing.store(true);
    }
}

float ParameterValue::getSmoothed(float sampleRate) {
    if (!mNeedsSmoothing.load()) {
        return mCurrentValue.load();
    }

    // Calculate smoothing coefficient based on sample rate and desired time
    float smoothingTimeSeconds = mSmoothingTime.load() / 1000.0f;
    float samplesForSmoothing = smoothingTimeSeconds * sampleRate;

    // Avoid division by zero
    if (samplesForSmoothing < 1.0f) {
        samplesForSmoothing = 1.0f;
    }

    float coeff = 1.0f / samplesForSmoothing;

    // Ensure coefficient is in valid range (0,1)
    coeff = std::min(1.0f, std::max(0.0001f, coeff));

    // Apply smoothing formula: current = current + coeff * (target - current)
    float current = mCurrentValue.load();
    float target = mTargetValue.load();
    float newValue = current + coeff * (target - current);

    // If we're close enough to target, snap to it and mark smoothing as complete
    if (std::abs(newValue - target) < 0.0001f) {
        newValue = target;
        mNeedsSmoothing.store(false);
    }

    mCurrentValue.store(newValue);
    return newValue;
}

float ParameterValue::getTarget() const {
    return mTargetValue.load();
}

void ParameterValue::setSmoothingTime(float timeMs) {
    mSmoothingTime.store(std::max(0.0f, timeMs));
}

void ParameterValue::resetSmoothing() {
    mCurrentValue.store(mTargetValue.load());
    mNeedsSmoothing.store(false);
}

//------------------------------------------------------------------------
// Parameter base class implementation
//------------------------------------------------------------------------
Parameter::Parameter(ParamID id,
                   const std::string& name,
                   const std::string& shortName,
                   ParameterType type,
                   ParameterFlags flags,
                   float smoothingTimeMs)
    : mId(id)
    , mName(name)
    , mShortName(shortName)
    , mType(type)
    , mFlags(flags)
    , mValue(0.0f, smoothingTimeMs)
    , mDefaultNormalizedValue(0.0f)
    , mChangeCallback(nullptr)
{
}

ParamID Parameter::getId() const {
    return mId;
}

const std::string& Parameter::getName() const {
    return mName;
}

const std::string& Parameter::getShortName() const {
    return mShortName;
}

ParameterType Parameter::getType() const {
    return mType;
}

ParameterFlags Parameter::getFlags() const {
    return mFlags;
}

bool Parameter::hasFlag(ParameterFlags flag) const {
    return ::GranularPlunderphonics::hasFlag(mFlags, flag);
}

float Parameter::getDefaultNormalizedValue() const {
    return mDefaultNormalizedValue;
}

void Parameter::setNormalized(float normalizedValue) {
    // Clamp to valid range
    normalizedValue = std::min(1.0f, std::max(0.0f, normalizedValue));
    mValue.setTarget(normalizedValue);

    // Notify callback if registered
    if (mChangeCallback) {
        mChangeCallback(normalizedValue);
    }
}

float Parameter::getNormalized() const {
    return mValue.getTarget();
}

float Parameter::getSmoothedNormalized(float sampleRate) {
    return mValue.getSmoothed(sampleRate);
}

void Parameter::setReal(float realValue) {
    setNormalized(normalize(realValue));
}

float Parameter::getReal() const {
    return denormalize(getNormalized());
}

float Parameter::getSmoothedReal(float sampleRate) {
    return denormalize(getSmoothedNormalized(sampleRate));
}

void Parameter::resetSmoothing() {
    mValue.resetSmoothing();
}

void Parameter::setChangeCallback(std::function<void(float)> callback) {
    mChangeCallback = callback;
}

void Parameter::fillParameterInfo() const {
    // Empty base implementation that derived classes will override
}

//------------------------------------------------------------------------
// FloatParameter implementation
//------------------------------------------------------------------------
FloatParameter::FloatParameter(ParamID id,
                             const std::string& name,
                             const std::string& shortName,
                             float minValue,
                             float maxValue,
                             float defaultValue,
                             ParameterFlags flags,
                             const std::string& units,
                             float smoothingTimeMs)
    : Parameter(id, name, shortName, ParameterType::Float, flags, smoothingTimeMs)
    , mMinValue(minValue)
    , mMaxValue(maxValue)
    , mUnits(units)
{
    // Ensure min < max
    if (mMinValue > mMaxValue) {
        std::swap(mMinValue, mMaxValue);
    }

    // Clamp default value to valid range
    defaultValue = std::min(mMaxValue, std::max(mMinValue, defaultValue));

    // Calculate and store default normalized value
    mDefaultNormalizedValue = normalize(defaultValue);

    // Initialize parameter with default value
    setNormalized(mDefaultNormalizedValue);
}

float FloatParameter::denormalize(float normalizedValue) const {
    // Clamp normalized value to [0,1]
    normalizedValue = std::min(1.0f, std::max(0.0f, normalizedValue));

    // Handle logarithmic scaling if requested
    if (hasFlag(ParameterFlags::IsLogarithmic)) {
        // Ensure min/max are positive for logarithmic scaling
        float logMin = std::max(1e-7f, mMinValue);
        float logMax = std::max(logMin * 1.1f, mMaxValue);

        // Log interpolation formula: result = min * (max/min)^normalized
        return logMin * std::pow(logMax / logMin, normalizedValue);
    }

    // Linear scaling
    return mMinValue + normalizedValue * (mMaxValue - mMinValue);
}

float FloatParameter::normalize(float realValue) const {
    // Clamp real value to valid range
    realValue = std::min(mMaxValue, std::max(mMinValue, realValue));

    // Handle logarithmic scaling if requested
    if (hasFlag(ParameterFlags::IsLogarithmic)) {
        // Ensure min/max are positive for logarithmic scaling
        float logMin = std::max(1e-7f, mMinValue);
        float logMax = std::max(logMin * 1.1f, mMaxValue);

        // Ensure real value is in valid log range
        realValue = std::max(logMin, realValue);

        // Log normalization formula: normalized = log(value/min) / log(max/min)
        return std::log(realValue / logMin) / std::log(logMax / logMin);
    }

    // Linear normalization
    if (mMaxValue == mMinValue) {
        return 0.0f; // Avoid division by zero
    }

    return (realValue - mMinValue) / (mMaxValue - mMinValue);
}

std::string FloatParameter::toString(float normalizedValue) const {
    float realValue = denormalize(normalizedValue);

    // Format based on value range
    std::ostringstream oss;
    float range = mMaxValue - mMinValue;

    if (range < 0.1f) {
        // Small range values need more precision
        oss << std::fixed << std::setprecision(5);
    } else if (range < 1.0f) {
        oss << std::fixed << std::setprecision(3);
    } else if (range < 10.0f) {
        oss << std::fixed << std::setprecision(2);
    } else {
        oss << std::fixed << std::setprecision(1);
    }

    oss << realValue;

    // Append units if present
    if (!mUnits.empty()) {
        oss << " " << mUnits;
    }

    return oss.str();
}

bool FloatParameter::fromString(const std::string& str, float& normalizedValue) const {
    try {
        // Extract numeric part from string (strip units if present)
        std::istringstream iss(str);
        float realValue;
        iss >> realValue;

        if (iss.fail()) {
            return false;
        }

        // Convert to normalized value
        normalizedValue = normalize(realValue);
        return true;
    }
    catch (...) {
        return false;
    }
}

void FloatParameter::fillParameterInfo() const {
    // Placeholder implementation for when VST3 SDK is available
}

float FloatParameter::getMin() const {
    return mMinValue;
}

float FloatParameter::getMax() const {
    return mMaxValue;
}

const std::string& FloatParameter::getUnits() const {
    return mUnits;
}

//------------------------------------------------------------------------
// IntParameter implementation
//------------------------------------------------------------------------
IntParameter::IntParameter(ParamID id,
                         const std::string& name,
                         const std::string& shortName,
                         int minValue,
                         int maxValue,
                         int defaultValue,
                         ParameterFlags flags,
                         const std::string& units,
                         float smoothingTimeMs)
    : Parameter(id, name, shortName, ParameterType::Integer, flags | ParameterFlags::IsStepInteger, smoothingTimeMs)
    , mMinValue(minValue)
    , mMaxValue(maxValue)
    , mUnits(units)
{
    // Ensure min < max
    if (mMinValue > mMaxValue) {
        std::swap(mMinValue, mMaxValue);
    }

    // Clamp default value to valid range
    defaultValue = std::min(mMaxValue, std::max(mMinValue, defaultValue));

    // Calculate and store default normalized value
    mDefaultNormalizedValue = normalize(static_cast<float>(defaultValue));

    // Initialize parameter with default value
    setNormalized(mDefaultNormalizedValue);
}

float IntParameter::denormalize(float normalizedValue) const {
    // Clamp normalized value to [0,1]
    normalizedValue = std::min(1.0f, std::max(0.0f, normalizedValue));

    // Calculate real float value
    float realFloatValue;
    if (hasFlag(ParameterFlags::IsLogarithmic)) {
        // Ensure min/max are positive for logarithmic scaling
        int logMin = std::max(1, mMinValue);
        int logMax = std::max(logMin + 1, mMaxValue);

        // Log interpolation formula: result = min * (max/min)^normalized
        realFloatValue = logMin * std::pow(static_cast<float>(logMax) / logMin, normalizedValue);
    } else {
        // Linear scaling
        realFloatValue = mMinValue + normalizedValue * (mMaxValue - mMinValue);
    }

    // Round to nearest integer
    return std::round(realFloatValue);
}

float IntParameter::normalize(float realValue) const {
    // Round to nearest integer and clamp to valid range
    int intValue = static_cast<int>(std::round(realValue));
    intValue = std::min(mMaxValue, std::max(mMinValue, intValue));

    // Handle logarithmic scaling if requested
    if (hasFlag(ParameterFlags::IsLogarithmic)) {
        // Ensure min/max are positive for logarithmic scaling
        int logMin = std::max(1, mMinValue);
        int logMax = std::max(logMin + 1, mMaxValue);

        // Ensure real value is in valid log range
        intValue = std::max(logMin, intValue);

        // Log normalization formula: normalized = log(value/min) / log(max/min)
        return std::log(static_cast<float>(intValue) / logMin) /
               std::log(static_cast<float>(logMax) / logMin);
    }

    // Linear normalization
    if (mMaxValue == mMinValue) {
        return 0.0f; // Avoid division by zero
    }

    return static_cast<float>(intValue - mMinValue) / (mMaxValue - mMinValue);
}

std::string IntParameter::toString(float normalizedValue) const {
    int realValue = static_cast<int>(denormalize(normalizedValue));

    std::ostringstream oss;
    oss << realValue;

    // Append units if present
    if (!mUnits.empty()) {
        oss << " " << mUnits;
    }

    return oss.str();
}

bool IntParameter::fromString(const std::string& str, float& normalizedValue) const {
    try {
        // Extract numeric part from string (strip units if present)
        std::istringstream iss(str);
        int realValue;
        iss >> realValue;

        if (iss.fail()) {
            return false;
        }

        // Convert to normalized value
        normalizedValue = normalize(static_cast<float>(realValue));
        return true;
    }
    catch (...) {
        return false;
    }
}

void IntParameter::fillParameterInfo() const {
    // Placeholder implementation for when VST3 SDK is available
}

int IntParameter::getIntValue() const {
    return static_cast<int>(denormalize(getNormalized()));
}

int IntParameter::getSmoothedIntValue(float sampleRate) {
    return static_cast<int>(denormalize(getSmoothedNormalized(sampleRate)));
}

void IntParameter::setIntValue(int value) {
    setReal(static_cast<float>(value));
}

int IntParameter::getMinInt() const {
    return mMinValue;
}

int IntParameter::getMaxInt() const {
    return mMaxValue;
}

//------------------------------------------------------------------------
// BoolParameter implementation
//------------------------------------------------------------------------
BoolParameter::BoolParameter(ParamID id,
                           const std::string& name,
                           const std::string& shortName,
                           bool defaultValue,
                           ParameterFlags flags)
    : Parameter(id, name, shortName, ParameterType::Boolean, flags, 0.0f) // No smoothing for boolean
{
    mDefaultNormalizedValue = defaultValue ? 1.0f : 0.0f;
    setNormalized(mDefaultNormalizedValue);
}

float BoolParameter::denormalize(float normalizedValue) const {
    // Threshold at 0.5
    return normalizedValue >= 0.5f ? 1.0f : 0.0f;
}

float BoolParameter::normalize(float realValue) const {
    // Threshold at 0.5
    return realValue >= 0.5f ? 1.0f : 0.0f;
}

std::string BoolParameter::toString(float normalizedValue) const {
    bool boolValue = normalizedValue >= 0.5f;
    return boolValue ? "On" : "Off";
}

bool BoolParameter::fromString(const std::string& str, float& normalizedValue) const {
    std::string lowerStr = str;
    // Convert to lowercase for case-insensitive comparison
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    if (lowerStr == "on" || lowerStr == "yes" || lowerStr == "true" || lowerStr == "1") {
        normalizedValue = 1.0f;
        return true;
    } else if (lowerStr == "off" || lowerStr == "no" || lowerStr == "false" || lowerStr == "0") {
        normalizedValue = 0.0f;
        return true;
    }

    return false;
}

void BoolParameter::fillParameterInfo() const {
    // Placeholder implementation for when VST3 SDK is available
}

bool BoolParameter::getBoolValue() const {
    return getNormalized() >= 0.5f;
}

void BoolParameter::setBoolValue(bool value) {
    setNormalized(value ? 1.0f : 0.0f);
}

//------------------------------------------------------------------------
// EnumParameter implementation
//------------------------------------------------------------------------
EnumParameter::EnumParameter(ParamID id,
                           const std::string& name,
                           const std::string& shortName,
                           const std::vector<EnumValue>& options,
                           int defaultValue,
                           ParameterFlags flags)
    : Parameter(id, name, shortName, ParameterType::Enum, flags | ParameterFlags::IsStepInteger, 0.0f) // No smoothing for enum
    , mOptions(options)
{
    // Validate default value
    if (mOptions.empty()) {
        // If no options provided, create a dummy option
        mOptions.push_back({0, "Default", "Def"});
        defaultValue = 0;
    } else {
        // Find the option with the specified default value, or use the first option
        auto it = std::find_if(mOptions.begin(), mOptions.end(),
                             [defaultValue](const EnumValue& opt) { return opt.value == defaultValue; });

        if (it == mOptions.end()) {
            defaultValue = mOptions.front().value;
        }
    }

    // Set default value
    float normalizedDefault = normalize(static_cast<float>(defaultValue));
    mDefaultNormalizedValue = normalizedDefault;
    setNormalized(normalizedDefault);
}

float EnumParameter::denormalize(float normalizedValue) const {
    if (mOptions.empty()) {
        return 0.0f;
    }

    // Clamp normalized value to [0,1]
    normalizedValue = std::min(1.0f, std::max(0.0f, normalizedValue));

    // Calculate the index in the options array
    int numOptions = static_cast<int>(mOptions.size());
    int index = std::min(numOptions - 1, static_cast<int>(normalizedValue * numOptions));

    // Return the enum value at the calculated index
    return static_cast<float>(mOptions[index].value);
}

float EnumParameter::normalize(float realValue) const {
    if (mOptions.empty()) {
        return 0.0f;
    }

    // Find the option with the closest value
    int intValue = static_cast<int>(std::round(realValue));
    auto it = std::find_if(mOptions.begin(), mOptions.end(),
                         [intValue](const EnumValue& opt) { return opt.value == intValue; });

    // If not found, use the first option
    int index = (it != mOptions.end()) ?
                std::distance(mOptions.begin(), it) : 0;

    // Convert index to normalized value
    return static_cast<float>(index) / (mOptions.size() - 1);
}

std::string EnumParameter::toString(float normalizedValue) const {
    if (mOptions.empty()) {
        return "Invalid";
    }

    // Get the enum value
    int enumValue = static_cast<int>(denormalize(normalizedValue));

    // Find the option with this value
    auto it = std::find_if(mOptions.begin(), mOptions.end(),
                         [enumValue](const EnumValue& opt) { return opt.value == enumValue; });

    // Return the option name, or a default if not found
    return (it != mOptions.end()) ? it->name : "Unknown";
}

bool EnumParameter::fromString(const std::string& str, float& normalizedValue) const {
    if (mOptions.empty()) {
        return false;
    }

    // Try to match by name or short name
    auto it = std::find_if(mOptions.begin(), mOptions.end(),
                         [&str](const EnumValue& opt) {
                             return opt.name == str || opt.shortName == str;
                         });

    if (it != mOptions.end()) {
        // Found a match - convert to normalized value
        normalizedValue = static_cast<float>(std::distance(mOptions.begin(), it)) /
                        (mOptions.size() - 1);
        return true;
    }

    // Try parsing as a numeric value
    try {
        int intValue;
        std::istringstream iss(str);
        iss >> intValue;

        if (!iss.fail()) {
            // Try to find the enum with this value
            auto valueIt = std::find_if(mOptions.begin(), mOptions.end(),
                                     [intValue](const EnumValue& opt) { return opt.value == intValue; });

            if (valueIt != mOptions.end()) {
                normalizedValue = static_cast<float>(std::distance(mOptions.begin(), valueIt)) /
                                (mOptions.size() - 1);
                return true;
            }
        }
    }
    catch (...) {
        // Ignore parsing errors
    }

    return false;
}

void EnumParameter::fillParameterInfo() const {
    // Placeholder implementation for when VST3 SDK is available
}

int EnumParameter::getEnumValue() const {
    return static_cast<int>(denormalize(getNormalized()));
}

void EnumParameter::setEnumValue(int value) {
    // Find the option with this value
    auto it = std::find_if(mOptions.begin(), mOptions.end(),
                         [value](const EnumValue& opt) { return opt.value == value; });

    if (it != mOptions.end()) {
        float normalizedValue = static_cast<float>(std::distance(mOptions.begin(), it)) /
                              (mOptions.size() - 1);
        setNormalized(normalizedValue);
    }
}

std::string EnumParameter::getEnumName() const {
    return toString(getNormalized());
}

const std::vector<EnumValue>& EnumParameter::getEnumOptions() const {
    return mOptions;
}

//------------------------------------------------------------------------
// ParameterManager implementation
//------------------------------------------------------------------------
ParameterManager::ParameterManager()
    : mLogger("ParameterManager")
{
    // No logger call needed during construction
}

ParameterManager::~ParameterManager()
{
    // No logger call needed during destruction
}

bool ParameterManager::registerParameter(std::shared_ptr<Parameter> parameter)
{
    if (!parameter) {
        mLogger.error("Attempted to register null parameter");
        return false;
    }

    ParamID id = parameter->getId();

    std::lock_guard<std::mutex> lock(mParameterMutex);

    // Check if parameter with this ID already exists
    if (mParameters.find(id) != mParameters.end()) {
        mLogger.error("Parameter with ID already registered");
        return false;
    }

    // Register the parameter
    mParameters[id] = parameter;
    mLogger.debug("Registered parameter");

    return true;
}

std::shared_ptr<Parameter> ParameterManager::getParameter(ParamID id)
{
    std::lock_guard<std::mutex> lock(mParameterMutex);

    auto it = mParameters.find(id);
    if (it != mParameters.end()) {
        return it->second;
    }

    return nullptr;
}

std::shared_ptr<const Parameter> ParameterManager::getParameter(ParamID id) const
{
    std::lock_guard<std::mutex> lock(mParameterMutex);

    auto it = mParameters.find(id);
    if (it != mParameters.end()) {
        return it->second;
    }

    return nullptr;
}

bool ParameterManager::setParameterNormalized(ParamID id, float value)
{
    auto parameter = getParameter(id);
    if (!parameter) {
        mLogger.debug("Attempted to set non-existent parameter");
        return false;
    }

    parameter->setNormalized(value);
    return true;
}

float ParameterManager::getParameterNormalized(ParamID id, float defaultValue) const
{
    auto parameter = getParameter(id);
    if (!parameter) {
        return defaultValue;
    }

    return parameter->getNormalized();
}

std::vector<std::shared_ptr<Parameter>> ParameterManager::getAllParameters() const
{
    std::lock_guard<std::mutex> lock(mParameterMutex);

    std::vector<std::shared_ptr<Parameter>> result;
    result.reserve(mParameters.size());

    for (const auto& pair : mParameters) {
        result.push_back(pair.second);
    }

    return result;
}

size_t ParameterManager::getParameterCount() const
{
    std::lock_guard<std::mutex> lock(mParameterMutex);
    return mParameters.size();
}

void ParameterManager::resetToDefaults()
{
    std::lock_guard<std::mutex> lock(mParameterMutex);

    for (auto& pair : mParameters) {
        auto& param = pair.second;
        param->setNormalized(param->getDefaultNormalizedValue());
    }

    mLogger.info("All parameters reset to defaults");
}

void ParameterManager::processParameterChanges(float sampleRate)
{
    // Process all parameters that need smoothing
    // This should be called from the audio thread at the start of each block
    std::lock_guard<std::mutex> lock(mParameterMutex);

    for (auto& pair : mParameters) {
        auto& param = pair.second;
        param->getSmoothedNormalized(sampleRate);
    }
}

    bool ParameterManager::saveState(std::ostream& stream) const
{
    std::lock_guard<std::mutex> lock(mParameterMutex);

    try {
        // Write the number of parameters
        size_t paramCount = mParameters.size();
        stream.write(reinterpret_cast<const char*>(&paramCount), sizeof(paramCount));

        // Write each parameter's ID and normalized value
        for (const auto& pair : mParameters) {
            ParamID id = pair.first;
            float value = pair.second->getNormalized();

            stream.write(reinterpret_cast<const char*>(&id), sizeof(id));
            stream.write(reinterpret_cast<const char*>(&value), sizeof(value));
        }

        mLogger.info("Saved parameter state");  // Added 'template' keyword
        return true;
    }
    catch (const std::exception& e) {
        mLogger.error("Failed to save parameter state");  // Added 'template' keyword
        return false;
    }
}


}