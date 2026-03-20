; RDX Tree-sitter Highlights
; Spec: https://github.com/rdx-lang/rdx/blob/main/SPECIFICATION.md

; Frontmatter
(frontmatter_delimiter) @punctuation.special
(frontmatter_content) @embedded

; Tags
(tag_name) @tag
(open_tag "<" @punctuation.bracket)
(open_tag ">" @punctuation.bracket)
(close_tag "</" @punctuation.bracket)
(close_tag ">" @punctuation.bracket)
(self_closing_component "<" @punctuation.bracket)
(self_closing_component "/>" @punctuation.bracket)

; Attributes
(attribute_name) @property

; String values
(string_value) @string
(double_quoted_content) @string
(single_quoted_content) @string

; Primitive values
(number_literal) @number
(boolean_literal) @constant.builtin
(null_literal) @constant.builtin

; JSON attributes
(json_content) @embedded

; Citations (Section 2.7)
(citation "[" @punctuation.special)
(citation "]" @punctuation.special)
(citation_content) @markup.link

; Cross-references (Section 2.8)
(cross_reference "{@" @punctuation.special)
(cross_reference "}" @punctuation.special)
(cross_ref_target) @markup.link

; Display math labels (Section 2.10)
(math_label "{#" @punctuation.special)
(math_label "}" @punctuation.special)
(math_label_id) @label

; Variables
(variable_path) @variable
(variable_interpolation "{" @punctuation.special)
(variable_interpolation "}" @punctuation.special)

; Code
(fenced_code_block) @markup.raw.block
(code_language) @label
(inline_code) @markup.raw.inline

; Escaped sequences
(escaped_sequence) @string.escape

; Text
(text) @markup
(component_text) @markup
