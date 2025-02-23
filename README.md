# The UYB Compiler Backend

UYB is a from-scratch optimising compiler backend written in C, designed to be small and have fast compilation, while still being complete enough to be used for an actual compiler.

There's a Discord server for UYB which you can join for help setting up your language with UYB, helping to contribute, or just having a chat, which you can join [here](https://discord.gg/W5uYqPAJg5).

## Building
To clone and build UYB, simply run:
```sh
git clone https://github.com/UnmappedStack/UYB
cd UYB
make build
```

## Usage
Actual documentation coming soon :)

There are more examples for UYB programs in `/examples`, or try run this small "Hello World" program to test UYB like so:
- Copy this code into a file named `test.ssa` or something similar:
    ```
    data $msg = {b "Hello, world!", b 10, b 0}
    export function w $main(l %argc, l %argv) {
    @start
        call $printf(l $msg)
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
