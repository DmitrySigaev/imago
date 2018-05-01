#include "weak_segmentator.h"
#include <queue>
#include "color_channels.h"
#include "log_ext.h"
#include "pixel_boundings.h"
#include "thin_filter2.h"

namespace imago
{	
	static const double RECTANGULAR_AREA_THRESHOLD = 0.30; // %, minimal rectangle coverage of image
	static const double RECTANGULAR_PASS_THRESHOLD = 0.95; // %, for rectangle testing

	Points2i WeakSegmentator::getNeighbors(const Image& img, const Vec2i& p, int range)
	{
		Points2i neighb;
		for (int dy = -range; dy <= range; dy++)
			for (int dx = -range; dx <= range; dx++)
				if ((dx != 0 || dy != 0) && p.x+dx >= 0 && p.y+dy >= 0
					&& p.x+dx < img.getWidth() && p.y+dy < img.getHeight() )
					if (img.getByte(p.x+dx, p.y+dy) == INK)
						neighb.push_back(Vec2i(p.x+dx, p.y+dy));
		return neighb;
	}

	void WeakSegmentator::updateRefineMap(int max_len)
	{
		logEnterFunction();

		currentRefineGeneration++;

		Image thin(width(), height());
		for (int y = 0; y < height(); y++)
			for (int x = 0; x < width(); x++)
				thin.getByte(x, y) = readyForOutput(x,y) ? INK : BLANK;

		ThinFilter2(thin).apply();

		for (int y = 0; y < height(); y++)
		{
			for (int x = 0; x < width(); x++)
			{
				if (thin.getByte(x,y) != INK)
					continue;
				Vec2i current = Vec2i(x,y);
				if (getNeighbors(thin, current).size() == 1) // endpoint
				{					
					Points2i p = getNeighbors(thin, current, 1 + max_len / 2); 
					double average_dx = 0, average_dy = 0;
					for (size_t u = 0; u < p.size(); u++)
					{
						average_dx += (double)(current.x - p[u].x) / p.size();
						average_dy += (double)(current.y - p[u].y) / p.size();
					}

					if (fabs(average_dx) < 1.5 && fabs(average_dy) < 1.5)
					{
						// draw square
						for (int j = -max_len/2; j <= max_len/2; j++)
						{
							for (int k = -max_len/2; k <= max_len/2; k++)
							{
								int _x = x + j;
								int _y = y + k;
								if (inRange(_x, _y))
								{
									at(_x, _y).refineGeneration = currentRefineGeneration;
								}
							}
						}
					}
					else
					{
						// draw trinagle
						double norm = sqrt(average_dx*average_dx + average_dy*average_dy);
						double dx = average_dx/norm, dy = average_dy/norm;
						double subpx = 0.5;

						for (int j = (int)(-max_len / subpx); j < (int)(max_len / subpx); j++)
						{
							int bnd = (int)(abs(j)*0.9 + 2);
							for (int k = -bnd; k <= bnd; k++)
							{
								int _x = (int)((double)x + dx * (double)(j*subpx) - dy * (double)(k*subpx));
								int _y = (int)((double)y + dy * (double)(j*subpx) + dx * (double)(k*subpx));
								if (inRange(_x, _y))
								{
									at(_x, _y).refineGeneration = currentRefineGeneration;
								}
							}
						}
					}
				}
			}
		}

		if (getLogExt().loggingEnabled())
		{
			for (int y = 0; y < height(); y++)
				for (int x = 0; x < width(); x++)
					if (at(x, y).refineGeneration == currentRefineGeneration)
						thin.getByte(x, y) = 128; // only visual

			getLogExt().appendImage("Image with endvectors", thin);
		}
	}

	Points2i WeakSegmentator::getEndpointsDecornered() const
	{
		logEnterFunction();

		Points2i result;

		const int DECORNERED = 200;
		const int TRACED = 230;
		const int ENDPOINT = 1;

		Image thin(width(), height());
		for (int y = 0; y < height(); y++)
			for (int x = 0; x < width(); x++)
				thin.getByte(x, y) = readyForOutput(x,y) ? INK : BLANK;

		ThinFilter2(thin).apply();

		// decorner
		for (int y = 0; y < height(); y++)
		{
			for (int x = 0; x < width(); x++)
			{
				if (thin.getByte(x,y) != INK)
					continue;
				if (getNeighbors(thin, Vec2i(x,y)).size() > 2)
					thin.getByte(x,y) = DECORNERED;
			}
		}
		
		getLogExt().appendImage("Fast decorner", thin);

		// now all objects are just curve lines with exactly 2 endpoints
		for (int y = 0; y < height(); y++)
		{
			for (int x = 0; x < width(); x++)
			{
				if (thin.getByte(x,y) != INK)
					continue;
				// search enpoints, go to 2 different directions
				// <------- . ------->				
				thin.getByte(x,y) = TRACED;
				Points2i startPoints = getNeighbors(thin, Vec2i(x,y));				
				for (size_t u = 0; u < startPoints.size(); u++)
				{
					Vec2i current = startPoints[u];
					bool first = true;
					while (true)
					{
						thin.getByte(current.x, current.y) = TRACED;
						Points2i n = getNeighbors(thin, current);
						if (n.size() > 1 && !first)
						{
							// bad decorner, or decorner not applied
							break;
						}
						if (n.empty())
						{
							thin.getByte(current.x, current.y) = ENDPOINT;
							result.push_back(current);
							break;
						}
						current = n[0];
						first = false;
					}
				}
				if (startPoints.size() == 1)
				{
					// we started from the edge
					thin.getByte(x,y) = ENDPOINT;
					result.push_back(Vec2i(x,y));
				}
			}
		}

		getLogExt().appendImage("Traced image", thin);

		return result;
	}

	bool WeakSegmentator::alreadyExplored(int x, int y) const
	{
		return at(x, y).id > 0 || at(x, y).refineGeneration > 0;
	}

	bool WeakSegmentator::refineIsAllowed(int x, int y) const
	{
		return at(x, y).refineGeneration == currentRefineGeneration;
	}

	int WeakSegmentator::appendData(const ImageInterface &img, int lookup_range)
	{
		logEnterFunction();
			
		int added_pixels = 0;

		for (int y = 0; y < height(); y++)
			for (int x = 0; x < width(); x++)
				if (at(x,y).id == 0 && img.isFilled(x,y) && refineIsAllowed(x,y))
				{
					int id = SegmentPoints.size()+1;
					fill(img, id, x, y, lookup_range);			
					added_pixels += SegmentPoints[id].size();
				}

		getLogExt().append("Currently added pixels", added_pixels);
		getLogExt().append("Total segments count", SegmentPoints.size());

		return added_pixels;
	}

	void WeakSegmentator::performPixelOptimizations()
	{
		logEnterFunction();

		for (int y = 0; y < height(); y++)
			for (int x = 0; x < width(); x++)
			{
				int count = 0, id = 0;
				for (int dy = -1; dy <= 1; dy++)
				{
					for (int dx = -1; dx <= 1; dx++)
						if ((dx != 0 || dy != 0) && inRange(x+dx, y+dy))
						{
							if (at(x+dx,y+dy).id != 0)
							{
								id = at(x+dx,y+dy).id;
								count++;
							}
						}
				}
				if (at(x,y).id != 0 && count == 0) // erase lonely pixel
					at(x,y).id = 0;
				else if (at(x,y).id == 0 && count >= 7) // fill inside pixels group
					at(x,y).id = id;
			}
	}

	/*void WeakSegmentator::eraseNoise(double threshold)
	{
		// TODO:
		logEnterFunction();

		for (int y = 0; y < height(); y++)
			for (int x = 0; x < width(); x++)
			{
				int id = at(x, y).id;
				if (id > 0 && (int)SegmentPoints[id].size() < threshold)
				{
					SegmentPoints[id].clear();
					at(x,y).id = 0;
				}
			}			
	}*/

	bool WeakSegmentator::needCrop(Rectangle& crop, int winSize)
	{
		logEnterFunction();

		int area_pixels = (int)(width() * height() * RECTANGULAR_AREA_THRESHOLD);
		for (size_t id = 1; id <= SegmentPoints.size(); id++)
		{
			getLogExt().appendPoints("Segment", SegmentPoints[id]);
			Rectangle bounds;
			if (getRectangularArea(id) > area_pixels && hasRectangularStructure(id, bounds, winSize))
			{
				getLogExt().append("Has rectangular structure, id", id);	
				bounds.adjustBorder(winSize*2);
				crop = bounds;
				return true;
			}
		}
		return false;
	}

	int WeakSegmentator::getRectangularArea(int id)
	{
		RectShapedBounding b(SegmentPoints[id]);		
		return b.getBounding().width * b.getBounding().height;
	}		

	bool WeakSegmentator::hasRectangularStructure(int id, Rectangle& bound, int winSize)
	{
		Points2i& p = SegmentPoints[id];
			
		int map_x[MAX_IMAGE_DIMENSIONS] = {0};
		int map_y[MAX_IMAGE_DIMENSIONS] = {0};
		for (Points2i::iterator it = p.begin(); it != p.end(); it++)
		{
			map_x[it->x]++;
			map_y[it->y]++;
		}

		double x1c, x2c, y1c, y2c;			
		if (get2centers(map_x, width(), x1c, x2c) && get2centers(map_y, height(), y1c, y2c))
		{
			// now update maps
			memset(map_x, 0, sizeof(map_x));
			memset(map_y, 0, sizeof(map_y));
			for (Points2i::iterator it = p.begin(); it != p.end(); it++)
			{
				if (it->y > y1c && it->y < y2c) map_x[it->x]++;
				if (it->x > x1c && it->x < x2c) map_y[it->y]++;
			}
			// and centers
			if (get2centers(map_x, width(), x1c, x2c) && get2centers(map_y, height(), y1c, y2c) &&
				fabs(x1c - x2c) > 2*winSize && fabs(y1c - y2c) > 2*winSize)
			{
				int good = 0, bad = 0;
				for (Points2i::iterator it = p.begin(); it != p.end(); it++)
					if ((fabs(it->x - x1c) < winSize || fabs(it->x - x2c) < winSize) ||
						(fabs(it->y - y1c) < winSize || fabs(it->y - y2c) < winSize))
						good++;
					else
						bad++;
				if ((double)good / (good+bad) > RECTANGULAR_PASS_THRESHOLD)
				{
					bound = Rectangle((int)x1c, (int)y1c, (int)x2c, (int)y2c, 0);
					return true;
				}
			}
		}

		return false;
	}

	void WeakSegmentator::fill(const ImageInterface &img, int& id, int sx, int sy, int lookup_range)
	{
		std::queue<Vec2i> v;
		v.push(Vec2i(sx,sy));
		while (!v.empty())
		{
			Vec2i cur = v.front();
			v.pop(); // remove top

			if (at(cur.x,cur.y).id == 0)
			{
				at(cur.x,cur.y).id = id;
				SegmentPoints[id].push_back(cur);

				for (int dx = -lookup_range; dx <= lookup_range; dx++)
					for (int dy = -lookup_range; dy <= lookup_range; dy++)
					{
						Vec2i t(cur.x + dx,cur.y + dy);
						if ((dx != 0 || dy != 0) && inRange(t.x, t.y))
						{
							if (at(t.x, t.y).id == 0)
							{						
								if (img.isFilled(t.x, t.y) && refineIsAllowed(t.x, t.y))
									v.push(t);
							}
							else if (at(t.x, t.y).id != id)
							{
								// merge segments								
								int merge_id = at(t.x, t.y).id;
								// getLogExt().append("Merge id", id);
								// getLogExt().append("With", merge_id);
								for (size_t u = 0; u < SegmentPoints[id].size(); u++)
								{
									Vec2i p = SegmentPoints[id][u];
									at(p.x, p.y).id = merge_id;
									SegmentPoints[merge_id].push_back(p);
								}
								SegmentPoints.erase(SegmentPoints.find(id));
								id = merge_id;
							}
						}
					}
			}
		} // while
	}

	bool WeakSegmentator::get2centers(int* data, int size, double &c1, double& c2) // c1 < c2
	{
		double mean = 0.0, count = 0.0;
		for (int u = 0; u < size; u++)
		{
			mean += u * data[u];
			count += data[u];
		}			
			
		if (count < 1)
			return false;

		mean /= count;
		c1 = 0.0;
		c2 = 0.0;
		double count1 = 0.0;
		double count2 = 0.0;

		for (int u = 0; u < size; u++)
		{
			if (u < mean)
			{
				c1 += u * data[u];
				count1 += data[u];
			}
			else
			{
				c2 += u * data[u];
				count2 += data[u];
			}
		}

		if (count1 < 1 || count2 < 1)
			return false;

		c1 /= count1;
		c2 /= count2;
		return true;
	}
}