#ifndef PARSER_H_
# define PARSER_H_

# include <solver/cnf.h>
# include <stdio.h>

int parse_dimacs (FILE *, struct cnf *);

#endif /* PARSER_H_ */
