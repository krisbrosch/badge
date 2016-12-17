
CC=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy
CFLAGS=-mcpu=cortex-m4 -march=armv7e-m -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 -D__MSP432P401R__ -Dgcc 
INCLUDEFLAGS=-I"../TIinclude" -I"../TIinclude/CMSIS" -I"../driverlib/MSP432P4xx"
DEBUGFLAGS=-g -Wall 
DEPFLAGS=-MMD -MP -MF"$(@:%.o=%.d)"
LINKFLAGS=-Wl,-Map,"badge.map" -Wl,-T"../msp432p401r.lds" -Wl,--start-group -l"nosys" -l"c" -Wl,--end-group

OBJECTS:=$(patsubst %.c,%.o,$(patsubst ../%,%,$(wildcard ../*.c ../driverlib/MSP432P4xx/*.c)))


all: badge.out badge.hex

driverlib:
	mkdir -p driverlib/MSP432P4xx

$(OBJECTS): %.o: ../%.c driverlib
	$(CC) -c $(CFLAGS) $(INCLUDEFLAGS) $(DEBUGFLAGS) $(DEPFLAGS) -o $@ $<

badge.out: $(OBJECTS)
	$(CC) $(CFLAGS) -o badge.out $(OBJECTS) $(LINKFLAGS)

badge.hex: badge.out
	$(OBJCOPY) -O ihex badge.out badge.hex

.PHONY: clean

clean:
	rm -rf *.o *.d driverlib badge.out badge.map badge.hex

-include $(patsubst %.o,%.d,$(OBJECTS))

