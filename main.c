/*
 * GPIO-Keypad.c
 *
 * Created: 2/14/2025 12:50:05 PM
 * Author: Alex Cooper
 */ 

#define F_CPU 16000000UL // 16MHz clock from the debug processor
#define BAUD 9600
#define BAUDRATE (F_CPU / (BAUD * 16UL) - 1)
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>

void USART_Init(uint32_t baud_rate) {
	// Set baud rate (high and low)
	UBRR0H = (uint8_t)(baud_rate >> 8);
	UBRR0L = (uint8_t)baud_rate;
	// Enable receive and transfer on USART0
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);
	// 8bit Data
	UCSR0C = (1 << UCSZ00) | (1 << UCSZ01);
	// Disable parity
	UCSR0C &= ~((1 << UPM01) ^ (1 << UPM00));
	// 1bit Stop
	UCSR0C &= ~(1 << USBS0);
}

void Keypad_Init() {
	DDRD |= (1<<DDD4) | (1 << DDD5) | (1 << DDD6) | (1 << DDD7); // Set registers PD4-7 to high (output) // Acts as rows
	PORTD |=  (1 << PORTD4) | (1 << PORTD5) | (1 << PORTD6) | (1 << PORTD7); // enable pullup resistors for the rows
	DDRB &= ~(1 << DDB0) & ~(1 << DDB1) & ~(1 << DDB2) & ~(1 << DDB3); // Set registers PB0-3 to low (input) // Acts as columns
	PORTB |=  (1 << PORTB0) | (1 << PORTB1) | (1 << PORTB2) | (1 << PORTB3); // enable pullup resistors for the columns
}

const unsigned char keypad[4][4] = {
	{'1', '2', '3', 'A'},
	{'4', '5', '6', 'B'},
	{'7', '8', '9', 'C'},
	{'*', '0', '#', 'D'},
};

// Transmits a single piece of data
void USART_Transmit(uint8_t data) {
	/* Wait for empty transmit buffer */
	while (!(UCSR0A & (1 << UDRE0)))
	{}
	// Put data into buffer, sends the data
	UDR0 = data;
}

// Receives a single piece of data
uint8_t USART_Receive() {
	// Wait for data to be received
	while (!(UCSR0A & (1 << RXC0))) {}
	// Get and return received data from buffer
	return UDR0;
}

// Transmits an array of data
void USART_Send(const char* str) {
	while (*str != '\0') {
		USART_Transmit(*str);
		str++;
	}
}

int main() {
	USART_Init(BAUDRATE);
	Keypad_Init();
	while (1) {
		for (int i = 4; i < 8; i++)  {
			PORTD &= ~(1 << i); // Disable row pullup resistor
			for (int j = 0; j < 4; j++) {
				if (!(PINB & (1 << j))) {
					USART_Transmit(keypad[i-4][j]);
					_delay_ms(300); // debounce
				while (!(PINB & (1 << j))) {}; // prevent repeating keys when key is held down
				}
			}
			PORTD |= (1 << i); // reenable row pullup resistor
		}
	}
	return 0;
}
