# tree-sitter-rdx

Tree-sitter grammar for [RDX (Reactive Document eXpressions)](https://github.com/rdx-lang/rdx/blob/main/SPECIFICATION.md).

Provides syntax highlighting, code folding, and structural analysis for any editor that supports tree-sitter (Neovim, Helix, Zed, Emacs, etc.).

## Parsed constructs

- Frontmatter (`---` delimited YAML metadata)
- Components — open/close tags, self-closing, nested
- Attributes — string, primitive, JSON, variable, boolean shorthand (including hyphenated names like `data-id`)
- Variable interpolation — `{$path.to.value}`
- Escape sequences — `\{$`, `\{{`, `\}}`, `\{`, `\\`
- Fenced code blocks with language tags (external scanner)
- Display math `$$...$$` (external scanner) and inline math `$...$`
- Plain text and inline code

Markdown formatting (bold, italic, headings, lists) is intentionally treated as plain text — RDX components and variables are the tree-sitter grammar's concern, not Markdown structure.

## Usage

### Neovim

Add to your tree-sitter config:

```lua
require("nvim-treesitter.configs").setup({
  ensure_installed = { "rdx" },
})
```

### Helix

Add to `languages.toml`:

```toml
[[language]]
name = "rdx"
scope = "source.rdx"
file-types = ["rdx"]
roots = []

[language.auto-pairs]

[[grammar]]
name = "rdx"
source = { git = "https://github.com/rdx-lang/tree-sitter-rdx", rev = "main" }
```

## Development

```sh
bun install
bunx tree-sitter generate
bunx tree-sitter test
```

### Parse a file

```sh
bunx tree-sitter parse test/example.rdx
```

### Project structure

```
grammar.js          — Grammar definition
src/scanner.c       — External scanner (code fences, math fences, frontmatter)
queries/
  highlights.scm    — Syntax highlighting queries
  injections.scm    — Language injection queries
test/
  corpus/           — Tree-sitter corpus tests
  example.rdx       — Full kitchen-sink example
```

## License

MIT
