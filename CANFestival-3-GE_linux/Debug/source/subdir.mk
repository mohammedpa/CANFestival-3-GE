################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/dcf.c \
../source/emcy.c \
../source/lifegrd.c \
../source/lss.c \
../source/nmtMaster.c \
../source/nmtSlave.c \
../source/objacces.c \
../source/pdo.c \
../source/sdo.c \
../source/states.c \
../source/sync.c \
../source/timer.c \
../source/timers_unix.c \
../source/unix.c 

OBJS += \
./source/dcf.o \
./source/emcy.o \
./source/lifegrd.o \
./source/lss.o \
./source/nmtMaster.o \
./source/nmtSlave.o \
./source/objacces.o \
./source/pdo.o \
./source/sdo.o \
./source/states.o \
./source/sync.o \
./source/timer.o \
./source/timers_unix.o \
./source/unix.o 

C_DEPS += \
./source/dcf.d \
./source/emcy.d \
./source/lifegrd.d \
./source/lss.d \
./source/nmtMaster.d \
./source/nmtSlave.d \
./source/objacces.d \
./source/pdo.d \
./source/sdo.d \
./source/states.d \
./source/sync.d \
./source/timer.d \
./source/timers_unix.d \
./source/unix.d 


# Each subdirectory must supply rules for building sources it contributes
source/%.o: ../source/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -Dinline=static -I../include/canfestival -O0 -g3 -pedantic -Wall -Wextra -Wconversion -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


