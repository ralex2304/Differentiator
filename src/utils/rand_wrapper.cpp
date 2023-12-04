#include "rand_wrapper.h"

long long rand_normal_generate_ll(long long min, long long max) {
    static std::random_device rd;

    // Mersenne twister PRNG, initialized with seed from previous random device instance
    static std::mt19937 gen(rd());

    std::uniform_int_distribution<long long> rand_normal {min, max};

    return rand_normal(gen);
}
