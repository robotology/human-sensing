/*
 * Copyright (C) 2018 iCub Facility - Istituto Italiano di Tecnologia
 * Author: Ilaria Carlini Laura Cavaliere Vadim Tikhanoff 
 * email:  ilaria.carlini@iit.it laura.cavaliere@iit.it vadim.tikhanoff@iit.it 
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

#include <vector>
#include <iostream>
#include <deque>
#include <cstdio>
#include <cmath>
#include <fstream>
#include <iterator>
#include <string>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/ResourceFinder.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/Network.h>
#include <yarp/os/Time.h>
#include <yarp/os/Log.h>
#include <yarp/os/LogStream.h>
#include <yarp/os/Semaphore.h>
#include <yarp/sig/SoundFile.h>
#include <yarp/sig/Image.h>
#include <yarp/dev/PolyDriver.h>
#include <regex>
#include <yarp/cv/Cv.h>

#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>


#include <grpc++/grpc++.h>

#include "google/cloud/vision/v1/image_annotator.grpc.pb.h"
#include "google/cloud/vision/v1/geometry.grpc.pb.h"
#include "google/cloud/vision/v1/web_detection.grpc.pb.h"
#include "google/cloud/vision/v1/text_annotation.grpc.pb.h"
#include "google/cloud/vision/v1/product_search_service.grpc.pb.h"
#include "google/cloud/vision/v1/product_search.grpc.pb.h"


static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";



#include "googleVisionAI_IDL.h"

using namespace google::cloud::vision::v1;



using namespace std;

string SCOPE = "vision.googleapis.com";


/********************************************************/
class Processing : public yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelRgb> >
{
    std::string moduleName;  
    
    yarp::os::RpcServer handlerPort;
    yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelRgb> > outPort;

    yarp::os::BufferedPort<yarp::os::Bottle> targetPort;

    yarp::sig::ImageOf<yarp::sig::PixelRgb> annotate_img;

    std::mutex mtx;

public:
    /********************************************************/

    Processing( const std::string &moduleName )
    {
        this->moduleName = moduleName;

    }

    /********************************************************/
    ~Processing()
    {

    };

    /********************************************************/
    bool open()
    {
        this->useCallback();
        BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelRgb> >::open( "/" + moduleName + "/image:i" );
        outPort.open("/" + moduleName + "/image:o");
        targetPort.open("/"+ moduleName + "/result:o");
        yarp::os::Network::connect(outPort.getName().c_str(), "/view");
        yarp::os::Network::connect("/icub/camcalib/left/out", BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelRgb> >::getName().c_str());

        return true;
    }

    /********************************************************/
    void close()
    {
        BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelRgb> >::close();
        outPort.close();
        targetPort.close();
    }

    /********************************************************/
    void interrupt()
    {
        BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelRgb> >::interrupt();
    }
    

    /********************************************************/
    void onRead( yarp::sig::ImageOf<yarp::sig::PixelRgb> &img )
    {   

        yarp::sig::ImageOf<yarp::sig::PixelRgb> &outImage  = outPort.prepare();
        yarp::os::Bottle &outTargets = targetPort.prepare();

        //outImage.resize(img.width(), img.height());
        outImage.resize(640, 480);
        
        outImage = img;

        std::lock_guard<std::mutex> lg(mtx);
        annotate_img = img;

        outPort.write();

        //outTargets.clear();
        //outTargets = queryGoogleVisionAI(img);
        //yDebug() << "img" << outTargets.toString();
        targetPort.write();

        //yDebug() << "done querying google";
    }



 static inline bool is_base64( unsigned char c ) 
{ 
	return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64_encode(uchar const* bytes_to_encode, unsigned int in_len) 
    {
        std::string ret;

        int i = 0;
        int j = 0;
        unsigned char char_array_3[3];
        unsigned char char_array_4[4];

        while (in_len--) 
        {
            char_array_3[i++] = *(bytes_to_encode++);
            if (i == 3)
            {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;

                for (i = 0; (i <4); i++) 
                {
                    ret += base64_chars[char_array_4[i]];
                }
                i = 0;
            }
        }

        if (i) 
        {
            for (j = i; j < 3; j++) 
            {
                char_array_3[j] = '\0';
            }

            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (j = 0; (j < i + 1); j++) 
            {
                ret += base64_chars[char_array_4[j]];
            }
            
            while ((i++ < 3)) 
            {
                ret += '=';
            }
        }

        return ret;
    }

/********************************************************/
    yarp::os::Bottle queryGoogleVisionAI( yarp::sig::ImageOf<yarp::sig::PixelRgb> &img )
    {
        BatchAnnotateImagesRequest requests; // Consists of multiple AnnotateImage requests // 
        BatchAnnotateImagesResponse responses;
        AnnotateImageResponse response;

        cv::Mat input_cv = yarp::cv::toCvMat(img);

        //Grayscale matrix
        cv::Mat grayscaleMat (input_cv.size(), CV_8U);

        //Binary image
        cv::Mat binaryMat (grayscaleMat.size(), grayscaleMat.type());



        //cv::Mat dst;
        //cv::resize(input_cv, dst, cv::Size(640,480), 0, 0, cv::INTER_CUBIC);
        cv::imwrite("test.jpg", binaryMat);

        //cv::Size size = input_cv.size();

        //int total = size.width * size.height * input_cv.channels();

        //std::vector<uchar> data(input_cv.ptr(), input_cv.ptr() + total);
        //std::string s(data.begin(), data.end());                   
        

        //std::vector<uchar> buffer;
        //buffer.resize(static_cast<size_t>(input_cv.rows) * static_cast<size_t>(input_cv.cols));
        //cv::imencode(".jpg", input_cv, buffer);
        //std::string encoded = base64_encode(buffer.data(), buffer.size());

        //std::vector<uchar> buf;
        //cv::imencode(".jpg", input_cv, buf);
        //auto *enc_msg = reinterpret_cast<unsigned char*>(buf.data());
        //std::string encoded = base64_encode(enc_msg, buf.size());   

        //std::string encoded = base64_encode(input_cv.data, size.width * size.height);

        ////////


        int params[3] = {0};
        params[0] = CV_IMWRITE_JPEG_QUALITY;
        params[1] = 100;
        std::vector<uchar> buf;
	    bool code = cv::imencode(".jpg", binaryMat, buf, std::vector<int>(params, params+2));
	    uchar* result = reinterpret_cast<uchar*> (&buf[0]);


        //std::string encoded = base64_encode(result, buf.size());

        //std::cout << encoded <<std::endl;

	 
        //----------------------//
        // Set up Configuration //
        //----------------------//

        // Image Source //
        requests.add_requests();

        std::string encoded = "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";
        requests.mutable_requests( 0 )->mutable_image()->set_content( encoded ); // base64 of local image
        //requests.mutable_requests( 0 )->mutable_image()->mutable_source()->set_image_uri( "https://i.ibb.co/Ws3SCs8/test.jpg" ); // TODO [GCS_URL] // 
        //requests.mutable_requests( 0 )->mutable_image()->mutable_source()->set_gcs_image_uri( "gs://personal_projects/photo_korea.jpg" ); // TODO [GCS_URL] // 
        //requests.mutable_requests( 0 )->mutable_image_context(); // optional?? 


        // Features // 
        requests.mutable_requests( 0 )->add_features();
        requests.mutable_requests( 0 )->add_features();
        requests.mutable_requests( 0 )->add_features();
        requests.mutable_requests( 0 )->add_features();
        requests.mutable_requests( 0 )->add_features();
        requests.mutable_requests( 0 )->add_features();
        requests.mutable_requests( 0 )->add_features();
        requests.mutable_requests( 0 )->add_features();
        requests.mutable_requests( 0 )->add_features();

        requests.mutable_requests( 0 )->mutable_features( 0 )->set_type( Feature_Type_FACE_DETECTION );
        requests.mutable_requests( 0 )->mutable_features( 1 )->set_type( Feature_Type_LANDMARK_DETECTION );
        requests.mutable_requests( 0 )->mutable_features( 2 )->set_type( Feature_Type_LOGO_DETECTION );
        requests.mutable_requests( 0 )->mutable_features( 3 )->set_type( Feature_Type_LABEL_DETECTION );
        requests.mutable_requests( 0 )->mutable_features( 4 )->set_type( Feature_Type_TEXT_DETECTION );
        requests.mutable_requests( 0 )->mutable_features( 5 )->set_type( Feature_Type_SAFE_SEARCH_DETECTION );
        requests.mutable_requests( 0 )->mutable_features( 6 )->set_type( Feature_Type_IMAGE_PROPERTIES );
        requests.mutable_requests( 0 )->mutable_features( 7 )->set_type( Feature_Type_CROP_HINTS );
        requests.mutable_requests( 0 )->mutable_features( 8 )->set_type( Feature_Type_WEB_DETECTION );

        //requests.mutable_requests( 0 )->mutable_features( 0 )->set_max_results( int );


        // Print Configuration //
        std::cout << "\n\n---- Checking Request ----" << std::endl;
        std::cout << "Features size: " << requests.mutable_requests( 0 )->features_size() << std::endl;
        for ( int i = 0; i < requests.mutable_requests( 0 )->features_size(); i++ ) {
            //requests.mutable_requests( 0 ).features( int ); // Feature
            //requests.mutable_requests( 0 ).features( i ).type(); // Feature_Type
            std::cout << "Feature " << i << " name: " << Feature_Type_Name( requests.mutable_requests( 0 )->features( i ).type() ) << std::endl;
            std::cout << "max results: " << requests.mutable_requests( 0 )->features( i ).max_results() << std::endl;

        }

        std::cout << "Image Source: ";
        requests.mutable_requests( 0 )->mutable_image()->has_source() ? std::cout << "OK" << std::endl  :  std::cout << "FALSE" << std::endl; 

        std::cout << "Request has Image: ";
        requests.mutable_requests( 0 )->has_image() ? std::cout << "OK" << std::endl  :  std::cout << "FALSE" << std::endl;

        std::cout << "BatchRequests size: " << requests.requests_size() << std::endl;



        //--------------//
        // Send Request //
        //--------------//
        std::cout << "\n---- Sending Request ----" << std::endl;
        grpc::Status status;
        grpc::ClientContext context;

        std::cout << "Getting GoogleDefaultCredentials...";
        auto creds = grpc::GoogleDefaultCredentials();
        std::cout << "DONE!\nCreating Channel...";
        auto channel = ::grpc::CreateChannel( SCOPE, creds );
        std::cout << "DONE!\nGetting Status...";
        std::unique_ptr< ImageAnnotator::Stub> stub( ImageAnnotator::NewStub( channel ) );
        status = stub->BatchAnnotateImages( &context, requests, &responses );
        std::cout << "DONE!" << std::endl;



        //---------------//
        // Read Response //
        //---------------//
        std::cout << "\n\n------ Responses ------" << std::endl;
        if ( status.ok() ) {
            std::cout << "Status returned OK\nResponses size: " << responses.responses_size() << std::endl;
            for ( int h = 0; h < responses.responses_size(); h++ ) {
                response = responses.responses( h );

                // Response Error //
                std::cout << "Response has Error: " << response.has_error() << std::endl;
                if ( response.has_error() ) {
                    std::cout
                        << "Error Code: "
                        << response.error().code()
                        << "\nError Message: "
                        << response.error().message()
                        << std::endl;

                    std::cout << "Do you wish to continue?[y/n]...";
                    string input;
                    std::cin >> input;
                    if ( std::regex_match( input, std::regex( "y(?:es)?|1" ) ) ) {
                        std::cout << "Continuing... " << std::endl;
                    } else {
                        std::cout << "Breaking... " << std::endl;
                        break;
                    
                    }
                
                }

                std::cout << "\n\n----Face Annotations----" << std::endl;
                std::cout << "Size: " << response.face_annotations_size() << std::endl;
                for ( int i = 0; i <  response.face_annotations_size(); i++ ) {

                    if ( response.face_annotations( i ).has_bounding_poly() ) {
                        //response.face_annotation( i ).bounding_poly();
                        std::cout << "Has Bounding Poly: "
                            << response.face_annotations( i ).has_bounding_poly()
                            << std::endl;

                        for ( int j = 0; j < response.face_annotations( i ).bounding_poly().vertices_size(); j++ ) {
                            std::cout
                                << "\tvert: "
                                << response.face_annotations( i ).bounding_poly().vertices( j ).x() // vertex
                                << ","
                                << response.face_annotations( i ).bounding_poly().vertices( j ).y() // vertex
                                << std::endl;
                        }
                    }

                    if ( response.face_annotations( i ).has_fd_bounding_poly() ) {
                        //response.face_annotations( 0 ).fd_bounding_poly();
                        for ( int j = 0; j < response.face_annotations( i ).fd_bounding_poly().vertices_size(); j++ ) {
                            std::cout
                                << "FD Bounding Poly: "
                                << response.face_annotations( i ).fd_bounding_poly().vertices( j ).x() // vertex
                                << ","
                                << response.face_annotations( i ).fd_bounding_poly().vertices( j ).y() // vertex
                                << std::endl;
                        }
                    }

                    // TODO [FA_LANDMARK] //
                    for ( int j = 0; j < response.face_annotations( i ).landmarks_size(); j++ ) {
                        //response.face_annotations( i ).landmarks( j ). // FaceAnnotation_Landmarks
                        std::cout << "FaceAnnotationLandmark: "
                            << "\n\tType: "
                            << FaceAnnotation_Landmark_Type_Name( response.face_annotations( i ).landmarks( j ).type() )
                            << "\n\tValue: "
                            << response.face_annotations( i ).landmarks( j ).type()
                            << "\n\ti: " << i << "  j: " << j;
                        
                        if ( response.face_annotations( i ).landmarks( j ).has_position() ) {
                            std::cout
                            << "X: "
                            << response.face_annotations( i ).landmarks( j ).position().x()
                            << "  Y: "
                            << response.face_annotations( i ).landmarks( j ).position().y()
                            << "  Z: "
                            << response.face_annotations( i ).landmarks( j ).position().z();
                        
                        } else {
                            std::cout << "\n\tNO POSITION";
                        }
                        std::cout
                        << std::endl;
                    
                    }
                    
                    std::cout << "roll angle: "
                        << response.face_annotations( i ).roll_angle() // float
                        << "\npan angle: "
                        << response.face_annotations( i ).pan_angle() // float
                        << "\ntilt angle: "
                        << response.face_annotations( i ).tilt_angle() // float
                        << "\ndetection confidence: "
                        << response.face_annotations( i ).detection_confidence() // float
                        << "\nlandmarking confidence: "
                        << response.face_annotations( i ).landmarking_confidence() // float
                        << std::endl;

                    std::cout << "Alt Info:"
                        << "\n\tJoy: "
                        << Likelihood_Name( response.face_annotations( i ).joy_likelihood() )
                        << "\n\tSorrow: "
                        << Likelihood_Name( response.face_annotations( i ).sorrow_likelihood() )
                        << "\n\tAnger: "
                        << Likelihood_Name( response.face_annotations( i ).anger_likelihood() )
                        << "\n\tSurprise: "
                        << Likelihood_Name( response.face_annotations( i ).surprise_likelihood() )
                        << "\n\tUnder Exposed: "
                        << Likelihood_Name( response.face_annotations( i ).under_exposed_likelihood() )
                        << "\n\tBlured: "
                        << Likelihood_Name( response.face_annotations( i ).blurred_likelihood() )
                        << "\n\tHeadwear: "
                        << Likelihood_Name( response.face_annotations( i ).headwear_likelihood() )
                        << std::endl;



                }


                std::cout << "\n\n----Landmark Annotations----" << std::endl;
                std::cout << "Size: " << response.landmark_annotations_size() << std::endl;

                for ( int i = 0; i < response.landmark_annotations_size(); i++ ) {
                    //response.landmark_annotations( i ); // EntityAnnotation
                    // 4977
                    response.landmark_annotations( i ).mid(); // string
                    response.landmark_annotations( i ).locale(); //string
                    response.landmark_annotations( i ).description(); // string
                    response.landmark_annotations( i ).score(); // float 
                    response.landmark_annotations( i ).confidence(); // float
                    response.landmark_annotations( i ).topicality(); // float

                    /*
                    response.landmark_annotations( int )// ---- BoundingPoly ---- //
                    response.landmark_annotations( int ).has_bounding_poly(); //bool
                    response.landmark_annotations( int ).bounding_poly(); // BoundingPoly
                    response.landmark_annotations( int ).bounding_poly().verticies_size(); // int
                    response.landmark_annotations( int ).bounding_poly().verticies( int ); // Vertex
                    response.landmark_annotations( int ).bounding_poly().verticies( int ).x(); // google::protobuf::int32
                    response.landmark_annotations( int ).bounding_poly().verticies( int ).y(); // google::protobuf::int32
                    response.landmark_annotations( int )// ---- LocationInfo ---- //
                    response.landmark_annotations( int ).locations_size(); // int
                    response.landmark_annotations( int ).locations( int ); // LocationInfo
                    response.landmark_annotations( int ).locations( int ).has_lat_lng(); // bool
                    response.landmark_annotations( int ).locations( int ).lat_lng(); // LatLng
                    response.landmark_annotations( int ).locations( int ).lat_lng().latitude(); // double
                    response.landmark_annotations( int ).locations( int ).lat_lng().longtude(); // double
                    response.landmark_annotations( int )// ---- Propert ---- //
                    response.landmark_annotations( int ).properties_size(); // int
                    response.landmark_annotations( int ).properties( int ); // Property
                    response.landmark_annotations( int ).properties( int ).name(); //string
                    response.landmark_annotations( int ).properties( int ).value(); // string
                    response.landmark_annotations( int ).properties( int ).unit64_value(); // google::protobuf
                    */
                    //
                    std::cout
                        << "Label "
                        << i 
                        << "\n\tmid: "
                        << response.landmark_annotations( i ).mid() // string
                        << "\n\tlocale: "
                        << response.landmark_annotations( i ).locale() // string
                        << "\n\tdescription: "
                        << response.landmark_annotations( i ).description() // string
                        << "\n\tscore: "
                        << response.landmark_annotations( i ).score() // float
                        << "\n\tconfidence: "
                        << response.landmark_annotations( i ).confidence() // float
                        << "\n\ttopicality: "
                        << response.landmark_annotations( i ).topicality() // float
                        << std::endl;

                    std::cout
                        << "\tHas Bounding Poly: "
                        << response.landmark_annotations(i ).has_bounding_poly()
                        << std::endl;

                    if ( response.landmark_annotations( i ).has_bounding_poly() ) {
                        std::cout
                            << "\t\tVerticies: "
                            << response.landmark_annotations( i ).bounding_poly().vertices_size()
                            << std::endl;

                        for ( int j = 0; j < response.landmark_annotations( i ).bounding_poly().vertices_size(); j++ ) {
                            std::cout
                                << "\t\tvert " << j << ": "
                                << response.landmark_annotations( i ).bounding_poly().vertices( j ).x()
                                << ", "
                                << response.landmark_annotations( i ).bounding_poly().vertices( j ).y()
                                << std::endl;
                        }
                    }

                    std::cout
                        << "\tLocations: "
                        << response.landmark_annotations( i ).locations_size()
                        << std::endl;

                    for ( int j = 0; j < response.landmark_annotations( i ).locations_size(); j++ ) {
                        if ( response.landmark_annotations( i ).locations( j ).has_lat_lng() ) {
                            std::cout
                                << j
                                << ": "
                                << response.landmark_annotations( i ).locations( j ).lat_lng().latitude() // double
                                << " , "
                                << response.landmark_annotations( i ).locations( j ).lat_lng().longitude() // double
                                << std::endl;
                        
                        }
                    }

                    std::cout
                        << "\tProperties: "
                        << response.landmark_annotations( i ).properties_size()
                        << std::endl;

                    for ( int j = 0; j < response.landmark_annotations( i ).properties_size(); j++ ) {
                        std::cout
                            << "\tname: "
                            << response.landmark_annotations( i ).properties( j ).name() // string
                            << "\t\nvalue: "
                            << response.landmark_annotations( i ).properties( j ).value() // string
                            << "\t\nuint64 value: "
                            << response.landmark_annotations( i ).properties( j ).uint64_value() // uint64
                            << std::endl;
                    
                    }

                }

                std::cout << "\n\n----Logo Annotations----" << std::endl;
                std::cout << "Size: " << response.logo_annotations_size() << std::endl;
                for ( int i = 0; i <  response.logo_annotations_size(); i++ ) {
                    //response.logo_annotations( i ); // EntityAnnotation
                    std::cout
                        << "Label "
                        << i 
                        << "\n\tmid: "
                        << response.logo_annotations( i ).mid() // string
                        << "\n\tlocale: "
                        << response.logo_annotations( i ).locale() // string
                        << "\n\tdescription: "
                        << response.logo_annotations( i ).description() // string
                        << "\n\tscore: "
                        << response.logo_annotations( i ).score() // float
                        << "\n\tconfidence: "
                        << response.logo_annotations( i ).confidence() // float
                        << "\n\ttopicality: "
                        << response.logo_annotations( i ).topicality() // float
                        << std::endl;

                    std::cout
                        << "\tHas Bounding Poly: "
                        << response.logo_annotations(i ).has_bounding_poly()
                        << std::endl;

                    if ( response.logo_annotations( i ).has_bounding_poly() ) {
                        std::cout
                            << "\tVerticies: "
                            << response.logo_annotations( i ).bounding_poly().vertices_size()
                            << std::endl;

                        for ( int j = 0; j < response.logo_annotations( i ).bounding_poly().vertices_size(); j++ ) {
                            std::cout
                                << "\t\tvert " << j << ": "
                                << response.logo_annotations( i ).bounding_poly().vertices( j ).x()
                                << ", "
                                << response.logo_annotations( i ).bounding_poly().vertices( j ).y()
                                << std::endl;
                        }
                    }

                    std::cout
                        << "\tLocations: "
                        << response.logo_annotations( i ).locations_size()
                        << std::endl;

                    for ( int j = 0; j < response.logo_annotations( i ).locations_size(); j++ ) {
                        if ( response.logo_annotations( i ).locations( j ).has_lat_lng() ) {
                            std::cout
                                << j
                                << ": "
                                << response.logo_annotations( i ).locations( j ).lat_lng().latitude() // double
                                << " , "
                                << response.logo_annotations( i ).locations( j ).lat_lng().longitude() // double
                                << std::endl;
                        
                        }
                    }

                    std::cout
                        << "\tProperties: "
                        << response.logo_annotations( i ).properties_size()
                        << std::endl;

                    for ( int j = 0; j < response.logo_annotations( i ).properties_size(); j++ ) {
                        std::cout
                            << "\tname: "
                            << response.logo_annotations( i ).properties( j ).name() // string
                            << "\t\nvalue: "
                            << response.logo_annotations( i ).properties( j ).value() // string
                            << "\t\nuint64 value: "
                            << response.logo_annotations( i ).properties( j ).uint64_value() // uint64
                            << std::endl;
                    
                    }


                }

                std::cout << "\n\n----Label Annotations----" << std::endl;
                std::cout << "Size: " << response.label_annotations_size() << std::endl;
                for ( int i = 0; i < response.label_annotations_size(); i++ ) {
                    //response.label_annotations( i ); // EntityAnnotation
                    std::cout
                        << "Label "
                        << i 
                        << "\n\tmid: "
                        << response.label_annotations( i ).mid() // string
                        << "\n\tlocale: "
                        << response.label_annotations( i ).locale() // string
                        << "\n\tdescription: "
                        << response.label_annotations( i ).description() // string
                        << "\n\tscore: "
                        << response.label_annotations( i ).score() // float
                        << "\n\tconfidence: "
                        << response.label_annotations( i ).confidence() // float
                        << "\n\ttopicality: "
                        << response.label_annotations( i ).topicality() // float
                        << std::endl;

                    std::cout
                        << "\tHas Bounding Poly: "
                        << response.label_annotations(i ).has_bounding_poly()
                        << std::endl;

                    if ( response.label_annotations( i ).has_bounding_poly() ) {
                        std::cout
                            << "\tVerticies: "
                            << response.label_annotations( i ).bounding_poly().vertices_size()
                            << std::endl;

                        for ( int j = 0; j < response.label_annotations( i ).bounding_poly().vertices_size(); j++ ) {
                            std::cout
                                << "\t\tvert " << j << ": "
                                << response.label_annotations( i ).bounding_poly().vertices( j ).x()
                                << ", "
                                << response.label_annotations( i ).bounding_poly().vertices( j ).y()
                                << std::endl;
                        }
                    }

                    std::cout
                        << "\tLocations: "
                        << response.label_annotations( i ).locations_size()
                        << std::endl;

                    for ( int j = 0; j < response.label_annotations( i ).locations_size(); j++ ) {
                        if ( response.label_annotations( i ).locations( j ).has_lat_lng() ) {
                            std::cout
                                << j
                                << ": "
                                << response.label_annotations( i ).locations( j ).lat_lng().latitude() // double
                                << " , "
                                << response.label_annotations( i ).locations( j ).lat_lng().longitude() // double
                                << std::endl;
                        
                        }
                    }

                    std::cout
                        << "\tProperties: "
                        << response.label_annotations( i ).properties_size()
                        << std::endl;

                    for ( int j = 0; j < response.label_annotations( i ).properties_size(); j++ ) {
                        std::cout
                            << "\tname: "
                            << response.label_annotations( i ).properties( j ).name() // string
                            << "\t\nvalue: "
                            << response.label_annotations( i ).properties( j ).value() // string
                            << "\t\nuint64 value: "
                            << response.label_annotations( i ).properties( j ).uint64_value() // uint64
                            << std::endl;
                    
                    }


                }

                std::cout << "\n\n----Text Annotations----" << std::endl;
                std::cout << "Size: " << response.text_annotations_size() << std::endl;
                for ( int i = 0; i < response.text_annotations_size(); i++ ) {
                    //response.text_annotations( i ); // EntityAnnotation
                    std::cout
                        << "Label "
                        << i 
                        << "\n\tmid: "
                        << response.text_annotations( i ).mid() // string
                        << "\n\tlocale: "
                        << response.text_annotations( i ).locale() // string
                        << "\n\tdescription: "
                        << response.text_annotations( i ).description() // string
                        << "\n\tscore: "
                        << response.text_annotations( i ).score() // float
                        << "\n\tconfidence: "
                        << response.text_annotations( i ).confidence() // float
                        << "\n\ttopicality: "
                        << response.text_annotations( i ).topicality() // float
                        << std::endl;

                    std::cout
                        << "\tHas Bounding Poly: "
                        << response.text_annotations(i ).has_bounding_poly()
                        << std::endl;

                    if ( response.text_annotations( i ).has_bounding_poly() ) {
                        std::cout
                            << "\tVerticies: "
                            << response.text_annotations( i ).bounding_poly().vertices_size()
                            << std::endl;

                        for ( int j = 0; j < response.text_annotations( i ).bounding_poly().vertices_size(); j++ ) {
                            std::cout
                                << "\t\tvert " << j << ": "
                                << response.text_annotations( i ).bounding_poly().vertices( j ).x()
                                << ", "
                                << response.text_annotations( i ).bounding_poly().vertices( j ).y()
                                << std::endl;
                        }
                    }

                    std::cout
                        << "\tLocations: "
                        << response.text_annotations( i ).locations_size()
                        << std::endl;

                    for ( int j = 0; j < response.text_annotations( i ).locations_size(); j++ ) {
                        if ( response.text_annotations( i ).locations( j ).has_lat_lng() ) {
                            std::cout
                                << j
                                << ": "
                                << response.text_annotations( i ).locations( j ).lat_lng().latitude() // double
                                << " , "
                                << response.text_annotations( i ).locations( j ).lat_lng().longitude() // double
                                << std::endl;
                        
                        }
                    }

                    std::cout
                        << "\tProperties: "
                        << response.text_annotations( i ).properties_size()
                        << std::endl;

                    for ( int j = 0; j < response.text_annotations( i ).properties_size(); j++ ) {
                        std::cout
                            << "\tname: "
                            << response.text_annotations( i ).properties( j ).name() // string
                            << "\t\nvalue: "
                            << response.text_annotations( i ).properties( j ).value() // string
                            << "\t\nuint64 value: "
                            << response.text_annotations( i ).properties( j ).uint64_value() // uint64
                            << std::endl;
                    
                    }

                
                }

                std::cout << "\n\n----Full Text Annotation----" << std::endl;
                if ( response.has_full_text_annotation() ) {
                    std::cout << response.has_full_text_annotation() << std::endl;
                    response.full_text_annotation(); // TextAnnotation
                
                } else { std::cout << "NONE" << std::endl; }


                std::cout << "\n\n----Safe Search Annotation----" << std::endl;
                if ( response.has_safe_search_annotation() ) {
                    response.safe_search_annotation(); // SafeSearchAnnotation 

                } else { std::cout << "NONE" << std::endl; }


                std::cout << "\n\n----Image Properties Annotations----" << std::endl;
                if ( response.has_image_properties_annotation()  ) {
                    std::cout << response.has_image_properties_annotation() << std::endl; // ImageProperties
                    std::cout << "Dominant Colors: " << response.image_properties_annotation().has_dominant_colors() << std::endl;
                    std::cout << "\tSize: " << response.image_properties_annotation().dominant_colors().colors_size() << std::endl;

                    for ( int i = 0; i <  response.image_properties_annotation().dominant_colors().colors_size(); i++ ) {
                        std::cout << "Has Color: " << response.image_properties_annotation().dominant_colors().colors( i ).has_color() << std::endl;
                        if ( response.image_properties_annotation().dominant_colors().colors( i ).has_color() ) {
                            std::cout
                                << "\trgb: "
                                << response.image_properties_annotation().dominant_colors().colors( i ).color().red() // float
                                << ","
                                << response.image_properties_annotation().dominant_colors().colors( i ).color().green() // float
                                << ","
                                << response.image_properties_annotation().dominant_colors().colors( i ).color().blue() // float
                                << std::endl;

                                if ( response.image_properties_annotation().dominant_colors().colors( i ).color().has_alpha() ) {
                                    std::cout
                                        << "\talpha: "
                                        << response.image_properties_annotation().dominant_colors().colors( i ).color().alpha().value() // float
                                        << std::endl;
                                }
                        }

                    }

                } else { std::cout << "NONE" << std::endl; }


                std::cout << "\n\n----Crop Hints Annotations----" << std::endl;

                if ( response.has_crop_hints_annotation() ) {
                    std::cout << "Size: " << response.crop_hints_annotation().crop_hints_size() << std::endl; 
                    for ( int i = 0; i < response.crop_hints_annotation().crop_hints_size(); i++ ) {
                        std::cout << "BoundingPoly: " << response.crop_hints_annotation().crop_hints( i ).has_bounding_poly() << std::endl;
                        std::cout << "\tVertSize: " << response.crop_hints_annotation().crop_hints( i ).bounding_poly().vertices_size() << std::endl;
                        for ( int j = 0; j < response.crop_hints_annotation().crop_hints( i ).bounding_poly().vertices_size(); j++ ) {
                            std::cout << "\tVert: X: " << response.crop_hints_annotation().crop_hints( i ).bounding_poly().vertices( j ).x() << " Y: " << response.crop_hints_annotation().crop_hints( i ).bounding_poly().vertices( j ).y() << std::endl;
                        }
                    }
                } else { std::cout << "NONE" << std::endl; }


                std::cout << "\n\n----Web Detection----" << std::endl;
                if ( response.has_web_detection()  ) {
                    
                    std::cout << "WebEntities: " << response.web_detection().web_entities_size() << std::endl;
                    for ( int i = 0; i < response.web_detection().web_entities_size(); i++ ) {
                        std::cout
                            << "\tid: "
                            << response.web_detection().web_entities( i ).entity_id() // string
                            << "\n\tscore: "
                            << response.web_detection().web_entities( i ).score() // float
                            << "\n\tdescription: "
                            << response.web_detection().web_entities( i ).description() // string
                            << std::endl;
                    }

                    std::cout << "Full Matching Images: " << response.web_detection().full_matching_images_size() << std::endl;
                    for ( int i = 0; i < response.web_detection().full_matching_images_size(); i++ ) {
                        std::cout << "\tScore: "
                            << response.web_detection().full_matching_images( i ).score() // float
                            << "    URL: "
                            << response.web_detection().full_matching_images( i ).url() // string
                            << std::endl;
                    }
                    
                    std::cout << "Partially Matching Images: " << response.web_detection().partial_matching_images_size() << std::endl;
                    for ( int i = 0; i < response.web_detection().partial_matching_images_size(); i++ ) {
                        std::cout << "\tScore: "
                            << response.web_detection().partial_matching_images( i ).score() // float
                            << "    URL: "
                            << response.web_detection().partial_matching_images( i ).url() // string
                            << std::endl;
                    }

                    std::cout << "Pages With Matching Images: " << response.web_detection().pages_with_matching_images_size() << std::endl;
                    for ( int i = 0; i < response.web_detection().pages_with_matching_images_size(); i ++ ) {
                        std::cout << "\tScore: "
                            << response.web_detection().pages_with_matching_images( i ).score() // float
                            << "    URL: "
                            << response.web_detection().pages_with_matching_images( i ).url() // string
                            << std::endl;
                    }

                    
                
                
                } else { std::cout << "NONE" << std::endl; }

            }


        } else if (  status.ok() ){
            std::cout << "Status Returned Canceled" << std::endl;

            //sleep(0.5);
        }else {
            std::cerr << "RPC failed" << std::endl;
            //sleep(0.5);
            std::cout << status.error_code() << ": " << status.error_message() << status.ok() << std::endl;
        }

        google::protobuf::ShutdownProtobufLibrary();

        std::cout << "\nAll Finished!" << std::endl;

        
       /*

       yarp::os::Bottle result;
       result.clear();
       if ( dialog_status.ok() ) {

           yInfo() << "Status returned OK";
           yInfo() << "\n------Response------\n";

           result.addString(response.query_result().response_messages().Get(0).text().text().Get(0).c_str());
           yDebug() << "result bottle" << result.toString();

      } else if ( !dialog_status.ok() ) {
            yError() << "Status Returned Canceled";
      }
      request.release_query_input();
      query_input.release_text();
      
      return result;*/
   }

    /********************************************************/
    bool annotate()
    {
        std::lock_guard<std::mutex> lg(mtx);

        queryGoogleVisionAI(annotate_img);
        return true;
    }

    /********************************************************/
    bool stop_acquisition()
    {
        return true;
    }

};

/********************************************************/
class Module : public yarp::os::RFModule, public googleVisionAI_IDL
{
    yarp::os::ResourceFinder    *rf;
    yarp::os::RpcServer         rpcPort;

    Processing                  *processing;
    friend class                processing;

    bool                        closing;

    /********************************************************/
    bool attach(yarp::os::RpcServer &source)
    {
        return this->yarp().attachAsServer(source);
    }

public:

    /********************************************************/
    bool configure(yarp::os::ResourceFinder &rf)
    {
        this->rf=&rf;
        std::string moduleName = rf.check("name", yarp::os::Value("googleVisionAI"), "module name (string)").asString();
        std::string project_id = rf.check("project", yarp::os::Value("1"), "id of the project").asString();
        std::string language_code = rf.check("language", yarp::os::Value("en-US"), "language of the dialogflow").asString();
        
        yDebug() << "Module name" << moduleName;
        setName(moduleName.c_str());

        rpcPort.open(("/"+getName("/rpc")).c_str());

        closing = false;

        processing = new Processing( moduleName );

        /* now start the thread to do the work */
        processing->open();

        attach(rpcPort);

        return true;
    }

    /**********************************************************/
    bool close()
    {
        processing->close();
        delete processing;
        return true;
    }

    /********************************************************/
    double getPeriod()
    {
        return 0.1;
    }

    /********************************************************/
    bool quit()
    {
        closing=true;
        return true;
    }

    /********************************************************/
    bool annotate()
    {
        processing->annotate();
        return true;
    }

    /********************************************************/
    bool updateModule()
    {
        return !closing;
    }
};

/********************************************************/
int main(int argc, char *argv[])
{
    yarp::os::Network::init();

    yarp::os::Network yarp;
    if (!yarp.checkNetwork())
    {
        yError("YARP server not available!");
        return 1;
    }

    Module module;
    yarp::os::ResourceFinder rf;

    rf.setVerbose( true );
    rf.setDefaultContext( "googleVisionAI" );
    rf.setDefaultConfigFile( "config.ini" );
    rf.setDefault("name","googleVisionAI");
    rf.configure(argc,argv);

    return module.runModule(rf);
}
