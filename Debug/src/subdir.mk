################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/barcode_generate.cpp \
../src/fastq_transpose.cpp \
../src/main.cpp 

C_SRCS += \
../src/utils.c 

CPP_DEPS += \
./src/barcode_generate.d \
./src/fastq_transpose.d \
./src/main.d 

C_DEPS += \
./src/utils.d 

OBJS += \
./src/barcode_generate.o \
./src/fastq_transpose.o \
./src/main.o \
./src/utils.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.cpp src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/barcode_generate.d ./src/barcode_generate.o ./src/fastq_transpose.d ./src/fastq_transpose.o ./src/main.d ./src/main.o ./src/utils.d ./src/utils.o

.PHONY: clean-src

