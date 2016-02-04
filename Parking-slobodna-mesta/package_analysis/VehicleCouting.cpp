#include "VehicleCouting.h"

namespace FAV1
{
	IplImage* img_input1 = 0;
	IplImage* img_input2 = 0;
	int roi_x0 = 0;
	int roi_y0 = 0;
	int roi_x1 = 0;
	int roi_y1 = 0;
	int startDraw = 0;
	bool roi_defined = false;
	bool use_roi = true;

}

VehicleCouting::VehicleCouting() : firstTime(true), showOutput(true), key(0), countAB(22), countBA(0), showSpots(0), countCD(0), countDC(0)
{
	std::cout << "VehicleCouting()" << std::endl;
}

VehicleCouting::~VehicleCouting()
{
	std::cout << "~VehicleCouting()" << std::endl;
}

void VehicleCouting::setInput(const cv::Mat &i)
{
	img_input = i;
}

void VehicleCouting::setTracks(const cvb::CvTracks &t)
{
	tracks = t;
}

VehiclePosition VehicleCouting::getVehiclePosition(const CvPoint2D64f centroid)
{
	VehiclePosition vehiclePosition = VP_NONE;


	if (centroid.x < FAV1::roi_x0)
	{
		cv::putText(img_input, "STATE: A", cv::Point(10, 200), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 255, 255));
		vehiclePosition = VP_A;
	}

	if (centroid.x > FAV1::roi_x0 && centroid.x < 210)
	{
		cv::putText(img_input, "STATE: B", cv::Point(10, 200), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 255, 255));//*********ove izmenjeno (10, img_h/2)
		vehiclePosition = VP_B;
	}

	if (centroid.x > 215 && centroid.x < FAV1::roi_x0 + 305)
	{
		cv::putText(img_input, "STATE: C", cv::Point(10, 200), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 255, 255));
		vehiclePosition = VP_C;
	}

	if (centroid.x > FAV1::roi_x0 + 305)
	{
		cv::putText(img_input, "STATE: D", cv::Point(10, 200), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 255, 255));
		vehiclePosition = VP_D;

	}



	return vehiclePosition;
}

void VehicleCouting::process()
{
	if (img_input.empty())
		return;

	img_w = img_input.size().width;
	img_h = img_input.size().height;

	loadConfig();

	//--------------------------------------------------------------------------

	if (FAV1::use_roi == true && FAV1::roi_defined == true)
		cv::line(img_input, cv::Point(FAV1::roi_x0, FAV1::roi_y0), cv::Point(FAV1::roi_x1, FAV1::roi_y1), cv::Scalar(0, 255, 255)); //LINIJA
	cv::line(img_input, cv::Point(FAV1::roi_x0 + 305, FAV1::roi_y0), cv::Point(FAV1::roi_x1 + 305, FAV1::roi_y1), cv::Scalar(0, 255, 255));//DODATO!!!!!!!!!!

	bool ROI_OK = false;

	if (FAV1::use_roi == true && FAV1::roi_defined == true)
		ROI_OK = true;

	if (ROI_OK)
	{
		if (!firstTime){

			cv::putText(img_input, "(A)", cv::Point(FAV1::roi_x0 - 32, FAV1::roi_y0), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 255, 255));
			cv::putText(img_input, "(B)", cv::Point(FAV1::roi_x0 + 12, FAV1::roi_y0), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 255, 255));
			cv::putText(img_input, "(C)", cv::Point(FAV1::roi_x0 + 305 - 32, FAV1::roi_y0), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 255, 255));
			cv::putText(img_input, "(D)", cv::Point(FAV1::roi_x0 + 305 + 12, FAV1::roi_y0), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 255, 255));
		}

	}

	for (std::map<cvb::CvID, cvb::CvTrack*>::iterator it = tracks.begin(); it != tracks.end(); it++)
	{
		cvb::CvID id = (*it).first;
		cvb::CvTrack* track = (*it).second;

		CvPoint2D64f centroid = track->centroid;

		if (track->inactive == 0)
		{
			if (positions.count(id) > 0)
			{
				std::map<cvb::CvID, VehiclePosition>::iterator it2 = positions.find(id);
				VehiclePosition old_position = it2->second;

				VehiclePosition current_position = getVehiclePosition(centroid);

				if (current_position != old_position)
				{
					if (old_position == VP_A && current_position == VP_B)
						countAB++;

					if (old_position == VP_B && current_position == VP_A)
						countBA++;

					if (old_position == VP_C && current_position == VP_D)
						countCD++;

					if (old_position == VP_D && current_position == VP_C)
						countDC++;

					positions.erase(positions.find(id));
				}
			}
			else
			{
				VehiclePosition vehiclePosition = getVehiclePosition(centroid);

				if (vehiclePosition != VP_NONE)
					positions.insert(std::pair<cvb::CvID, VehiclePosition>(id, vehiclePosition));
			}
		}
		else
		{
			if (positions.count(id) > 0)
				positions.erase(positions.find(id));
		}


		if (points.count(id) > 0)
		{
			std::map<cvb::CvID, std::vector<CvPoint2D64f>>::iterator it2 = points.find(id);
			std::vector<CvPoint2D64f> centroids = it2->second;

			std::vector<CvPoint2D64f> centroids2;
			if (track->inactive == 0 && centroids.size() < 30)
			{
				centroids2.push_back(centroid);

				for (std::vector<CvPoint2D64f>::iterator it3 = centroids.begin(); it3 != centroids.end(); it3++)
				{
					centroids2.push_back(*it3);
				}

				for (std::vector<CvPoint2D64f>::iterator it3 = centroids2.begin(); it3 != centroids2.end(); it3++)
				{
					cv::circle(img_input, cv::Point((*it3).x, (*it3).y), 3, cv::Scalar(255, 255, 255), -1);
				}

				points.erase(it2);
				points.insert(std::pair<cvb::CvID, std::vector<CvPoint2D64f>>(id, centroids2));
			}
			else
			{
				points.erase(it2);
			}
		}
		else
		{
			if (track->inactive == 0)
			{
				std::vector<CvPoint2D64f> centroids;
				centroids.push_back(centroid);
				points.insert(std::pair<cvb::CvID, std::vector<CvPoint2D64f>>(id, centroids));
			}
		}


	}

	//-------------------------------------------------------------------------

	std::string countSpots = "Broj zauzetih mesta: " + std::to_string((countAB - countBA) + (countDC - countCD)) + " / 27";
	cv::putText(img_input, countSpots, cv::Point(10, img_h - 7), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 255, 255));

	if (showOutput)
		cv::imshow("PrebrojavanjeVozila", img_input);

	if (firstTime)
		saveConfig();

	firstTime = false;
}

void VehicleCouting::saveConfig()
{
	CvFileStorage* fs = cvOpenFileStorage("config/VehicleCouting.xml", 0, CV_STORAGE_WRITE);

	cvWriteInt(fs, "showOutput", showOutput);
	cvWriteInt(fs, "showSpots", showSpots);

	cvWriteInt(fs, "fav1_use_roi", FAV1::use_roi);
	cvWriteInt(fs, "fav1_roi_defined", FAV1::roi_defined);
	cvWriteInt(fs, "fav1_roi_x0", FAV1::roi_x0);
	cvWriteInt(fs, "fav1_roi_y0", FAV1::roi_y0);
	cvWriteInt(fs, "fav1_roi_x1", FAV1::roi_x1);
	cvWriteInt(fs, "fav1_roi_y1", FAV1::roi_y1);

	cvReleaseFileStorage(&fs);
}

void VehicleCouting::loadConfig()
{
	CvFileStorage* fs = cvOpenFileStorage("config/VehicleCouting.xml", 0, CV_STORAGE_READ);

	showOutput = cvReadIntByName(fs, 0, "showOutput", true);
	showSpots = cvReadIntByName(fs, 0, "showSpots", 1);

	FAV1::use_roi = cvReadIntByName(fs, 0, "fav1_use_roi", true);
	FAV1::roi_defined = cvReadIntByName(fs, 0, "fav1_roi_defined", false);
	FAV1::roi_x0 = cvReadIntByName(fs, 0, "fav1_roi_x0", 0);
	FAV1::roi_y0 = cvReadIntByName(fs, 0, "fav1_roi_y0", 0);
	FAV1::roi_x1 = cvReadIntByName(fs, 0, "fav1_roi_x1", 0);
	FAV1::roi_y1 = cvReadIntByName(fs, 0, "fav1_roi_y1", 0);

	cvReleaseFileStorage(&fs);
}
