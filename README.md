<div align="center">
<h1>The UYB Compiler Backend</h1>

<b>UYB is a from-scratch optimising compiler backend written in C, designed to be small and have fast compilation, while still being complete enough to be used for an actual compiler.</b>

[![License badge](https://img.shields.io/badge/license-MPL_2.0-red?style=flat-square&label=license)](https://github.com/UnmappedStack/UYB/blob/main/LICENSE)
![Repo size badge](https://img.shields.io/github/repo-size/UnmappedStack/UYB?style=flat-square)
![Commit activity badge](https://img.shields.io/github/commit-activity/t/UnmappedStack/UYB?style=flat-square)

<br /><img alt="Black box icon" width="50%" src="blackbox.png" />
</div>

UYB is based heavily on QBE IR syntax and is almost fully instruction set compatible. The goal is self hosting through [cproc](https://github.com/michaelforney/cproc), which is a C compiler which targets QBE's IR.

There's a Discord server for UYB which you can join for help setting up your language with UYB, helping to contribute, or just having a chat, which you can join [here](https://discord.gg/W5uYqPAJg5).

## Why not just the original QBE?
I myself absolutely love QBE, and am a huge fan of the "80% of the performance in 10% of the code," however there are a few things that UYB improves upon (or may also just be a different use case rather than being better):
 - **QBE doesn't support inline assembly.** In most cases, this is fine, but when working on a very low level language where you simply need to interact with the CPU's instructions directly, a lack of inline assembly support can be unfortunate, and it's a massive Quality of Life feature to not need to put every single piece of assembly in a seperate file.
 - **There's still room for QBE to be smaller.** UYB accepts slower runtime speeds of generated assembly, and uses less optimisations in return for a smaller amount of code and faster compilation - the goal with UYB is more like 60% of the speed in 5% of the code.
 - **Debug symbols support.** Unfortunately, QBE doesn't support debug symbols, which means debugging generated programs with GDB is near impossible to do effectively.

## Support
UYB supports every QBE instruction except for floating point instructions. UYB also supports:

### Optimisations
 - Folding
 - Copy elimination
 - Unused label removal

### Targets
 - x86_64 generic System-V
 - SSA IR

## Usage
Since UYB is supposed to be based on QBE's IR, you can see [QBE's documentation](https://c9x.me/compile/doc/il.html) for a full IR reference.

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
    $ uyb test.ssa -o out.S
    ```
- Use a standard toolchain to assemble and link the generated Assembly to an executable program, then run it:
    ```sh
    $ gcc out.S -o out
    $ ./out
    Hello, world!
    ```

**To use debug symbols**, you can use GAS-AT&T like syntax. To use the previous example program as an example:
```
# Define the source file that this SSA was generated from.
# The first argument is the index ID of this file (so if you have more files then they need to each
# have a different ID) and the second argument is the filename.
.file 1 "test.c"

data $msg = {b "Hello, world!", b 10, b 0}
export function w $main(l %argc, l %argv) {
@start
    # The .loc pseudoinstruction specifies where in the file the following instructions are built from.
    # The first argument is the index of the file it came from (same as the ID for relevant .file),
    # and the next two arguments are the row and column, respectively.
    .loc 1 3 0
    call $printf(l $msg)
    .loc 1 4 0
    ret 0
}
```

**To use inline assembly**, you can use the following syntax. Note that inline assembly is not supported in the IR self-targetting target.
```
asm("<assembly goes here>" : %inputValue | "<reg>", %inputValue2 | "<reg>" : %outputValue | "<reg>", %outputValue2 | "<reg>" : "<reg>", "<reg>")
```
The types of inputs are split with colons (`:`):

 - The first input type is the raw assembly. It cannot contain any new lines within the source IL, however it may contain escape sequences such as `\t` and `\n`.
 - The second input type is the input list, split by commas. Each entry is in the format of `%label | "%rax"`, where the label contains the input value to pass in and `%rax` is replaced with the register that the input should be passed to in. The label and the register must both be 64 bits.
 - The third input type is the output list, which follows the same format as the input list.
 - The fourth and final input type is the clobber list. This is a list of string literals containing register names split by commas, in the form of `"%rax", "%rbx"`. These shouldn't contain input or output registers, but they *can*. These are the registers that are used by the inline assembly, so that UYB knows to be careful with them since they may be messed up. They must be 64 bit general purpose registers.

Note that checking of most inline assembly is left to the assembler and linker for the sake of lightweightedness, which means that programs containing inline assembly cannot be confirmed to work while they are still not assembled or linked.

You can use `uyb --help` to see all the command line options for UYB.

## Building
To clone and build UYB, simply run:
```sh
git clone https://github.com/UnmappedStack/UYB
cd UYB
make
```
This will also install a symlink in your bin directory so that you can call UYB from anywhere. CMake is required.

## License
This project is under the Mozilla Public License 2.0. See `LICENSE` for more information.
