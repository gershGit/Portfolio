/*
* PNG Plus image compression concept
* Developed by Noah Gershmel 
* Originally created 11/14/2017
*/


#include <cstdio>
#include <stdio.h>
#include <thread>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <queue>
#include <stack>
#include <map>
#include "Node.h"
#include "d:\Documents\Visual Studio 2017\Libraries\bitmap\bitmap_image.hpp"

using namespace std;

/*Stucture to control placement into the Priority Queue*/
struct MoreThanByWeight {
	bool operator()(const Node* lhs, const Node* rhs)
	{
		return lhs->weight > rhs->weight; //Lower weights need to be at the front of the queue
	}
};

/*Saves the "Image" as a text file to be compared against other compression*/
void saveImage(vector<vector<string>>  &compressedImage, int cropValue) {
	//Open the file and give it a file type of pngp (PNG Plus)
	ofstream fileWriter;
	string s = "CompressedImages\\Image_";
	s.append(to_string(cropValue));
	s.append(".pngp");
	fileWriter.open(s);
	//On each line write the mapped value for each number
	for (int i = 0; i < compressedImage.size(); i++) {
		for (int j = 0; j < compressedImage[i].size(); j++) {
			fileWriter << compressedImage[i][j];
		}
		if (i < compressedImage.size() - 1) {
			fileWriter << "\n";
		}
	}
	fileWriter.close();
}

/*Create a node for every value that exists in the filtered image*/
void CreateNodes(vector<vector<int>> &filterImage, priority_queue<Node*, vector<Node*>, MoreThanByWeight> &myQueue) {
	//Loop through the possible values of the filtered image and count each frequency
	int numFrequency = 0;
	for (int i = -255; i < 256; i++) {
		for (int a = 0; a < filterImage.size(); a++) {
			for (int b = 0; b < filterImage[a].size(); b++) {
				if (filterImage[a][b] == i) {
					numFrequency++;
				}
			}
		}
		//If a value does appear in the filtered image it must have a node and be pushed onto the queue
		if (numFrequency > 0) {
			Node* newNode = new Node();
			newNode->weight = numFrequency;
			newNode->name = i;
			myQueue.push(newNode);
			numFrequency = 0;	//Reset the frequency for the next value
		}
	}
}

/*Creates a Huffman Tree out of the priority queue of nodes*/
Node* CreateTree(priority_queue<Node*, vector<Node*>, MoreThanByWeight> &myQueue) {
	//Initialize a root node with null values
	Node* rootNull;
	rootNull->name = -1;

	//Instantiate values and node pointers for the first two nodes in the queue
	int aNum;
	int bNum;
	Node* poppedNodeA;
	Node* poppedNodeB;

	//Continue building the huffman tree until the queue has been exhausted
	while (myQueue.size() > 0) {
		//Each addition to the tree requires a new root
		rootNull = new Node();
		
		//Pull and store the first two nodes/trees in the queue
		poppedNodeA = myQueue.top();
		myQueue.pop();
		poppedNodeB = myQueue.top();
		myQueue.pop();

		//Build a sub tree for these two nodes/trees using the rootnull as the parent
		rootNull->weight = poppedNodeA->weight + poppedNodeB->weight;
		rootNull->leftChild = poppedNodeA;
		rootNull->rightChild = poppedNodeB;
		
		//If the queue is empty our rootnull is now the root of the entire tree
		if (myQueue.size() == 0) {
			return rootNull;
		}
		else {
			//Otherwise we must push the newly created tree back onto the queue
			myQueue.push(rootNull);
		}
	}
	return rootNull; //This occurs if the while loop catches that the queue is empty, it should never actually be used
}

/*Creates a map out of the huffman tree so that values can be retrieved for each number*/
void CreateMap(Node* currentNode, string currentCode, stack<Node*> s, string biteCode, map<int, string> &myMap) {	
	if (currentNode->getLeftChild() == NULL && currentNode->getRightChild() == NULL) {
		//If the current node has no children then whatever code we are at is what should be assigned to it
		myMap.insert(pair<int, string>(currentNode->name, currentCode));
		return;
	}
	else {
		s.push(currentNode); //Otherwise push this node onto the stack so it can be returned to for its children
		biteCode = biteCode.append("0");  //Add a zero to the code as we move left
		CreateMap(currentNode->leftChild, biteCode, s, biteCode, myMap);

		biteCode = biteCode.substr(0, biteCode.size() - 1);  //Take off the last number in the code to reset to the parent
		biteCode = biteCode.append("1"); //Add a 1 as we are now moving right
		CreateMap(currentNode->rightChild, biteCode, s, biteCode, myMap);
		s.pop();
		biteCode = biteCode.substr(0, biteCode.size() - 1); //Take off the last number in the code to reset to the parent
	}
}

/*Function that populates a 2D array with the string values for each number in the filtered image*/
void Compress(vector<vector<string>> &compressedArray, vector<vector<int>> &filterImage, map<int, string> myMap) {
	//Loop through the 2D array and insert the mapped value for each pixel
	for (int i = 0; i < compressedArray.size(); i++){
		for (int j = 0; j < compressedArray[i].size(); j++) {
			compressedArray[i][j] = myMap.at(filterImage[i][j]);
		}
	}
}

/*Control function that takes a filtered image and runs each stage of the compression, including a call to saving the compressed image*/
void compressImage(vector<vector<int>> &filterImage, int cropValue) {
	priority_queue<Node*, vector<Node*>, MoreThanByWeight> myQueue; //Create a new priority queue for the compression of this image
	stack<Node*> s; //The stack is used to create a map in the recursive function
	string biteCode = "";	//This string is used when creating a map to stand in for bit values
	map<int, string> myMap;	//A map that matches a standard integer value to the compressed form from the huffman tree
	
	Node* root = new Node(); //This node pointer will be the root of the tree returned by the create tree function
	string compressedString = "";	//A string that can be updated each recursive call to determine the string imitating the compressed form of a value during the creation of the map
	printf("Creating Huffman Tree %d\n", cropValue);	//Line to track the progress of our algorithm
	CreateNodes(filterImage, myQueue);	//Call to create nodes function for this image
	root = CreateTree(myQueue);		//Set our node to the tree created by the CreateTree function
	CreateMap(root, biteCode, s, biteCode, myMap);	//Create a map out of the tree
	printf("Created Map %d\n", cropValue);	//Line to track the progress of our algorithm

	//Create a new 2D array to store the compressed form of the picture
	vector<vector<string>> compressedArray;		
	compressedArray.resize(filterImage.size());
	for (int i = 0; i < filterImage.size(); ++i)
		compressedArray[i].resize(filterImage[i].size() - cropValue);
	for (int i = 0; i < filterImage.size(); i++) {
		for (int j = 0; j < filterImage[i].size() - cropValue; j++) {
			compressedArray[i][j] = "0";
		}
	} 
	printf("Creating compressed image %d from map\n", cropValue);	//Line to track the progress of our algorithm
	Compress(compressedArray, filterImage, myMap);	//Populate the newly created array with the mapped values
	printf("Saving image %d\n", cropValue);	//Line to track the progress of our algorithm
	saveImage(compressedArray, cropValue);	//Save the image into a text file to compare the results of our compression
}

/*A simplified version of PNG filtering that determines value based only on the difference between each pixel and the pixel to the left
* In addition this filter is called on a single channel of an image
*/
void filter(int cropValue, vector<vector<int>> &image, vector<vector<int>>  &filterImage) {
	printf("Beginning filter on filterImage with crop value of %d\n", cropValue);	//Line to track algorithms progress
	//Loop through the array and set each value (except the left column) to the difference between it and the value to its left
	for (int i = 0; i < filterImage.size(); i++) {
		for (int j = filterImage[i].size()-1; j > 0; j--) {
			filterImage[i][j] = image[i][j - 1] - image[i][j];
		}
	}
	//We can now run standard huffman compression on this filtered version of the image
	compressImage(filterImage, cropValue);
}

/*This function simply returns a 2D representation of an images red channel from the name of images file*/
vector<vector<int>> getImageArray(string fileName) {
	printf("Opening image\n");	//Line to track our algorithms progress

	vector<vector<int>> retImage; //Array to store the result of our function
	bitmap_image realImage(fileName);	//Call to library that loads a bmp file
	//Ensure that our image was correctly loaded before continueing
	if (!realImage)
	{
		printf("Error - Failed to open: %s\n", fileName);
		return retImage;
	}
	else {
		//If the image was correctly loadrd, populate the 2D array after resizing it
		int width = realImage.width();
		retImage.resize(realImage.height());	//Resize the height of the array to the images height
		for (int y = 0; y < realImage.height(); y++) {	//Resize the width of the array to the images width
			retImage[y].resize(width);
			for (int x = 0; x < width; x++) {	//Populate the array with the red value of each pixel
				rgb_t colour;
				realImage.get_pixel(x, y, colour);
				retImage[y][x] = colour.red;
			}
		}

		printf("Successfully Loaded image\n");	//Line to track algorithms progress
		return retImage;
	}
}

/*Main function that runs 4 different crops of an image through basic PNG compression
*The results of the different compressions are compared to find the best compression 
*Information is also outputted to the console so that it can be used in data analysis
*/
int main()
{
	//Prompt the user for which file in the associated image folder they would like to run the test on
	string fileName;
	printf("Please enter the name of the image\n-----DO NOT INCLUDE THE FOLDER NAME OR THE EXTENSION-----\n");
	string myImageName = "Images\\"; 
	cin >> fileName;
	myImageName.append(fileName);
	myImageName.append(".bmp");
	//Get a vector representation of the inputed image
	vector<vector<int>> image = getImageArray(myImageName);

	printf("Retrieved image array\n");	//Line tracks our algorithms progress

	//3D array that simply stores 4 2D arrays
	vector<vector<vector<int>>*> filteredImages;
	filteredImages.resize(4);

	//Create 4 different croppings of the image, add them to the 3D array and run the filtering/compression on a new thread
	//The number of logical cores was known before hand for the system to test this on so hard coding 4 threads created more understandable code
	//-----------------------------------------------
	vector<vector<int>> filterImageA;
	filterImageA.resize(image.size());
	for (int a = 0; a < image.size(); ++a)
		filterImageA[a].resize(image[a].size());

	filteredImages[0] = &filterImageA;
	thread imgThreadA(filter, 0, image, filterImageA);

	vector<vector<int>> filterImageB;
	filterImageB.resize(image.size());
	for (int a = 0; a < image.size(); ++a)
		filterImageB[a].resize(image[a].size() - 2);

	filteredImages[1] = &filterImageB;
	thread imgThreadB(filter, 2, image, filterImageB);

	vector<vector<int>> filterImageC;
	filterImageC.resize(image.size());
	for (int a = 0; a < image.size(); ++a)
		filterImageC[a].resize(image[a].size() - 4);

	filteredImages[2] = &filterImageC;
	thread imgThreadC(filter, 4, image, filterImageC);

	vector<vector<int>> filterImageD;
	filterImageD.resize(image.size());
	for (int a = 0; a < image.size(); ++a)
		filterImageD[a].resize(image[a].size() - 8);

	filteredImages[3] = &filterImageD;
	thread imgThreadD(filter, 8, image, filterImageD);
	//------------------------------------------------

	//Join the four threads in preparation for the comparison
	imgThreadA.join();
	imgThreadB.join();
	imgThreadC.join();
	imgThreadD.join();

	//Saves a fake version of the bitmap image that is the correct size
	printf("Saving original image\n");
	ofstream fileWriter;
	fileWriter.open("CompressedImages\\Image_orig.pngp");
	for (int i = 0; i < image.size(); i++) {
		for (int j = 0; j < image[i].size(); j++) {
			fileWriter << "00000000";
		}
		if (i < image.size() - 1) {
			fileWriter << "\n";
		}
	}
	fileWriter.close();

	//Find the size of the original image to compare against
	ifstream maxFile("CompressedImages\\Image_orig.pngp", ios::binary | ios::ate);
	int oSize = maxFile.tellg();
	maxFile.close();

	//Run a comparison on our 4 images
	int minFile = 0;
	int minSize = 0;
	int pSize = 0;
	//Loop through our 4 different crop values
	for (int f = 0; f <= 8; f *= 2) {
		//Load each image from the compressed image folder and the size of the image
		string fName = "CompressedImages\\Image_";
		fName.append(to_string(f).append(".pngp"));
		ifstream maxFile(fName, ios::binary | ios::ate);
		int fSize = maxFile.tellg();
		maxFile.close();
		
		//If the current image is smaller than the current minimum we can check if the compression was worth it
		if (fSize < minSize) {		
			double compressionPercent = (double) fSize / pSize * 100;		//Determines what percent of the standard png images size the compressed image takes up
			double comparisonBytes = (((double) image.size() * (image[0].size() - f)) / (image.size() * image[0].size()) * 100);	//Determines how much of the original image remains
			if (compressionPercent < comparisonBytes) { //If the current image is compressed more than it is cropped save it as the new minmum
				minFile = f;
				minSize = fSize;
			}
		}
		//Crop value 0 is standard (uncropped PNG) so specific information for its file size must be saved
		if (f == 0) {
			minSize = fSize;
			pSize = fSize;
			f += 1;	//Special change to the iteration value to not get stuck in an infinite loop
		}

	}

	//Final output information for the user so that our algorithms performance can be determined across many images
	double compressionPercent = (double) minSize / pSize * 100;	//Percent of the standard png images size this image takes up
	double compressionTotal = ((double) minSize / (oSize)) * 100;	//Percent of the original images size this image takes up
	double comparisonBytes = ((double) image.size()*(image[0].size()-minFile)) / (image.size()*image[0].size()) * 100;	//Determines how much of the original image remains
	//Output for the user about the best compressed image
	printf("\n\nMinumum file where compression is greater than crop is %d with a size of %d KB \n", minFile, minSize/1000);
	printf("Compression compared to png standard: %.2lf %%\n", compressionPercent);
	printf("Compression compared to raw standard: %.2lf %%\n", compressionTotal);
	printf("%.2lf %% of image remains with compression of %.2lf %% compared to png\n", comparisonBytes, compressionPercent);

	//Hacky way to ensure the console doesn't close too soon on the user
	string endString;
	printf("\nEnter any string and press enter to exit . . .");
	cin >> endString;
	return 0;
}