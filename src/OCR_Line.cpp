#include "OCR_Line.h"

namespace IISc_KannadaClassifier {

OCR_Line::OCR_Line(IplImage* src, int _lineTop, int _lineBottom) {
	lineTop = _lineTop;
	lineBottom = _lineBottom;
	updateImage(src);
}

OCR_Line::~OCR_Line() {
}

void OCR_Line::updateImage(IplImage* parentImg) {
	img = extractSubImage(parentImg, cvRect(0, lineTop, parentImg->width, lineBottom - lineTop + 1));
}

/** \brief
 * Word Segmentation based on vertical projection.
 * Split words based on zero-gaps in the vertical projection profile.
 */
void OCR_Line::segmentWords() {
	// Find vertical projection profile (or column sum)
	vector<int> col_sum(img->width);
	CvMat col;
	for (int k = 0; k < img->width; k++) {
		cvGetCol(img, &col, k);
		col_sum[k] = cvCountNonZero(&col);
	}

	vector<int> seqStart;
	vector<int> seqEnd;
	getNonZeroSequences(col_sum, seqStart, seqEnd, 1);

	// Split words based on zero-gaps in the vertical projection profile.
	words.push_back(OCR_Word(seqStart[0], baseLine - lineTop, lineBottom - lineTop));
	int wordCount = 0;
	int minWordGap = (int) floor(WORD_GAP_FRACTION * (baseLine - lineTop));
	for (unsigned int j = 1; j < seqStart.size(); j++) {
		if (seqStart[j] - seqEnd[j - 1] - 1 >= minWordGap) {
			words[wordCount].xEnd = seqEnd[j - 1];
			words[wordCount++].storeImg(img);
			words.push_back(OCR_Word(seqStart[j], baseLine - lineTop, lineBottom - lineTop));
		}
	}
	words[wordCount].xEnd = seqEnd[seqStart.size() - 1];
	words[wordCount].storeImg(img);
}

void OCR_Line::saveSegmentedWords(unsigned int blockNum, unsigned int lineNum) {
	if (SAVE_SEGMENTED_COMPONENTS) {
		// Save each word as a separate image under folder segmentedWordsDir
		for (unsigned int w = 0; w < words.size(); w++) {
			string wordName = segmentedWordsDir + getBlockPrefix(blockNum) + getLinePrefix(lineNum)
					+ getWordPrefix(w) + ".tif";
			cvSaveImage(wordName.c_str(), words[w].img);
		}
	}
}

}
