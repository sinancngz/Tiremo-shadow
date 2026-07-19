################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/tiremo_shadow/Tiremo-shadow/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Source/TmplUserApp/Tiremo/sht40/bsp/lib/tiremo_sht40_common.c \
C:/tiremo_shadow/Tiremo-shadow/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Source/TmplUserApp/Tiremo/sht40/bsp/lib/tiremo_sht40_driver.c \
C:/tiremo_shadow/Tiremo-shadow/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Source/TmplUserApp/Tiremo/sht40/bsp/lib/tiremo_sht40_i2c.c \
C:/tiremo_shadow/Tiremo-shadow/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Source/TmplUserApp/Tiremo/sht40/bsp/lib/tiremo_sht40_i2c_hal.c 

OBJS += \
./Main/Tiremo/sht40/bsp/lib/tiremo_sht40_common.o \
./Main/Tiremo/sht40/bsp/lib/tiremo_sht40_driver.o \
./Main/Tiremo/sht40/bsp/lib/tiremo_sht40_i2c.o \
./Main/Tiremo/sht40/bsp/lib/tiremo_sht40_i2c_hal.o 

C_DEPS += \
./Main/Tiremo/sht40/bsp/lib/tiremo_sht40_common.d \
./Main/Tiremo/sht40/bsp/lib/tiremo_sht40_driver.d \
./Main/Tiremo/sht40/bsp/lib/tiremo_sht40_i2c.d \
./Main/Tiremo/sht40/bsp/lib/tiremo_sht40_i2c_hal.d 


# Each subdirectory must supply rules for building sources it contributes
Main/Tiremo/sht40/bsp/lib/tiremo_sht40_common.o: C:/tiremo_shadow/Tiremo-shadow/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Source/TmplUserApp/Tiremo/sht40/bsp/lib/tiremo_sht40_common.c Main/Tiremo/sht40/bsp/lib/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU Arm Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mlittle-endian -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -g3 -DEXTRN_SUBFAMILY_A34G43x -DEXTRN_ABOV_MODULE_CONFIG -I"C:/tiremo_shadow/Tiremo-shadow/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Framework/CMSIS/Core/Include" -I"C:/tiremo_shadow/Tiremo-shadow/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Framework/CMSIS/Device/ABOV/Include" -I"C:/tiremo_shadow/Tiremo-shadow/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../ProductConfig/Config" -I"C:/tiremo_shadow/Tiremo-shadow/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Platform/HAL/Include" -I"C:/tiremo_shadow/Tiremo-shadow/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Platform/HAL/HPL/Include" -I"C:/tiremo_shadow/Tiremo-shadow/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Platform/Library/ABOV/Debug/Include" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Main/Tiremo/sht40/bsp/lib/tiremo_sht40_driver.o: C:/tiremo_shadow/Tiremo-shadow/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Source/TmplUserApp/Tiremo/sht40/bsp/lib/tiremo_sht40_driver.c Main/Tiremo/sht40/bsp/lib/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU Arm Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mlittle-endian -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -g3 -DEXTRN_SUBFAMILY_A34G43x -DEXTRN_ABOV_MODULE_CONFIG -I"C:/tiremo_shadow/Tiremo-shadow/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Framework/CMSIS/Core/Include" -I"C:/tiremo_shadow/Tiremo-shadow/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Framework/CMSIS/Device/ABOV/Include" -I"C:/tiremo_shadow/Tiremo-shadow/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../ProductConfig/Config" -I"C:/tiremo_shadow/Tiremo-shadow/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Platform/HAL/Include" -I"C:/tiremo_shadow/Tiremo-shadow/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Platform/HAL/HPL/Include" -I"C:/tiremo_shadow/Tiremo-shadow/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Platform/Library/ABOV/Debug/Include" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Main/Tiremo/sht40/bsp/lib/tiremo_sht40_i2c.o: C:/tiremo_shadow/Tiremo-shadow/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Source/TmplUserApp/Tiremo/sht40/bsp/lib/tiremo_sht40_i2c.c Main/Tiremo/sht40/bsp/lib/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU Arm Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mlittle-endian -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -g3 -DEXTRN_SUBFAMILY_A34G43x -DEXTRN_ABOV_MODULE_CONFIG -I"C:/tiremo_shadow/Tiremo-shadow/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Framework/CMSIS/Core/Include" -I"C:/tiremo_shadow/Tiremo-shadow/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Framework/CMSIS/Device/ABOV/Include" -I"C:/tiremo_shadow/Tiremo-shadow/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../ProductConfig/Config" -I"C:/tiremo_shadow/Tiremo-shadow/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Platform/HAL/Include" -I"C:/tiremo_shadow/Tiremo-shadow/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Platform/HAL/HPL/Include" -I"C:/tiremo_shadow/Tiremo-shadow/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Platform/Library/ABOV/Debug/Include" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Main/Tiremo/sht40/bsp/lib/tiremo_sht40_i2c_hal.o: C:/tiremo_shadow/Tiremo-shadow/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Source/TmplUserApp/Tiremo/sht40/bsp/lib/tiremo_sht40_i2c_hal.c Main/Tiremo/sht40/bsp/lib/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU Arm Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mlittle-endian -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -g3 -DEXTRN_SUBFAMILY_A34G43x -DEXTRN_ABOV_MODULE_CONFIG -I"C:/tiremo_shadow/Tiremo-shadow/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Framework/CMSIS/Core/Include" -I"C:/tiremo_shadow/Tiremo-shadow/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Framework/CMSIS/Device/ABOV/Include" -I"C:/tiremo_shadow/Tiremo-shadow/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../ProductConfig/Config" -I"C:/tiremo_shadow/Tiremo-shadow/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Platform/HAL/Include" -I"C:/tiremo_shadow/Tiremo-shadow/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Platform/HAL/HPL/Include" -I"C:/tiremo_shadow/Tiremo-shadow/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Platform/Library/ABOV/Debug/Include" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


