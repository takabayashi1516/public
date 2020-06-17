######################################
# target
######################################
TARGET = stm32l475vg


######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -Og


#######################################
# paths
#######################################
# Build path
BUILD_DIR = build

FRAMEWORK_DIR = ../../framework
REF_SOURCES_DIR = ../../discovery-stm32l475

######################################
# source
######################################
# C sources
C_SOURCES =  \
$(REF_SOURCES_DIR)/Src/main.c \
$(REF_SOURCES_DIR)/Src/freertos.c \
$(REF_SOURCES_DIR)/Src/usb_device.c \
$(REF_SOURCES_DIR)/Src/usbd_conf.c \
$(REF_SOURCES_DIR)/Src/usbd_desc.c \
$(REF_SOURCES_DIR)/Src/usbd_cdc_if.c \
$(REF_SOURCES_DIR)/Src/stm32l4xx_it.c \
$(REF_SOURCES_DIR)/Src/stm32l4xx_hal_msp.c \
$(REF_SOURCES_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pcd.c \
$(REF_SOURCES_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pcd_ex.c \
$(REF_SOURCES_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_ll_usb.c \
$(REF_SOURCES_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_adc.c \
$(REF_SOURCES_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_adc_ex.c \
$(REF_SOURCES_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_i2c.c \
$(REF_SOURCES_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_i2c_ex.c \
$(REF_SOURCES_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_spi.c \
$(REF_SOURCES_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_spi_ex.c \
$(REF_SOURCES_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_tim.c \
$(REF_SOURCES_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_tim_ex.c \
$(REF_SOURCES_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_uart.c \
$(REF_SOURCES_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_uart_ex.c \
$(REF_SOURCES_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal.c \
$(REF_SOURCES_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc.c \
$(REF_SOURCES_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc_ex.c \
$(REF_SOURCES_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash.c \
$(REF_SOURCES_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash_ex.c \
$(REF_SOURCES_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash_ramfunc.c \
$(REF_SOURCES_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_gpio.c \
$(REF_SOURCES_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dma.c \
$(REF_SOURCES_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dma_ex.c \
$(REF_SOURCES_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pwr.c \
$(REF_SOURCES_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pwr_ex.c \
$(REF_SOURCES_DIR)/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_cortex.c \
$(REF_SOURCES_DIR)/Src/system_stm32l4xx.c \
$(REF_SOURCES_DIR)/Middlewares/Third_Party/FreeRTOS/Source/croutine.c \
$(REF_SOURCES_DIR)/Middlewares/Third_Party/FreeRTOS/Source/event_groups.c \
$(REF_SOURCES_DIR)/Middlewares/Third_Party/FreeRTOS/Source/list.c \
$(REF_SOURCES_DIR)/Middlewares/Third_Party/FreeRTOS/Source/queue.c \
$(REF_SOURCES_DIR)/Middlewares/Third_Party/FreeRTOS/Source/stream_buffer.c \
$(REF_SOURCES_DIR)/Middlewares/Third_Party/FreeRTOS/Source/tasks.c \
$(REF_SOURCES_DIR)/Middlewares/Third_Party/FreeRTOS/Source/timers.c \
$(REF_SOURCES_DIR)/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS/cmsis_os.c \
$(REF_SOURCES_DIR)/Middlewares/Third_Party/FreeRTOS/Source/portable/MemMang/heap_3.c \
$(REF_SOURCES_DIR)/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c \
$(REF_SOURCES_DIR)/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_core.c \
$(REF_SOURCES_DIR)/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ctlreq.c \
$(REF_SOURCES_DIR)/Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ioreq.c \
$(REF_SOURCES_DIR)/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc.c \
$(REF_SOURCES_DIR)/system/source/cortexm/_reset_hardware.c \
$(REF_SOURCES_DIR)/system/source/lib/assert.c \
$(REF_SOURCES_DIR)/system/source/lib/_exit.c \
$(REF_SOURCES_DIR)/system/source/lib/_sbrk.c \
$(REF_SOURCES_DIR)/system/source/lib/_syscalls.c \
$(REF_SOURCES_DIR)/system/source/diag/Trace.c \
$(REF_SOURCES_DIR)/system/source/diag/trace_impl.c \

# C++ sources
CPP_SOURCES =  \
$(REF_SOURCES_DIR)/system/source/lib/_cxx.cpp \
$(FRAMEWORK_DIR)/source/framework.cpp \
$(REF_SOURCES_DIR)/application/source/bme280.cpp \
$(REF_SOURCES_DIR)/application/source/peripheral_interface.cpp \
$(REF_SOURCES_DIR)/application/source/zigbee.cpp \
./application/source/main_task_cust.cpp \

# ASM sources
ASM_SOURCES =  \
$(REF_SOURCES_DIR)/startup_stm32l475xx.s


#######################################
# binaries
#######################################
PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
CX = $(GCC_PATH)/$(PREFIX)g++
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)gcc
CX = $(PREFIX)g++
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
 
#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m4

# fpu
FPU = -mfpu=fpv4-sp-d16

# float-abi
FLOAT-ABI = -mfloat-abi=hard

# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS = 

# C defines
C_DEFS =  \
-DUSE_HAL_DRIVER \
-DSTM32L475xx \
#-DOS_USE_TRACE_SEMIHOSTING_STDOUT \
#-DOS_USE_SEMIHOSTING \
#-DTRACE \
-DUSE_FULL_ASSERT \
#-DDEBUG_DUMP

# AS includes
AS_INCLUDES =  \
-I\Inc

# C includes
C_INCLUDES =  \
-I../include \
-I./application/include \
-I$(REF_SOURCES_DIR)/Inc \
-I$(REF_SOURCES_DIR)/Drivers/STM32L4xx_HAL_Driver/Inc \
-I$(REF_SOURCES_DIR)/Drivers/STM32L4xx_HAL_Driver/Inc/Legacy \
-I$(REF_SOURCES_DIR)/Middlewares/Third_Party/FreeRTOS/Source/include \
-I$(REF_SOURCES_DIR)/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS \
-I$(REF_SOURCES_DIR)/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F \
-I$(REF_SOURCES_DIR)/Middlewares/ST/STM32_USB_Device_Library/Core/Inc \
-I$(REF_SOURCES_DIR)/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc \
-I$(REF_SOURCES_DIR)/Drivers/CMSIS/Device/ST/STM32L4xx/Include \
-I$(REF_SOURCES_DIR)/Drivers/CMSIS/Include \
-I$(REF_SOURCES_DIR)/system/include \
-I$(REF_SOURCES_DIR)/system/include/arm \
-I$(REF_SOURCES_DIR)/system/include/cortexm \
-I$(FRAMEWORK_DIR)/include \
-I$(REF_SOURCES_DIR)/application/include \

# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections -Wno-ignored-qualifiers -Wno-unused-parameter

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif


# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"


#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = $(REF_SOURCES_DIR)/STM32L475VGTx_FLASH.ld

# libraries
LIBS = -lc -lm -lnosys 
LIBDIR = 
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections -nostartfiles

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin


#######################################
# build the application
#######################################
# list of cpp objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(CPP_SOURCES:.cpp=.o)))
vpath %.cpp $(sort $(dir $(CPP_SOURCES)))
# list of objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c -std=gnu11 $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.cpp Makefile | $(BUILD_DIR) 
	$(CX) -c -std=gnu++11 $(CFLAGS) -fno-exceptions -fno-rtti -fno-use-cxa-atexit -fno-threadsafe-statics -fpermissive -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.cpp=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CX) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@
	
$(BUILD_DIR):
	mkdir $@

#######################################
# clean up
#######################################
clean:
	-rm -fR $(BUILD_DIR)

#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

