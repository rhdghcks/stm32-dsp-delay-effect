#include "controls.h"

namespace pedal
{
namespace
{
constexpr std::uint32_t kDebounceMs = 8;
constexpr std::uint32_t kMinTapMs = 150;
constexpr std::uint32_t kMaxTapMs = static_cast<std::uint32_t>(kMaxDelayMs);
constexpr float kKnobSensitivity = 0.025f;
constexpr float kKnobSmoothing = 0.10f;

// A small end zone makes the minimum and maximum achievable with real pots.
float NormalizePot(float value)
{
    constexpr float edge = 0.01f;
    if (value <= edge)
        return 0.0f;
    if (value >= 1.0f - edge)
        return 1.0f;
    return (value - edge) / (1.0f - 2.0f * edge);
}
} // namespace

void Controls::DebouncedInput::Init(bool value, std::uint32_t now)
{
    stable = raw = value;
    changed_at = now;
}

bool Controls::DebouncedInput::Update(bool value, std::uint32_t now)
{
    if (value != raw)
    {
        raw = value;
        changed_at = now;
    }

    if (stable == raw || now - changed_at < kDebounceMs)
        return false;

    stable = raw;
    return stable;
}

Settings Controls::Init(const Inputs& inputs, std::uint32_t now)
{
    bypass_.Init(inputs.bypass_pressed, now);
    tap_.Init(inputs.tap_pressed, now);
    glide_.Init(inputs.glide_on, now);
    have_previous_tap_ = tap_owns_time_ = false;
    interval_index_ = interval_count_ = 0;
    time_knob_ = NormalizePot(inputs.time);
    settings_ = {TimeFromKnob(), NormalizePot(inputs.mix),
                 NormalizePot(inputs.feedback), NormalizePot(inputs.tone),
                 false, glide_.stable};
    return settings_;
}

float Controls::TimeFromKnob() const
{
    return kMinDelayMs + (kMaxDelayMs - kMinDelayMs) * time_knob_ * time_knob_;
}

void Controls::Tap(std::uint32_t now)
{
    if (!have_previous_tap_)
    {
        previous_tap_ = now;
        have_previous_tap_ = true;
        return;
    }

    const std::uint32_t elapsed = now - previous_tap_;
    if (elapsed < kMinTapMs)
        return;

    previous_tap_ = now;
    intervals_[interval_index_] = elapsed;
    interval_index_ = (interval_index_ + 1) % 3;
    if (interval_count_ < 3)
        ++interval_count_;

    std::uint32_t sum = 0;
    for (unsigned i = 0; i < interval_count_; ++i)
        sum += intervals_[i];
    settings_.delay_ms = static_cast<float>(sum) / static_cast<float>(interval_count_);
    tap_owns_time_ = true;
    tap_knob_position_ = time_knob_;
}

Settings Controls::Update(const Inputs& inputs, std::uint32_t now)
{
    time_knob_ += kKnobSmoothing * (NormalizePot(inputs.time) - time_knob_);
    const float motion = time_knob_ - tap_knob_position_;
    if (tap_owns_time_ && (motion >= kKnobSensitivity || motion <= -kKnobSensitivity))
        tap_owns_time_ = false;
    if (!tap_owns_time_)
        settings_.delay_ms = TimeFromKnob();

    if (have_previous_tap_ && now - previous_tap_ > kMaxTapMs)
    {
        have_previous_tap_ = false;
        interval_index_ = interval_count_ = 0;
    }
    if (tap_.Update(inputs.tap_pressed, now))
        Tap(now);

    if (bypass_.Update(inputs.bypass_pressed, now))
        settings_.enabled = !settings_.enabled;
    glide_.Update(inputs.glide_on, now);

    settings_.mix = NormalizePot(inputs.mix);
    settings_.feedback = NormalizePot(inputs.feedback);
    settings_.tone = NormalizePot(inputs.tone);
    settings_.glide = glide_.stable;
    return settings_;
}
} // namespace pedal
