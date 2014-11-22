#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define fatal(msg)\
    do {\
        fflush(stdout);\
        fprintf(stderr, "%s\n", msg);\
        abort();\
    } while (0)

#define array(p,x,y,w) p[((y) * (w) + (x))]

/* Directional movement */
#define RIGHT 0 
#define UP    1
#define DOWN  2
#define LEFT  3

/* The fungespace struct stores the data for an instruction pointer, and also
 * the information each pointer requires for the topology of the space we are
 * executing on. */
struct fungespace {
    int x;   /* Current x position */
    int y;   /* Current y position */
    int dx;  /* Current x delta */
    int dy;  /* Current y delta */
    int w;   /* The width of the space we are in */
    int h;   /* The height of the space we are in */
    int dim; /* The dimension of the given fungespace */
};

/* Alter the passed fungespace to a random direction */
void random_direction(struct fungespace *f)
{
    static int seeded = 0;
    if (!seeded) { srand(time(NULL)); seeded = 1; }

    f->dx = f->dy = 0;
    switch (rand() & ((1 << f->dim) - 1)) {
        case RIGHT: f->dx =  1; break;
        case UP:    f->dy = -1; break;
        case DOWN:  f->dy =  1; break;
        case LEFT:  f->dx = -1; break;
    }
}

/* Move the instruction pointer to it's next location in space */
void update_position(struct fungespace *f)
{
    f->x += f->dx;
    f->y += f->dy;

         if (f->y == f->h) f->y = 0;
    else if (f->x == f->w) f->x = 0;
    else if (f->y < 0)     f->y = f->h - 1;
    else if (f->x < 0)     f->x = f->w - 1;
}

/* Stack operations */
#define STACK_SIZE 8192

#define STYPE_FMT "%ld"
typedef signed long int stype;

static stype stack[STACK_SIZE];
static size_t sp = 0;

inline void stack_push(int op1)
{
    if (sp + 1 > STACK_SIZE)
        fatal("stack overflow");

    stack[sp++] = op1;
}

inline stype stack_pop(void)
{
#ifdef CATCH_UNDERFLOW
    if (sp - 1 < 0)
        fatal("stack underflow");
    return stack[--sp];
#else
    return !sp ? 0 : stack[--sp];
#endif
}

inline void pop2(stype *a, stype *b)
{
    *a = stack_pop();
    *b = stack_pop();
}

/* Program execution operations */
int funge_tick(char *p, struct fungespace *f)
{
    static int stringmode = 0;

    if (stringmode && array(p, f->x, f->y, f->w) != '"') {
        stack_push(array(p, f->x, f->y, f->w));
    }
    else { switch (array(p, f->x, f->y, f->w)) {
        /* Generic variables which may be used during switch */
        stype a, b;
        case '+':
            pop2(&a, &b);
            stack_push(a + b);
            break;
        case '-':
            pop2(&a, &b);
            stack_push(b - a);
            break;
        case '*':
            pop2(&a, &b);
            stack_push(a * b);
            break;
        case '/':
            pop2(&a, &b);
            stack_push(b / a);
            break;
        case '%':
            pop2(&a, &b);
            stack_push(b % a);
            break;
        case '!':
            stack_push(!stack_pop() ? 1 : 0);
            break;
        case '`':
            pop2(&a, &b);
            stack_push(b > a ? 1 : 0);
            break;
        case '>':
            f->dx = f->dy = 0;
            f->dx = 1;
            break;
        case '<':
            f->dx = f->dy = 0;
            f->dx = -1;
            break;
        case '^':
            f->dx = f->dy = 0;
            f->dy = -1;
            break;
        case 'v':
            f->dx = f->dy = 0;
            f->dy = 1;
            break;
        case '?':
            random_direction(f);
            break;
        case '_':
            f->dx = f->dy = 0;
            f->dx = !stack_pop() ? 1 : -1;
            break;
        case '|':
            f->dx = f->dy = 0;
            f->dy = !stack_pop() ? 1 : -1;
            break;
        case '"':
            stringmode = !stringmode;
            break;
        case ':':
            a = stack_pop();
            stack_push(a);
            stack_push(a);
            break;
        case '\\':
            pop2(&a, &b);
            stack_push(a);
            stack_push(b);
            break;
        case '$':
            a = stack_pop();
            break;
        case '.':
            printf(STYPE_FMT " ", stack_pop());
            break;
        case ',':
            printf("%c", (char)stack_pop());
            break;
        case '#':
            update_position(f);
            break;
        case 'g':
            pop2(&a, &b);
            stack_push(array(p, b, a, f->w));
            break;
        case 'p':
            pop2(&a, &b);
            array(p, b, a, f->w) = stack_pop();
            break;
        case '&':
            b = fscanf(stdin, STYPE_FMT, &a);
            stack_push(a);
            break;
        case '~':
            a = fgetc(stdin);
            stack_push(a);
            break;
        case '@':
            return 1; 
        case '0' ... '9':
            stack_push(array(p, f->x, f->y, f->w) - '0');
            break;
        case ' ':
        default:
            /* ignore invalid */
            break;

    } /* switch */ } /* else */

    return 0;
}

/* Attempt to load an arbitary sized funge program, dealing properly with 
 * whitespace and size constraints */
char* funge_load(char *filename, struct fungespace *f)
{
    int i, j;
    FILE *fd;
    char *rawdata;
    long fs;

    /* Read entire file */
    fd = fopen(filename, "r");
    fseek(fd, 0, SEEK_END);
    fs = ftell(fd);
    fseek(fd, 0, SEEK_SET);
    rawdata = malloc(fs);
    /* Unused */ i = fread(rawdata, 1, fs, fd);
    fclose(fd);

    /* Search for longest line to construct grid */
    int len_line = 0;
    int max_line = 0;
    int no_line  = 0;
    for (i = 0; i < fs; ++i, ++len_line) {
        if (rawdata[i] == '\n') {
            if (len_line > max_line)
                max_line = len_line;
            len_line = -1;
            ++no_line;
        }
    }

    if (len_line > max_line)
        max_line = len_line;

    /* Use a 1-d array, but treat it as 2d */
    char *fmtdata = malloc(max_line * no_line);
    int lc = 0;
    for (i = 0, j = 0; i < fs; ++i, ++lc) {
        if (rawdata[i] == '\n') {
            while (lc++ < max_line) {
                fmtdata[j++] = ' ';
            }
            lc = -1;
        }
        else {
            fmtdata[j++] = rawdata[i];
        }
    }

    f->w = max_line;
    f->h = no_line;
    free(rawdata);
    return fmtdata;
}

int main(int argc, char **argv)
{
    if (argc < 2) return 0;

    struct fungespace pos = { 0 }; pos.dx = 1; pos.dim = 2;
    char *program = funge_load(argv[1], &pos);

#ifdef DEBUG
    int x, y; 
    for (y = 0; y < pos.h; ++y) {
        for (x = 0; x < pos.w; ++x)
            printf("%c", array(program, x, y, pos.w));
        printf("[EOL]\n");
    }
    printf("\n");
#endif

    int end = 0;
    while (!end) {
        end = funge_tick(program, &pos);
        update_position(&pos);
    }

    fflush(stdout);
    free(program);
    return 0;
}
