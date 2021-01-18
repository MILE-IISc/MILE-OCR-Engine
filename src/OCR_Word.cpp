#include "OCR_Word.h"

namespace IISc_KannadaClassifier {

OCR_Word::OCR_Word(int _xStart, int _relBaseLine, int _lineHeight) {
	xStart = _xStart;
	relBaseLine = _relBaseLine;
	lineHeight = _lineHeight;
	lineMiddle = (int) floor(lineHeight * LINE_MIDDLE_FRACTION);
	refBelowBaseLine = (int) (OTTU_REFERENCE_BELOWBASELINE_FRACTION * relBaseLine);
	if (refBelowBaseLine > lineHeight) {
		refBelowBaseLine = lineHeight;
	}
}

OCR_Word::~OCR_Word() {
}

void OCR_Word::storeImg(IplImage *src) {
	img = extractSubImage(src, cvRect(xStart, 0, xEnd - xStart + 1, src->height));
}

void OCR_Word::segmentComponents() {
	CvScalar color = CV_RGB(255, 255, 255);
	CvSeq* contour = 0;
	IplImage* tempImg1 = cvCloneImage(img);
	IplImage* tempImg2 = cvCreateImage(cvGetSize(img), IPL_DEPTH_8U, 1);
	cvFindContours(tempImg1, storage, &contour, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);
	// Note that cvFindContours() function modifies the source image content.
	// Hence instead of directly sending word.img to cvFindContours(), it's clone is sent.
	for (; contour != 0; contour = contour->h_next) {
		CvRect rect = cvBoundingRect(contour, 0);
		if (rect.width * rect.height < CC_MIN_AREA) {
			//Assume CC corresponds to noise, and hence ignore.
			continue;
		}
		cvZero(tempImg2);
		cvDrawContours(tempImg2, contour, color, color, -1, CV_FILLED, 8);
		segmentedComponents.push_back(OCR_SegmentedComponent(tempImg2, rect));
	}
	cvReleaseImage(&tempImg2);
	cvReleaseImage(&tempImg1);
	//std::sort(segmentedComponents.begin(), segmentedComponents.end(), compareSegmentedComponents);
}

bool OCR_Word::compareSegmentedComponents(OCR_SegmentedComponent c1, OCR_SegmentedComponent c2) {
	CvRect rect1 = c1.boundingBox;
	CvRect rect2 = c2.boundingBox;
	int charGap = 10; //TODO make this a percentage of parentLineHeight
	if (abs(rect1.x - rect2.x + 1) < charGap) {
		return rect1.y < rect2.y;
	}
	return rect1.x < rect2.x;
}

void OCR_Word::combinePartCharacters() {
	//Assumption: CCs are sorted, in increasing order of xStart, before calling this function.
	vector<OCR_SegmentedComponent> newSegmentedComponents;
	for (unsigned int c = 0; c < segmentedComponents.size(); c++) {
		bool isOttu = segmentedComponents[c].isOttu(lineMiddle, refBelowBaseLine) ? true : false;
		bool merged = false;
		unsigned c2 = c + 1;
		int merge_xStart = segmentedComponents[c].boundingBox.x;
		int merge_xEnd = merge_xStart + segmentedComponents[c].boundingBox.width;
		int merge_yStart = segmentedComponents[c].boundingBox.y;
		int merge_yEnd = merge_yStart + segmentedComponents[c].boundingBox.height;
		int merge_xMidValue = (merge_xStart + merge_xEnd) / 2;
		while (c2 < segmentedComponents.size()) {
			int nextCC_xStart = segmentedComponents[c2].boundingBox.x;
			int nextCC_width = segmentedComponents[c2].boundingBox.width;
			int nextCC_xEnd = nextCC_xStart + nextCC_width;
			int nextCC_yStart = segmentedComponents[c2].boundingBox.y;
			int nextCC_height = segmentedComponents[c2].boundingBox.height;
			int nextCC_yEnd = nextCC_yStart + nextCC_height;
			float nextCC_AspectRatio = float(nextCC_width) / nextCC_height;
			bool isNextComponentOttu = segmentedComponents[c2].isOttu(lineMiddle, refBelowBaseLine);

			if (!isOttu && isNextComponentOttu) {
				//Check for padam
				bool isPadam = nextCC_xStart <= merge_xMidValue && nextCC_xEnd < merge_xEnd
						&& nextCC_AspectRatio <= 0.7;
				if (!isPadam) {
					break;
				}
				//current component is a padam and hence merge it.
			}
			if (isOttu && !isNextComponentOttu) {
				//check if the component is a dot inside the parent ottu (like mahaprana tha or ta)
				bool isDotInsideOttu = nextCC_xStart > merge_xStart && nextCC_yStart > merge_yStart
						&& nextCC_xEnd < merge_xEnd && nextCC_yEnd < merge_yEnd;
				if (!isDotInsideOttu) {
					break;
				}
			}

			int overlap = merge_xEnd - nextCC_xStart;
			if (overlap < MIN_OVERLAP_OF_COMPONENTS) {
				break;
			}
			merged = true;
			if (nextCC_xStart < merge_xStart) {
				merge_xStart = nextCC_xStart;
			}
			if (nextCC_yStart < merge_yStart) {
				merge_yStart = nextCC_yStart;
			}
			if (nextCC_xEnd > merge_xEnd) {
				merge_xEnd = nextCC_xEnd;
			}
			if (nextCC_yEnd > merge_yEnd) {
				merge_yEnd = nextCC_yEnd;
			}
			if (isOttu && merge_xEnd - merge_xStart > MAX_WIDTH_OTTUCOMPONENT_FRACTION * relBaseLine) {
				merged = false;
				break;
			}
			c2++;
		}
		if (!merged) {
			newSegmentedComponents.push_back(segmentedComponents[c]);
		} else {
			CvRect mergeRect = cvRect(merge_xStart, merge_yStart, merge_xEnd - merge_xStart, merge_yEnd
					- merge_yStart);
			newSegmentedComponents.push_back(OCR_SegmentedComponent(img, mergeRect, c2 - c + 1));
			c = c2 - 1; //note: 'c' would be incremented in the for loop
		}
	}
	//replace segmentedComponents with newSegmentedComponents
	segmentedComponents.clear();
	for (unsigned int c = 0; c < newSegmentedComponents.size(); c++) {
		segmentedComponents.push_back(newSegmentedComponents[c]);
	}
}

/** \brief
 * This function merges the components(base characters) which has more than one CC like sa, pa etc
 *
 */
void OCR_Word::combinePartCharactersUsingVpp() {
	int wordWidth = img->width;
	int wordHeight = img->height;
	int refBelowBaseLine = (int) (relBaseLine + 0.15 * wordHeight);

	//Take VPP of base character and get the sequence starting and ending point
	CvMat wordAboveBaseLine;
	cvGetRows(img, &wordAboveBaseLine, 0, relBaseLine - 2);
	vector<int> col_sum(wordWidth);
	CvMat col;
	for (int j = 0; j < wordWidth; j++) {
		cvGetCol(&wordAboveBaseLine, &col, j);
		col_sum[j] = cvCountNonZero(&col);
	}
	vector<int> seqStart;
	vector<int> seqEnd;
	getNonZeroSequences(col_sum, seqStart, seqEnd, 0);
	if (seqStart.size() == 0) {
		return;
	}

	unsigned int vppSeqIndex = 0;
	int numCCsWithinCurrentVPP = 1;
	for (unsigned int c = 0; c < segmentedComponents.size(); c++) {
		//Check if the segmented component lies within the vpp and if yes combine
		int baseLocation = 1;
		int cc_xStart = segmentedComponents[c].boundingBox.x;
		int cc_yStart = segmentedComponents[c].boundingBox.y;
		int ccWidth = segmentedComponents[c].boundingBox.width;
		int ccHeight = segmentedComponents[c].boundingBox.height;
		int cc_xEnd = cc_xStart + ccWidth;
		int cc_yEnd = cc_yStart + ccHeight;
		float ccAspectRatio = (float) ccWidth / ccHeight;
		segmentedComponents[c].connectedComponentCount = 1;

		while (vppSeqIndex < seqStart.size() && cc_xStart > seqEnd[vppSeqIndex]) {
			//to go to next vpp or to eliminate small part of ottu coming to base
			vppSeqIndex++;
			numCCsWithinCurrentVPP = 1;
		}
		if (cc_yStart > lineMiddle && cc_yEnd > refBelowBaseLine && c >= 1) {// if true => ottu or padam
			int vppMidValue = (int) ((seqEnd[vppSeqIndex] - seqStart[vppSeqIndex]) / 2.0)
					+ seqStart[vppSeqIndex];
			int prev_cc_width = segmentedComponents[c - 1].boundingBox.width;
			int prev_cc_height = segmentedComponents[c - 1].boundingBox.height;
			int prev_cc_xEnd = segmentedComponents[c - 1].boundingBox.x + prev_cc_width;

			if (seqStart.size() == 1 && prev_cc_width <= FRAC_PUNC * relBaseLine && prev_cc_height <= ceil(
					FRAC_PUNC * relBaseLine)) {
				//current component is bottom part of semicolon and hence combine it with the part above
			} else if (cc_xStart <= vppMidValue && cc_xEnd < prev_cc_xEnd && ccAspectRatio
					<= PADAM_ASPECT_RATIO) {
				//current component is a padam and hence combine it with base component.
			} else if (cc_yStart > relBaseLine) {
				//current component can be ottu
				segmentedComponents[c].classGroup = OTTU_GROUP;
				continue;
			} else {
				//Check the density above baseline and decide if it is ottu or not
				//Calculate the density above baseline
				unsigned int density = 0;
				// VPP above relBaseLine
				CvMat componentAboveBaseLine;
				int offset = (int) floor(lineHeight * 0.05);
				if (relBaseLine - cc_yStart - offset <= 0) {
					//It is ottu
					segmentedComponents[c].classGroup = OTTU_GROUP;
					continue;
				}
				cvGetRows(segmentedComponents[c].img, &componentAboveBaseLine, 0, relBaseLine - cc_yStart
						- offset);
				vector<int> colSum_AboveBaseLine(ccWidth);
				CvMat col;
				for (int j = 0; j < ccWidth; j++) {
					cvGetCol(&componentAboveBaseLine, &col, j);
					colSum_AboveBaseLine[j] = cvCountNonZero(&col);
					density += colSum_AboveBaseLine[j];
				}
				if (density > 2.75 * relBaseLine) {
					//It is not ottu
					segmentedComponents[c].classGroup = BASE_GROUP;
				} else {
					segmentedComponents[c].classGroup = OTTU_GROUP;
					continue;
				}
			}
		}
		//I added this because if the first component of word is ottu then it fails
		//if (cc_yEnd > refBelowBaseLine) {
		//	continue;
		//}
		if (cc_xStart >= seqStart[vppSeqIndex] - 2 && cc_xStart < seqEnd[vppSeqIndex]) {
			if (numCCsWithinCurrentVPP > 1) {
				while (segmentedComponents[c - baseLocation].classGroup == OTTU_GROUP) {
					baseLocation++;
					// need a while loop because sometimes even the ottu is split!
				}
				//I added this since failed left merged ottu gets combined with previous base component because of overlap
				if (ccHeight > 0.75 * lineHeight) {
					//if it is padam don't continue
					continue;
				}
				int prev_cc_xStart = segmentedComponents[c - baseLocation].boundingBox.x;
				int prev_cc_yStart = segmentedComponents[c - baseLocation].boundingBox.y;
				int prev_ccWidth = segmentedComponents[c - baseLocation].boundingBox.width;
				int prev_ccHeight = segmentedComponents[c - baseLocation].boundingBox.height;
				int prev_cc_xEnd = prev_cc_xStart + prev_ccWidth;
				int prev_cc_yEnd = prev_cc_yStart + prev_ccHeight;

				// check if the prev & cur CCs are overlapping by a good margin, if not ignore.
				int overlap = prev_cc_xEnd - cc_xStart;
				//int percentageOverlap = int(overlap * 100.0
				//		/ segmentedComponents[c - baseLocation].boundingBox.width);
				//if (percentageOverlap < MIN_OVERLAP_OF_COMPONENTS) {
				//	continue;
				//}
				if (overlap < MIN_OVERLAP_OF_COMPONENTS) {
					continue;
				}

				//Perform merge
				int new_xStart = prev_cc_xStart < cc_xStart ? prev_cc_xStart : cc_xStart;
				int new_xEnd = prev_cc_xEnd > cc_xEnd ? prev_cc_xEnd : cc_xEnd;
				int new_yStart = prev_cc_yStart < cc_yStart ? prev_cc_yStart : cc_yStart;
				int new_yEnd = prev_cc_yEnd > cc_yEnd ? prev_cc_yEnd : cc_yEnd;
				//if (new_xEnd - new_xStart > 1.2 * MAX_WIDTH_BASECOMPONENT_FRACTION * relBaseLine) {
				//	//wrong merge
				//	continue;
				//}
				segmentedComponents[c - baseLocation].boundingBox.x = new_xStart;
				segmentedComponents[c - baseLocation].boundingBox.width = new_xEnd - new_xStart;
				segmentedComponents[c - baseLocation].boundingBox.y = new_yStart;
				segmentedComponents[c - baseLocation].boundingBox.height = new_yEnd - new_yStart;
				segmentedComponents[c - baseLocation].img = extractSubImage(img, segmentedComponents[c
						- baseLocation].boundingBox);
				segmentedComponents[c - baseLocation].connectedComponentCount = numCCsWithinCurrentVPP;
				segmentedComponents.erase(segmentedComponents.begin() + c);
				c--;
			}
			numCCsWithinCurrentVPP++;
		}
	}
	seqStart.clear(); //free memory
	seqEnd.clear(); //free memory
}

void OCR_Word::splitMergedCharacters() {
	static unsigned int mergedComponentCount = 0;

	vector<OCR_SegmentedComponent> newSegmentedComponents;
	for (unsigned int c = 0; c < segmentedComponents.size(); c++) {
		bool isOttuTowardsLeft, isOttuTowardsRight;
		cvSaveImage("testt.tif", segmentedComponents[c].img);
		if (!segmentedComponents[c].isBaseAndOttuMerged(relBaseLine, lineHeight, isOttuTowardsLeft,
				isOttuTowardsRight)) {
			newSegmentedComponents.push_back(segmentedComponents[c]);
			continue;
		}
		string mergedComponentName = mergedComponentsDir + toString(++mergedComponentCount);
		if (SAVE_SEGMENTED_COMPONENTS) {
			cvSaveImage((mergedComponentName + "_0.tif").c_str(), segmentedComponents[c].img);
		}

		int cc_xStart = segmentedComponents[c].boundingBox.x;
		int cc_yStart = segmentedComponents[c].boundingBox.y;
		int ccWidth = segmentedComponents[c].boundingBox.width;
		//int ccHeight = segmentedComponents[c].boundingBox.height;
		//float ccAspectRatio = (float) ccWidth / ccHeight;
		//int cc_xEnd = cc_xStart + ccWidth;
		//int cc_yEnd = cc_yStart + ccHeight;

		// Make a copy of the image.
		IplImage *originalSCImg = segmentedComponents[c].img;
		IplImage *tempImg = cvCloneImage(originalSCImg);
		CvScalar zeroVal = cvScalarAll(0);

		int lCutPosition;
		if (isOttuTowardsLeft && isOttuTowardsRight) {
			// Assume it must be lla where lOttu comes directly below base-part
			// Apply horizontal cut at the relBaseLine
			int y = relBaseLine - cc_yStart + 1;
			for (int x = 0; x < ccWidth; x++) {
				cvSet2D(tempImg, y, x, zeroVal);
			}
		} else {
			// Find lCutPosition
			// VPP above relBaseLine
			CvMat componentAboveBaseLine;
			int offset = (int) floor(lineHeight * 0.15);
			cvGetRows(segmentedComponents[c].img, &componentAboveBaseLine, 0, relBaseLine - cc_yStart
					- offset);
			vector<int> colSum_AboveBaseLine(ccWidth);
			CvMat col;
			for (int j = 0; j < ccWidth; j++) {
				cvGetCol(&componentAboveBaseLine, &col, j);
				colSum_AboveBaseLine[j] = cvCountNonZero(&col);
			}
			if (isOttuTowardsLeft) {
				// Ottu is on the left side of Base character
				lCutPosition = ccWidth - 2;
				while (lCutPosition >= 0 && colSum_AboveBaseLine[lCutPosition] != 0) {
					lCutPosition--;
				}
				lCutPosition++;
			} else {
				// Ottu is on the right side of Base character
				lCutPosition = 1;
				while (lCutPosition < ccWidth && colSum_AboveBaseLine[lCutPosition] != 0) {
					lCutPosition++;
				}
				lCutPosition--;
			}

			int x, y;
			// Apply vertical cut (at x=lCutPosition from y=0 to relBaseLine-cc_yStart)
			x = lCutPosition;
			for (y = 0; y <= relBaseLine - cc_yStart; y++) {
				cvSet2D(tempImg, y, x, zeroVal);
			}
			y = relBaseLine - cc_yStart;
			if (isOttuTowardsLeft) {
				// Apply horizontal cut at y=relBaseLine-cc_yStart from x=lCutPosition to ccWidth
				for (x = lCutPosition; x < ccWidth; x++) {
					cvSet2D(tempImg, y, x, zeroVal);
				}
			} else {
				// Apply horizontal cut at y=relBaseLine-cc_yStart from x=0 to lCutPosition
				for (x = 0; x < lCutPosition; x++) {
					cvSet2D(tempImg, y, x, zeroVal);
				}
			}

		}
		cvSaveImage("testtLcut.tif", segmentedComponents[c].img);

		// Extract the CC in the image corresponding to ottu.
		IplImage* tempImg1 = cvCloneImage(tempImg);
		CvSeq* contour = 0;
		cvFindContours(tempImg1, storage, &contour, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);
		// Search for the ConnectedComponent corresponding to ottu
		for (; contour != 0; contour = contour->h_next) {
			CvRect rect = cvBoundingRect(contour, 0);
			int contour_yStart = rect.y + cc_yStart;
			int contour_yEnd = contour_yStart + rect.height;
			if (contour_yStart > lineMiddle && contour_yEnd > refBelowBaseLine) {
				//TODO: current contour may correspond to padam; if so ignore it and continue searching for ottu.
				break;
			}
		}
		if (contour != 0) {
			CvScalar color = CV_RGB(255, 255, 255);
			IplImage* tempImg2 = cvCreateImage(cvGetSize(tempImg), IPL_DEPTH_8U, 1);
			cvZero(tempImg2);
			cvDrawContours(tempImg2, contour, color, color, -1, CV_FILLED, 8);
			OCR_SegmentedComponent ottuComponent(tempImg2, cvBoundingRect(contour, 0));
			ottuComponent.boundingBox.x += cc_xStart;
			ottuComponent.boundingBox.y += cc_yStart;

			//subtract ottu (tempImg2) from original image (originalSCImg)
			cvSub(originalSCImg, tempImg2, originalSCImg);
			//extract the remaining component (corresponding to base part)
			CvRect newBaseRect = cvRect(0, 0, 0, relBaseLine - cc_yStart);
			if (isOttuTowardsLeft && isOttuTowardsRight) {
				newBaseRect.x = 0;
				newBaseRect.width = ccWidth;
			} else if (isOttuTowardsLeft) {
				newBaseRect.x = lCutPosition;
				newBaseRect.width = ccWidth - lCutPosition;
			} else {
				newBaseRect.x = 0;
				newBaseRect.width = lCutPosition;
			}
			OCR_SegmentedComponent baseComponent(originalSCImg, newBaseRect);
			baseComponent.boundingBox.x += cc_xStart;
			baseComponent.boundingBox.y += cc_yStart;

			if (isOttuTowardsLeft) {
				// insert ottuComponent followed by baseComponent into newSegmentedComponents
				newSegmentedComponents.push_back(ottuComponent);
				newSegmentedComponents.push_back(baseComponent);
				if (SAVE_SEGMENTED_COMPONENTS) {
					cvSaveImage((mergedComponentName + "_1.tif").c_str(), ottuComponent.img);
					cvSaveImage((mergedComponentName + "_2.tif").c_str(), baseComponent.img);
				}
			} else {
				// insert baseComponent followed by ottuComponent into newSegmentedComponents
				newSegmentedComponents.push_back(baseComponent);
				newSegmentedComponents.push_back(ottuComponent);
				if (SAVE_SEGMENTED_COMPONENTS) {
					cvSaveImage((mergedComponentName + "_1.tif").c_str(), baseComponent.img);
					cvSaveImage((mergedComponentName + "_2.tif").c_str(), ottuComponent.img);
				}
			}
			cvReleaseImage(&tempImg2);
		} else {
			newSegmentedComponents.push_back(segmentedComponents[c]);
		}
		cvReleaseImage(&tempImg1);
		cvReleaseImage(&tempImg);
	}
	//replace segmentedComponents with newSegmentedComponents
	segmentedComponents.clear();
	for (unsigned int c = 0; c < newSegmentedComponents.size(); c++) {
		segmentedComponents.push_back(newSegmentedComponents[c]);
	}
}

void OCR_Word::saveSegmentedComponents(unsigned int blockNum, unsigned int lineNum, unsigned int wordNum) {
	if (SAVE_SEGMENTED_COMPONENTS) {
		// Save each segmented-component as a separate image under folder segmentedComponentsDir
		for (unsigned int c = 0; c < segmentedComponents.size(); c++) {
			string scName = segmentedComponentsDir + getBlockPrefix(blockNum) + getLinePrefix(lineNum)
					+ getWordPrefix(wordNum) + getSegmentedComponentPrefix(c) + ".tif";
			cvSaveImage(scName.c_str(), segmentedComponents[c].img);
		}
	}
}

/** \brief
 * This function groups the segmented components into Aksharas
 *
 */
void OCR_Word::groupIntoAksharas() {
	vector<string> labelNames;
	for (unsigned int c = 0; c < segmentedComponents.size(); c++) {
		labelNames.push_back(segmentedComponents[c].classLabel[0]);
	}
	vector<OCR_Akshara>::iterator currAkshara;
	for (unsigned int i = 0; i < labelNames.size(); i++) {
		OCR_ClassLabel currentLabel = getClassLabel(labelNames[i]);
		if (currentLabel.isValidAksharaStart || currentLabel.isSpecialSymbol) {
			aksharas.push_back(OCR_Akshara(labelNames[i]));
			currAkshara = aksharas.end() - 1;
		} else if (currentLabel.isOttu || currentLabel.isVowelModifier || currentLabel.isYogaVaaha) {
			if (aksharas.size() == 0) {
				// TODO handle error
				continue;
			}
			OCR_ClassLabel previousLabel = getClassLabel(labelNames[i - 1]);
			if (previousLabel.isSpecialSymbol /*||previousLabel.isVowelModifier || previousLabel.isYogaVaaha || previousLabel.isPartCharacter*/) {
				continue;
			}
			currAkshara->pushLabel(labelNames[i]);
		} else if (currentLabel.isArkaaOttu) {
			if (aksharas.size() >= 2) { //safety check
				vector<OCR_Akshara>::iterator prevAkshara = currAkshara - 1;
				prevAkshara->pushLabel(labelNames[i]);
			}
		} else if (currentLabel.isPartCharacter) {
			string combinedLabelName;
			// Replace part characters with complete character as output from automata.
			switch (currentLabel.partCharacterId) {
			case 1: // rruPart
			case 16: // yPart
			case 17: // yiPart
			case 18: // yePart
				if (i <= labelNames.size() - 2) { //safety check
					combinedLabelName = getCombinedCharacter(currentLabel.partCharacterId, labelNames[i + 1]);
					aksharas.push_back(OCR_Akshara(combinedLabelName));
					currAkshara = aksharas.end() - 1;
					i++;
					labelNames[i] = combinedLabelName;
				}
				break;
			case 13: // j1MPart
				if (i >= 1 && i <= labelNames.size() - 2 && aksharas.size() >= 1) {
					combinedLabelName = getCombinedCharacter(labelNames[i - 1], currentLabel.partCharacterId,
							labelNames[i + 1]);
					currAkshara->replaceLastLabel(combinedLabelName);
					i++;
					labelNames[i] = combinedLabelName;
				}
				break;
			case 27: // b1apart
				aksharas.push_back(OCR_Akshara("b1a"));
				currAkshara = aksharas.end() - 1;
				break;
			default:
				if (i >= 1 && aksharas.size() >= 1) {
					combinedLabelName = getCombinedCharacter(labelNames[i - 1], currentLabel.partCharacterId);
					currAkshara->replaceLastLabel(combinedLabelName);
					labelNames[i] = combinedLabelName;
				}
			}
		} else {
			// Log error, and then add current label as a separate akshara.
		}
	}
}

bool OCR_Word::isNumber(string segmentedComponentName) {
	string numbers[] = { "ondu", "eraDu", "muuru", "naalku", "aidu", "aaru", "eeLu", "enTu", "ombattu",
			"one", "two", "three", "four", "five", "six", "seven", "eight", "nine" };
	for (int i = 0; i < 18; i++) {
		if (numbers[i] == segmentedComponentName) {
			return true;
		}
	}
	return false;
}

void OCR_Word::verify() {
	numBetweenWord = false;
	int numSegComps = segmentedComponents.size();
	//check for a number inside a character based word
	for (int i = 1; i < numSegComps; i++) {
		if ((isNumber(segmentedComponents[i].classLabel[0])) && (!isNumber(
				segmentedComponents[i - 1].classLabel[0]))) {
			numBetweenWord = true;
		}
	}
	if (numBetweenWord) {
		for (int i = 0; i < numSegComps; i++) {
			if (segmentedComponents[i].classLabel[0] == "ombattu") {
				// replace ombattu with arkaaOttu
				segmentedComponents[i].classLabel[0] = "arkaaOttu";
			} else if (segmentedComponents[i].classLabel[0] == "enTu") {
				// replace enTu with _la
				segmentedComponents[i].classLabel[0] = "_la";
			} else if (segmentedComponents[i].classLabel[0] == "naalku") {
				// replace naalku with LPart
				segmentedComponents[i].classLabel[0] = "LPart";
			}
		}
		// replace eeLu (occurring at the end of the word) with question-mark
		OCR_SegmentedComponent &lastComponent = segmentedComponents[numSegComps - 1];
		if (lastComponent.classLabel[0] == "eeLu" || lastComponent.classLabel[0] == "seven") {
			lastComponent.classLabel[0] = "question";
		}
	}
}

}
