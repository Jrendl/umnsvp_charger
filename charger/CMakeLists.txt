add_executable(charger)
setup_board(charger STM32F405 RGTX)

add_cmsis(charger)
add_hal(charger)
add_umnsvp(charger)
add_skylab2(charger)

use_stm32_linker_scripts(charger)

# Add the `inc` directory for this board
target_include_directories(charger PRIVATE inc)
add_CPU_options(charger)

# Set all the sources for this board.
# When adding a source to this project, be sure to check if the thread safety 
# generator created a new file. That new file needs to be added here too.
target_sources(charger
PRIVATE
  src/application.cc
  src/main.cc
  src/pwm_driver.cc
  src/timing.cc
  src/can2.cc
  src/stm32f4xx_hal_msp.cc
  src/stm32f4xx_it.cc
  # src/system_stm32f4xx.cc
  src/bms.cc
  src/thunderstruck.cc
  src/j1772.cc
  src/status_lights.cc
)
