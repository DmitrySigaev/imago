#include <cmath>
#include "boost/foreach.hpp"

#include "orientation_finder.h"
#include "vec2d.h"
#include "fourier_descriptors.h"
#include "contour_extractor.h"
#include "exception.h"
#include "segmentator.h"
#include "molecule.h"
#include "current_session.h"
#include "wedge_bond_extractor.h"
#include "separator.h"

namespace imago
{
   OrientationFinder::OrientationFinder( const CharacterRecognizer &cr ) : _cr(cr)
   {
   }

   OrientationFinder::~OrientationFinder()
   {
   }

   int OrientationFinder::findFromImage( const Image &img )
   {
      Image _img;
      _img.copy(img);
      int symbols[4] = {0};
      double scores[4] = {0.0};
      double err;
      static const std::string candidates = CharacterRecognizer::upper +
                                            CharacterRecognizer::lower +
                                            "123456";


      Molecule mol;
      SegmentDeque segments, layer_symbols, layer_graphics;

      RecognitionSettings &rs = getSettings();
      
      for (int i = 0; i < 4; i++)
      {
         if (i > 0)
            _img.rotate90();

         mol.clear();
         segments.clear();
         layer_symbols.clear();
         layer_graphics.clear();

         Segmentator::segmentate(_img, segments);
         WedgeBondExtractor wbe(segments, _img);
         wbe.singleDownFetch(mol);

         Separator sep(segments, _img);

         //Settings for handwriting separation
         rs.set("SymHeightErr", 42);
         rs.set("MaxSymRatio", 1.4);
         sep.firstSeparation(layer_symbols, layer_graphics);
         symbols[i] = layer_symbols.size();
         BOOST_FOREACH(Segment *seg, layer_symbols)
         {
            _cr.recognize(*seg, candidates, &err);
            scores[i] += err;
            delete seg;
         }

         BOOST_FOREACH(Segment *seg, layer_graphics)
            delete seg;

#ifndef NDEBUG
         printf("%d %lf %lf\n", symbols[i], scores[i], scores[i] / symbols[i]);
#endif
      }


      int r = 0;
      err = 1e16;
      for (int i = 0; i < 4; i++)
         if (symbols[i] > 0 && scores[i] / symbols[i] < err)
            err = scores[i] / symbols[i], r = i;

      return r;
   }

   int OrientationFinder::findFromSymbols( const SegmentDeque &symbols )
   {
      int rotations[4] = {0};
      double dists[4] = {0.0};
      int r, nr; char c, cup; double d;
      static const std::string bioriented = "BCDEHIKNPSMWZbcdiklnpqsuz";
      std::vector<boost::tuple<int, char, double> > skipped;
      BOOST_FOREACH(Segment *s, symbols)
      {
         boost::tuple<int, char, double> tup = _processSymbol(*s);
         boost::tie(r, c, d) = tup;
         if (r == -1)
            continue;

         cup = toupper(c);
         if (cup == 'O' || cup == '0' || cup == 'X')
            continue;

         rotations[r]++;
         dists[r] += d;

         if (std::find(bioriented.begin(), bioriented.end(), c) !=
             bioriented.end())
         {
            nr = (r + 2) % 4;
            rotations[nr]++;
            dists[nr] += d;
         }
         else if (c == 'U')
         {
            nr = (r + 1) % 4;
            rotations[nr]++;
            dists[nr] += d;
            nr = (r + 4 - 1) % 4;
            rotations[nr]++;
            dists[nr] += d;
         }
      }

#ifndef NDEBUG
      for (int i = 0; i < 4; i++)
         printf("%d %lf %lf\n", rotations[i], dists[i],
                dists[i] / rotations[i]);
#endif

      r = 0; d = 1e16;
      for (int i = 0; i < 4; i++)
         if (rotations[i] > 0 && dists[i] / rotations[i] < d)
            d = dists[i] / rotations[i], r = i;

      return r;
   }

   boost::tuple<int, char, double>
   OrientationFinder::_processSymbol( const Segment &seg )
   {
      int x, y;
      int w = seg.getWidth(), h = seg.getHeight();
      double d = 0, dist_ul, dist_ur, dist_dl, dist_dr;
      dist_ul = dist_ur = dist_dl = dist_dr = 1e16;
      Vec2d ul, ur, dl, dr;
      char c, best_c;
      double best_d;
      int best_r;

      SymbolFeatures features;
      int count = _cr.getDescriptorsCount();
      ContourExtractor contour_ext;
      Points approxContour, contour;

      try
      {
         contour_ext.getRawContour(seg, contour);
      }
      catch (NoContourException &e)
      {
         return boost::make_tuple(-1, -1, -1);
      }

      const byte *segData = seg.getData();
      for (int i = 0; i < w * h; i++)
      {
         if (segData[i] == 255) //white
            continue;

         x = i % w; y = i / w;

         d = sqrt((double)x * x + y * y);
         if (dist_ul - d > EPS)
         {
            dist_ul = d;
            ul.set(x, y);
         }

         d = sqrt((double)(w - 1 - x) * (w - 1 - x) + y * y);
         if (dist_ur - d > EPS)
         {
            dist_ur = d;
            ur.set(x + 1, y);
         }

         d = sqrt((double)x * x + (h - 1 - y) * (h - 1 - y));
         if (dist_dl - d > EPS)
         {
            dist_dl = d;
            dl.set(x, y + 1);
         }

         d = sqrt((double)(w - 1 - x) * (w - 1 - x) + (h - 1 - y) * (h - 1 - y));
         if (dist_dr - d > EPS)
         {
            dist_dr = d;
            dr.set(x + 1, y + 1);
         }

      }

      assert(std::find(contour.begin(), contour.end(), ul) != contour.end());
      assert(std::find(contour.begin(), contour.end(), ur) != contour.end());
      assert(std::find(contour.begin(), contour.end(), dl) != contour.end());
      assert(std::find(contour.begin(), contour.end(), dr) != contour.end());

      features.init = features.recognizable = 1;
      //TODO:It's copied from Segment::initFeatures
      //Segment's inner parts
      features.inner_contours_count = 0;
      SegmentDeque segments;
      Segmentator::segmentate(seg, segments, 3, 255); //all white parts
      BOOST_FOREACH(Segment *in_seg, segments)
      {
         int in_x = in_seg->getX(), in_y = in_seg->getY(), in_w =
             in_seg->getWidth(), in_h = in_seg->getHeight();
         delete in_seg;
         
         if (in_x == 0 || in_y == 0 || in_x + in_w == w || in_y + in_h == h)
            continue;

         features.inner_contours_count++;
      }
      features.inner_descriptors.resize(features.inner_contours_count);

      std::string candidates = CharacterRecognizer::upper +
                               CharacterRecognizer::lower +
                               "123456";
      candidates.erase(candidates.find('U'));
      candidates.erase(candidates.find('G'));

      //Not rotated
      approxContour = contour;
      contour_ext._approximize(approxContour);
      FourierDescriptors::calculate(approxContour, count, features.descriptors);
      c = _cr.recognize(features, candidates, &d);
      best_r = 0, best_c = c, best_d = d;

      //Rotated 90 cw
      _rotateContourTo(ur, contour);
      approxContour = contour;
      contour_ext._approximize(approxContour);
      FourierDescriptors::calculate(approxContour, count, features.descriptors);
      c = _cr.recognize(features, candidates, &d);
      if (d < best_d)
         best_r = 1, best_c = c, best_d = d;

      //Rotated 180
      _rotateContourTo(dr, contour);
      approxContour = contour;
      contour_ext._approximize(approxContour);
      FourierDescriptors::calculate(approxContour, count, features.descriptors);
      c = _cr.recognize(features, candidates, &d);
      if (d < best_d)
         best_r = 2, best_c = c, best_d = d;

      //Rotated 90 ccw (270)
      _rotateContourTo(dl, contour);
      approxContour = contour;
      contour_ext._approximize(approxContour);
      FourierDescriptors::calculate(approxContour, count, features.descriptors);
      c = _cr.recognize(features, candidates, &d);
      if (d < best_d)
         best_r = 3, best_c = c, best_d = d;

      return boost::make_tuple(best_r, best_c, best_d);
   }

   void OrientationFinder::_rotateContourTo( const Vec2d &p, Points &contour )
   {
      Points ncontour;
      Points::iterator it = find(contour.begin(), contour.end(), p);

      ncontour.insert(ncontour.begin(), it, contour.end());
      ncontour.insert(ncontour.end(), contour.begin() + 1, it + 1);

      contour.assign(ncontour.begin(), ncontour.end());
   }
}