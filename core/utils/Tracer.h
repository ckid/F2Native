#include <string>

#ifdef ANDROID
#include "android/F2NativeJNI.h"
#endif

#ifndef X_CHART_UTILS_TRACER_H
#define X_CHART_UTILS_TRACER_H
namespace utils {
class Tracer {
  public:
    Tracer(const std::string &name) : name_(name) {}

    void trace(const char *fmt, ...) {
        char buffer[4096] = {0};

        va_list args;
        va_start(args, fmt);
        vsnprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer) - 1, fmt, args);
        va_end(args);

#ifdef ANDROID
        F2_LOG_I(this->name_.data(), "%s", buffer);
#else
        printf("%s %s\n", this->name_.data(), buffer);
#endif

        // Logger::Info(buffer);
    }

  private:
    std::string name_;
};
} // namespace utils

#endif // X_CHART_UTILS_TRACER_H
