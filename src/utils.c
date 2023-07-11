#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "utils.h"

char *readString(FILE *stream, char *delimiters) {
  char *string = NULL;

  char currentChar;
  int stringSize = 0;
  while (!strIncludes(delimiters, (currentChar = fgetc(stream)))) {

    // if we read a \r, just ignore it, we probably don't want it
    if (currentChar == '\r') continue;

    // if we are in EOF, there's nothing else to read
    if (currentChar == EOF) break;

    char *temp = realloc(string, (stringSize + 2) * sizeof(*string));

    if (temp == NULL) {
      free(string);
      printf("Erro de realocação\n");
      return NULL;
    } else {
      string = temp;

      string[stringSize] = currentChar;
      string[stringSize + 1] = '\0';

      stringSize++;
    }
  }

  return string;
}

char *substringUntil(char *string, char *delimiters) {
  char *substring = NULL;
  int substringSize = 0;

  for (int i = 0; i < strLen(string); i++) {
    if (strIncludes(delimiters, string[i])) {
      break;
    }

    substring = realloc(substring, i + 1);
    substring[i] = string[i];
    substringSize++;
  }

  if (substring != NULL) {
    substring = realloc(substring, substringSize + 1);
    substring[substringSize] = '\0';
  }

  return substring;
}

void consumeNewLine() {

  char currentChar;
  while ((currentChar = getchar()) == '\n' || currentChar == '\r') {
    continue;
  }

  ungetc(currentChar, stdin);
}

bool strIncludes(const char *str, char c) {
  for (long unsigned int i = 0; i < strLen(str); i++) {
    if (str[i] == c) return true;
  }
  return false;
}

int strLen(const char *str) {
  int counter = 0;
  for (int i = 0; str[i] != '\0'; i++) {
    counter++;
  }
  return counter;
}

char *intToString(int num) {
  char *string = NULL;
  int stringSize = 0;
  while (num > 0) {
    string = realloc(string, ++stringSize * sizeof(char));
    string[stringSize - 1] = '0' + (num % 10);
    num /= 10;
  }

  string = realloc(string, (stringSize + 1) * sizeof(char));
  string[stringSize] = '\0';

  revertString(string);

  return string;
}

void revertString(char *str) {
  int stringSize = strLen(str);

  for (int i = 0; i < stringSize / 2; i++) {
    char temp = str[i];
    str[i] = str[stringSize - i - 1];
    str[stringSize - i - 1] = temp;
  }
}

void binarioNaTela(char *nomeArquivoBinario) {
  /* Você não precisa entender o código dessa função. */

  /* Use essa função para comparação no run.codes. Lembre-se de ter fechado (fclose) o arquivo
   * anteriormente. Ela vai abrir de novo para leitura e depois fechar (você não vai perder
   * pontos por isso se usar ela).
   */

  unsigned long i, cs;
  unsigned char *mb;
  size_t fl;
  FILE *fs;
  if (nomeArquivoBinario == NULL || !(fs = fopen(nomeArquivoBinario, "rb"))) {
    fprintf(stderr,
            "ERRO AO ESCREVER O BINARIO NA TELA (função binarioNaTela): não foi possível "
            "abrir o arquivo que me passou para leitura. Ele existe e você tá passando o nome "
            "certo? Você lembrou de fechar ele com fclose depois de usar?\n");
    return;
  }
  fseek(fs, 0, SEEK_END);
  fl = ftell(fs);
  fseek(fs, 0, SEEK_SET);
  mb = (unsigned char *)malloc(fl);
  fread(mb, 1, fl, fs);

  cs = 0;
  for (i = 0; i < fl; i++) {
    cs += (unsigned long)mb[i];
  }
  printf("%lf\n", (cs / (double)100));
  free(mb);
  fclose(fs);
}
