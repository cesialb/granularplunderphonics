#pragma once

#include "pluginterfaces/vst/vsttypes.h"
#include "../common/Logger.h"
#include <string>
#include <vector>
#include <map>
#include <atomic>
#include <functional>
#include <ostream>

namespace GranularPlunderphonics {
    namespace Vst = Steinberg::Vst;
    using ParamID = Vst::ParamID;

    // Forward declarations
    class Parameter;
    class ParameterValue;

    // Enums
    enum class ParameterType {
        Float,
        Integer,
        Boolean,
        Enum
    };

    enum class ParameterFlags {
        NoFlags = 0,
        IsBypass = 1 << 0,
        IsLogarithmic = 1 << 1,
        IsStepInteger = 1 << 2
    };

    inline ParameterFlags operator|(ParameterFlags a, ParameterFlags b) {
        return static_cast<ParameterFlags>(
            static_cast<int>(a) | static_cast<int>(b)
        );
    }

    inline bool hasFlag(ParameterFlags flags, ParameterFlags flag) {
        return (static_cast<int>(flags) & static_cast<int>(flag)) != 0;
    }

    // Enum value structure
    struct EnumValue {
        int value;
        std::string name;
        std::string shortName;
    };

    // Parameter value class for smoothing
    class ParameterValue {
    public:
        ParameterValue(float initialValue, float smoothingTimeMs);
        void setTarget(float newValue);
        float getSmoothed(float sampleRate);
        float getTarget() const;
        void setSmoothingTime(float timeMs);
        void resetSmoothing();

    private:
        std::atomic<float> mTargetValue;
        std::atomic<float> mCurrentValue;
        std::atomic<float> mSmoothingTime;
        std::atomic<bool> mNeedsSmoothing;
    };

    // Base Parameter class
    class Parameter {
    public:
        Parameter(ParamID id,
                 const std::string& name,
                 const std::string& shortName,
                 ParameterType type,
                 ParameterFlags flags,
                 float smoothingTimeMs);
        virtual ~Parameter() = default;

        ParamID getId() const;
        const std::string& getName() const;
        const std::string& getShortName() const;
        ParameterType getType() const;
        ParameterFlags getFlags() const;
        bool hasFlag(ParameterFlags flag) const;
        float getDefaultNormalizedValue() const;

        virtual void setNormalized(float normalizedValue);
        virtual float getNormalized() const;
        virtual float getSmoothedNormalized(float sampleRate);
        virtual void setReal(float realValue);
        virtual float getReal() const;
        virtual float getSmoothedReal(float sampleRate);
        virtual void resetSmoothing();

        virtual float denormalize(float normalizedValue) const = 0;
        virtual float normalize(float realValue) const = 0;
        virtual std::string toString(float normalizedValue) const = 0;
        virtual bool fromString(const std::string& str, float& normalizedValue) const = 0;
        virtual void fillParameterInfo() const = 0;

        void setChangeCallback(std::function<void(float)> callback);

    protected:
        ParamID mId;
        std::string mName;
        std::string mShortName;
        ParameterType mType;
        ParameterFlags mFlags;
        ParameterValue mValue;
        float mDefaultNormalizedValue;
        std::function<void(float)> mChangeCallback;
    };

    // Float Parameter
    class FloatParameter : public Parameter {
    public:
        FloatParameter(ParamID id,
                      const std::string& name,
                      const std::string& shortName,
                      float minValue,
                      float maxValue,
                      float defaultValue,
                      ParameterFlags flags = ParameterFlags::NoFlags,
                      const std::string& units = "",
                      float smoothingTimeMs = 0.0f);

        float denormalize(float normalizedValue) const override;
        float normalize(float realValue) const override;
        std::string toString(float normalizedValue) const override;
        bool fromString(const std::string& str, float& normalizedValue) const override;
        void fillParameterInfo() const override;

        float getMin() const;
        float getMax() const;
        const std::string& getUnits() const;

    private:
        float mMinValue;
        float mMaxValue;
        std::string mUnits;
    };

    // Integer Parameter
    class IntParameter : public Parameter {
    public:
        IntParameter(ParamID id,
                    const std::string& name,
                    const std::string& shortName,
                    int minValue,
                    int maxValue,
                    int defaultValue,
                    ParameterFlags flags = ParameterFlags::NoFlags,
                    const std::string& units = "",
                    float smoothingTimeMs = 0.0f);

        float denormalize(float normalizedValue) const override;
        float normalize(float realValue) const override;
        std::string toString(float normalizedValue) const override;
        bool fromString(const std::string& str, float& normalizedValue) const override;
        void fillParameterInfo() const override;

        int getIntValue() const;
        int getSmoothedIntValue(float sampleRate);
        void setIntValue(int value);
        int getMinInt() const;
        int getMaxInt() const;

    private:
        int mMinValue;
        int mMaxValue;
        std::string mUnits;
    };

    // Boolean Parameter
    class BoolParameter : public Parameter {
    public:
        BoolParameter(ParamID id,
                     const std::string& name,
                     const std::string& shortName,
                     bool defaultValue,
                     ParameterFlags flags = ParameterFlags::NoFlags);

        float denormalize(float normalizedValue) const override;
        float normalize(float realValue) const override;
        std::string toString(float normalizedValue) const override;
        bool fromString(const std::string& str, float& normalizedValue) const override;
        void fillParameterInfo() const override;

        bool getBoolValue() const;
        void setBoolValue(bool value);
    };

    // Enum Parameter
    class EnumParameter : public Parameter {
    public:
        EnumParameter(ParamID id,
                     const std::string& name,
                     const std::string& shortName,
                     const std::vector<EnumValue>& options,
                     int defaultValue,
                     ParameterFlags flags = ParameterFlags::NoFlags);

        float denormalize(float normalizedValue) const override;
        float normalize(float realValue) const override;
        std::string toString(float normalizedValue) const override;
        bool fromString(const std::string& str, float& normalizedValue) const override;
        void fillParameterInfo() const override;

        int getEnumValue() const;
        void setEnumValue(int value);
        std::string getEnumName() const;
        const std::vector<EnumValue>& getEnumOptions() const;

    private:
        std::vector<EnumValue> mOptions;
    };

    // Parameter Manager
    class ParameterManager {
    public:
        ParameterManager();
        ~ParameterManager();

        bool registerParameter(std::shared_ptr<Parameter> parameter);
        std::shared_ptr<Parameter> getParameter(ParamID id);
        std::shared_ptr<const Parameter> getParameter(ParamID id) const;
        bool setParameterNormalized(ParamID id, float value);
        float getParameterNormalized(ParamID id, float defaultValue = 0.0f) const;
        std::vector<std::shared_ptr<Parameter>> getAllParameters() const;
        size_t getParameterCount() const;
        void resetToDefaults();
        void processParameterChanges(float sampleRate);
        bool saveState(std::ostream& stream) const;

    private:
        std::map<ParamID, std::shared_ptr<Parameter>> mParameters;
        mutable std::mutex mParameterMutex;
        Logger mLogger;
    };

} // namespace GranularPlunderphonics
