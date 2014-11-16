#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libtcc.h>

#define emitline(buf)\
    do {\
        const size_t sl = strlen(buf);\
        if (outsize + sl >= outalloc) {\
            outalloc *= 2;\
            out = realloc(out, outalloc);\
        }\
        memmove(out + outsize, buf, sl);\
        outsize += sl;\
    } while (0)

char* generate(char *in, const int is)
{
    int outalloc = 1028;
    int outsize  = 0;
    char *out    = malloc(1028);
    char tbuf[32];

    emitline("int main(void){");
    emitline("unsigned char array[30000]={0};");
    emitline("unsigned char*ptr=array;");

    int i;
    for (i = 0; i < is; ++i) {
        int jump = 0;

        switch (in[i]) {
        case '>':
        case '<':
            while (i < is && (in[i] == '>' || in[i] == '<'))
                jump += in[i++] == '>' ? 1 : -1;
            if (i < is && (in[i] != '<' || in[i] != '>')) 
                i--;

            snprintf(tbuf, 32, "ptr+=%d;", jump);
            emitline(tbuf);
            break;
        case '+':
        case '-':
            while (i < is && (in[i] == '+' || in[i] == '-'))
                jump += in[i++] == '+' ? 1 : -1;
            if (i < is && (in[i] != '<' || in[i] != '>')) 
                i--;

            snprintf(tbuf, 32, "*ptr+=%d;", jump);
            emitline(tbuf);
            break;
        case '.':
            emitline("putchar(*ptr);");
            break;
        case ',':
            emitline("{char c=getchar();if(c!=-1)*ptr=c;}");
            break;
        case '[':
            emitline("while(*ptr){");
            break;
        case ']':
            emitline("}");
            break;
        }
    }

    emitline("return 0;}");
    out[outsize] = 0;
    return out;
}

int main(int argc, char **argv)
{
    if (argc < 2) return;

    FILE *fd;
    char *p, *np;
    long fl;

    fd = fopen(argv[1], "rb");
    fseek(fd, 0, SEEK_END);
    fl = ftell(fd);
    fseek(fd, 0, SEEK_SET);
    p = malloc(fl);
    if(fread(p, fl, 1, fd)); /* Unused return values, great */
    fclose(fd);

    
    np = generate(p, fl);
    TCCState *s = tcc_new();
    tcc_set_output_type(s, TCC_OUTPUT_MEMORY);
    tcc_compile_string(s, np);
    tcc_run(s, 0, NULL);
    tcc_delete(s);
    return 0;
}
