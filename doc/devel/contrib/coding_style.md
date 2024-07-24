# Coding Style

The coding style used for the Taskwarrior is based on the [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html), with small modifications in the line length.

# Automatic formatting

In order to have consistancy and automatic formatting [pre-commit](https://pre-commit.com) is used to apply [clang-format](https://clang.llvm.org/docs/ClangFormat.html) and [black](https://github.com/psf/black) are used.
In order to set them up locally please run:
```python
pip install pre-commit
pre-commit install
```

For more information refer to the [quick-start of pre-commit](https://pre-commit.com/#quick-start).
The setup is also included in the CI, hence if one can not install it automatically then the CI will take care for it in the PR.

## Rust

Rust code should be formatted with `rustfmt` and generally follow Rust style guidelines.
