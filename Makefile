# Compiler and Flags
CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99

# Linker Flags (Include -lm for math functions)
LDFLAGS = -lm

# Source Files
SRC_FILES = src/assg.c src/vector.c src/spheres.c src/color.c

# Header Files
HDR_FILES = src/color.h src/spheres.h src/vector.h

# Milestone 1 Source Files (Exclude color.c)
MS1_SRC_FILES = src/assg.c src/vector.c src/spheres.c src/color.c

# Executables
MS1_EXEC = MS1_assg
MS2_EXEC = MS2_assg
FS_EXEC  = FS_assg

# Default Target
all: $(MS1_EXEC) $(MS2_EXEC) $(FS_EXEC)

# Milestone 1
$(MS1_EXEC): $(MS1_SRC_FILES) $(HDR_FILES)
	$(CC) $(CFLAGS) -DMS1 $(MS1_SRC_FILES) -o $(MS1_EXEC) $(LDFLAGS)

# Milestone 2
$(MS2_EXEC): $(SRC_FILES) $(HDR_FILES)
	$(CC) $(CFLAGS) -DMS2 $(SRC_FILES) -o $(MS2_EXEC) $(LDFLAGS)

# Final Submission
$(FS_EXEC): $(SRC_FILES) $(HDR_FILES)
	$(CC) $(CFLAGS) -DFS $(SRC_FILES) -o $(FS_EXEC) $(LDFLAGS)

# Clean Rule
clean:
	rm -f $(MS1_EXEC) $(MS2_EXEC) $(FS_EXEC) *.o
