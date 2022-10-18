int very_easy(int x) {
    asm(" ");
    return 3 * x;
}

int asm_example(int a) {
    if (a == 42) {
        return -1;
    }
    asm("addl  $5, %0" : "+r"(a));
    return a;
}

int jmp_asm(int a) {
    asm volatile ("mov $0x10, %%eax\n\t"
        "sub %%eax, %0\n\t"
        "cmp $4, %0\n\t"
        "jle end\n\t"
        "add $0300, %0\n\t"
        "end:\n\t"
    : "+r"(a)
    :: "eax");
    return a;
}

float float_avx(float a, float b) {
    if (a == 239) {
        float data[4] = {a, b, 1, 9};
        float res;

        asm("movups %0, %%xmm0\n\t"
            "haddps %%xmm0, %%xmm0\n\t"
            "haddps %%xmm0, %%xmm0\n\t"
            "movss %%xmm0, %1\n\t"
            : "=m"(data)
            : "m"(res)
            : "xmm0");

        return res;
    }
    return 42;
}
