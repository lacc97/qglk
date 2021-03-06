#ifndef	DJGCC_386

#include "os.h"

#ifndef	HAVE_TPARM

#include <stdlib.h>
#include <stdio.h>

static char *tparam1();

char *
Tparm(string, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
	char	*string;
	int	arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8;
{
	int		argp[9];
	static char	result[256];

	argp[0] = arg0;
	argp[1] = arg1;
	argp[2] = arg2;
	argp[3] = arg3;
	argp[4] = arg4;
	argp[5] = arg5;
	argp[6] = arg6;
	argp[7] = arg7;
	argp[8] = arg8;

	tparam1(string, result, 256, 0, 0, argp);

	return result;
}

static char *
tparam1 (string, outstring, len, up, left, argp)
     char *string;
     char *outstring;
     int len;
     char *up, *left;
     register int *argp;
{
  register int c;
  register char *p = string;
  register char *op = outstring;
  char *outend;
  int outlen = 0;
  int i;

  register int tem;
  int *old_argp = argp;
  int doleft = 0;
  int doup = 0;

  outend = outstring + len;

  while (1)
    {
      /* If the buffer might be too short, make it bigger.  */
      if (op + 5 >= outend)
	{
	  register char *New;
	  if (outlen == 0)
	    {
	      outlen = len + 40;
	      New = (char *) malloc (outlen);
	      outend += 40;
	      bcopy (outstring, New, op - outstring);
	    }
	  else
	    {
	      outend += outlen;
	      outlen *= 2;
	      New = (char *) realloc (outstring, outlen);
	    }
	  op += New - outstring;
	  outend += New - outstring;
	  outstring = New;
	}
      c = *p++;
      if (!c)
	break;
      if (c == '%')
	{
	  c = *p++;
	  tem = *argp;
	  switch (c)
	    {
	    case 'd':		/* %d means output in decimal.  */
	      if (tem < 10)
		goto onedigit;
	      if (tem < 100)
		goto twodigit;
	    case '3':		/* %3 means output in decimal, 3 digits.  */
	      if (tem > 999)
		{
		  *op++ = tem / 1000 + '0';
		  tem %= 1000;
		}
	      *op++ = tem / 100 + '0';
	    case '2':		/* %2 means output in decimal, 2 digits.  */
	    twodigit:
	      tem %= 100;
	      *op++ = tem / 10 + '0';
	    onedigit:
	      *op++ = tem % 10 + '0';
	      argp++;
	      break;

	    case 'C':
	      /* For c-100: print quotient of value by 96, if nonzero,
		 then do like %+.  */
	      if (tem >= 96)
		{
		  *op++ = tem / 96;
		  tem %= 96;
		}
	    case '+':		/* %+x means add character code of char x.  */
	      tem += *p++;
	    case '.':		/* %. means output as character.  */
	      if (left)
		{
		  /* If want to forbid output of 0 and \n and \t,
		     and this is one of them, increment it.  */
		  while (tem == 0 || tem == '\n' || tem == '\t')
		    {
		      tem++;
		      if (argp == old_argp)
			doup++, outend -= strlen (up);
		      else
			doleft++, outend -= strlen (left);
		    }
		}
	      *op++ = tem | 0200;
	    case 'f':		/* %f means discard next arg.  */
	      argp++;
	      break;

	    case 'b':		/* %b means back up one arg (and re-use it).  */
	      argp--;
	      break;

	    case 'r':		/* %r means interchange following two args.  */
	      argp[0] = argp[1];
	      argp[1] = tem;
	      old_argp++;
	      break;

	    case '>':		/* %>xy means if arg is > char code of x, */
	      if (argp[0] > *p++) /* then add char code of y to the arg, */
		argp[0] += *p;	/* and in any case don't output.  */
	      p++;		/* Leave the arg to be output later.  */
	      break;

	    case 'a':		/* %a means arithmetic.  */
	      /* Next character says what operation.
		 Add or subtract either a constant or some other arg.  */
	      /* First following character is + to add or - to subtract
		 or = to assign.  */
	      /* Next following char is 'p' and an arg spec
		 (0100 plus position of that arg relative to this one)
		 or 'c' and a constant stored in a character.  */
	      tem = p[2] & 0177;
	      if (p[1] == 'p')
		tem = argp[tem - 0100];
	      if (p[0] == '-')
		argp[0] -= tem;
	      else if (p[0] == '+')
		argp[0] += tem;
	      else if (p[0] == '*')
		argp[0] *= tem;
	      else if (p[0] == '/')
		argp[0] /= tem;
	      else
		argp[0] = tem;

	      p += 3;
	      break;

	    case 'i':		/* %i means add one to arg, */
	      argp[0] ++;	/* and leave it to be output later.  */
	      argp[1] ++;	/* Increment the following arg, too!  */
	      break;

	    case '%':		/* %% means output %; no arg.  */
	      goto ordinary;

	    case 'n':		/* %n means xor each of next two args with 140.  */
	      argp[0] ^= 0140;
	      argp[1] ^= 0140;
	      break;

	    case 'm':		/* %m means xor each of next two args with 177.  */
	      argp[0] ^= 0177;
	      argp[1] ^= 0177;
	      break;

	    case 'B':		/* %B means express arg as BCD char code.  */
	      argp[0] += 6 * (tem / 10);
	      break;

	    case 'D':		/* %D means weird Delta Data transformation.  */
	      argp[0] -= 2 * (tem % 16);
	      break;
	    }
	}
      else
	/* Ordinary character in the argument string.  */
      ordinary:
	*op++ = c;
    }
  *op = 0;
  while (doup-- > 0)
    strcat (op, up);
  while (doleft-- > 0)
    strcat (op, left);
  return outstring;
}

#endif	/* !HAVE_TPARM */

#endif	/* DJGCC_386 */
