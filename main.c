#include <ctype.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>

#define STR_IMPLEMENTATION
#include "str.h"

// Famous readline() function only works in GNU/Linux
// If this doesn't defined it'll use fgets() instead.
#ifdef USE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

// You can change these
#define REPL_INPUT  ">>> "
#define REPL_OUTPUT "<<< "

#ifdef __GNUC__ // GCC, Clang, ICC
#define UNREACHABLE_INTR() (__builtin_unreachable())
#elif defined(_MSC_VER) // MSVC
#define UNREACHABLE_INTR() (__assume(false))
#endif

#define UNREACHABLE(fmt, ...) \
  do { \
    fprintf(stderr, "%s:%d: UNREACHABLE: "fmt"\n", __FILE__, __LINE__, ##__VA_ARGS__); \
    UNREACHABLE_INTR(); \
  } while (0)

typedef struct {
  StringView expr; // whole expression
  StringView cur;  // cursor
  size_t nerr;     // error count 0 when no error
} Calculator;

double parse_expr(Calculator *self);
double parse_term(Calculator *self);
double parse_factor(Calculator *self);
double parse_number(Calculator *self);

bool expect(StringView *sv, char c)
{
  char current = sv_at(sv, 0);
  if (current == c) {
    sv_chop_left(sv, 1);
    return true;
  }
  return false;
}

void cal_report_error(const Calculator *self, size_t offset, size_t length, const char *fmt, ...)
{
  int cursor = self->expr.count - self->cur.count + 1;
  cursor += offset;
  if (cursor == 1) cursor = 0;
  const char *err_header = "\e[0;31mERROR\e[0m: ";

  fputs(err_header, stderr);
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  fputc('\n', stderr);

  fprintf(stderr, " |     "SV_FMT"\n", SV_ARG(self->expr));
  fprintf(stderr, " |     \e[0;36m%*c", cursor, '^');

  if (length <= 1) fprintf(stderr, "\e[0m");
  for (size_t i = 1; i < length; i++) {
    fputc('~', stderr);
  }
  fprintf(stderr, "\e[0m");

  fputc('\n', stderr);
}

double parse_expr(Calculator *self)
{
  double lhs = parse_term(self);
  sv_trim_left(&self->cur);
  char op = sv_at(&self->cur, 0);
  while (op == '+' || op == '-') {
    sv_chop_left(&self->cur, 1);
    double rhs = parse_term(self);
    sv_trim_left(&self->cur);
    if (op == '+') lhs += rhs;
    else lhs -= rhs;
    op = sv_at(&self->cur, 0);
  }
  return lhs;
}

double parse_term(Calculator *self)
{
  double lhs = parse_factor(self);
  sv_trim_left(&self->cur);
  char op = sv_at(&self->cur, 0);
  while (op == '*' || op == '/' || op == '%' || op == '^') {
    sv_chop_left(&self->cur, 1);
    double rhs = parse_factor(self);
    switch (op) {
    case '*': lhs *= rhs; break;
    case '/': lhs /= rhs; break;
    case '%': lhs = fmod(lhs, rhs); break;
    case '^': lhs = pow(lhs, rhs); break;
    default: UNREACHABLE("parse_term");
    }
    op = sv_at(&self->cur, 0);
  }
  return lhs;
}

double parse_factor(Calculator *self)
{
  sv_trim_left(&self->cur);
  if (expect(&self->cur, '(')) {
    double result = parse_expr(self);
    sv_trim_left(&self->cur);
    if (!expect(&self->cur, ')')) {
      cal_report_error(self, 0, 1, "expected ')'");
      return (self->nerr++, 0.0);
    }
    return result;
  }

  unsigned char c = sv_at(&self->cur, 0);
  if ((c >= '0' && c <= '9') ||
      c == '.' || c == '-' ||
      isspace(c)) return parse_number(self);

  cal_report_error(self, 0, 1, "unexpected character");
  return (self->nerr++, 0.0);
}

double parse_number(Calculator *self)
{
  sv_trim_left(&self->cur);
  double r = 0.0;
  double sign = 1.0;
  bool is_integer = true; // integer / decimal part

  size_t i = 0;
  size_t dot = 0;
  size_t digits = 0;

  for (; i < self->cur.count; i++) {
    char c = sv_at(&self->cur, i);
    if (c == '-' && i == 0) {
      sign = -1.0;
      continue;
    }

    if (c == '.') {
      if (!is_integer) {
        cal_report_error(self, i, 1, "unexpected dot ('.') here");
        return (self->nerr++, 0.0);
      }
      is_integer = false;
      dot = i;
      continue;
    }

    if (c >= '0' && c <= '9') {
      digits++;
      if (is_integer) {
        r = r * 10.0 + (c - '0');
      } else {
        r += (double)(c - '0') / pow(10.0, i - dot);
      }
    } else break;
  }

  if (digits == 0 && (i > 0 && sv_at(&self->cur, i - 1) == '.')) {
    cal_report_error(self, i - 1, 1, "expected at least 1 digit after '.'");
    return (self->nerr++, 0.0);
  }

  sv_chop_left(&self->cur, i);
  return sign * r;
}

bool repl_body(char *line)
{
  StringView sv = SV(line);
  sv_trim(&sv);
  putchar('\n');

  if (sv_eq(sv, SVL("quit")) ||
      sv_eq(sv, SVL("q"))    ||
      sv_eq(sv, SVL("exit"))) return false;

  Calculator cal = {.expr=sv, .cur=sv};
  double res = parse_expr(&cal);
  if (cal.nerr > 0) {
    fprintf(stderr, "total errors: \e[0;31m%zu\e[0m\n", cal.nerr);
  } else {
    printf(REPL_OUTPUT"%lf\n", res);
  }
  putchar('\n');
  return true;
}

int main(void)
{
#ifdef USE_READLINE
  char *line;
  while ((line = readline(REPL_INPUT)) != NULL) {
    if (*line) add_history(line);
    if (!repl_body(line)) break;
    free(line);
  }

#else
  bool eof = true;
  printf(REPL_INPUT);
  char buf[1024];
  while (fgets(buf, sizeof(buf), stdin) != NULL) {
    if (!repl_body(buf)) { eof = false; break; }
    printf(REPL_INPUT);
  }
  if (eof) putchar('\n');
#endif
}
