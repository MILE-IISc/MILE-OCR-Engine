1) Download and install OpenCV v2.4 http://sourceforge.net/projects/opencvlibrary/files/opencv-unix/2.4.0/ . Build instructions can be found at http://opencv.willowgarage.com/wiki/InstallGuide.

2) Download and install liblinear: 
a) Download liblinear-1.91.tar.gz from http://www.csie.ntu.edu.tw/~cjlin/liblinear/
b) Extract it to your home folder 
c) Open command prompt and run "cd ~/liblinear-1.9"
d) Run "make lib" to build.
e) Run "ln -s liblinear.so.1 liblinear.so" to create a symbolic link required for building Kannada OCR.

3) Download and install tiff-3.8.2:
a) Download tiff-3.8.2.tar.gz fro  http://dl.maptools.org/dl/libtiff/
b) Extract it to your home folder
c) Open command prompt and run "cd ~/tiff-3.8.2"
d) Run "make " to build.e) Run "sudo make install" to add to usr/lib

3) Download and install libsvm-2.91: 
a) Download and extract libsvm-2.91.zip from "http://www.csie.ntu.edu.tw/~cjlin/libsvm/oldfiles/libsvm-2.91.zip"
b) As the project was build on a older version of svm
c) Extract it to your home folder
d) Open command prompt and run "libsvm-2.91"
c) Run "make lib" to build.
d) Run "libsvm.so libsvm.so.1" to create a symbolic link required for building Kannada OCR.

4) Build Kannada OCR:
a)  make respective directory changes in the MAKE file 
b)  Edit "config_env.sh" and specify the correct value for LIBLINEAR_HOME variable and LIBSVM_HOME (which points to the directory where liblinear is built)
c)  Run ". ./config_env.sh" to update the LD_LIBRARY_PATH to include shared libraries of opencv and liblinear. 
d)   Run "cd ~/KannadaClassifier"
e)   Run "make"
f)   Run "./KannadaClassifier.exe <input_xml> <output.xml> <output_textfile_path>"

