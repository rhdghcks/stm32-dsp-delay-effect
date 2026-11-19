#pragma once

#include <cstddef>

namespace pedal
{
constexpr float kSampleRate = 48000.0f;
constexpr std::size_t kBlockFrames = 48;
constexpr float kMinDelayMs = 20.0f;
constexpr float kMaxDelayMs = 2000.0f;
constexpr std::size_t kDelayCapacity =
    static_cast<std::size_t>(kSampleRate * kMaxDelayMs / 1000.0f) + 2;

struct Settings
{
    float delay_ms = 350.0f;
    float mix = 0.35f;
    float feedback = 0.40f;
    float tone = 0.75f;
    bool enabled = false;
    bool glide = false;
};

class Delay
{
  public:
    void Init(float (&memory)[kDelayCapacity], const Settings& initial);
    void Set(const Settings& settings);
    float Process(float input);

  private:
    float Read(float delay_samples) const;
    float ReadTransition();

    float* memory_ = nullptr;
    std::size_t write_ = 0;

    float delay_ = 0.0f;
    float target_delay_ = 0.0f;
    float fade_to_ = 0.0f;
    unsigned fade_remaining_ = 0;
    bool glide_ = false;

    float mix_ = 0.0f;
    float feedback_ = 0.0f;
    float tone_alpha_ = 0.0f;
    float mix_target_ = 0.0f;
    float feedback_target_ = 0.0f;
    float tone_target_ = 0.0f;
    float filtered_ = 0.0f;

    float engage_ = 0.0f;
    float engage_target_ = 0.0f;
};
} // namespace pedal
