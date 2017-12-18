Overview
========

This is a modification of the last stable release of TCOFFEE
that allows it to build and run natively on Windows with MSVC.

Modifications
=============

- Use threads instead of fork, because there is no equivalent on Windows.
- Renamed some constants and variables that conflicted with macro names
  defined deep in Windows headers, such as
  DELETE, IN, OUT, OPTIONAL, UNALIGNED, min, max.
- Unportable functions were renamed or re-implemented to use a Windows equivalent,
  such as directory creation or traversal, file existance.
- Temp names were altered to use process AND thread ids,
  because unforkable code needs more info than just process id.
- Custom makefile to use MSVC compiler and linker.
- Complex function pointer declarations had to be typedef-ed to compile.
- Variable Length Arrays (VLA) are not supported, so arrays
  were allocated or made reasonable static sizes.
- Some functions returned nothing, but declared int return type.
- Uninitialized variables.
- Garbage indentation.
- Insanely large else-if block split to make it compilable.
- Process memory ownership assumptions had to be refactored due to threading.
- Clock utilities were replaced with std::chrono.
- Get core count.
- Use Windows environment variables.
- Disabled embedded perl script due to exceeding msvc compiler limits.
  Requires splitting, but it looks like a web server feature.
- Removed unused variables and function args.

Debugging
=========

QtCreator can be used to debug the executable.

Build it from the command line with GNU Make:
  gmake -f makefile.win DEBUG_ENALBED=1

Use these args in Project->Run:
  -infile C:\tmp\input.fasta -type=dna -outfile C:\tmp\output.txt
