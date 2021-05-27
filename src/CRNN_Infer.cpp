#include "CRNN_Infer.h"

#include <opencv2/core/core_c.h>
#include <opencv2/core/types_c.h>
#include <opencv2/imgproc/imgproc_c.h>
#include <tensorflow/core/framework/tensor.h>
#include <tensorflow/core/framework/tensor_shape.h>
#include <tensorflow/core/framework/types.pb.h>
#include <tensorflow/core/platform/status.h>
#include <tensorflow/core/protobuf/config.pb.h>
#include <cmath>
#include <iostream>

using std::cout;
using namespace tensorflow;

namespace IISc_KannadaClassifier {

    std::unordered_map<string, InferCRNN*> InferCRNN::instanceMap;

    InferCRNN::InferCRNN (string modelPath) {
        this->modelPath = modelPath;
        session_options.config.mutable_gpu_options()->set_allow_growth(true);
        auto status = LoadSavedModel(session_options, run_options, modelPath, {"serve"}, &model);
        if (status.ok()) {
            cout << "Model loaded successfully...\n";
        } else {
            cout << "Error in loading model\n";
        }
    }

    InferCRNN* InferCRNN::getInstance(string modelPath){
        InferCRNN* instance = InferCRNN::instanceMap[modelPath];
        if (instance == NULL) {
            instance = new InferCRNN(modelPath);
            instanceMap[modelPath] = instance;
        }
        return instance;
    }

    string InferCRNN::infer(IplImage *img) {
        return "";
    }

	static IplImage * resizeAndPad(IplImage* inputImage, int outputImgHeight, int outputImgWidth) {
		double aspectRatio = 1.0 * inputImage->width / inputImage->height;
		int newWidth = (int) round(1.0 * outputImgHeight / inputImage->height * inputImage->width);
		if (aspectRatio < 8.0 && newWidth < outputImgWidth) {
			IplImage* tempImage = cvCreateImage(cvSize(newWidth, outputImgHeight), IPL_DEPTH_8U, 1);
			cvResize(inputImage, tempImage);
			IplImage* outputImage = cvCreateImage(cvSize(outputImgWidth, outputImgHeight), IPL_DEPTH_8U, 1);
			for (int r = 0; r < outputImgHeight; r++) {
				for (int c = 0; c < outputImgWidth; c++) {
					cvSet2D(outputImage, r, c, cvGet2D(tempImage, r, c).val[0]);
				}
				for (int c = newWidth; c < outputImgWidth; c++) {
					cvSet2D(outputImage, r, c, 0);
				}
			}
			cvReleaseImage(&tempImage);
			return outputImage;
		} else {
			IplImage* resizedImage = cvCreateImage(cvSize(outputImgWidth, outputImgHeight), IPL_DEPTH_8U, 1);
			cvResize(inputImage, resizedImage);
			return resizedImage;
		}
	}

	static Tensor convertImageToCRNNInput(IplImage* inputImage, CRNNModelConfig& modelConfig) {
		IplImage* resizedImage = resizeAndPad(inputImage, modelConfig.height, modelConfig.width);
		Tensor image_tensor(DT_FLOAT, TensorShape({ 1, modelConfig.height, modelConfig.width, 1 }));
		// FloatNdArray ndArray = NdArrays.ofFloats(Shape.of(1, modelConfig.width, modelConfig.height, 1));
		for (int r = 0; r < resizedImage->height; r++) {
			for (int c = 0; c < resizedImage->width; c++) {
				double value = cvGet2D(resizedImage, r, c) / 255.0;
				// Transpose and Flip
				image_tensor(0, c, resizedImage->height - 1 - r, 0) = (float)*value;
			}
		}
		return image_tensor;
	}

};
