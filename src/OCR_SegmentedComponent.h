#ifndef OCR_SEGMENTEDCOMPONENT_H_
#define OCR_SEGMENTEDCOMPONENT_H_

#include <string>
#include <cxcore.h>
#include <cv.h>
#include <highgui.h>
#include <cstdio>
#include "OCR_GlobalDef.h"
#include "OCR_Util.h"
#include "OCR_ClassLabel.h"
#include <svm.h>
using std::string;

namespace IISc_KannadaClassifier {

class OCR_SegmentedComponent {
public:
	CvRect boundingBox;
	IplImage *img;

	int classGroup;

	bool W_punc;
	bool H_punc;

	svm_node featureVectorBase[520];
	svm_node featureVectorOttu[150];
	svm_node featureVectorSS[90];

	feature_node *lfeatureVector;

	string classLabel[2];
	int confidenceLevel[2];
	int errorCode; // 0->no error, 1->error w.r.t aspect ratio, etc
	int connectedComponentCount;
	float aspectRatio;

	OCR_SegmentedComponent(IplImage *src, CvRect rect, int _connectedComponentCount = 1);
	virtual ~OCR_SegmentedComponent();
	//This function identifies the class Group of the Segmented Component
	void identifyClassGroup(int relBaseLine, int lineHeight, int lineMiddle);
	//This function extracts the PCA of Segmented Components and stores in the featureVector array.
	void extractFeatures(CvMat *avgVectorBase, CvMat *eigenVectorsBase, CvMat *avgVectorOttu,
			CvMat *eigenVectorsOttu, CvMat *avgVectorSS, CvMat *eigenVectorsSS);
	void extractFeatures();
	void classify(model *lsvmModelBase, model *lsvmModelOttu, model *lsvmModelSS);
	void classify(svm_model* svmModelBase, svm_model* svmModelOttu, svm_model* svmModelSS);
	string ssPredict(int baseline, int lineHeight);
	bool isOttu(int lineMiddle, int refBelowBaseLine);
	bool isTop(int lineMiddle, int lineHeight);
	bool isBaseAndOttuMerged(int relBaseLine, int lineHeight, bool &isOttuTowardsLeft, bool &isOttuTowardsRight);
};

}

#endif /* OCR_SEGMENTEDCOMPONENT_H_ */
