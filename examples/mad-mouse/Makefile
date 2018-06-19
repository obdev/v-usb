TARGET 	:= usbasp

all: src/main.cpp
	platformio run -e $(TARGET)

install: .pioenvs/$(TARGET)/firmware.hex
	platformio run --target upload -e $(TARGET)

clean:
	platformio run --target clean -e $(TARGET)

com:
	platformio device monitor -b 115200

init:
	platformio init --board $(TARGET)

.PHONY: clean
