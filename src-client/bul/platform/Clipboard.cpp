#include "bul/platform/Clipboard.h"

#include <cstring>
#include <string>

#include <windows.h>

namespace bul::platform {

namespace {

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

[[nodiscard]] std::string toUtf8(std::wstring_view text) {
    if (text.empty()) {
        return {};
    }
    int const length =
        WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0, nullptr, nullptr);
    if (length <= 0) {
        return {};
    }
    std::string out(static_cast<std::size_t>(length), '\0');
    WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), out.data(), length, nullptr, nullptr);
    return out;
}

} // namespace

std::string clipboardText() {
    if (IsClipboardFormatAvailable(CF_UNICODETEXT) == 0) {
        return {};
    }
    if (OpenClipboard(nullptr) == 0) {
        return {};
    }
    std::string result;
    if (HANDLE const data = GetClipboardData(CF_UNICODETEXT)) {
        if (auto const* characters = static_cast<wchar_t const*>(GlobalLock(data))) {
            result = toUtf8(std::wstring_view{characters});
            GlobalUnlock(data);
        }
    }
    CloseClipboard();
    return result;
}

void setClipboardText(std::string_view utf8) {
    std::wstring const wide = toWide(utf8);
    if (OpenClipboard(nullptr) == 0) {
        return;
    }
    EmptyClipboard();
    std::size_t const bytes = (wide.size() + 1) * sizeof(wchar_t);
    if (HANDLE const memory = GlobalAlloc(GMEM_MOVEABLE, bytes)) {
        if (void* destination = GlobalLock(memory)) {
            std::memcpy(destination, wide.c_str(), bytes);
            GlobalUnlock(memory);
            // Ownership moves to the clipboard; a failure frees it.
            if (SetClipboardData(CF_UNICODETEXT, memory) == nullptr) {
                GlobalFree(memory);
            }
        } else {
            GlobalFree(memory);
        }
    }
    CloseClipboard();
}

} // namespace bul::platform
