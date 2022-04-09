print-%  : ; @echo $* = $($*)

app_name 	= DPM_USB_App
app_version = v0.1.3.0 

SOURCES = 	./Src/main.cpp\
			./Src/mySerial.cpp\
			./Src/app_utils.cpp\
			./Src/keyinput.cpp

INCLUDE_PATH +=	./Inc/
INCLUDE_PATH += /usr/include/libusb-1.0/
INC += $(shell find -L $(INCLUDE_PATH) -name '*.h' -exec dirname {} \; | uniq)
INCLUDES += $(INC:%=-I%)

LIBS += -lusb-1.0\
		-lpthread

DEFINES += -D PCAPP -U WAIT_HEARTBEAT -D LOG_IN_FILE

.PHONY: build clean deletelog
build:
	@echo Building app...
	@mkdir -p Out
	@g++ -o Out/$(app_name)_$(app_version) $(SOURCES) $(INCLUDES) $(LIBS) $(DEFINES)
	@cp Out/$(app_name)_$(app_version) $(app_name)_$(app_version)
	@echo Compiled.
clean:
	@echo Cleaning...
	@rm -f Out/*
	@rm -f $(app_name)*
	@rm -f *.log
	@rm -f Log/*
	@echo Done.
deletelog:
	@rm -f *.log
	@rm -f Log/*
	@echo Done.