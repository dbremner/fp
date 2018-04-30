#ifndef LEX_H
#define LEX_H

void lex_init(FILE *f);
void set_prompt(char ch);
int yylex(void);
void fp_cmd(void);

#endif
