// ====================================================================
// Logger.h
// Lightweight, configurable debug logger for Arduino AVR (Nano ATmega328P)
//
// Features:
//   - Levels: I / W / E / D
//   - Optional timestamp (millis)
//   - Compile-time disable (LOG_ENABLE=0 -> logs compiled out)
//   - Format strings stored in flash using PSTR() + vsnprintf_P()
//
// Notes:
//   - LOGI/LOGW/LOGE/LOGD expect the first argument to be a *string literal*
//     (so PSTR(...) works). Example: LOGE("Bad value %d", x);
//   - For plain messages with no formatting, prefer LOG*_SIMPLE("...").
// ====================================================================
#pragma once

#include <Arduino.h>
#include <avr/pgmspace.h>
#include <stdarg.h>
#include <stdio.h>

// --------------------------------------------------------------------
// Configuration (override from platformio.ini with -DLOG_ENABLE=..., etc.)
// --------------------------------------------------------------------
#ifndef LOG_ENABLE
    #define LOG_ENABLE 1
#endif

#ifndef LOG_TIMESTAMP
    #define LOG_TIMESTAMP 1
#endif

#ifndef LOG_BUFFER_SIZE
    #define LOG_BUFFER_SIZE 192
#endif

namespace logger
{
    inline void printPrefix(char level)
    {
    #if LOG_TIMESTAMP
        Serial.print('[');
        Serial.print(millis());
        Serial.print(F(" ms] "));
    #endif
        Serial.print('[');
        Serial.print(level);
        Serial.print(F("] "));
    }

    // Print a formatted log message.
    // fmt is a PROGMEM pointer (PGM_P) so we can keep format strings in flash.
    inline void log_vprintf_P(char level, PGM_P fmt, va_list ap)
    {
        char buf[LOG_BUFFER_SIZE];

        // Build a RAM prefix first.
        int n = 0;
    #if LOG_TIMESTAMP
        n = snprintf(buf, sizeof(buf), "[%lu ms] [%c] ", (unsigned long)millis(), level);
    #else
        n = snprintf(buf, sizeof(buf), "[%c] ", level);
    #endif

        if (n < 0) {
            // snprintf failed, fall back to direct printing.
            printPrefix(level);
            Serial.println(F("<snprintf failed>"));
            return;
        }

        size_t used = (static_cast<size_t>(n) >= sizeof(buf)) ? (sizeof(buf) - 1u) : static_cast<size_t>(n);

        // Append formatted message from PROGMEM format string.
        vsnprintf_P(buf + used, sizeof(buf) - used, fmt, ap);

        Serial.println(buf);
    }

    inline void log_printf_P(char level, PGM_P fmt, ...)
    {
        va_list ap;
        va_start(ap, fmt);
        log_vprintf_P(level, fmt, ap);
        va_end(ap);
    }

    // Plain message stored in flash (fast + small SRAM impact)
    inline void log_simple_P(char level, const __FlashStringHelper* msg)
    {
        printPrefix(level);
        Serial.println(msg);
    }
}

// --------------------------------------------------------------------
// Public macros
// --------------------------------------------------------------------
#if LOG_ENABLE

    // Formatted logs (printf-style).
    // GNU extension used to swallow the comma when no extra args are provided.
    // If -Wpedantic complains, use the *_SIMPLE macros for no-arg logs,
    // or compile with -Wno-gnu-zero-variadic-macro-arguments.
    #define LOGI(fmt, ...)  logger::log_printf_P('I', PSTR(fmt), ##__VA_ARGS__)
    #define LOGW(fmt, ...)  logger::log_printf_P('W', PSTR(fmt), ##__VA_ARGS__)
    #define LOGE(fmt, ...)  logger::log_printf_P('E', PSTR(fmt), ##__VA_ARGS__)
    #define LOGD(fmt, ...)  logger::log_printf_P('D', PSTR(fmt), ##__VA_ARGS__)

    // Plain message logs (no formatting)
    #define LOGI_SIMPLE(msg_literal) logger::log_simple_P('I', F(msg_literal))
    #define LOGW_SIMPLE(msg_literal) logger::log_simple_P('W', F(msg_literal))
    #define LOGE_SIMPLE(msg_literal) logger::log_simple_P('E', F(msg_literal))
    #define LOGD_SIMPLE(msg_literal) logger::log_simple_P('D', F(msg_literal))

    // Progress dots (useful during long init)
    #define LOG_DOT() do { Serial.print('.'); } while (0)

#else

    #define LOGI(... )          do {} while (0)
    #define LOGW(... )          do {} while (0)
    #define LOGE(... )          do {} while (0)
    #define LOGD(... )          do {} while (0)
    #define LOGI_SIMPLE(... )   do {} while (0)
    #define LOGW_SIMPLE(... )   do {} while (0)
    #define LOGE_SIMPLE(... )   do {} while (0)
    #define LOGD_SIMPLE(... )   do {} while (0)
    #define LOG_DOT()           do {} while (0)

#endif
