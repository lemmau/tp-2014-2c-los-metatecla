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
all: Kernel

dependents:
	-cd /home/utnso/workspace/ansisop-panel/Release && $(MAKE) all
	-cd /home/utnso/tp-2014-2c-los-metatecla/MiniGral_lib/Release && $(MAKE) all

# Tool invocations
Kernel: $(OBJS) $(USER_OBJS) /home/utnso/workspace/ansisop-panel/Release/libansisop-panel.so /home/utnso/tp-2014-2c-los-metatecla/MiniGral_lib/Release/libMiniGral_lib.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/home/utnso/tp-2014-2c-los-metatecla/MiniGral_lib/Release" -L"/home/utnso/workspace/ansisop-panel/Release" -o "Kernel" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) Kernel
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
/home/utnso/workspace/ansisop-panel/Release/libansisop-panel.so:
/home/utnso/tp-2014-2c-los-metatecla/MiniGral_lib/Release/libMiniGral_lib.so:

-include ../makefile.targets
