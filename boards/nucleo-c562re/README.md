# NUCLEO-C562RE Board Support

## Status: Placeholder

The STM32C5 family CMSIS device headers are **not yet publicly available** from
STMicroelectronics. This board support package contains placeholder files.

## When headers become available

1. Update `scripts/fetch-deps.sh` to clone `STMicroelectronics/cmsis-device-c5`
2. Uncomment the relevant lines in `CMakeLists.txt`
3. Verify memory layout in `stm32c562re.ld` against the datasheet
4. Implement `system.c` clock configuration
5. Update `openocd.cfg` with the correct target config

## Resources

- [STM32C5 product page](https://www.st.com/en/microcontrollers-microprocessors/stm32c5-series.html)
- [NUCLEO-C562RE user manual](https://www.st.com/resource/en/user_manual/) (check ST website)
