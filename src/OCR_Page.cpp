#include "OCR_Page.h"

namespace IISc_KannadaClassifier {

OCR_Page::OCR_Page(IplImage *_img) {
	img = _img;
	skewCorrected = 0.0;
}

OCR_Page::~OCR_Page() {
}

void OCR_Page::binarize() {
	if (SAVE_INTERIM_IMAGES) {
		cvSaveImage(string(imageDir + PATH_SEPARATOR + toString(saveInterimImagesCounter++)
				+ "_OriginalImage.tif").c_str(), img);
	}

	cout << "Performing Binarization... ";
	outFileLog << "\nPerforming Binarization...\n";
	clock_t cBegin = clock();
	cvThreshold(img, img, findOtsuThreshold(img), 255, CV_THRESH_BINARY);
	//cvAdaptiveThreshold(img, img, 255, CV_ADAPTIVE_THRESH_MEAN_C, CV_THRESH_BINARY, 25, 30);
	//binarizeNiBlack(img, img, 25);
	if (isImageBackgroundWhite(img)) {
		invertImage(img);
	}
	clock_t cEnd = clock();
	double timeInSecs = (cEnd - cBegin) / (float) CLOCKS_PER_SEC;
	cout << "Binarization complete. Time spent = " << timeInSecs << "secs\n";
	outFileLog << "Binarization complete. Time spent = " << timeInSecs << "secs\n";
	if (SAVE_INTERIM_IMAGES) {
		cvSaveImage(string(imageDir + PATH_SEPARATOR + toString(saveInterimImagesCounter++)
				+ "_BinarizedImage.tif").c_str(), img);
	}

	cout << "Eliminating Spurious Connected Components... ";
	outFileLog << "\nEliminating Spurious Connected Components...\n";
	cBegin = clock();
	smoothBinarizedImage(img, img);
	cEnd = clock();
	timeInSecs = (cEnd - cBegin) / (float) CLOCKS_PER_SEC;
	cout << "Eliminating Spurious Connected Components complete. Time spent = " << timeInSecs << "secs\n";
	outFileLog << "Eliminating Spurious Connected Components complete. Time spent = " << timeInSecs << "secs\n";
	if (SAVE_INTERIM_IMAGES) {
		cvSaveImage(string(imageDir + PATH_SEPARATOR + toString(saveInterimImagesCounter++)
				+ "_SmoothedImage.tif").c_str(), img);
	}
}

void OCR_Page::smoothBinarizedImage(IplImage *srcImg, IplImage *dstImg) {
	IplImage* outputImg = cvCloneImage(srcImg);
	IplImage* tempImg = cvCloneImage(srcImg);

	// First delete all CCs whose size is utmost NOISE_MAX_AREA
	CvSeq* contour = 0;
	cvFindContours(tempImg, storage, &contour, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);
	for (; contour != 0; contour = contour->h_next) {
		CvRect rect = cvBoundingRect(contour, 0);
		if (rect.width * rect.height <= NOISE_MAX_AREA) {
			// Assume CC corresponds to noise, and hence delete it from outputImg.
			cvSetImageROI(outputImg, rect);
			cvZero(outputImg);
			cvResetImageROI(outputImg);
		}
	}

	// Perform a morphological-close operation (dilation followed by erosion).
	// This merges broken parts of characters as well as dots, apostrophes & other valid components inside a word.
	cvCopy(outputImg, tempImg);
	IplConvKernel* structuringElement = cvCreateStructuringElementEx(11, 11, 5, 5, CV_SHAPE_RECT);
	cvDilate(tempImg, tempImg, structuringElement);
	cvErode(tempImg, tempImg, structuringElement);
	cvReleaseStructuringElement(&structuringElement);

	// On the morphologically-closed image, delete all CCs whose size is less than CC_MIN_AREA
	cvFindContours(tempImg, storage, &contour, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE);
	for (; contour != 0; contour = contour->h_next) {
		CvRect rect = cvBoundingRect(contour, 0);
		if (rect.width * rect.height < CC_MIN_AREA) {
			// Assume CC corresponds to noise, and hence delete it from outputImg.
			cvSetImageROI(outputImg, rect);
			cvZero(outputImg);
			cvResetImageROI(outputImg);
		}
	}
	cvReleaseImage(&tempImg);

	cvCopy(outputImg, dstImg);
	cvReleaseImage(&outputImg);
}

void OCR_Page::createVerticalBurstImage(IplImage *srcImg, IplImage *dstImg) {
	cvSetZero(dstImg);
	for (int j = 0; j < dstImg->width; j++) {
		bool isWhiteRun = getPixelValue(srcImg, 0, j) > 0 ? true : false;
		int whitePixelCount = isWhiteRun ? 1 : 0;
		for (int i = 1; i < dstImg->height; i++) {
			uchar curPixelValue = getPixelValue(srcImg, i, j);
			if (isWhiteRun) {
				if (curPixelValue > 0) {
					whitePixelCount++;
					continue;
				} else {
					//encountered a black pixel
					setPixelValue(dstImg, i - 1, j, whitePixelCount);
					whitePixelCount = 0;
					isWhiteRun = false;
				}
			} else {
				if (curPixelValue == 0) {
					continue;
				} else {
					whitePixelCount++;
					isWhiteRun = true;
				}
			}
		}
		if (isWhiteRun) {
			setPixelValue(dstImg, dstImg->height - 1, j, whitePixelCount);
		}
	}
}

void OCR_Page::rotateImage(IplImage *srcImg, IplImage **dstImgPtr,
		double rotationAngle) {
	//padding zeros on the four sides to avoid loss of data during resize
	double theta = fabs(rotationAngle * CV_PI / 180);
	int rotatedWidth = (int) ceil(srcImg->width * cos(theta) + srcImg->height
			* sin(theta));
	int rotatedHeight = (int) ceil(srcImg->width * sin(theta) + srcImg->height
			* cos(theta));
	int zeroPaddedImgWidth = srcImg->width;
	int zeroPaddedImgHeight = srcImg->height;
	int newX = 0, newY = 0;
	if (rotatedWidth > srcImg->width) {
		zeroPaddedImgWidth = rotatedWidth;
		newX = (int) ((rotatedWidth - srcImg->width) / 2);
	}
	if (rotatedHeight > srcImg->height) {
		zeroPaddedImgHeight = rotatedHeight;
		newY = (int) ((rotatedHeight - srcImg->height) / 2);
	}
	IplImage *zeropaddedSrcImg = cvCreateImage(cvSize(zeroPaddedImgWidth,
			zeroPaddedImgHeight), srcImg->depth, srcImg->nChannels);
	cvSetZero(zeropaddedSrcImg);
	CvRect imgRect = cvRect(newX, newY, srcImg->width, srcImg->height);
	cvSetImageROI(zeropaddedSrcImg, imgRect);
	cvCopy(srcImg, zeropaddedSrcImg);
	cvResetImageROI(zeropaddedSrcImg);

	//The dstImg passed by the caller may be of different dimension. So its memory is released
	//and new memory allocation is done based on the new rotated image dimensions.
	cvReleaseImage(dstImgPtr);
	*dstImgPtr = cvCreateImage(cvGetSize(zeropaddedSrcImg),
			zeropaddedSrcImg->depth, zeropaddedSrcImg->nChannels);
	cvSetZero(*dstImgPtr);
	CvMat *translationMatrix = cvCreateMat(2, 3, CV_32FC1);
	cvSetZero(translationMatrix);
	CvPoint2D32f center;
	center.x = zeropaddedSrcImg->width / 2;
	center.y = zeropaddedSrcImg->height / 2;
	cv2DRotationMatrix(center, rotationAngle, 1.0, translationMatrix);
	cvWarpAffine(zeropaddedSrcImg, *dstImgPtr, translationMatrix,
			CV_INTER_LINEAR);
	cvReleaseImage(&zeropaddedSrcImg);
	cvReleaseMat(&translationMatrix);
}

void OCR_Page::skewCorrect() {
	//Reference: A DOCUMENT SKEW DETECTION METHOD USING RUN-LENGTH ENCODING AND THE HOUGH TRANSFORM -
	// Stuart C. Hinds, James L. Fisher, and Donald P. D'Amato
	cout << "Performing Skew Detection... ";
	outFileLog << "\nPerforming Skew Detection...\n";
	clock_t cBegin = clock();

	IplImage *resizedImg = cvCreateImage(cvSize((int) floor(img->width / 4), (int) floor(img->height / 4)),
			img->depth, img->nChannels);
	cvResize(img, resizedImg, CV_INTER_AREA);
	cvThreshold(resizedImg, resizedImg, 127, 255, CV_THRESH_BINARY);
	IplImage *burstImg = cvCreateImage(cvSize(resizedImg->width, resizedImg->height), 8, 1);
	createVerticalBurstImage(resizedImg, burstImg);

	// Apply Hough Transform
	// rho = x * cos(theta) + y * sin(theta)
	// where origin is at top-left corner, x is along vertical direction and y is along horizontal direction of image
	// theta (== skew-angle for the above co-ordinate space) is measured in anticlockwise direction.

	//Calculate dimensions for accumulator-array
	int maxRows = (int) ceil(sqrt(pow(burstImg->width, 2.0) * pow(burstImg->height, 2.0)));
	int maxSkew = (int) (ceil(MAX_SKEW_ANGLE / SKEW_ANGLE_STEP_SIZE) * SKEW_ANGLE_STEP_SIZE);
	int maxCols = (int) (2 * maxSkew / SKEW_ANGLE_STEP_SIZE + 1);
	CvMat* accumulatorArray = cvCreateMat(maxRows, maxCols, CV_32FC1);

	// Mapping of accumulatorArray's column-index to skew-angle:
	// accumulatorArray[0 to (maxSkew/SKEW_ANGLE_STEP_SIZE) - 1] corresponds to skew-angles from -MAX_SKEW_ANGLE to -SKEW_ANGLE_STEP_SIZE
	// accumulatorArray[maxSkew/SKEW_ANGLE_STEP_SIZE] corresponds to skew-angle of 0
	// accumulatorArray[(maxSkew/SKEW_ANGLE_STEP_SIZE)+1 to 2*(maxSkew/SKEW_ANGLE_STEP_SIZE)] corresponds to skew-angles from +SKEW_ANGLE_STEP_SIZE to +MAX_SKEW_ANGLE

	cvSetZero(accumulatorArray);
	for (int x = 0; x < burstImg->height; x++) {
		for (int y = 0; y < burstImg->width; y++) {
			uchar curPixelValue = getPixelValue(burstImg, x, y);
			if (curPixelValue > 0 && curPixelValue <= 25) { //large run lengths might correspond to figures and black margins and hence ignored
				for (int k = 0; k < maxCols; k++) {
					// map k to skew-angle
					double theta = -maxSkew + k * SKEW_ANGLE_STEP_SIZE;
					//convert it to radians
					theta *= MATH_PI / 180;
					int rho = abs((int) floor((x + 1) * cos(theta) + (y + 1) * sin(theta)));
					if (rho >= maxRows) { //safety check
						rho = maxRows - 1;
					}
					cvmSet(accumulatorArray, rho, k, cvmGet(accumulatorArray, rho, k) + curPixelValue);
				}
			}
		}
	}

	double * sumOfAbsoluteDifferences = new double[maxCols];
	for (int k = 0; k < maxCols; k++) {
		sumOfAbsoluteDifferences[k] = 0;
		for (int rho = 0; rho < maxRows - 1; rho++) {
			sumOfAbsoluteDifferences[k] += fabs(cvmGet(accumulatorArray, rho, k)
					- cvmGet(accumulatorArray, rho + 1, k));
		}
	}
	int maxSumOfAbsoluteDifferences = (int) (maxSkew / SKEW_ANGLE_STEP_SIZE); //no skew
	for (int k = 0; k < maxCols; k++) {
		if (sumOfAbsoluteDifferences[k] > sumOfAbsoluteDifferences[maxSumOfAbsoluteDifferences]) {
			maxSumOfAbsoluteDifferences = k;
		}
	}
	bool SAVE_SKEWDETECTION_ACCUMULATOR_ARRAY = false;
	if (SAVE_SKEWDETECTION_ACCUMULATOR_ARRAY) {
		ofstream out(string(imageDir + PATH_SEPARATOR + "SkewDetectionAccumulatorArray.csv").c_str());
		for (int k = 0; k < maxCols; k++) {
			// map k to skew-angle
			double theta = -maxSkew + k * SKEW_ANGLE_STEP_SIZE;
			out << theta << ",";
		}
		out << endl;
		for (int rho = 0; rho < maxRows; rho++) {
			for (int k = 0; k < maxCols; k++) {
				int curValue = cvmGet(accumulatorArray, rho, k);
				out << curValue << ",";
			}
			out << endl;
		}
		for (int k = 0; k < maxCols; k++) {
			out << sumOfAbsoluteDifferences[k] << ",";
		}
		out << endl;
		out.close();
	}

	//double minVal, maxVal;
	//CvPoint minLoc, maxLoc;
	//cvMinMaxLoc(accumulatorArray, &minVal, &maxVal, &minLoc, &maxLoc);
	// maxLoc.x (which is a column no.) corresponds to maximum skew-angle
	// map accumulatorArray's column no. to skew-angle
	double skew = -maxSkew + maxSumOfAbsoluteDifferences* SKEW_ANGLE_STEP_SIZE;
	cout << "Skew Angle detected = " << skew << " degree. ";
	outFileLog << "Skew Angle detected = " << skew << " degree. " << endl;

	cvReleaseImage(&resizedImg);
	cvReleaseImage(&burstImg);
	cvReleaseMat(&accumulatorArray);

	clock_t cEnd = clock();
	double timeInSecs = (cEnd - cBegin) / (float) CLOCKS_PER_SEC;
	cout << "Skew Detection complete. Time spent = " << timeInSecs << "secs\n";
	outFileLog << "Skew Detection complete. Time spent = " << timeInSecs << "secs\n";

	if (fabs(skew) > fabs(SKEW_ANGLE_TO_IGNORE)) {
		// Skew Correction based on Affine Transform
		skewCorrected = skew;
		cout << "Performing Skew Correction... ";
		outFileLog << "\nPerforming Skew Correction...\n";
		cBegin = clock();
		rotateImage(img, &img, -skew);
		cvThreshold(img, img, 127, 255, CV_THRESH_BINARY);
		cEnd = clock();
		timeInSecs = (cEnd - cBegin) / (float) CLOCKS_PER_SEC;
		cout << "Skew Correction complete. Time spent = " << timeInSecs << "secs\n";
		outFileLog << "Skew Correction complete. Time spent = " << timeInSecs << "secs\n";
		if (SAVE_INTERIM_IMAGES) {
			cvSaveImage(string(imageDir + PATH_SEPARATOR + toString(saveInterimImagesCounter++)
					+ "_SkewCorrectedImage.tif").c_str(), img);
		}
	}
}

IplImage* OCR_Page::performRunLengthSmoothing(IplImage *srcImg, int rlsa_C_H, int rlsa_C_V, int rlsa_C_ALPHA) {
	// The improvement of four-step run-length smoothing algorithm into a three-step method proposed in
	// "Adaptive Document Block Segmentation and Classification - Frank Y. Shih and Shy-Shyan Chen" is used.
	IplImage *dstImg = cvCloneImage(srcImg);

	// Step-1 of the 3-step process:
	// Apply vertical smoothing to the original document image by a threshold rlsa_C_V
	for (int j = 0; j < dstImg->width; j++) {
		bool isBlackRun = getPixelValue(srcImg, 0, j) == 0 ? true : false;
		int blackRunStart = 0;
		for (int i = 1; i < dstImg->height; i++) {
			uchar curPixelValue = getPixelValue(srcImg, i, j);
			if (isBlackRun && curPixelValue > 0) {
				if (i - blackRunStart <= rlsa_C_V) {
					if (blackRunStart != 0) { //ignore black-run at the beginning of the column
						for (int k = blackRunStart; k < i; k++) {
							setPixelValue(dstImg, k, j, 255);
						}
					}
				}
				isBlackRun = false;
			} else if (!isBlackRun && curPixelValue == 0) {
				isBlackRun = true;
				blackRunStart = i;
			}
		}
	}

	// Step-2 of the 3-step process:
	// If the run length of 0's in the horizontal direction of the original	image is greater than rlsa_C_H, then
	// the corresponding pixels in the output of step 1 are set 0's; otherwise, they remain unchanged;
	for (int i = 0; i < dstImg->height; i++) {
		bool isBlackRun = getPixelValue(srcImg, i, 0) == 0 ? true : false;
		int blackRunStart = 0;
		for (int j = 1; j < dstImg->width; j++) {
			uchar curPixelValue = getPixelValue(srcImg, i, j);
			if (isBlackRun && curPixelValue > 0) {
				if (j - blackRunStart > rlsa_C_H || blackRunStart == 0) {
					for (int k = blackRunStart; k < j; k++) {
						setPixelValue(dstImg, i, k, 0);
					}
				}
				isBlackRun = false;
			} else if (!isBlackRun && curPixelValue == 0) {
				isBlackRun = true;
				blackRunStart = j;
			}
		}
		if (isBlackRun) {
			for (int k = blackRunStart; k < dstImg->width; k++) {
				setPixelValue(dstImg, i, k, 0);
			}
		}
	}

	// Step-3 of the 3-step process:
	// An additional horizontal smoothing is applied to the output of step 2 by a relatively small threshold rlsa_C_ALPHA
	for (int i = 0; i < dstImg->height; i++) {
		bool isBlackRun = getPixelValue(dstImg, i, 0) == 0 ? true : false;
		int blackRunStart = 0;
		for (int j = 1; j < dstImg->width; j++) {
			uchar curPixelValue = getPixelValue(dstImg, i, j);
			if (isBlackRun && curPixelValue > 0) {
				if (j - blackRunStart <= rlsa_C_ALPHA) {
					if (blackRunStart != 0) { //ignore black-run at the beginning of the row
						for (int k = blackRunStart; k < j; k++) {
							setPixelValue(dstImg, i, k, 255);
						}
					}
				}
				isBlackRun = false;
			} else if (!isBlackRun && curPixelValue == 0) {
				isBlackRun = true;
				blackRunStart = j;
			}
		}
	}

	if (SAVE_INTERIM_IMAGES) {
		cvSaveImage(string(imageDir + PATH_SEPARATOR + toString(saveInterimImagesCounter++)
				+ "_RLSA_OutputImage.tif").c_str(), dstImg);
	}
	return dstImg;
}

void OCR_Page::extractTextBlocks() {
	// Algorithm: "Block Segmentation and Text Extraction in Mixed Text/Image Documents - Friedrich M. Wahl, Kwan Y. Wong, and Richard G. Casey"
	if (isImageBackgroundWhite(img)) {
			invertImage(img);
		}
	cout << "Performing Text Blocks Extraction... ";
	outFileLog << "\nPerforming Text Blocks Extraction...\n";
	clock_t cBegin = clock();

	double imgResizeFactor = 4.0;
	IplImage *resizedImg;
	if (imgResizeFactor != 1.0) {
		resizedImg = cvCreateImage(cvSize((int) ceil(img->width / imgResizeFactor), (int) ceil(img->height
				/ imgResizeFactor)), img->depth, img->nChannels);
		cvResize(img, resizedImg, CV_INTER_AREA);
		cvThreshold(resizedImg, resizedImg, 127, 255, CV_THRESH_BINARY);
	} else {
		resizedImg = cvCloneImage(img);
	}

	double constantsMulFactor = 300.0 / (240 * imgResizeFactor);
	int rlsa_C_H = (int) ceil(RLSA_C_H * constantsMulFactor);
	int rlsa_C_V = (int) ceil(RLSA_C_V * constantsMulFactor);
	int rlsa_C_ALPHA = (int) ceil(RLSA_C_ALPHA * constantsMulFactor);
	double rlsa_C1 = RLSA_C1, rlsa_C2 = RLSA_C2 * constantsMulFactor, rlsa_C3 = RLSA_C3, rlsa_C4 = RLSA_C4;
	double rlsa_C11 = RLSA_C11, rlsa_C12 = RLSA_C12, rlsa_C13 = RLSA_C13 * constantsMulFactor, rlsa_C14 =
			RLSA_C14 * constantsMulFactor, rlsa_C15 = RLSA_C15 * constantsMulFactor, rlsa_C16 = RLSA_C16
			* constantsMulFactor, rlsa_C17 = RLSA_C17, rlsa_C18 = RLSA_C18;
	double rlsa_C21 = RLSA_C21, rlsa_C22 = RLSA_C22, rlsa_C23 = RLSA_C23;
	int rlsa_CC_MIN_AREA = (int) ceil(RLSA_CC_MIN_AREA * constantsMulFactor * constantsMulFactor);

	IplImage *rlsaOutputImg = performRunLengthSmoothing(resizedImg, rlsa_C_H, rlsa_C_V, rlsa_C_ALPHA);

	// Find connected components based on contours
	CvScalar color = CV_RGB(255, 255, 255);
	CvSeq* firstContour = 0;
	IplImage* tempImg = cvCreateImage(cvGetSize(rlsaOutputImg), IPL_DEPTH_8U, 1);
	cvFindContours(rlsaOutputImg, storage, &firstContour, sizeof(CvContour), CV_RETR_CCOMP,
			CV_CHAIN_APPROX_NONE);

	vector<int> heightSegBlock; // H
	vector<double> meanHorWhiteRunLengthOrigData; // R
	double meanH = 0, meanR = 0, stdH = 0, stdR = 0;
	int numBlocks = 0, numSupposedTextBlocks = 0;
	vector<bool> isSupposedTextBlock;

	for (CvSeq* contour = firstContour; contour != 0; contour = contour->h_next) {
		CvRect rect = cvBoundingRect(contour, 0);
		if (rect.width * rect.height < rlsa_CC_MIN_AREA) {
			// Assume CC corresponds to noise, and hence ignore.
			continue;
		}
		numBlocks++;
		cvZero(tempImg);
		cvDrawContours(tempImg, contour, color, color, -1, CV_FILLED, 8);

		cvSetImageROI(tempImg, rect);
		cvSetImageROI(resizedImg, rect);
		int BC = cvCountNonZero(tempImg); // numWhitePixelsSegBlock
		cvAnd(tempImg, resizedImg, tempImg);
		int DC = cvCountNonZero(tempImg); // numWhitePixelsOrigData
		cvResetImageROI(tempImg);
		cvResetImageROI(resizedImg);

		int TC = 0; // numHorBlackWhiteTransitionsOrigData
		for (int i = rect.y; i < rect.y + rect.height; i++) {
			bool isBlackRun = getPixelValue(tempImg, i, rect.x) == 0 ? true : false;
			for (int j = rect.x + 1; j < rect.x + rect.width; j++) {
				uchar curPixelValue = getPixelValue(tempImg, i, j);
				if (isBlackRun && curPixelValue > 0) {
					TC++;
					isBlackRun = false;
				} else if (!isBlackRun && curPixelValue == 0) {
					isBlackRun = true;
				}
			}
		}

		int H = rect.height; // heightSegBlock
		double E = rect.width / rect.height; // aspectRatioSegBlock
		double S = (double) BC / (rect.width * rect.height); // occupancyRatioSegBlock
		double R = (double) DC / TC; // meanHorWhiteRunLengthOrigData

		heightSegBlock.push_back(H);
		meanHorWhiteRunLengthOrigData.push_back(R);

		if (H / R > rlsa_C1 && H < rlsa_C2 && E > rlsa_C3 && S > rlsa_C4) {
			isSupposedTextBlock.push_back(true);
			meanH += H;
			meanR += R;
			numSupposedTextBlocks++;
		} else {
			isSupposedTextBlock.push_back(false);
		}
	}
	meanH /= numSupposedTextBlocks;
	meanR /= numSupposedTextBlocks;
	for (int k = 0; k < numBlocks; k++) {
		if (isSupposedTextBlock[k] == true) {
			stdH += pow(heightSegBlock[k] - meanH, 2);
			stdR += pow(meanHorWhiteRunLengthOrigData[k] - meanR, 2);
		}
	}
	stdH = sqrt(stdH / numSupposedTextBlocks);
	stdR = sqrt(stdR / numSupposedTextBlocks);

	outFileLog << "Text Extraction Statistics : " << endl;
	outFileLog << "\tnumBlocks = " << numBlocks << endl;
	outFileLog << "\tnumSupposedTextBlocks = " << numSupposedTextBlocks << endl;
	outFileLog << "\tmeanH = " << meanH << endl;
	outFileLog << "\tmeanR = " << meanR << endl;
	outFileLog << "\tstdH = " << stdH << endl;
	outFileLog << "\tstdR = " << stdR << endl;

	//IplImage* textBlocksExtractedImg = cvCreateImage(cvGetSize(rlsaOutputImg), IPL_DEPTH_8U, 1);
	IplImage* textBlocksExtractedImg = cvCreateImage(cvGetSize(img), img->depth, img->nChannels);
	cvSetZero(textBlocksExtractedImg);
	if (numSupposedTextBlocks > rlsa_C11 && ((double) numSupposedTextBlocks / numBlocks) > rlsa_C12 && meanR
			< rlsa_C13 && meanH < rlsa_C14 && stdH < rlsa_C15 && stdR < rlsa_C16 && stdH / meanH < rlsa_C17
			&& stdR / meanR < rlsa_C18) {
		// Use the variable, linear, separable classification scheme proposed in Wahl, Wong, and Casey's RLSA paper
		// to assign the following four classes to the blocks.
		int k = 0;
		for (CvSeq* contour = firstContour; contour != 0; contour = contour->h_next) {
			CvRect rect = cvBoundingRect(contour, 0);
			if (rect.width * rect.height < rlsa_CC_MIN_AREA) {
				// Assume CC corresponds to noise, and hence ignore.
				continue;
			}
			double R = meanHorWhiteRunLengthOrigData[k];
			int H = heightSegBlock[k];
			double E = (double) rect.width / rect.height;
			if (R < rlsa_C21 * meanR && H < rlsa_C22 * meanH) {
				// Class 1: Text
				outFileLog << "\tDetected a text block" << endl;

				CvRect rect = cvBoundingRect(contour, 0);
				int newX = (int) floor((rect.x - 1) * imgResizeFactor);
				newX = newX < 0 ? 0 : newX;
				int newY = (int) floor((rect.y - 1) * imgResizeFactor);
				newY = newY < 0 ? 0 : newY;
				int newWidth = (int) ceil((rect.width + 1) * imgResizeFactor);
				newWidth = newWidth > img->width ? img->width : newWidth;
				int newHeight = (int) ceil((rect.height + 1) * imgResizeFactor);
				newHeight = newHeight > img->height ? img->height : newHeight;
				CvRect origRect = cvRect(newX, newY, newWidth, newHeight);

				cvSetImageROI(img, origRect);
				cvSetImageROI(textBlocksExtractedImg, origRect);
				cvOr(textBlocksExtractedImg, img, textBlocksExtractedImg);
				cvResetImageROI(img);
				cvResetImageROI(textBlocksExtractedImg);
			} else if (R > rlsa_C21 * meanR && H < rlsa_C22 * meanH) {
				// Class 2: Horizontal solid black lines:
				outFileLog << "\tDetected an horizontal solid black line" << endl;
			} else if (H > rlsa_C22 * meanH && E > 1 / rlsa_C23) {
				// Class 3: Graphic and halftone images
				outFileLog << "\tDetected a graphic and halftone block" << endl;
			} else if (H > rlsa_C22 * meanH && E < 1 / rlsa_C23) {
				// Class 4: Vertical solid black lines
				outFileLog << "\tDetected a vertical solid black line" << endl;
			} else {
				// Unknown block
				outFileLog << "\tUnknown block found" << endl;
			}
			k++;
		}
		//replace original image with the textBlocksExtractedImg
		cvCopy(textBlocksExtractedImg, img);

		if (SAVE_INTERIM_IMAGES) {
			cvSaveImage(string(imageDir + PATH_SEPARATOR + toString(saveInterimImagesCounter++)
					+ "_TextBlocksExtractedImage.tif").c_str(), textBlocksExtractedImg);
		}
	} else {
		// Use some other approach for block classification.
		cout << "\nWahl, Wong, and Casey's RLSA algorithm failed to detect a text-cluster.\n";
		cout << "Please use some other approach for extracting text blocks.\n";
		outFileLog << "\nWahl, Wong, and Casey's RLSA algorithm failed to detect a text-cluster.\n";
		outFileLog << "Please use some other approach for extracting text blocks.\n";
	}

	cvReleaseImage(&textBlocksExtractedImg);
	cvReleaseImage(&tempImg);
	cvReleaseImage(&rlsaOutputImg);
	cvReleaseImage(&resizedImg);

	clock_t cEnd = clock();
	double timeInSecs = (cEnd - cBegin) / (float) CLOCKS_PER_SEC;
	cout << "Text Blocks Extraction complete. Time spent = " << timeInSecs << "secs\n";
	outFileLog << "Text Blocks Extraction complete. Time spent = " << timeInSecs << "secs\n";

	// TODO : Extract the logical order of text-blocks from textBlocksExtractedImg and use it to populate OCR_Page.blocks
	// time-being assume only single text block
	blocks.push_back(OCR_Block(img, cvRect(0, 0, img->width, img->height)));
}

void OCR_Page::saveSegmentedTextBlocks() {
	if (SAVE_SEGMENTED_COMPONENTS) {
		// Save each text-block as a separate image under folder segmentedTextBlocksDir
		for (unsigned int b = 0; b < blocks.size(); b++) {
			string blockName = segmentedTextBlocksDir + getBlockPrefix(b) + ".tif";
			cvSaveImage(blockName.c_str(), blocks[b].img);
		}
	}
}

void OCR_Page::addTextBlock(CvRect &rect) {
	blocks.push_back(OCR_Block(img, rect));
}

void OCR_Page::createFoldersForEachLabel() {
	makeDirectory("classes");
	makeDirectory("misc");
	for (unsigned int l = 0; l < 328; l++) {
		makeDirectory(("classes" + PATH_SEPARATOR + getLabelName(l + 1)).c_str());
	}
}

}
