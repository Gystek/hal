#include <getopt.h>
#include <parser.h>
#include <cnf.h>
#include <solve.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int
process_file (fp)
     FILE *fp;
{
  int		r = 1;
  size_t	rcl = 0;
  int8_t	*sig;
  int		sol = -1;
  struct cnf	cnf;

  clock_t	beg, end;

  (void)beg;
  (void)end;

  if ((r = parse_dimacs (fp, &cnf)))
    {
      fprintf (stderr, "hal: failed to read file\n");
      return r;
    }

  sig = malloc (cnf.var_n);

  if (!sig)
    {
        destroy_cnf (&cnf);
	goto cleanup;
    }

  memset (sig, 0, cnf.var_n);

  beg = clock ();
  sol = solve (&cnf, sig, &rcl);
  end = clock ();

  switch (sol)
    {
    case 1:
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
    case 0:
      puts ("⊥");

      r = 1;
      goto cleanup;
    default:
      fprintf (stderr, "hal: failed to compute satisfiability\n");
      break;
    }

 cleanup:

#ifdef _HEURISTICS_JWOS
  free (jwos_j);
#endif

#ifdef _PERF
  if (sol >= 0)
    printf ("solved in %ld calls (%f seconds)\n",
	    rcl, (double)(end - beg) / CLOCKS_PER_SEC);
#endif
  free (sig);

  return r;
}

int
main (argc, argv)
     int argc;
     char *const argv[];
{
  int i, r = 0;

  if (argc == 1)
    {
      return process_file (stdin);
    }

  for (i = 1; i < argc; i++)
    {
      int s;

      FILE *fp;

      if ((s = (strcmp (argv[i], "-") == 0)))
	fp = stdin;
      else
	fp = fopen (argv[i], "r");

      printf ("%s: ", argv[i]);

      r |= process_file (fp);

      if (!s)
	fclose (fp);

    }

  return r;
}
