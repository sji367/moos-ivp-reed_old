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
CMAKE_SOURCE_DIR = /home/sreed/moos-ivp/moos-ivp-reed

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/sreed/moos-ivp/moos-ivp-reed/build

# Include any dependencies generated for this target.
include src/iGPS_MB1/CMakeFiles/iGPS.dir/depend.make

# Include the progress variables for this target.
include src/iGPS_MB1/CMakeFiles/iGPS.dir/progress.make

# Include the compile flags for this target's objects.
include src/iGPS_MB1/CMakeFiles/iGPS.dir/flags.make

src/iGPS_MB1/CMakeFiles/iGPS.dir/GPS_MB1.cpp.o: src/iGPS_MB1/CMakeFiles/iGPS.dir/flags.make
src/iGPS_MB1/CMakeFiles/iGPS.dir/GPS_MB1.cpp.o: ../src/iGPS_MB1/GPS_MB1.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/sreed/moos-ivp/moos-ivp-reed/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object src/iGPS_MB1/CMakeFiles/iGPS.dir/GPS_MB1.cpp.o"
	cd /home/sreed/moos-ivp/moos-ivp-reed/build/src/iGPS_MB1 && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/iGPS.dir/GPS_MB1.cpp.o -c /home/sreed/moos-ivp/moos-ivp-reed/src/iGPS_MB1/GPS_MB1.cpp

src/iGPS_MB1/CMakeFiles/iGPS.dir/GPS_MB1.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/iGPS.dir/GPS_MB1.cpp.i"
	cd /home/sreed/moos-ivp/moos-ivp-reed/build/src/iGPS_MB1 && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/sreed/moos-ivp/moos-ivp-reed/src/iGPS_MB1/GPS_MB1.cpp > CMakeFiles/iGPS.dir/GPS_MB1.cpp.i

src/iGPS_MB1/CMakeFiles/iGPS.dir/GPS_MB1.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/iGPS.dir/GPS_MB1.cpp.s"
	cd /home/sreed/moos-ivp/moos-ivp-reed/build/src/iGPS_MB1 && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/sreed/moos-ivp/moos-ivp-reed/src/iGPS_MB1/GPS_MB1.cpp -o CMakeFiles/iGPS.dir/GPS_MB1.cpp.s

src/iGPS_MB1/CMakeFiles/iGPS.dir/GPS_MB1.cpp.o.requires:

.PHONY : src/iGPS_MB1/CMakeFiles/iGPS.dir/GPS_MB1.cpp.o.requires

src/iGPS_MB1/CMakeFiles/iGPS.dir/GPS_MB1.cpp.o.provides: src/iGPS_MB1/CMakeFiles/iGPS.dir/GPS_MB1.cpp.o.requires
	$(MAKE) -f src/iGPS_MB1/CMakeFiles/iGPS.dir/build.make src/iGPS_MB1/CMakeFiles/iGPS.dir/GPS_MB1.cpp.o.provides.build
.PHONY : src/iGPS_MB1/CMakeFiles/iGPS.dir/GPS_MB1.cpp.o.provides

src/iGPS_MB1/CMakeFiles/iGPS.dir/GPS_MB1.cpp.o.provides.build: src/iGPS_MB1/CMakeFiles/iGPS.dir/GPS_MB1.cpp.o


src/iGPS_MB1/CMakeFiles/iGPS.dir/iGPSMain.cpp.o: src/iGPS_MB1/CMakeFiles/iGPS.dir/flags.make
src/iGPS_MB1/CMakeFiles/iGPS.dir/iGPSMain.cpp.o: ../src/iGPS_MB1/iGPSMain.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/sreed/moos-ivp/moos-ivp-reed/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object src/iGPS_MB1/CMakeFiles/iGPS.dir/iGPSMain.cpp.o"
	cd /home/sreed/moos-ivp/moos-ivp-reed/build/src/iGPS_MB1 && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/iGPS.dir/iGPSMain.cpp.o -c /home/sreed/moos-ivp/moos-ivp-reed/src/iGPS_MB1/iGPSMain.cpp

src/iGPS_MB1/CMakeFiles/iGPS.dir/iGPSMain.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/iGPS.dir/iGPSMain.cpp.i"
	cd /home/sreed/moos-ivp/moos-ivp-reed/build/src/iGPS_MB1 && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/sreed/moos-ivp/moos-ivp-reed/src/iGPS_MB1/iGPSMain.cpp > CMakeFiles/iGPS.dir/iGPSMain.cpp.i

src/iGPS_MB1/CMakeFiles/iGPS.dir/iGPSMain.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/iGPS.dir/iGPSMain.cpp.s"
	cd /home/sreed/moos-ivp/moos-ivp-reed/build/src/iGPS_MB1 && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/sreed/moos-ivp/moos-ivp-reed/src/iGPS_MB1/iGPSMain.cpp -o CMakeFiles/iGPS.dir/iGPSMain.cpp.s

src/iGPS_MB1/CMakeFiles/iGPS.dir/iGPSMain.cpp.o.requires:

.PHONY : src/iGPS_MB1/CMakeFiles/iGPS.dir/iGPSMain.cpp.o.requires

src/iGPS_MB1/CMakeFiles/iGPS.dir/iGPSMain.cpp.o.provides: src/iGPS_MB1/CMakeFiles/iGPS.dir/iGPSMain.cpp.o.requires
	$(MAKE) -f src/iGPS_MB1/CMakeFiles/iGPS.dir/build.make src/iGPS_MB1/CMakeFiles/iGPS.dir/iGPSMain.cpp.o.provides.build
.PHONY : src/iGPS_MB1/CMakeFiles/iGPS.dir/iGPSMain.cpp.o.provides

src/iGPS_MB1/CMakeFiles/iGPS.dir/iGPSMain.cpp.o.provides.build: src/iGPS_MB1/CMakeFiles/iGPS.dir/iGPSMain.cpp.o


# Object files for target iGPS
iGPS_OBJECTS = \
"CMakeFiles/iGPS.dir/GPS_MB1.cpp.o" \
"CMakeFiles/iGPS.dir/iGPSMain.cpp.o"

# External object files for target iGPS
iGPS_EXTERNAL_OBJECTS =

../bin/iGPS: src/iGPS_MB1/CMakeFiles/iGPS.dir/GPS_MB1.cpp.o
../bin/iGPS: src/iGPS_MB1/CMakeFiles/iGPS.dir/iGPSMain.cpp.o
../bin/iGPS: src/iGPS_MB1/CMakeFiles/iGPS.dir/build.make
../bin/iGPS: /home/sreed/moos-ivp/build/MOOS/MOOSCore/lib/libMOOS.a
../bin/iGPS: /home/sreed/moos-ivp/build/MOOS/MOOSGeodesy/lib/libMOOSGeodesy.so
../bin/iGPS: ../lib/libanrp_util.a
../bin/iGPS: /home/sreed/moos-ivp/build/MOOS/MOOSCore/lib/libMOOS.a
../bin/iGPS: src/iGPS_MB1/CMakeFiles/iGPS.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/sreed/moos-ivp/moos-ivp-reed/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX executable ../../../bin/iGPS"
	cd /home/sreed/moos-ivp/moos-ivp-reed/build/src/iGPS_MB1 && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/iGPS.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
src/iGPS_MB1/CMakeFiles/iGPS.dir/build: ../bin/iGPS

.PHONY : src/iGPS_MB1/CMakeFiles/iGPS.dir/build

src/iGPS_MB1/CMakeFiles/iGPS.dir/requires: src/iGPS_MB1/CMakeFiles/iGPS.dir/GPS_MB1.cpp.o.requires
src/iGPS_MB1/CMakeFiles/iGPS.dir/requires: src/iGPS_MB1/CMakeFiles/iGPS.dir/iGPSMain.cpp.o.requires

.PHONY : src/iGPS_MB1/CMakeFiles/iGPS.dir/requires

src/iGPS_MB1/CMakeFiles/iGPS.dir/clean:
	cd /home/sreed/moos-ivp/moos-ivp-reed/build/src/iGPS_MB1 && $(CMAKE_COMMAND) -P CMakeFiles/iGPS.dir/cmake_clean.cmake
.PHONY : src/iGPS_MB1/CMakeFiles/iGPS.dir/clean

src/iGPS_MB1/CMakeFiles/iGPS.dir/depend:
	cd /home/sreed/moos-ivp/moos-ivp-reed/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/sreed/moos-ivp/moos-ivp-reed /home/sreed/moos-ivp/moos-ivp-reed/src/iGPS_MB1 /home/sreed/moos-ivp/moos-ivp-reed/build /home/sreed/moos-ivp/moos-ivp-reed/build/src/iGPS_MB1 /home/sreed/moos-ivp/moos-ivp-reed/build/src/iGPS_MB1/CMakeFiles/iGPS.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : src/iGPS_MB1/CMakeFiles/iGPS.dir/depend

