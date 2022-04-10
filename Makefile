print-%  : ; @echo $* = $($*)

app_name 	= main

SOURCES = 	./main.cpp\
			./keyinput.cpp

INCLUDE_PATH +=	
INC += $(shell find -L $(INCLUDE_PATH) -name '*.h' -exec dirname {} \; | uniq)
INCLUDES += $(INC:%=-I%)
LIBS += 
DEFINES += 

.PHONY: build clean deletelog
build:
	@echo Building app...
	@g++ -o $(app_name).out $(SOURCES) $(INCLUDES) $(LIBS) $(DEFINES)
	@echo Compiled.
clean:
	@echo Cleaning...
	@rm -f $(app_name)*
	@echo Done.