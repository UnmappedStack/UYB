# The UYB Compiler Backend

UYB (abbreviation of `Up Your Backend`) is a from-scratch optimising compiler backend written in C, designed to be small and have fast compilation, while still being complete enough to be used for an actual compiler.

## Building
To clone and build UYB, simply run:
```sh
git clone https://github.com/UnmappedStack/UYB
cd UYB
make build
```

## Usage
Actual documentation coming soon :)

Try run this small "Hello World" program to test UYB like so:
- Copy this code into a file named `test.ssa` or something similar:
    ```
    data $msg = {b "Hello, world!", b 10, b 0}
    export function w $main(l argc, l argv) {
        %message =l copy $msg
        call $printf(l %message)
        ret 0
    }
    ```
- Compile the IR to x86_64 Assembly using the following command:
    ```sh
    $ ./uyb test.ssa -o out.S
    ```
- Use a standard toolchain to assemble and link the generated Assembly to an executable program, then run it:
    ```sh
    $ gcc out.S -o out
    $ ./out
    Hello, world!
    ```

You can use `./uyb --help` to see all the command line options for UYB.

## License
This project is under the Mozilla Public License 2.0. See `LICENSE` for more information.
