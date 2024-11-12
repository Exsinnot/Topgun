CC = gcc # Compiler and flags
CFLAGS = -Wall -Wextra -g
LIBS = -lm -lsqlite3 # Libraries to link

TARGET = db_app # Target application name
SRCS = db_app.c db_helper.c # Source files and object files
OBJS = $(SRCS:.c=.o)

all: $(TARGET) # Default target
$(TARGET): $(OBJS) # Rule to link object/lib files
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

%.o: %.c # Rule to compile .c to .o files
	$(CC) $(CFLAGS) -c $< -o $@

clean: # Clean up build artifacts
	rm -f $(OBJS) $(TARGET)

run: $(TARGET) # Run the application
	./$(TARGET)

.PHONY: all clean run
