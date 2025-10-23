#include "room_control.h"

#include "gpio.h"    // Para controlar LEDs
#include "systick.h" // Para obtener ticks y manejar tiempos
#include "uart.h"    // Para enviar mensajes
#include "tim.h"     // Para controlar el PWM

// Estados de la sala
typedef enum {
    ROOM_IDLE,
    ROOM_OCCUPIED
} room_state_t;

// Variable de estado global
room_state_t current_state = ROOM_IDLE;
static uint32_t led_on_time = 0;
static uint8_t g_pwm_duty = PWM_INITIAL_DUTY; // Duty actual del PWM (0-100)

// Establece PWM y guarda duty actual
static void room_set_pwm(uint8_t duty)
{
    tim3_ch1_pwm_set_duty_cycle(duty);
    g_pwm_duty = duty;
}

// Devuelve el duty actual del PWM
static uint8_t room_control_get_pwm_duty(void)
{
    return g_pwm_duty;
}

// Envía un número (0-100) por UART
static void uart_send_uint8(uint8_t v)
{
    if (v >= 100) {
        uart_send('1');
        uart_send('0');
        uart_send('0');
        return;
    }
    if (v >= 10) {
        uart_send((char)('0' + (v / 10)));
        uart_send((char)('0' + (v % 10)));
        return;
    }
    uart_send((char)('0' + v));
}

void room_control_app_init(void)
{
    // Inicializar PWM al duty cycle inicial (estado IDLE -> LED apagado)
    room_set_pwm(PWM_INITIAL_DUTY);

    //Estado inicial del sistema de lámpara y puerta 
    current_state = ROOM_IDLE;
    //Imprime estado inicial del sistema, porcentaje de la lámpara al 20% y puerta cerrada, sin modificar la lógica del entorno
    uart_send_string("Controlador de Sala v2.0");
    uart_send_string("Sistema inicializado:\r\n");
    uart_send_string("PWM: 20%\r\n");
    uart_send_string("Puerta cerrada\r\n");
    room_control_send_status();

}

void room_control_on_button_press(void)
{
    if (current_state == ROOM_IDLE) {
        current_state = ROOM_OCCUPIED;
        room_set_pwm(100);  // PWM al 100%
        led_on_time = systick_get_ms();
        uart_send_string("Sala ocupada\r\n");
    } else {
        current_state = ROOM_IDLE;
        room_set_pwm(0);  // PWM al 0%
        uart_send_string("Sala vacía\r\n");
    }
}

void room_control_on_uart_receive(char received_char)
{
    switch (received_char) {
        case 'h':
        case 'H':
            room_set_pwm(100);
            uart_send_string("PWM: 100%\r\n");
            break;
        case 'l':
        case 'L':
            room_set_pwm(0);
            uart_send_string("PWM: 0%\r\n");
            break;
        case 'O':
        case 'o':
            current_state = ROOM_OCCUPIED;
            room_set_pwm(100);
            led_on_time = systick_get_ms();
            uart_send_string("Sala ocupada\r\n");
            break;
        case 'I':
        case 'i':
            current_state = ROOM_IDLE;
            room_set_pwm(0);
            uart_send_string("Sala vacía\r\n");
            break;
        
        //Implementamos comando 's' para enviar el estado actual de la sala y de la puerta abierta/cerrada   
        case 's':
        case 'S':
            room_control_send_status();
            break;

        //Implementación del comando 'g' para que la lámpara aumente gradualmente de 0% a 100% en pasos de 10% cada 500ms.
        case 'g':
        case 'G':
            for (uint8_t duty = 0; duty <= 100; duty += 10) {
                room_set_pwm(duty);
                uart_send_string("PWM: ");
                uart_send_uint8(duty);
                uart_send_string("%\r\n");
                uint32_t start = systick_get_ms();
                while ((systick_get_ms() - start) < 500U) { }
            }
            break;
        //Comando ? para mostrar menu de comandos disponibles
        case '?':
            uart_send_string("Comandos disponibles:\r\n");
            uart_send_string("H: PWM 100%\r\n");
            uart_send_string("L: PWM 0%\r\n");
            uart_send_string("O: Marcar sala como OCUPADA\r\n");
            uart_send_string("I: Marcar sala como VACÍA\r\n");
            uart_send_string("S: Enviar estado actual de la sala\r\n");
            uart_send_string("G: Rampa de PWM de 0% a 100%\r\n");
            uart_send_string("1-5: Establecer PWM a 10%,20%,30%,40%,50%\r\n");
            break;
        case '1':
            room_set_pwm(10);
            uart_send_string("PWM: 10%\r\n");
            break;
        case '2':
            room_set_pwm(20);
            uart_send_string("PWM: 20%\r\n");
            break;
        case '3':
            room_set_pwm(30);
            uart_send_string("PWM: 30%\r\n");
            break;
        case '4':
            room_set_pwm(40);
            uart_send_string("PWM: 40%\r\n");
            break;
        case '5':
            room_set_pwm(50);
            uart_send_string("PWM: 50%\r\n");
            break;
        default:
            uart_send_string("Comando desconocido: ");
            uart_send(received_char);
            uart_send_string("\r\n");
            break;
    }
}

void room_control_send_status(void)
{
    uart_send_string("Estado actual de la sala: ");
    if (current_state == ROOM_IDLE) {
        uart_send_string("VACÍA\r\n");
    } else {
        uart_send_string("OCUPADA\r\n");
    }
}

void room_control_update(void)
{
    if (current_state == ROOM_OCCUPIED) {
        if (systick_get_ms() - led_on_time >= LED_TIMEOUT_MS) {
            current_state = ROOM_IDLE;
            tim3_ch1_pwm_set_duty_cycle(0);
            uart_send_string("Timeout: Sala vacía\r\n");
        }
    }
}
