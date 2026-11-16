#include <cstdio>

namespace {

int g_failures = 0;

void check(bool condition, const char* expr, int line) {
    if (!condition) {
        std::fprintf(stderr, "FAILED: %s (line %d)\n", expr, line);
        ++g_failures;
    }
}

}

#define SPY_CHECK(expr) check((expr), #expr, __LINE__)

int main() {
    SPY_CHECK(true);
    SPY_CHECK(1 + 1 == 2);

    if (g_failures == 0) {
        std::puts("all tests passed");
        return 0;
    }
    std::fprintf(stderr, "%d check(s) failed\n", g_failures);
    return 1;
}
