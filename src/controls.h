#pragma once

#include "delay.h"

#include <cstdint>

namespace pedal
{
struct Inputs
{
    float time;
    float mix;
    float feedback;
    float tone;
    bool bypass_pressed;
    bool tap_pressed;
    bool glide_on;
};

class Controls
{
  public:
    Settings Init(const Inputs& inputs, std::uint32_t now);
    Settings Update(const Inputs& inputs, std::uint32_t now);

  private:
    struct DebouncedInput
    {
        bool stable;
        bool raw;
        std::uint32_t changed_at;

        void Init(bool value, std::uint32_t now);
        bool Update(bool value, std::uint32_t now);
    };

    void Tap(std::uint32_t now);
    float TimeFromKnob() const;

    DebouncedInput bypass_{};
    DebouncedInput tap_{};
    DebouncedInput glide_{};
    Settings settings_{};

    float time_knob_ = 0.0f;
    float tap_knob_position_ = 0.0f;
    bool tap_owns_time_ = false;
    bool have_previous_tap_ = false;
    std::uint32_t previous_tap_ = 0;

    std::uint32_t intervals_[3]{};
    unsigned interval_index_ = 0;
    unsigned interval_count_ = 0;
};
} // namespace pedal
