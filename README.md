# EmbeddedStm1

STM32F411CEU6 course project. The baseline uses the board's 25 MHz HSE to run at 100 MHz and leaves SWD available for ST-LINK.

Open stm32_start.ioc in STM32CubeMX to configure the board. The generated sources and STM32CubeF4 drivers are included so a fresh clone can build directly.

## Build

Install the Arm GNU Toolchain, CMake, and Ninja, and add their executables to PATH.

    cmake --preset Debug
    cmake --build --preset Debug

The output is build/Debug/stm32_start.elf. In CLion, open the project folder and use the Debug preset. Configure ST-LINK and Arm GDB in your local debug profile.

Keep custom code in separate modules or inside CubeMX's USER CODE blocks. Commit changes to the .ioc alongside the generated source changes they require.
