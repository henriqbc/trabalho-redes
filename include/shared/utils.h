/**
 * @file utils.h
 * @author Arthur Vergaças (arthur.vergacas@usp.br)
 * @brief Some utilitary functions to make use in my college assignments
 * @date 2021-09-15
 *
 * @copyright Copyright (c) 2021
 *
 */

#pragma once

#include <stdbool.h>
#include <stdio.h>

typedef char byte;

#define min(a, b) \
  ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

/**
 * @brief Reads a string from a specified stream until find the separator or EOF, allocates
 * memory for it and returns its pointer.
 *
 * @param stream The source stream
 * @param separators A string of separators. When one of these characters are
 * found, the function stops and return the string read
 * @return The string
 */
char *readString(FILE *stream, char *separators);

/**
 * @brief Returns the first characters of a string until one of the delimiters
 * is found in the form of a substring, i.e. with a '\0' at the end.
 *
 * @param string The complete string
 * @param delimiters A string of characters used as delimiters
 * @return The substring
 */
char *substringUntil(char *string, char *delimiters);

/**
 * @brief Consumes any '\\r' and '\\n' characters from stdin. Useful when reading data with
 * scanf().
 *
 */
void consumeNewLine();

/**
 * @brief Returns if a given char is inside a given string.
 *
 * @param str The string where the char will be verified
 * @param c The char to look for
 * @return True if the char is inside the string, false otherwise
 */
bool strIncludes(const char *str, char c);

/**
 * @brief A function that works similar to string.h's strlen().
 *
 * @param str The string whose characters will be counted
 * @return The number of characters in the string
 */
int strLen(const char *str);

/**
 * @brief Converts a number into a string. For example, intToString(12) = "12".
 *
 * @param num The number to be converted
 * @return The string representation of the number
 */
char *intToString(int num);

/**
 * @brief Reverts a string
 *
 * @param str The string to be reverted
 * @param size The size of the string
 */
void revertString(char *str);

/**
 * @brief Helper function used to compare the contents of a binary file in run.codes.
 * It prints the sum of all digits in the binary file.
 *
 * @param nomeArquivoBinario The binary file to be printed
 */
void binarioNaTela(char *nomeArquivoBinario);

/**
 * @brief Helper function used to copy values from source to destination.
 * Although similar to strcpy, this function also allocates memory to the desired destination.
 *
 * @param destination
 * @param source
 */
void assignString(char *destination, char *source);
