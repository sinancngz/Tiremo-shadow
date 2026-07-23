################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/Users/sinan/Desktop/Tiremo/Generation/AUDK32_A34xxxx-1.0.12/Example/Source/TmplUserApp/Libraries/SHT40/SHT40_app.c \
C:/Users/sinan/Desktop/Tiremo/Generation/AUDK32_A34xxxx-1.0.12/Example/Source/TmplUserApp/Libraries/SHT40/sensirion_common.c \
C:/Users/sinan/Desktop/Tiremo/Generation/AUDK32_A34xxxx-1.0.12/Example/Source/TmplUserApp/Libraries/SHT40/sensirion_i2c.c \
C:/Users/sinan/Desktop/Tiremo/Generation/AUDK32_A34xxxx-1.0.12/Example/Source/TmplUserApp/Libraries/SHT40/sensirion_i2c_hal.c \
C:/Users/sinan/Desktop/Tiremo/Generation/AUDK32_A34xxxx-1.0.12/Example/Source/TmplUserApp/Libraries/SHT40/sht4x_i2c.c 

OBJS += \
./Main/Libraries/SHT40/SHT40_app.o \
./Main/Libraries/SHT40/sensirion_common.o \
./Main/Libraries/SHT40/sensirion_i2c.o \
./Main/Libraries/SHT40/sensirion_i2c_hal.o \
./Main/Libraries/SHT40/sht4x_i2c.o 

C_DEPS += \
./Main/Libraries/SHT40/SHT40_app.d \
./Main/Libraries/SHT40/sensirion_common.d \
./Main/Libraries/SHT40/sensirion_i2c.d \
./Main/Libraries/SHT40/sensirion_i2c_hal.d \
./Main/Libraries/SHT40/sht4x_i2c.d 


# Each subdirectory must supply rules for building sources it contributes
Main/Libraries/SHT40/SHT40_app.o: C:/Users/sinan/Desktop/Tiremo/Generation/AUDK32_A34xxxx-1.0.12/Example/Source/TmplUserApp/Libraries/SHT40/SHT40_app.c Main/Libraries/SHT40/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU Arm Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mlittle-endian -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -g3 -DEXTRN_SUBFAMILY_A34G43x -DEXTRN_ABOV_MODULE_CONFIG -I"C:/Users/sinan/Desktop/Tiremo/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Framework/CMSIS/Core/Include" -I"C:/Users/sinan/Desktop/Tiremo/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Framework/CMSIS/Device/ABOV/Include" -I"C:/Users/sinan/Desktop/Tiremo/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../ProductConfig/Config" -I"C:/Users/sinan/Desktop/Tiremo/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Platform/HAL/Include" -I"C:/Users/sinan/Desktop/Tiremo/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Platform/HAL/HPL/Include" -I"C:/Users/sinan/Desktop/Tiremo/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Platform/Library/ABOV/Debug/Include" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Main/Libraries/SHT40/sensirion_common.o: C:/Users/sinan/Desktop/Tiremo/Generation/AUDK32_A34xxxx-1.0.12/Example/Source/TmplUserApp/Libraries/SHT40/sensirion_common.c Main/Libraries/SHT40/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU Arm Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mlittle-endian -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -g3 -DEXTRN_SUBFAMILY_A34G43x -DEXTRN_ABOV_MODULE_CONFIG -I"C:/Users/sinan/Desktop/Tiremo/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Framework/CMSIS/Core/Include" -I"C:/Users/sinan/Desktop/Tiremo/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Framework/CMSIS/Device/ABOV/Include" -I"C:/Users/sinan/Desktop/Tiremo/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../ProductConfig/Config" -I"C:/Users/sinan/Desktop/Tiremo/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Platform/HAL/Include" -I"C:/Users/sinan/Desktop/Tiremo/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Platform/HAL/HPL/Include" -I"C:/Users/sinan/Desktop/Tiremo/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Platform/Library/ABOV/Debug/Include" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Main/Libraries/SHT40/sensirion_i2c.o: C:/Users/sinan/Desktop/Tiremo/Generation/AUDK32_A34xxxx-1.0.12/Example/Source/TmplUserApp/Libraries/SHT40/sensirion_i2c.c Main/Libraries/SHT40/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU Arm Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mlittle-endian -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -g3 -DEXTRN_SUBFAMILY_A34G43x -DEXTRN_ABOV_MODULE_CONFIG -I"C:/Users/sinan/Desktop/Tiremo/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Framework/CMSIS/Core/Include" -I"C:/Users/sinan/Desktop/Tiremo/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Framework/CMSIS/Device/ABOV/Include" -I"C:/Users/sinan/Desktop/Tiremo/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../ProductConfig/Config" -I"C:/Users/sinan/Desktop/Tiremo/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Platform/HAL/Include" -I"C:/Users/sinan/Desktop/Tiremo/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Platform/HAL/HPL/Include" -I"C:/Users/sinan/Desktop/Tiremo/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Platform/Library/ABOV/Debug/Include" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Main/Libraries/SHT40/sensirion_i2c_hal.o: C:/Users/sinan/Desktop/Tiremo/Generation/AUDK32_A34xxxx-1.0.12/Example/Source/TmplUserApp/Libraries/SHT40/sensirion_i2c_hal.c Main/Libraries/SHT40/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU Arm Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mlittle-endian -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -g3 -DEXTRN_SUBFAMILY_A34G43x -DEXTRN_ABOV_MODULE_CONFIG -I"C:/Users/sinan/Desktop/Tiremo/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Framework/CMSIS/Core/Include" -I"C:/Users/sinan/Desktop/Tiremo/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Framework/CMSIS/Device/ABOV/Include" -I"C:/Users/sinan/Desktop/Tiremo/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../ProductConfig/Config" -I"C:/Users/sinan/Desktop/Tiremo/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Platform/HAL/Include" -I"C:/Users/sinan/Desktop/Tiremo/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Platform/HAL/HPL/Include" -I"C:/Users/sinan/Desktop/Tiremo/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Platform/Library/ABOV/Debug/Include" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Main/Libraries/SHT40/sht4x_i2c.o: C:/Users/sinan/Desktop/Tiremo/Generation/AUDK32_A34xxxx-1.0.12/Example/Source/TmplUserApp/Libraries/SHT40/sht4x_i2c.c Main/Libraries/SHT40/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU Arm Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mlittle-endian -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -g3 -DEXTRN_SUBFAMILY_A34G43x -DEXTRN_ABOV_MODULE_CONFIG -I"C:/Users/sinan/Desktop/Tiremo/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Framework/CMSIS/Core/Include" -I"C:/Users/sinan/Desktop/Tiremo/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Framework/CMSIS/Device/ABOV/Include" -I"C:/Users/sinan/Desktop/Tiremo/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../ProductConfig/Config" -I"C:/Users/sinan/Desktop/Tiremo/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Platform/HAL/Include" -I"C:/Users/sinan/Desktop/Tiremo/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Platform/HAL/HPL/Include" -I"C:/Users/sinan/Desktop/Tiremo/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Platform/Library/ABOV/Debug/Include" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


