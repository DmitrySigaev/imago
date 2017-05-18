#pragma once
#include <vector>
#include "../Image/Image.h"
#include "Bounds.h"
#include "ImageMap.h"
#include "Contour.h"
#include "RangeArray.h"
#include "LinearApproximation.h"

namespace gga
{
	class Vectorize
	{
		const Image& SourceImage;
		ImageMap   CurrentImageSplit;
		std::vector<Contour*> OuterContours;
	public:
		Vectorize(const Image& image);
		~Vectorize();
		
		size_t getContoursCount() const;
		const Contour& getContour(size_t index) const;
		const ImageMap& getImageSplit() const;
	};
}


