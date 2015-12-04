#include "hvlIO.h"

#include <itkImage.h>
#include <itkImageFileReader.h>

void hvl::readImage() {
	typedef itk::Image<float, 3> hImg;
	typedef itk::ImageFileReader<hImg> hReader;

	hReader::Pointer reader = hReader::New();
	reader->SetFileName("E:\\Programming\\Patient01Homo.mha");
	reader->Update();
	hImg* img = reader->GetOutput();

	float p = img->GetPixel({0, 0, 0});
	printf("pixel 0: %f", p);
}