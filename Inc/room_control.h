#ifndef ROOM_CONTROL_H
#define ROOM_CONTROL_H

#include <stdint.h>

// Constantes
#define LED_TIMEOUT_MS 3000  // Tiempo para apagar LED después de presionar botón
#define PWM_INITIAL_DUTY 0   // Duty cycle inicial para PWM LED (IDLE = apagado)

/**
 * @brief Función a ser llamada por EXTI15_10_IRQHandler cuando se detecta
 *        la pulsación del botón B1.
 */
void room_control_on_button_press(void);

/**
 * @brief Función a ser llamada por USART2_IRQHandler cuando se recibe un carácter.
 * @param received_char El carácter recibido por UART.
 */
void room_control_on_uart_receive(char received_char);

/**
 * @brief (Opcional) Función para realizar inicializaciones específicas de la lógica
 *        de room_control, si las hubiera.
 */
void room_control_app_init(void);

/**
 * @brief Función para actualizar la lógica de estados periódicamente (llamar en el bucle principal).
 *        Maneja timeouts, transiciones automáticas, etc.
 */
void room_control_update(void);
/**
 * @brief Envía por UART el estado actual de la sala (VACÍA / OCUPADA).
 */
void room_control_send_status(void);

/**
 * @brief Envía el nivel actual de PWM por UART
 */
void room_control_send_pwm_status(void);

#endif // ROOM_CONTROL_H