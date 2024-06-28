/**
 * @file main.cc
 * @author Joe Rendleman (rendl008@umn.edu), Owen Zeller (Zelle105@umn.edu),
 * Tori Sigfrid (sigfri013@umn.edu), Pachia Thor (thor0572@umn.edu)
 * @brief Charger main task.
 * @version 0.1
 * @date 2022-01-27
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "main.h"

#include "application.h"

umnsvp::charger::Application app;

/**
 * @brief  Main program.
 * @param  None
 * @retval None
 */
int main(void) {
    /*
     * Create and run the main application.
     */

    app.main();
}

void timer_handler_callback(TIM_HandleTypeDef* htim) {
    if (htim->Instance == TIM7) {
        app.broadcast_car_can();
    } else if (htim->Instance == TIM6) {
        app.broadcast_charger_can();
    } else if (htim->Instance == TIM1) {
        app.pwm_measure();
    }
}

#ifdef USE_FULL_ASSERT

/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t* file, uint32_t line) {
    /* User can add his own implementation to report the file name and line
       number, ex: printf("Wrong parameters value: file %s on line %d\r\n",
       file, line) */

    /* Infinite loop */
    while (1) {
    }
}

#endif
