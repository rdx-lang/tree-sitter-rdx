/// <reference types="tree-sitter-cli/dsl" />
// @ts-check

// Tree-sitter grammar for RDX (Reactive Document eXpressions)
// Spec: https://github.com/rdx-lang/rdx/blob/main/SPECIFICATION.md

module.exports = grammar({
  name: "rdx",

  extras: ($) => [/\s/],

  externals: ($) => [
    $._code_fence_open,
    $._code_content,
    $._code_fence_close,
    $._math_fence_open,
    $._math_content,
    $._math_fence_close,
    $._frontmatter_open,
    $._frontmatter_content,
    $._frontmatter_close,
  ],

  rules: {
    // Root document: optional frontmatter followed by body content
    document: ($) =>
      seq(optional($.frontmatter), repeat($._body_content)),

    // -----------------------------------------------------------
    // 2.1 Frontmatter
    // Must start at line 1, col 1. Delimited by `---`.
    // -----------------------------------------------------------
    frontmatter: ($) =>
      seq(
        alias($._frontmatter_open, $.frontmatter_delimiter),
        alias($._frontmatter_content, $.frontmatter_content),
        alias($._frontmatter_close, $.frontmatter_delimiter),
      ),

    // -----------------------------------------------------------
    // Body content: markdown text, components, variables, or
    // code constructs (where variables are NOT interpolated).
    // -----------------------------------------------------------
    _body_content: ($) =>
      choice(
        $.component,
        $.self_closing_component,
        $.citation,
        $.cross_reference,
        $.variable_interpolation,
        $.escaped_sequence,
        $.fenced_code_block,
        $.display_math,
        $.inline_math,
        $.inline_code,
        $.text,
      ),

    // -----------------------------------------------------------
    // 2.5 Escaping
    // -----------------------------------------------------------
    escaped_sequence: ($) =>
      token(
        choice(
          "\\{$",
          "\\{@",
          "\\[@",
          "\\$",
          "\\{{",
          "\\}}",
          "\\{",
          "\\\\",
        ),
      ),

    // -----------------------------------------------------------
    // Code constructs — variables must NOT be interpolated here
    // Handled by external scanner (src/scanner.c) to correctly
    // match opening/closing fence backtick counts.
    // -----------------------------------------------------------
    fenced_code_block: ($) =>
      seq(
        alias($._code_fence_open, $.code_fence_delimiter),
        optional($.code_language),
        /\n/,
        optional(alias($._code_content, $.code_content)),
        alias($._code_fence_close, $.code_fence_delimiter),
      ),

    code_language: ($) => /[^\n`]+/,

    inline_code: ($) => seq("`", /[^`]+/, "`"),

    // -----------------------------------------------------------
    // 2.7 Citations
    // [@key] or [@key, p. 42] or [@a; @b]
    // -----------------------------------------------------------
    citation: ($) =>
      prec(2, seq("[", $.citation_content, "]")),

    citation_content: ($) => /[@][a-zA-Z][a-zA-Z0-9_:.\/;,\s@-]*/,

    // -----------------------------------------------------------
    // 2.8 Cross-References
    // {@target}
    // -----------------------------------------------------------
    cross_reference: ($) =>
      seq("{@", $.cross_ref_target, "}"),

    cross_ref_target: ($) => /[a-zA-Z_][a-zA-Z0-9_:.-]*/,

    // -----------------------------------------------------------
    // 2.6 Math (LaTeX)
    // Display math: $$ on own line, handled by external scanner.
    // Inline math: $...$ (not preceded by {, which is variable syntax)
    // -----------------------------------------------------------
    display_math: ($) =>
      seq(
        alias($._math_fence_open, $.math_delimiter),
        optional($.math_label),
        /\n/,
        optional(alias($._math_content, $.math_content)),
        alias($._math_fence_close, $.math_delimiter),
      ),

    // {#identifier} label on display math opening line
    math_label: ($) => seq("{#", $.math_label_id, "}"),

    math_label_id: ($) => /[a-zA-Z_][a-zA-Z0-9_:.-]*/,

    inline_math: ($) => prec(1, seq("$", $.math_expression, "$")),

    math_expression: ($) => /[^\s$][^$]*/,

    // -----------------------------------------------------------
    // 2.2 Components
    // Tag names: [A-Z][a-zA-Z0-9_]*
    // -----------------------------------------------------------
    component: ($) =>
      seq(
        $.open_tag,
        repeat($._component_child),
        $.close_tag,
      ),

    _component_child: ($) =>
      choice(
        $.component,
        $.self_closing_component,
        $.citation,
        $.cross_reference,
        $.variable_interpolation,
        $.escaped_sequence,
        $.inline_math,
        $.inline_code,
        $.component_text,
      ),

    component_text: ($) => /[^<{`\\]+/,

    open_tag: ($) =>
      seq(
        "<",
        $.tag_name,
        repeat($.attribute),
        ">",
      ),

    close_tag: ($) => seq("</", $.tag_name, ">"),

    self_closing_component: ($) =>
      seq(
        "<",
        $.tag_name,
        repeat($.attribute),
        "/>",
      ),

    tag_name: ($) => /[A-Z][a-zA-Z0-9_]*/,

    // -----------------------------------------------------------
    // 2.3 Strictly-Typed Attributes
    // No whitespace around `=` (zero-tolerance rule).
    // -----------------------------------------------------------
    attribute: ($) =>
      choice(
        $.string_attribute,
        $.primitive_attribute,
        $.json_attribute,
        $.variable_attribute,
        $.boolean_shorthand_attribute,
      ),

    // 2.2.5 Boolean shorthand: <Input disabled />
    boolean_shorthand_attribute: ($) => $.attribute_name,

    // 2.3.2 String: label="Click Me" or theme='dark'
    string_attribute: ($) =>
      seq($.attribute_name, token.immediate("="), $.string_value),

    // 2.3.3 Primitive: activePage={2}, isActive={true}
    primitive_attribute: ($) =>
      seq($.attribute_name, token.immediate("="), $.primitive_value),

    // 2.3.4 JSON Object/Array: config={{ "key": "value" }}
    json_attribute: ($) =>
      seq($.attribute_name, token.immediate("={{"), $.json_content, "}}"),

    // 2.3.5 Variable: label={$frontmatter.buttonText}
    variable_attribute: ($) =>
      seq($.attribute_name, token.immediate("="), $.variable_value),

    attribute_name: ($) => /[a-zA-Z_][a-zA-Z0-9_-]*/,

    string_value: ($) =>
      choice(
        seq('"', optional($.double_quoted_content), '"'),
        seq("'", optional($.single_quoted_content), "'"),
      ),

    double_quoted_content: ($) => /([^"\\]|\\.)+/,
    single_quoted_content: ($) => /([^'\\]|\\.)+/,

    // Note: empty strings (label="") are handled by the `optional()` wrapper
    // in string_value. These patterns match the non-empty content only.

    // Primitives: integers, floats, booleans, null
    primitive_value: ($) =>
      seq("{", $._primitive_literal, "}"),

    _primitive_literal: ($) =>
      choice(
        $.number_literal,
        $.boolean_literal,
        $.null_literal,
      ),

    number_literal: ($) => /-?[0-9]+(\.[0-9]+)?([eE][+-]?[0-9]+)?/,
    boolean_literal: ($) => choice("true", "false"),
    null_literal: ($) => "null",

    // JSON content inside {{ }} — grab everything until }}
    json_content: ($) => repeat1(/[^}]+|\}[^}]/),

    // Variable value in attributes: {$path}
    variable_value: ($) => seq("{", $.variable_path, "}"),

    // -----------------------------------------------------------
    // 2.4 Context Variables
    // -----------------------------------------------------------
    variable_interpolation: ($) =>
      seq("{", $.variable_path, "}"),

    // 2.4.1 Variable path: $[a-zA-Z_][a-zA-Z0-9_]*(\.[a-zA-Z_][a-zA-Z0-9_]*)*
    variable_path: ($) => /\$[a-zA-Z_][a-zA-Z0-9_]*(\.[a-zA-Z_][a-zA-Z0-9_]*)*/,

    // -----------------------------------------------------------
    // Fallback text
    // -----------------------------------------------------------
    // Match non-special chars, or `<` not followed by [A-Z] or `/`
    // (so autolinks like <https://...> and HTML tags fall through to text)
    // Match non-special chars. Excludes: < { ` \ $ [ (for citations)
    text: ($) => /[^<{`\\$\[]+|<[^A-Z/\s]|[{\\]|\[[^@]/,
  },
});
