// Fast, completely branchless PRNG (xoshiro256**)
#include <algorithm>
#include <bit>
#include <cstdint>
#include <random>

namespace {
    struct FastRng {
        uint64_t s[4];

        FastRng() {
            std::random_device rd;
            for (auto& x : s) {
                x = (static_cast<uint64_t>(rd()) << 32) | rd();
            }
        }

        constexpr uint64_t next() noexcept {
            const uint64_t result = std::rotl(s[1] * 5, 7) * 9;
            const uint64_t t = s[1] << 17;

            s[2] ^= s[0];
            s[3] ^= s[1];
            s[1] ^= s[2];
            s[0] ^= s[3];

            s[2] ^= t;
            s[3] = std::rotl(s[3], 45);
            return result;
        }

        constexpr double next_double() noexcept {
            return static_cast<double>(next() >> 11) * 0x1.0p-53;
        }

        constexpr int next_int(int min, int max) noexcept {
            const uint64_t range = static_cast<uint64_t>(static_cast<int64_t>(max) - min) + 1ULL;
            const uint32_t rand32 = static_cast<uint32_t>(next() >> 32);
            const uint32_t offset = static_cast<uint32_t>((static_cast<uint64_t>(rand32) * range) >> 32);
            return static_cast<int>(static_cast<int64_t>(min) + offset);
        }

        constexpr int next_rand() noexcept {
            return static_cast<int>(next() >> 33);
        }
    };

    thread_local FastRng rng;
}
