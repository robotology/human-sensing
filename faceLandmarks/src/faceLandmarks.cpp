/*
 * Copyright (C) 2011 Department of Robotics Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
 * Author: Vadim Tikhanoff
 * email:  vadim.tikhanoff@iit.it
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * A copy of the license can be found at
 * http://www.robotcub.org/icub/license/gpl.txt
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
 */

#include <yarp/sig/all.h>
#include "faceLandmarks.h"

/**********************************************************/
bool FACEModule::configure(yarp::os::ResourceFinder &rf)
{
    rf.setVerbose();
    moduleName = rf.check("name", yarp::os::Value("faceLandmarks"), "module name (string)").asString();
    skipFrames = rf.check("skipFrames", yarp::os::Value(2), "skip frames (int)").asInt32();
    downsampleRatio = rf.check("downsample", yarp::os::Value(1), "face downsample ratio (int)").asInt32();

    predictorFile = rf.check("faceLandmarksFile", yarp::os::Value("shape_predictor_68_face_landmarks.dat"), "path name (string)").asString();

    std::string firstStr = rf.findFile(predictorFile.c_str());

    setName(moduleName.c_str());

    handlerPortName =  "/";
    handlerPortName += getName();
    handlerPortName +=  "/rpc:i";

    cntxHomePath = rf.getHomeContextPath().c_str();

    if (!rpcPort.open(handlerPortName.c_str()))
    {
        yError() << getName().c_str() << " Unable to open port " << handlerPortName.c_str();
        return false;
    }

    attach(rpcPort);
    closing = false;

    /* create the thread and pass pointers to the module parameters */
    faceManager = new FACEManager( moduleName, firstStr, cntxHomePath, skipFrames, downsampleRatio );

    /* now start the thread to do the work */
    faceManager->open();

    return true ;
}

/**********************************************************/
bool FACEModule::interruptModule()
{
    rpcPort.interrupt();
    return true;
}

/**********************************************************/
bool FACEModule::close()
{
    //rpcPort.close();
    yDebug() << "starting the shutdown procedure";
    faceManager->interrupt();
    faceManager->close();
    yDebug() << "deleting thread";
    delete faceManager;
    closing = true;
    yDebug() << "done deleting thread";
    return true;
}

/**********************************************************/
bool FACEModule::updateModule()
{
    return !closing;
}

/**********************************************************/
double FACEModule::getPeriod()
{
    return 0.01;
}

/************************************************************************/
bool FACEModule::attach(yarp::os::RpcServer &source)
{
    return this->yarp().attachAsServer(source);
}

/**********************************************************/
bool FACEModule::display(const std::string& element, const std::string& value)
{
    bool returnVal = false;

    if (element == "landmarks" || element == "points" || element == "labels" || element == "dark-mode")
    {
        if (element == "landmarks")
        {
            if (value=="on")
            {
                faceManager->displayLandmarks=true;
                returnVal = true;
            }
            else if (value=="off")
            {
                faceManager->displayLandmarks = false;
                returnVal = true;
            }
            else
                yInfo() << "error setting value for landmarks";
        }
        if (element == "points")
        {
            if (value=="on")
            {
                faceManager->displayPoints=true;
                returnVal = true;
            }
            else if (value=="off")
            {
                faceManager->displayPoints = false;
                returnVal = true;
            }
            else
                yInfo() << "error setting value for points";
        }
        if (element == "labels")
        {
            if (value=="on")
            {
                faceManager->displayLabels=true;
                returnVal = true;
            }
            else if (value=="off")
            {
                faceManager->displayLabels = false;
                returnVal = true;
            }
            else
                yInfo() << "error setting value for labels";
        }
        if (element == "dark-mode")
        {
            if (value=="on")
            {
                faceManager->displayDarkMode=true;
                returnVal = true;
            }
            else if (value=="off")
            {
                faceManager->displayDarkMode = false;
                returnVal = true;
            }
            else
                yInfo() << "error setting value for darkMode";
        }
        //yInfo() << "should now display \"landmarks\" " << faceManager->displayLandmarks << "\"points\"" << faceManager->displayPoints << "\"labels\"" << faceManager->displayLabels  << "\"dark-mode\"" << faceManager->displayDarkMode;
    }
    else
    {
        returnVal = false;
        yInfo() << "Error in display request";
    }
    return returnVal;
}

/**********************************************************/
bool FACEModule::quit()
{
    closing = true;
    return true;
}

/**********************************************************/
FACEManager::~FACEManager()
{
}

/**********************************************************/
FACEManager::FACEManager( const std::string &moduleName,  const std::string &predictorFile, const std::string &cntxHomePath, const int &skipFrames, const int &downsampleRatio)
{
    yDebug() << "initialising Variables";
    this->moduleName = moduleName;
    this->predictorFile = predictorFile;
    this->cntxHomePath = cntxHomePath;
    this->skipFrames = skipFrames;
    this->downsampleRatio = downsampleRatio;
    yInfo() << "contextPATH = " << cntxHomePath.c_str();
    yInfo() << "skipFrames = " << skipFrames;
    yInfo() << "downsampleRatio = " << downsampleRatio;
}

/**********************************************************/
bool FACEManager::open()
{
    this->useCallback();

    //create all ports
    inImgPortName = "/" + moduleName + "/image:i";
    BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelRgb> >::open( inImgPortName.c_str() );

    outImgPortName = "/" + moduleName + "/image:o";
    imageOutPort.open( outImgPortName.c_str() );

    outPropagImgPortName = "/" + moduleName + "/propag_image:o";
    propagImageOutPort.open( outPropagImgPortName.c_str() );

    outTargetPortName = "/" + moduleName + "/target:o";
    targetOutPort.open( outTargetPortName.c_str() );

    outLandmarksPortName = "/" + moduleName + "/landmarks:o";
    landmarksOutPort.open( outLandmarksPortName.c_str() );

    yDebug() << "path is: " << predictorFile.c_str();

    faceDetector = dlib::get_frontal_face_detector();
    dlib::deserialize(predictorFile.c_str()) >> sp;

    color = cv::Scalar( 0, 255, 0 );

    displayLandmarks = true;
    displayPoints = false;
    displayLabels = false;
    displayDarkMode = false;

    count = 0;

    draw_res = 1;

    return true;
}

/**********************************************************/
void FACEManager::close()
{
    mutex.wait();
    yDebug() <<"now closing ports...";
    imageOutPort.writeStrict();
    imageOutPort.close();
    propagImageOutPort.close();
    imageInPort.close();
    targetOutPort.close();
    landmarksOutPort.close();
    BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelRgb> >::close();
    mutex.post();
    yDebug() <<"finished closing the read port...";
}

/**********************************************************/
void FACEManager::interrupt()
{
    yDebug() << "cleaning up...";
    yDebug() << "attempting to interrupt ports";
    BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelRgb> >::interrupt();
    yDebug() << "finished interrupt ports";
}

/**********************************************************/
void FACEManager::onRead(yarp::sig::ImageOf<yarp::sig::PixelRgb> &img)
{
    mutex.wait();
    yarp::sig::ImageOf<yarp::sig::PixelRgb> &outImg  = imageOutPort.prepare();
    yarp::sig::ImageOf<yarp::sig::PixelRgb> &propagOutImg  = propagImageOutPort.prepare();
    yarp::os::Bottle &target=targetOutPort.prepare();
    yarp::os::Bottle &landmarks=landmarksOutPort.prepare();
    target.clear();
    
    // Propagate input image to output
    propagOutImg = img;
   
    // Get the image from the yarp port
    imgMat = yarp::cv::toCvMat(img);

    draw_res = imgMat.cols / 320;

    cv::Mat im_small, im_display;

    cv::resize(imgMat, im_small, cv::Size(), 1.0/downsampleRatio, 1.0/downsampleRatio);

    // Change to dlib's image format. No memory is copied.
    dlib::cv_image<dlib::bgr_pixel> cimg_small(im_small);
    dlib::cv_image<dlib::bgr_pixel> cimg(imgMat);

    std::vector<dlib::rect_detection> detections;

    // Detect faces on resize image
    if ( count % skipFrames == 0 )
    {
        faceDetector(cimg_small, detections);
    }



    // Find the pose of each face.
    std::vector<dlib::full_object_detection> shapes;
    std::vector<std::pair<int, double >> idTargets;

    for (unsigned long i = 0; i < detections.size(); ++i)
    {
        // Resize obtained rectangle for full resolution image.
        dlib::rectangle r(
                    (long)(detections[i].rect.left() * downsampleRatio),
                    (long)(detections[i].rect.top() * downsampleRatio),
                    (long)(detections[i].rect.right() * downsampleRatio),
                    (long)(detections[i].rect.bottom() * downsampleRatio)
                    );

        // Landmark detection on full sized image
        dlib::full_object_detection shape = sp(cimg, r);
        shapes.push_back(shape);

        cv::Point pt1, pt2;
        pt1.x = detections[i].rect.tl_corner().x()* downsampleRatio;
        pt1.y = detections[i].rect.tl_corner().y()* downsampleRatio;
        pt2.x = detections[i].rect.br_corner().x()* downsampleRatio;
        pt2.y = detections[i].rect.br_corner().y()* downsampleRatio;

        double areaCalculation =0.0;
        areaCalculation = (std::fabs(pt2.x-pt1.x)*std::fabs(pt2.y-pt1.y));

        idTargets.push_back(std::make_pair(i, areaCalculation));

        if (idTargets.size()>0)
        {
            std::sort(idTargets.begin(), idTargets.end(), [](const std::pair<int, double> &left, const std::pair<int, double> &right) {
                return left.second > right.second;
            });
        }
    }   

    if (detections.size() > 0 )
    {
        landmarks.clear();
        for (int k=0; k< idTargets.size(); k++)
        {
            const dlib::full_object_detection& d = shapes[idTargets[k].first];

            if (displayDarkMode)
                imgMat.setTo(cv::Scalar(0, 0, 0));

            // Custom Face Render
            if (displayLandmarks)
                render_face(imgMat, d);

            
            yarp::os::Bottle &landM = landmarks.addList();
            for (int f=1; f<shapes[idTargets[k].first].num_parts(); f++)
            {

                if (f != 17 || f != 22 || f != 27 || f != 42 || f != 48)
                {
                    yarp::os::Bottle &temp = landM.addList();
                    temp.addInt32(d.part(f).x());
                    temp.addInt32(d.part(f).y());
                }
            }
            landM.addFloat64(detections[idTargets[k].first].detection_confidence);
            if (displayPoints || displayLabels)
            {
                int pointSize = landmarks.get(0).asList()->size();

                if (displayPoints)
                {
                    for (size_t i = 0; i < pointSize - 1; i++)
                    {
                        int pointx = landmarks.get(0).asList()->get(i).asList()->get(0).asInt32();
                        int pointy = landmarks.get(0).asList()->get(i).asList()->get(1).asInt32();
                        cv::Point center(pointx, pointy);
                        circle(imgMat, center, 3, cv::Scalar(255, 0 , 0), -1, 8);
                    }
                }
                if (displayLabels)
                {
                    for (size_t i = 0; i < pointSize - 1; i++)
                    {
                        int pointx = landmarks.get(0).asList()->get(i).asList()->get(0).asInt32();
                        int pointy = landmarks.get(0).asList()->get(i).asList()->get(1).asInt32();
                        cv::Point center(pointx, pointy);
                        std::string s = std::to_string(i);
                        putText(imgMat, s, cvPoint(pointx, pointy), cv::FONT_HERSHEY_COMPLEX_SMALL, 0.6, cvScalar(200,200,250), 1, cv::LINE_AA);
                    }
                }
            }

            rightEye.x = d.part(42).x() + ((d.part(45).x()) - (d.part(42).x()))/2;
            rightEye.y = d.part(43).y() + ((d.part(46).y()) - (d.part(43).y()))/2;
            leftEye.x  = d.part(36).x() + ((d.part(39).x()) - (d.part(36).x()))/2;
            leftEye.y  = d.part(38).y() + ((d.part(41).y()) - (d.part(38).y()))/2;

            //yDebug("rightEye %d %d leftEye %d %d ", rightEye.x, rightEye.y, leftEye.x, leftEye.y);
            //draw center of each eye
            circle(imgMat, leftEye , draw_res*2, cv::Scalar( 0, 0, 255 ), -1);
            circle(imgMat, rightEye , draw_res*2, cv::Scalar( 0, 0, 255 ), -1);
        
            cv::Point pt1, pt2;
            pt1.x = detections[idTargets[k].first].rect.tl_corner().x()* downsampleRatio;
            pt1.y = detections[idTargets[k].first].rect.tl_corner().y()* downsampleRatio;
            pt2.x = detections[idTargets[k].first].rect.br_corner().x()* downsampleRatio;
            pt2.y = detections[idTargets[k].first].rect.br_corner().y()* downsampleRatio;

            if (pt1.x < 2)
                pt1.x = 1;
            if (pt1.x > imgMat.cols-2)
                pt1.x = imgMat.cols-1;
            if (pt1.y < 2)
                pt1.y = 1;
            if (pt1.y > imgMat.rows)
                pt1.y = imgMat.rows-1;

            if (pt2.x < 2)
                pt2.x = 1;
            if (pt2.x > imgMat.cols-2)
                pt2.x = imgMat.cols-1;
            if (pt2.y < 2)
                pt2.y = 1;
            if (pt2.y > imgMat.rows-2)
                pt2.y = imgMat.rows-2;


            yarp::os::Bottle &pos = target.addList();
            pos.addFloat64(pt1.x);
            pos.addFloat64(pt1.y);
            pos.addFloat64(pt2.x);
            pos.addFloat64(pt2.y);

            cv::Point biggestpt1, biggestpt2;
            biggestpt1.x = detections[idTargets[0].first].rect.tl_corner().x()* downsampleRatio;
            biggestpt1.y = detections[idTargets[0].first].rect.tl_corner().y()* downsampleRatio;
            biggestpt2.x = detections[idTargets[0].first].rect.br_corner().x()* downsampleRatio;
            biggestpt2.y = detections[idTargets[0].first].rect.br_corner().y()* downsampleRatio;

            rectangle(imgMat, biggestpt1, biggestpt2, cv::Scalar( 0, 255, 0 ), draw_res, 8, 0);
        }
        targetOutPort.write();
        if (landmarksOutPort.getOutputCount()>0)
        {
            landmarksOutPort.write();
        }
    }

    outImg.resize(imgMat.size().width, imgMat.size().height);
    outImg = yarp::cv::fromCvMat<yarp::sig::PixelRgb>(imgMat);

    imageOutPort.write();
    propagImageOutPort.write();

    mutex.post();
}

void FACEManager::draw_polyline(cv::Mat &img, const dlib::full_object_detection& d, const int start, const int end, bool isClosed)
{
    std::vector <cv::Point> points;
    for (int i = start; i <= end; ++i)
    {
        points.push_back(cv::Point(d.part(i).x(), d.part(i).y()));
    }
    cv::polylines(img, points, isClosed, cv::Scalar(0,255,0), draw_res, 16);

}

void FACEManager::render_face (cv::Mat &img, const dlib::full_object_detection& d)
{
    DLIB_CASSERT
    (
     d.num_parts() == 68,
     "\n\t Invalid inputs were given to this function. "
     << "\n\t d.num_parts():  " << d.num_parts()
     );

    draw_polyline(img, d, 0, 16);           // Jaw line
    draw_polyline(img, d, 17, 21);          // Left eyebrow
    draw_polyline(img, d, 22, 26);          // Right eyebrow
    draw_polyline(img, d, 27, 30);          // Nose bridge
    draw_polyline(img, d, 30, 35, true);    // Lower nose
    draw_polyline(img, d, 36, 41, true);    // Left eye
    draw_polyline(img, d, 42, 47, true);    // Right Eye
    draw_polyline(img, d, 48, 59, true);    // Outer lip
    draw_polyline(img, d, 60, 67, true);    // Inner lip

}

void FACEManager::drawLandmarks(cv::Mat &mat, const dlib::full_object_detection &d)
{
    //draw face contour, jaw
    for (unsigned long i = 1; i <= 16; ++i)
        line(mat, cv::Point(d.part(i).x()/2, d.part(i).y()/2), cv::Point(d.part(i-1).x()/2, d.part(i-1).y()/2),  color);

    //draw right eyebrow
    for (unsigned long i = 18; i <= 21; ++i)
        line(mat, cv::Point(d.part(i).x()/2, d.part(i).y()/2), cv::Point(d.part(i-1).x()/2, d.part(i-1).y()/2),  color);

    //draw left eyebrow
    for (unsigned long i = 23; i <= 26; ++i)
        line(mat, cv::Point(d.part(i).x()/2, d.part(i).y()/2), cv::Point(d.part(i-1).x()/2, d.part(i-1).y()/2),  color);

    //draw nose
    for (unsigned long i = 28; i <= 30; ++i)
        line(mat, cv::Point(d.part(i).x()/2, d.part(i).y()/2), cv::Point(d.part(i-1).x()/2, d.part(i-1).y()/2),  color);

    //draw nostrils
    line(mat, cv::Point(d.part(30).x()/2, d.part(30).y()/2), cv::Point(d.part(35).x()/2, d.part(35).y()/2),  color);
    for (unsigned long i = 31; i <= 35; ++i)
        line(mat, cv::Point(d.part(i).x()/2, d.part(i).y()/2), cv::Point(d.part(i-1).x()/2, d.part(i-1).y()/2),  color);

    //draw right eye
    for (unsigned long i = 37; i <= 41; ++i)
        line(mat, cv::Point(d.part(i).x()/2, d.part(i).y()/2), cv::Point(d.part(i-1).x()/2, d.part(i-1).y()/2),  color);
    line(mat, cv::Point(d.part(36).x()/2, d.part(36).y()/2), cv::Point(d.part(41).x()/2, d.part(41).y()/2),  color);

    //draw left eye
    for (unsigned long i = 43; i <= 47; ++i)
        line(mat, cv::Point(d.part(i).x()/2, d.part(i).y()/2), cv::Point(d.part(i-1).x()/2, d.part(i-1).y()/2),  color);
    line(mat, cv::Point(d.part(42).x()/2, d.part(42).y()/2), cv::Point(d.part(47).x()/2, d.part(47).y()/2),  color);

    //draw outer mouth
    for (unsigned long i = 49; i <= 59; ++i)
        line(mat, cv::Point(d.part(i).x()/2, d.part(i).y()/2), cv::Point(d.part(i-1).x()/2, d.part(i-1).y()/2),  color);
    line(mat, cv::Point(d.part(48).x()/2, d.part(48).y()/2), cv::Point(d.part(59).x()/2, d.part(59).y()/2),  color);

    //draw inner mouth
    line(mat, cv::Point(d.part(60).x()/2, d.part(60).y()/2), cv::Point(d.part(67).x()/2, d.part(67).y()/2),  color);
    for (unsigned long i = 61; i <= 67; ++i)
        line(mat, cv::Point(d.part(i).x()/2, d.part(i).y()/2), cv::Point(d.part(i-1).x()/2, d.part(i-1).y()/2),  color);

}
//empty line to make gcc happy
