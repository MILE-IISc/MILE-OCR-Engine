/** @mainpage Kannada Optical Character Recognition module
 * \section intro_sec Introduction
 *
 * Copyright (c) 2009, Prof. A G Ramakrishnan, MILE Lab, IISc, Bangalore
 *
 * All rights reserved.
 *
 * @author      Prof. A G Ramakrishnan, Shiva Kumar H R, Chetan Ramaiah, Prathibha Prabhakar, Akshay Rao
 * @note		Contact: ramkiag@ee.iisc.ernet.in
 *
 */

#include <iostream>
#include <vector>
#include <cctype>
#include <string>
#include <cmath>
using namespace std;
#include <ctime>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>

#include "OCR_Util.h"
#include "OCR_Akshara.h"
#include "OCR_SegmentedComponent.h"
#include "OCR_Word.h"
#include "OCR_Line.h"
#include "OCR_Block.h"
#include "OCR_Page.h"
#include "OCR_XML.h"

using namespace IISc_KannadaClassifier;

bool metaDataLoaded = false;

int performOCR(string imagePath, string imageName, vector<CvRect> *textBlocks = NULL,
		char *outputFileNamePrefix = NULL, const char *outputXml = NULL) {
	clock_t cBegin_main = clock();
	imageDir = "output" + PATH_SEPARATOR + imageName + PATH_SEPARATOR;
	makeDirectory("output");
	makeDirectory(imageDir.c_str());
	outFileLog.open((imageDir + "OCR_log.txt").c_str());
	outFileLog << "MILE Lab Kannada OCR\n";
	outFileLog << "\nInput image : " << imagePath << endl;

	if (!metaDataLoaded) {
		if (USE_LINEAR_SVM) {
			loadMetaData2();
		} else {
			loadMetaData();
		}
		loadClassLabels();
		metaDataLoaded = true;
	} else {
		outFileLog << "\nRequired MetaData is already loaded into the memory\n";
	}

	cout << "\nLoading input image " << imageName << "... ";
	outFileLog << "\nLoading input image " << imageName << "...\n";
	IplImage* img = cvLoadImage(imagePath.c_str(), CV_LOAD_IMAGE_GRAYSCALE);
	if (img == NULL) {
		cout << "Error reading input image '" + imagePath + "'\n";
		outFileLog << "Error reading input image : '" + imagePath + "' !!!\n";
		return -1;
	}
	cout << "Successfully loaded the input image\n";
	outFileLog << "Successfully loaded the input image\n";

	segmentedTextBlocksDir = imageDir + "1_SegmentedTextBlocks" + PATH_SEPARATOR;
	segmentedLinesDir = imageDir + "2_SegmentedLines" + PATH_SEPARATOR;
	segmentedWordsDir = imageDir + "3_SegmentedWords" + PATH_SEPARATOR;
	segmentedComponentsDir = imageDir + "4_SegmentedComponents" + PATH_SEPARATOR;
	mergedComponentsDir = imageDir + "5_MergedComponents" + PATH_SEPARATOR;
	saveInterimImagesCounter = 1;

	if (SAVE_SEGMENTED_COMPONENTS) {
		makeDirectory(segmentedTextBlocksDir.c_str());
		makeDirectory(segmentedLinesDir.c_str());
		makeDirectory(segmentedWordsDir.c_str());
		makeDirectory(segmentedComponentsDir.c_str());
		makeDirectory(mergedComponentsDir.c_str());
	}

	OCR_Page page = OCR_Page(img);
	page.binarize();
	if (textBlocks != NULL && textBlocks->size() > 0) {
		page.isBlockSegmented = true;
		// if (isImageBackgroundWhite(page.img)) {
		// 	invertImage(page.img);
		// }
		for (unsigned int b = 0; b < textBlocks->size(); b++) {
			page.addTextBlock((*textBlocks)[b]);
		}
	} else {
		page.skewCorrect();
		page.extractTextBlocks();
		page.saveSegmentedTextBlocks();
	}

	clock_t cBegin = clock();
	cout << "Performing Segmentation+Classification+UnicodeGeneration...\n";
	outFileLog << "\nPerforming Segmentation+Classification+UnicodeGeneration...\n\n";
	vector<OCR_Block> &blocks = page.blocks;
	for (unsigned int b = 0; b < blocks.size(); b++) {
		outFileLog << "Block " << (b + 1) << ": ";
		CvRect &cvRect = blocks[b].boundingBox;
		outFileLog << "x=" << cvRect.x << ", y=" << cvRect.y << ", width=" << cvRect.width << ", height=" << cvRect.height << "\n";
		blocks[b].segmentLines();
		blocks[b].saveSegmentedLines(b);
		vector<OCR_Line> &lines = blocks[b].lines;
		outFileLog << "Lines Segmented. Count = " <<lines.size()<<endl ;
		for (unsigned int l = 0; l < lines.size(); l++) {
			outFileLog << "  Line " << (l + 1) << ": ";
			lines[l].segmentWords();
			lines[l].saveSegmentedWords(b, l);
			vector<OCR_Word> &words = lines[l].words;
			outFileLog << "Words Segmented. Count = " <<words.size()<<endl;
			for (unsigned int w = 0; w < words.size(); w++) {
				outFileLog << "    Word " << (w + 1) << ": ";
				words[w].segmentComponents();
				words[w].splitMergedCharacters();
				sort(words[w].segmentedComponents.begin(), words[w].segmentedComponents.end(),
						words[w].compareSegmentedComponents);
				words[w].combinePartCharacters();
				words[w].saveSegmentedComponents(b, l, w);
				vector<OCR_SegmentedComponent> &segmentedComponents = words[w].segmentedComponents;
				outFileLog << "Components Segmented. Count = " <<segmentedComponents.size()<<endl;
				for (unsigned int c = 0; c < segmentedComponents.size(); c++) {
					outFileLog << "      Component " << (c + 1) << ": ";
					segmentedComponents[c].identifyClassGroup(words[w].relBaseLine, words[w].lineHeight,
							words[w].lineMiddle);
					if (!USE_LINEAR_SVM) {
						segmentedComponents[c].extractFeatures(avgVectorBase,
								eigenVectorsBase, avgVectorOttu,
								eigenVectorsOttu, avgVectorSS, eigenVectorsSS);
						segmentedComponents[c].classify(svmModelBase,
								svmModelOttu, svmModelSS);
					} else {
						segmentedComponents[c].extractFeatures();
						segmentedComponents[c].classify(lsvmModelBase,
								lsvmModelOttu, lsvmModelSS);
					}
					outFileLog << segmentedComponents[c].classLabel[0]<<endl;
				}
				outFileLog << "\n";
				words[w].verify();
				words[w].groupIntoAksharas();
				vector<OCR_Akshara> &aksharas = words[w].aksharas;
				outFileLog << "     Aksharas Grouped. Count = " <<aksharas.size()<<endl;
				for (unsigned int a = 0; a < aksharas.size(); a++) {
					outFileLog << "      Akshara " << (a + 1) << ": ";
					aksharas[a].emitAksharaUnicode();
					outFileLog << aksharas[a] << endl;
				}
				outFileLog << "\n";
			}
		}
		// Save unicode for each text block.
		string outputFileName;
		if (outputFileNamePrefix == NULL || strcmp(outputFileNamePrefix, "") == 0) {
			stringstream ss;
			ss <<  imageDir << PATH_SEPARATOR << "output_" << (b + 1) << ".txt";
			outputFileName = ss.str().c_str();
		} else {
			outputFileName = outputFileNamePrefix;
			// imageDir + PATH_SEPARATOR + string(extractFileName(outputFileNamePrefix));
		}
		blocks[b].writeUnicodesToFile(outputFileName);
	}
	clock_t cEnd = clock();
	double timeInSecs = (cEnd - cBegin) / (float) CLOCKS_PER_SEC;
	cout << "Segmentation+Classification+UnicodeGeneration complete. Time spent on = " << timeInSecs << "secs\n";
	outFileLog << "Segmentation+Classification+UnicodeGeneration complete. Time spent = " << timeInSecs << "secs\n";

	if (outputXml != NULL &&  strcmp(outputXml, "") != 0) {
		writeOcrOutputXML(page, outputXml);
	} else {
		string xmlFileName = imageDir + PATH_SEPARATOR + "output"+ ".xml";
		writeOcrOutputXML(page, xmlFileName.c_str());
	}
	cvReleaseImage(&img);

	clock_t cEnd_main = clock();
	double timeInSecs_main = (cEnd_main - cBegin_main) / (float) CLOCKS_PER_SEC;
	cout << "Total time spent for this image = " << timeInSecs_main << "secs\n";
	outFileLog << "\nTotal time spent for this image = " << timeInSecs_main << "secs\n";
	outFileLog.close();
	if (!SAVE_LOG_FILE) {
		remove((imageDir + "OCR_log.txt").c_str());
	}
	return 0;
}

string removeExtension(string fileNameFull) {
	int indx = fileNameFull.rfind(".");
	string fileName = fileNameFull.substr(0, indx);
	return fileName;
}

void handleImageOption(string imagePath, int blockCount, CvRect *textBlocks, const char *outputXmlPath = NULL) {
	vector<CvRect> textBlocksVector;
	for (int b = 0; b < blockCount; b++) {
		textBlocksVector.push_back(textBlocks[b]);
	}
	performOCR(imagePath, extractFileName(imagePath), blockCount > 0 ? &textBlocksVector : NULL, NULL, outputXmlPath);
}

void handleImageOption(string imagePath, const char *blockXmlPath = NULL, const char *outputXmlPath = NULL) {
	int blockCount = 0;
	CvRect textBlocks[100];
	if (blockXmlPath != NULL) {
		blockCount = readBlocksFromXML(blockXmlPath, textBlocks);
	}
	handleImageOption(imagePath, blockCount, textBlocks, outputXmlPath);
}

void handleDirOption(string baseDir) {
	DIR *dp;
	struct dirent *dirp;
	dp = opendir(baseDir.c_str());
	if (dp == NULL) {
		cout << "Error opening the input directory : " << baseDir << endl;
	} else {
		while ((dirp = readdir(dp))) {
			if (dirp->d_name[0] != '.' && strlen(dirp->d_name) > 3 && strcmp(dirp->d_name, "Thumbs.db")) {
				performOCR(baseDir + PATH_SEPARATOR + dirp->d_name, dirp->d_name);
			}
		}
	}
	closedir(dp);
}

void handleMultiPageTiffOption(string imagePath) {
	string tiffDir = "MultipageTiffInputs" + PATH_SEPARATOR + extractFileName(imagePath) + PATH_SEPARATOR;
	makeDirectory("MultipageTiffInputs");
	makeDirectory(tiffDir.c_str());

	TIFF *tiffInput, *tiffOutput;
	char suffixName[10];
	int pageNum = 0;
	string outputPath;
	tiffInput = TIFFOpen(imagePath.c_str(), "r");
	if (tiffInput != NULL) {
		do {
			pageNum++;
			sprintf(suffixName, "_%04d.tif", pageNum);
			outputPath = tiffDir + removeExtension(extractFileName(imagePath)) + suffixName;
			tiffOutput = TIFFOpen(outputPath.c_str(), TIFFIsBigEndian(tiffInput)?"wb":"wl");
			if (tiffOutput == NULL)
				cerr << "Unable to create the output split TIFF files.";
			if (!copyTiffImage(tiffInput, tiffOutput))
				cerr << "Unable to split the input TIFF file.";
			TIFFClose(tiffOutput);
			performOCR(outputPath, extractFileName(outputPath));
		} while (TIFFReadDirectory(tiffInput));
		(void) TIFFClose(tiffInput);
	}
}

static const string MENU_OPTION = "-menu";
static const string IMG_OPTION = "-img";
static const string MULTIPAGE_OPTION = "-multipage";
static const string DIR_OPTION = "-dir";
static const string SERVER_OPTION = "-server";

void printHelp() {
	cout << "Please invoke the program in one of the following ways:\n";
	cout << "a) KannadaClassifier.exe " + IMG_OPTION + " <image_path> <blocksXML_path>\n";
	cout << "b) KannadaClassifier.exe " + MULTIPAGE_OPTION + " <image_path>\n";
	cout << "c) KannadaClassifier.exe " + DIR_OPTION + " <path_of_directory_containing_input_images>\n";
	cout << "d) KannadaClassifier.exe " + string("<input_xml> <output.xml> <output_filename_prefix>\n");
	cout << "e) KannadaClassifier.exe " + MENU_OPTION + "\n";
	cout << "e) KannadaClassifier.exe " + SERVER_OPTION + "\n";
}

void printMenu() {
	//system(CLEAR_SCREEN_CMD.c_str());
	cout << "Please select from one of the following choices:\n";
	cout << "1: Run OCR on an input image\n";
	cout << "2: Run OCR on a multi-page TIFF input image\n";
	cout << "3: Run OCR on all images in a directory\n";
	cout << "4: Quit\n";
	cout << "\nEnter your choice number: ";
}

int saveNetworkDataToFile(int socketId, const char *filePath) {
	FILE* file = fopen(filePath, "w+");
	if (file == NULL) {
		perror("Error saving network data to file: ");
		return -1;
	}
	char buff[1024];
	int bytesRead;
	do {
		bytesRead = read(socketId, buff, 1024);
		if (bytesRead < 0) {
			perror("Error reading from network socket\n");
			fclose(file);
			return -1;
		} else if (bytesRead > 0) {
			fwrite(buff, 1, bytesRead, file);
		}
	} while (bytesRead != 0);
	fclose(file);
	return 0;
}

int sendFileOverNetwork(int socketId, const char *filePath) {
	FILE *file = fopen(filePath, "r");
	if (file == NULL) {
		return -1;
	}
	char buff[1024];
	int bytesRead;
	do {
		bytesRead = fread(buff, 1, 1024, file);
		if (bytesRead < 0) {
			perror("Error reading from file\n");
			fclose(file);
			return -1;
		} else if (bytesRead > 0) {
			write(socketId, buff, bytesRead);
		}
	} while (bytesRead != 0);
	return 0;

}

void* handleClient(void *arg) {
#define OCR_WORK_DIR "/tmp"
#define MAX_PATH_LEN 256
	int socketId = ((unsigned long long)arg);
	cout << "Inside new client request with socketId = " << socketId << "\n";

	char basePath[MAX_PATH_LEN];
	char inputXmlPath[MAX_PATH_LEN];
	char inputImagePath[MAX_PATH_LEN];
	char outputXmlPath[MAX_PATH_LEN];
	sprintf(basePath, "%s/%d", OCR_WORK_DIR, socketId);
	sprintf(inputXmlPath, "%s.xml", basePath);
	sprintf(inputImagePath, "%s.tif", basePath);
	sprintf(outputXmlPath, "%s_output.xml", basePath);

	if (saveNetworkDataToFile(socketId, inputXmlPath) < 0) {
		close(socketId);
		return NULL;
	}
	CvRect textBlocks[100];
	int blockCount = readBlocksAndImageFromXML(inputXmlPath, textBlocks, inputImagePath);
	remove(inputXmlPath);
	handleImageOption(inputImagePath, blockCount, textBlocks, outputXmlPath);
	remove(inputImagePath);
	sendFileOverNetwork(socketId, outputXmlPath);
	remove(outputXmlPath);
	close(socketId);
	return NULL;
}

int getServerPortNumber() {
#define DEFAULT_OCR_SERVER_PORT 8182
	char *port = getenv("OCR_SERVER_PORT_NUMBER");
	if (port == NULL) {
		return DEFAULT_OCR_SERVER_PORT;
	} else {
		return atoi(port);
	}
}

int startServer() {
	int serverSocketId, clientSocketId;
	struct sockaddr_in address;
	int addrlen = sizeof(address);
	pthread_t thread;
	int serverPortNumber = getServerPortNumber();
	// Creating socket file descriptor
	if ((serverSocketId = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		perror("Server socket creation failed");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(serverPortNumber);

	if (bind(serverSocketId, (struct sockaddr *)&address, sizeof(address)) < 0) {
		perror("Server port bind failed");
		exit(EXIT_FAILURE);
	}
	if (listen(serverSocketId, 3) < 0) {
		perror("Setting server queue size failed");
		exit(EXIT_FAILURE);
	}
	printf("Listening on port %d ...\n", serverPortNumber);
	while(1) {
		if ((clientSocketId = accept(serverSocketId, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
			perror("Error while listening for socket connections");
			exit(EXIT_FAILURE);
		}
		pthread_create(&thread, NULL, handleClient, (void*)clientSocketId);
	}
}

void processArguments(int argc, char** argv) {
	if (argc < 2 || argc > 4) {
		printHelp();
		exit(1);
	}
	string subCommand = argv[1];
	if (argc == 2 && subCommand == MENU_OPTION) {
		string imagePath;
		string baseDir;
		string inputXml;
		string outputXml;
		string outputPrefix;
		int choice;
		string tempStr;
		do {
			printMenu();
			cin >> tempStr;
			choice = atoi(tempStr.c_str());
			switch (choice) {
			case 1:
				cout << "\nEnter the path of input image: ";
				cin >> imagePath;
				cout << "\nEnter the path of blocks XML: ";
				cin >> inputXml;
				handleImageOption(imagePath, inputXml.c_str());
				cout << "\nPress any key followed by Enter to continue\n";
				cin >> tempStr;
				break;
			case 2:
				cout << "\nEnter the path of input image: ";
				cin >> imagePath;
				handleMultiPageTiffOption(imagePath);
				cout << "\nPress any key followed by Enter to continue\n";
				cin >> tempStr;
				break;
			case 3:
				cout << "\nEnter the path of directory where input images are stored: ";
				cin >> baseDir;
				handleDirOption(baseDir);
				cout << "\nPress any key followed by Enter to continue\n";
				cin >> tempStr;
				break;
			case 4:
				cout << "Thanks for using the Kannada OCR developed by MILE Lab, IISc.";
				cout << "\nThe program will quit now...\n";
				break;
			default:
				printMenu();
			}
		} while (choice != 5);
	} else if (argc == 3 && subCommand == IMG_OPTION) {
		handleImageOption(argv[2]);
	} else if (argc == 4 && subCommand == IMG_OPTION) {
		handleImageOption(argv[2], argv[3]);
	} else if (argc == 3 && subCommand == MULTIPAGE_OPTION) {
		handleMultiPageTiffOption(argv[2]);
	} else if (argc == 3 && subCommand == DIR_OPTION) {
		handleDirOption(argv[2]);
	} else if (argc == 2 && subCommand == SERVER_OPTION) {
		startServer();
	} else {
		printHelp();
		exit(1);
	}
}

int main_user(int argc, char** argv) {
	processArguments(argc, argv);

	if (metaDataLoaded) {
		if (USE_LINEAR_SVM) {
			deleteMetaData2();
		} else {
			deleteMetaData();
		}
	}
	return 0;
}

int main_train() {
	generateTrainingDataForSVM("trainingData\\KannadaBaseSamplesList.txt",
			"trainingData\\LinearSVM_KanBase_19Aug2010.txt", 32, 32);
	generateTrainingDataForSVM("trainingData\\KannadaOttuSamplesList.txt",
			"trainingData\\LinearSVM_KanOttu_19Aug2010.txt", 16, 16);
	generateTrainingDataForSVM("trainingData\\KannadaSpecialSymbolsSamplesList.txt",
			"trainingData\\LinearSVM_KanSpecialSymbols_19Aug2010.txt", 32, 32, true);
	return 0;
}

int main(int argc, char** argv) {
	return main_user(argc, argv);
	// return main_train();
}
