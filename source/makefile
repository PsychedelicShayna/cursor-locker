arc = -m32
stl = c++17
cmp = g++

bin = $(CURDIR)"/../build/x86/mlock.exe"

ifeq ($(arc), -m64)
	bin = $(CURDIR)"/../build/x64/mlock.exe"
endif

# Clang is causing compile errors, therefore g++ is used for now. 
# cmp = "C:\Program Files\LLVM\bin\clang++.EXE"

build: *.o
	# --------------------------------------
	# Building Executable
	# --------------------------------------
	$(cmp) $(arc) *.o -o $(bin) -std=$(stl)

compile:
	# --------------------------------------
	# Compiling All Translation units
	# --------------------------------------
	$(cmp) $(arc) *.cc -c -std=$(stl)

rebuild:
	# --------------------------------------
	# Compiling All Translation units
	# --------------------------------------
	$(cmp) $(arc) *.cc -c -std=$(stl)
	# --------------------------------------
	# Building Executable
	# --------------------------------------
	$(cmp) $(arc) *.o -o $(bin) -std=$(stl)

main.o: main.cc
	# --------------------------------------
	#  Compiling Main.cc
	# --------------------------------------
	$(cmp) $(arc) main.cc -c -std=$(stl)

MouseLocker.o: MouseLocker.cc headers/MouseLocker.hh
	# --------------------------------------
	#  Compiling MouseLocker.cc
	#  --------------------------------------
	$(cmp) $(arc) MouseLocker.cc -c -std=$(stl)

IConditionChecker.o: IConditionChecker.cc headers/IConditionChecker.hh
	# --------------------------------------
	# Compiling IConditionChecker.cc
	# --------------------------------------
	$(cmp) $(arc) IConditionChecker.cc -c -std=$(stl)
	
InputChecker.o: InputChecker.cc headers/InputChecker.hh
	# --------------------------------------
	# Compiling InputChecker.cc
	# --------------------------------------
	$(cmp) $(arc) InputChecker.cc -c -std=$(stl)

ProcessChecker.o: ProcessChecker.cc headers/ProcessChecker.hh
	# --------------------------------------
	# Compiling ProcessChecker.cc
	# --------------------------------------
	$(cmp) $(arc) ProcessChecker.cc -c -std=$(stl)
	
UiCore.o: UiCore.cc headers/UiCore.hh
	# --------------------------------------
	# Compiling UiCore.cc
	# --------------------------------------
	$(cmp) $(arc) UiCore.cc -c -std=$(stl)