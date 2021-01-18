The LC-900a Assembler
===============

To aid in testing your processor, we have provided an *assembler*
for the LC-900a architecture. The assembler supports
converting text `.s` files into either binary (for the simulator) or
hexadecimal (for pasting into CircuitSim) formats.

Requirements
-----------

The assembler runs on any version of Python 2.6+. An
instruction set architecture definition file is required along with
the assembler. The LC-900a assembler definition is included.

Included Files
-----------

* `assembler.py`: the main assembler program
* `LC-900a.py`: the LC-900a assembler definition

Using the Assembler
-----------

### Assemble for CircuitSim

To output assembled code in hexadecimal (for use with *CircuitSim*):

    python assembler.py -i lc900a --hex prj2.s

You can then open the resulting `test.hex` file in your favorite text
editor.  In CircuitSim, right-click on your RAM, select **Edit
Contents...**, and then copy-and-paste the contents of the hex file
into the window.

Assembler Pseudo-ops
-----------

In addition to the syntax described in the LC-900 ISA reference,
the assembler supports the following *pseudo-instructions*:

* `.fill`: fills a word of memory with the literal value provided

For example, `mynumber: .fill 0x500` places `0x500` at the memory
location labeled by `mynumber`.

* `.word`: an alias for `.fill`
