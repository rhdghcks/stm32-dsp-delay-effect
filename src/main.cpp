#include "daisy_seed.h"

#include "controls.h"
#include "delay.h"

namespace
{
daisy::DaisySeed board;
daisy::GPIO bypass_switch;
daisy::GPIO tap_switch;
daisy::GPIO mode_toggle;

pedal::Controls controls;
pedal::Delay delay;
DSY_SDRAM_BSS float delay_memory[pedal::kDelayCapacity];
bool effect_led = false;

enum Knob : unsigned
{
    Time,
    Mix,
    Feedback,
    Tone,
    KnobCount
};

pedal::Inputs ReadControls()
{
    return {board.adc.GetFloat(Time), board.adc.GetFloat(Mix),
            board.adc.GetFloat(Feedback), board.adc.GetFloat(Tone),
            !bypass_switch.Read(), !tap_switch.Read(), !mode_toggle.Read()};
}

void AudioCallback(daisy::AudioHandle::InputBuffer input,
                   daisy::AudioHandle::OutputBuffer output,
                   std::size_t size)
{
    const pedal::Settings settings = controls.Update(ReadControls(), daisy::System::GetNow());
    delay.Set(settings);

    if (effect_led != settings.enabled)
    {
        effect_led = settings.enabled;
        board.SetLed(effect_led);
    }

    for (std::size_t i = 0; i < size; ++i)
    {
        const float sample = delay.Process(input[0][i]);
        output[0][i] = sample;
        output[1][i] = sample;
    }
}
} // namespace

int main()
{
    board.Init();
    board.SetAudioSampleRate(daisy::SaiHandle::Config::SampleRate::SAI_48KHZ);
    board.SetAudioBlockSize(pedal::kBlockFrames);

    daisy::AdcChannelConfig knobs[KnobCount];
    // Pots are polled once per audio block. Make room for ADC input settling.
    constexpr auto pot_sample_time = daisy::AdcChannelConfig::SPEED_387CYCLES_5;
    knobs[Time].InitSingle(daisy::seed::A0, pot_sample_time);
    knobs[Mix].InitSingle(daisy::seed::A1, pot_sample_time);
    knobs[Feedback].InitSingle(daisy::seed::A2, pot_sample_time);
    knobs[Tone].InitSingle(daisy::seed::A3, pot_sample_time);
    board.adc.Init(knobs, KnobCount);
    board.adc.Start();

    constexpr auto input = daisy::GPIO::Mode::INPUT;
    constexpr auto pull_up = daisy::GPIO::Pull::PULLUP;
    bypass_switch.Init(daisy::seed::D0, input, pull_up);
    tap_switch.Init(daisy::seed::D1, input, pull_up);
    mode_toggle.Init(daisy::seed::D2, input, pull_up);

    board.SetLed(false);
    board.DelayMs(20);
    delay.Init(delay_memory, controls.Init(ReadControls(), daisy::System::GetNow()));
    board.StartAudio(AudioCallback);

    for (;;)
        __WFI();
}
