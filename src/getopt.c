/* Getopt for GNU.
   NOTE: getopt is now part of the C library, so if you don't know what
   "Keep this file name-space clean" means, talk to roland@gnu.ai.mit.edu
   before changing it!

   Copyright (C) 1987, 88, 89, 90, 91, 92, 1993
        Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2, or (at your option) any
   later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* Modified for use with hexcurse by Blake Matheny <matheny@dbaseiv.net> */

#include <stdio.h>
#include <stdlib.h>
#include "hgetopt.h"

#undef optarg
#undef opterr
#undef optopt
#undef optind

#include <alloca.h>

#ifdef HAVE_OPTIND
extern char* optarg;
#else
char *optarg = 0;
#endif

#ifdef HAVE_OPTIND
extern int optind;
#else
int optind = 0;
#endif

static char *nextchar;

#ifdef HAVE_OPTIND
extern int opterr;
#else
int opterr = 1;
#endif

#ifdef HAVE_OPTIND
extern int optint;
#else
int optopt = '?';
#endif

static enum
{
  REQUIRE_ORDER, PERMUTE, RETURN_IN_ORDER
} ordering, default_ordering = PERMUTE;

#include <string.h>
#define	my_index	strchr
#define	my_bcopy(src, dst, n)	memcpy ((dst), (src), (n))

static int first_nonopt;
static int last_nonopt;

static void
exchange (argv)
     char **argv;
{
  int nonopts_size = (last_nonopt - first_nonopt) * sizeof (char *);
  char **temp = (char **) alloca (nonopts_size);

  /* Interchange the two blocks of data in ARGV.  */

  my_bcopy ((char *) &argv[first_nonopt], (char *) temp, nonopts_size);
  my_bcopy ((char *) &argv[last_nonopt], (char *) &argv[first_nonopt],
	    (optind - last_nonopt) * sizeof (char *));
  my_bcopy ((char *) temp,
	    (char *) &argv[first_nonopt + optind - last_nonopt],
	    nonopts_size);

  /* Update records for the slots the non-options now occupy.  */

  first_nonopt += (optind - last_nonopt);
  last_nonopt = optind;
}

/* Call this function with an argument of 1 to set the default option
   ordering to that required by Posix.  The normal default is PERMUTE. */
void
getopt_set_posix_option_order (on_or_off)
     int on_or_off;
{
  if (on_or_off == 1)
    default_ordering = REQUIRE_ORDER;
  else
    default_ordering = PERMUTE;
}
	
int
_getopt_internal (argc, argv, optstring, longopts, longind, long_only)
     int argc;
     char *const *argv;
     const char *optstring;
     const struct option *longopts;
     int *longind;
     int long_only;
{
  int option_index;

  optarg = 0;

  if (optind > argc || optind < 0)
    {
      optind = argc;
      return (EOF);
    }

  /* Initialize the internal data when the first call is made.
     Start processing options with ARGV-element 1 (since ARGV-element 0
     is the program name); the sequence of previously skipped
     non-option ARGV-elements is empty.  */

  if (optind == 0)
    {
      first_nonopt = last_nonopt = optind = 1;

      nextchar = NULL;

      /* Determine how to handle the ordering of options and nonoptions.  */

      if (optstring[0] == '-')
	{
	  ordering = RETURN_IN_ORDER;
	  ++optstring;
	}
      else if (optstring[0] == '+')
	{
	  ordering = REQUIRE_ORDER;
	  ++optstring;
	}
      else if (getenv ("POSIXLY_CORRECT") != NULL)
	ordering = REQUIRE_ORDER;
      else
	ordering = default_ordering;
    }

  if (nextchar == NULL || *nextchar == '\0')
    {
      if (ordering == PERMUTE)
	{
	  /* If we have just processed some options following some non-options,
	     exchange them so that the options come first.  */

	  if (first_nonopt != last_nonopt && last_nonopt != optind)
	    exchange ((char **) argv);
	  else if (last_nonopt != optind)
	    first_nonopt = optind;

	  /* Now skip any additional non-options
	     and extend the range of non-options previously skipped.  */

	  while (optind < argc
		 && (argv[optind][0] != '-' || argv[optind][1] == '\0')
#ifdef GETOPT_COMPAT
		 && (longopts == NULL
		     || argv[optind][0] != '+' || argv[optind][1] == '\0')
#endif				/* GETOPT_COMPAT */
		 )
	    optind++;
	  last_nonopt = optind;
	}

      /* Special ARGV-element `--' means premature end of options.
	 Skip it like a null option,
	 then exchange with previous non-options as if it were an option,
	 then skip everything else like a non-option.  */

      if (optind != argc && !strcmp (argv[optind], "--"))
	{
	  optind++;

	  if (first_nonopt != last_nonopt && last_nonopt != optind)
	    exchange ((char **) argv);
	  else if (first_nonopt == last_nonopt)
	    first_nonopt = optind;
	  last_nonopt = argc;

	  optind = argc;
	}

      /* If we have done all the ARGV-elements, stop the scan
	 and back over any non-options that we skipped and permuted.  */

      if (optind == argc)
	{
	  /* Set the next-arg-index to point at the non-options
	     that we previously skipped, so the caller will digest them.  */
	  if (first_nonopt != last_nonopt)
	    optind = first_nonopt;
	  return EOF;
	}

      /* If we have come to a non-option and did not permute it,
	 either stop the scan or describe it to the caller and pass it by.  */

      if ((argv[optind][0] != '-' || argv[optind][1] == '\0')
#ifdef GETOPT_COMPAT
	  && (longopts == NULL
	      || argv[optind][0] != '+' || argv[optind][1] == '\0')
#endif				/* GETOPT_COMPAT */
	  )
	{
	  if (ordering == REQUIRE_ORDER)
	    return EOF;
	  optarg = argv[optind++];
	  return 1;
	}

      /* We have found another option-ARGV-element.
	 Start decoding its characters.  */

      nextchar = (argv[optind] + 1
		  + (longopts != NULL && argv[optind][1] == '-'));
    }

  if (longopts != NULL
      && ((argv[optind][0] == '-'
	   && (argv[optind][1] == '-' || long_only))
#ifdef GETOPT_COMPAT
	  || argv[optind][0] == '+'
#endif				/* GETOPT_COMPAT */
	  ))
    {
      const struct option *p;
      char *s = nextchar;
      int exact = 0;
      int ambig = 0;
      const struct option *pfound = NULL;
      int indfound = 0;

      while (*s && *s != '=')
	s++;

      /* Test all options for either exact match or abbreviated matches.  */
      for (p = longopts, option_index = 0; p->name;
	   p++, option_index++)
	if (!strncmp (p->name, nextchar, s - nextchar))
	  {
	    if (s - nextchar == (int)strlen(p->name))
	      {
		/* Exact match found.  */
		pfound = p;
		indfound = option_index;
		exact = 1;
		break;
	      }
	    else if (pfound == NULL)
	      {
		/* First nonexact match found.  */
		pfound = p;
		indfound = option_index;
	      }
	    else
	      /* Second nonexact match found.  */
	      ambig = 1;
	  }

      if (ambig && !exact)
	{
	  if (opterr)
	    fprintf (stderr, "%s: option `%s' is ambiguous\n",
		     argv[0], argv[optind]);
	  nextchar += strlen (nextchar);
	  optind++;
	  return '?';
	}

      if (pfound != NULL)
	{
	  option_index = indfound;
	  optind++;
	  if (*s)
	    {
	      /* Don't test has_arg with >, because some C compilers don't
		 allow it to be used on enums.  */
	      if (pfound->has_arg)
		optarg = s + 1;
	      else
		{
		  if (opterr)
		    {
		      if (argv[optind - 1][1] == '-')
			/* --option */
			fprintf (stderr,
				 "%s: option `--%s' doesn't allow an argument\n",
				 argv[0], pfound->name);
		      else
			/* +option or -option */
			fprintf (stderr,
			     "%s: option `%c%s' doesn't allow an argument\n",
			     argv[0], argv[optind - 1][0], pfound->name);
		    }
		  nextchar += strlen (nextchar);
		  return '?';
		}
	    }
	  else if (pfound->has_arg == 1)
	    {
	      if (optind < argc)
		optarg = argv[optind++];
	      else
		{
		  if (opterr)
		    fprintf (stderr, "%s: option `%s' requires an argument\n",
			     argv[0], argv[optind - 1]);
		  nextchar += strlen (nextchar);
		  return optstring[0] == ':' ? ':' : '?';
		}
	    }
	  nextchar += strlen (nextchar);
	  if (longind != NULL)
	    *longind = option_index;
	  if (pfound->flag)
	    {
	      *(pfound->flag) = pfound->val;
	      return 0;
	    }
	  return pfound->val;
	}
      /* Can't find it as a long option.  If this is not getopt_long_only,
	 or the option starts with '--' or is not a valid short
	 option, then it's an error.
	 Otherwise interpret it as a short option.  */
      if (!long_only || argv[optind][1] == '-'
#ifdef GETOPT_COMPAT
	  || argv[optind][0] == '+'
#endif				/* GETOPT_COMPAT */
	  || my_index (optstring, *nextchar) == NULL)
	{
	  if (opterr)
	    {
	      if (argv[optind][1] == '-')
		/* --option */
		fprintf (stderr, "%s: unrecognized option `--%s'\n",
			 argv[0], nextchar);
	      else
		/* +option or -option */
		fprintf (stderr, "%s: unrecognized option `%c%s'\n",
			 argv[0], argv[optind][0], nextchar);
	    }
	  nextchar = (char *) "";
	  optind++;
	  return '?';
	}
    }

  /* Look at and handle the next option-character.  */

  {
    char c = *nextchar++;
    char *temp = my_index (optstring, c);

    /* Increment `optind' when we start to process its last character.  */
    if (*nextchar == '\0')
      ++optind;

    optopt = c;

    if (temp == NULL || c == ':')
      {
	if (opterr)
	  {
#if 0
	    if (c < 040 || c >= 0177)
	      fprintf (stderr, "%s: unrecognized option, character code 0%o\n",
		       argv[0], c);
	    else
	      fprintf (stderr, "%s: unrecognized option `-%c'\n", argv[0], c);
#else
	    /* 1003.2 specifies the format of this message.  */
	    fprintf (stderr, "%s: illegal option -- %c\n", argv[0], c);
#endif
	  }
	optopt = c;
	return '?';
      }
    if (temp[1] == ':')
      {
	if (temp[2] == ':')
	  {
	    /* This is an option that accepts an argument optionally.  */
	    if (*nextchar != '\0')
	      {
		optarg = nextchar;
		optind++;
	      }
	    else
	      optarg = 0;
	    nextchar = NULL;
	  }
	else
	  {
	    /* This is an option that requires an argument.  */
	    if (*nextchar != '\0')
	      {
		optarg = nextchar;
		/* If we end this ARGV-element by taking the rest as an arg,
		   we must advance to the next element now.  */
		optind++;
	      }
	    else if (optind == argc)
	      {
		if (opterr)
		  {
#if 0
		    fprintf (stderr, "%s: option `-%c' requires an argument\n",
			     argv[0], c);
#else
		    /* 1003.2 specifies the format of this message.  */
		    fprintf (stderr, "%s: option requires an argument -- %c\n",
			     argv[0], c);
#endif
		  }
		optopt = c;
		if (optstring[0] == ':')
		  c = ':';
		else
		  c = '?';
	      }
	    else
	      /* We already incremented `optind' once;
		 increment it again when taking next ARGV-elt as argument.  */
	      optarg = argv[optind++];
	    nextchar = NULL;
	  }
      }
    return c;
  }
}

int
hgetopt (argc, argv, optstring)
     int argc;
     char *const *argv;
     const char *optstring;
{
  return _getopt_internal (argc, argv, optstring,
			   (const struct option *) 0,
			   (int *) 0,
			   0);
}
