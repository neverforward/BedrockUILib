#include "bul/platform/FileDialog.h"

#include <array>
#include <string>

// clang-format off
#include <windows.h>
#include <commdlg.h> // needs the types windows.h defines
// clang-format on

namespace bul::platform {

namespace {

constexpr std::size_t kPathCapacity = 1024;

[[nodiscard]] std::wstring toWide(std::string_view text) {
    if (text.empty()) {
        return {};
    }
    int const length = MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0);
    if (length <= 0) {
        return {};
    }
    std::wstring out(static_cast<std::size_t>(length), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), out.data(), length);
    return out;
}

[[nodiscard]] std::string toUtf8(wchar_t const* text) {
    if (text == nullptr) {
        return {};
    }
    int const length = WideCharToMultiByte(CP_UTF8, 0, text, -1, nullptr, 0, nullptr, nullptr);
    if (length <= 1) {
        return {};
    }
    std::string out(static_cast<std::size_t>(length - 1), '\0');
    WideCharToMultiByte(CP_UTF8, 0, text, -1, out.data(), length, nullptr, nullptr);
    return out;
}

} // namespace

std::optional<std::string> openFileDialog(std::string_view title) {
    std::array<wchar_t, kPathCapacity> path{};
    std::wstring const                 titleWide = toWide(title);

    OPENFILENAMEW dialog{};
    dialog.lStructSize = sizeof(dialog);
    // The game window may run borderless/exclusive, but it is still a foreground
    // window as far as the common dialog is concerned: without an owner the
    // dialog can open behind the game and the game looks frozen.
    dialog.hwndOwner   = GetForegroundWindow();
    dialog.lpstrFilter = L"All files\0*.*\0\0";
    dialog.lpstrFile   = path.data();
    dialog.nMaxFile    = static_cast<DWORD>(path.size());
    dialog.lpstrTitle  = titleWide.empty() ? nullptr : titleWide.c_str();
    dialog.Flags       = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_EXPLORER;

    if (GetOpenFileNameW(&dialog) != TRUE) {
        return std::nullopt;
    }
    std::string result = toUtf8(path.data());
    if (result.empty()) {
        return std::nullopt;
    }
    return result;
}

} // namespace bul::platform
