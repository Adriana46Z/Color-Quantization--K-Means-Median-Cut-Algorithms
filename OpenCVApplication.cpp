// OpenCVApplication.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "common.h"
#include <random>
#include <set>
#include <vector>
#include <cmath>
#include <ctime>
#include <algorithm>
#include <opencv2/core/utils/logger.hpp>

// PROJECT - K-Means & Median Cut Color Quantization

struct ColorCompare {
	bool operator()(const cv::Vec3b& a, const cv::Vec3b& b) const {
		if (a[0] != b[0]) return a[0] < b[0];
		if (a[1] != b[1]) return a[1] < b[1];
		return a[2] < b[2];
	}
};

int countUniqueColors(const cv::Mat& img) {
	std::set<cv::Vec3b, ColorCompare> uniqueColors;
	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			uniqueColors.insert(img.at<cv::Vec3b>(i, j));
		}
	}
	return uniqueColors.size();
}

// 2. HELPER FUNCTIONS

// Calculates Euclidean distance in the 3D space

double getEuclideanDistance(const Vec3b& p1, const Vec3b& p2) {
	return sqrt(pow(p1[0] - p2[0], 2) + pow(p1[1] - p2[1], 2) + pow(p1[2] - p2[2], 2));
}

// Initializes K centroids by randomly selecting UNIQUE colors from the image

std::vector<Vec3b> initializeCentroidsUnique(const Mat& img, int K) {
	std::vector<Vec3b> centroids;
	std::set<Vec3b, ColorCompare> setUnique;

	srand(42);

	int maxAttempts = 100;
	int attempts = 0;

	while (centroids.size() < K && attempts < maxAttempts) {
		attempts++;
		int r = rand() % img.rows;
		int c = rand() % img.cols;
		Vec3b color = img.at<Vec3b>(r, c);

		if (setUnique.find(color) == setUnique.end()) {
			setUnique.insert(color);
			centroids.push_back(color);
		}
	}

	while (centroids.size() < K) {
		centroids.push_back(Vec3b(rand() % 256, rand() % 256, rand() % 256));
	}

	return centroids;
}

// Initializes K centroids in a purely random manner

std::vector<Vec3b> initializeCentroidsRandom(const Mat& img, int K) {
	std::vector<Vec3b> centroids;

	srand(time(NULL));

	for (int i = 0; i < K; i++) {
		int r = rand() % img.rows;
		int c = rand() % img.cols;

		Vec3b randomColor = img.at<Vec3b>(r, c);
		centroids.push_back(randomColor);
	}

	return centroids;
}


// 3. K-MEANS ALGORITHM IMPLEMENTATIONS

// K-Means with Optimal Unique Centroids Initialization

Mat kMeansColorQuantization(const Mat& img, int K) {
	std::vector<Vec3b> centroids = initializeCentroidsUnique(img, K);
	Mat labels(img.rows, img.cols, CV_32SC1, Scalar(0));
	int currentIteration = 0;

	printf("\n[K-Means Unique Sets] Starting algorithm...\n");

	while (true) {
		currentIteration++;

		//assign
		for (int i = 0; i < img.rows; i++) {
			for (int j = 0; j < img.cols; j++) {
				Vec3b pixel = img.at<Vec3b>(i, j);
				double minDist = 100000.0;
				int bestLabel = 0;

				for (int k = 0; k < K; k++) {
					double dist = getEuclideanDistance(pixel, centroids[k]);
					if (dist < minDist) {
						minDist = dist;
						bestLabel = k;
					}
				}
				labels.at<int>(i, j) = bestLabel;
			}
		}

		//update 
		std::vector<Vec3d> channelSum(K, Vec3d(0, 0, 0));
		std::vector<int> pixelCount(K, 0);

		for (int i = 0; i < img.rows; i++) {
			for (int j = 0; j < img.cols; j++) {
				int k = labels.at<int>(i, j);
				Vec3b p = img.at<Vec3b>(i, j);

				channelSum[k][0] += p[0]; // B Channel
				channelSum[k][1] += p[1]; // G Channel
				channelSum[k][2] += p[2]; // R Channel
				pixelCount[k]++;
			}
		}

		std::vector<Vec3b> newCentroids(K);
		for (int k = 0; k < K; k++) {
			if (pixelCount[k] > 0) {
				newCentroids[k][0] = (uchar)(channelSum[k][0] / pixelCount[k]);
				newCentroids[k][1] = (uchar)(channelSum[k][1] / pixelCount[k]);
				newCentroids[k][2] = (uchar)(channelSum[k][2] / pixelCount[k]);
			}
			else {
				newCentroids[k] = centroids[k];
			}
		}

		// convergence check step
		bool converged = true;
		for (int k = 0; k < K; k++) {
			if (getEuclideanDistance(centroids[k], newCentroids[k]) > 1.0) {
				converged = false;
				break;
			}
		}

		centroids = newCentroids;

		if (converged) {
			printf("[K-Means Unique Sets] Convergence reached at iteration %d.\n", currentIteration);
			break;
		}
	}

	// recostruct the image
	Mat result(img.size(), img.type());
	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			result.at<Vec3b>(i, j) = centroids[labels.at<int>(i, j)];
		}
	}

	return result;
}

// K-Means with Purely Random Centroids Initialization

Mat kMeansColorQuantizationRandom(const Mat& img, int K) {
	std::vector<Vec3b> centroids = initializeCentroidsRandom(img, K);
	Mat labels(img.rows, img.cols, CV_32SC1, Scalar(0));
	int currentIteration = 0;

	printf("\n[K-Means Random] Starting algorithm...\n");

	while (true) {
		currentIteration++;

		//assignment
		for (int i = 0; i < img.rows; i++) {
			for (int j = 0; j < img.cols; j++) {
				Vec3b pixel = img.at<Vec3b>(i, j);
				double minDist = 100000.0;
				int bestLabel = 0;

				for (int k = 0; k < K; k++) {
					double dist = getEuclideanDistance(pixel, centroids[k]);
					if (dist < minDist) {
						minDist = dist;
						bestLabel = k;
					}
				}
				labels.at<int>(i, j) = bestLabel;
			}
		}

		//update
		std::vector<Vec3d> channelSum(K, Vec3d(0, 0, 0));
		std::vector<int> pixelCount(K, 0);

		for (int i = 0; i < img.rows; i++) {
			for (int j = 0; j < img.cols; j++) {
				int k = labels.at<int>(i, j);
				Vec3b p = img.at<Vec3b>(i, j);

				channelSum[k][0] += p[0]; // B Channel
				channelSum[k][1] += p[1]; // G Channel
				channelSum[k][2] += p[2]; // R Channel
				pixelCount[k]++;
			}
		}

		std::vector<Vec3b> newCentroids(K);
		for (int k = 0; k < K; k++) {
			if (pixelCount[k] > 0) {
				newCentroids[k][0] = (uchar)(channelSum[k][0] / pixelCount[k]);
				newCentroids[k][1] = (uchar)(channelSum[k][1] / pixelCount[k]);
				newCentroids[k][2] = (uchar)(channelSum[k][2] / pixelCount[k]);
			}
			else {
				newCentroids[k] = centroids[k];
			}
		}

		// convergence check
		bool converged = true;
		for (int k = 0; k < K; k++) {
			if (getEuclideanDistance(centroids[k], newCentroids[k]) > 1.0) {
				converged = false;
				break;
			}
		}

		centroids = newCentroids;

		if (converged) {
			printf("[K-Means Random] Convergence reached at iteration %d.\n", currentIteration);
			break;
		}
	}

	// reconstruct the image
	Mat result(img.size(), img.type());
	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			result.at<Vec3b>(i, j) = centroids[labels.at<int>(i, j)];
		}
	}

	return result;
}

// 4. K-MEANS INITIALIZATION COMPARISON TEST FUNCTION (Option 200)

void testKMeansInitializationAnalysis()
{
	char fname[MAX_PATH];
	while (openFileDlg(fname))
	{
		cv::Mat src = cv::imread(fname, cv::IMREAD_COLOR);
		if (src.empty()) {
			printf("Error opening image!\n");
			continue;
		}

		cv::Mat srcResized;
		cv::resize(src, srcResized, cv::Size(300, 300));

		int initialColors = countUniqueColors(srcResized);
		int K = 64;

		printf("\n--- INITIAL IMAGE ANALYSIS ---\n");
		printf("Number of unique colors in input image: %d\n", initialColors);
		printf("Chosen K value (requested number of centroids): %d\n", K);
		printf("Running both convergence-based K-Means configurations...\n");

		// Run 1: K-Means with unique initialization via Sets
		cv::Mat dstOptim = kMeansColorQuantization(srcResized, K);
		int finalColorsOptim = countUniqueColors(dstOptim);

		// Run 2: K-Means with standard Random initialization
		cv::Mat dstRandom = kMeansColorQuantizationRandom(srcResized, K);
		int finalColorsRandom = countUniqueColors(dstRandom);

		printf("\n--- FINAL COMPARATIVE STATISTICS ---\n");
		printf("Unique final colors - Unique Sets Variant (Optimal): %d\n", finalColorsOptim);
		printf("Unique final colors - Pure Random Variant: %d\n", finalColorsRandom);
		printf("Quantization Factor (Compression): %.2f\n", (double)initialColors / finalColorsOptim);
		printf("------------------------------------------------------------------------\n");

		cv::namedWindow("1. Original Image", cv::WINDOW_NORMAL);
		cv::namedWindow("2. K-Means Unique Sets (Optimal)", cv::WINDOW_NORMAL);
		cv::namedWindow("3. K-Means Pure Random", cv::WINDOW_NORMAL);

		cv::resizeWindow("1. Original Image", 500, 500);
		cv::resizeWindow("2. K-Means Unique Sets (Optimal)", 500, 500);
		cv::resizeWindow("3. K-Means Pure Random", 500, 500);

		cv::imshow("1. Original Image", srcResized);
		cv::imshow("2. K-Means Unique Sets (Optimal)", dstOptim);
		cv::imshow("3. K-Means Pure Random", dstRandom);

		cv::waitKey(0);
		cv::destroyAllWindows();
	}
}

// 5. MEDIAN CUT ALGORITHM IMPLEMENTATION

struct Box {
	std::vector<Vec3b> pixels;
	int minB, maxB, minG, maxG, minR, maxR;

	void calculateBounds() {
		minB = minG = minR = 255;
		maxB = maxG = maxR = 0;
		for (const auto& p : pixels) {
			if (p[0] < minB) minB = p[0]; if (p[0] > maxB) maxB = p[0];
			if (p[1] < minG) minG = p[1]; if (p[1] > maxG) maxG = p[1];
			if (p[2] < minR) minR = p[2]; if (p[2] > maxR) maxR = p[2];
		}
	}

	int getLongestAxis() const {
		int rangeB = maxB - minB;
		int rangeG = maxG - minG;
		int rangeR = maxR - minR;
		if (rangeB >= rangeG && rangeB >= rangeR) return 0; // B Axis
		if (rangeG >= rangeB && rangeG >= rangeR) return 1; // G Axis
		return 2; // R Axis
	}
};

Mat medianCutColorQuantization(const Mat& img, int K) {
	std::vector<Box> boxes;
	Box initialBox;

	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			initialBox.pixels.push_back(img.at<Vec3b>(i, j));
		}
	}
	initialBox.calculateBounds();
	boxes.push_back(initialBox);

	while (boxes.size() < K) {
		int maxBoxIndex = -1;
		size_t maxPixels = 0;

		for (size_t i = 0; i < boxes.size(); i++) {
			if (boxes[i].pixels.size() > maxPixels && boxes[i].pixels.size() > 1) {
				maxPixels = boxes[i].pixels.size();
				maxBoxIndex = i;
			}
		}

		if (maxBoxIndex == -1) break;

		Box boxToSplit = boxes[maxBoxIndex];
		boxes.erase(boxes.begin() + maxBoxIndex);

		int sortAxis = boxToSplit.getLongestAxis();

		std::sort(boxToSplit.pixels.begin(), boxToSplit.pixels.end(), [sortAxis](const Vec3b& a, const Vec3b& b) {
			return a[sortAxis] < b[sortAxis];
			});

		size_t median = boxToSplit.pixels.size() / 2;
		Box b1, b2;
		b1.pixels.assign(boxToSplit.pixels.begin(), boxToSplit.pixels.begin() + median);
		b2.pixels.assign(boxToSplit.pixels.begin() + median, boxToSplit.pixels.end());

		b1.calculateBounds();
		b2.calculateBounds();

		boxes.push_back(b1);
		boxes.push_back(b2);
	}

	std::vector<Vec3b> medianCutPalette(K);
	for (size_t k = 0; k < boxes.size(); k++) {
		long long sumB = 0, sumG = 0, sumR = 0;
		for (const auto& p : boxes[k].pixels) {
			sumB += p[0]; sumG += p[1]; sumR += p[2];
		}
		size_t count = boxes[k].pixels.size();
		if (count > 0) {
			medianCutPalette[k] = Vec3b((uchar)(sumB / count), (uchar)(sumG / count), (uchar)(sumR / count));
		}
	}

	Mat result(img.size(), img.type());
	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			Vec3b pixel = img.at<Vec3b>(i, j);
			double minDist = 100000.0;
			Vec3b chosenColor = medianCutPalette[0];

			for (size_t k = 0; k < boxes.size(); k++) {
				double d = getEuclideanDistance(pixel, medianCutPalette[k]);
				if (d < minDist) {
					minDist = d;
					chosenColor = medianCutPalette[k];
				}
			}
			result.at<Vec3b>(i, j) = chosenColor;
		}
	}
	return result;
}

// 6. PERFORMANCE COMPARISON TEST FUNCTION (Option 201)

void runPerformanceComparison()
{
	char fname[MAX_PATH];
	while (openFileDlg(fname))
	{
		cv::Mat src = cv::imread(fname, cv::IMREAD_COLOR);
		if (src.empty()) {
			printf("Error opening image!\n");
			continue;
		}

		cv::Mat srcResized;
		cv::resize(src, srcResized, cv::Size(300, 300));

		int K = 32;
		printf("\n======= PERFORMANCE STUDY USING GETTICKCOUNT (K = %d) =======\n", K);

		// 1. Measure and Run MEDIAN CUT
		int64 startMedianCut = cv::getTickCount();

		cv::Mat imgMedianCut = medianCutColorQuantization(srcResized, K);

		int64 stopMedianCut = cv::getTickCount();
		double timeMedianCut = ((stopMedianCut - startMedianCut) / cv::getTickFrequency()) * 1000.0;

		// 2. Measure and Run K-MEANS
		int64 startKMeans = cv::getTickCount();

		cv::Mat imgKMeans = kMeansColorQuantization(srcResized, K);

		int64 stopKMeans = cv::getTickCount();
		double timeKMeans = ((stopKMeans - startKMeans) / cv::getTickFrequency()) * 1000.0;

		printf("\n--- TEMPORAL PERFORMANCE RESULTS ---\n");
		printf("Median Cut execution time: %.2f ms\n", timeMedianCut);
		printf("K-Means (Fully convergent) execution time: %.2f ms\n", timeKMeans);
		printf("-----------------------------------------\n");

		cv::namedWindow("1. Original Image", cv::WINDOW_NORMAL);
		cv::namedWindow("2. Median Cut Result", cv::WINDOW_NORMAL);
		cv::namedWindow("3. K-Means Result", cv::WINDOW_NORMAL);

		cv::resizeWindow("1. Original Image", 450, 450);
		cv::resizeWindow("2. Median Cut Result", 450, 450);
		cv::resizeWindow("3. K-Means Result", 450, 450);

		cv::imshow("1. Original Image", srcResized);
		cv::imshow("2. Median Cut Result", imgMedianCut);
		cv::imshow("3. K-Means Result", imgKMeans);

		cv::waitKey(0);
		cv::destroyAllWindows();
	}
}

int main()
{
	cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_FATAL);

	int op;
	do
	{
		system("cls");
		destroyAllWindows();
		printf("Menu:\n");
		printf("200 - K-Means (Random vs Unique Set Initializations)\n");
		printf("201 - Median Cut vs K-Means Performance Study\n");
		printf(" 0  - Exit\n\n");
		printf("Option: ");

		scanf("%d", &op);
		switch (op)
		{
		case 200:
			testKMeansInitializationAnalysis();
			waitKey();
			break;
		case 201:
			runPerformanceComparison();
			waitKey();
			break;
		}
	} while (op != 0);
	return 0;
}