# RP2040-RotaryEncoder-Reader
![Version](https://img.shields.io/badge/version-1.0.0-green.svg)

A C library for reading rotary encoders on RP2040-based boards like the Raspberry Pi Pico.

# Dependencies
1. [RP2040_UART_Debugger](https://github.com/Tyrhaton/RP2040_UART_Debugger)
   A repository demonstrating UART communication using the Raspberry Pi Pico Debug Probe with RP2040-based boards. 

# Setup Instructions

## Option 1: Standalone Project Setup
1. Import the dependencies into the root directory:
   ```bash
   git clone https://github.com/Tyrhaton/RP2040_UART_Debugger.git
   ```

2. Create a build directory within the root directory:
   ```bash
   mkdir build
   ```

3. Navigate into the build directory:
   ```bash
   cd build
   ```

4. Generate the build files using CMake:
   ```bash
   cmake ..
   ```

5. Compile the project:
   ```bash
   make
   ```
   * Tip: Use `make -j4` to enable parallel compilation for faster build times.

6. Run the project using OpenOCD:
   ```bash
   sudo openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "program main.elf verify reset exit"
   ```

---

## Option 2: Integration into Another Project

1. Navigate to your project's `modules` directory:
   ```bash
   cd modules/
   ```

2. Clone this repository into the modules directory:
   ```bash
   git clone https://github.com/Tyrhaton/RP2040_RotaryEncoder_Reader.git
   ```

3. Update your project's `CMakeLists.txt`:
   ```bash
   # Add the RP2040_RotaryEncoder_Reader module
   add_library(RP2040_RotaryEncoder_Reader STATIC
     modules/RP2040_RotaryEncoder_Reader/rot.c
   )

   # Include the RP2040_RotaryEncoder_Reader module's header files
   target_include_directories(RP2040_RotaryEncoder_Reader PUBLIC modules/RP2040_RotaryEncoder_Reader)

   # Link the main executable with the RP2040_RotaryEncoder_Reader library and Pico SDK
   target_link_libraries(${PROJECT_NAME} RP2040_RotaryEncoder_Reader pico_stdlib)
    ```

4. Make sure to also do these steps for the dependencies!

5. Run the project using OpenOCD:
   ```bash
   sudo openocd -f interface/cmsis-dap.cfg -f target/rp2040.cfg -c "adapter speed 5000" -c "program main.elf verify reset exit"
   ```