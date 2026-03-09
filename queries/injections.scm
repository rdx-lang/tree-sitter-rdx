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
