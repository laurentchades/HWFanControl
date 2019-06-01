SOURCES = HWFanControl.c
OBJECTS = $(SOURCES:%C=%.o)

TARGET = HWFanControl

$(TARGET): $(OBJECTS)
	gcc $^ -o $@ -I/usr/local/include -L/usr/local/lib -lwiringPi -lm  

