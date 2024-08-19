# Define the compiler
CC = gcc

# Define compiler flags
CFLAGS = -Wall -Wextra -pedantic -std=c99

# Define the target executable
TARGET = client

# Define the source files
SRCS = client.c helpers.c buffer.c

# Define the object files
OBJS = $(SRCS:.c=.o)

# Default target
all: $(TARGET)

# Rule to build the target executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Rule to build object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean rule
clean:
	rm -f $(TARGET) $(OBJS)

# Phony targets
.PHONY: all clean
