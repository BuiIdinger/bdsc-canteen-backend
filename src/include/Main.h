#pragma once

void shutdown(const int& code) noexcept;

// Used on returnErrorNotification
constexpr const char* ERROR_OCCURRED = "An internal server error occurred, try again later.";