# use "g++" to compile source files.
CC = g++
# the linker is also "g++". It might be something else with other compilers.
LD = g++

# Compiler flags go here.
CFLAGS = -g -Wall

# Linker flags go here. Currently there aren't any, but if we'll switch to
# code optimization, we might add "-s" here to strip debug info and symbols.
LDFLAGS = -llinear -lsvm -ltiff -lpthread -lopencv_shape -lopencv_stitching -lopencv_superres -lopencv_videostab -lopencv_aruco -lopencv_bgsegm -lopencv_bioinspired -lopencv_ccalib -lopencv_datasets -lopencv_dpm -lopencv_face -lopencv_freetype -lopencv_fuzzy -lopencv_hdf -lopencv_line_descriptor -lopencv_optflow -lopencv_video -lopencv_plot -lopencv_reg -lopencv_saliency -lopencv_stereo -lopencv_structured_light -lopencv_phase_unwrapping -lopencv_rgbd -lopencv_viz -lopencv_surface_matching -lopencv_text -lopencv_ximgproc -lopencv_calib3d -lopencv_features2d -lopencv_flann -lopencv_xobjdetect -lopencv_objdetect -lopencv_ml -lopencv_xphoto -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_photo -lopencv_imgproc -lopencv_core

# use this command to erase files.
RM = rm -rf
# list of generated object files.
OBJS = base64.o tinyxml2.o OCR_Util.o OCR_ClassLabel.o OCR_Akshara.o OCR_SegmentedComponent.o OCR_Word.o OCR_Line.o OCR_Block.o OCR_Page.o OCR_XML.o KannadaClassifier.o

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
	$(RM) $(PROG) $(OBJS) output
