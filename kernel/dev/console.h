#pragma once

typedef enum ConsoleMode {
    CONSOLE_COOKED_MODE,
    CONSOLE_RAW_MODE
} ConsoleMode;


void console_init();
void console_handler();
void console_set(ConsoleMode);
