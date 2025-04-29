#include <getopt.h>
#include <parser.h>
#include <cnf.h>
#include <linux/limits.h>
#include <solve.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int
process_file (fp, outfp)
     FILE *fp;
     FILE *outfp;
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

	if (outfp)
	  fputs ("SAT\n", outfp);

	for (i = 0; i < cnf.var_n; i++)
	  {
	    if (i)
	      putchar (' ');
	    if (sig[i] < 0)
	      putchar ('-');
	    printf ("%ld", i + 1);

	    if (outfp)
	      {
		if (sig[i] < 0)
		  fputc ('-', outfp);

		fprintf (outfp, "%ld ", i + 1);
	      }
	  }

	putchar ('\n');

	if (outfp)
	  {
	    fputs ("0\n", outfp);
	  }

	r = 0;
	goto cleanup;
      }
    case 0:
      puts ("⊥");

      if (outfp)
	fputs ("UNSAT", outfp);

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
  FILE	*outfp;
  char	outfile[PATH_MAX] = { 0 };
  int	opt, i, r = 0;

  while ((opt = getopt (argc, argv, "s:")) != -1)
    {
      switch (opt)
	{
	case 's':
	  strcpy (outfile, optarg);
	  break;
	default:
	  fprintf (stderr, "Usage: %s [-s outfile] [file...]\n", argv[0]);
	  return EXIT_FAILURE;
	}
    }

  if (strlen (outfile) == 0)
    outfp = NULL;
  else
    {
      outfp = fopen (outfile, "w");

      if (!outfp)
	{
	  perror ("hal");

	  return EXIT_FAILURE;
	}
    }

  if (optind >= argc)
    {
      return process_file (stdin, outfp);
    }

  for (i = optind; i < argc; i++)
    {
      int s;

      FILE *fp;

      if ((s = (strcmp (argv[i], "-") == 0)))
	fp = stdin;
      else
	fp = fopen (argv[i], "r");

      printf ("%s: ", argv[i]);

      r |= process_file (fp, outfp);

      if (!s)
	fclose (fp);
    }

  if (outfp)
    fclose (outfp);

  return r;
}
