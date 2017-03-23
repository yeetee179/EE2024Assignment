################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: Copy\ of\ assignment2.axf

# Tool invocations
Copy\ of\ assignment2.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU Linker'
	arm-none-eabi-gcc -nostdlib -L"C:\Users\user-pc\Desktop\EE2024\Lib_CMSISv1p30_LPC17xx\Debug" -L"C:\Users\user-pc\Desktop\EE2024\Lib_EaBaseBoard\Debug" -L"C:\Users\user-pc\Desktop\EE2024\Lib_MCU\Debug" -Xlinker --gc-sections -Xlinker -Map=Copy of assignment2.map -mcpu=cortex-m3 -mthumb -T "Copy_of_assignment2_Debug.ld" -o "Copy of assignment2.axf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) "Copy of assignment2.axf"
	-@echo ' '

post-build:
	-@echo 'Performing post-build steps'
	-arm-none-eabi-size Copy of assignment2.axf; # arm-none-eabi-objdump -h -S Copy of assignment2.axf >Copy of assignment2.lss
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
