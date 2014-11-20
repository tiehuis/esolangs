#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

inline unsigned int rotr(unsigned int op)
{
    return (op / 3) + (op % 3) * 19683;
}

inline unsigned int tadd(unsigned int rop, unsigned int op)
{
    return (rop + op) % 59049;
}

inline unsigned int tsub(unsigned int rop, unsigned int op)
{
    return op > rop ? op = 59048 - (op - rop) : (rop - op) % 59049;
}

unsigned int crz(unsigned int op1, unsigned int op2)
{
    static const int table[] = { 1, 0, 0, 1, 0, 2, 2, 2, 1 };
    unsigned int rop = 0;
    unsigned int dig = 1;

    int i;
    for (i = 0; i < 10; ++i) {
        rop += table[3*(op1 % 3) + (op2 % 3)] * dig;
        op1 /= 3; op2 /= 3; dig *= 3;
    }

    return rop;
}

unsigned int encrypt(unsigned int op1)
{
    static const int enctable[] = {
        57, 109, 60, 46, 84, 86, 97, 99, 96, 117,
        89, 42, 77, 75, 39, 88, 126, 120, 68, 108,
        125, 82, 69, 111, 107, 78, 58, 35, 63, 71,
        34, 105, 64, 53, 122, 93, 38, 103, 113, 116,
        121, 102, 114, 36, 40, 119, 101, 52, 123, 87,
        80, 41, 72, 45, 90, 110, 44, 91, 37, 92,
        51, 100, 76, 43, 81, 59, 62, 85, 33, 112,
        74, 83, 55, 50, 70, 104, 79, 65, 49, 67,
        66, 54, 118, 94, 61, 73, 95, 48, 47, 56,
        124, 106, 115, 98
    };

    if (op1 > 93) {
        fprintf(stderr, "encrypt(): value to encrypt is out of range\n");
        abort();
    }

    return enctable[op1];
}

int main(int argc, char **argv)
{
    if (argc < 2) return;

    int a = 0;
    int c = 0;
    int d = 0;

    /* Max value is 59048 */
    unsigned int m[59048] = { 0 }; 
    
    FILE *fd; long fs;
    fd = fopen(argv[1], "r");
    fseek(fd, 0, SEEK_END);
    fs = ftell(fd);
    fseek(fd, 0, SEEK_SET);
    if (fs > 59048) return;
    char *pc = malloc(fs);
    int t_ = fread(pc, 1, fs, fd);

    int i, j = 0;
    for (i = 0; i < fs; ++i)
        if (!isspace(pc[i]))
            m[j++] = pc[i];

    for (i = j + 1; i < 59049; ++i)
        pc[i] = crz(pc[tsub(i, 1)], pc[tsub(i, 2)]); 

    while (1) {
        int op = tadd(m[c], c) % 94;
        switch (op) {
            case 4:
                d = tadd(m[d], 1);
                break;
            case 5:
                printf("%c", a);
                break;
            case 23:
                a = getchar() == EOF ? 59048 : a;
                break;
            case 39:
                m[d] = rotr(m[d]);
                a = m[d];
                break;
            case 40:
                d = m[d];
                break;
            case 62:
                a = crz(m[d], a);
                m[d] = a;
                break;
            case 81:
                return 0;
            case 68:
            default:
                /* nop */
                break;
        }

        if (op != 4)
            m[c] = encrypt(m[c] % 94);

        c = tadd(c, 1);
        d = tadd(d, 1);
    }
}
