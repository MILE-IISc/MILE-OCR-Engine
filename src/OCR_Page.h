#ifndef OCR_PAGE_H_
#define OCR_PAGE_H_

#include <vector>
#include <string>
#include <sstream>
#include "OCR_Util.h"
#include "OCR_ClassLabel.h"
#include "OCR_Block.h"
using std::vector;
using std::string;
using std::stringstream;
using std::endl;

namespace IISc_KannadaClassifier {

class OCR_Page {
public:
	IplImage *img;
	vector<OCR_Block> blocks;
	bool isBlockSegmented;
	double skewCorrected;
	OCR_Page(IplImage *_img);
	virtual ~OCR_Page();
	void binarize();
	void skewCorrect();
	void rotateImage(double rotationAngle);
	void extractTextBlocks();
	void addTextBlock(CvRect &rect);
	//This function saves the images of blocks, lines, words and components of the input image in the output directory
	void createFoldersForEachLabel();
	void createVerticalBurstImage(IplImage *srcImg, IplImage *dstImg);
	void rotateImage(IplImage *srcImg, IplImage **dstImgPtr, double rotationAngle);
	IplImage* performRunLengthSmoothing(IplImage *srcImg, int rlsa_C_H, int rlsa_C_V, int rlsa_C_ALPHA);
	void saveSegmentedTextBlocks();
	void smoothBinarizedImage(IplImage *srcImg, IplImage *dstImg);
};

}

#endif /* OCR_PAGE_H_ */
