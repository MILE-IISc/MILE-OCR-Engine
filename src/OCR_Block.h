#ifndef OCR_BLOCK_H_
#define OCR_BLOCK_H_

#include <fstream>
#include "OCR_Util.h"
#include "OCR_Line.h"
using std::vector;
using std::ofstream;

namespace IISc_KannadaClassifier {

class OCR_Block {
public:
	static const int MIN_LINE_HEIGHT = 20;
	CvRect boundingBox;
	IplImage *img;
	bool invalidBlockSize;
	vector<OCR_Line> lines;
	OCR_Block(IplImage *src, CvRect rect);
	virtual ~OCR_Block();
	//Line Segmentation based on horizontal projection
	void segmentLines();
	void writeUnicodesToFile(string outputFileName);
	void saveSegmentedLines(unsigned int blockNum);
};

}

#endif /* OCR_BLOCK_H_ */
