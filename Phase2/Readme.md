# Phase 2: Bare-Metal Peripheral Drivers

**Name:** Jay Dukare  
**Target MCU:** STM32F103C8T6 (Blue Pill)  
**Development Environment:** PlatformIO with VS Code  

## Overview
In this phase, I implemented bare-metal drivers for UART and ADC on the STM32F103.  
I deliberately avoided HAL and abstraction libraries and worked directly with peripheral registers. The goal was to understand how the hardware actually behaves and how data flows between peripherals, DMA, and the CPU.

This submission covers UART communication using polling, interrupts, and DMA, along with ADC data acquisition using polling and DMA.

---

## Part A: UART Driver (Transmission & Loopback)

**UART Configuration**
- Baud Rate: 9600  
- Data Bits: 8  
- Parity: None  
- Stop Bits: 1 (8N1)  
- Pins Used:  
  - TX → PA9  
  - RX → PA10  

### Implemented Modes

#### 1. Polling Mode (`uart_polling`)
- On startup, the firmware sends a fixed 50-byte string (`ABCDEFGHIJKLMNOPQRSTUVWXYZ...`) to confirm transmission.
- Implements a blocking loopback mechanism:
  - The CPU waits until a character is received.
  - The same character is immediately sent back over UART.
- This mode helped me understand basic UART flags and blocking communication.
- **Status:** Tested and verified (see `uart_output.png`).

#### 2. Interrupt Mode (`uart_interrupt`)
- UART reception is handled inside `USART1_IRQHandler`.
- The main loop executes `__WFI()` so the CPU stays in sleep mode when idle.
- When data arrives:
  - The UART interrupt wakes the CPU.
  - The ISR reads the received byte and echoes it back.
- This approach demonstrates interrupt-driven, low-power communication.

#### 3. DMA Mode (`uart_dma`)
- DMA1 Channel 4 is configured for USART1 TX.
- The 50-byte message is transferred directly from memory to `USART1->DR`.
- The CPU is completely free during the transfer.
- This mode shows how DMA can offload repetitive data movement from the processor.

---

### Technical Details & Design Choices

**GPIO Speed Selection**
- PA9 (TX) is configured as Alternate Function Push-Pull with **50 MHz** output speed.
- A higher GPIO speed ensures faster signal edges, which helps maintain clean UART timing and reduces the chance of framing errors, especially at higher baud rates.

**Baud Rate Calculation**
- System Clock: 72 MHz  
- Target Baud Rate: 9600  

USARTDIV = 72,000,000 / (16 × 9600) = 468.75
Mantissa = 468 (0x1D4)
Fraction = 0.75 × 16 = 12 (0xC)
Final BRR = 0x1D4C


**Error Handling**
- **Overrun Error (ORE):** Occurs if new data arrives before the previous byte is read.
- **Framing Error (FE):** Happens when the stop bit is not detected correctly.
- Both errors are cleared by reading `USART1->SR` followed by `USART1->DR`, as per the reference manual.

---

## Part B: ADC Driver (Analog Data Acquisition)

**ADC Configuration**
- Resolution: 12-bit  
- Input Channel: PA0 (ADC Channel 0)  
- Mode: Single-ended  

### Implemented Modes

#### 1. Polling Mode (`adc_polling`)
- ADC conversion is manually started using the `ADON` bit.
- The firmware waits until the `EOC` (End of Conversion) flag is set.
- The converted value (0–4095) is sent over UART for validation.
- This mode helped verify basic ADC functionality and scaling.

#### 2. DMA Mode (`adc_dma`)
- DMA1 Channel 1 is used in **Circular Mode**.
- ADC conversion results are continuously written into a memory buffer (`adc_buffer`).
- The CPU simply reads the buffer when needed—no polling or waiting.
- This setup is suitable for continuous sensor monitoring applications.

---

### Technical Details & Design Choices

**ADC Clock Configuration**
- ADC prescaler is set to `RCC_CFGR_ADCPRE_DIV6`.
- ADC Clock = 72 MHz / 6 = **12 MHz**
- This value is within the datasheet limit (maximum 14 MHz), ensuring reliable ADC operation.

**Calibration**
- ADC calibration (`ADC_CR2_CAL`) is performed once after power-up.
- This improves conversion accuracy and stability.

**DMA Circular Mode Usage**
- Circular mode allows the DMA to automatically restart at the beginning of the buffer after reaching the end.
- This eliminates the need for reinitializing DMA transfers and enables continuous sampling.

**Recovery Strategy**
- If abnormal ADC readings occur (noise or stalled values), the ADC can be safely recovered by:
  - Clearing the `ADON` bit
  - Waiting briefly
  - Re-running the calibration sequence

---

## Summary
This phase strengthened my understanding of:
- Register-level peripheral configuration
- UART communication using polling, interrupts, and DMA
- ADC data acquisition with and without DMA
- Efficient CPU usage and peripheral offloading using DMA

All modules were tested on actual hardware, and outputs were verified through UART logs.