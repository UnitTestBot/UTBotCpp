#ifndef FILE_H
#define FILE_H

int file_fgetc(FILE *fA, FILE *fB, FILE *fC);

int file_fgets(FILE *fA);

char file_fputc(int x, int y, FILE *fA);

char file_fputs(char c, FILE *fA);

int sum_two_from_file(FILE *f);

int file_fread(FILE *fA, FILE *fB);

char file_fwrite(FILE *fA, int x);

#endif
