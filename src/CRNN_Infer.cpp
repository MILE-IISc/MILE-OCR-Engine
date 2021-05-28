#include "CRNN_Infer.h"

#include <absl/container/inlined_vector.h>
#include <opencv2/core/core_c.h>
#include <opencv2/core/types_c.h>
#include <opencv2/imgproc/imgproc_c.h>
#include <tensorflow/core/framework/tensor.h>
#include <tensorflow/core/framework/tensor_shape.h>
#include <tensorflow/core/framework/types.pb.h>
#include <tensorflow/core/platform/default/integral_types.h>
#include <tensorflow/core/platform/status.h>
#include <tensorflow/core/protobuf/config.pb.h>
#include <tensorflow/core/public/session.h>
#include <tensorflow/core/util/ctc/ctc_decoder.h>
#include <cmath>
#include <iostream>
#include <vector>

using std::cout;
using namespace tensorflow;

namespace IISc_KannadaClassifier {

    std::unordered_map<string, InferCRNN*> InferCRNN::instanceMap;

    InferCRNN::InferCRNN (string modelPath) {
        this->modelPath = modelPath;
        session_options.config.mutable_gpu_options()->set_allow_growth(true);
        auto status = LoadSavedModel(session_options, run_options, modelPath, {"serve"}, &model);
        if (status.ok()) {
            cout << "Model loaded successfully.\n";
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

	static IplImage* resizeAndPad(IplImage* inputImage, int outputImgHeight, int outputImgWidth) {
		double aspectRatio = 1.0 * inputImage->width / inputImage->height;
		int newWidth = (int) round(aspectRatio * outputImgHeight);
		if (aspectRatio < 8.0 && newWidth < outputImgWidth) {
			IplImage* tempImage = cvCreateImage(cvSize(newWidth, outputImgHeight), IPL_DEPTH_8U, 1);
			cvResize(inputImage, tempImage);
			IplImage* outputImage = cvCreateImage(cvSize(outputImgWidth, outputImgHeight), IPL_DEPTH_8U, 1);
			for (int r = 0; r < outputImgHeight; r++) {
				for (int c = 0; c < newWidth; c++) {
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

	static Tensor* convertImageToCRNNInput(IplImage* inputImage, CRNNModelConfig& modelConfig) {
		IplImage* resizedImage = resizeAndPad(inputImage, modelConfig.height, modelConfig.width);
		Tensor* image_tensor = new Tensor(DT_FLOAT, TensorShape({ 1, modelConfig.width, modelConfig.height, 1 }));
		auto input_tensor_mapped = image_tensor->tensor<float, 4>();

		// https://danishshres.medium.com/construction-of-tensorflow-tensor-with-c-a53356d4f41e
		for (int r = 0; r < resizedImage->height; r++) {
			for (int c = 0; c < resizedImage->width; c++) {
				double value = cvGet2D(resizedImage, r, c).val[0] / 255.0;
				// Transpose and Flip
				input_tensor_mapped(0, c, resizedImage->height - 1 - r, 0) = (float) value;
			}
		}
		cvReleaseImage(&resizedImage);
		return image_tensor;
	}

	string InferCRNN::infer(IplImage *inputImage) {
		Tensor* crnnInputTensor = convertImageToCRNNInput(inputImage, this->modelConfig);
		int timeSteps = modelConfig.timeSteps;
		int numClasses = modelConfig.numClasses;

		Tensor crnnOutputTensor(DT_FLOAT, TensorShape({ 1, timeSteps, numClasses}));
		std::vector<Tensor> crnnOutput = {crnnOutputTensor};
		// https://medium.com/analytics-vidhya/inference-tensorflow2-model-in-c-aa73a6af41cf
		Status runStatus = model.GetSession()->Run({{"input_1", *crnnInputTensor}}, {"fc_12"}, {}, &crnnOutput);
		delete crnnInputTensor;

		auto crnnOutputTensorMapped = crnnOutputTensor.tensor<float, 3>();
		int prevClassIndex = -1;
		int blankIndex = numClasses - 1;
		std::vector<int> ctcDecodedValues;
		double score = 0.0;
		for (int t = 2; t < timeSteps; t++) {
			int maxClassIndex = 0;
			float maxValue = 0;
			for (int c = 0; c < numClasses; c++) {
				float value = crnnOutputTensorMapped(0, t, c);
				if (value > maxValue) {
					maxValue = value;
					maxClassIndex = c;
				}
			}
			score += -log(maxValue + EPSILON);
			if (maxClassIndex != blankIndex && maxClassIndex != prevClassIndex) {
				ctcDecodedValues.push_back(maxClassIndex);
			}
			prevClassIndex = maxClassIndex;
		}

		for (int c: ctcDecodedValues) {
			cout << c << " ";
		}
		return "";
    }

};
