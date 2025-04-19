#include <getopt.h>
#include <parser.h>
#include <solver/cnf.h>
#include <solver/solve.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int
process_file (fp)
     FILE *fp;
{
  int		r = 1;
  int8_t	*sig;
  struct cnf	cnf;

  if ((r = parse_dimacs (fp, &cnf)))
    {
      fprintf (stderr, "hal: failed to read file\n");
      return r;
    }

  sig = malloc (cnf.var_n);

  if (!sig)
    goto cleanup;

  memset (sig, 0, cnf.var_n);

  if (solve (&cnf, sig))
    {
      size_t i;

      for (i = 0; i < cnf.var_n; i++)
	{
	  if (i)
	    putchar (' ');
	  printf ("%hd", sig[i] >= 0 ? 1 : 0);
	}

      putchar ('\n');

      r = 0;
      goto cleanup;
    }
  else
    {
      puts ("⊥");

      r = 1;
      goto cleanup;
    }

 cleanup:
  free (sig);
  destroy_cnf (&cnf);

  return r;
}

int
main (argc, argv)
     int argc;
     char *const argv[];
{
  int i;

  if (argc == 1)
    {
      return process_file (stdin);
    }

  for (i = 1; i < argc; i++)
    {
      int r, s;

      FILE *fp;

      if ((s = (strcmp (argv[i], "-") == 0)))
	fp = stdin;
      else
	fp = fopen (argv[i], "r");

      r = process_file (fp);

      if (!s)
	fclose (fp);

      if (r)
	return r;
    }

  return 0;
}
