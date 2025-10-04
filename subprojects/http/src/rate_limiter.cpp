#include <sscan/rate_limiter.h>

#include <thread>

using namespace http;

RateLimiter::RateLimiter(const int a_rps) : 
    rps {a_rps}, requests {0}, last_reset {std::chrono::system_clock::now()} {};

void RateLimiter::acquire() {
    requests++;
    if (requests <= rps) {
        return;
    }

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - last_reset).count();
    auto time_to_wait = 1000 - elapsed;
    if (time_to_wait > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(time_to_wait));
    }
    last_reset = std::chrono::system_clock::now();
    requests = 1;
}

