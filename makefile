
# Constants
PROGRAM=test.exe
OBJ=obj
BIN=bin
INC=inc
SRC=src
RSC=rsc

# C source files
SOURCES = main.c pixel.c screen.c gamestate.c pool.c

# RC resource scripts
RESOURCE_SCRIPTS = resource.rc

# Resources to be copied to the BIN directory
RESOURCES = BG_Beach.bmp Spr_BreadSlice.bmp Spr_Crecent.bmp Spr_Lettuce.bmp Spr_Quadropus.bmp Spr_Tomato.bmp T_BoneSand.bmp T_SesameSand.bmp T_WavySand.bmp Ring05.wav

# Destination paths of objects and resource files
OBJECTS = $(SOURCES:%.c=$(OBJ)\\%.o)
RESOURCE_OBJECTS = $(RESOURCE_SCRIPTS:%.rc=$(OBJ)\\%.o)
COPIED_RESOURCES = $(RESOURCES:%=$(BIN)\\%)

# Allow all warnings
CFLAGS = -Wall -g -O0

# Targets the Windows subsystem--suppresses the console window on startup
LFLAGS = -lgdi32 -lwinmm -Wl,-subsystem,windows

# The main application target
$(BIN)\$(PROGRAM): $(COPIED_RESOURCES) $(RESOURCE_OBJECTS) $(OBJECTS) $(BIN)
	gcc $(CFLAGS) $(OBJECTS) $(RESOURCE_OBJECTS) $(LFLAGS) -o $@

# Build object files from C sources
$(OBJECTS) : $(OBJ)\\%.o : $(SRC)\\%.c $(OBJ)
	gcc -c -I$(INC) $(CFLAGS) $< -o $@

# Build object files from resource scripts
$(RESOURCE_OBJECTS) : $(OBJ)\\%.o : $(RSC)\\%.rc $(OBJ)
	windres $< -o $@

# Copy all resoureces to the BIN directory
$(COPIED_RESOURCES) : $(BIN)\\% : $(RSC)\\% $(BIN)
	copy $< $@

# Create the OBJ directory
$(OBJ) :
	mkdir $(OBJ)

# Create the BIN directory
$(BIN) :
	mkdir $(BIN)

# Delete all objects and binaries
.PHONY: clean
clean :
	-del /Q $(OBJ)
	-rd $(OBJ)
	-del /Q $(BIN)
	-rd $(BIN)
