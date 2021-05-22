#ifndef OCR_WORD_H_
#define OCR_WORD_H_

#include <vector>
#include <iostream>
#include "OCR_GlobalDef.h"
#include "OCR_Util.h"
#include "OCR_SegmentedComponent.h"
#include "OCR_Akshara.h"
using std::vector;
using std::string;
using std::cout;

namespace IISc_KannadaClassifier {

class OCR_Word {
public:
	int xStart, xEnd;
	IplImage *img;
	int relBaseLine, lineHeight, lineMiddle, refBelowBaseLine ;
	vector<OCR_SegmentedComponent> segmentedComponents;
	vector<OCR_Akshara> aksharas;
	string unicode;
	int language;
	short int specialAttributes; //like isBold, isItalic, isUnderline
	string fontType;
	int fontSize;

	bool numBetweenWord;
	OCR_Word(int _xStart, int _relBaseLine, int _lineHeight);
	virtual ~OCR_Word();

	//Component Segmentation based on Connected Components
	void segmentComponents();

	//This function compares two segmented components and returns a bool
	static bool compareSegmentedComponents(OCR_SegmentedComponent c1, OCR_SegmentedComponent c2);

	//This function merges the components(base characters) which has more than one CC like sa, pa etc
	void combinePartCharacters();
	void combinePartCharactersUsingVpp();
	void splitMergedCharacters();
	void verify();
	//This function groups the components into Aksharas
	void groupIntoAksharas();
	bool isNumber(string segmentedComponentClassName);
	void storeImg(IplImage *src);
	void saveSegmentedComponents(unsigned int blockNum, unsigned int lineNum, unsigned int wordNum);
};

}

#endif /* OCR_WORD_H_ */
