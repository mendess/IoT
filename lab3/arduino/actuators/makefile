.PHONY: all compile setup deploy permissions

ifndef $(SERIAL)
SERIAL=/dev/ttyACM0
endif

$(info Testing permissions)
ifeq ($(shell [ -w $(SERIAL) ] || echo 0), 0)
	PERMISSIONS=permissions
endif

all: compile deploy

setup:
	arduino-cli core install arduino:avr

compile:
	arduino-cli compile --warnings all --fqbn arduino:avr:uno .

debug:
	arduino-cli compile --optimize-for-debug --warnings all --fqbn arduino:avr:uno .

deploy: $(PERMISSIONS)
	arduino-cli upload -p $(SERIAL) --fqbn arduino:avr:uno .

attach: $(PERMISSIONS)
	screen $(SERIAL) 9600

permissions:
	sudo chown :${USER} $(SERIAL)
