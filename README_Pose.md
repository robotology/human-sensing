# yarpOpenPose dependencies installation
This readme contains information on how to build and install all required dependencies for the yarpOpenPose module.

Table of Contents
=================
* [Requirements](#requirements)
* [Required Dependencies](#generic_dep)
* [OpenPose Installation](#openposeinstallation)
* [Compilation](#compilation)
* [Parameters](#parameters)

#### Requirements

* **Ubuntu** (tested on 14 and 16) or **Windows** (tested on 10). We do not support any other OS but the community has been able to install it on: CentOS, Windows 7, and Windows 8.
* **NVIDIA graphics card** with at least 1.6 GB available (the nvidia-smi command checks the available GPU memory in Ubuntu).
* **CUDA** and **cuDNN** installed. Note: We found OpenPose working with cuDNN 5.1 ~10% faster than with cuDNN 6.
* At least **2 GB** of free **RAM** memory.
* Highly recommended: A **CPU** with at least **8 cores**.

Note: These requirements assume the default configuration (i.e. --net_resolution "656x368" and num_scales 1). You might need more (with a greater net resolution and/or number of scales) or less resources (with smaller net resolution and/or using the MPI and MPI_4 models).

#### Required Dependencies

- [YARP](https://github.com/robotology/yarp)
- [iCub](https://github.com/robotology/icub-main)
- [icub-contrib-common](https://github.com/robotology/icub-contrib-common)
- [OpenCV](http://opencv.org/downloads.html)
- [openPose](https://github.com/CMU-Perceptual-Computing-Lab/openpose)
- [CUDA](https://developer.nvidia.com/cuda-downloads)
- [cuDNN](https://developer.nvidia.com/cudnn)
- [Caffe](http://caffe.berkeleyvision.org/installation.html)

#### OpenPose Installation

    $ git clone https://github.com/CMU-Perceptual-Computing-Lab/openpose.git

Follow the installation procedure at the following link [manual compilation](https://github.com/CMU-Perceptual-Computing-Lab/openpose/blob/master/doc/installation_cmake.md).

####Side note
Instead of installing the library with `make install`I suggest to add to your `bash` of export the `openpose_ROOT` variable:

    export openpose_ROOT=/path/to/the/root/of/openpose

#### Compilation

While in the root folder of `yarpOpenPose` do:

    mkdir build
    cd build
    ccmake ..
    configure and generate
    make install

#### Parameters
The required parameters can be modified at runtime using the installed **yarpOpenPose.ini** file

* **name**: Name of the module.
* **model_name**: Model to be used e.g. COCO, MPI, MPI_4_layers. (string)
* **model_folder**: Folder where the pose models (COCO and MPI) are located. (string)
* **img_resolution**: The resolution of the image (display and output). (string)
* **num_gpu**: The number of GPU devices to use.(int)
* **num_gpu_start**: The GPU device start number.(int)
* **num_scales**: Number of scales to average.(int)
* **scale_gap**: Scale gap between scales. No effect unless num_scales>1. Initial scale is always 1. If you want to change the initial scale, you actually want to multiply the `net_resolution` by your desired initial scale.(float)
* **keypoint_scale**: Scaling of the (x,y) coordinates of the final pose data array (op::Datum::pose), i.e. the scale of the (x,y) coordinates that will be saved with the `write_pose` & `write_pose_json` flags. Select `0` to scale it to the original source resolution, `1` to scale it to the net output size (set with `net_resolution`), `2` to scale it to the final output size (set with `resolution`), `3` to scale it in the range [0,1], and 4 for range [-1,1]. Non related with `num_scales` and `scale_gap`.(int)
* **heatmaps_add_parts**: If true, it will add the body part heatmaps to the final op::Datum::poseHeatMaps array (program speed will decrease). Not required for our library, enable it only if you intend to process this information later. If more than one `add_heatmaps_X` flag is enabled, it will place then in sequential memory order: body parts + bkg + PAFs. It will follow the order on POSE_BODY_PART_MAPPING in `include/openpose/pose/poseParameters.hpp`.(bool)
* **heatmaps_add_bkg**: Same functionality as `add_heatmaps_parts`, but adding the heatmap corresponding to background. (bool)

* **heatmaps_add_PAFs**: Same functionality as `add_heatmaps_parts`, but adding the PAFs.(bool)
* **heatmaps_scale_mode**: Set 0 to scale op::Datum::poseHeatMaps in the range [0,1], 1 for [-1,1]; and 2 for integer rounded [0,255].(int)"
* **render_pose**: Set to 0 for no rendering, 1 for CPU rendering (slightly faster), and 2 for GPU rendering(int)
* **part_to_show**: Part to show from the start.(int)
* **disable_blending**: If false, it will blend the results with the original frame. If true, it will only display the results.(bool)
* **alpha_pose**: Blending factor (range 0-1) for the body part rendering. 1 will show it completely, 0 will hide it.(double)
* **alpha_heatmap**: Blending factor (range 0-1) between heatmap and original frame. 1 will only show the heatmap, 0 will only show the frame.(double)
* **render_threshold**: Only estimated keypoints whose score confidences are higher than this threshold will be rendered. Generally, a high threshold (> 0.5) will only render very clear body parts.(double)
* **body_enable**: Disable body keypoint detection. Option only possible for faster (but less accurate) face. (bool)
