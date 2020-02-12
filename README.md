# Overview

This is a modification of the last stable release of TCOFFEE
that allows it to build and run natively on Windows with MSVC.

# Modifications

- Use threads instead of process forking, because there is no equivalent on Windows.
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
- Some functions returned nothing, but declared non-void return type.
- Uninitialized variables were give reasonable defaults.
- Indentation was adjusted and to use only spaces.
- A large else-if block was split to make it compilable by MSVC.
- Process memory ownership assumptions had to be refactored due to threading.
- Clock utilities were replaced with std::chrono.
- Get core count was enhanced to support Windows.
- Use Windows environment variables where necessary.
- Disabled embedded perl script due to exceeding MSVC compiler limits.
  This needs to be revisited to re-activate.
- Removed some unused variables, functions and function args, but some might still remain.
- Allow input and executable filenames to have spaces.
- Build and run without crashing 64-bit executable on Windows built natively with MSVC.

# Building

You'll need a c++11 compliant compiler.

## Mac/Linux

    cd t_coffee_source
    make -j
    
## Windows

You have to use GNU Make to support some convenient make extensions. 
Get it from either Chocolatey, GnuWin32 or a Git package.

    cd t_coffee_source
    gmake -j -f makefile.win

## Debugging

QtCreator can be used to debug the executable. First create a project file:

    cd t_coffee_source
    qmake -project

Then open the new t_coffee_source.pro file in QtCreator and disable qmake in
QtCreator > Projects > Build > Build Steps, otherwise qmake will overwrite our
makefile.
  
Only on Windows, use an extra required flag when building due to how the
MSVC compiler works:

    gmake -f makefile.win DEBUG_ENABLED=1

Set args in QtCreator > Projects > Run:

    -infile C:\tmp\input.fasta -type=dna -outfile C:\tmp\output.txt
