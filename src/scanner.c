#include "tree_sitter/parser.h"

#include <string.h>

enum TokenType {
  CODE_FENCE_OPEN,
  CODE_CONTENT,
  CODE_FENCE_CLOSE,
  MATH_FENCE_OPEN,
  MATH_CONTENT,
  MATH_FENCE_CLOSE,
  FRONTMATTER_OPEN,
  FRONTMATTER_CONTENT,
  FRONTMATTER_CLOSE,
};

typedef struct {
  uint8_t fence_length;  // backtick count for code fences
  bool in_math;          // true if inside a $$ math block
  bool in_frontmatter;   // true if inside frontmatter block
} Scanner;

void *tree_sitter_rdx_external_scanner_create(void) {
  Scanner *scanner = calloc(1, sizeof(Scanner));
  return scanner;
}

void tree_sitter_rdx_external_scanner_destroy(void *payload) {
  free(payload);
}

unsigned tree_sitter_rdx_external_scanner_serialize(void *payload,
                                                     char *buffer) {
  Scanner *scanner = (Scanner *)payload;
  buffer[0] = (char)scanner->fence_length;
  buffer[1] = (char)scanner->in_math;
  buffer[2] = (char)scanner->in_frontmatter;
  return 3;
}

void tree_sitter_rdx_external_scanner_deserialize(void *payload,
                                                    const char *buffer,
                                                    unsigned length) {
  Scanner *scanner = (Scanner *)payload;
  if (length >= 3) {
    scanner->fence_length = (uint8_t)buffer[0];
    scanner->in_math = (bool)buffer[1];
    scanner->in_frontmatter = (bool)buffer[2];
  } else {
    scanner->fence_length = 0;
    scanner->in_math = false;
    scanner->in_frontmatter = false;
  }
}

static bool is_newline_or_eof(TSLexer *lexer) {
  return lexer->lookahead == '\n' || lexer->lookahead == '\r' ||
         lexer->eof(lexer);
}

static void skip_trailing_whitespace(TSLexer *lexer) {
  while (lexer->lookahead == ' ' || lexer->lookahead == '\t') {
    lexer->advance(lexer, false);
  }
}

// Cap at 255 to prevent uint8_t overflow on pathological input
static uint8_t consume_backticks(TSLexer *lexer) {
  uint8_t count = 0;
  while (lexer->lookahead == '`' && count < 255) {
    count++;
    lexer->advance(lexer, false);
  }
  // Skip any remaining backticks beyond the cap
  while (lexer->lookahead == '`') {
    lexer->advance(lexer, false);
  }
  return count;
}

static bool scan_math_fence(TSLexer *lexer) {
  if (lexer->lookahead != '$') return false;
  lexer->advance(lexer, false);
  if (lexer->lookahead != '$') return false;
  lexer->advance(lexer, false);
  // Ensure rest of line is whitespace
  while (lexer->lookahead == ' ' || lexer->lookahead == '\t') {
    lexer->advance(lexer, false);
  }
  return is_newline_or_eof(lexer);
}

bool tree_sitter_rdx_external_scanner_scan(void *payload, TSLexer *lexer,
                                            const bool *valid_symbols) {
  Scanner *scanner = (Scanner *)payload;

  // === FRONTMATTER ===
  // Must start at row 0, col 0. Opening --- must be the very first thing.
  if (valid_symbols[FRONTMATTER_OPEN] && !scanner->in_frontmatter &&
      scanner->fence_length == 0 && !scanner->in_math) {
    // Check we're at column 0 by checking get_column
    if (lexer->get_column(lexer) == 0 && lexer->lookahead == '-') {
      lexer->advance(lexer, false);
      if (lexer->lookahead == '-') {
        lexer->advance(lexer, false);
        if (lexer->lookahead == '-') {
          lexer->advance(lexer, false);
          // Must be followed by newline
          if (lexer->lookahead == '\n' || lexer->lookahead == '\r') {
            if (lexer->lookahead == '\r') lexer->advance(lexer, false);
            if (lexer->lookahead == '\n') lexer->advance(lexer, false);
            scanner->in_frontmatter = true;
            lexer->mark_end(lexer);
            lexer->result_symbol = FRONTMATTER_OPEN;
            return true;
          }
        }
      }
    }
  }

  // === FRONTMATTER_CONTENT ===
  if (scanner->in_frontmatter && valid_symbols[FRONTMATTER_CONTENT]) {
    bool has_content = false;

    while (!lexer->eof(lexer)) {
      lexer->mark_end(lexer);

      // Skip leading whitespace before checking for closing ---
      while (lexer->lookahead == ' ' || lexer->lookahead == '\t') {
        lexer->advance(lexer, false);
      }

      // Check if this line is the closing ---
      if (lexer->lookahead == '-') {
        lexer->advance(lexer, false);
        if (lexer->lookahead == '-') {
          lexer->advance(lexer, false);
          if (lexer->lookahead == '-') {
            lexer->advance(lexer, false);
            // Must be followed by whitespace-only then newline or EOF
            skip_trailing_whitespace(lexer);
            if (is_newline_or_eof(lexer)) {
              // Found closing delimiter — return content before it
              if (has_content) {
                lexer->result_symbol = FRONTMATTER_CONTENT;
                return true;
              }
              return false;
            }
          }
        }
        // Not a closing delimiter, fall through to consume line
      }

      // Consume entire line
      has_content = true;
      while (!lexer->eof(lexer) && lexer->lookahead != '\n') {
        lexer->advance(lexer, false);
      }
      if (lexer->lookahead == '\n') {
        lexer->advance(lexer, false);
      }
    }

    if (has_content) {
      lexer->mark_end(lexer);
      lexer->result_symbol = FRONTMATTER_CONTENT;
      return true;
    }
    return false;
  }

  // === FRONTMATTER_CLOSE ===
  if (scanner->in_frontmatter && valid_symbols[FRONTMATTER_CLOSE]) {
    // Allow leading whitespace before ---
    while (lexer->lookahead == ' ' || lexer->lookahead == '\t') {
      lexer->advance(lexer, false);
    }

    if (lexer->lookahead != '-') return false;
    lexer->advance(lexer, false);
    if (lexer->lookahead != '-') return false;
    lexer->advance(lexer, false);
    if (lexer->lookahead != '-') return false;
    lexer->advance(lexer, false);

    skip_trailing_whitespace(lexer);
    if (!is_newline_or_eof(lexer)) return false;

    if (lexer->lookahead == '\r') lexer->advance(lexer, false);
    if (lexer->lookahead == '\n') lexer->advance(lexer, false);

    scanner->in_frontmatter = false;
    lexer->mark_end(lexer);
    lexer->result_symbol = FRONTMATTER_CLOSE;
    return true;
  }

  // === MATH_FENCE_OPEN ===
  // Display math $$ must start at the beginning of a line (column 0).
  if (valid_symbols[MATH_FENCE_OPEN] && !scanner->in_math &&
      scanner->fence_length == 0) {
    if (lexer->lookahead != '$') goto try_code;
    if (lexer->get_column(lexer) != 0) goto try_code;

    lexer->mark_end(lexer);

    if (scan_math_fence(lexer)) {
      scanner->in_math = true;
      lexer->mark_end(lexer);
      lexer->result_symbol = MATH_FENCE_OPEN;
      return true;
    }
    // Not a valid math fence, fall through
  }

try_code:
  // === CODE_FENCE_OPEN ===
  // Per CommonMark, code fences may have 0-3 leading spaces.
  if (valid_symbols[CODE_FENCE_OPEN] && scanner->fence_length == 0 &&
      !scanner->in_math) {
    // Must be at start of line (column 0, possibly after extras-consumed whitespace)
    uint32_t col = lexer->get_column(lexer);
    if (col > 3) goto try_math_content;
    if (lexer->lookahead != '`') goto try_math_content;

    uint8_t backticks = consume_backticks(lexer);
    if (backticks >= 3) {
      scanner->fence_length = backticks;
      lexer->mark_end(lexer);
      lexer->result_symbol = CODE_FENCE_OPEN;
      return true;
    }
    return false;
  }

try_math_content:
  // === Inside a math block ===
  if (scanner->in_math) {
    // === MATH_CONTENT ===
    if (valid_symbols[MATH_CONTENT]) {
      bool has_content = false;

      while (!lexer->eof(lexer)) {
        lexer->mark_end(lexer);

        // Skip leading whitespace on this line
        while (lexer->lookahead == ' ' || lexer->lookahead == '\t') {
          lexer->advance(lexer, false);
        }

        // Check for closing $$
        if (lexer->lookahead == '$') {
          // Peek: is this $$?
          lexer->advance(lexer, false);
          if (lexer->lookahead == '$') {
            lexer->advance(lexer, false);
            skip_trailing_whitespace(lexer);
            if (is_newline_or_eof(lexer)) {
              // This is a closing fence
              if (has_content) {
                lexer->result_symbol = MATH_CONTENT;
                return true;
              }
              return false;
            }
          }
          // Not a closing fence, fall through to consume line
        }

        // Consume entire line
        has_content = true;
        while (!lexer->eof(lexer) && lexer->lookahead != '\n') {
          lexer->advance(lexer, false);
        }
        if (lexer->lookahead == '\n') {
          lexer->advance(lexer, false);
        }
      }

      if (has_content) {
        lexer->mark_end(lexer);
        lexer->result_symbol = MATH_CONTENT;
        return true;
      }
      return false;
    }

    // === MATH_FENCE_CLOSE ===
    if (valid_symbols[MATH_FENCE_CLOSE]) {
      while (lexer->lookahead == ' ' || lexer->lookahead == '\t') {
        lexer->advance(lexer, false);
      }

      if (lexer->lookahead != '$') return false;
      lexer->advance(lexer, false);
      if (lexer->lookahead != '$') return false;
      lexer->advance(lexer, false);

      skip_trailing_whitespace(lexer);
      if (!is_newline_or_eof(lexer)) return false;

      if (lexer->lookahead == '\r') lexer->advance(lexer, false);
      if (lexer->lookahead == '\n') lexer->advance(lexer, false);

      scanner->in_math = false;
      lexer->mark_end(lexer);
      lexer->result_symbol = MATH_FENCE_CLOSE;
      return true;
    }

    return false;
  }

  // === Inside a code block (fence is open) ===
  if (scanner->fence_length == 0) return false;

  // === CODE_CONTENT ===
  // Consume all lines until we find a line that is a valid closing fence.
  if (valid_symbols[CODE_CONTENT]) {
    bool has_content = false;

    while (!lexer->eof(lexer)) {
      // Mark before each line so we can stop here if it's a closing fence
      lexer->mark_end(lexer);

      // Check if this line starts with enough backticks to close
      uint8_t leading_spaces = 0;
      while (lexer->lookahead == ' ' && leading_spaces < 3) {
        lexer->advance(lexer, false);
        leading_spaces++;
      }

      if (lexer->lookahead == '`') {
        uint8_t backticks = consume_backticks(lexer);
        if (backticks >= scanner->fence_length) {
          skip_trailing_whitespace(lexer);
          if (is_newline_or_eof(lexer)) {
            // This line is a closing fence — return content before it
            if (has_content) {
              // mark_end was set before this line
              lexer->result_symbol = CODE_CONTENT;
              return true;
            }
            // No content before closing fence (empty code block)
            return false;
          }
        }
        // Not a valid close — fall through to consume as content
      }

      // Consume the entire line (including chars we already peeked)
      has_content = true;
      while (!lexer->eof(lexer) && lexer->lookahead != '\n') {
        lexer->advance(lexer, false);
      }
      if (lexer->lookahead == '\n') {
        lexer->advance(lexer, false);
      }
    }

    // EOF inside code block
    if (has_content) {
      lexer->mark_end(lexer);
      lexer->result_symbol = CODE_CONTENT;
      return true;
    }
    return false;
  }

  // === CODE_FENCE_CLOSE ===
  if (valid_symbols[CODE_FENCE_CLOSE]) {
    uint8_t leading_spaces = 0;
    while (lexer->lookahead == ' ' && leading_spaces < 3) {
      lexer->advance(lexer, false);
      leading_spaces++;
    }

    if (lexer->lookahead != '`') return false;

    uint8_t backticks = consume_backticks(lexer);
    if (backticks < scanner->fence_length) return false;

    skip_trailing_whitespace(lexer);
    if (!is_newline_or_eof(lexer)) return false;

    // Consume trailing newline
    if (lexer->lookahead == '\r') {
      lexer->advance(lexer, false);
    }
    if (lexer->lookahead == '\n') {
      lexer->advance(lexer, false);
    }

    scanner->fence_length = 0;
    lexer->mark_end(lexer);
    lexer->result_symbol = CODE_FENCE_CLOSE;
    return true;
  }

  return false;
}
