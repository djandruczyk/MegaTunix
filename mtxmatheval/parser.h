#ifndef BISON_PARSER_H
# define BISON_PARSER_H

#ifndef YYSTYPE
typedef union {
  Node *node;
  Record *record;
} yystype;
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif
# define	NUMBER	257
# define	VARIABLE	258
# define	FUNCTION	259
# define	NEG	260
# define	END	261


extern YYSTYPE yylval;

#endif /* not BISON_PARSER_H */
