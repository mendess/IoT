.PHONY: all compile setup deploy permissions

$(info Testing permissions)
ifeq ($(shell [ -w /dev/ttyACM0 ] || echo 0), 0)
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
	arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:avr:uno .

attach: $(PERMISSIONS)
	screen /dev/ttyACM0 9600

permissions:
	sudo chown :${USER} /dev/ttyACM0
