//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// NOTE: unfinished port  from ElfScript SDL3 Audio to univeral usable
//-----------------------------------------------------------------------------
#pragma once
#include <stdint.h>
#include <cmath>
#include <vector>
#include <cstdlib>  // malloc() and free()
#include <cstring>  // memset

namespace MelodyMaker {
    typedef double F64 ;
    typedef float F32 ;
    typedef int32_t S32 ;

    struct Tone {
        S32 midiNote;
        F32 duration;
        F32 amplitute = 0.2f;
    };

    struct ToneEntry {
        S32 LineNumber = 0;
        Tone tone = {0};
    };

    typedef std::vector<ToneEntry> Melody;

    /* Convert a MIDI note number to frequency */
    static F64 midiToFrequency(S32 midi_note)
    {
        if (midi_note <= 0) return 0.0; //silents
        return 440.0 *  std::pow(2.0, (midi_note - 69) / 12.0);
    }

    /*
     * Add a short fade in/out to avoid clicks when notes change.
     */
    static float envelope(S32 sample, S32 total_samples, S32 sampleRate )
    {
        const S32 fade_samples = sampleRate / 100;

        F32 gain = 1.0f;

        if (sample < fade_samples) {
            gain = (F32)sample / fade_samples;
        }

        if (sample > total_samples - fade_samples) {
            gain = (F32)(total_samples - sample) / fade_samples;
        }

        if (gain < 0.0f) {
            gain = 0.0f;
        }

        return gain;
    }
    // -----------------
    // NOTE dont forget to std::free the result!
    static F32 *generateNoteF(int midi_note, F32 duration, uint32_t *byte_count, F32 amplitude /*= 0.20f*/, S32 sampleRate /*= 48000*/)
    {
        const uint32_t sample_count = (uint32_t)(sampleRate * duration);

        F32* samples = (F32*)std::malloc(sample_count * sizeof(F32));
        if (!samples) {
            return NULL;
        }

        *byte_count = sample_count * (uint32_t)sizeof(F32);

        if (midi_note <= 0) {
            std::memset(samples, 0, *byte_count);
            return samples;
        }

        const double frequency = midiToFrequency(midi_note);

        for (int i = 0; i < sample_count; i++) {
            double time = (F64)i / sampleRate;
            F64 wave = std::sin(2.0 * M_PI * frequency * time) * 0.85 +
            std::sin(2.0 * M_PI * frequency * 2.0 * time) * 0.15;

            F32 gain = amplitude * envelope(i, sample_count, sampleRate);

            samples[i] = (F32)(wave * gain);
        }


        return samples;
    }
}
