#ifndef SECURITIES_SCANNER_RATE_LIMITER_H
#define SECURITIES_SCANNER_RATE_LIMITER_H

#include <chrono>

namespace http {

    class RateLimiter {
        public:
            RateLimiter(const int rps);

            RateLimiter(const RateLimiter& other) = delete;
            RateLimiter& operator=(const RateLimiter& other) = delete;

            RateLimiter(RateLimiter&& other) = default;
            RateLimiter& operator=(RateLimiter&& other) = default;

            void acquire();
        private:
            const int rps;
            int requests;
            std::chrono::system_clock::time_point last_reset;
    };

}

#endif // SECURITIES_SCANNER_RATE_LIMITER_H