#ifdef SWIG
%module cvblob
%{
#include "cvblob.h"
%}
#endif

#ifndef CVBLOB_H
#define CVBLOB_H

#include <iostream>
#include <map>
#include <list>
#include <vector>
#include <limits>

#if (defined(_WIN32) || defined(__WIN32__) || defined(__TOS_WIN__) || defined(__WINDOWS__) || (defined(__APPLE__) & defined(__MACH__)))
#include <cv.h>
#else
#include <opencv/cv.h>
#endif

#ifndef __CV_BEGIN__
#define __CV_BEGIN__ __BEGIN__
#endif
#ifndef __CV_END__
#define __CV_END__ __END__
#endif

#ifdef __cplusplus
extern "C" {
#endif

  namespace cvb
  {

#define CV_CHAINCODE_UP		0 
#define CV_CHAINCODE_UP_RIGHT	1
#define CV_CHAINCODE_RIGHT	2 
#define CV_CHAINCODE_DOWN_RIGHT	3 
#define CV_CHAINCODE_DOWN	4 
#define CV_CHAINCODE_DOWN_LEFT	5 
#define CV_CHAINCODE_LEFT	6 
#define CV_CHAINCODE_UP_LEFT	7 

  const char cvChainCodeMoves[8][2] = { { 0, -1},
                                        { 1, -1},
					{ 1,  0},
					{ 1,  1},
					{ 0,  1},
					{-1,  1},
					{-1,  0},
					{-1, -1}
                                      };
  typedef unsigned char CvChainCode;

  typedef std::list<CvChainCode> CvChainCodes;

  struct CvContourChainCode
  {
    CvPoint startingPoint; 
    CvChainCodes chainCode; 
  };

  typedef std::list<CvContourChainCode *> CvContoursChainCode; 

  typedef std::vector<CvPoint> CvContourPolygon;

  void cvRenderContourChainCode(CvContourChainCode const *contour, IplImage const *img, CvScalar const &color=CV_RGB(255, 255, 255));

  CvContourPolygon *cvConvertChainCodesToPolygon(CvContourChainCode const *cc);

  void cvRenderContourPolygon(CvContourPolygon const *contour, IplImage *img, CvScalar const &color=CV_RGB(255, 255, 255));

  double cvContourPolygonArea(CvContourPolygon const *p);

  double cvContourChainCodePerimeter(CvContourChainCode const *c);

  double cvContourPolygonPerimeter(CvContourPolygon const *p);

  double cvContourPolygonCircularity(const CvContourPolygon *p);

  CvContourPolygon *cvSimplifyPolygon(CvContourPolygon const *p, double const delta=1.);

  CvContourPolygon *cvPolygonContourConvexHull(CvContourPolygon const *p);

  void cvWriteContourPolygonCSV(const CvContourPolygon& p, const std::string& filename);

  void cvWriteContourPolygonSVG(const CvContourPolygon& p, const std::string& filename, const CvScalar& stroke=cvScalar(0,0,0), const CvScalar& fill=cvScalar(255,255,255));


  typedef unsigned int CvLabel;

#define IPL_DEPTH_LABEL (sizeof(cvb::CvLabel)*8)

#define CV_BLOB_MAX_LABEL std::numeric_limits<CvLabel>::max()

  typedef unsigned int CvID;

  struct CvBlob
  {
    CvLabel label; ///< Label assigned to the blob.
    
    union
    {
      unsigned int area; ///< Area (moment 00).
      unsigned int m00; ///< Moment 00 (area).
    };
    
    unsigned int minx; ///< X min.
    unsigned int maxx; ///< X max.
    unsigned int miny; ///< Y min.
    unsigned int maxy; ///< y max.
    
    CvPoint2D64f centroid; ///< Centroid.
    
    double m10; ///< Moment 10.
    double m01; ///< Moment 01.
    double m11; ///< Moment 11.
    double m20; ///< Moment 20.
    double m02; ///< Moment 02.
    
    double u11; ///< Central moment 11.
    double u20; ///< Central moment 20.
    double u02; ///< Central moment 02.

    double n11;
    double n20; 
    double n02; 

    double p1; 
    double p2; 

    CvContourChainCode contour;           ///< Contour.
    CvContoursChainCode internalContours; ///< Internal contours.
  };
  
 
  typedef std::map<CvLabel,CvBlob *> CvBlobs;

  typedef std::pair<CvLabel,CvBlob *> CvLabelBlob;

  unsigned int cvLabel (IplImage const *img, IplImage *imgOut, CvBlobs &blobs);

  void cvFilterLabels(IplImage *imgIn, IplImage *imgOut, const CvBlobs &blobs);

  CvLabel cvGetLabel(IplImage const *img, unsigned int x, unsigned int y);

  inline void cvReleaseBlob(CvBlob *blob)
  {
    if (blob)
    {
      for (CvContoursChainCode::iterator jt=blob->internalContours.begin(); jt!=blob->internalContours.end(); ++jt)
      {
	CvContourChainCode *contour = *jt;
	if (contour)
	  delete contour;
      }
      blob->internalContours.clear();

      delete blob;
    }
  }

  inline void cvReleaseBlobs(CvBlobs &blobs)
  {
    for (CvBlobs::iterator it=blobs.begin(); it!=blobs.end(); ++it)
    {
      cvReleaseBlob((*it).second);
    }
    blobs.clear();
  }

  CvLabel cvLargestBlob(const CvBlobs &blobs);

  inline CvLabel cvGreaterBlob(const CvBlobs &blobs)
  {
    return cvLargestBlob(blobs);
  }

  void cvFilterByArea(CvBlobs &blobs, unsigned int minArea, unsigned int maxArea);

  void cvFilterByLabel(CvBlobs &blobs, CvLabel label);

  inline CvPoint2D64f cvCentroid(CvBlob *blob)
  {
    return blob->centroid=cvPoint2D64f(blob->m10/blob->area, blob->m01/blob->area);
  }

  double cvAngle(CvBlob *blob);

  void cvSaveImageBlob(const char *filename, IplImage *img, CvBlob const *blob);
  
#define CV_BLOB_RENDER_COLOR            0x0001 
#define CV_BLOB_RENDER_CENTROID         0x0002 
#define CV_BLOB_RENDER_BOUNDING_BOX     0x0004 
#define CV_BLOB_RENDER_ANGLE            0x0008 
#define CV_BLOB_RENDER_TO_LOG           0x0010 
#define CV_BLOB_RENDER_TO_STD           0x0020 

  void cvRenderBlob(const IplImage *imgLabel, CvBlob *blob, IplImage *imgSource, IplImage *imgDest, unsigned short mode=0x000f, CvScalar const &color=CV_RGB(255, 255, 255), double alpha=1.);

  void cvRenderBlobs(const IplImage *imgLabel, CvBlobs &blobs, IplImage *imgSource, IplImage *imgDest, unsigned short mode=0x000f, double alpha=1.);

  inline void cvSetImageROItoBlob(IplImage *img, CvBlob const *blob)
  {
    cvSetImageROI(img, cvRect(blob->minx, blob->miny, blob->maxx-blob->minx, blob->maxy-blob->miny));
  };

  CvScalar cvBlobMeanColor(CvBlob const *blob, IplImage const *imgLabel, IplImage const *img);

  double cvDotProductPoints(CvPoint const &a, CvPoint const &b, CvPoint const &c);
 
  double cvCrossProductPoints(CvPoint const &a, CvPoint const &b, CvPoint const &c);

  double cvDistancePointPoint(CvPoint const &a, CvPoint const &b);

  double cvDistanceLinePoint(CvPoint const &a, CvPoint const &b, CvPoint const &c, bool isSegment=true);
  
  struct CvTrack
  {
    CvID id; 

    CvLabel label; 

    unsigned int minx; ///< X min.
    unsigned int maxx; ///< X max.
    unsigned int miny; ///< Y min.
    unsigned int maxy; ///< y max.
    
    CvPoint2D64f centroid; ///< Centroid.

    unsigned int lifetime;
    unsigned int active; 
    unsigned int inactive; 
  };

  typedef std::map<CvID, CvTrack *> CvTracks;

  typedef std::pair<CvID, CvTrack *> CvIDTrack;

  inline void cvReleaseTracks(CvTracks &tracks)
  {
    for (CvTracks::iterator it=tracks.begin(); it!=tracks.end(); it++)
    {
      CvTrack *track = (*it).second;
      if (track) delete track;
    }

    tracks.clear();
  }

  void cvUpdateTracks(CvBlobs const &b, CvTracks &t, const double thDistance, const unsigned int thInactive, const unsigned int thActive=0);

#define CV_TRACK_RENDER_ID            0x0001 
#define CV_TRACK_RENDER_BOUNDING_BOX  0x0002 
#define CV_TRACK_RENDER_TO_LOG        0x0010 
#define CV_TRACK_RENDER_TO_STD        0x0020 

  void cvRenderTracks(CvTracks const tracks, IplImage *imgSource, IplImage *imgDest, unsigned short mode=0x000f, CvFont *font=NULL);
  }
#ifdef __cplusplus
}
#endif

std::ostream& operator<< (std::ostream& output, const cvb::CvBlob& b);

std::ostream& operator<< (std::ostream& output, const cvb::CvContourPolygon& p);

std::ostream& operator<< (std::ostream& output, const cvb::CvTrack& t);
#endif
