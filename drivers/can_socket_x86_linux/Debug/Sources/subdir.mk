################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Sources/can_socket.c 

OBJS += \
./Sources/can_socket.o 

C_DEPS += \
./Sources/can_socket.d 


# Each subdirectory must supply rules for building sources it contributes
Sources/can_socket.o: ../Sources/can_socket.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I../../../CANFestival-3-GE_linux/include/canfestival -O0 -g3 -pedantic -Wall -Wextra -Wconversion -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"Sources/can_socket.d" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


