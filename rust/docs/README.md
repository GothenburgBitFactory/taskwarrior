This is an [mdbook](https://rust-lang.github.io/mdBook/index.html) book.
Minor modifications can be made without installing the mdbook tool, as the content is simple Markdown.
Changes are verified on pull requests.

To build the docs locally, you will need to build `usage-docs`:

```
cargo build -p taskchampion-cli --feature usage-docs --bin usage-docs
mdbook build docs/
```
