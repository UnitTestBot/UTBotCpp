#include <stdio.h>
#include <fcntl.h>
#include "file.h"

int file_fgetc(FILE *fA, FILE *fB, FILE *fC) {
    unsigned char x = fgetc(fA);
    if (x >= '0' && x <= '9') {
        unsigned char a = fgetc(fB);
        if (a >= 'a' && a <= 'z') {
            return 1;
        } else {
            return 2;
        }
    } else {
        unsigned char a = fgetc(fC);
        unsigned char b = fgetc(fA);
        if (a >= 'a' && a <= 'z') {
            if (b >= '0' && b <= '9') {
                return 3;
            } else {
                return 4;
            }
        } else {
            return 5;
        }
    }
}

int file_fgets(FILE *fA) {
    char a[8];
    fgets(a, 6, fA);
    if (a[0] == 'u' && a[1] == 't' && a[2] == 'b' && a[3] == 'o' && a[4] == 't') {
        return 1;
    }
    return 0;
}

char file_fputc(int x, int y, FILE *fA) {
    if (x < y) {
        fputc('<', fA);
        return '<';
    } else if (x > y) {
        fputc('>', fA);
        return '>';
    } else {
        fputc('=', fA);
        return '=';
    }
}

char file_fputs(char c, FILE *fA) {
    if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u') {
        char a[] = "Vowel";
        fputs("Vowel", fA);
        return 'V';
    } else {
        char a[] = "Consonant";
        fputs("Consonant", fA);
        return 'C';
    }
}

int sum_two_from_file(FILE *f) {
    int x, y;
    fread(&x, sizeof(int), 1, f);
    fread(&y, sizeof(int), 1, f);
    if (x + y > 0) {
        return 1;
    } else if (x + y < 0) {
        return -1;
    } else {
        return 0;
    }
}

int file_fread(FILE *fA, FILE *fB) {
    int arrA[4];
    int arrB[4];
    fread(arrA, sizeof(int), 4, fA);
    fread(arrB, sizeof(int), 4, fB);

    int sumA = 0, sumB = 0;
    for (int i = 0; i < 4; i++) {
        sumA += arrA[i];
        sumB += arrB[i];
    }
    if (sumA == sumB) {
        return 0;
    } else if (sumA > sumB) {
        return 1;
    } else {
        return -1;
    }
}

char file_fwrite(FILE *fA, int x) {
    if (x > 0) {
        char a[] = "Positive";
        fwrite(a, sizeof(char), 8, fA);
        return 'P';
    } else if (x < 0) {
        char a[] = "Negative";
        fwrite(a, sizeof(char), 8, fA);
        return 'N';
    } else {
        char a[] = "Zero";
        fwrite(a, sizeof(char), 4, fA);
        return 'Z';
    }
}
