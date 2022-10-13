#include <stdio.h>
#include "input_output.h"

int simple_getc() {
    unsigned char a = getc(stdin);
    if (a == '0') {
        return 0;
    } else if (a == '1') {
        return 1;
    } else if (a == '2') {
        return 2;
    } else if (a == '3') {
        return 3;
    } else if (a == '4') {
        return 4;
    } else if (a == '5') {
        return 5;
    } else if (a == '6') {
        return 6;
    } else if (a == '7') {
        return 7;
    } else if (a == '8') {
        return 8;
    } else if (a == '9') {
        return 9;
    }
    return -1;
}

int simple_fgetc(int x) {
    if (x >= 0 && x <= 9) {
        unsigned char a = fgetc(stdin);
        if (a >= 'a' && a <= 'z') {
            return 1;
        } else {
            return 2;
        }
    } else {
        unsigned char a = fgetc(stdin);
        unsigned char b = fgetc(stdin);
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

int simple_fread() {
    int arrA[4];
    int arrB[4];
    fread(arrA, sizeof(int), 4, stdin);
    fread(arrB, sizeof(int), 4, stdin);

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

int simple_fgets() {
    char a[8];
    fgets(a, 6, stdin);
    if (a[0] == 'u' && a[1] == 't' && a[2] == 'b' && a[3] == 'o' && a[4] == 't') {
        return 1;
    }
    return 0;
}

int simple_getchar() {
    unsigned char a = getchar();
    unsigned char b = getchar();

    if (a + b < 0) {
        return -1;
    }

    if (a + b < 100) {
        return 0;
    } else if (a + b < 200) {
        return 1;
    } else {
        return 2;
    }
}

int simple_gets() {
    char a[8];
    gets(a);
    if (a[0] == 'u' && a[1] == 't' && a[2] == 'b' && a[3] == 'o' && a[4] == 't') {
        return 1;
    }
    return 0;
}

char simple_putc(int x, int y) {
    if (x + y > 0) {
        putc('1', stdout);
        return '1';
    } else if (x + y < 0) {
        putc('2', stdout);
        return '2';
    } else {
        putc('0', stdout);
        return '0';
    }
}

char simple_fputc(int x, int y) {
    if (x < y) {
        fputc('<', stdout);
        return '<';
    } else if (x > y) {
        fputc('>', stdout);
        return '>';
    } else {
        fputc('=', stdout);
        return '=';
    }
}

char simple_fwrite(int x) {
    if (x > 0) {
        char a[] = "Positive";
        fwrite(a, sizeof(char), 8, stdout);
        return 'P';
    } else if (x < 0) {
        char a[] = "Negative";
        fwrite(a, sizeof(char), 8, stdout);
        return 'N';
    } else {
        char a[] = "Zero";
        fwrite(a, sizeof(char), 4, stdout);
        return 'Z';
    }
}

char simple_fputs(char c) {
    if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u') {
        char a[] = "Vowel";
        fputs("Vowel", stdout);
        return 'V';
    } else {
        char a[] = "Consonant";
        fputs("Consonant", stdout);
        return 'C';
    }
}

char simple_putchar(int x, int y) {
    if (3 * x > 2 * y) {
        putchar('>');
        return '>';
    } else if (3 * x < 2 * y) {
        putchar('<');
        return '<';
    } else {
        putchar('=');
        return '=';
    }
}

char simple_puts(char c) {
    if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u') {
        char a[] = "Vowel";
        puts(a);
        return 'V';
    } else {
        char a[] = "Consonant";
        puts(a);
        return 'C';
    }
}
