#include "CRNN_Infer.h"
using std::cout;

namespace IISc_KannadaClassifier {

    std::unordered_map<string, InferCRNN*> InferCRNN::instanceMap;

    InferCRNN::InferCRNN (string modelPath) {
        this->modelPath = modelPath;
        session_options.config.mutable_gpu_options()->set_allow_growth(true);
        auto status = tensorflow::LoadSavedModel(session_options, run_options, modelPath, {"serve"}, &model);
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
};
