/**
 * This code is modified for the iCub humanoid robot through YARP connection.
 * The program reads the image from one camera (robot's eye) and tries to detect a face and exports the result on a new view.
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * by: Reza Ahmadzadeh (reza.ahmadzadeh@iit.it)
 **/
// reference for YARP
#include <yarp/sig/Image.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/Module.h>
#include <yarp/os/Network.h>
#include <yarp/sig/Vector.h>
#include <yarp/os/Time.h>
// opencv (this version of the code uses opencv 3.0)
/**
#include <cv.h>
include <highgui.h>
**/
#include <opencv2/videoio/videoio.hpp>  // Video write
#include <opencv2/videoio/videoio_c.h>  // Video write
//#include <opencv2/highgui/highgui.h>

// c++
/**
#include <iostream>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <cstdio>
**/
#include <fstream>
#include <sstream>
// CLM library
#include <CLM_core.h>
/**
#include <CLM.h>
#include <CLMTracker.h>
#include <CLMParameters.h>
#include <CLM_utils.h>
**/
// ------------------
// ----- Macros -----
// ------------------

#define INFO_STREAM( stream ) \
std::cout << stream << std::endl

#define WARN_STREAM( stream ) \
std::cout << "Warning: " << stream << std::endl

#define ERROR_STREAM( stream ) \
std::cout << "Error: " << stream << std::endl

static void printErrorAndAbort( const std::string & error )
{
    std::cout << error << std::endl;
    abort();
}

#define FATAL_STREAM( stream ) \
printErrorAndAbort( std::string( "Fatal error: " ) + stream )

// ----------------------
// ----- Namespaces -----
// ----------------------

using namespace std;
using namespace cv;
using namespace yarp::os;
using namespace yarp::sig;

// -----------------
// ----- Class -----
// -----------------

class MyModule:public RFModule
{
	
    BufferedPort<ImageOf<PixelRgb> > imageIn;  // make a port for reading images
    BufferedPort<ImageOf<PixelRgb> > imageSegOut; // make a port for passing the result to
	
	IplImage* cvImage;
    IplImage* display;
	
	
	vector<string> arguments;					// The arguments from the input 
	vector<string> files, depth_directories, pose_output_files, tracked_videos_output, landmark_output_files, landmark_3D_output_files;
	bool use_camera_plane_pose, done, cx_undefined, use_depth, write_settings, ok2;
	int device, f_n, frame_count;
	float fx,fy,cx,cy; 							// parameters of the camera
	string current_file;
	double fps;
	int64 t1,t0;								// time parameters for calculating and storing the framerate of the camera

	CLMTracker::CLMParameters* clm_parameters;
	CLMTracker::CLM* clm_model;

	double visualisation_boundary;

	std::ofstream pose_output_file;
	std::ofstream landmarks_output_file;	
	std::ofstream landmarks_3D_output_file;
	
public:



    MyModule(){}; 							// constructor 
    ~MyModule(){};							// deconstructor


    double getPeriod()						// the period of the loop.
    {
        return 0.0; 						// 0 is equal to the real-time
    }


    bool configure(yarp::os::ResourceFinder &rf)
    {
		arguments.push_back("-f");							// provide two default arguments in case we want to use no real camera
		arguments.push_back("../../videos/default.wmv"); 	// the video file 
		device = 0;   										// camera
		fx = 225.904; //500; (default)
		fy = 227.041; //500; (default)
		cx = 157.858; //0; (default)
		cy = 113.51; //0; (default)
		clm_parameters = new CLMTracker::CLMParameters(arguments);

		CLMTracker::get_video_input_output_params(files, depth_directories, pose_output_files, tracked_videos_output, landmark_output_files, landmark_3D_output_files, use_camera_plane_pose, arguments);
		CLMTracker::get_camera_params(device, fx, fy, cx, cy, arguments);    
        
        //get custom model folder location
        Property config;
        config.fromConfigFile(rf.findFile("from").c_str());
        
        //if(config != NULL)
        //{
            Bottle &fileLoc = config.findGroup("path_to_file");
            
            string modelPath = fileLoc.find("modelFile").asString().c_str();
            std::cout << "fromFile" << modelPath << std::endl;
            
            clm_parameters->model_location = modelPath;

            ok2 = imageIn.open(fileLoc.find("inputPort").asString().c_str());
	        ok2 = ok2 && imageSegOut.open(fileLoc.find("outputPort").asString().c_str());                  //
			if (!ok2) {
				fprintf(stderr, "Error. Failed to open image ports. \n");
				return false;
			}	
        //}
        std::cout << "default" << clm_parameters->model_location << std::endl;
		clm_model = new CLMTracker::CLM(clm_parameters->model_location);	
		
		done = false;
		write_settings = false;	
		
		// f_n = -1; 												// for reading from file
        f_n = 0; 													// for reading from device
        
        use_depth = !depth_directories.empty();
		frame_count = 0;
		t1,t0 = cv::getTickCount();									// get the current time as a baseline
		fps = 10;
		visualisation_boundary = 0.2;
        
		cx_undefined = false;
		if(cx == 0 || cy == 0)
		{
			cx_undefined = true;
		}		

		
		ImageOf<PixelRgb> *imgTmp = imageIn.read();  				// read an image
		if (imgTmp != NULL) 
		{ 
			display = (IplImage*)imgTmp->getIplImage();
            //Mat captured_image = display;                         // opencv < 3.0
            Mat captured_image = cvarrToMat(display);               // opencv 3.0
		}

	
		// Creating output files

		if(!pose_output_files.empty())
		{
			pose_output_file.open (pose_output_files[f_n], ios_base::out);
		}
	
		
		if(!landmark_output_files.empty())
		{
			landmarks_output_file.open(landmark_output_files[f_n], ios_base::out);
		}

		
		if(!landmark_3D_output_files.empty())
		{
			landmarks_3D_output_file.open(landmark_3D_output_files[f_n], ios_base::out);
		}
	
		INFO_STREAM( "Starting tracking");

        
        return true;
    }




	bool updateModule()
	{


		//cout << __FILE__ << ": " << __LINE__ << endl;


		ImageOf<PixelRgb> *imgTmp = imageIn.read();  // read an image
		if (imgTmp == NULL) 
		{
			FATAL_STREAM( "Failed to open video source" );
			return true;
		}
		
		bool faceAvailableFlag = false;
		display = (IplImage*)imgTmp->getIplImage();
        Mat captured_image = cvarrToMat(display);
        Mat segFace;

		// If optical centers are not defined just use center of image
		if(cx_undefined)
		{
			cx = captured_image.cols / 2.0f;
			cy = captured_image.rows / 2.0f;
			cx_undefined = true;
		}

		VideoWriter writerFace;
		if (!write_settings)
		{
			// saving the videos
			if(!tracked_videos_output.empty())
			{
				writerFace = VideoWriter(tracked_videos_output[f_n], CV_FOURCC('D','I','V','X'), 30, captured_image.size(), true);		
			}
			write_settings = true;
		}


		//cout << __FILE__ << ": " << __LINE__ << endl;
		// Reading the images
		Mat_<float> depth_image;
		Mat_<uchar> grayscale_image;

		if(captured_image.channels() == 3)
		{
			cvtColor(captured_image, grayscale_image, CV_BGR2GRAY);				
		}
		else
		{
			grayscale_image = captured_image.clone();				
		}
	
		// Get depth image
		if(use_depth)
		{
			char* dst = new char[100];
			std::stringstream sstream;

			sstream << depth_directories[f_n] << "\\depth%05d.png";
			sprintf(dst, sstream.str().c_str(), frame_count + 1);
			// Reading in 16-bit png image representing depth
			Mat_<short> depth_image_16_bit = imread(string(dst), -1);

			// Convert to a floating point depth image
			if(!depth_image_16_bit.empty())
			{
				depth_image_16_bit.convertTo(depth_image, CV_32F);
			}
			else
			{
				WARN_STREAM( "Can't find depth image" );
			}
		}
		
		// The actual facial landmark detection / tracking
		bool detection_success = CLMTracker::DetectLandmarksInVideo(grayscale_image, depth_image, *clm_model, *clm_parameters);

		// Work out the pose of the head from the tracked model
		Vec6d pose_estimate_CLM;
		if(use_camera_plane_pose)
		{
			pose_estimate_CLM = CLMTracker::GetCorrectedPoseCameraPlane(*clm_model, fx, fy, cx, cy, *clm_parameters);
		}
		else
		{
			pose_estimate_CLM = CLMTracker::GetCorrectedPoseCamera(*clm_model, fx, fy, cx, cy, *clm_parameters);
		}

		// Visualising the results
		// Drawing the facial landmarks on the face and the bounding box around it if tracking is successful and initialised
		double detection_certainty = clm_model->detection_certainty;
		Mat faceMask;
		Mat captureCopy;
		Mat croppedFace;
		captured_image.copyTo(captureCopy);
		
		// Only draw if the reliability is reasonable, the value is slightly ad-hoc
		if(detection_certainty < visualisation_boundary)
		{
			//CLMTracker::Draw(captured_image, *clm_model);

			//Daniel Extracting Landmarks
			int idx = clm_model->patch_experts.GetViewIdx(clm_model->params_global, 0);
			Mat_<double>& shape2D = clm_model->detected_landmarks;
			Mat_<int>& visibilities = clm_model->patch_experts.visibilities[0][idx];
			int n = shape2D.rows/2;
			
			int numFacePoints = 17;
			int numEyeBrowPoints = 10;
			int numNoseBridgePoints = 4;

			//Point facePoints[1][numFacePoints+numEyeBrowPoints];
			vector<Point> facePointsVec(numFacePoints+numEyeBrowPoints,Point(0,0));
			vector<Point> noseBridgePoints(numNoseBridgePoints,Point(0,0));

			RotatedRect minFaceRect;

			Scalar colour;
			faceAvailableFlag = false;
			int maxX = 0;
			int minX = 1000;
			int maxY = 0;
			int minY = 1000;
			int imgW = captureCopy.cols;
			int imgH = captureCopy.rows;

			for( int i = 0; i < n; ++i)
			{	
				Point featurePoint = Point((int)shape2D.at<double>(i), (int)shape2D.at<double>(i +n));
				if(i<numFacePoints)
				{
					facePointsVec[numFacePoints-i-1] = featurePoint;
					colour = Scalar(0,255,0);
				}
				else if(i<numEyeBrowPoints+numFacePoints)
				{
					facePointsVec[i] = featurePoint;
				}
				else
				{
					faceAvailableFlag = true;
				}


				if(visibilities.at<int>(i))
				{
					// A rough heuristic for drawn point size
					int thickness = (int)std::ceil(5.0* ((double)captured_image.cols) / 640.0);
					int thickness_2 = (int)std::ceil(1.5* ((double)captured_image.cols) / 640.0);

					cv::circle(captured_image, featurePoint, 1, Scalar(0,0,255), thickness);
					cv::circle(captured_image, featurePoint, 1, colour, thickness_2);
				}	

			}
			Size imageSize = captured_image.size();
			faceMask=Mat::ones(imageSize, CV_8UC3);
			minFaceRect = minAreaRect(Mat(facePointsVec));

			float RectAngle = minFaceRect.angle;
			Size minRectSize = minFaceRect.size;
			Point2f center = minFaceRect.center;

			if (RectAngle < -45.)
			{	
				RectAngle += 90.0;
	            swap(minRectSize.width, minRectSize.height);
        	}

        	Mat M = getRotationMatrix2D(minFaceRect.center, RectAngle, 1.0);
        	warpAffine(captureCopy, segFace, M, captureCopy.size(), INTER_CUBIC);
        	center.y -= minRectSize.height*0.25;
        	minRectSize.height *= 1.5;
        	getRectSubPix(segFace,minRectSize,center,croppedFace);

			Point2f rect_points[4]; 
			minFaceRect.points( rect_points );

	        for( int j = 0; j < 4; j++ )
	        {
	        	line(captured_image, rect_points[j], rect_points[(j+1)%4], colour, 1, 8 );
	        }
	        rectangle(segFace, Rect((center-Point2f(minRectSize.width/2,minRectSize.height/2)), minRectSize), Scalar(0,255,0));

			imshow("facePoints", segFace);
			waitKey(1);
			
			resize(croppedFace,segFace,Size(400,400));
			GaussianBlur(segFace, segFace, Size(7,7), 1, 1);

			//-------------
			if(detection_certainty > 1)
				detection_certainty = 1;
			if(detection_certainty < -1)
				detection_certainty = -1;

			detection_certainty = (detection_certainty + 1)/(visualisation_boundary +1);

			// A rough heuristic for box around the face width
			int thickness = (int)std::ceil(2.0* ((double)captured_image.cols) / 640.0);
			
			Vec6d pose_estimate_to_draw = CLMTracker::GetCorrectedPoseCameraPlane(*clm_model, fx, fy, cx, cy, *clm_parameters);

			// Draw it in reddish if uncertain, blueish if certain
			CLMTracker::DrawBox(captured_image, pose_estimate_to_draw, Scalar((1-detection_certainty)*255.0,0, detection_certainty*255), thickness, fx, fy, cx, cy);

		}

		// Work out the framerate
		if(frame_count % 10 == 0)
		{      
			t1 = cv::getTickCount();
			fps = 10.0 / (double(t1-t0)/cv::getTickFrequency());    // 10.0 because the if is executed every 10 frames and the result has to be averaged over 10
			t0 = t1;
		}
		
		// Write out the framerate on the image before displaying it
		char fpsC[255];
		sprintf(fpsC, "%d", (int)fps);
		string fpsSt("FPS:");
		fpsSt += fpsC;
		cv::putText(captured_image, fpsSt, cv::Point(10,20), CV_FONT_HERSHEY_SIMPLEX, 0.5, CV_RGB(255,0,0));		
		
		
		if(!clm_parameters->quiet_mode)
		{
			namedWindow("tracking_result",1);		
			imshow("tracking_result", captured_image);

			if(!depth_image.empty())
			{
				// Division needed for visualisation purposes
				imshow("depth", depth_image/2000.0);
			}
		}

		// Output the detected facial landmarks
		if(!landmark_output_files.empty())
		{
			landmarks_output_file << frame_count + 1 << " " << detection_success;
			for (int i = 0; i < clm_model->pdm.NumberOfPoints() * 2; ++ i)
			{
				landmarks_output_file << " " << clm_model->detected_landmarks.at<double>(i) << " ";
			}
			landmarks_output_file << endl;
		}

		// Output the detected facial landmarks
		if(!landmark_3D_output_files.empty())
		{
			landmarks_3D_output_file << frame_count + 1 << " " << detection_success;
			Mat_<double> shape_3D = clm_model->GetShape(fx, fy, cx, cy);
			for (int i = 0; i < clm_model->pdm.NumberOfPoints() * 3; ++i)
			{
				landmarks_3D_output_file << " " << shape_3D.at<double>(i);
			}
			landmarks_3D_output_file << endl;
		}

		// Output the estimated head pose
		if(!pose_output_files.empty())
		{
			pose_output_file << frame_count + 1 << " " << (float)frame_count * 1000/30 << " " << detection_success << " " << pose_estimate_CLM[0] << " " << pose_estimate_CLM[1] << " " << pose_estimate_CLM[2] << " " << pose_estimate_CLM[3] << " " << pose_estimate_CLM[4] << " " << pose_estimate_CLM[5] << endl;
		}				

		cout << " "  << pose_estimate_CLM[0] << " " << pose_estimate_CLM[1] << " " << pose_estimate_CLM[2] << " " << pose_estimate_CLM[3] << " " << pose_estimate_CLM[4] << " " << pose_estimate_CLM[5] << endl;
		// output the tracked video
		if(!tracked_videos_output.empty())
		{		
			writerFace << captured_image;
		}

		if(faceAvailableFlag)
		{
			ImageOf<PixelRgb> &outSegImage = imageSegOut.prepare(); //get an output image
			IplImage* IPLfromMat = new IplImage(segFace);
			outSegImage.resize(IPLfromMat->width,IPLfromMat->height);
			IplImage * iplYarpImage = (IplImage*)outSegImage.getIplImage();
			if (IPL_ORIGIN_TL == IPLfromMat->origin){
					cvCopy(IPLfromMat, iplYarpImage, 0);
			}
			else{
					cvFlip(IPLfromMat, iplYarpImage, 0);
			}
			//cvCvtColor(iplYarpImage, iplYarpImage, CV_BGR2RGB);
			imageSegOut.write();
		}
		// Update the frame count
		frame_count++;
		
		return true;
	}
	
    bool interruptModule()
    {
        cout<<"Interrupting your module, for port cleanup"<<endl;
        imageIn.interrupt();
        return true;
    }

    bool close()
    {
		cout<<"Calling close function\n";
		delete clm_parameters;
		delete clm_model;
		imageIn.close();
		
		
		clm_model->Reset();
		pose_output_file.close();
		landmarks_output_file.close();
		
		
        return true;
    }	
    
protected:

private:

};

// ----------------
// ----- Main -----
// ----------------

int main (int argc, char **argv)
{

	MyModule thisModule;
	ResourceFinder rf;
	cout<<"Object initiated!"<<endl;
	
	rf.configure(argc,argv);
	
	//thisModule.configure(rf);
	thisModule.runModule(rf);
	return 0;
}

