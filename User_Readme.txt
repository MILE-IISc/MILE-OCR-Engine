MILE Lab OCR for Kannada
(c) Medical Intelligence and Language Engineering Laboratory,
Indian Institute of Science, Bangalore, INDIA
Author Prof. A.G. Ramakrishnan
Website http://mile.ee.iisc.ernet.in

Installation Instructions
1) First install OpenCV by double clicking on OpenCV_1.0.exe . Once installed, add the location of OpenCV's bin directory into System Path. (See below for instructions on how to add a directory to the System Path)
2) Install liblinear-1.7 by just extracting the zipped content preferably at C:\liblinear-1.7. Add the folder C:\liblinear-1.7\windows to the System Path.
3) Install libsvm-2.91 by extracting the zipped content preferably at C:\libsvm-2.91. Add the folder C:\libsvm-2.91\windows to the System Path.
4) Install cygwin preferably at C:\cygwin . Add the folder C:\cygwin\bin to the system path.
5) Copy the directory "IISc_MILE_Lab_Kannada_OCR" to your preferred location. See to it that there are no blank spaces anywhere in the address of this location.

Adding a directory to System Path in Windows XP
1) Go to "Control Panel" -> "System"
2) Go the "Advanced" tab and Click on "Environment Variables"
3) A new dialog box will open. Under the "System Variables" container, select the variable "Path". Double click on it to open the "Edit System Variable" dialog box.
4) The "Variable value" text box will contain all the Path variables. Check if your required folder is already present there, else append to it a ";" (semicolon) followed by your diectory path.

Invoking the OCR:
1) Open Windows Command Prompt. (Windows -> Accessories -> Command Prompt)
2) Change directory to the place where OCR is copied. 
3) Execute OCR program by using the command 
"IISc_MILE_Lab_Kannada_OCR_Speed.exe -img <image_path>"
OR
"IISc_MILE_Lab_Kannada_OCR_Speed.exe -dir <path_of_directory_containing_input_images> " 

Note: Instead of "IISc_MILE_Lab_Kannada_OCR_Speed.exe" you may use "IISc_MILE_Lab_Kannada_OCR_Accuracy.exe" which takes more time but might give a compartively higher accuracy.

4) All output text files are stored directly in a folder named Output in the OCR folder and are named with the same name as that of the image. Don't perform OCR on images with same names, in which case, these text files will be replaced.  So after every execution, it is advisable to move the output text files from the output folder to the required directory elsewhere. 
