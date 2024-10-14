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
 * @return code of error or 0 if successful.
 */
int wait_program(const int pid);

/**
 * Starts program in background with ability to wait or leave it.
 *
 * @param program_path Absolute or relative path to program.
 * @param is_wait If true then waits program to finish.
 * @return code of error or 0 if successful.
 */
int start_wait(const char *program_path, bool is_wait = true);