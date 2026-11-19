#include "controls.h"
#include "delay.h"

#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>

using namespace pedal;
static float ring[kDelayCapacity];

static bool Close(float a, float b, float tolerance = 0.00001f)
{
    return std::fabs(a - b) < tolerance;
}

static void TestEcho()
{
    Delay delay;
    Settings s{350.0f, 1.0f, 0.5f, 1.0f, true, false};
    delay.Init(ring, s);
    float first = 0.0f;
    for (int i = 0; i <= 33600; ++i)
    {
        const float out = delay.Process(i == 0 ? 1.0f : 0.0f);
        if (i < 16800)
            assert(out == 0.0f);
        if (i == 16800)
            first = out;
        if (i == 33600)
            assert(Close(out, first * first * 0.47f));
    }
    assert(Close(first, 0.6536f));

    s.tone = 0.0f;
    s.delay_ms = 2000.0f;
    s.feedback = 0.0f;
    delay.Init(ring, s);
    for (int i = 0; i <= 196001; ++i)
    {
        const float out = delay.Process(i == 100001 ? 1.0f : 0.0f);
        if (i < 196001)
            assert(out == 0.0f);
        else
            assert(Close(out, 0.0875f)); // max delay with ring index already wrapped
    }

    s.delay_ms = 20.0f + 0.5f / 48.0f;
    s.tone = 1.0f;
    delay.Init(ring, s);
    for (int i = 0; i < 960; ++i)
        assert(delay.Process(i == 0 ? 1.0f : 0.0f) == 0.0f);
    assert(Close(delay.Process(0.0f), 0.6536f / 2.0f));
    assert(Close(delay.Process(0.0f), 0.6536f * (2.0f - 0.6536f) / 2.0f));
}

static void TestTransition(bool glide)
{
    Delay delay;
    Settings s{350.0f, 1.0f, 0.0f, 1.0f, true, glide};
    delay.Init(ring, s);
    constexpr float slope = 0.00001f;
    for (int i = 0; i < 30000; ++i)
        delay.Process(slope * static_cast<float>(i));
    s.delay_ms = 500.0f;
    delay.Set(s);
    float previous = 0.0f;
    for (int i = 30000; i < 60000; ++i)
    {
        float out = delay.Process(slope * static_cast<float>(i));
        if (i == 30200)
            assert(Close(out - previous, slope * (glide ? 0.75f : -5.0f), 0.0000002f));
        if (i == 59800)
            assert(Close(out - previous, slope, 0.0000002f));
        previous = out;
    }
}

static void TestBypassAndCeiling()
{
    Delay delay;
    Settings s{20.0f, 0.65f, 1.0f, 1.0f, false, false};
    delay.Init(ring, s);
    for (int i = 0; i < 1400; ++i)
        assert(delay.Process(0.23f) == 0.23f);
    s.enabled = true;
    delay.Set(s);
    for (int i = 0; i < 200000; ++i)
    {
        const float in = i % 6000 < 3000 ? 1.0f : -1.0f;
        assert(std::fabs(delay.Process(in)) <= 1.0f);
    }
    s.enabled = false;
    delay.Set(s);
    for (int i = 0; i < 485; ++i)
        delay.Process(0.12f);
    assert(delay.Process(0.12f) == 0.12f);
}

static void TestControls()
{
    Controls controls;
    Inputs in{0.5f, 0.0f, 1.0f, 0.5f, false, false, false};
    std::uint32_t tick = std::numeric_limits<std::uint32_t>::max() - 450;
    std::uint32_t age = 0;
    Settings s = controls.Init(in, tick);
    assert(!s.enabled && Close(s.delay_ms, 515.0f));
    for (; age <= 5000; ++age)
    {
        // The tap interval spans a 32-bit tick wrap. 200 ms is ignored.
        in.tap_pressed = (age >= 100 && age < 125)
                         || (age >= 200 && age < 225)
                         || (age >= 600 && age < 625)
                         || (age >= 1120 && age < 1145)
                         || (age >= 3900 && age < 3925)
                         || (age >= 4500 && age < 4525);
        in.bypass_pressed = (age >= 15 && age < 19) || (age >= 20 && age < 130);
        in.glide_on = age >= 300;
        if (age >= 1250)
            in.time = 0.8f;
        s = controls.Update(in, tick + age);
        if (age == 19)
            assert(!s.enabled);
        if (age == 28 || age == 200)
            assert(s.enabled);
        if (age == 307)
            assert(!s.glide);
        if (age == 308)
            assert(s.glide);
        if (age == 607)
            assert(Close(s.delay_ms, 515.0f));
        if (age == 608)
            assert(Close(s.delay_ms, 500.0f));
        if (age == 1128)
            assert(Close(s.delay_ms, 510.0f));
        if (age == 1300 || age == 3908)
            assert(s.delay_ms > 1250.0f);
        if (age == 4508)
            assert(Close(s.delay_ms, 600.0f));
    }
}

int main()
{
    TestEcho();
    TestTransition(false);
    TestTransition(true);
    TestBypassAndCeiling();
    TestControls();
    std::cout << "PASS: exact and fractional echo timing, ring wrap at 2 s, feedback and tone, "
                 "both time transitions, bypass and feedback ceiling, tap/debounce/tick wrap.\n";
}
