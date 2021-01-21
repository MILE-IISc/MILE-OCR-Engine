#include "OCR_Block.h"

namespace IISc_KannadaClassifier {

OCR_Block::OCR_Block(IplImage *src, CvRect rect) {
	if (rect.height * rect.width < 225) {
		cerr << "Block size is too small";
		invalidBlockSize = true;
	} else {
		invalidBlockSize = false;
	}
	copyCvRect(rect, boundingBox);
	img = extractSubImage(src, rect);
}

OCR_Block::~OCR_Block() {
}

/** \brief
 * Line Segmentation based on horizontal projection. Checks the value of each line height, and decides
 * whether the line segmentation was successful or not. Wherever the width is roughly 1.5 times that of
 * average line height, horizontal projection values around the middle rows might reveal whether it is
 * a bigger font line, or two text lines being merged.
 * @return void
 */
void OCR_Block::segmentLines() {
	// find hpp (horizontal projection profile) or row_sum.
	vector<int> row_sum(img->height);
	CvMat row;
	for (int i = 0; i < img->height; i++) {
		cvGetRow(img, &row, i);
		row_sum[i] = cvCountNonZero(&row);
	}

	vector<int> seqStart;
	vector<int> seqEnd;
	getNonZeroSequences(row_sum, seqStart, seqEnd, 1);

	// Calculate average line height
	vector<int> lineHeights;
	for (unsigned int i = 0; i < seqStart.size(); i++) {
		int currLineHeight = seqEnd[i] - seqStart[i] + 1;
		if (currLineHeight < MIN_LINE_HEIGHT) {
			continue; //ignore spurious lines
			// without this a block having a single text line along with spurious dots will not be properly handled.
		}
		lineHeights.push_back(currLineHeight);
	}
	sort(lineHeights.begin(), lineHeights.end());
	// After sorting, last few heights can correspond to merged lines.
	// Hence consider only the mid portion of lineHeights for finding mean.
	int avgLineHeight = 0;
	if (lineHeights.size() == 1) {
		avgLineHeight = lineHeights[0];
	} else {
		int iStart = (int) (lineHeights.size() * 0.10);
		int iEnd = (int) (lineHeights.size() * 0.75);
		for (int i = iStart; i <= iEnd; i++) {
			avgLineHeight += lineHeights[i];
		}
		avgLineHeight = (int) (avgLineHeight / (iEnd - iStart + 1));
	}
	lineHeights.clear(); //free memory

	// Filter out spurious lines (containing only noise => line height < 0.1 * avgLineHeight),
	// merge lines containing ottus with their previous lines, and
	// split merged lines (line height > 1.5 times average line height).
	int mergedLineThreshold;
	for (unsigned int i = 0; i < seqStart.size(); i++) {
		int currLineHeight = seqEnd[i] - seqStart[i] + 1;
		if (lines.size() == 0 || lines.size() == 1) {
			// first or second line could correspond to a header line, hence might be of a bigger font size.
			mergedLineThreshold = 2 * avgLineHeight;
		} else {
			mergedLineThreshold = (int)(1.5 * avgLineHeight);
		}
		if (currLineHeight < 0.5 * avgLineHeight) {
			if (currLineHeight < 0.1 * avgLineHeight) {
				// Assume spurious line, and hence ignore.
				continue;
			}
			int prevLineEnd = lines.size() > 0 ? lines[lines.size() - 1].lineBottom : 0;
			int gapBetweenLines = seqStart[i] - prevLineEnd;
			if (prevLineEnd > 0 && (gapBetweenLines < 0.25 * avgLineHeight)) {
				// Assume current line corresponds to ottus, and hence merge with previous line.
				lines[lines.size() - 1].lineBottom = seqEnd[i];
				lines[lines.size() - 1].updateImage(img);
			} else {
				//TODO: Check if line contains any valid CCs, else ignore.
			}
		} else if (currLineHeight > mergedLineThreshold) {
			// Assume 2 or more lines merged, hence split the block into multiple lines.
			int blockStart = seqStart[i];
			int blockEnd = seqEnd[i];
			while (blockStart < blockEnd) {
				// The next line-break corresponds to minimum value of row_sum in
				// the range (blockStart+0.5*avgHeight) to (blockStart+1.25*avgHeight)
				int rangeStart = (int) (blockStart + 0.5 * avgLineHeight);
				int rangeEnd = (int) (blockStart + 1.25 * avgLineHeight);
				int lineBreak;
				if (rangeEnd < blockEnd) {
					lineBreak = min_element(row_sum.begin() + rangeStart, row_sum.begin() + rangeEnd + 1)
							- row_sum.begin();
					// TODO: Additional check to avoid splitting of bigger font lines.
					// Make the check actual_value_of_minima <= 0.10 times maxima OR use correlation
				} else {
					lineBreak = blockEnd;
				}
				int newLineHeight = lineBreak - blockStart + 1;
				if (newLineHeight < 0.5 * avgLineHeight) {
					// assume it to be a ottu line and merge it back with previous line
					lines[lines.size() - 1].lineBottom = lineBreak;
					lines[lines.size() - 1].updateImage(img);
				}
				else {
					lines.push_back(OCR_Line(img, blockStart, lineBreak + 1));
				}
				blockStart = lineBreak + 1;
			}
		} else {
			lines.push_back(OCR_Line(img, seqStart[i], seqEnd[i] + 1));
		}
	}
	seqStart.clear(); //free memory
	seqEnd.clear(); //free memory

	// Find Base of each line.
	vector<int> baseLine;
	for (unsigned int i = 0; i < lines.size(); i++) {
		// Find position of Base Line for each line.
		int lStart = lines[i].lineTop;
		int lEnd = lines[i].lineBottom;
		int lMid = (int) (lStart + (lEnd - lStart + 1) * 0.5);
		// Peak-1 lies in the top 40% of line height
		int maxIndex1 = max_element(row_sum.begin() + lStart, row_sum.begin() + lMid) - row_sum.begin();
		// Peak-2 lies in the bottom 60% of line height
		int maxIndex2 = max_element(row_sum.begin() + lMid, row_sum.begin() + lEnd) - row_sum.begin();
		int minVal = *min_element(row_sum.begin() + maxIndex1, row_sum.begin() + maxIndex2);
		//int minVal = 0.8 * row_sum[maxIndex2];
		int k = maxIndex2;

		while (k < lEnd) {
			if (row_sum[k] <= minVal) {
				break;
			}
			k++;
		}
		lines[i].baseLine = k + 1;
		// BaseLine has now been so positioned that all ottus come below it.
	}
}

void OCR_Block::saveSegmentedLines(unsigned int blockNum) {
	if (SAVE_SEGMENTED_COMPONENTS) {
		// Save each line as a separate image under folder segmentedLinesDir
		for (unsigned int l = 0; l < lines.size(); l++) {
			string lineName = segmentedLinesDir + getBlockPrefix(blockNum) + getLinePrefix(l) + ".tif";
			cvSaveImage(lineName.c_str(), lines[l].img);
		}
	}
}

ofstream &operator<<(ofstream &out, vector<wchar_t> &unicodes) {
	out << toString(unicodes);
	return out;
}

/** \brief
 * writes the unicode text to file in UTF-8 format
 *
 * @return void
 */
void OCR_Block::writeUnicodesToFile(string outputFileName) {
	ofstream out(outputFileName.c_str());
	for (unsigned int l = 0; l < lines.size(); l++) {
		vector<OCR_Word> &words = lines[l].words;
		for (unsigned int w = 0; w < words.size(); w++) {
			vector<OCR_Akshara> &aksharas = words[w].aksharas;
			for (unsigned int a = 0; a < aksharas.size(); a++) {
				out << aksharas[a].unicodes;
			}
			out << " ";
		}
		out << "\n";
	}
	out.close();
}

}
