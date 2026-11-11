#include "common.h"
#include <chrono>

double now_seconds()
{

    return std::chrono::duration_cast<std::chrono::duration<double>>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}
