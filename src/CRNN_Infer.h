#ifndef CRNN_INFER_H_
#define CRNN_INFER_H_

#include <fstream>
#include <unordered_map>
#include <string>
#include <opencv/cv.h>
#include <tensorflow/cc/saved_model/loader.h>

using std::map;
using std::string;

using tensorflow::SavedModelBundle;
using tensorflow::SessionOptions;
using tensorflow::RunOptions;

namespace IISc_KannadaClassifier {

class CRNNModelConfig {
public:
	int width = 300;
	int height = 50;
	int timeSteps = 75;
	int numClasses = 178;
};

class InferCRNN {
private:
    InferCRNN (string modelPath);
    static map<string, InferCRNN*> instanceMap;
    string modelPath;
    SavedModelBundle model;
    SessionOptions session_options;
    RunOptions run_options;
    string inputTensorName;
    string outputTensorName;
    map<int, string> charactersMap;
    CRNNModelConfig modelConfig;
	const float EPSILON = (float) 0.0000001;
    void loadCharactersMap(string modelPath);

public:
    static InferCRNN* getInstance(string modelPath);
    string infer(IplImage *img);
};

};

#endif /* CRNN_INFER_H_ */
