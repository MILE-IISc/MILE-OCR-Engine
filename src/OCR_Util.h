#ifndef OCR_UTIL_H_
#define OCR_UTIL_H_

#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <cstring>
#include <fstream>
#include <opencv/cxcore.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <tiffio.h>
#include <sys/stat.h>
#include <sys/types.h>
using std::cout;
using std::cerr;
using std::endl;
using std::vector;
using std::string;
using std::stringstream;
using std::ifstream;
using std::ofstream;

#include "OCR_GlobalDef.h"
#include <libsvm/svm.h>
#include <linear.h>

namespace IISc_KannadaClassifier {

#if defined(_WIN32) || defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__BORLANDC__)
static const string PATH_SEPARATOR = "\\";
static const string CLEAR_SCREEN_CMD = "cls";
#else
static const string PATH_SEPARATOR = "/";
static const string CLEAR_SCREEN_CMD = "clear";
#endif

extern CvMat *avgVectorBase;
extern CvMat *eigenVectorsBase;
extern CvMat *avgVectorOttu;
extern CvMat *eigenVectorsOttu;
extern CvMat *avgVectorSS;
extern CvMat *eigenVectorsSS;
extern svm_model *svmModelBase;
extern svm_model *svmModelOttu;
extern svm_model *svmModelSS;

extern model *lsvmModelBase;
extern model *lsvmModelOttu;
extern model *lsvmModelSS;

extern CvMemStorage* storage;

extern ofstream outFileLog;

extern string imageDir;
extern string segmentedTextBlocksDir;
extern string segmentedLinesDir;
extern string segmentedWordsDir;
extern string segmentedComponentsDir;
extern string mergedComponentsDir;

extern unsigned int saveInterimImagesCounter;

void loadMetaData();
void deleteMetaData();
void loadPCAMetaData();

void loadMetaData2();
void deleteMetaData2();

int copyTiffImage(TIFF* tiffInput, TIFF* tiffOutput);
int copyTiffStrips(TIFF* tiffInput, TIFF* tiffOutput);
int copyTiffTiles(TIFF* tiffInput, TIFF* tiffOutput);

IplImage* extractSubImage(IplImage *src, CvRect rect);

IplImage* cloneSubImage(IplImage *src, CvRect rect);

void copyCvRect(const CvRect &src, CvRect &dst);

void getNonZeroSequences(vector<int> &arr, vector<int> &seqStart, vector<int> &seqEnd, int zeroValue);

bool isImageBackgroundWhite(IplImage* img);

void invertImage(IplImage* img);

void displayImage(IplImage* img, char* windowLabel);

void copyVector(vector<wchar_t> &fromVector, vector<wchar_t> &toVector);

unsigned int wcharToUTF8(wchar_t unicode, unsigned char* ch);

string toString(vector<wchar_t> &unicodes);

string toString(unsigned int i);

void resizeSpecialSymbol(IplImage* src, IplImage* dst);

void generateTrainingDataForSVM(string samplesListFileName, string outputFileName, int resizeWidth,
		int resizeHeight, bool isSpecialSymbol = false);

void binarizeNiBlack(IplImage* src, IplImage* dst, int blockSize);

int findOtsuThreshold(IplImage *img);

uchar getPixelValue(IplImage *img, int row, int col);
void setPixelValue(IplImage *img, int row, int col, uchar newValue);

string getBlockPrefix(unsigned int blockNum);
string getLinePrefix(unsigned int lineNum);
string getWordPrefix(unsigned int wordNum);
string getSegmentedComponentPrefix(unsigned int segmentedComponentNum);
void makeDirectory(string directoryName);
string extractFileName(string filePath);

}

#endif /* OCR_UTIL_H_ */
