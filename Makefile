# use "g++" to compile source files.
CC = g++
# the linker is also "g++". It might be something else with other compilers.
LD = g++

# Compiler flags go here.
CFLAGS = -g -Wall

# Linker flags go here. Currently there aren't any, but if we'll switch to
# code optimization, we might add "-s" here to strip debug info and symbols.
LDFLAGS = -lopencv_core -lopencv_highgui -lopencv_contrib -lopencv_features2d -lopencv_imgproc -llinear -lsvm -ltiff -lpthread

# use this command to erase files.
RM = rm -f
# list of generated object files.
OBJS = tinyxml2.o OCR_Util.o OCR_ClassLabel.o OCR_Akshara.o OCR_SegmentedComponent.o OCR_Word.o OCR_Line.o OCR_Block.o OCR_Page.o OCR_XML.o KannadaClassifier.o xmlParser.o

# program executable file name.
PROG = KannadaClassifier.exe

# top-level rule, to compile everything.
all: $(PROG)

# meta-rule to link the program
KannadaClassifier.exe: $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) -o $@

# meta-rule for compiling any "C" source file.
%.o: src/%.cpp
	$(CC) $(CFLAGS) -c $<

# rule for cleaning re-compilable files.
clean:
	$(RM) $(PROG) $(OBJS)
