################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/libs/subdir.mk
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
all: CPU

dependents:
	-cd /home/utnso/tp-2014-2c-los-metatecla/ansisop-panel/Debug && $(MAKE) all
	-cd /home/utnso/tp-2014-2c-los-metatecla/MiniGral_lib/Debug && $(MAKE) all

# Tool invocations
CPU: $(OBJS) $(USER_OBJS) /home/utnso/tp-2014-2c-los-metatecla/ansisop-panel/Debug/libansisop-panel.so /home/utnso/tp-2014-2c-los-metatecla/MiniGral_lib/Debug/libMiniGral_lib.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/home/utnso/tp-2014-2c-los-metatecla/MiniGral_lib/Debug" -L"/home/utnso/tp-2014-2c-los-metatecla/ansisop-panel/Debug" -o "CPU" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) CPU
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
/home/utnso/tp-2014-2c-los-metatecla/ansisop-panel/Debug/libansisop-panel.so:
/home/utnso/tp-2014-2c-los-metatecla/MiniGral_lib/Debug/libMiniGral_lib.so:

-include ../makefile.targets
