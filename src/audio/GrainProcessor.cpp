#include "GrainProcessor.h"

namespace GranularPlunderphonics {

float GrainProcessor::interpolateSample(const float* buffer, std::size_t size, 
                                      float position, InterpolationType type) {
    switch(type) {
        case InterpolationType::Linear: {
            int pos0 = static_cast<int>(std::floor(position));
            int pos1 = pos0 + 1;
            float frac = position - pos0;
            
            if (pos0 >= 0 && pos1 < static_cast<int>(size)) {
                return buffer[pos0] + frac * (buffer[pos1] - buffer[pos0]);
            }
            return 0.0f;
        }
        
        case InterpolationType::Cubic: {
            int pos1 = static_cast<int>(std::floor(position));
            int pos0 = pos1 - 1;
            int pos2 = pos1 + 1;
            int pos3 = pos1 + 2;
            float frac = position - pos1;
            
            float y0 = (pos0 >= 0) ? buffer[pos0] : buffer[pos1];
            float y1 = buffer[pos1];
            float y2 = (pos2 < static_cast<int>(size)) ? buffer[pos2] : y1;
            float y3 = (pos3 < static_cast<int>(size)) ? buffer[pos3] : y2;
            
            float a0 = y3 - y2 - y0 + y1;
            float a1 = y0 - y1 - a0;
            float a2 = y2 - y0;
            float a3 = y1;
            
            return a0 * frac * frac * frac + a1 * frac * frac + a2 * frac + a3;
        }
        
        case InterpolationType::Sinc4:
            return sincInterpolate(buffer, size, position, 4);
            
        case InterpolationType::Sinc8:
            return sincInterpolate(buffer, size, position, 8);
            
        default:
            return buffer[static_cast<int>(std::round(position))];
    }
}

void GrainProcessor::processPhaseVocoder(fftwf_complex* freqData,
                                       std::vector<float>& previousPhase,
                                       std::vector<float>& synthesisPhase,
                                       const ProcessingParameters& params) {
    const float pi2 = 2.0f * static_cast<float>(M_PI);
    const std::size_t binCount = mFFTSize/2 + 1;
    
    std::vector<bool> isLockedPhase(binCount, false);
    if (params.vocoderSettings.phaseLocking) {
        detectTransients(freqData, binCount, params.vocoderSettings.transientThreshold, isLockedPhase);
    }
    
    for (std::size_t bin = 0; bin < binCount; ++bin) {
        float magnitude = std::sqrt(freqData[bin][0] * freqData[bin][0] + 
                                  freqData[bin][1] * freqData[bin][1]);
        float phase = std::atan2(freqData[bin][1], freqData[bin][0]);
        
        float phaseDiff = phase - previousPhase[bin];
        phaseDiff = phaseDiff - pi2 * std::round(phaseDiff / pi2);
        
        float omega = pi2 * bin * params.vocoderSettings.analysisHopSize / mFFTSize;
        float freq = omega + phaseDiff;
        
        float newPhase;
        if (isLockedPhase[bin]) {
            newPhase = phase;
        } else {
            newPhase = synthesisPhase[bin] + 
                      freq * params.pitchShift * params.timeStretch;
        }
        
        previousPhase[bin] = phase;
        synthesisPhase[bin] = newPhase;
        
        freqData[bin][0] = magnitude * std::cos(newPhase);
        freqData[bin][1] = magnitude * std::sin(newPhase);
    }
}

void GrainProcessor::detectTransients(fftwf_complex* freqData,
                                    std::size_t binCount,
                                    float threshold,
                                    std::vector<bool>& isLockedPhase) {
    float flux = 0.0f;
    for (std::size_t bin = 0; bin < binCount; ++bin) {
        float magnitude = std::sqrt(freqData[bin][0] * freqData[bin][0] + 
                                  freqData[bin][1] * freqData[bin][1]);
        float diff = magnitude - mPreviousMagnitudes[bin];
        if (diff > 0) {
            flux += diff;
        }
        mPreviousMagnitudes[bin] = magnitude;
    }
    
    if (flux > threshold) {
        std::fill(isLockedPhase.begin(), isLockedPhase.end(), true);
    }
}

std::vector<float> GrainProcessor::createWindow(std::size_t size) {
    std::vector<float> window(size);
    for (std::size_t i = 0; i < size; ++i) {
        window[i] = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (size - 1)));
    }
    return window;
}

void GrainProcessor::processGrain(AudioBuffer& grain, const ProcessingParameters& params) {
    if (params.timeStretch == 1.0f && params.pitchShift == 1.0f) {
        return;
    }

    std::vector<float> windowedData(mFFTSize);
    std::vector<float> outputData(grain.getNumSamples());
    std::vector<float> window = createWindow(mFFTSize);
    
    std::vector<float> previousPhase(mFFTSize/2 + 1, 0.0f);
    std::vector<float> synthesisPhase(mFFTSize/2 + 1, 0.0f);
    
    for (std::size_t i = 0; i < grain.getNumSamples(); 
         i += params.vocoderSettings.synthesisHopSize) {
        
        for (std::size_t j = 0; j < mFFTSize; ++j) {
            float pos = static_cast<float>(i + j);
            if (pos < grain.getNumSamples()) {
                windowedData[j] = grain.getSample(0, pos) * window[j];
            } else {
                windowedData[j] = 0.0f;
            }
        }
        
        std::memcpy(mTimeData, windowedData.data(), mFFTSize * sizeof(float));
        fftwf_execute(mForwardPlan);
        
        processPhaseVocoder(mFreqData, previousPhase, synthesisPhase, params);
        
        fftwf_execute(mInversePlan);
        
        for (std::size_t j = 0; j < mFFTSize; ++j) {
            std::size_t outPos = i + j;
            if (outPos < outputData.size()) {
                outputData[outPos] += mTimeData[j] * window[j] / (mFFTSize * 0.5f);
            }
        }
    }
    
    for (std::size_t i = 0; i < grain.getNumSamples(); ++i) {
        grain.write(0, &outputData[i], 1, i);
    }
}

} // namespace GranularPlunderphonics