#include <solver/solve.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

static int
is_clause_empty (cnf, i)
     struct cnf *cnf;
     size_t	i;
{
  size_t j;

  for (j = 0; j < cnf->var_n; j++)
    {
      if (cnf->clauses[i][j])
	return 0;
    }

  return 1;
}

static int
simplify (cnf, sig, i, v)
     struct cnf *cnf;
     int8_t	*sig;
     size_t	i;
     int8_t	v;
{
  int		r = 0;
  size_t	j;
  sig[i] = v;

  for (j = 0; j < cnf->clause_n;)
    {
      int8_t w = cnf->clauses[j][i];

      /* Γ[i := T] - the clause becomes valid */
      if (w == v)
	{
	  r |= 1;
	  if (del_clause (cnf, j))
	    fprintf (stderr, "hal: failed to delete clause");

	  continue;
	}
      /* Γ[i := 0] */
      else if (w == -v)
	{
	  r |= 1;
	  cnf->clauses[j][i] = 0;
	}

      j += 1;
    }

  return r;
}

static int
empty_clause (cnf)
     struct cnf *cnf;
{
  size_t i;

  for (i = 0; i < cnf->clause_n; i++)
    {
      if (is_clause_empty (cnf, i))
	return 1;
    }

  return 0;
}

static int
is_unit (cnf, i)
     struct cnf *cnf;
     size_t	i;
{
  int first = -1;
  size_t j;

  for (j = 0; j < cnf->var_n; j++)
    {
      if (cnf->clauses[i][j])
	{
	  if (first == -1)
	    {
	      first = j;
	    }
	  else
	    return -1;
	}
    }

  return first;
}

static int
find_unit (cnf, i, v)
     struct cnf *cnf;
     size_t	*i;
     int8_t	*v;
{
  size_t k;

  for (k = cnf->clause_n; k > 0;)
    {
      int x;

      k -= 1;
      x = is_unit (cnf, k);

      if (x != -1)
	{
	  *i = (size_t)x;
	  *v = cnf->clauses[k][*i];

#ifdef _DEBUG
	  printf ("+ found unit clause %ld (variable is %ld = %hd)\n",
		  k, *i, *v);
#endif

	  return 1;
	}
    }

  return 0;
}

static int
find_pure (cnf, i, v)
     struct cnf *cnf;
     size_t	*i;
     int8_t	*v;
{
  size_t k;
  /* 0  - not seen
   * 1  - pure positive
   * -1 - pure negative
   * 2  - impure
   */
  int8_t *tsig = malloc (cnf->var_n);

  if (!tsig)
    return 0;

  memset (tsig, 0, cnf->var_n);

  for (k = cnf->clause_n; k > 0;)
    {
      size_t j;

      k -= 1;

      for (j = 0; j < cnf->var_n; j++)
	{
	  int8_t x = cnf->clauses[k][j];

	  /* if not seen yet, replace anyways */
	  /* if same value, purity is kept */
	  if (tsig[j] == 0 || tsig[j] == x)
	    tsig[j] = x;
	  /* if different value or already impure, set to impure */
	  else
	    tsig[j] = 2;
	}
    }

  for (k = 0; k < cnf->var_n; k++)
    {
      if (tsig[k] == 1 || tsig[k] == -1)
	{
	  *i = k;
	  *v = tsig[k];

#ifdef _DEBUG
	  printf ("+ found pure variable %ld (val. %hd)\n", *i, *v);
#endif

	  return 1;
	}
    }

  return 0;
}

size_t
select_variable (cnf, sig)
     struct cnf *cnf;
     int8_t	*sig;
{
  size_t i;

  for (i = 0; i < cnf->clause_n; i++)
    {
      size_t j;

      for (j = 0; j < cnf->var_n; j++)
	{
	  if (cnf->clauses[i][j] && !sig[j])
	    return j;
	}
    }

  fprintf (stderr, "hal: failed to select variable\n");

  exit (1);
}

static void
print_sig (cnf, sig)
     struct cnf *cnf;
     int8_t	*sig;
{
  size_t i;

  for (i = 0; i < cnf->var_n; i++)
    {
      printf ("%ld = %hd ; ", i, sig[i]);
    }

  putchar ('\n');
}

/*
 * `cnf` contains no valid clauses, as the storage format for clauses
 * prevents it by design
 */
int
solve (cnf, sig, rcl)
     struct cnf *cnf;
     int8_t	*sig;
     size_t	*rcl;
{
  size_t	i, j;
  int8_t	v, *save;
  struct cnf	cnf2;

  *rcl += 1;

#ifdef _DEBUG
  printf ("> solving for %ld clauses\n", cnf->clause_n);
  print_sig (cnf, sig);
#endif

  for (i = 0; i < cnf->var_n; i++)
    {
      if (sig[i] < -1 || sig[i] > 1)
	sig[i] = 0;
    }

  for (; find_unit (cnf, &i, &v) || find_pure (cnf, &i, &v);)
    {
      if (!simplify (cnf, sig, i, v))
	return -1;
    }

  if (cnf->clause_n == 0)
      return 1;

  if (empty_clause (cnf))
      return 0;

  i = select_variable (cnf, sig);

#ifdef _DEBUG
  printf ("- selecting variable %ld\n", i);
#endif

  save = malloc (cnf->var_n);

  if (!save)
    return -1;

  memcpy (save, sig, cnf->var_n);

  if (!(j = add_clause (cnf)))
    {
      free (save);
      return -1;
    }

  /* add_clause returns i + 1 */
  j -= 1;

  cnf->clauses[j][i] = -1;
#ifdef _DEBUG
  printf ("\t== Γ[%ld = -1] ==", i);
#endif

  copy_cnf (&cnf2, cnf);

  switch (solve (&cnf2, save, rcl))
    {
    case 1:
      memcpy (sig, save, cnf->var_n);
      return 1;
    case -1:
      free (save);
      return -1;
    default:
      break;
    }

  free (save);

  cnf->clauses[j][i] = 1;

#ifdef _DEBUG
  printf ("\t== Γ[%ld = 1] ==", i);
#endif

  return (solve (cnf, sig, rcl));
}
