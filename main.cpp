#include <iostream>
#include <opencv2/opencv.hpp>
#include <filesystem>

class Action {
public:
	virtual ~Action() {}
	virtual void execute(std::filesystem::path image, std::filesystem::path targetDir) = 0;

	static Action *getActionFromChar(char key);
};

class SkipImage : public Action {
public:
	void execute(std::filesystem::path image, std::filesystem::path targetDir) override {
		return;
	}
};

class MoveImage : public Action {
public:
	void execute(std::filesystem::path image, std::filesystem::path targetDir) override {
		std::filesystem::rename(image, targetDir / image.filename());
	}
};

class CopyImage : public Action {
public:
	void execute(std::filesystem::path image, std::filesystem::path targetDir) override {
		std::filesystem::copy_file(image, targetDir / image.filename());
	}
};

std::filesystem::path getDirectory(const char* inputMessage);
std::filesystem::path getPathInput(const char * inputMessage);

void filterImagesInPath(std::filesystem::path imageDirectoryPath, std::filesystem::path targetDir);
void processEntry(std::filesystem::path entry, std::filesystem::path targetDir);
bool isValidImageFile(std::filesystem::path imagePath);

void showImage(std::filesystem::path imagePath);
Action* listenToAction();

int main() {
	for (;;) {
		try
		{
			std::filesystem::path sourceDir = getDirectory("Source Dir: ");
			std::filesystem::path targetDir = getDirectory("Target Dir: ");
			filterImagesInPath(sourceDir, targetDir);
		}
		catch (const std::exception& e)
		{
			std::cout << "Warning: Directory does not exist or is not directory: " << e.what() << std::endl;
		}
	}
	
	return 0;
}

std::filesystem::path getDirectory(const char* inputMessage) {
	namespace fs = std::filesystem;
	fs::path filePath = getPathInput(inputMessage);
	bool pathIsValid = fs::exists(filePath) && fs::is_directory(filePath);

	if (!pathIsValid) {
		std::exception invalidPathException{ "Path does not exist or is not directory" };
		throw invalidPathException;
	}

	return filePath;
}

std::filesystem::path getPathInput(const char* inputMessage) {
	std::cout << inputMessage;
	std::string inputFilePath;
	std::cin >> inputFilePath;

	return { inputFilePath };
}


void filterImagesInPath(std::filesystem::path imageDirectoryPath, std::filesystem::path targetDir) {
	for (const auto& entry : std::filesystem::directory_iterator(imageDirectoryPath)) {
		processEntry(entry.path(), targetDir);
	}
}

void processEntry(std::filesystem::path entry, std::filesystem::path targetDir) {
	if (!isValidImageFile(entry)) {
		return;
	}

	showImage(entry);
	Action *action = listenToAction();
	action->execute(entry, targetDir);
	cv::destroyAllWindows();
}

bool isValidImageFile(std::filesystem::path imagePath) {
	std::vector<std::filesystem::path> validExtensions{ ".jpeg", ".png", ".jpg", ".tif", ".tiff", ".jp2" };
	auto extension = imagePath.extension();
	bool correctExtension = std::find(validExtensions.begin(), validExtensions.end(), extension) != validExtensions.end();
	bool isFile = std::filesystem::is_regular_file(imagePath);

	return isFile && correctExtension;
}

void showImage(std::filesystem::path imagePath)
{
	cv::Mat im = cv::imread(imagePath.string(), cv::WindowFlags::WINDOW_NORMAL);

	cv::imshow("Image", im);
	cv::resizeWindow("Image", 600, 600);
}

Action *listenToAction()
{
	int key = cv::waitKey(0) & 0xFF;

	return Action::getActionFromChar(key);
}


Action *Action::getActionFromChar(char key) {
	if (key == 'm' || key == 'M') {
		return new MoveImage;
	}

	if (key == 's' || key == 'S') {
		return new SkipImage;
	}

	if (key == 'c' || key == 'C') {
		return new CopyImage;
	}
}