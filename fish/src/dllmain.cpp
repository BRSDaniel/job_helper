#include <windows.h>
#include <thread>
#include <chrono>
#include <cmath>
#include "shared.h" // assuming this contains ivector2, c_pixel, etc.

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        std::thread([] {
            SetProcessDPIAware();

            HWND hwnd = GetForegroundWindow();
            if (!hwnd) return;

            HDC device_context = GetDC(hwnd);

            RECT rc{};
            GetClientRect(hwnd, &rc);
            shared::ivector2 screen{ rc.right - rc.left, rc.bottom - rc.top };

            const shared::ivector2 base{ 1920, 1080 };
            const double sx = static_cast<double>(screen.m_x) / base.m_x;
            const double sy = static_cast<double>(screen.m_y) / base.m_y;

            auto scale_pt = [&](int x, int y) -> shared::ivector2 {
                return {
                    static_cast<int>(std::lround(x * sx)),
                    static_cast<int>(std::lround(y * sy))
                };
            };

            auto scale_size = [&](int w, int h) -> shared::ivector2 {
                return {
                    std::max(1, static_cast<int>(std::lround(w * sx))),
                    std::max(1, static_cast<int>(std::lround(h * sy)))
                };
            };

            shared::ivector2 marker_position;
            shared::ivector2 skillcheck_position;

            while (true) {
                if (marker_position.zero()) {
                    // Auto-detect marker at the middle of the monitor
                    shared::ivector2 mid_point{
                        screen.m_x / 2,
                        screen.m_y / 2
                    };

                    marker_position = shared::c_pixel::find_marker_position(
                        device_context,
                        mid_point
                    );
                }

                if (skillcheck_position.zero()) {
                    skillcheck_position = shared::c_pixel::find_skillcheck_position(
                        device_context,
                        scale_pt(812, 1027),
                        scale_size(300, 1)
                    );
                }

                // Example: process logic here using scaled positions

                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }

            ReleaseDC(hwnd, device_context);
        }).detach();
        break;
    }
    return TRUE;
}
