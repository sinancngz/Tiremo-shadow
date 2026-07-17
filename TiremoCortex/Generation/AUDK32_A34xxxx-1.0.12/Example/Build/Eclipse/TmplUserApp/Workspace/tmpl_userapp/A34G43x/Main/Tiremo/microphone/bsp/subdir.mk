################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
C:/proje2/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Source/TmplUserApp/Tiremo/microphone/bsp/tiremo_mic_bsp.c 

OBJS += \
./Main/Tiremo/microphone/bsp/tiremo_mic_bsp.o 

C_DEPS += \
./Main/Tiremo/microphone/bsp/tiremo_mic_bsp.d 


# Each subdirectory must supply rules for building sources it contributes
Main/Tiremo/microphone/bsp/tiremo_mic_bsp.o: C:/proje2/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Source/TmplUserApp/Tiremo/microphone/bsp/tiremo_mic_bsp.c Main/Tiremo/microphone/bsp/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GNU Arm Cross C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mlittle-endian -Og -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -g3 -DEXTRN_SUBFAMILY_A34G43x -DEXTRN_ABOV_MODULE_CONFIG -I"C:/proje2/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Framework/CMSIS/Core/Include" -I"C:/proje2/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Framework/CMSIS/Device/ABOV/Include" -I"C:/proje2/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../ProductConfig/Config" -I"C:/proje2/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Platform/HAL/Include" -I"C:/proje2/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Platform/HAL/HPL/Include" -I"C:/proje2/TiremoCortex/Generation/AUDK32_A34xxxx-1.0.12/Example/Build/Eclipse/TmplUserApp/Workspace/tmpl_userapp/../../../../../../Platform/Library/ABOV/Debug/Include" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


