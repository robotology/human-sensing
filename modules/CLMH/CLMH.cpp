/**
 * This code is modified for the iCub humanoid robot through YARP connection.
 * The program reads the image from one camera (robot's eye) and tries to detect a face and exports the result on a new view.
 * It also sends the estimated pose of the head to
 * 
 * 
 * 
 * output:
 *  1- Sends the 3D pose of the Gaze to the port /clmgaze/3dpose/out
 * this can be used to connect to the iKinGazeControl  module
 *
 * 2- Sends a bottle including the corners of a bounding box and the center of two eyes
 * this can be used as an input for the face recognition module (for Vadim)
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
#include <yarp/os/Bottle.h>
#include <yarp/dev/all.h>
#include <yarp/dev/Drivers.h>
#include <yarp/dev/ControlBoardInterfaces.h>
#include <yarp/dev/GazeControl.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/math/Math.h>
#include <yarp/os/Time.h>
#include <yarp/os/RateThread.h>
// opencv
/**
#include <cv.h>
#include <highgui.h>
**/
#include <opencv2/videoio/videoio.hpp>  // Video write
#include <opencv2/videoio/videoio_c.h>  // Video write
// c++
/**
#include <iostream>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <cstdio>
**/
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <random>
#include <time.h>
// CLM library
/**
#include <CLM.h>
#include <CLMTracker.h>
#include <CLMParameters.h>
#include <CLM_utils.h>
**/
#include <CLM_core.h>
// others
#include <gsl/gsl_math.h>
#include <deque>

/**
#define CTRL_THREAD_PER 0.02 // [s]
#define PRINT_STATUS_PER 1.0 // [s]
#define STORE_POI_PER 3.0 // [s]
#define SWITCH_STATE_PER 10.0 // [s]
#define STILL_STATE_TIME 5.0 // [s]
#define STATE_TRACK 0
#define STATE_RECALL 1
#define STATE_WAIT 2
#define STATE_STILL 3
**/
// --------------------------------------------------------------
// ------------------
// ----- Macros -----
// ------------------
// --------------------------------------------------------------
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

double diffclock(clock_t clock1,clock_t clock2)
{
    double diffticks=clock1-clock2;
    double diffms=(diffticks*10)/CLOCKS_PER_SEC;
    return diffms;
}



#define FATAL_STREAM( stream ) \
printErrorAndAbort( std::string( "Fatal error: " ) + stream )
// --------------------------------------------------------------
// ----------------------
// ----- Namespaces -----
// ----------------------
// --------------------------------------------------------------
using namespace std;
using namespace cv;
using namespace yarp::os;
using namespace yarp::sig;
using namespace yarp::dev;
using namespace yarp::math;

// --------------------------------------------------------------
// -----------------
// ----- Class -----
// -----------------
// --------------------------------------------------------------
class MyModule:public RFModule
{

private:

	//Mat cvImage;
	//BufferedPort<Bottle> inPort, outPort;
	
    BufferedPort<ImageOf<PixelRgb> > imageIn;  // make a port for reading images
    BufferedPort<ImageOf<PixelRgb> > imageOut; // make a port for passing the result to
    BufferedPort<Bottle> targetPort; // for Vadim

    BufferedPort<Bottle> posePort; // for Ali
	
	IplImage* cvImage;
    IplImage* display;

    Mat_<float> depth_image;
    Mat_<uchar> grayscale_image;
    Mat captured_image;
    Mat_<short> depth_image_16_bit;
    Mat_<double> shape_3D;

		
	vector<string> arguments;					// The arguments from the input 
	vector<string> files, depth_directories, pose_output_files, tracked_videos_output, landmark_output_files, landmark_3D_output_files;

	bool use_camera_plane_pose, done, cx_undefined, use_depth, write_settings, ok2, gazeControl;
    bool runGaze; // set this value to true to run gaze control on the icub
	int device, f_n, frame_count;
    int state, startup_context_id, jnt;
    int thickness;
    int gazeChangeFlag;

	float fx,fy,cx,cy; 							// parameters of the camera

	string current_file;

	double fps;
    double visualisation_boundary;
    double detection_certainty;

	int64 t1,t0;								// time parameters for calculating and storing the framerate of the camera

	CLMTracker::CLMParameters* clm_parameters;
	CLMTracker::CLM* clm_model;

	std::ofstream pose_output_file;
	std::ofstream landmarks_output_file;	
	std::ofstream landmarks_3D_output_file;
	
	PolyDriver clientGazeCtrl;
	IGazeControl *igaze;
    IPositionControl *pos;
    IEncoders *encs;
    IVelocityControl *vel;

    Vector tmp, position, command, encoders, velocity, acceleration;
    Vector fp, x;
    Vec6d pose_estimate_CLM, pose_estimate_to_draw;

    // parameters for transfromation of pose w.r.t eye to pose w.r.t root
    Vector pose_act, ori_act;       // actual pose and actual orientation of the left eye of icub
    Vector pose_clm, pose_robot;    // estimated pose by clm, caculated pose w.r.t the root of the robot
    Vector pose_clm_left_corner, pose_clm_right_corner; // for storing the data for the corners of the eyes
    Matrix H;                       // transformation matrx


    std::default_random_engine generator;
    std::normal_distribution<double> distribution;
    clock_t beginTime, endTime;
    double timeToKeepAGaze;
    bool oneIter;


public:

    MyModule(){} 							// constructor
    ~MyModule(){}							// deconstructor


    double getPeriod()						// the period of the loop.
    {
        return 0.0; 						// 0 is equal to the real-time
    }


    // --------------------------------------------------------------
    bool configure(yarp::os::ResourceFinder &rf)
    {

        // --- open the ports ---
        ok2 = imageIn.open("/clmgaze/image/in");
        ok2 = ok2 && imageOut.open("/clmgaze/image/out");
        if (!ok2)
        {
			fprintf(stderr, "Error. Failed to open image ports. \n");
			return false;
		}	

        ok2 = targetPort.open("/clmgaze/cornerPose/out");
        if (!ok2)
        {
            fprintf(stderr,"Error. failed to open a port for the pose. \n");
            return false;
        }

        ok2 = posePort.open("/clmgaze/centerPose/out");
        if (!ok2)
        {
            fprintf(stderr,"Error. failed to open a port for the 3D pose. \n");
            return false;
        }
        // ---

		arguments.push_back("-f");							// provide two default arguments in case we want to use no real camera
		arguments.push_back("../../videos/default.wmv"); 	// the video file 
		device = 0;   										// camera
        fx = 225.904; //443; //211; //225.904; //409.9; //225.904; //500; (default)
        fy = 227.041; //444; //211; //227.041;//409.023; //227.041; //500; (default)
        cx = 157.875; //344; //161; //157.858; //337.575; //157.858; //0; (default)
        cy = 113.51; //207; //128; //113.51; //250.798; //113.51; //0; (default)
		clm_parameters = new CLMTracker::CLMParameters(arguments);

        // checking the otehr face detectors
        //cout <<  "Current Face Detector : "<<clm_parameters->curr_face_detector << endl;
        clm_parameters->curr_face_detector = CLMTracker::CLMParameters::HAAR_DETECTOR; // change to HAAR
        //cout <<  "Current Face Detector : "<<clm_parameters->curr_face_detector << endl;

		//CLMTracker::CLMParameters clm_parameters(arguments);
		CLMTracker::get_video_input_output_params(files, depth_directories, pose_output_files, tracked_videos_output, landmark_output_files, landmark_3D_output_files, use_camera_plane_pose, arguments);
		CLMTracker::get_camera_params(device, fx, fy, cx, cy, arguments);    
		//CLMTracker::CLM clm_model(clm_parameters.model_location);	
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
            //IplImage *cvImage = (IplImage*)imgTmp->getIplImage();
			ImageOf<PixelRgb> &outImage = imageOut.prepare(); 		//get an output image
			outImage.resize(imgTmp->width(), imgTmp->height());		
			outImage = *imgTmp;
			display = (IplImage*) outImage.getIplImage();
            //captured_image = display;
            captured_image = cvarrToMat(display);  // convert to MAT format
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

        gazeControl = true;  // true if you want to activate the gaze controller
        
        Property option;
        option.put("device","gazecontrollerclient");
        option.put("remote","/iKinGazeCtrl");
        option.put("local","/client/gaze");

        runGaze = true;   // change to use the gaze controller
        if (runGaze)
        {
            if (!clientGazeCtrl.open(option))
            {
                fprintf(stderr,"Error. could not open the gaze controller!");
                return false;
            }


            igaze = NULL;
            if (clientGazeCtrl.isValid())
            {
                clientGazeCtrl.view(igaze);     // open the view
            }
            else
            {
                INFO_STREAM( "could not open");
                return false;
            }

            // latch the controller context in order to preserve it after closing the module
            igaze->storeContext(&startup_context_id);

        }

        // create a normal distribution PDF for timing the gaze change
        generator;
        std::normal_distribution<double> distribution(5.0,1.0);
        beginTime = clock();
        timeToKeepAGaze = distribution(generator);
        gazeChangeFlag = 1;
        oneIter = false;

        /* --- Testing other things
        Property option;
        option.put("device", "remote_controlboard");
        option.put("local", "/test/client");   //local port names
        //option.put("remote", "/icubSim/head"); // for simulation
        option.put("remote", "/icub/head");
        */

        /*
        Property option("(device gazecontrollerclient)");
        option.put("remote","/iKinGazeCtrl");
        option.put("local","/client/gaze"); //("local","/gaze_client");
        */


        /*
        clientGazeCtrl.view(pos);
        clientGazeCtrl.view(vel);
        clientGazeCtrl.view(encs);
        */

        /*
        // set trajectory time:
        igaze->setNeckTrajTime(0.8);
        igaze->setEyesTrajTime(0.4);

        // put the gaze in tracking mode, so that
        // when the torso moves, the gaze controller
        // will compensate for it
        igaze->setTrackingMode(true);
        igaze->bindNeckPitch();
        */

        /*
          // when not using gaze controller
        jnt = 0;

        pos->getAxes(&jnt);
        // 6 joints for the head
        // cout << "----------------------- " << jnt <<"------------------------" << endl;

        encoders.resize(jnt);
        command.resize(jnt);
        tmp.resize(jnt);
        velocity.resize(jnt);
        acceleration.resize(jnt);

        for (int iii = 0; iii < jnt; iii++)
        {
           acceleration[iii] = 50.0;
        }
        pos->setRefAccelerations(acceleration.data());

        for (int iii = 0; iii < jnt; iii++)
        {
             velocity[iii] = 10.0;
            pos->setRefSpeed(iii, velocity[iii]);
        }

        printf("waiting for encoders");
         while(!encs->getEncoders(encoders.data()))
         {
             Time::delay(0.1);
             printf(".");
         }
         printf("\n;");

         command=encoders;
         //now set the shoulder to some value
         command[0]=-10; // up-down (pitch)
         command[1]=0; // roll
         command[2]=-10; // yaw
         command[3]=0;
         command[4]=0;
         command[5]=0;
         pos->positionMove(command.data()); // just to test
        */

        return true;
    }
    // --------------------------------------------------------------
	bool updateModule()
	{

		//cout << __FILE__ << ": " << __LINE__ << endl;
        //cout << "////////////////" << beginTime << "hhhhhhhhhhhhhhhh" << endl;

		ImageOf<PixelRgb> *imgTmp = imageIn.read();  // read an image
		if (imgTmp == NULL) 
		{
            FATAL_STREAM( "Failed to read image!" );
			return true;
		}

        //IplImage *cvImage = (IplImage*)imgTmp->getIplImage();
		ImageOf<PixelRgb> &outImage = imageOut.prepare(); //get an output image
		outImage.resize(imgTmp->width(), imgTmp->height());		
		outImage = *imgTmp;
		display = (IplImage*) outImage.getIplImage();
        //captured_image = display;
        captured_image = cvarrToMat(display); // conver to MAT format

		// If optical centers are not defined just use center of image
		if(cx_undefined)
		{
			cx = captured_image.cols / 2.0f;
			cy = captured_image.rows / 2.0f;
			cx_undefined = true;
		}

        /*
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
        */
        //cout << __FILE__ << ": " << __LINE__ << endl;

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
            depth_image_16_bit = imread(string(dst), -1);

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
        detection_certainty = clm_model->detection_certainty;

		
		// Only draw if the reliability is reasonable, the value is slightly ad-hoc
		if(detection_certainty < visualisation_boundary)
		{
			CLMTracker::Draw(captured_image, *clm_model);


            // ---------------------------
            // REZA - for Vadim
            // ---------------------------
            //
            // 1- Finding the center of the eyes
            //
            // eye features are at 36-48 (6 feature for each eye)
            double mean_xx_right_eye = 0;
            double mean_yy_right_eye = 0;
            double mean_xx_left_eye = 0;
            double mean_yy_left_eye = 0;


            for (int ii = 36; ii < 48; ii++)
            {
                double xx = clm_model->detected_landmarks.at<double>(ii);
                double yy = clm_model->detected_landmarks.at<double>(ii+clm_model->pdm.NumberOfPoints());


                //cv::putText(captured_image, "+", cv::Point(xx,yy), CV_FONT_HERSHEY_SIMPLEX, 0.5, CV_RGB(255,255,0));
                if (ii < 42){
                    //cv::circle(captured_image,cv::Point(xx,yy),3,CV_RGB(255,255,0));
                    mean_xx_right_eye = mean_xx_right_eye + xx;
                    mean_yy_right_eye = mean_yy_right_eye + yy;
                }
                else
                {
                    //cv::circle(captured_image,cv::Point(xx,yy),3,CV_RGB(0,255,0));
                    mean_xx_left_eye = mean_xx_left_eye + xx;
                    mean_yy_left_eye = mean_yy_left_eye + yy;
                }
            }
            mean_xx_left_eye = mean_xx_left_eye / 6;
            mean_yy_left_eye = mean_yy_left_eye / 6;
            mean_xx_right_eye = mean_xx_right_eye / 6;
            mean_yy_right_eye = mean_yy_right_eye / 6;

            /*
            cout << mean_xx_left_eye << endl;
            cout << mean_yy_left_eye << endl;
            cout << mean_xx_right_eye << endl;
            cout << mean_yy_right_eye << endl;
            */

            //cv::circle(captured_image,cv::Point(mean_xx_left_eye,mean_yy_left_eye),10,CV_RGB(255,255,0));
            //cv::circle(captured_image,cv::Point(mean_xx_right_eye,mean_yy_right_eye),10,CV_RGB(0,255,0));


            // 2- Finding the corner of the box

            // initialize with the x,y of the first landmark
            double xx_min = clm_model->detected_landmarks.at<double>(0);
            double xx_max = clm_model->detected_landmarks.at<double>(0);
            double yy_min = clm_model->detected_landmarks.at<double>(clm_model->pdm.NumberOfPoints());
            double yy_max = clm_model->detected_landmarks.at<double>(clm_model->pdm.NumberOfPoints());

            for (int ii=0; ii<clm_model->pdm.NumberOfPoints(); ii++)
            {
                double xx = clm_model->detected_landmarks.at<double>(ii);
                double yy = clm_model->detected_landmarks.at<double>(ii+clm_model->pdm.NumberOfPoints());
                // finding the corners
                if (xx < xx_min)
                    xx_min = xx;
                if (xx > xx_max)
                    xx_max = xx;
                if (yy < yy_min)
                    yy_min = yy;
                if (yy > yy_max);
                    yy_max = yy;
            }

            //cv::circle(captured_image,cv::Point(xx_min,yy_min),10,CV_RGB(255,255,255));
            //cv::circle(captured_image,cv::Point(xx_max,yy_max),10,CV_RGB(255,255,255));
            //cout << "[" << xx_min << "," << xx_max  << "," << yy_min << "," << yy_max << "]" <<endl;
            //cout << "[" << mean_xx_left_eye << "," << mean_yy_left_eye  << "," << mean_xx_right_eye << "," << mean_yy_right_eye << "]" <<endl;

            Bottle& poseVadim = targetPort.prepare();
            poseVadim.clear();
            poseVadim.addDouble(xx_min);
            poseVadim.addDouble(yy_min);
            poseVadim.addDouble(xx_max);
            poseVadim.addDouble(yy_max);
            poseVadim.addDouble(mean_xx_left_eye);
            poseVadim.addDouble(mean_yy_left_eye);
            poseVadim.addDouble(mean_xx_right_eye);
            poseVadim.addDouble(mean_yy_right_eye);

            targetPort.write();
            // --------------------------------


			if(detection_certainty > 1)
				detection_certainty = 1;
			if(detection_certainty < -1)
				detection_certainty = -1;

            // cout << "Certainty : " << detection_certainty << "---" << visualisation_boundary << endl;
			detection_certainty = (detection_certainty + 1)/(visualisation_boundary +1);
            cout << "Normalized Certainty : " << detection_certainty << endl;

			// A rough heuristic for box around the face width
            thickness = (int)std::ceil(2.0* ((double)captured_image.cols) / 640.0);
			
            pose_estimate_to_draw = CLMTracker::GetCorrectedPoseCameraPlane(*clm_model, fx, fy, cx, cy, *clm_parameters);

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
                imshow("depth", depth_image/2000.0);    // Division needed for visualisation purposes
			}
		}

		// Output the detected facial landmarks
        /*
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
            shape_3D = clm_model->GetShape(fx, fy, cx, cy);
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
        */

        //cout << "CLM Estimated pose and quaternion : "  << pose_estimate_CLM[0] << " " << pose_estimate_CLM[1] << " " << pose_estimate_CLM[2] << " " << pose_estimate_CLM[3] << " " << pose_estimate_CLM[4] << " " << pose_estimate_CLM[5] << endl;

        if (runGaze)
        {
        if (detection_success)
        {
            if (abs(pose_estimate_CLM[0]) < 1000 & abs(pose_estimate_CLM[1])< 1000 & abs(pose_estimate_CLM[2]) < 1000 )
            {

                // 1- Finding the center of the eyes
                //
                // eye features are at 36-48 (6 feature for each eye)
                double mean_xx_right_eye = 0;
                double mean_yy_right_eye = 0;
                double mean_xx_left_eye = 0;
                double mean_yy_left_eye = 0;


                for (int ii = 36; ii < 48; ii++)
                {
                    double xx = clm_model->detected_landmarks.at<double>(ii);
                    double yy = clm_model->detected_landmarks.at<double>(ii+clm_model->pdm.NumberOfPoints());


                    if (ii < 42){
                        mean_xx_right_eye = mean_xx_right_eye + xx;
                        mean_yy_right_eye = mean_yy_right_eye + yy;
                    }
                    else
                    {
                        mean_xx_left_eye = mean_xx_left_eye + xx;
                        mean_yy_left_eye = mean_yy_left_eye + yy;
                    }
                }
                mean_xx_left_eye = mean_xx_left_eye / 6;
                mean_yy_left_eye = mean_yy_left_eye / 6;
                mean_xx_right_eye = mean_xx_right_eye / 6;
                mean_yy_right_eye = mean_yy_right_eye / 6;



                // transforming the pose w.r.t the root of the robot
                igaze->getLeftEyePose(pose_act,ori_act);
                H = axis2dcm(ori_act);
                H(0,3) = pose_act[0];
                H(1,3) = pose_act[1];
                H(2,3) = pose_act[2];

                /*
                pose_clm.resize(4);
                pose_clm[0] = pose_estimate_CLM[0] / 1000; //convert to [m]
                pose_clm[1] = pose_estimate_CLM[1] / 1000;
                pose_clm[2] = pose_estimate_CLM[2] / 1000;
                pose_clm[3] = 1;
                pose_robot = H*pose_clm;
                */

                /*
                // ============= METHOD - 1 (using counters) ==================
                static int lookCounter = 0;
                // using a counter

                //int face = 1 + rand() % 100;
                // int face = 1 + rand() % 3;


                if (lookCounter < 100)
                {
                    cout << "looking at center " << lookCounter <<endl;
                    pose_clm.resize(4);
                    pose_clm[0] = pose_estimate_CLM[0] / 1000; //convert to [m]
                    pose_clm[1] = pose_estimate_CLM[1] / 1000;
                    pose_clm[2] = pose_estimate_CLM[2] / 1000;
                    pose_clm[3] = 1;
                    //cout << pose_clm[0] << " , " << pose_clm[1] << " , " << pose_clm[2] << endl;
                    pose_robot = H * pose_clm;
                    //cout << pose_robot[0] << " , " << pose_robot[1] << " , " << pose_robot[2] << endl;
                    lookCounter += 1;

                    //cout << lookCounter << endl;
                }
                else if (lookCounter >= 100 && lookCounter < 120)
                {
                    cout << "looking at corner "  << lookCounter <<endl;
                    pose_clm_left_corner.resize(4);
                    pose_clm_left_corner[0] = pose_estimate_CLM[0]/1000 + 0.1; //mean_xx_left_eye / 1000; //convert to [m]
                    pose_clm_left_corner[1] = pose_estimate_CLM[1]/1000; //mean_yy_left_eye / 1000;
                    pose_clm_left_corner[2] = pose_estimate_CLM[2] / 1000;
                    pose_clm_left_corner[3] = 1;
                    //cout << pose_clm_left_corner[0] << " , " << pose_clm_left_corner[1] << " , " << pose_clm_left_corner[2] << endl;
                    pose_robot = H * pose_clm_left_corner;
                    //cout << pose_robot[0] << " , " << pose_robot[1] << " , " << pose_robot[2] << endl;
                    lookCounter += 1;

                }
                else
                {
                    lookCounter = 0;
                }
                */





                // ============== Method 2 - using elapsed time =====================
                endTime = clock();
                // cout << "Flag : " << gazeChangeFlag << endl;
                // start timing
                timeToKeepAGaze = distribution(generator);
                // cout << "time to keep : " << 3 + timeToKeepAGaze << " diffclock : " << diffclock(endTime,beginTime) << endl;
                if (abs(diffclock(endTime,beginTime)) > 40.0)
                {
                    cout << "############################### Changing Gaze >>> " << endl;


                    //randomly
                    //int output;
                    gazeChangeFlag = 1 + (rand() % (int)(3 - 1 + 1));
                    cout << "looking at : " << gazeChangeFlag << endl;

                    /*
                    // cycling through different gaze flags

                    if (gazeChangeFlag == 1)
                    {
                        gazeChangeFlag = 2;
                    }
                    else if (gazeChangeFlag == 2 & ~oneIter)
                    {
                        gazeChangeFlag = 1;
                        oneIter = ~oneIter;
                    }
                    else if (gazeChangeFlag == 2 & oneIter)
                    {
                        gazeChangeFlag = 3;
                        oneIter = ~oneIter;
                    }

                    else if (gazeChangeFlag == 3)
                    {
                        gazeChangeFlag = 1;
                    }
                    */
                    beginTime = clock();
                    endTime = clock();
                    timeToKeepAGaze = distribution(generator);

                }



                if (gazeChangeFlag == 1)
                {
                    cout << "looking at left corner " <<endl;
                    pose_clm.resize(4);
                    pose_clm[0] = pose_estimate_CLM[0] / 1000 - 0.05; //convert to [m]
                    pose_clm[1] = pose_estimate_CLM[1] / 1000;
                    pose_clm[2] = pose_estimate_CLM[2] / 1000;
                    pose_clm[3] = 1;
                    pose_robot = H * pose_clm;
                }
                else if (gazeChangeFlag == 2)
                {
                    cout << "looking at right corner "  <<endl;
                    pose_clm_left_corner.resize(4);
                    pose_clm_left_corner[0] = pose_estimate_CLM[0]/1000 + 0.05; //mean_xx_left_eye / 1000; //convert to [m]
                    pose_clm_left_corner[1] = pose_estimate_CLM[1]/1000; //mean_yy_left_eye / 1000;
                    pose_clm_left_corner[2] = pose_estimate_CLM[2]/1000;
                    pose_clm_left_corner[3] = 1;
                    //cout << pose_clm_left_corner[0] << " , " << pose_clm_left_corner[1] << " , " << pose_clm_left_corner[2] << endl;
                    pose_robot = H * pose_clm_left_corner;
                    //cout << pose_robot[0] << " , " << pose_robot[1] << " , " << pose_robot[2] << endl;
                }
                else if (gazeChangeFlag == 3)
                {
                    cout << "looking at mouth "  <<endl;
                    pose_clm_left_corner.resize(4);
                    pose_clm_left_corner[0] = pose_estimate_CLM[0]/1000; //mean_xx_left_eye / 1000; //convert to [m]
                    pose_clm_left_corner[1] = pose_estimate_CLM[1]/1000+ 0.07; //mean_yy_left_eye / 1000;
                    pose_clm_left_corner[2] = pose_estimate_CLM[2]/1000;
                    pose_clm_left_corner[3] = 1;
                    //cout << pose_clm_left_corner[0] << " , " << pose_clm_left_corner[1] << " , " << pose_clm_left_corner[2] << endl;
                    pose_robot = H * pose_clm_left_corner;
                    //cout << pose_robot[0] << " , " << pose_robot[1] << " , " << pose_robot[2] << endl;
                }



                Bottle& fp = posePort.prepare();
                fp.clear();
                fp.addDouble(pose_robot[0]);
                fp.addDouble(pose_robot[1]);
                fp.addDouble(pose_robot[2]);
                posePort.write();
                cout << pose_robot[0] << " , " << pose_robot[1] << " , " << pose_robot[2] << endl;



            }
        }
        }
        //if(!tracked_videos_output.empty())
        //{
        //    writerFace << captured_image;     // output the tracked video
        //}

        frame_count++;                          // Update the frame count
		imageOut.write();

		return true;
	}
    // --------------------------------------------------------------
    bool interruptModule()
    {
        cout<<"Interrupting your module, for port cleanup"<<endl;
        //inPort.interrupt();
        imageIn.interrupt();
        
        if (clientGazeCtrl.isValid())
        {

            cout << "restoring the head context ..." << endl;
            fp.resize(3);
            fp[0]=-1;
            fp[1]=0;
            fp[2]=0;

            igaze->lookAtFixationPoint(fp); // move the gaze to the desired fixation point
            igaze->waitMotionDone();


 
        }
        return true;
    }
    // --------------------------------------------------------------
    bool close()
    {
		cout<<"Calling close function\n";
		delete clm_parameters;
		delete clm_model;
		//inPort.close();
		//outPort.close();
        targetPort.writeStrict();
        targetPort.close();
        posePort.writeStrict();
        posePort.close();
		imageIn.close();
		imageOut.close();
		clientGazeCtrl.close();
		//frame_count = 0;
        clm_model->Reset();               // reset the model
		pose_output_file.close();
		landmarks_output_file.close();
		
        return true;
    }	
    // --------------------------------------------------------------

protected:

};

// --------------------------------------------------------------
// ----------------
// ----- Main -----
// ----------------
// --------------------------------------------------------------

int main (int argc, char **argv)
{
    Network yarp;

    if (!yarp.checkNetwork())
    {
        fprintf(stderr,"Error. Yarp server not available!");
        return false;
    }

	MyModule thisModule;
	ResourceFinder rf;
	cout<<"Object initiated!"<<endl;
	thisModule.configure(rf);
	thisModule.runModule();
	return 0;
}

