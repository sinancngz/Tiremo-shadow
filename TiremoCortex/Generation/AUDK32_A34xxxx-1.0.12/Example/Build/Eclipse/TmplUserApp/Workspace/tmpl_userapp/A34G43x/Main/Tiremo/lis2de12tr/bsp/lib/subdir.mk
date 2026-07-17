################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/proje2/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Source/TmplUserApp/Tiremo/lis2de12tr/bsp/lib/tiremo_lis2de12tr_driver.c \
C:/proje2/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Source/TmplUserApp/Tiremo/lis2de12tr/bsp/lib/tiremo_lis2de12tr_i2c_hal.c 

OBJS += \
./Main/Tiremo/lis2de12tr/bsp/lib/tiremo_lis2de12tr_driver.o \
./Main/Tiremo/lis2de12tr/bsp/lib/tiremo_lis2de12tr_i2c_hal.o 

C_DEPS += \
./Main/Tiremo/lis2de12tr/bsp/lib/tiremo_lis2de12tr_driver.d \
./Main/Tiremo/lis2de12tr/bsp/lib/tiremo_lis2de12tr_i2c_hal.d 


# Each subdirectory must supply rules for building sources it contributes
Main/Tiremo/lis2de12tr/bsp/lib/tiremo_lis2de12tr_driver.o: C:/proje2/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Source/TmplUserApp/Tiremo/lis2de12tr/bsp/lib/tiremo_lis2de12tr_driver.c Main/Tiremo/lis2de12tr/bsp/lib/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU Arm Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mlittle-endian -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -g3 -DEXTRN_SUBFAMILY_A34G43x -DEXTRN_ABOV_MODULE_CONFIG -I"C:/proje2/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Framework/CMSIS/Core/Include" -I"C:/proje2/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Framework/CMSIS/Device/ABOV/Include" -I"C:/proje2/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../ProductConfig/Config" -I"C:/proje2/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Platform/HAL/Include" -I"C:/proje2/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Platform/HAL/HPL/Include" -I"C:/proje2/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Platform/Library/ABOV/Debug/Include" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

Main/Tiremo/lis2de12tr/bsp/lib/tiremo_lis2de12tr_i2c_hal.o: C:/proje2/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Source/TmplUserApp/Tiremo/lis2de12tr/bsp/lib/tiremo_lis2de12tr_i2c_hal.c Main/Tiremo/lis2de12tr/bsp/lib/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU Arm Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mlittle-endian -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -g3 -DEXTRN_SUBFAMILY_A34G43x -DEXTRN_ABOV_MODULE_CONFIG -I"C:/proje2/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Framework/CMSIS/Core/Include" -I"C:/proje2/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Framework/CMSIS/Device/ABOV/Include" -I"C:/proje2/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../ProductConfig/Config" -I"C:/proje2/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Platform/HAL/Include" -I"C:/proje2/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Platform/HAL/HPL/Include" -I"C:/proje2/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Platform/Library/ABOV/Debug/Include" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


