#include "MaqamDetector.h"
#include <algorithm>
#include <cmath>

namespace Maqam
{
const std::vector<MaqamTemplate>& templates()
{
    static const std::vector<MaqamTemplate> t = {
        { "Rast",     { 0, 200, 350, 500, 700, 900, 1050 } },
        { "Bayati",   { 0, 150, 300, 500, 700, 800, 1000 } },
        { "Hijaz",    { 0, 100, 400, 500, 700, 800, 1000 } },
        { "Saba",     { 0, 150, 300, 400, 700, 800, 1000 } },
        { "Nahawand", { 0, 200, 300, 500, 700, 800, 1100 } },
    };
    return t;
}

juce::String binName (int bin)
{
    static const char* n[] = { "C", "C#", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B" };
    return juce::String (n[bin / 2]) + (bin % 2 ? "+" : "");
}

// Basic YIN pitch detector. Returns Hz, or 0 if unvoiced.
static float yinPitch (const float* x, int W, int tauMax, double sr, float threshold = 0.15f)
{
    std::vector<float> d ((size_t) tauMax + 1, 0.f), cmnd ((size_t) tauMax + 1, 1.f);
    for (int tau = 1; tau <= tauMax; ++tau)
    {
        float sum = 0.f;
        for (int i = 0; i < W; ++i) { const float diff = x[i] - x[i + tau]; sum += diff * diff; }
        d[(size_t) tau] = sum;
    }
    float running = 0.f;
    for (int tau = 1; tau <= tauMax; ++tau)
    {
        running += d[(size_t) tau];
        cmnd[(size_t) tau] = d[(size_t) tau] * (float) tau / juce::jmax (running, 1e-9f);
    }
    const int tauMin = juce::jmax (2, (int) (sr / 1000.0));
    for (int tau = tauMin; tau < tauMax - 1; ++tau)
    {
        if (cmnd[(size_t) tau] < threshold)
        {
            while (tau + 1 < tauMax - 1 && cmnd[(size_t) tau + 1] < cmnd[(size_t) tau]) ++tau;
            const float a = cmnd[(size_t) tau - 1], b = cmnd[(size_t) tau], c = cmnd[(size_t) tau + 1];
            const float shift = (a - c) / (2.f * (a - 2.f * b + c) + 1e-9f);
            return (float) (sr / ((double) tau + shift));
        }
    }
    return 0.f;
}

std::vector<Result> detect (const std::vector<float>& mono, double sr)
{
    constexpr int W = 1024, frame = 2048, hop = 2048;
    const int tauMax = juce::jmin (W, (int) (sr / 80.0));

    std::array<double, 24> hist {};
    double total = 0.0;

    for (size_t pos = 0; pos + (size_t) frame <= mono.size(); pos += (size_t) hop)
    {
        const float* x = mono.data() + pos;
        float e = 0.f;
        for (int i = 0; i < W; ++i) e += x[i] * x[i];
        if (std::sqrt (e / (float) W) < 0.01f) continue;          // silence gate

        const float f = yinPitch (x, W, tauMax, sr);
        if (f < 80.f || f > 1000.f) continue;

        const double cents = 1200.0 * std::log2 ((double) f / 261.6256);   // relative to C4
        const int bin = (((int) std::lround (cents / 50.0)) % 24 + 24) % 24;
        hist[(size_t) bin] += 1.0;
        total += 1.0;
    }
    if (total < 10.0) return {};
    for (auto& h : hist) h /= total;

    std::vector<Result> all;
    const auto& tpls = templates();
    for (size_t ti = 0; ti < tpls.size(); ++ti)
        for (int tonic = 0; tonic < 24; ++tonic)
        {
            double in = 0.0;
            for (int c : tpls[ti].cents) in += hist[(size_t) ((tonic + c / 50) % 24)];
            Result r;
            r.name = tpls[ti].name;
            r.templateIndex = (int) ti;
            r.tonicBin = tonic;
            r.score = (float) (1.5 * in - 0.5 + 0.5 * hist[(size_t) tonic]);
            all.push_back (r);
        }

    std::sort (all.begin(), all.end(), [] (const Result& a, const Result& b) { return a.score > b.score; });
    all.resize (3);
    return all;
}
} // namespace Maqam
