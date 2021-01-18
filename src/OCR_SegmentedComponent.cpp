#include "OCR_SegmentedComponent.h"

namespace IISc_KannadaClassifier {

OCR_SegmentedComponent::OCR_SegmentedComponent(IplImage *src, CvRect rect, int _connectedComponentCount) {
	copyCvRect(rect, boundingBox);
	img = cloneSubImage(src, rect);
	connectedComponentCount = _connectedComponentCount;
}

OCR_SegmentedComponent::~OCR_SegmentedComponent() {
	//cvReleaseImage(&img);
}

void OCR_SegmentedComponent::identifyClassGroup(int relBaseLine, int lineHeight, int lineMiddle) {
	int refBelowBaseLine = (int) (OTTU_REFERENCE_BELOWBASELINE_FRACTION * relBaseLine);
	int specialSymbolThreshold = (int) (FRAC_PUNC * lineHeight);

	if (isOttu(lineMiddle, refBelowBaseLine)) {
		classGroup = OTTU_GROUP;
	} else if (boundingBox.width <= specialSymbolThreshold || boundingBox.height <= specialSymbolThreshold) {
		classGroup = SPECIAL_SYMBOL_GROUP;
		W_punc = false;
		H_punc = false;
		if (boundingBox.width <= specialSymbolThreshold && boundingBox.y <= relBaseLine) {
			W_punc = true;
		}
		if (boundingBox.height <= specialSymbolThreshold && boundingBox.y <= relBaseLine) {
			H_punc = true;
		}

		// change the bounding box to cover the height of the line
		IplImage* newImg = cvCreateImage(cvSize(boundingBox.width, lineHeight), IPL_DEPTH_8U, 1);
		cvZero(newImg);
		cvSetImageROI(newImg, cvRect(0, boundingBox.y, boundingBox.width, boundingBox.height));
		cvCopy(img, newImg);
		cvResetImageROI(newImg);
		cvReleaseImage(&img); //delete the earlier cloned image
		img = newImg;
		boundingBox.y = 0;
		boundingBox.height = lineHeight;
	} else {
		classGroup = BASE_GROUP;
	}
}

void OCR_SegmentedComponent::extractFeatures(CvMat *avgVectorBase, CvMat *eigenVectorsBase,
		CvMat *avgVectorOttu, CvMat *eigenVectorsOttu, CvMat *avgVectorSS, CvMat *eigenVectorsSS) {
	if (classGroup == SPECIAL_SYMBOL_GROUP) {
		IplImage* resizedImgSS = cvCreateImage(cvSize(32, 32), IPL_DEPTH_8U, 1);
		resizeSpecialSymbol(img, resizedImgSS);
		cvThreshold(resizedImgSS, resizedImgSS, 127, 1, CV_THRESH_BINARY);
		CvMat dataHeaderSS, *dataSS;
		dataSS = cvReshape(resizedImgSS, &dataHeaderSS, 0, 1);
		CvMat* resultSS = cvCreateMat(1, 81, CV_32FC1);

		cvProjectPCA(dataSS, avgVectorSS, eigenVectorsSS, resultSS);

		float *resdataSS = resultSS->data.fl;
		for (int i = 0; i < 81; i++) {
			featureVectorSS[i].index = i + 1;
			featureVectorSS[i].value = resdataSS[i];
		}
		featureVectorSS[81].index = -1;

		cvReleaseImage(&resizedImgSS);
		cvReleaseMat(&resultSS);
	} else if (classGroup == OTTU_GROUP) {
		IplImage* resizedImgOttu = cvCreateImage(cvSize(16, 16), IPL_DEPTH_8U, 1);
		cvResize(img, resizedImgOttu);
		cvThreshold(resizedImgOttu, resizedImgOttu, 127, 1, CV_THRESH_BINARY);
		CvMat dataHeaderOttu, *dataOttu;
		dataOttu = cvReshape(resizedImgOttu, &dataHeaderOttu, 0, 1);
		CvMat* resultOttu = cvCreateMat(1, 134, CV_32FC1);

		cvProjectPCA(dataOttu, avgVectorOttu, eigenVectorsOttu, resultOttu);

		float *resdataOttu = resultOttu->data.fl;
		for (int i = 0; i < 134; i++) {
			featureVectorOttu[i].index = i + 1;
			featureVectorOttu[i].value = resdataOttu[i];
		}
		featureVectorOttu[134].index = -1;

		cvReleaseImage(&resizedImgOttu);
		cvReleaseMat(&resultOttu);
	} else {
		IplImage* resizedImgBase = cvCreateImage(cvSize(32, 32), IPL_DEPTH_8U, 1);
		cvResize(img, resizedImgBase);
		cvThreshold(resizedImgBase, resizedImgBase, 127, 1, CV_THRESH_BINARY);
		CvMat dataHeaderBase, *dataBase;
		dataBase = cvReshape(resizedImgBase, &dataHeaderBase, 0, 1);
		CvMat* resultBase = cvCreateMat(1, 507, CV_32FC1);

		cvProjectPCA(dataBase, avgVectorBase, eigenVectorsBase, resultBase);

		float *resdata_base = resultBase->data.fl;
		for (int i = 0; i < 507; i++) {
			featureVectorBase[i].index = i + 1;
			featureVectorBase[i].value = resdata_base[i];
		}
		featureVectorBase[507].index = -1;

		cvReleaseImage(&resizedImgBase);
		cvReleaseMat(&resultBase);
	}
}

void OCR_SegmentedComponent::extractFeatures() {
	int resizeWidth = 32, resizeHeight = 32;
	if (classGroup == OTTU_GROUP) {
		resizeWidth = 16;
		resizeHeight = 16;
	}
	IplImage* resizedImg = cvCreateImage(cvSize(resizeWidth, resizeHeight), IPL_DEPTH_8U, 1);
	if (classGroup == SPECIAL_SYMBOL_GROUP) {
		resizeSpecialSymbol(img, resizedImg);
	} else {
		cvResize(img, resizedImg);
	}
	// output of cvResize is a graylevel image => binarization required
	cvThreshold(resizedImg, resizedImg, 127, 1, CV_THRESH_BINARY);
	lfeatureVector = new feature_node[cvCountNonZero(resizedImg) + 1];
	int k = 0;
	for (int i = 0; i < resizeHeight; i++) {
		for (int j = 0; j < resizeWidth; j++) {
			double pixelValue = cvGet2D(resizedImg, i, j).val[0];
			if (pixelValue != 0) {
				lfeatureVector[k].index = i * resizeWidth + j + 1;
				lfeatureVector[k].value = pixelValue;
				k++;
			}
		}
	}
	lfeatureVector[k].index = -1;
	cvReleaseImage(&resizedImg);
}

void OCR_SegmentedComponent::classify(model *lsvmModelBase, model *lsvmModelOttu, model *lsvmModelSS) {
	if (classGroup == SPECIAL_SYMBOL_GROUP) {
		classLabel[0] = getLabelName((int) predict(lsvmModelSS, lfeatureVector), classGroup);
	} else if (classGroup == OTTU_GROUP) {
		classLabel[0] = getLabelName((int) predict(lsvmModelOttu, lfeatureVector), classGroup);
	} else {
		classLabel[0] = getLabelName((int) predict(lsvmModelBase, lfeatureVector), classGroup);
	}
	delete []lfeatureVector;
}

void OCR_SegmentedComponent::classify(svm_model *svmModelBase, svm_model *svmModelOttu, svm_model *svmModelSS) {
	if (classGroup == SPECIAL_SYMBOL_GROUP) {
		classLabel[0] = getLabelName((int) svm_predict(svmModelSS, featureVectorSS));
	} else if (classGroup == OTTU_GROUP) {
		classLabel[0] = getLabelName((int) svm_predict(svmModelOttu, featureVectorOttu));
	} else {
		classLabel[0] = getLabelName((int) svm_predict(svmModelBase, featureVectorBase));
	}
}

string OCR_SegmentedComponent::ssPredict(int relBaseline, int lineHeight) {
	if (W_punc == true && H_punc == true) {
		return "dot";
	}
	string ssClassName = "";
	if (W_punc == true) {
		switch (connectedComponentCount) {
		case 1:
			if (boundingBox.height > DANDA_FACTOR * relBaseline) {
				if (boundingBox.width <= BDThreshold * relBaseline) {
					ssClassName = "danda";
				} else {
					//Take VPP
					vector<int> col_sum(img->width);
					CvMat col;
					for (unsigned int l = 0; l < col_sum.size(); l++) {
						cvGetCol(img, &col, l);
						col_sum[l] = cvCountNonZero(&col);
					}
					int vppLM = 0;
					int vppRM = 0;
					for (unsigned int l = 0; l < 4; l++) {
						if (vppLM < col_sum[l]) {
							vppLM = col_sum[l];
						}
						if (vppRM < col_sum[img->width - 1 - l]) {
							vppRM = col_sum[img->width - 1 - l];
						}
					}
					if (vppLM > vppRM) {
						ssClassName = "lSqBracket";
					} else {
						ssClassName = "rSqBracket";
					}
				}
			} else if ((boundingBox.y + boundingBox.height) < MID_FACTOR * relBaseline) {
				ssClassName = "apostrophe"; // can be ' or " and other variations of the same
			} else {
				ssClassName = "comma";
			}
			break;

		case 2:
			//Here set the CC's flag
			vector<OCR_SegmentedComponent> specialSymbolComponents;
			CvScalar color = CV_RGB(255, 255, 255);
			CvSeq* contour = 0;
			IplImage* tempImg1 = cvCloneImage(img);
			IplImage* tempImg2 = cvCreateImage(cvGetSize(img), IPL_DEPTH_8U, 1);
			cvFindContours(tempImg1, storage, &contour, sizeof(CvContour), CV_RETR_CCOMP,
					CV_CHAIN_APPROX_NONE);
			// Note that cvFindContours() function modifies the source image content.
			// Hence instead of directly sending img to cvFindContours(), it's clone is sent.
			for (; contour != 0; contour = contour->h_next) {
				CvRect rect = cvBoundingRect(contour, 0);
				cvZero(tempImg2);
				cvDrawContours(tempImg2, contour, color, color, -1, CV_FILLED, 8);
				specialSymbolComponents.push_back(OCR_SegmentedComponent(tempImg2, rect));
			}
			cvReleaseImage(&tempImg2);
			cvReleaseImage(&tempImg1);
			if (specialSymbolComponents[0].boundingBox.y > specialSymbolComponents[1].boundingBox.y) {
				//swap
				OCR_SegmentedComponent tempComponent = specialSymbolComponents[0];
				specialSymbolComponents[0] = specialSymbolComponents[1];
				specialSymbolComponents[1] = tempComponent;
			}
			int specialSymbolThreshold = int(FRAC_PUNC * lineHeight);
			specialSymbolComponents[0].W_punc = false;
			specialSymbolComponents[0].H_punc = false;
			if (specialSymbolComponents[0].boundingBox.width <= specialSymbolThreshold) {
				specialSymbolComponents[0].W_punc = true;
			}
			if (specialSymbolComponents[0].boundingBox.height <= specialSymbolThreshold) {
				specialSymbolComponents[0].H_punc = true;
			}
			specialSymbolComponents[1].W_punc = false;
			specialSymbolComponents[1].H_punc = false;
			if (specialSymbolComponents[1].boundingBox.width <= specialSymbolThreshold) {
				specialSymbolComponents[1].W_punc = true;
			}
			if (specialSymbolComponents[1].boundingBox.height <= specialSymbolThreshold) {
				specialSymbolComponents[1].H_punc = true;
			}

			if (specialSymbolComponents[0].H_punc == true && specialSymbolComponents[0].W_punc == true) {
				if (specialSymbolComponents[1].W_punc == true) {
					ssClassName = "semicolon";
					if (specialSymbolComponents[1].H_punc == true) {
						ssClassName = "colon";
					}
				}
			}
			if (specialSymbolComponents[1].H_punc == true && specialSymbolComponents[1].W_punc == true) {
				if (specialSymbolComponents[0].boundingBox.width < QEThreshold * relBaseline) {
					ssClassName = "exclamation";
				}
			}
			specialSymbolComponents.clear();
			break;
		}
	}
	else {
		if (boundingBox.y > DASH_FACTOR * relBaseline) {
			ssClassName = "underscore";
		} else {
			ssClassName = "dash"; //implement comparision between dash and tilde
		}
	}
	return ssClassName;
}

bool OCR_SegmentedComponent::isOttu(int lineMiddle, int refBelowBaseLine) {
	int cc_yStart = boundingBox.y;
	int cc_yEnd = boundingBox.y + boundingBox.height;
	if (cc_yStart > lineMiddle && cc_yEnd > refBelowBaseLine) {
		return true;
	}
	return false;
}

bool OCR_SegmentedComponent::isTop(int lineMiddle, int lineHeight) {
	if ((boundingBox.y < lineMiddle) && boundingBox.height <= LINE_MIDDLE_FRACTION * lineHeight) {
		return true;
	}
	return false;
}

/**
 * Returns true if the component is suspected to have to a ottu merged with base part, else returns false.
 *
 */
bool OCR_SegmentedComponent::isBaseAndOttuMerged(int relBaseLine, int lineHeight, bool &isOttuTowardsLeft,
		bool &isOttuTowardsRight) {
	//int xStart = boundingBox.x;
	int yStart = boundingBox.y;
	int width = boundingBox.width;
	int height = boundingBox.height;
	int refBelowBaseLine = (int) (OTTU_REFERENCE_BELOWBASELINE_FRACTION * relBaseLine);
	//int xEnd = xStart + width;
	int yEnd = yStart + height;
	float aspectRatio = (float) width / height;
	int lineMiddle = (int) floor(lineHeight * LINE_MIDDLE_FRACTION);

	if (isOttu(lineMiddle, refBelowBaseLine)) {
		return false;
	}
	if (yEnd < relBaseLine + relBaseLine * THRESHOLD_FOR_PUREBASE_BELOWBASELINE_FRACTION) {
		//segmentedComponent doesn't have ottu-part
		return false;
	}
	if (aspectRatio < SPECIALSYMBOL_ASPECT_RATIO) {
		//segmentedComponent one of ')', '(' or '!'
		return false;
	}
	if (height < MIN_HEIGHT_BASECOMPONENT_FRACTION * relBaseLine || width
			< MIN_WIDTH_BASE_OOTU_MERGED_COMPONENT_FRACTION * relBaseLine) {
		return false;
	}

	//Take VPP of bottom region (i.e. between BaseLine and LineBottom)
	CvMat componentBelowBaseLine;
	cvGetRows(img, &componentBelowBaseLine, relBaseLine - yStart, height);
	vector<int> colSum_BelowBaseLine(width);
	CvMat col;
	for (int j = 0; j < width; j++) {
		cvGetCol(&componentBelowBaseLine, &col, j);
		colSum_BelowBaseLine[j] = cvCountNonZero(&col);
	}
	//vector<int> seqStart;
	//vector<int> seqEnd;
	//getNonZeroSequences(colSum_BelowBaseLine, seqStart, seqEnd, 1);

	int bottomRegion_xStart = 0;
	while (colSum_BelowBaseLine[bottomRegion_xStart] == 0) {
		bottomRegion_xStart++;
	}
	int bottomRegion_xEnd = width - 1;
	while (colSum_BelowBaseLine[bottomRegion_xEnd] == 0) {
		bottomRegion_xEnd--;
	}
	int offset = 3;
	if (bottomRegion_xStart > offset && bottomRegion_xEnd < width - 1 - offset) {
		//components such as g1, ch1, j1, D1, t1, d1, p1, b1 (the ones with padham) or one of puu/poo, p1uu/p1oo, vuu/voo.

		//if (seqEnd[0] - seqStart[0] < MIN_WIDTH_OTTUCOMPONENT_FRACTION * relBaseLine ||
		//		yEnd - relBaseLine <= MIN_HEIGHT_OTTUCOMPONENT_FRACTION * relBaseLine) {
		//	return false;
		//}
		//ottu towards center => apply horizontal cut for the whole length
		//return true;

		return false;
	}

	// Numbers reaching below relBaseLine and special symbols like '?' might sometime be cut.
	// Need to include additional checks for these.

	isOttuTowardsRight = false;
	isOttuTowardsLeft = false;
	if (bottomRegion_xStart <= offset) {
		isOttuTowardsLeft = true;
	}
	if (bottomRegion_xEnd >= width - 1 - offset) {
		isOttuTowardsRight = true;
	}
	if (isOttuTowardsLeft && isOttuTowardsRight) {
		//assume it must be lla where lOttu comes directly below base-part
		return true;
	}

	if (isOttuTowardsRight) {
		//In case of lower part of sha or pa gets connected to ottu then return true
		//if (yStart > LINE_MIDDLE_FRACTION * relBaseLine) {
		//	//only one ottu is at right
		//	lCutPositionEnd = seqStart[0] - 3;
		//	return true;
		//}

		if (yEnd - relBaseLine <= MIN_HEIGHT_OTTUCOMPONENT_FRACTION * relBaseLine) {
			return false;
		}

		//Take 5% (w.r.t. width) of the right-end of the component and take its HPP
		//Faled in 0165_0082 Line-10 Word-1 Component-4
		//CvMat componentRightEnd;
		//cvGetCols(img, &componentRightEnd, (int) (width * 0.95), width);
		//vector<int> rowSum_RightEnd(height);
		//CvMat row;
		//for (int i = 0; i < height; i++) {
		//	cvGetRow(&componentRightEnd, &row, i);
		//	rowSum_RightEnd[i] = cvCountNonZero(&row);
		//}
		//int rowSum_yStart = 0;
		//while (rowSum_RightEnd[rowSum_yStart] == 0) {
		//	rowSum_yStart++;
		//}
		//if (rowSum_yStart < lineMiddle) {
		//	//assume it must be one of pu, p1u, vu
		//	return false;
		//}

		//lCutPositionEnd = bottomRegion_xStart - 3;
		//if (lCutPositionEnd < 0) {
		//	lCutPositionEnd = 0;
		//}
	}

	if (isOttuTowardsLeft) {
		//case of na extending below baseline if it is very small
		if (yEnd - relBaseLine < FRAC_PUNC * relBaseLine || bottomRegion_xEnd - bottomRegion_xStart
				< MIN_WIDTH_OTTUCOMPONENT_FRACTION * relBaseLine) {
			return false;
		}
		//if (isOttuTowardsCenter && seqStart.size() == 2) {
		//	// 1st ottu is at left and 2nd ottu is at center
		//	lCutPositionEnd = seqEnd[1] + 3;
		//} else {
		//	//Only one ottu is at left
		//	lCutPositionEnd = seqEnd[0] + 3;
		//}
		//Not to pass beyond width
		//if (lCutPositionEnd >= width) {
		//	lCutPositionEnd = width - 1;
		//}
	}

	//assume component has a ottu merged with base.
	return true;
}

}
