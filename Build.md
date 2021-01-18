1. Install OpenCV, LinLinear, LibSVM, LibTIFF
```
$ sudo apt install libopencv-dev
$ sudo apt install liblinear-dev
$ sudo apt install libsvm-dev
$ sudo apt install libtiff5-dev
```

2. Build Kannada OCR
```
$ make
```

3. Run Kannada OCR
```
./KannadaClassifier.exe <input_xml> <output.xml> <output_textfile_path>
```
