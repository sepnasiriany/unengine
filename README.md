# unengine - Banana Emulator

unengine is a C++ emulator for the fictional **Banana** video game console, capable of executing Slug ROM files. It accurately emulates CPU instructions, memory management, GPU rendering, and controller input, supporting games like **Flappy Bird** and **Snake**.

## Features

- **Complete Emulator for Banana Console**: Supports running Slug ROMs with accurate execution.
- **CPU Instruction Decoding**: Implements I-type and R-type instruction sets with bit-masking techniques.
- **Memory Management**: Handles memory-mapped IO, permissions, and endian conversion.
- **GPU Rendering with SDL2**: Translates VRAM data into a visual output using SDL.
- **Controller Input Handling**: Maps keyboard inputs to Banana console buttons.
- **Save/Load States**: Enables users to save and restore emulation sessions.
- **Disassembler**: Converts binary instructions into human-readable assembly code.
- **Unit Testing & CI/CD**: Includes Google Test framework and GitHub Actions integration for automated testing.

## Installation

### Prerequisites
- C++ compiler (GCC/Clang/MSVC)
- CMake
- SDL2
- Google Test (for unit testing)

### Build Instructions
```bash
# Clone the repository
git clone https://github.com/cse111-sp24-g17/unengine.git
cd unengine

# Create a build directory
mkdir build && cd build

# Run CMake to generate Makefiles
cmake ..

# Build the emulator
make
```

## Usage
```bash
# Run the emulator with a Slug ROM
./unengine path/to/rom.slug
```

## Controls
- **Arrow Keys**: Move (if applicable in the game)
- **Spacebar**: Jump/Action
- **Escape**: Exit emulator

## Testing
To run unit tests:
```bash
make test
```

## Credits
Developed by:
- Max Carr
- Jonah Ryan
- Evan Lake
- Sep Nasiriany

---
