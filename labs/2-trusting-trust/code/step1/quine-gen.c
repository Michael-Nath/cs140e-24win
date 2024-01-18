// convert the contents of stdin to their ASCII values (e.g., 
// '\n' = 10) and spit out the <prog> array used in Figure 1 in
// Thompson's paper.
#include <stdio.h>

int main(void) { 
    // put your code here.
    char c;
    char buffer[10000];
    int amnt = 0;
    c = fgetc(stdin);
    printf("char prog[] = {\n");
    while (c != EOF) {
        buffer[amnt++] = c;
        printf("\t%d,%c", c, (amnt)%8==0 ? '\n' : ' ');
        c = fgetc(stdin);
    }
    printf("0 };\n");
    for (int i = 0; i < amnt; i++) {
        printf("%c", buffer[i]);
    }
	return 0;
}
