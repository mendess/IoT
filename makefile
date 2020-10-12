.PHONY: all compile setup deploy permissions

$(info Testing permissions)
ifeq ($(shell [ -w /dev/ttyACM0 ] || echo 0), 0)
	PERMISSIONS=permissions
endif

all: compile deploy

setup:
	arduino-cli core install arduino:avr

compile:
	arduino-cli compile --fqbn arduino:avr:uno .

deploy: $(PERMISSIONS)
	arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:avr:uno .

permissions:
	sudo chown :${USER} /dev/ttyACM0
