#include <sys/types.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

int s_array_len(char** in){
	int count = 0;
	while (in[count] != NULL) count++;

	return count;
}

int sum_array(int A[], int size){
	int sum = 0;
	int i = 0;
	while (i < size)
		sum += A[i++];

	return sum;
}

char** tokenize(char* in, const char* delim){
	char* p = strtok(in, delim);
	int n_spaces = 0;
	char** res = NULL;

	while (p) {
  		res = realloc(res, sizeof(char*) * ++n_spaces);

  		if (res == NULL)
    		exit(EXIT_FAILURE);

  		res[n_spaces - 1] = p;

 		p = strtok(NULL, delim);
	}

	res = realloc(res, sizeof(char*) * (n_spaces + 1));
	res[n_spaces] = 0;

	return res;
}

char** read_input(const char* input_file){

	FILE *fp;
	ssize_t read;
	size_t len, nums = 0;
	char** res = NULL;
	char* line = NULL;

	if ((fp = fopen(input_file, "r")) == NULL){
		printf("Incorrect file \n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	
	while ((read = getline(&line, &len, fp)) != -1){
		if ((res = realloc(res, sizeof(char*) * ++nums)) == NULL)
			exit(EXIT_FAILURE);

		if (line[read - 1] == "\n")
			line[read - 1] = 0;
		else
			line[read] = 0;

  		res[nums - 1] = strdup(line);
	}
	free(line);

	if ((res = realloc(res, sizeof(char*) * (nums + 1))) == NULL)
		exit(EXIT_FAILURE);
	res[nums] = 0;

	fclose(fp);

	return res;
}

int getstr (char ** lineptr, size_t *n, FILE * stream, char terminator, int offset)
{
  int nchars_avail;             /* Allocated but unused chars in *LINEPTR.  */
  char *read_pos;               /* Where we're reading into *LINEPTR. */
  int ret;

  if (!lineptr || !n || !stream)
    return -1;

  if (!*lineptr)
    {
      *n = 64;
      *lineptr = (char *) malloc (*n);
      if (!*lineptr)
        return -1;
    }

  nchars_avail = *n - offset;
  read_pos = *lineptr + offset;

  for (;;)
    {
      register int c = getc (stream);

      /* We always want at least one char left in the buffer, since we
         always (unless we get an error while reading the first char)
         NUL-terminate the line buffer.  */

      assert(*n - nchars_avail == read_pos - *lineptr);
      if (nchars_avail < 1)
        {
          if (*n > 64)
            *n *= 2;
          else
            *n += 64;

          nchars_avail = *n + *lineptr - read_pos;
          *lineptr = (char *) realloc (*lineptr, *n);
          if (!*lineptr)
            return -1;
          read_pos = *n - nchars_avail + *lineptr;
          assert(*n - nchars_avail == read_pos - *lineptr);
        }

      if (c == EOF || ferror (stream))
        {
          /* Return partial line, if any.  */
          if (read_pos == *lineptr)
            return -1;
          else
            break;
        }

      *read_pos++ = c;
      nchars_avail--;

      if (c == terminator)
        /* Return the line.  */
        break;
    }

  /* Done - NUL terminate and return the number of chars read.  */
  *read_pos = '\0';

  ret = read_pos - (*lineptr + offset);
  return ret;
}

ssize_t getline(char **lineptr, size_t *n, FILE *stream)
{
  return getstr (lineptr, n, stream, '\n', 0);
}