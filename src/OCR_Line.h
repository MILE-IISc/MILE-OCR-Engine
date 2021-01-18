#ifndef OCR_LINE_H_
#define OCR_LINE_H_

#include <vector>
#include "OCR_GlobalDef.h"
#include "OCR_Util.h"
#include "OCR_Word.h"

namespace IISc_KannadaClassifier {

class OCR_Line {
public:
	int lineTop, shiroRekha, baseLine, lineBottom;
	IplImage *img;
	vector<OCR_Word> words;
	OCR_Line(IplImage* src, int _lineTop, int _lineBottom);
	virtual ~OCR_Line();
	//Word Segmentation based on vertical projection.
	void segmentWords();
	void updateImage(IplImage* parentImg);
	void saveSegmentedWords(unsigned int blockNum, unsigned int lineNum);
};

}

#endif /* OCR_LINE_H_ */
