# RDX Language Support

Syntax highlighting, snippets, and language support for [RDX (Reactive Document eXpressions)](https://github.com/rdx-lang/rdx) — a strictly typed, declarative document format built on CommonMark.

## Features

- Syntax highlighting for `.rdx` files
- Component tags (`<Notice>`, `<Badge />`) with attribute highlighting
- All five attribute types: string, primitive, JSON, variable, boolean shorthand
- Variable interpolation (`{$path}`) highlighting
- Escape sequence highlighting (`\{$`, `\{{`, `\}}`, `\{`, `\\`)
- YAML frontmatter with embedded YAML highlighting
- Inline and display math (`$x^2$`, `$$...$$`)
- Fenced code blocks with embedded language highlighting (38+ languages)
- Markdown formatting (headings, bold, italic, strikethrough, links, images)
- GFM extensions (footnotes, task lists)
- HTML comment support
- Code snippets for common patterns
- Auto-closing pairs and bracket matching
- Folding and indentation for component blocks

## Snippets

| Prefix        | Description                   |
| ------------- | ----------------------------- |
| `frontmatter` | Insert frontmatter block      |
| `comp`        | Block component with children |
| `comps`       | Self-closing component        |
| `var`         | Variable interpolation        |
| `jsonattr`    | JSON object attribute         |
| `primattr`    | Primitive attribute           |
| `varattr`     | Variable attribute            |
| `math`        | Display math block            |
| `imath`       | Inline math                   |
| `code`        | Fenced code block             |

## Links

- [RDX Specification](https://github.com/rdx-lang/rdx/blob/main/SPECIFICATION.md)
- [RDX Parser](https://github.com/rdx-lang/rdx)
- [Tree-sitter Grammar](https://github.com/rdx-lang/tree-sitter-rdx)

## License

MIT
