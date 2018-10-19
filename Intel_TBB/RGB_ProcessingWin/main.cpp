#pragma region Libaries&Namespaces

#include <iostream>
#include <vector>
//Thread building blocks library
#include <tbb/task_scheduler_init.h>
#include <tbb/parallel_for.h>
//Free Image library
#include <FreeImagePlus.h>
#include<thread>
#include <mutex>
#include <chrono>
#include <cmath>
#include <math.h>
#include <tbb/blocked_range.h>
#include <tbb/blocked_range2d.h>

std::mutex ilock;

using namespace std;
using namespace tbb;
using namespace std::chrono;

#pragma endregion

#pragma region FunctionDefinitions

void Sequential();
void Parallel();
void Part2();

#pragma endregion

int main()
{

	int nt = task_scheduler_init::default_num_threads();
	task_scheduler_init T(nt);

	//Part 1 (Greyscale Gaussian blur): -----------DO NOT REMOVE THIS COMMENT----------------------------//
	Sequential();
	Parallel();
	//Part 2 (Colour image processing): -----------DO NOT REMOVE THIS COMMENT----------------------------//
	Part2();
	return 0;
}

#pragma region Function definitions

void Sequential()
{
	cout << "-------------------" << endl;
	cout << "Start of Sequential" << endl;

#pragma region StartSequentialTime
	// Record start time
	auto startsequential = std::chrono::high_resolution_clock::now();
#pragma endregion

#pragma region Loadimage
	//Load image
	fipImage InputImage;
	//InputImage.load("../Images/render_1.png");
	InputImage.load("../Images/1.jpg");
	InputImage.convertToFloat();

	auto width1 = InputImage.getWidth();
	auto height1 = InputImage.getHeight();
	const float* const inputBuffer = (float*)InputImage.accessPixels();
#pragma endregion

#pragma region outputImageArray
	// Setup output image array
	fipImage outputImage0;
	outputImage0 = fipImage(FIT_FLOAT, width1, height1, 32);
	float *outputBuffer = (float*)outputImage0.accessPixels();
#pragma endregion

#pragma region KernalVariables
	// Sequential Code here
	int const kernalSize = 13;
	int halfKernal = (kernalSize / 2);
	double sigma = 5.0;
	double sum = 0;
#pragma endregion

#pragma region generateKernal

	// generate kernel
	double GKernel[kernalSize][kernalSize];
	for (int x = -halfKernal; x <= halfKernal; x++)
	{
		for (int y = -halfKernal; y <= halfKernal; y++)
		{
			GKernel[halfKernal + x][halfKernal + y] = 1.0f / (2.0f * 3.14 * sigma * sigma) * exp(-((x * x + y * y) / (2.0f * sigma * sigma)));
			sum += GKernel[x + halfKernal][y + halfKernal];
		}
	}
	cout << endl << "Kernal generated" << endl << endl;

#pragma endregion

#pragma region normalisekernal

	// normalising the Kernel
	for (int i = 0; i < kernalSize; i++)
	{
		for (int j = 0; j < kernalSize; j++)
		{
			GKernel[i][j] /= sum;
			cout << GKernel[i][j] << '\t';
		}
		cout << endl;
	}

	cout << endl << "Kernal normalised" << endl;

#pragma endregion

#pragma  region  SequencialBlur

	for (int y = halfKernal; y < height1 - halfKernal; y++)
	{
		for (int x = halfKernal; x < width1 - halfKernal; x++)
		{
			for (int y2 = 0; y2 < kernalSize; y2++)
			{
				for (int x2 = 0; x2 < kernalSize; x2++)
				{
					outputBuffer[y*width1 + x] += inputBuffer[(y - (y2 - halfKernal)) * width1 + (x - (x2 - halfKernal))] * GKernel[x2][y2];
				}
			}
		}
	}

	cout << endl << "Image Blurred" << endl;

#pragma endregion

#pragma region ConvertAndSaveImage
	// Convert & save Image
	outputImage0.convertToType(FREE_IMAGE_TYPE::FIT_BITMAP);
	outputImage0.convertTo24Bits();
	outputImage0.save("../Images/Grey_blurred.png");
	cout << "Image saved (Sequential)" << endl;
#pragma endregion

#pragma region EndAndOutputTime
	// Record end time
	auto finishsequential = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed = finishsequential - startsequential;
	// Output elapsed time
	std::cout << "Total  time:  " << elapsed.count() << " seconds" << std::endl;
#pragma endregion

	cout << "End of Sequential" << endl;
}
void Parallel()
{
	cout << "-----------------" << endl;
	cout << "Start of Parallel" << endl << endl;;

#pragma region StartParallelTime
	// Record start time
	auto startparallel = std::chrono::high_resolution_clock::now();
#pragma endregion

#pragma region LoadImage
	// Parallel
	fipImage InputImage;
	//InputImage.load("../Images/render_1.png");
	InputImage.load("../Images/1.jpg");
	InputImage.convertToFloat();

	auto width1 = InputImage.getWidth();
	auto height1 = InputImage.getHeight();
	const float* const inputBuffer = (float*)InputImage.accessPixels();
#pragma endregion

#pragma region OutputImageArray
	//    // Setup output image array
	fipImage outputImage0;
	outputImage0 = fipImage(FIT_FLOAT, width1, height1, 32);
	float *outputBuffer = (float*)outputImage0.accessPixels();
#pragma endregion

#pragma region  KernalVariables
	int const kernalSize = 13;
	int halfKernal = ((kernalSize-1) / 2);
	double sigma = 5;
	double sum = 0;
#pragma endregion

#pragma region generateKernal

	// generate kernel
	double GKernel[kernalSize][kernalSize];
	for (int x = -halfKernal; x <= halfKernal; x++)
	{
		for (int y = -halfKernal; y <= halfKernal; y++)
		{
			GKernel[halfKernal + x][halfKernal + y] = 1.0f / (2.0f * 3.14 * sigma * sigma) * exp(-((x * x + y * y) / (2.0f * sigma * sigma)));
			sum += GKernel[x + halfKernal][y + halfKernal];
		}
	}

	cout << endl << "Kernal generated" << endl << endl;

#pragma endregion

#pragma region normalisekernal

	// normalising the Kernel
	for (int i = 0; i < kernalSize; i++)
	{
		for (int j = 0; j < kernalSize; j++)
		{
			GKernel[i][j] /= sum;
			cout << GKernel[i][j] << '\t';
		}
		cout << endl;
	}

	cout << endl << "Kernal normalised" << endl;

#pragma endregion

#pragma region ParallelBlur

	parallel_for(blocked_range2d<uint64_t, uint64_t>(halfKernal, (height1 - halfKernal), halfKernal, (width1 - halfKernal)), [&](const blocked_range2d<uint64_t>& r)
	{
		auto x1 = r.rows().begin();
		auto x2 = r.rows().end();
		auto y1 = r.cols().begin();
		auto y2 = r.cols().end();

		for (int x = x1; x < x2; x++)
		{
			for (int y = y1; y < y2; y++)
			{
				for (int i = 0; i < kernalSize; i++)
				{
					for (int j = 0; j < kernalSize; j++)
					{
						outputBuffer[x * width1 + y] += inputBuffer[(x - (i - halfKernal)) * width1 + (y - (j - halfKernal))] * GKernel[j][i];
					}
				}
			}
		}
	});

	cout << endl << "Image Blurred" << endl;

#pragma endregion

#pragma region ConvertAndSaveImage

	outputImage0.convertToType(FREE_IMAGE_TYPE::FIT_BITMAP);
	outputImage0.convertTo24Bits();
	outputImage0.save("../Images/Grey_blurred.png");
	cout << "Image saved (Parallel)" << endl;

#pragma endregion

# pragma region EndAndOutputTime
	// Record end time
	auto finishparallel = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed = finishparallel - startparallel;
	// Output elapsed time
	std::cout << "Total  time:  " << elapsed.count() << " seconds" << std::endl;
#pragma  endregion

	cout << "End of Parallel" << endl;

}
void Part2()
{
	cout << "---------------" << endl;
	cout << "Start of Part 2" << endl << endl;;

#pragma region StartPart2Time
	// Record start time
	auto Part2Start = high_resolution_clock::now();
#pragma endregion

#pragma region  LoadImage1

	// Setup Input image array
	fipImage inputImage;
	inputImage.load("../Images/render_1.png");
#pragma endregion

#pragma region LoadImage2
	// Setup Input image array 2
	fipImage inputImage2;
	inputImage2.load("../Images/render_2.png");
#pragma endregion

#pragma region GetImageDimensions
	unsigned int width = inputImage.getWidth();
	unsigned int height = inputImage.getHeight();
#pragma endregion

#pragma region Image1Variables

	//2D Vector to hold the RGB colour data of image 1
	vector<vector<RGBQUAD>> rgbValues;
	rgbValues.resize(height, vector<RGBQUAD>(width));
	//FreeImage structure to hold RGB values of a single pixel
	RGBQUAD rgb;  

#pragma endregion

#pragma region Image2Variables

	//2D Vector to hold the RGB colour data of an image
	vector<vector<RGBQUAD>> rgbValues2;
	rgbValues2.resize(height, vector<RGBQUAD>(width));
	//FreeImage structure to hold RGB values of a single pixel
	RGBQUAD rgb2;  

#pragma endregion

#pragma region OutputImageArray1
	// Setup Output image array
	fipImage outputImage;
	outputImage = fipImage(FIT_BITMAP, width, height, 24);
#pragma endregion

#pragma region OutputImageArray2
	// Setup Output image array 2
	fipImage outputImage2;
	outputImage2 = fipImage(FIT_BITMAP, width, height, 24);
#pragma endregion


#pragma region ExtractImage1RGB

	//Extract colour data from image and store it as individual RGBQUAD elements for every pixel
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			inputImage.getPixelColor(x, y, &rgb); //Extract pixel(x,y) colour data and place it in rgb
			rgbValues[y][x].rgbRed = rgb.rgbRed;
			rgbValues[y][x].rgbGreen = rgb.rgbGreen;
			rgbValues[y][x].rgbBlue = rgb.rgbBlue;
		}
	}

#pragma endregion

#pragma region ExtractImage2RGB
	//Extract colour data from image and store it as individual RGBQUAD elements for every pixel
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			inputImage2.getPixelColor(x, y, &rgb2); //Extract pixel(x,y) colour data and place it in rgb
			rgbValues2[y][x].rgbRed = rgb2.rgbRed;
			rgbValues2[y][x].rgbGreen = rgb2.rgbGreen;
			rgbValues2[y][x].rgbBlue = rgb2.rgbBlue;
		}
	}
#pragma endregion

	//Place the pixel colour values into output image 1
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			outputImage.setPixelColor(x, y, &rgbValues[y][x]);
		}
	}

	//Place the pixel colour values into output image 2
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			outputImage2.setPixelColor(x, y, &rgbValues2[y][x]);
		}
	}

	fipImage take;
	take = fipImage(FIT_BITMAP, width, height, 24);

	fipImage out;
	out = fipImage(FIT_BITMAP, width, height, 24);

#pragma region ResultingImageVariables
	vector<vector<RGBQUAD>> subtract;
	subtract.resize(height, vector<RGBQUAD>(width));
#pragma endregion

	const int threshold = 0;
	int a = 0;
	int b = 0;

	parallel_for(blocked_range2d<uint64_t, uint64_t>(0, height, 8, 0, width, 8), [&](const blocked_range2d<uint64_t, uint64_t>&r)
	{
		auto g1 = r.rows().begin();
		auto g2 = r.rows().end();
		auto h1 = r.cols().begin();
		auto h2 = r.cols().end();

		RGBQUAD rtake;

		

		for (uint64_t g = g1; g<g2; g++)
		{
			for (uint64_t h = h1; h<h2; h++)
			{
				// Extract pixel (x,y) colour data and place it in rgb
				take.getPixelColor(h, g, &rtake); 

				subtract[g][h].rgbRed = (unsigned char)abs(rgbValues[g][h].rgbRed - rgbValues2[g][h].rgbRed);
				subtract[g][h].rgbGreen = (unsigned char)abs(rgbValues[g][h].rgbGreen - rgbValues2[g][h].rgbGreen);
				subtract[g][h].rgbBlue = (unsigned char)abs(rgbValues[g][h].rgbBlue - rgbValues2[g][h].rgbBlue);

				// White
				if (subtract[g][h].rgbRed > threshold && subtract[g][h].rgbBlue > threshold && subtract[g][h].rgbGreen > threshold)
				{
					subtract[g][h].rgbRed = 255;
					subtract[g][h].rgbGreen = 255;
					subtract[g][h].rgbBlue = 255;

					a += 1;

				}
				else if (subtract[g][h].rgbRed < threshold && subtract[g][h].rgbBlue < threshold && subtract[g][h].rgbGreen < threshold)
				{
					subtract[g][h].rgbRed = 0;
					subtract[g][h].rgbGreen = 0;
					subtract[g][h].rgbBlue = 0;

					b += 1;

				}
				out.setPixelColor(h, g, &subtract[g][h]);
			}
		}
	});

	cout << "Number of white pixels: " << a << endl;
	cout << "Processed" << endl;

#pragma region SaveImage
	//Save the processed image
	out.save("../Images/RGB_processed.png");
#pragma endregion

# pragma region EndAndOutputTime
	auto Part2Finish = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(Part2Finish - Part2Start);
#pragma endregion

	cout << "End of Part 2" << endl;


}

#pragma endregion