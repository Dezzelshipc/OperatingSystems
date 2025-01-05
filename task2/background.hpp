#pragma once

/**
 * Starts program in background.
 *
 * @param program_path Absolute or relative path to program.
 * @return process id (pid) of started process.
 */
int start_background(const char *program_path);

/**
 * Starts program in background.
 *
 * @param program_path Absolute or relative path to program.
 * @param status Returns code of error or 0 if successful.
 * @return process id (pid) of started process.
 */
int start_background(const char *program_path, int &status);

/**
 * Wait for program to end.
 *
 * @param pid Process id of program.
 * @param exit_code Pointer to store exit_code. (optional)
 * @return code of error or 0 if successful.
 */
int wait_program(const int pid, int* exit_code = nullptr);