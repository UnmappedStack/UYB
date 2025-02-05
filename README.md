# The UYB Compiler Backend

UYB (abbreviation of `Up Your Backend`) is a from-scratch optimising compiler backend written in C, designed to be small and have fast compilation, while still being complete enough to be used for an actual compiler.

Currently, UYB doesn't have a textual Intermediate Representation, and everything is in the form of C structures in a library-like form (not an actual library however since it's a self-contained program for testing at the moment). UYB is *not* yet ready for production and has some simple functionality.

So far, it only compiles to a x86_64 Assembly target (GAS, AT&T).

## License
This project is under the Mozilla Public License 2.0. See `LICENSE` for more information.
