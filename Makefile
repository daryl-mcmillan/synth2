CXXFLAGS=-Ofast -DF_CPU=16000000UL -mmcu=atmega328p
PROCESSOR=atmega168
LDFLAGS=-mmcu=$(PROCESSOR)
CXX=avr-gcc
CC=avr-gcc
BAUD=19200
AS=avr-as

PROGNAME=main
COMPORT=/dev/ttyUSB0

$(PROGNAME).hex: $(PROGNAME)
	avr-objcopy -O ihex -R .eeprom $(PROGNAME) $(PROGNAME).hex

upload: $(PROGNAME).hex
	avrdude -v -carduino -p$(PROCESSOR) -P$(COMPORT) -b$(BAUD) -D -Uflash:w:$(PROGNAME).hex:i

serial.o: serial.cc serial.h

main.o: main.cc serial.h

$(PROGNAME): main.o serial.o

clean:
	$(RM) *.hex
	$(RM) *.o
	$(RM) $(PROGNAME)
