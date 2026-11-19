#include "delay.h"

namespace pedal
{
namespace
{
constexpr float kSamplesPerMs = kSampleRate / 1000.0f;
constexpr unsigned kFadeSamples = 1200; // 25 ms
constexpr float kGlideStep = 0.25f;      // samples of delay change per audio sample
constexpr float kCrossfadeThreshold = 8.0f;
constexpr float kControlSmoothing = 0.003f;
constexpr float kBypassStep = 1.0f / 480.0f; // 10 ms
constexpr float kFeedbackCeiling = 0.94f;

float Clamp(float value, float lo, float hi)
{
    if (value < lo)
        return lo;
    if (value > hi)
        return hi;
    return value;
}

float Interpolate(float from, float to, float amount)
{
    return from + (to - from) * amount;
}
} // namespace

void Delay::Init(float (&memory)[kDelayCapacity], const Settings& initial)
{
    memory_ = memory;
    for (float& sample : memory)
        sample = 0.0f;

    write_ = 0;
    fade_remaining_ = 0;
    filtered_ = 0.0f;

    Set(initial);
    delay_ = fade_to_ = target_delay_;
    mix_ = mix_target_;
    feedback_ = feedback_target_;
    tone_alpha_ = tone_target_;
    engage_ = engage_target_;
}

void Delay::Set(const Settings& settings)
{
    target_delay_ = Clamp(settings.delay_ms, kMinDelayMs, kMaxDelayMs) * kSamplesPerMs;
    mix_target_ = Clamp(settings.mix, 0.0f, 1.0f);
    feedback_target_ = Clamp(settings.feedback, 0.0f, 1.0f) * kFeedbackCeiling;
    const float tone = Clamp(settings.tone, 0.0f, 1.0f);
    // One-pole LPF. Alpha endpoints approximate -3 dB at 700 and 9000 Hz.
    tone_target_ = Interpolate(0.0875f, 0.6536f, tone * tone);
    engage_target_ = settings.enabled ? 1.0f : 0.0f;
    glide_ = settings.glide;
}

float Delay::Read(float delay_samples) const
{
    const auto whole = static_cast<std::size_t>(delay_samples);
    const float fraction = delay_samples - static_cast<float>(whole);

    const std::size_t newer = write_ >= whole
                                  ? write_ - whole
                                  : write_ + kDelayCapacity - whole;
    const std::size_t older = newer == 0 ? kDelayCapacity - 1 : newer - 1;
    return Interpolate(memory_[newer], memory_[older], fraction);
}

float Delay::ReadTransition()
{
    const float delta = target_delay_ - delay_;
    if (fade_remaining_ == 0 && !glide_
        && (delta >= kCrossfadeThreshold || delta <= -kCrossfadeThreshold))
    {
        fade_to_ = target_delay_;
        fade_remaining_ = kFadeSamples;
    }

    if (fade_remaining_ != 0)
    {
        const float blend =
            static_cast<float>(kFadeSamples - fade_remaining_ + 1) / kFadeSamples;
        const float wet = Interpolate(Read(delay_), Read(fade_to_), blend);
        if (--fade_remaining_ == 0)
            delay_ = fade_to_;
        return wet;
    }

    // Close the gap left by the crossfade threshold smoothly as well.
    delay_ += Clamp(delta, -kGlideStep, kGlideStep);
    return Read(delay_);
}

float Delay::Process(float input)
{
    mix_ = Interpolate(mix_, mix_target_, kControlSmoothing);
    feedback_ = Interpolate(feedback_, feedback_target_, kControlSmoothing);
    tone_alpha_ = Interpolate(tone_alpha_, tone_target_, kControlSmoothing);
    engage_ += Clamp(engage_target_ - engage_, -kBypassStep, kBypassStep);

    filtered_ = Interpolate(filtered_, ReadTransition(), tone_alpha_);
    memory_[write_] = Clamp(input * engage_ + feedback_ * filtered_, -1.0f, 1.0f);
    if (++write_ == kDelayCapacity)
        write_ = 0;

    return Interpolate(input, filtered_, mix_ * engage_);
}
} // namespace pedal
