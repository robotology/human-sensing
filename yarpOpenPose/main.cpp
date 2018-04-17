/*
 * Copyright (C) 2016 iCub Facility - Istituto Italiano di Tecnologia
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
// C++ std library dependencies
#include <atomic>
#include <chrono>
#include <cstdio>
#include <string>
#include <thread>
#include <vector>
#include <array>

// OpenCV dependencies
#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>

// Other 3rdpary depencencies
#include <gflags/gflags.h> // DEFINE_bool, DEFINE_int32, DEFINE_int64, DEFINE_uint64, DEFINE_double, DEFINE_string
#include <glog/logging.h> // google::InitGoogleLogging, CHECK, CHECK_EQ, LOG, VLOG, ...

// OpenPose dependencies
 #include <openpose/core/headers.hpp>
 #include <openpose/experimental/headers.hpp>
 #include <openpose/filestream/headers.hpp>
 #include <openpose/gui/headers.hpp>
 #include <openpose/pose/headers.hpp>
 #include <openpose/producer/headers.hpp>
 #include <openpose/thread/headers.hpp>
 #include <openpose/utilities/headers.hpp>
 #include <openpose/wrapper/headers.hpp>

// yarp dependencies
#include <yarp/os/BufferedPort.h>
#include <yarp/os/ResourceFinder.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/Network.h>
#include <yarp/os/Log.h>
#include <yarp/os/Time.h>
#include <yarp/os/LogStream.h>
#include <yarp/os/Semaphore.h>
#include <yarp/sig/Image.h>
#include <yarp/os/RpcClient.h>

#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>

/********************************************************/
class ImageInput : public op::WorkerProducer<std::shared_ptr<std::vector<op::Datum>>>
{
private:
    std::string moduleName;
    yarp::os::RpcServer handlerPort;
    yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelRgb> > inPort;
    bool mClosed;
public:
    /********************************************************/
    ImageInput(const std::string& moduleName) : mClosed{false}
    {
        this->moduleName = moduleName;
    }

    /********************************************************/
    ~ImageInput()
    {
        inPort.close();
    }

    /********************************************************/
    void initializationOnThread() {
        inPort.open("/" + moduleName + "/image:i");
    }

    /********************************************************/
    std::shared_ptr<std::vector<op::Datum>> workProducer()
    {
        if (mClosed)
        {
            mClosed = true;
            return nullptr;
        }
        else
        {
            // Create new datum
            auto datumsPtr = std::make_shared<std::vector<op::Datum>>();
            datumsPtr->emplace_back();
            auto& datum = datumsPtr->at(0);
            yarp::sig::ImageOf<yarp::sig::PixelRgb> *inImage = inPort.read();
            cv::Mat in_cv = cv::cvarrToMat((IplImage *)inImage->getIplImage());
            // Fill datum
            datum.cvInputData = in_cv;
            // If empty frame -> return nullptr
            if (datum.cvInputData.empty())
            {
                mClosed = true;
                op::log("Empty frame detected. Closing program.", op::Priority::Max);
                datumsPtr = nullptr;
            }
            return datumsPtr;
        }
    }

    /********************************************************/
    bool isFinished() const
    {
        return mClosed;
    }
};

/********************************************************/
class ImageProcessing : public op::Worker<std::shared_ptr<std::vector<op::Datum>>>
{
private:
    std::string moduleName;
    yarp::os::BufferedPort<yarp::os::Bottle>  targetPort;
public:
    std::map<unsigned int, std::string> mapParts {
        {0,  "Nose"},
        {1,  "Neck"},
        {2,  "RShoulder"},
        {3,  "RElbow"},
        {4,  "RWrist"},
        {5,  "LShoulder"},
        {6,  "LElbow"},
        {7,  "LWrist"},
        {8,  "RHip"},
        {9,  "RKnee"},
        {10, "RAnkle"},
        {11, "LHip"},
        {12, "LKnee"},
        {13, "LAnkle"},
        {14, "REye"},
        {15, "LEye"},
        {16, "REar"},
        {17, "LEar"},
        {18, "Background"}
    };
    /********************************************************/
    ImageProcessing(const std::string& moduleName)
    {
        this->moduleName = moduleName;
    }

    ~ImageProcessing()
    {
        targetPort.close();
    }
    /********************************************************/
    void initializationOnThread()
    {
        targetPort.open("/"+ moduleName + "/target:o");
    }

    /********************************************************/
    void work(std::shared_ptr<std::vector<op::Datum>>& datumsPtr)
    {
        if (datumsPtr != nullptr && !datumsPtr->empty())
        {
            yarp::os::Bottle &peopleBottle = targetPort.prepare();
            peopleBottle.clear();
            yarp::os::Bottle &mainList = peopleBottle.addList();
            auto& tDatumsNoPtr = *datumsPtr;
            //Record people pose data
            op::Array<float> pose(tDatumsNoPtr.size());
            for (auto i = 0; i < tDatumsNoPtr.size(); i++)
            {
                pose = tDatumsNoPtr[i].poseKeypoints;

                if (!pose.empty() && pose.getNumberDimensions() != 3)
                    op::error("pose.getNumberDimensions() != 3.", __LINE__, __FUNCTION__, __FILE__);

                const auto numberPeople = pose.getSize(0);
                const auto numberBodyParts = pose.getSize(1);

                //std::cout << "Number of people is " << numberPeople << std::endl;

                for (auto person = 0 ; person < numberPeople ; person++)
                {
                    yarp::os::Bottle &peopleList = mainList.addList();
                    for (auto bodyPart = 0 ; bodyPart < numberBodyParts ; bodyPart++)
                    {
                        yarp::os::Bottle &partList = peopleList.addList();
                        const auto finalIndex = 3*(person*numberBodyParts + bodyPart);
                        partList.addString(mapParts[bodyPart].c_str());
                        partList.addDouble(pose[finalIndex]);
                        partList.addDouble(pose[finalIndex+1]);
                        partList.addDouble(pose[finalIndex+2]);
                    }
                }
            }
            if (peopleBottle.size())
                targetPort.write();
        }
    }
};

/**********************************************************/
class ImageOutput : public op::WorkerConsumer<std::shared_ptr<std::vector<op::Datum>>>
{
private:
    std::string moduleName;
    yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelRgb> > outPort;
    yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelRgb> > outPortPropag;
public:

    /********************************************************/
    ImageOutput(const std::string& moduleName)
    {
        this->moduleName = moduleName;
    }

    /********************************************************/
    void initializationOnThread()
    {
        outPort.open("/" + moduleName + "/image:o");
        outPortPropag.open("/" + moduleName + "/propag:o");
    }

    /********************************************************/
    ~ImageOutput()
    {
        outPort.close();
        outPortPropag.close();
    }

    /********************************************************/
    void workConsumer(const std::shared_ptr<std::vector<op::Datum>>& datumsPtr)
    {
        if (datumsPtr != nullptr && !datumsPtr->empty())
        {
            yarp::sig::ImageOf<yarp::sig::PixelRgb> &outImage  = outPort.prepare();
            yarp::sig::ImageOf<yarp::sig::PixelRgb> &outImagePropag  = outPortPropag.prepare();

            outImage.resize(datumsPtr->at(0).cvOutputData.cols, datumsPtr->at(0).cvOutputData.rows);
            outImagePropag.resize(datumsPtr->at(0).cvInputData.cols, datumsPtr->at(0).cvInputData.rows);

            IplImage colour = datumsPtr->at(0).cvOutputData;
            cvCopy( &colour, (IplImage *) outImage.getIplImage());
            outPort.write();

            IplImage colourOrig = datumsPtr->at(0).cvInputData;
            cvCopy( &colourOrig, (IplImage *) outImagePropag.getIplImage());
            outPortPropag.write();
        }
        else
            op::log("Nullptr or empty datumsPtr found.", op::Priority::Max, __LINE__, __FUNCTION__, __FILE__);
    }
};

/********************************************************/
class Module : public yarp::os::RFModule
{
private:
    yarp::os::ResourceFinder    *rf;
    yarp::os::RpcServer         rpcPort;

    std::string                 model_name;
    std::string                 model_folder;
    std::string                 net_resolution;
    std::string                 img_resolution;
    int                         num_gpu;
    int                         num_gpu_start;
    int                         num_scales;
    float                       scale_gap;
    int                         keypoint_scale;
    bool                        heatmaps_add_parts;
    bool                        heatmaps_add_bkg;
    bool                        heatmaps_add_PAFs;
    int                         heatmaps_scale_mode;
    int                         render_pose;
    int                         part_to_show;
    bool                        disable_blending;
    double                      alpha_pose;
    double                      alpha_heatmap;
    double                      render_threshold;
    bool                        body_enable;
    bool                        hand_enable;
    std::string                 hand_net_resolution;
    int                         hand_scale_number;
    double                      hand_scale_range;
    bool                        hand_tracking;
    double                      hand_alpha_pose;
    double                      hand_alpha_heatmap;
    double                      hand_render_threshold;
    int                         hand_render;

    ImageInput                  *inputClass;
    ImageProcessing             *processingClass;
    ImageOutput                 *outputClass;

    op::Wrapper<std::vector<op::Datum>> opWrapper{op::ThreadManagerMode::Asynchronous};

    bool                        closing;
public:
    /********************************************************/
    bool configure(yarp::os::ResourceFinder &rf)
    {
        this->rf=&rf;
        std::string moduleName = rf.check("name", yarp::os::Value("yarpOpenPose"), "module name (string)").asString();

        model_name = rf.check("model_name", yarp::os::Value("COCO"), "Model to be used e.g. COCO, MPI, MPI_4_layers. (string)").asString();
        model_folder = rf.check("model_folder", yarp::os::Value("/models"), "Folder where the pose models (COCO and MPI) are located. (string)").asString();
        net_resolution = rf.check("net_resolution", yarp::os::Value("656x368"), "The resolution of the net, multiples of 16. (string)").asString();
        img_resolution = rf.check("img_resolution", yarp::os::Value("320x240"), "The resolution of the image (display and output). (string)").asString();
        num_gpu = rf.check("num_gpu", yarp::os::Value("1"), "The number of GPU devices to use.(int)").asInt();
        num_gpu_start = rf.check("num_gpu_start", yarp::os::Value("0"), "The GPU device start number.(int)").asInt();
        num_scales = rf.check("num_scales", yarp::os::Value("1"), "Number of scales to average.(int)").asInt();
        scale_gap = rf.check("scale_gap", yarp::os::Value("0.3"), "Scale gap between scales. No effect unless num_scales>1. Initial scale is always 1. If you want to change the initial scale,"
                                                                " you actually want to multiply the `net_resolution` by your desired initial scale.(float)").asDouble();

        keypoint_scale = rf.check("keypoint_scale", yarp::os::Value("0"), "Scaling of the (x,y) coordinates of the final pose data array (op::Datum::pose), i.e. the scale of the (x,y) coordinates that"
                                                                " will be saved with the `write_pose` & `write_pose_json` flags. Select `0` to scale it to the original source resolution, `1`"
                                                                " to scale it to the net output size (set with `net_resolution`), `2` to scale it to the final output size (set with "
                                                                " `resolution`), `3` to scale it in the range [0,1], and 4 for range [-1,1]. Non related with `num_scales` and `scale_gap`.(int)").asInt();

        heatmaps_add_parts = rf.check("heatmaps_add_parts", yarp::os::Value("false"), "If true, it will add the body part heatmaps to the final op::Datum::poseHeatMaps array (program speed will decrease). Not"
                                                                " required for our library, enable it only if you intend to process this information later. If more than one `add_heatmaps_X`"
                                                                " flag is enabled, it will place then in sequential memory order: body parts + bkg + PAFs. It will follow the order on"
                                                                " POSE_BODY_PART_MAPPING in `include/openpose/pose/poseParameters.hpp`.(bool)").asBool();
        heatmaps_add_bkg = rf.check("heatmaps_add_bkg", yarp::os::Value("false"), "Same functionality as `add_heatmaps_parts`, but adding the heatmap corresponding to background. (bool)").asBool();

        heatmaps_add_PAFs = rf.check("heatmaps_add_PAFs", yarp::os::Value("false"),"Same functionality as `add_heatmaps_parts`, but adding the PAFs.(bool)").asBool();
        heatmaps_scale_mode = rf.check("heatmaps_scale_mode", yarp::os::Value("2"), "Set 0 to scale op::Datum::poseHeatMaps in the range [0,1], 1 for [-1,1]; and 2 for integer rounded [0,255].(int)").asInt();
        //no_render_output = rf.check("no_render_output", yarp::os::Value("false"), "If false, it will fill image with the original image + desired part to be shown. If true, it will leave them empty.(bool)").asBool();
        render_pose = rf.check("render_pose", yarp::os::Value("2"), "Set to 0 for no rendering, 1 for CPU rendering (slightly faster), and 2 for GPU rendering(int)").asInt();
        part_to_show = rf.check("part_to_show", yarp::os::Value("0"),"Part to show from the start.(int)").asInt();
        disable_blending = rf.check("disable_blending", yarp::os::Value("false"), "If false, it will blend the results with the original frame. If true, it will only display the results.").asBool();
        alpha_pose = rf.check("alpha_pose", yarp::os::Value("0.6"), "Blending factor (range 0-1) for the body part rendering. 1 will show it completely, 0 will hide it.(double)").asDouble();
        alpha_heatmap = rf.check("alpha_heatmap", yarp::os::Value("0.7"), "Blending factor (range 0-1) between heatmap and original frame. 1 will only show the heatmap, 0 will only show the frame.(double)").asDouble();
        render_threshold = rf.check("render_threshold", yarp::os::Value("0.05"), "Only estimated keypoints whose score confidences are higher than this threshold will be rendered. Generally, a high threshold (> 0.5) will only render very clear body parts.(double)").asDouble();
        body_enable = rf.check("body_enable", yarp::os::Value("true"), "Disable body keypoint detection. Option only possible for faster (but less accurate) face. (bool)").asBool();
        hand_enable = rf.check("hand_enable", yarp::os::Value("false"), "Enables hand keypoint detection. It will share some parameters from the body pose, e.g."
                                                                " `model_folder`. Analogously to `--face`, it will also slow down the performance, increase"
                                                                " the required GPU memory and its speed depends on the number of people.(int)").asBool();
        hand_net_resolution = rf.check("hand_net_resolution", yarp::os::Value("368x368"), "Multiples of 16 and squared. Analogous to `net_resolution` but applied to the hand keypoint (string)").asString();
        hand_scale_number = rf.check("hand_scale_number", yarp::os::Value("1"), "Analogous to `scale_number` but applied to the hand keypoint detector.(int)").asInt();
        hand_scale_range = rf.check("hand_scale_range", yarp::os::Value("0.4"), "Analogous purpose than `scale_gap` but applied to the hand keypoint detector. Total range"
                                                                " between smallest and biggest scale. The scales will be centered in ratio 1. E.g. if"
                                                                " scaleRange = 0.4 and scalesNumber = 2, then there will be 2 scales, 0.8 and 1.2.(double)").asDouble();
        hand_tracking = rf.check("hand_tracking", yarp::os::Value("false"), "Adding hand tracking might improve hand keypoints detection for webcam (if the frame rate"
                                                                " is high enough, i.e. >7 FPS per GPU) and video. This is not person ID tracking, it"
                                                                " simply looks for hands in positions at which hands were located in previous frames, but"
                                                                " it does not guarantee the same person ID among frames (bool)").asBool();
        hand_alpha_pose = rf.check("hand_alpha_pose", yarp::os::Value("0.6"), "Analogous to `alpha_pose` but applied to hand.(double)").asDouble();
        hand_alpha_heatmap = rf.check("hand_alpha_heatmap", yarp::os::Value("0.7"), "Analogous to `alpha_heatmap` but applied to hand.(double)").asDouble();
        hand_render_threshold = rf.check("hand_render_threshold", yarp::os::Value("0.2"), "Analogous to `render_threshold`, but applied to the hand keypoints.(double)").asDouble();
        hand_render = rf.check("hand_render", yarp::os::Value("-1"), "Analogous to `render_pose` but applied to the hand. Extra option: -1 to use the same(int)").asInt();

        setName(moduleName.c_str());
        rpcPort.open(("/"+getName("/rpc")).c_str());
        closing = false;

        yDebug() << "Starting yarpOpenPose";

        // Applying user defined configuration
        auto outputSize = op::flagsToPoint(img_resolution, img_resolution);
        // netInputSize
        auto netInputSize = op::flagsToPoint(net_resolution, net_resolution);
        //pose model
        op::PoseModel poseModel = gflagToPoseModel(model_name);
        // scaleMode
        op::ScaleMode keypointScale = op::flagsToScaleMode(keypoint_scale);
        // handNetInputSize
        auto handNetInputSize = op::flagsToPoint(hand_net_resolution, hand_net_resolution);

        // heatmaps to add
        std::vector<op::HeatMapType> heatMapTypes = gflagToHeatMaps(heatmaps_add_parts, heatmaps_add_bkg, heatmaps_add_PAFs);
        op::check(heatmaps_scale_mode >= 0 && heatmaps_scale_mode <= 2, "Non valid `heatmaps_scale_mode`.", __LINE__, __FUNCTION__, __FILE__);
        op::ScaleMode heatMapsScaleMode = (heatmaps_scale_mode == 0 ? op::ScaleMode::PlusMinusOne : (heatmaps_scale_mode == 1 ? op::ScaleMode::ZeroToOne : op::ScaleMode::UnsignedChar ));

        // Pose configuration
        const op::WrapperStructPose wrapperStructPose{body_enable, netInputSize, outputSize, keypointScale, num_gpu, num_gpu_start, num_scales, scale_gap,
                                                      op::flagsToRenderMode(render_pose), poseModel, !disable_blending, (float)alpha_pose, (float)alpha_heatmap,
                                                      part_to_show, model_folder, heatMapTypes, heatMapsScaleMode, (float)render_threshold};

        // Hand configuration
        const op::WrapperStructHand wrapperStructHand{hand_enable, handNetInputSize, hand_scale_number, (float)hand_scale_range,
                                                                                                    hand_tracking, op::flagsToRenderMode(hand_render, render_pose),
                                                                                                    (float)hand_alpha_pose, (float)hand_alpha_heatmap, (float)hand_render_threshold};

        opWrapper.configure(wrapperStructPose, wrapperStructHand, op::WrapperStructInput{}, op::WrapperStructOutput{});

        yDebug() << "Starting thread(s)";
        attach(rpcPort);
        opWrapper.start();
        yDebug() << "Done starting thread(s)";

        // User processing
        inputClass = new ImageInput(moduleName);
        outputClass = new ImageOutput(moduleName);
        processingClass = new ImageProcessing(moduleName);

        inputClass->initializationOnThread();
        outputClass->initializationOnThread();
        processingClass->initializationOnThread();

        yDebug() << "Running processses";

        return true;
    }

    /**********************************************************/
    op::PoseModel gflagToPoseModel(const std::string& poseModeString)
    {
        //op::log("", op::Priority::Low, __LINE__, __FUNCTION__, __FILE__);
        if (poseModeString == "COCO")
            return op::PoseModel::COCO_18;
        else if (poseModeString == "MPI")
            return op::PoseModel::MPI_15;
        else if (poseModeString == "MPI_4_layers")
            return op::PoseModel::MPI_15_4;
        else
        {
            yError() << "String does not correspond to any model (COCO, MPI, MPI_4_layers)";
            return op::PoseModel::COCO_18;
        }
    }

    /**********************************************************/
    op::ScaleMode gflagToScaleMode(const int scaleMode)
    {
        if (scaleMode == 0)
            return op::ScaleMode::InputResolution;
        else if (scaleMode == 1)
            return op::ScaleMode::NetOutputResolution;
        else if (scaleMode == 2)
            return op::ScaleMode::OutputResolution;
        else if (scaleMode == 3)
            return op::ScaleMode::ZeroToOne;
        else if (scaleMode == 4)
            return op::ScaleMode::PlusMinusOne;
        else
        {
            const std::string message = "String does not correspond to any scale mode: (0, 1, 2, 3, 4) for (InputResolution, NetOutputResolution, OutputResolution, ZeroToOne, PlusMinusOne).";
            yError() << message;
            return op::ScaleMode::InputResolution;
        }
    }

    /**********************************************************/
    std::vector<op::HeatMapType> gflagToHeatMaps(const bool heatmaps_add_parts, const bool heatmaps_add_bkg, const bool heatmaps_add_PAFs)
    {

        std::vector<op::HeatMapType> heatMapTypes;
        if (heatmaps_add_parts)
            heatMapTypes.emplace_back(op::HeatMapType::Parts);
        if (heatmaps_add_bkg)
            heatMapTypes.emplace_back(op::HeatMapType::Background);
        if (heatmaps_add_PAFs)
            heatMapTypes.emplace_back(op::HeatMapType::PAFs);
        return heatMapTypes;
    }

    /**********************************************************/
    bool close()
    {
        delete inputClass;
        delete outputClass;
        delete processingClass;
        return true;
    }
    /**********************************************************/
    bool quit(){
        closing = true;
        opWrapper.stop();
        return true;
    }
    /********************************************************/
    double getPeriod()
    {
        return 0.1;
    }
    /********************************************************/
    bool updateModule()
    {
        auto datumToProcess = inputClass->workProducer();
        if (datumToProcess != nullptr)
        {
            auto successfullyEmplaced = opWrapper.waitAndEmplace(datumToProcess);
            // Pop frame
            std::shared_ptr<std::vector<op::Datum>> datumProcessed;
            if (successfullyEmplaced && opWrapper.waitAndPop(datumProcessed))
            {
                outputClass->workConsumer(datumProcessed);
                processingClass->work(datumProcessed);
            }
            else
                yError() << "Processed datum could not be emplaced.";
        }
        return !closing;
    }
};

int main(int argc, char *argv[])
{
    yarp::os::Network::init();
    // Initializing google logging (Caffe uses it for logging)
    google::InitGoogleLogging("yarpOpenPose");
    // Parsing command line flags
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    yarp::os::Network yarp;
    if (!yarp.checkNetwork())
    {
        yError("YARP server not available!");
        return 1;
    }

    Module module;
    yarp::os::ResourceFinder rf;

    rf.setVerbose( true );
    rf.setDefaultContext( "yarpOpenPose" );
    rf.setDefaultConfigFile( "yarpOpenPose.ini" );
    rf.setDefault("name","yarpOpenPose");
    rf.configure(argc,argv);

    return module.runModule(rf);
}
