################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Startup/subdir.mk
-include Src/subdir.mk
-include Lib/STM32F0xx_HAL_Driver/Src/subdir.mk
-include Lib/CMSIS/Device/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
SECONDARY_FLASH += \
PS1MemoryCard.hex \

SECONDARY_SIZE += \
PS1MemoryCard.siz \


# All Target
all: PS1MemoryCard.elf secondary-outputs

# Tool invocations
PS1MemoryCard.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross ARM GNU C Linker'
	arm-none-eabi-gcc -mcpu=cortex-m0 -mthumb -O0 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -pedantic  -g3 -T "F:\WORK\!ARM\PS1MemoryCard\Startup\stm32_flash.ld" -Xlinker --gc-sections -Wl,-Map,"PS1MemoryCard.map" -o "PS1MemoryCard.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

PS1MemoryCard.hex: PS1MemoryCard.elf
	@echo 'Invoking: Cross ARM GNU Create Flash Image'
	arm-none-eabi-objcopy -O ihex "PS1MemoryCard.elf"  "PS1MemoryCard.hex"
	@echo 'Finished building: $@'
	@echo ' '

PS1MemoryCard.siz: PS1MemoryCard.elf
	@echo 'Invoking: Cross ARM GNU Print Size'
	arm-none-eabi-size --format=berkeley "PS1MemoryCard.elf"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(SECONDARY_FLASH)$(SECONDARY_SIZE)$(ASM_DEPS)$(S_UPPER_DEPS)$(C_DEPS) PS1MemoryCard.elf
	-@echo ' '

secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_SIZE)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
