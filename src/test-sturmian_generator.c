/* vim: set expandtab cindent fdm=marker ts=2 sw=2: */

/*
gcc -g -O3 -Wall -Wextra -Wpedantic -fsanitize=undefined -I./ -o test-sturmian_generator test-sturmian_generator.c libaprng-sturmian_generator.c libaprng-sturmian_word.c libaprng-util.c -lrt
gcc -DNDEBUG -O3 -Wall -Wextra -Wpedantic -I./ -o test-sturmian_generator test-sturmian_generator.c libaprng-sturmian_generator.c libaprng-sturmian_word.c libaprng-util.c -lrt
time ./test-sturmian_generator --gen=trib -s -o trib.txt -n10000
*/

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <inttypes.h>
#include <argp.h>

#include "libaprng-sturmian_generator.h"

double timespecDiff(struct timespec *timeA_p, struct timespec *timeB_p)
{
  double n,s;
  n = (double) timeA_p->tv_nsec - (double) timeB_p->tv_nsec;
  s = (double) timeA_p->tv_sec - (double) timeB_p->tv_sec;
  return ( s + n*1.0e-9);
}

const char *argp_program_version = "test-sturmian_generator 1.0";
const char *argp_program_bug_address = "<hladky.jiri@gmail.com>";
static char doc[] = "Test sturmian words generation";
/* The options we understand. */
static struct argp_option options[] = {
  {"gen",            'g', "NAME",   0,  "Generator to be tested: <fib,trib,AR>", 0 },
  {"statistics",     's'  , 0,      0,  "Print histogram of generated values", 0 },
  {"write_values",   'w', "SEP",    0,  "Output values. Use SEP to separate values. In Bash, use $'\\n' notation to enter newline. $'\\t' for tab.", 0 },
  {"output_file",    'o', "FILE",   0,  "Print output values to FILE using comma as separator. Use --write_values SEP to change separator", 0 },
  {"letters",        'n', "NUMBER", 0,  "How many values. Default 1.0e10", 0 },
  {"AR_rule",        'r', "RULE",   0,  "Sigma rules which defines Arnoux-Rauzy word. Numbers from 0 to 2, each number has to be used at least once. Example: 012", 0 },
  { 0 }
};
/* Used by main to communicate with parse_opt. */
struct arguments
{
  char *generator;
  int histogram;
  char *separator;
  int output;             //Write generator output?
  int file;               //Write to file?
  char *output_filename;
  size_t n;
  char *AR_rule;
  int rule;               //Was -r specified?
};
/* Parse a single option. */
static error_t parse_opt (int key, char *arg, struct argp_state *state) {
  /* Get the input argument from argp_parse, which we
     know is a pointer to our arguments structure. */
  struct arguments *arguments = state->input;
  char* end;
  double x;

  switch (key)
    {
    case 'g':
      arguments->generator = arg;
      break;
    case 's':
      arguments->histogram = 1;
      break;
    case 'w':
      arguments->output = 1;
      arguments->separator = arg;
      break;
    case 'n':
      x = strtod(arg, &end);
      arguments->n = x;
      break;
    case 'r':
      arguments->AR_rule = arg;
      arguments->rule = 1;
      break;
    case 'o':
      arguments->output = 1;
      arguments->file = 1;
      arguments->output_filename = arg;
      break;
    case ARGP_KEY_ARG:
      if (state->arg_num > 0)
        /* Too many arguments. */
        argp_usage (state);
      break;
    case ARGP_KEY_INIT:
      /* Do all initialization here */
      arguments->generator = "fib";
      arguments->histogram = 0;
      arguments->output = 0;      //No output. Time generator only.
      arguments->file = 0;        //Store generator output to file? 0=>stdout
      arguments->separator = ",";
      arguments->n = 1.0E9;
      arguments->AR_rule = "";
      arguments->rule = 0;     //Was -r specified?
      break;
    case ARGP_KEY_END:
      /* Do final argument validation here */
      if ( (strcmp(arguments->generator, "AR") == 0) && ( arguments->rule == 0 ) ) {
        argp_error (state, "For Arnoux-Rauzy generator, the sigma rules vector is required. See option --AR_rule=RULE\n");
      }
      if ( (strcmp(arguments->generator, "AR") != 0) && ( arguments->rule == 1 ) ) {
        fprintf(stderr, "Warning: --AR_rule=RULE will be ignored. It's relevant only for Arnoux-Rauzy(AR) generator.\n");
      }
      break;

    default:
      return ARGP_ERR_UNKNOWN;
    }

  return 0;
}
/* Our argp parser. */
static struct argp argp = { options, parse_opt, 0, doc, 0, 0, 0 };
int main(int argc, char **argv)
{
  struct arguments arguments;
  FILE* foutput = stdout;

  argp_parse (&argp, argc, argv, 0, 0, &arguments);

  if ( arguments.output ) {
    if ( arguments.file ) {
      foutput = SAFEFOPEN(arguments.output_filename, "w");
      fprintf(stderr, "Generator output will be stored in file '%s'\n", arguments.output_filename);
    } else {
      fprintf(stderr, "Generator output will be printed on stdout\n");
      foutput = stdout;
    }
  } else {
    fprintf(stderr, "Generator will be only tested for speed; generated values will be not printed.\n");
  }

  sturm_gen_t* A;

  uint64_t j;
  uint64_t k, histo[3];
  const unsigned int buf_size = arguments.n <= 10000 ? arguments.n : 10000;
  uint8_t l[buf_size];
  struct timespec t[4];
  double timeElapsed[2];

  if (strcmp(arguments.generator, "fib") == 0) {
    A = create_Fibonacci();
  } else if (strcmp(arguments.generator, "trib") == 0) {
    A = create_Tribonacci();
  } else if (strcmp(arguments.generator, "AR") == 0) {
    if ( strlen(arguments.AR_rule) < 3 ) {
      fprintf(stderr, "Unsupported AR sigma rules vector. Examples: '012', '21001'. Got '%s'\n", arguments.AR_rule);
      return 1;
    }
    uint8_t* rule = calloc(strlen(arguments.AR_rule), sizeof(uint8_t));
    if (! rule ) {
      fprintf(stderr, "Failed to allocate memory for rule '%s'\n", arguments.AR_rule);
      return 1;
    }
    for (size_t i=0; i < strlen(arguments.AR_rule); i++) {
      rule[i] = arguments.AR_rule[i] - '0';
      if (rule[i] > 2) {
        fprintf(stderr, "Unsupported AR sigma rules vector. Examples: '012', '21001'. Got '%s'. Error parsing '%c'\n", arguments.AR_rule, arguments.AR_rule[i]);
        return 1;
      }
    }
    A = create_AR(rule, strlen(arguments.AR_rule));
    free(rule);
  } else {
    fprintf(stderr, "Unsupported generator. Supported generators are 'fib', 'trib' and 'AR'. Got '%s'\n", arguments.generator);
    return 1;
  }

  if (! A) {
    fprintf(stderr, "Failed to create generator!\n");
    return 1;
  }

  uint64_t steps = ( arguments.n + buf_size - 1) / buf_size; //ceiling divison
  uint64_t total = steps * buf_size;
  uint64_t remains_to_write = arguments.n;

  fprintf(stdout, "Testing %s generator. Generating %Lg letters\n", A->name, (long double) total);

  if ( arguments.histogram ) {
    histo[0]=0; histo[1]=0; histo[2]=0;
  }
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t[0]);
    clock_gettime(CLOCK_MONOTONIC, &t[2]);

    for (j=0; j<steps; ++j) {
      sturm_gen_get_word (A, buf_size, l);
      if ( arguments.histogram ) {
        for (k=0; k<buf_size; ++k) {
          ++histo[l[k]];
        }
      }
      if ( arguments.output ) {
        uint64_t loop_limit = remains_to_write >= buf_size ? buf_size : remains_to_write;
        for (k=0; k<loop_limit; ++k) {
          fprintf(foutput,"%d%s",l[k],arguments.separator);
        }
      }
    }

    if ( arguments.file ) {
      fclose(foutput);
    }

    fprintf(stdout,"\n");

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t[1]);
    clock_gettime(CLOCK_MONOTONIC, &t[3]);
    timeElapsed[0] = timespecDiff(&t[1], &t[0]);
    timeElapsed[1] = timespecDiff(&t[3], &t[2]);
    sturm_gen_print_status(A, stdout);
    fprintf(stdout, "Generated %Lg elements\n", (long double) A->generated);
    fprintf(stdout, "WALLCLOCK TIME:\t%g\n"
        "CPU TIME:\t%g\n"
        "Last value %d\n",
        timeElapsed[1], timeElapsed[0], l[buf_size-1]);
    fprintf(stdout, "\n\n");
    if ( arguments.histogram ) {
      fprintf(stdout, "Histogram of generated values:\n");
      for (k=0; k<sizeof(histo)/sizeof(*histo); ++k) {
        fprintf(stdout, "%" PRIu64 " %Lg%%\n", k, (long double) histo[k] / (long double) A->generated * 100.0L);
      }
    }
  sturm_gen_delete(A);
  return 0;
}
