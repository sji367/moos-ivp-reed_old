# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.5

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/sji367/moos-ivp/moos-ivp-reed

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/sji367/moos-ivp/moos-ivp-reed

# Include any dependencies generated for this target.
include src/iOS5000/CMakeFiles/iOS5000.dir/depend.make

# Include the progress variables for this target.
include src/iOS5000/CMakeFiles/iOS5000.dir/progress.make

# Include the compile flags for this target's objects.
include src/iOS5000/CMakeFiles/iOS5000.dir/flags.make

src/iOS5000/CMakeFiles/iOS5000.dir/OS5000.cpp.o: src/iOS5000/CMakeFiles/iOS5000.dir/flags.make
src/iOS5000/CMakeFiles/iOS5000.dir/OS5000.cpp.o: src/iOS5000/OS5000.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/sji367/moos-ivp/moos-ivp-reed/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/iOS5000/CMakeFiles/iOS5000.dir/OS5000.cpp.o"
	cd /home/sji367/moos-ivp/moos-ivp-reed/src/iOS5000 && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/iOS5000.dir/OS5000.cpp.o -c /home/sji367/moos-ivp/moos-ivp-reed/src/iOS5000/OS5000.cpp

src/iOS5000/CMakeFiles/iOS5000.dir/OS5000.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/iOS5000.dir/OS5000.cpp.i"
	cd /home/sji367/moos-ivp/moos-ivp-reed/src/iOS5000 && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/sji367/moos-ivp/moos-ivp-reed/src/iOS5000/OS5000.cpp > CMakeFiles/iOS5000.dir/OS5000.cpp.i

src/iOS5000/CMakeFiles/iOS5000.dir/OS5000.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/iOS5000.dir/OS5000.cpp.s"
	cd /home/sji367/moos-ivp/moos-ivp-reed/src/iOS5000 && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/sji367/moos-ivp/moos-ivp-reed/src/iOS5000/OS5000.cpp -o CMakeFiles/iOS5000.dir/OS5000.cpp.s

src/iOS5000/CMakeFiles/iOS5000.dir/OS5000.cpp.o.requires:

.PHONY : src/iOS5000/CMakeFiles/iOS5000.dir/OS5000.cpp.o.requires

src/iOS5000/CMakeFiles/iOS5000.dir/OS5000.cpp.o.provides: src/iOS5000/CMakeFiles/iOS5000.dir/OS5000.cpp.o.requires
	$(MAKE) -f src/iOS5000/CMakeFiles/iOS5000.dir/build.make src/iOS5000/CMakeFiles/iOS5000.dir/OS5000.cpp.o.provides.build
.PHONY : src/iOS5000/CMakeFiles/iOS5000.dir/OS5000.cpp.o.provides

src/iOS5000/CMakeFiles/iOS5000.dir/OS5000.cpp.o.provides.build: src/iOS5000/CMakeFiles/iOS5000.dir/OS5000.cpp.o


src/iOS5000/CMakeFiles/iOS5000.dir/iOS5000Main.cpp.o: src/iOS5000/CMakeFiles/iOS5000.dir/flags.make
src/iOS5000/CMakeFiles/iOS5000.dir/iOS5000Main.cpp.o: src/iOS5000/iOS5000Main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/sji367/moos-ivp/moos-ivp-reed/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/iOS5000/CMakeFiles/iOS5000.dir/iOS5000Main.cpp.o"
	cd /home/sji367/moos-ivp/moos-ivp-reed/src/iOS5000 && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/iOS5000.dir/iOS5000Main.cpp.o -c /home/sji367/moos-ivp/moos-ivp-reed/src/iOS5000/iOS5000Main.cpp

src/iOS5000/CMakeFiles/iOS5000.dir/iOS5000Main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/iOS5000.dir/iOS5000Main.cpp.i"
	cd /home/sji367/moos-ivp/moos-ivp-reed/src/iOS5000 && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/sji367/moos-ivp/moos-ivp-reed/src/iOS5000/iOS5000Main.cpp > CMakeFiles/iOS5000.dir/iOS5000Main.cpp.i

src/iOS5000/CMakeFiles/iOS5000.dir/iOS5000Main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/iOS5000.dir/iOS5000Main.cpp.s"
	cd /home/sji367/moos-ivp/moos-ivp-reed/src/iOS5000 && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/sji367/moos-ivp/moos-ivp-reed/src/iOS5000/iOS5000Main.cpp -o CMakeFiles/iOS5000.dir/iOS5000Main.cpp.s

src/iOS5000/CMakeFiles/iOS5000.dir/iOS5000Main.cpp.o.requires:

.PHONY : src/iOS5000/CMakeFiles/iOS5000.dir/iOS5000Main.cpp.o.requires

src/iOS5000/CMakeFiles/iOS5000.dir/iOS5000Main.cpp.o.provides: src/iOS5000/CMakeFiles/iOS5000.dir/iOS5000Main.cpp.o.requires
	$(MAKE) -f src/iOS5000/CMakeFiles/iOS5000.dir/build.make src/iOS5000/CMakeFiles/iOS5000.dir/iOS5000Main.cpp.o.provides.build
.PHONY : src/iOS5000/CMakeFiles/iOS5000.dir/iOS5000Main.cpp.o.provides

src/iOS5000/CMakeFiles/iOS5000.dir/iOS5000Main.cpp.o.provides.build: src/iOS5000/CMakeFiles/iOS5000.dir/iOS5000Main.cpp.o


# Object files for target iOS5000
iOS5000_OBJECTS = \
"CMakeFiles/iOS5000.dir/OS5000.cpp.o" \
"CMakeFiles/iOS5000.dir/iOS5000Main.cpp.o"

# External object files for target iOS5000
iOS5000_EXTERNAL_OBJECTS =

bin/iOS5000: src/iOS5000/CMakeFiles/iOS5000.dir/OS5000.cpp.o
bin/iOS5000: src/iOS5000/CMakeFiles/iOS5000.dir/iOS5000Main.cpp.o
bin/iOS5000: src/iOS5000/CMakeFiles/iOS5000.dir/build.make
bin/iOS5000: /home/sji367/moos-ivp/MOOS/MOOSCore/lib/libMOOS.a
bin/iOS5000: /home/sji367/moos-ivp/MOOS/MOOSGeodesy/lib/libMOOSGeodesy.so
bin/iOS5000: lib/libanrp_util.a
bin/iOS5000: /home/sji367/moos-ivp/MOOS/MOOSCore/lib/libMOOS.a
bin/iOS5000: src/iOS5000/CMakeFiles/iOS5000.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/sji367/moos-ivp/moos-ivp-reed/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable ../../bin/iOS5000"
	cd /home/sji367/moos-ivp/moos-ivp-reed/src/iOS5000 && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/iOS5000.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/iOS5000/CMakeFiles/iOS5000.dir/build: bin/iOS5000

.PHONY : src/iOS5000/CMakeFiles/iOS5000.dir/build

src/iOS5000/CMakeFiles/iOS5000.dir/requires: src/iOS5000/CMakeFiles/iOS5000.dir/OS5000.cpp.o.requires
src/iOS5000/CMakeFiles/iOS5000.dir/requires: src/iOS5000/CMakeFiles/iOS5000.dir/iOS5000Main.cpp.o.requires

.PHONY : src/iOS5000/CMakeFiles/iOS5000.dir/requires

src/iOS5000/CMakeFiles/iOS5000.dir/clean:
	cd /home/sji367/moos-ivp/moos-ivp-reed/src/iOS5000 && $(CMAKE_COMMAND) -P CMakeFiles/iOS5000.dir/cmake_clean.cmake
.PHONY : src/iOS5000/CMakeFiles/iOS5000.dir/clean

src/iOS5000/CMakeFiles/iOS5000.dir/depend:
	cd /home/sji367/moos-ivp/moos-ivp-reed && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/sji367/moos-ivp/moos-ivp-reed /home/sji367/moos-ivp/moos-ivp-reed/src/iOS5000 /home/sji367/moos-ivp/moos-ivp-reed /home/sji367/moos-ivp/moos-ivp-reed/src/iOS5000 /home/sji367/moos-ivp/moos-ivp-reed/src/iOS5000/CMakeFiles/iOS5000.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/iOS5000/CMakeFiles/iOS5000.dir/depend

