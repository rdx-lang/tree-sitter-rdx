; Inject YAML into frontmatter
((frontmatter_content) @injection.content
  (#set! injection.language "yaml"))

; Inject JSON into json attributes
((json_content) @injection.content
  (#set! injection.language "json"))

; Inject language into fenced code blocks
((fenced_code_block
  (code_language) @injection.language
  (code_content) @injection.content))

; Inject LaTeX into math blocks
((math_content) @injection.content
  (#set! injection.language "latex"))

; Inject LaTeX into inline math
((math_expression) @injection.content
  (#set! injection.language "latex"))
