#include "OCR_Util.h"

namespace IISc_KannadaClassifier {

CvMat *avgVectorBase;
CvMat *eigenVectorsBase;
CvMat *avgVectorOttu;
CvMat *eigenVectorsOttu;
CvMat *avgVectorSS;
CvMat *eigenVectorsSS;
svm_model *svmModelBase;
svm_model *svmModelOttu;
svm_model *svmModelSS;

CvMemStorage* storage;

model *lsvmModelBase;
model *lsvmModelOttu;
model *lsvmModelSS;

ofstream outFileLog;

string imageDir;
string segmentedTextBlocksDir;
string segmentedLinesDir;
string segmentedWordsDir;
string segmentedComponentsDir;
string mergedComponentsDir;

unsigned int saveInterimImagesCounter;

void loadMetaData2() {
	storage = cvCreateMemStorage(0);
	cout << "Loading SVM Model file for Base Components... ";
	outFileLog << "\nLoading SVM Model file for Base Components...\n";
	clock_t cBegin = clock();
	lsvmModelBase = load_model("./etc/Kannada/LinearSVM_KanBase_19Aug2010.model");
	if (lsvmModelBase == NULL) {
		cerr << "Can't open SVM Model file for Base Components\n";
		outFileLog << "Can't open SVM Model file for Base Components\n";
		exit(-1);
	}
	clock_t cEnd = clock();
	double timeInSecs = (cEnd - cBegin) / (float) CLOCKS_PER_SEC;
	cout << "Successfully loaded the SVM Model file for Base Components. Time spent in Loading = " << timeInSecs << "secs\n";
	outFileLog << "Successfully loaded the SVM Model file for Base Components. Time spent in Loading = " << timeInSecs << "secs\n";

	cout << "Loading SVM Model file for Ottus... ";
	outFileLog << "\nLoading SVM Model file for Ottus...\n";
	cBegin = clock();
	lsvmModelOttu = load_model("./etc/Kannada/LinearSVM_KanOttu_19Aug2010.model");
	if (lsvmModelOttu == NULL) {
		cerr << "Can't open SVM Model file for Ottus\n";
		outFileLog << "Can't open SVM Model file for Ottus\n";
		exit(-1);
	}
	cEnd = clock();
	timeInSecs = (cEnd - cBegin) / (float) CLOCKS_PER_SEC;
	cout << "Successfully loaded the SVM Model file for Ottus. Time spent in Loading = " << timeInSecs << "secs\n";
	outFileLog << "Successfully loaded the SVM Model file for Ottus. Time spent in Loading = " << timeInSecs << "secs\n";

	cout << "Loading SVM Model file for Special Symbols... ";
	outFileLog << "\nLoading SVM Model file for Special Symbols...\n";
	cBegin = clock();
	lsvmModelSS = load_model("./etc/Kannada/LinearSVM_KanSpecialSymbols_19Aug2010.model");
	if (lsvmModelSS == NULL) {
		cerr << "Can't open SVM Model file for Special Symbols\n";
		outFileLog << "Can't open SVM Model file for Special Symbols\n";
		exit(-1);
	}
	cEnd = clock();
	timeInSecs = (cEnd - cBegin) / (float) CLOCKS_PER_SEC;
	cout << "Successfully loaded the SVM Model file for Special Symbols. Time spent in Loading = " << timeInSecs << "secs\n";
	outFileLog << "Successfully loaded the SVM Model file for Special Symbols. Time spent in Loading = " << timeInSecs << "secs\n";
}

void loadMetaData() {
	storage = cvCreateMemStorage(0);

	cout << "Loading PCA meta data... ";
	outFileLog << "\nLoading PCA meta data...\n";
	clock_t cBegin = clock();
	avgVectorBase = cvCreateMat(1, 1024, CV_32FC1);
	eigenVectorsBase = cvCreateMat(1024, 1024, CV_32FC1);
	avgVectorOttu = cvCreateMat(1, 256, CV_32FC1);
	eigenVectorsOttu = cvCreateMat(256, 256, CV_32FC1);
	avgVectorSS = cvCreateMat(1, 1024, CV_32FC1);
	eigenVectorsSS = cvCreateMat(1024, 1024, CV_32FC1);
	loadPCAMetaData();
	clock_t cEnd = clock();
	double timeInSecs = (cEnd - cBegin) / (float) CLOCKS_PER_SEC;
	cout << "Successfully loaded the PCA meta data. Time spent in Loading = " << timeInSecs << "secs\n";
	outFileLog << "Successfully loaded the PCA meta data. Time spent in Loading = " << timeInSecs << "secs\n";

	cout << "Loading SVM Model file for Base Components... ";
	outFileLog << "\nLoading SVM Model file for Base Components...\n";
	cBegin = clock();
	svmModelBase = svm_load_model("./etc/Kannada/svmBase_11Sep09.model");
	if (svmModelBase == 0) {
		cerr << "Can't open SVM Model file for Base Components\n";
		outFileLog << "Can't open SVM Model file for Base Components\n";
		exit(-1);
	}
	cEnd = clock();
	timeInSecs = (cEnd - cBegin) / (float) CLOCKS_PER_SEC;
	cout << "Successfully loaded the SVM Model file for Base Components. Time spent in Loading = " << timeInSecs << "secs\n";
	outFileLog << "Successfully loaded the SVM Model file for Base Components. Time spent in Loading = " << timeInSecs << "secs\n";

	cout << "Loading SVM Model file for Ottus... ";
	outFileLog << "\nLoading SVM Model file for Ottus...\n";
	cBegin = clock();
	svmModelOttu = svm_load_model("./etc/Kannada/svmOttu_16Sep09.model");
	if (svmModelOttu == 0) {
		cerr << "Can't open SVM Model file for Ottus\n";
		outFileLog << "Can't open SVM Model file for Ottus\n";
		exit(-1);
	}
	cEnd = clock();
	timeInSecs = (cEnd - cBegin) / (float) CLOCKS_PER_SEC;
	cout << "Successfully loaded the SVM Model file for Ottus. Time spent in Loading = " << timeInSecs << "secs\n";
	outFileLog << "Successfully loaded the SVM Model file for Ottus. Time spent in Loading = " << timeInSecs << "secs\n";

	cout << "Loading SVM Model file for Special Symbols... ";
	outFileLog << "\nLoading SVM Model file for Special Symbols...\n";
	cBegin = clock();
	svmModelSS = svm_load_model("./etc/Kannada/svmSS_5july10.model");
	if (svmModelSS == 0) {
		cerr << "Can't open SVM Model file for Special Symbols\n";
		outFileLog << "Can't open SVM Model file for Special Symbols\n";
		exit(-1);
	}
	cEnd = clock();
	timeInSecs = (cEnd - cBegin) / (float) CLOCKS_PER_SEC;
	cout << "Successfully loaded the SVM Model file for Special Symbols. Time spent in Loading = " << timeInSecs << "secs\n";
	outFileLog << "Successfully loaded the SVM Model file for Special Symbols. Time spent in Loading = " << timeInSecs << "secs\n";
}

void deleteMetaData2() {
	cvReleaseMemStorage(&storage);
	free_and_destroy_model(&lsvmModelBase);
	free_and_destroy_model(&lsvmModelOttu);
	free_and_destroy_model(&lsvmModelSS);
}

void deleteMetaData() {
	cvReleaseMemStorage(&storage);
	cvReleaseMat(&avgVectorBase);
	cvReleaseMat(&eigenVectorsBase);
	cvReleaseMat(&avgVectorOttu);
	cvReleaseMat(&eigenVectorsOttu);
	cvReleaseMat(&avgVectorSS);
	cvReleaseMat(&eigenVectorsSS);
	svm_free_and_destroy_model(&svmModelBase);
	svm_free_and_destroy_model(&svmModelOttu);
	svm_free_and_destroy_model(&svmModelSS);
}

void loadPCAMetaData() {
	float *avgDataBase = avgVectorBase->data.fl;
	float *eigenDataBase = eigenVectorsBase->data.fl;
	float *avgDataOttu = avgVectorOttu->data.fl;
	float *eigenDataOttu = eigenVectorsOttu->data.fl;
	float *avgDataSS = avgVectorSS->data.fl;
	float *eigenDataSS = eigenVectorsSS->data.fl;

	float buffer;
	FILE *avgfp_base = fopen("./etc/Kannada/avgVectorBase_11Sep09.txt", "r");
	FILE *eigenfp_base = fopen("./etc/Kannada/eigenVectorsBase_11Sep09.txt", "r");
	FILE *avgfp_vottu = fopen("./etc/Kannada/avgVectorOttu_16Sep09.txt", "r");
	FILE *eigenfp_vottu = fopen("./etc/Kannada/eigenVectorsOttu_16Sep09.txt", "r");
	FILE *avgfp_SS = fopen("./etc/Kannada/avgVectorSS_5july10.txt", "r");
	FILE *eigenfp_SS = fopen("./etc/Kannada/eigenVectorsSS_5july10.txt", "r");

	for (int i = 0; i < 1024; i++) {
		fscanf(avgfp_base, "%f", &buffer);
		*(avgDataBase + i) = buffer;
		for (int j = 0; j < 1024; j++) {
			fscanf(eigenfp_base, "%f", &buffer);
			(eigenDataBase + 1024 * i)[j] = buffer;
		}
	}

	for (int i = 0; i < 256; i++) {
		fscanf(avgfp_vottu, "%f", &buffer);
		*(avgDataOttu + i) = buffer;
		for (int j = 0; j < 256; j++) {
			fscanf(eigenfp_vottu, "%f", &buffer);
			(eigenDataOttu + 256 * i)[j] = buffer;
		}
	}

	for (int i = 0; i < 1024; i++) {
		fscanf(avgfp_SS, "%f", &buffer);
		*(avgDataSS + i) = buffer;
		for (int j = 0; j < 1024; j++) {
			fscanf(eigenfp_SS, "%f", &buffer);
			(eigenDataSS + 1024 * i)[j] = buffer;
		}
	}

	fclose(avgfp_base);
	fclose(eigenfp_base);
	fclose(avgfp_vottu);
	fclose(eigenfp_vottu);
	fclose(avgfp_SS);
	fclose(eigenfp_SS);
}

/*Source: LibTiff Tools - tiffsplit.c*/
#define	copyTiffTag1(tag, v) \
    if (TIFFGetField(tiffInput, tag, &v)) TIFFSetField(tiffOutput, tag, v)
#define	copyTiffTag2(tag, v1, v2) \
    if (TIFFGetField(tiffInput, tag, &v1, &v2)) TIFFSetField(tiffOutput, tag, v1, v2)
#define	copyTiffTag3(tag, v1, v2, v3) \
    if (TIFFGetField(tiffInput, tag, &v1, &v2, &v3)) TIFFSetField(tiffOutput, tag, v1, v2, v3)

int copyTiffImage(TIFF* tiffInput, TIFF* tiffOutput) {
	uint16 bitspersample, samplesperpixel, compression, shortv, *shortav;
	uint32 w, l;
	float floatv;
	char *stringv;
	uint32 longv;

	copyTiffTag1(TIFFTAG_SUBFILETYPE, longv);
	copyTiffTag1(TIFFTAG_TILEWIDTH, w);
	copyTiffTag1(TIFFTAG_TILELENGTH, l);
	copyTiffTag1(TIFFTAG_IMAGEWIDTH, w);
	copyTiffTag1(TIFFTAG_IMAGELENGTH, l);
	copyTiffTag1(TIFFTAG_BITSPERSAMPLE, bitspersample);
	copyTiffTag1(TIFFTAG_SAMPLESPERPIXEL, samplesperpixel);
	copyTiffTag1(TIFFTAG_COMPRESSION, compression);
	if (compression == COMPRESSION_JPEG) {
		uint16 count = 0;
		void *table = NULL;
		if (TIFFGetField(tiffInput, TIFFTAG_JPEGTABLES, &count, &table) && count > 0
				&& table) {
			TIFFSetField(tiffOutput, TIFFTAG_JPEGTABLES, count, table);
		}
	}
	copyTiffTag1(TIFFTAG_PHOTOMETRIC, shortv);
	copyTiffTag1(TIFFTAG_PREDICTOR, shortv);
	copyTiffTag1(TIFFTAG_THRESHHOLDING, shortv);
	copyTiffTag1(TIFFTAG_FILLORDER, shortv);
	copyTiffTag1(TIFFTAG_ORIENTATION, shortv);
	copyTiffTag1(TIFFTAG_MINSAMPLEVALUE, shortv);
	copyTiffTag1(TIFFTAG_MAXSAMPLEVALUE, shortv);
	copyTiffTag1(TIFFTAG_XRESOLUTION, floatv);
	copyTiffTag1(TIFFTAG_YRESOLUTION, floatv);
	copyTiffTag1(TIFFTAG_GROUP3OPTIONS, longv);
	copyTiffTag1(TIFFTAG_GROUP4OPTIONS, longv);
	copyTiffTag1(TIFFTAG_RESOLUTIONUNIT, shortv);
	copyTiffTag1(TIFFTAG_PLANARCONFIG, shortv);
	copyTiffTag1(TIFFTAG_ROWSPERSTRIP, longv);
	copyTiffTag1(TIFFTAG_XPOSITION, floatv);
	copyTiffTag1(TIFFTAG_YPOSITION, floatv);
	copyTiffTag1(TIFFTAG_IMAGEDEPTH, longv);
	copyTiffTag1(TIFFTAG_TILEDEPTH, longv);
	copyTiffTag1(TIFFTAG_SAMPLEFORMAT, shortv);
	copyTiffTag2(TIFFTAG_EXTRASAMPLES, shortv, shortav);
	{
		uint16 *red, *green, *blue;
		copyTiffTag3(TIFFTAG_COLORMAP, red, green, blue);
	}
	{
		uint16 shortv2;
		copyTiffTag2(TIFFTAG_PAGENUMBER, shortv, shortv2);
	}
	copyTiffTag1(TIFFTAG_ARTIST, stringv);
	copyTiffTag1(TIFFTAG_IMAGEDESCRIPTION, stringv);
	copyTiffTag1(TIFFTAG_MAKE, stringv);
	copyTiffTag1(TIFFTAG_MODEL, stringv);
	copyTiffTag1(TIFFTAG_SOFTWARE, stringv);
	copyTiffTag1(TIFFTAG_DATETIME, stringv);
	copyTiffTag1(TIFFTAG_HOSTCOMPUTER, stringv);
	copyTiffTag1(TIFFTAG_PAGENAME, stringv);
	copyTiffTag1(TIFFTAG_DOCUMENTNAME, stringv);
	copyTiffTag1(TIFFTAG_BADFAXLINES, longv);
	copyTiffTag1(TIFFTAG_CLEANFAXDATA, longv);
	copyTiffTag1(TIFFTAG_CONSECUTIVEBADFAXLINES, longv);
	copyTiffTag1(TIFFTAG_FAXRECVPARAMS, longv);
	copyTiffTag1(TIFFTAG_FAXRECVTIME, longv);
	copyTiffTag1(TIFFTAG_FAXSUBADDRESS, stringv);
	copyTiffTag1(TIFFTAG_FAXDCS, stringv);
	if (TIFFIsTiled(tiffInput))
		return (copyTiffTiles(tiffInput, tiffOutput));
	else
		return (copyTiffStrips(tiffInput, tiffOutput));
}

int copyTiffStrips(TIFF* tiffInput, TIFF* tiffOutput) {
	tsize_t bufsize  = TIFFStripSize(tiffInput);
	unsigned char *buf = (unsigned char *)_TIFFmalloc(bufsize);

	if (buf) {
		tstrip_t s, ns = TIFFNumberOfStrips(tiffInput);
		uint32 *bytecounts;

		TIFFGetField(tiffInput, TIFFTAG_STRIPBYTECOUNTS, &bytecounts);
		for (s = 0; s < ns; s++) {
			if (bytecounts[s] > (uint32)bufsize) {
				buf = (unsigned char *)_TIFFrealloc(buf, bytecounts[s]);
				if (!buf)
					return (0);
				bufsize = bytecounts[s];
			}
			if (TIFFReadRawStrip(tiffInput, s, buf, bytecounts[s]) < 0 ||
			    TIFFWriteRawStrip(tiffOutput, s, buf, bytecounts[s]) < 0) {
				_TIFFfree(buf);
				return (0);
			}
		}
		_TIFFfree(buf);
		return (1);
	}
	return (0);
}

int copyTiffTiles(TIFF* tiffInput, TIFF* tiffOutput) {
	tsize_t bufsize = TIFFTileSize(tiffInput);
	unsigned char *buf = (unsigned char *)_TIFFmalloc(bufsize);

	if (buf) {
		ttile_t t, nt = TIFFNumberOfTiles(tiffInput);
		uint32 *bytecounts;

		TIFFGetField(tiffInput, TIFFTAG_TILEBYTECOUNTS, &bytecounts);
		for (t = 0; t < nt; t++) {
			if (bytecounts[t] > (uint32) bufsize) {
				buf = (unsigned char *)_TIFFrealloc(buf, bytecounts[t]);
				if (!buf)
					return (0);
				bufsize = bytecounts[t];
			}
			if (TIFFReadRawTile(tiffInput, t, buf, bytecounts[t]) < 0 ||
			    TIFFWriteRawTile(tiffOutput, t, buf, bytecounts[t]) < 0) {
				_TIFFfree(buf);
				return (0);
			}
		}
		_TIFFfree(buf);
		return (1);
	}
	return (0);
}

IplImage* extractSubImage(IplImage *src, CvRect rect) {
	CvMat dataHdr;
	cvGetSubRect(src, &dataHdr, rect);
	IplImage* imgHdr = new IplImage();
	return cvGetImage(&dataHdr, imgHdr);
}

IplImage* cloneSubImage(IplImage *src, CvRect rect) {
	CvMat dataHdr;
	cvGetSubRect(src, &dataHdr, rect);
	IplImage imgHdr;
	cvGetImage(&dataHdr, &imgHdr);
	return cvCloneImage(&imgHdr);
}

void copyCvRect(const CvRect &src, CvRect &dst) {
	dst.x = src.x;
	dst.y = src.y;
	dst.width = src.width;
	dst.height = src.height;
}

/** \brief
 * Input: An array consisting of a sequence of non-zero values separated by	one or more zero-values.
 * Output: Vectors consisting of the starting and ending indices of the non-zero sequences in input array.
 *
 */
void getNonZeroSequences(vector<int> &arr, vector<int> &seqStart, vector<int> &seqEnd, int zeroValue) {
	bool lookForNonZero = true; // represents looking ahead for the next non-zeroValue element
	for (unsigned int i = 0; i < arr.size(); i++) {
		if (lookForNonZero && arr[i] > zeroValue) {
			seqStart.push_back(i); //start of non-zero sequence, inclusive
			lookForNonZero = false; //represents looking ahead for next zeroValue element
		} else if (!lookForNonZero && arr[i] <= zeroValue) {
			seqEnd.push_back(i - 1); //end of non-zero sequence, inclusive
			lookForNonZero = true;
		}
	}
	//take care of the case where last non-zero sequence ends without a zeroValue following it.
	if (!lookForNonZero) {
		seqEnd.push_back(arr.size() - 1);
	}
}

bool isImageBackgroundWhite(IplImage* img) {
	int whiteCount = 0, blackCount = 0;
	for (int i = 0; i < (img->height) * (img->width); i++) {
		img->imageData[i] < 0 ? whiteCount++ : blackCount++;
	}
	if (whiteCount > blackCount) {
		return true;
	} else {
		return false;
	}
}

void invertImage(IplImage* img) {
	for (int i = 0; i < (img->height) * (img->width); i++) {
		(img->imageData)[i] = (unsigned int) -1 - (unsigned int) (img->imageData)[i];
	}
}

void copyVector(vector<wchar_t> &fromVector, vector<wchar_t> &toVector) {
	for (unsigned j = 0; j < fromVector.size(); j++) {
		toVector.push_back(fromVector[j]);
	}
}

void displayImage(IplImage* img, char* windowLabel) {
	cvNamedWindow(windowLabel);
	cvShowImage(windowLabel, img);
	cvWaitKey();
}

void resizeSpecialSymbol(IplImage* src, IplImage* dst) {
	int newHeight, newWidth;
	float srcAspectRatio = float(src->width) / src->height;
	if (srcAspectRatio <= 1) {
		newHeight = dst->height;
		newWidth = (int) ceil(dst->height * srcAspectRatio);
	} else {
		newWidth = dst->width;
		newHeight = (int) ceil(dst->width / srcAspectRatio);
	}
	IplImage *tempImg = cvCreateImage(cvSize(newWidth, newHeight), src->depth, src->nChannels);
	cvResize(src, tempImg);

	for (int k = 0; k < dst->height * dst->width; k++) {
		(dst->imageData)[k] = 0;
	}
	if (newWidth == dst->width) {
		cvSetImageROI(dst, cvRect(0, (int) ceil((dst->height - newHeight) / 2.0), newWidth, newHeight));
	} else {
		cvSetImageROI(dst, cvRect((int) ceil((dst->width - newWidth) / 2.0), 0, newWidth, newHeight));
	}
	cvCopy(tempImg, dst);
	cvResetImageROI(dst);
	cvReleaseImage(&tempImg);
}

unsigned int wcharToUTF8(wchar_t unicode, unsigned char* ch) {
	union {
		unsigned char unsignedCharValue[2];
		unsigned short int unsignedIntValue;
	} adapter;
	adapter.unsignedCharValue[0] = unicode % 0X100;
	adapter.unsignedCharValue[1] = unicode / 0X100;
	unsigned int length;
	if (adapter.unsignedIntValue < 0x0080) {
		ch[0] = adapter.unsignedCharValue[0];
		length = 1;
	} else if (adapter.unsignedIntValue < 0x0800) {
		ch[0] = 0xc0 | ((adapter.unsignedIntValue) >> 6);
		ch[1] = 0x80 | ((adapter.unsignedIntValue) & 0x3f);
		length = 2;
	} else {
		ch[0] = 0xe0 | ((adapter.unsignedIntValue) >> 12);
		ch[1] = 0x80 | (((adapter.unsignedIntValue) >> 6) & 0x3f);
		ch[2] = 0x80 | ((adapter.unsignedIntValue) & 0x3f);
		length = 3;
	}
	return length;
}

string toString(vector<wchar_t> &unicodes) {
	stringstream ss;
	unsigned char ch[3];
	for (unsigned int i = 0; i < unicodes.size(); i++) {
		unsigned int len = wcharToUTF8(unicodes[i], ch);
		for (unsigned int j = 0; j < len; j++) {
			ss << ch[j];
		}
	}
	string s;
	ss >> s;
	return s;
}

string toString(unsigned int i) {
	stringstream ss;
	ss << i;
	string s;
	ss >> s;
	return s;
}

string getBlockPrefix(unsigned int blockNum) {
	return "B" + toString(blockNum + 1);
}

string getLinePrefix(unsigned int lineNum) {
	return "_L" + toString(lineNum + 1);
}

string getWordPrefix(unsigned int wordNum) {
	return "_W" + toString(wordNum + 1);;
}

string getSegmentedComponentPrefix(unsigned int segmentedComponentNum) {
	return "_C" + toString(segmentedComponentNum + 1);
}

void getImageVector(string imagePath, int resizeWidth, int resizeHeight, CvMat* outputVector,
		bool isSpecialSymbol) {
	IplImage* img = cvLoadImage(imagePath.c_str(), CV_LOAD_IMAGE_UNCHANGED);
	if (!img) {
		cout << "Could not load image file: " << imagePath << endl;
		return;
	}
	IplImage* resizedImg = cvCreateImage(cvSize(resizeWidth, resizeHeight), IPL_DEPTH_8U, 1);
	if (isSpecialSymbol) {
		resizeSpecialSymbol(img, resizedImg);
	} else {
		cvResize(img, resizedImg);
	}
	// output of cvResize is a graylevel image => binarization required
	cvThreshold(resizedImg, resizedImg, 127, 1, CV_THRESH_BINARY);
	for (int i = 0; i < resizeHeight; i++) {
		for (int j = 0; j < resizeWidth; j++) {
			outputVector->data.fl[i * resizeWidth + j] = cvGet2D(resizedImg, i, j).val[0];
			//cout << cvGet2D(resizedImg, i, j).val[0] << " ";
		}
		//cout << endl;
	}
	//cout << endl;
	cvReleaseImage(&resizedImg);
	cvReleaseImage(&img);
	return;
}

void generateTrainingDataForSVM(string samplesListFileName, string outputFileName, int resizeWidth,
		int resizeHeight, bool isSpecialSymbol) {
	ifstream in(samplesListFileName.c_str());
	if (!in) {
		cout << "Cannot open file '" << samplesListFileName << "' for reading.\n";
		return;
	}
	ofstream out(outputFileName.c_str());
	if (!out) {
		cout << "Cannot open file '" << outputFileName << "' for writing.\n";
		return;
	}

	//Each line of the file contains class number and path of the sample separated by a comma.
	//Path might have white spaces too. Hence using strtok() with "," as the token delimiter
	int MAX_LINE_LENGTH = 512;
	char str[MAX_LINE_LENGTH];
	CvMat* outputVector = cvCreateMat(1, resizeWidth * resizeHeight, CV_32FC1);
	while (!in.eof()) {
		in.getline(str, MAX_LINE_LENGTH);
		char *firstToken = strtok(str, ",");
		char *secondToken = strtok(NULL, ",");
		if (firstToken == NULL || secondToken == NULL) {
			continue;
		}
		int classNumber = atoi(firstToken);
		string sampleFilePath(secondToken);
		cout << classNumber << "\t" << sampleFilePath << endl;
		out << classNumber << " ";
		getImageVector(sampleFilePath, resizeWidth, resizeHeight, outputVector, isSpecialSymbol);
		for (int i = 0; i < resizeWidth * resizeHeight; i++) {
			if (outputVector->data.fl[i] != 0) {
				out << " " << i + 1 << ":" << outputVector->data.fl[i];
			}
		}
		out << endl;
	}
	cvReleaseMat(&outputVector);
	in.close();
	out.close();
}

uchar getPixelValue(IplImage *img, int row, int col) {
	return (uchar) *(img->imageData + row * img->widthStep + col);
}

void setPixelValue(IplImage *img, int row, int col, uchar newValue) {
	*(img->imageData + row * img->widthStep + col) = newValue;
}

void binarizeNiBlack(IplImage* src, IplImage* dst, int blockSize) {
	// Caller may pass the same pointer to source and destination images.
	// Hence store the intermediate output in a temporary image.
	IplImage *tempImg = cvCreateImage(cvSize(src->width, src->height), src->depth, src->nChannels);

	int windowSize = (int) floor(blockSize / 2);
	CvScalar mean, stdDeviation;
	for (int i = windowSize; i < src->height - windowSize; i++) {
		for (int j = windowSize; j < src->width - windowSize; j++) {
			cvSetImageROI(src, cvRect(j - windowSize, i - windowSize, 2* windowSize + 1, 2* windowSize + 1));
			cvAvgSdv(src, &mean, &stdDeviation);
			cvResetImageROI(src);
			int localThreshold = (int) (mean.val[0] + NI_BLACK_CONSTANT * stdDeviation.val[0]);
			int newPixelValue = getPixelValue(src, i, j) > localThreshold ? 255 : 0;
			setPixelValue(tempImg, i, j, newPixelValue);
		}
	}

	//Set the outer-boundary pixels to zero
	cvSetImageROI(tempImg, cvRect(0, 0, tempImg->width, windowSize));
	cvSetZero(tempImg);
	cvResetImageROI(tempImg);

	cvSetImageROI(tempImg, cvRect(0, tempImg->height - windowSize, tempImg->width, windowSize));
	cvSetZero(tempImg);
	cvResetImageROI(tempImg);

	cvSetImageROI(tempImg, cvRect(0, windowSize, windowSize, tempImg->height - 2* windowSize ));
	cvSetZero(tempImg);
	cvResetImageROI(tempImg);

	cvSetImageROI(tempImg, cvRect(tempImg->width - windowSize, windowSize, windowSize, tempImg->height - 2*
			windowSize ));
	cvSetZero(tempImg);
	cvResetImageROI(tempImg);

	cvCopy(tempImg, dst);
	cvReleaseImage(&tempImg);
}

int findOtsuThreshold(IplImage *img) {
	int L = 256;
	float hist[L];
	for (int i = 0; i < L; i++) {
		hist[i] = 0.0;
	}
	for (int i = 0; i < img->height; i++) {
		for (int j = 0; j < img->width; j++) {
			hist[getPixelValue(img, i, j)]++;
		}
	}
	float N = 0;
	for (int i = 0; i < L; i++) {
		N += hist[i];
	}
	float totalMean = 0.0;
	for (int i = 0; i < L; i++) {
		hist[i] /= N;
		totalMean += i * hist[i];
	}
	int max_k = 1;
	float maxBetClassVar = 0.0;
	for (int k = 1; k < L - 1; k++) {
		float zerothMoment = 0.0, firstMoment = 0.0;
		for (int i = 0; i < k; i++) {
			zerothMoment += hist[i];
			firstMoment += i * hist[i];
		}
		float betClassVar = pow(totalMean * zerothMoment - firstMoment, 2) / (zerothMoment * (1
				- zerothMoment));
		if (betClassVar > maxBetClassVar) {
			maxBetClassVar = betClassVar;
			max_k = k;
		}
	}
	return max_k;
}

void makeDirectory(string directoryName) {
#if defined(OCR_WIN)
	mkdir(directoryName.c_str());
#else
	mkdir(directoryName.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
#endif
}

string extractFileName(string filePath) {
#if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__BORLANDC__)
	int indx = filePath.rfind("\\");
	if (indx == -1) {
		//Windows users may sometimes specify the relative path using forward slash!
		indx = filePath.rfind("/");
	}
#else
	int indx = filePath.rfind("/");
#endif
	string fileName = filePath.substr(indx + 1, filePath.size() - (indx + 1));
	return fileName;
}

int writeDataToFile(const char *filePath, const unsigned char *data, size_t len) {
	FILE* fp = fopen(filePath, "w+");
	if (fp == NULL) {
		perror("Error writing string to file:");
		return -1;
	}
	int bytesWritten = fwrite(data, 1, len, fp);
	fclose(fp);
	return bytesWritten;
}

}
