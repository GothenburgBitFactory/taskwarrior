use mdbook::book::{Book, BookItem};
use mdbook::errors::Error;
use mdbook::preprocess::{CmdPreprocessor, PreprocessorContext};
use std::io;
use std::process;
use taskchampion_cli::Usage;

/// This is a simple mdbook preprocessor designed to substitute information from the usage
/// into the documentation.
fn main() -> anyhow::Result<()> {
    // cheap way to detect the "supports" arg
    if std::env::args().len() > 1 {
        // sure, whatever, we support it all
        process::exit(0);
    }

    let (ctx, book) = CmdPreprocessor::parse_input(io::stdin())?;

    if ctx.mdbook_version != mdbook::MDBOOK_VERSION {
        eprintln!(
            "Warning: This mdbook preprocessor was built against version {} of mdbook, \
             but we're being called from version {}",
            mdbook::MDBOOK_VERSION,
            ctx.mdbook_version
        );
    }

    let processed_book = process(&ctx, book)?;
    serde_json::to_writer(io::stdout(), &processed_book)?;

    Ok(())
}

fn process(_ctx: &PreprocessorContext, mut book: Book) -> Result<Book, Error> {
    let usage = Usage::new();

    book.for_each_mut(|sect| {
        if let BookItem::Chapter(ref mut chapter) = sect {
            let new_content = usage.substitute_docs(&chapter.content).unwrap();
            if new_content != chapter.content {
                eprintln!(
                    "Substituting usage in {:?}",
                    chapter
                        .source_path
                        .as_ref()
                        .unwrap_or(chapter.path.as_ref().unwrap())
                );
            }
            chapter.content = new_content;
        }
    });
    Ok(book)
}
