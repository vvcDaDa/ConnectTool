#include "tun_interface.h"

#ifdef __linux__
#include "tun_linux.h"
#elif defined(__APPLE__)
#include "tun_macos.h"
#elif defined(_WIN32)
#include "tun_windows.h"
#endif

namespace tun {

std::unique_ptr<TunInterface> create_tun() {
#ifdef __linux__
    return std::make_unique<TunLinux>();
#elif defined(__APPLE__)
    return std::make_unique<TunMacOS>();
#elif defined(_WIN32)
    return std::make_unique<TunWindows>();
#else
    #error "Unsupported platform"
#endif
}

} // namespace tun
