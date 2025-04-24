#include <solver/cnf.h>
#include <stdlib.h>
#include <string.h>

int
init_cnf (cnf, var_n, clause_n)
     struct cnf	*cnf;
     size_t	var_n;
     size_t	clause_n;
{
  size_t i;

  cnf->var_n = var_n;
  cnf->clause_n = clause_n;
  cnf->_clause_c = clause_n;

  cnf->clauses = malloc (sizeof(clause_t) * clause_n);

  if (!cnf->clauses)
    return 1;

  for (i = 0; i < clause_n; i++)
    {
      cnf->clauses[i] = malloc (var_n);

      if (!cnf->clauses[i])
	goto cleanup;

      memset (cnf->clauses[i], 0, var_n);
    }

  return 0;

 cleanup:
  for (; i > 0; --i)
    free (cnf->clauses[i]);

  free (cnf->clauses);

  return 1;
}

int
copy_cnf (dest, src)
     struct cnf *dest;
     struct cnf *src;
{
    size_t i;

  dest->var_n = src->var_n;
  dest->clause_n = src->clause_n;
  dest->_clause_c = src->_clause_c;

  dest->clauses = malloc (sizeof(clause_t) * dest->clause_n);

  if (!dest->clauses)
    return 1;

  for (i = 0; i < dest->clause_n; i++)
    {
      dest->clauses[i] = malloc (dest->var_n);

      if (!dest->clauses[i])
	goto cleanup;

      memcpy (dest->clauses[i], src->clauses[i], dest->var_n);
    }

  return 0;

 cleanup:
  for (; i > 0; --i)
    free (dest->clauses[i]);

  free (dest->clauses);

  return 1;
}

size_t
add_clause (cnf)
     struct cnf *cnf;
{
  clause_t *nc;

  if (cnf->clause_n >= cnf->_clause_c)
    {
      cnf->_clause_c *= 2;
      nc = realloc (cnf->clauses, sizeof(clause_t) * cnf->_clause_c);

      if (!nc)
	return 0;

      cnf->clauses = nc;
    }

  cnf->clauses[cnf->clause_n] = malloc (cnf->var_n);

  if (!cnf->clauses[cnf->clause_n])
    return 0;

  memset (cnf->clauses[cnf->clause_n], 0, cnf->var_n);

  return (++cnf->clause_n);
}

int
del_clause (cnf, i)
     struct cnf *cnf;
     size_t	i;
{
  size_t j;

  if (i >= cnf->clause_n)
    return 1;

  for (j = i; j < cnf->clause_n - 1; j++)
    {
      cnf->clauses[j] = cnf->clauses[j + 1];
    }

  cnf->clause_n -= 1;

  return 0;
}

void
destroy_cnf (cnf)
     struct cnf *cnf;
{
  size_t i;

  for (i = 0; i < cnf->_clause_c; i++)
    free (cnf->clauses[i]);

  free (cnf->clauses);
}
