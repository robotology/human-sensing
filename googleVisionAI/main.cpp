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
    yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelRgb> > inPort;

    yarp::os::BufferedPort<yarp::os::Bottle> targetPort;

    yarp::sig::ImageOf<yarp::sig::PixelRgb> annotate_img;

    std::mutex mtx;

    yarp::os::Bottle result_btl;

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
        yarp::os::Network::connect("/icub/camcalib/left/out", "/startImage" ); 
        yarp::os::Network::connect("/icub/camcalib/left/out", BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelRgb> >::getName().c_str());
        yarp::os::Network::connect("/googleVisionAI/image:o", "/outImage");  


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
       
        std::lock_guard<std::mutex> lg(mtx);
        annotate_img = img;
        //std::cout << "Inside the onRead" << std::endl;

    }

    /********************************************************/
     static inline bool is_base64( unsigned char c )
    {
        return (isalnum(c) || (c == '+') || (c == '/'));
    }

    /********************************************************/
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
    std::string base64_decode(std::string const& encoded_string)
    {
        int in_len = encoded_string.size();
        int i = 0;
        int j = 0;
        int in_ = 0;
        unsigned char char_array_4[4], char_array_3[3];
        std::string ret;

        while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_]))
        {
            char_array_4[i++] = encoded_string[in_]; in_++;

            if (i == 4)
            {
                for (i = 0; i < 4; i++)
                {
                    char_array_4[i] = base64_chars.find(char_array_4[i]);
                }

                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

                for (i = 0; (i < 3); i++)
                {
                    ret += char_array_3[i];
                }

                i = 0;
            }
        }

        if (i)
        {
            for (j = i; j < 4; j++)
            {
                char_array_4[j] = 0;
            }
            
            for (j = 0; j < 4; j++)
            {
                char_array_4[j] = base64_chars.find(char_array_4[j]);
            }

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (j = 0; (j < i - 1); j++)
            {
                ret += char_array_3[j];
            }
        }

        return ret;
    }

    /********************************************************/
    std::string adjustAccents(std::string initial) {

        std::map< std::string, std::string> dictionary;
        dictionary.insert ( std::pair<std::string,std::string>("á","a") );
        dictionary.insert ( std::pair<std::string,std::string>("à","a") );
        dictionary.insert ( std::pair<std::string,std::string>("é","e") );
        dictionary.insert ( std::pair<std::string,std::string>("è","e") );
        dictionary.insert ( std::pair<std::string,std::string>("í","i") );
        dictionary.insert ( std::pair<std::string,std::string>("ó","o") );
        dictionary.insert ( std::pair<std::string,std::string>("ú","u") );
        dictionary.insert ( std::pair<std::string,std::string>("ñ","n") );

        std::string tmp = initial;
        std::string strAux;
        for (auto it= dictionary.begin(); it != dictionary.end(); it++)
        {
            tmp=(it->first);
            std::size_t found=initial.find_first_of(tmp);

            while (found!=std::string::npos)
            {
                yError() << "in found" << found;
                strAux=(it->second);
                initial.erase(found,tmp.length());
                initial.insert(found,strAux);
                found=initial.find_first_of(tmp,found+1);
            }
        }
        return initial;
    }


/********************************************************/
    yarp::os::Bottle queryGoogleVisionAI( cv::Mat &input_cv)
    {
        BatchAnnotateImagesRequest requests; // Consists of multiple AnnotateImage requests // 
        BatchAnnotateImagesResponse responses;
        AnnotateImageResponse response;


        cv::imwrite("original.jpg", input_cv);


        // Encode data
        int params[3] = {0};
        params[0] = CV_IMWRITE_JPEG_QUALITY;
        params[1] = 100;
        std::vector<uchar> buf;
	    bool code = cv::imencode(".jpg", input_cv, buf, std::vector<int>(params, params+2));
	    uchar* result = reinterpret_cast<uchar*> (&buf[0]);
        /*
        std::string encoded = base64_encode(result, buf.size());
        
        
        // Decode data to verify consistency
        std::string decoded_string = base64_decode(encoded);
        std::vector<uchar> data(decoded_string.begin(), decoded_string.end());

        cv::Mat cv_out = imdecode(data, cv::IMREAD_UNCHANGED);
        
        cv::imwrite("decoded.jpg", cv_out);
         
        //std::cout << encoded <<std::endl; */

        //----------------------//
        // Set up Configuration //
        //----------------------//

        // Image Source //
        requests.add_requests();

        requests.mutable_requests( 0 )->mutable_image()->set_content(result, buf.size());



        //requests.mutable_requests( 0 )->mutable_image()->set_content( encoded ); // base64 of local image
        
        //requests.mutable_requests( 0 )->mutable_image()->mutable_source()->set_image_uri(encoded);
        //requests.mutable_requests( 0 )->mutable_image()->mutable_source()->set_image_uri( "https://media.istockphoto.com/photos/group-portrait-of-a-creative-business-team-standing-outdoors-three-picture-id1146473249?k=6&m=1146473249&s=612x612&w=0&h=W1xeAt6XW3evkprjdS4mKWWtmCVjYJnmp-LHvQstitU=" ); // TODO [GCS_URL] // 
        //requests.mutable_requests( 0 )->mutable_image()->mutable_source()->set_image_uri( "https://images.ctfassets.net/cnu0m8re1exe/1GxSYi0mQSp9xJ5svaWkVO/d151a93af61918c234c3049e0d6393e1/93347270_cat-1151519_1280.jpg?w=650&h=433&fit=fill" ); // TODO [GCS_URL] // 
        //requests.mutable_requests( 0 )->mutable_image()->mutable_source()->set_image_uri( "https://images.unsplash.com/photo-1578489758854-f134a358f08b?ixlib=rb-1.2.1&ixid=eyJhcHBfaWQiOjEyMDd9&w=1000&q=80" ); // TODO [GCS_URL] // 
        //requests.mutable_requests( 0 )->mutable_image()->mutable_source()->set_image_uri( "https://inchiostrovirtuale.it/wp-content/uploads/2019/05/Torre-Eiffel-copertina.jpg");
        //requests.mutable_requests( 0 )->mutable_image()->mutable_source()->set_image_uri( "https://gdsit.cdn-immedia.net/2018/02/giornale-di-sicilia-gazzetta-del-sud-625x350-1518680770.jpg");

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
        
        // Print Configuration //
        std::cout << "\n\n---- Checking Request ----" << std::endl;
        std::cout << "Features size: " << requests.mutable_requests( 0 )->features_size() << std::endl;
        for ( int i = 0; i < requests.mutable_requests( 0 )->features_size(); i++ ) {
            //requests.mutable_requests( 0 ).features( int ); // Features
            //std::cout << requests.mutable_requests( 0 )->features( i ).type() << std::endl; // Feature_Type
            std::cout << "Feature " << i << " name: " << Feature_Type_Name( requests.mutable_requests( 0 )->features( i ).type() ) << std::endl;
            std::cout << "max results: " << requests.mutable_requests( 0 )->features( i ).max_results() << std::endl;
        }

        std::cout << "Image Source: ";
        requests.mutable_requests( 0 )->mutable_image()->has_source() ? std::cout << "OK" << std::endl  :  std::cout << "FALSE" << std::endl; 

        std::cout << "Request has Image: ";
        requests.mutable_requests( 0 )->has_image() ? std::cout << "OK" << std::endl  :  std::cout << "FALSE" << std::endl;
        
        std::cout << "Request has Image Context: ";
        requests.mutable_requests( 0 )->has_image_context() ? std::cout << "OK" << std::endl  :  std::cout << "FALSE" << std::endl;

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
        yarp::os::Bottle b;
        b.clear();
        
        if ( status.ok() ) {


          
            b = get_result(responses, input_cv);

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
        
        requests.release_parent();
        response.release_error();
        response.release_context();
        response.release_web_detection();
        response.release_full_text_annotation();
        response.release_crop_hints_annotation();
        response.release_product_search_results();
        response.release_safe_search_annotation();
        response.release_image_properties_annotation();
        response.clear_face_annotations();
        response.clear_label_annotations();
        response.clear_landmark_annotations();
        response.clear_logo_annotations();
        

        std::cout << "\nAll Finished!" << std::endl;
       

     
        
        /*if ( dialog_status.ok() ) {

           yInfo() << "Status returned OK";
           yInfo() << "\n------Response------\n";

           result.addString(response.query_result().response_messages().Get(0).text().text().Get(0).c_str());
           yDebug() << "result bottle" << result.toString();

      } else if ( !dialog_status.ok() ) {
            yError() << "Status Returned Canceled";
      }
      request.release_query_input();
      query_input.release_text();*/
 
      return b;
   }

   /*********************************************************/

   yarp::os::Bottle get_result(BatchAnnotateImagesResponse responses, cv::Mat &input_cv)
   {    
        
        AnnotateImageResponse response;
        result_btl.clear();
        yarp::os::Bottle &ext_btl = result_btl.addList(); 

        for ( int h = 0; h < responses.responses_size() ; h++ ) {

            response = responses.responses( 0 );
		
            // FACE ANNOTATION : color = yellow
            if (response.face_annotations_size() > 0) {
                yarp::os::Bottle &face_annotation_btl = ext_btl.addList();
                face_annotation_btl.addString("face_annotation");

                for ( int i = 0; i <  response.face_annotations_size(); i++ ) {

                    yarp::os::Bottle &face_btl = face_annotation_btl.addList();
                    face_btl.addString("face");
                    face_btl.addInt(i+1);

                    if ( response.face_annotations( i ).has_bounding_poly() ) {
                        yarp::os::Bottle &bounding_poly_btl = face_btl.addList();
                        bounding_poly_btl.addString("bounding_poly");
                        cv::Point tl, br;
                        for ( int j = 0; j < response.face_annotations( i ).bounding_poly().vertices_size(); j++ ) {
                            yarp::os::Bottle &bounding_poly_xy = bounding_poly_btl.addList();
                            bounding_poly_xy.addDouble(response.face_annotations( i ).bounding_poly().vertices( j ).x());
                            bounding_poly_xy.addDouble(response.face_annotations( i ).bounding_poly().vertices( j ).y());
                            if (j==0) //for j=0 we have the top-left point
                               tl = cv::Point (response.face_annotations( i ).bounding_poly().vertices( j ).x(), response.face_annotations( i ).bounding_poly().vertices( j ).y());
                            else if (j==2) //for j=2 we have the bottom-right point
                               br = cv::Point (response.face_annotations( i ).bounding_poly().vertices( j ).x(), response.face_annotations( i ).bounding_poly().vertices( j ).y());
                        }
                        cv::rectangle(input_cv, tl, br, cvScalar(0, 255, 255), 1, 8); // cvScalar(B, G, R)
                        std::string face_label = "Face " + std::to_string(i+1);

                        cv::putText(input_cv, //target image
                                    face_label, //text
                                    cv::Point(tl.x + 5, tl.y + 8),
                                    cv::FONT_HERSHEY_SIMPLEX,
                                    0.3,
                                    CV_RGB(255, 255, 0), //font color yellow
                                    0.5);
                    }

                    if ( response.face_annotations( i ).has_fd_bounding_poly() ) {
                        yarp::os::Bottle &fd_bounding_poly_btl = face_btl.addList();
                        fd_bounding_poly_btl.addString("db_bounding_poly");
                        cv::Point db_tl, db_br;    
                        for ( int j = 0; j < response.face_annotations( i ).fd_bounding_poly().vertices_size(); j++ ) {
                            yarp::os::Bottle &fd_bounding_poly_xy = fd_bounding_poly_btl.addList();
                            fd_bounding_poly_xy.addDouble(response.face_annotations( i ).fd_bounding_poly().vertices( j ).x());
                            fd_bounding_poly_xy.addDouble(response.face_annotations( i ).fd_bounding_poly().vertices( j ).y());
                            if (j==0) //for j=0 we have the top-left point
                               db_tl = cv::Point (response.face_annotations( i ).fd_bounding_poly().vertices( j ).x(), response.face_annotations( i ).fd_bounding_poly().vertices( j ).y());
                            else if (j==2) //for j=2 we have the bottom-right point
                               db_br = cv::Point (response.face_annotations( i ).fd_bounding_poly().vertices( j ).x(), response.face_annotations( i ).fd_bounding_poly().vertices( j ).y());
                        }
                        cv::rectangle(input_cv, db_tl, db_br, cvScalar(0,255,255), 1, 8);
                    }
                    
                    for ( int j = 0; j < response.face_annotations( i ).landmarks_size(); j++ ) {   

                        yarp::os::Bottle &annotation_type_btl = face_btl.addList();   
                        annotation_type_btl.addString(FaceAnnotation_Landmark_Type_Name(response.face_annotations( i ).landmarks( j ).type()));

                    
                        if ( response.face_annotations( i ).landmarks( j ).has_position() ) {

                            yarp::os::Bottle &annotation_x = annotation_type_btl.addList();
                            annotation_x.addString("x");
                            annotation_x.addDouble(response.face_annotations( i ).landmarks( j ).position().x());

                            yarp::os::Bottle &annotation_y = annotation_type_btl.addList();
                            annotation_y.addString("y");
                            annotation_y.addDouble(response.face_annotations( i ).landmarks( j ).position().y());

                            yarp::os::Bottle &annotation_z = annotation_type_btl.addList();
                            annotation_z.addString("z");
                            annotation_z.addDouble(response.face_annotations( i ).landmarks( j ).position().z());
                            cv::circle(input_cv, cv::Point(response.face_annotations( i ).landmarks( j ).position().x(),
                                response.face_annotations( i ).landmarks( j ).position().y()), 1, cv::Scalar(0, 255, 255), -1, 8);

                        } else {

                            yarp::os::Bottle &annotation_x = annotation_type_btl.addList();
                            annotation_x.addString("x");
                            annotation_x.addString("");

                            yarp::os::Bottle &annotation_y = annotation_type_btl.addList();
                            annotation_y.addString("y");
                            annotation_y.addString("");

                            yarp::os::Bottle &annotation_z = annotation_type_btl.addList();
                            annotation_z.addString("z");
                            annotation_z.addString("");
                        }
                    }


                    yarp::os::Bottle &roll_angle_btl = face_btl.addList();
                    roll_angle_btl.addString("roll_angle");
                    roll_angle_btl.addFloat64(response.face_annotations( i ).roll_angle());

                    yarp::os::Bottle &pan_angle_btl = face_btl.addList();
                    pan_angle_btl.addString("pan_angle");
                    pan_angle_btl.addFloat64(response.face_annotations( i ).pan_angle());

                    yarp::os::Bottle &tilt_angle_btl = face_btl.addList();
                    tilt_angle_btl.addString("tilt_angle");
                    tilt_angle_btl.addFloat64(response.face_annotations( i ).tilt_angle());

                    yarp::os::Bottle &detection_confidence_btl = face_btl.addList();
                    detection_confidence_btl.addString("detection_confidence");
                    detection_confidence_btl.addFloat64(response.face_annotations( i ).detection_confidence());

                    yarp::os::Bottle &landmarking_confidence_btl = face_btl.addList();
                    landmarking_confidence_btl.addString("landmarking_confidence");
                    landmarking_confidence_btl.addFloat64(response.face_annotations( i ).landmarking_confidence());

                    yarp::os::Bottle &alt_info_btl = face_btl.addList();
                    alt_info_btl.addString("alt_info");

                    yarp::os::Bottle &alt_info_joy = alt_info_btl.addList();
                    alt_info_joy.addString("Joy");
                    alt_info_joy.addString(Likelihood_Name( response.face_annotations( i ).joy_likelihood() ));

                    yarp::os::Bottle &alt_info_sorrow = alt_info_btl.addList();
                    alt_info_sorrow.addString("Sorrow");
                    alt_info_sorrow.addString(Likelihood_Name( response.face_annotations( i ).sorrow_likelihood() ));

                    yarp::os::Bottle &alt_info_anger = alt_info_btl.addList();
                    alt_info_anger.addString("Anger");
                    alt_info_anger.addString(Likelihood_Name( response.face_annotations( i ).anger_likelihood() ));

                    yarp::os::Bottle &alt_info_surprise = alt_info_btl.addList();
                    alt_info_surprise.addString("Surprise");
                    alt_info_surprise.addString(Likelihood_Name( response.face_annotations( i ).surprise_likelihood() ));

                    yarp::os::Bottle &alt_info_under_exposed = alt_info_btl.addList();
                    alt_info_under_exposed.addString("Under_exposed");
                    alt_info_under_exposed.addString(Likelihood_Name( response.face_annotations( i ).under_exposed_likelihood() ));

                    yarp::os::Bottle &alt_info_blurred = alt_info_btl.addList();
                    alt_info_blurred.addString("Blurred");
                    alt_info_blurred.addString(Likelihood_Name( response.face_annotations( i ).blurred_likelihood() ));

                    yarp::os::Bottle &alt_info_headwear = alt_info_btl.addList();
                    alt_info_headwear.addString("Headwear");
                    alt_info_headwear.addString(Likelihood_Name( response.face_annotations( i ).headwear_likelihood() ));
                }            
            }

            // LABEL ANNOTATIONS : color = green
            if (response.label_annotations_size() > 0) {
                yarp::os::Bottle &label_annotation_btl = ext_btl.addList();
                label_annotation_btl.addString("label_annotation");
                
                for ( int i = 0; i < response.label_annotations_size(); i++ ) {

                    yarp::os::Bottle &label_btl = label_annotation_btl.addList();
                    label_btl.addString("label");
                    label_btl.addInt(i+1);

                    yarp::os::Bottle &label_description_btl = label_btl.addList();
                    label_description_btl.addString("description");
                    label_description_btl.addString(response.label_annotations( i ).description());

                    yarp::os::Bottle &label_score_btl = label_btl.addList();
                    label_score_btl.addString("score");
                    label_score_btl.addFloat64(response.label_annotations( i ).score() ); 

                    if ( response.label_annotations( i ).has_bounding_poly() ) {
                        yarp::os::Bottle &bounding_poly_btl_label = label_annotation_btl.addList();
                        bounding_poly_btl_label.addString("bounding_poly");

                        cv::Point tl, br;
                        for ( int j = 0; j < response.label_annotations( i ).bounding_poly().vertices_size(); j++ ) {
                            yarp::os::Bottle &bounding_poly_xy_label = bounding_poly_btl_label.addList();
                            bounding_poly_xy_label.addDouble(response.label_annotations( i ).bounding_poly().vertices( j ).x());
                            bounding_poly_xy_label.addDouble(response.label_annotations( i ).bounding_poly().vertices( j ).y());

                            if (j==0) //for j=0 we have the top-left point
                                tl = cv::Point (
                                   response.label_annotations( i ).bounding_poly().vertices( j ).x(), 
                                   response.label_annotations( i ).bounding_poly().vertices( j ).y()
                                );
                            else if (j==2) //for j=2 we have the bottom-right point
                                br = cv::Point (
                                   response.label_annotations( i ).bounding_poly().vertices( j ).x(), 
                                   response.label_annotations( i ).bounding_poly().vertices( j ).y()
                                );
                        }
                        cv::rectangle(input_cv, tl, br, cvScalar(0,255,0), 1, 8);

                        std::string text = response.label_annotations( i ).description();
                        cv::putText(input_cv, //target image
                                    text, //text
                                    cv::Point(tl.x + 5, tl.y + 8),
                                    cv::FONT_HERSHEY_SIMPLEX,
                                    0.3,
                                    CV_RGB(0, 255, 0), //font color green
                                    0.5);
                    }
                }
            }
            
            // LANDMARK ANNOTATIONS : color = magenta
            if (response.landmark_annotations_size() > 0) {
                yarp::os::Bottle &landmark_annotation_btl = ext_btl.addList();
                landmark_annotation_btl.addString("landmark_annotation");
                
                for ( int i = 0; i < response.landmark_annotations_size(); i++ ) {
                    yarp::os::Bottle &landmark_btl = landmark_annotation_btl.addList();
                    landmark_btl.addString("label");
                    landmark_btl.addInt(i+1);

                    yarp::os::Bottle &landmark_description_btl = landmark_btl.addList();
                    landmark_description_btl.addString("description");
                    landmark_description_btl.addString(response.landmark_annotations( i ).description());

                    yarp::os::Bottle &landmark_score_btl = landmark_btl.addList();
                    landmark_score_btl.addString("score");
                    landmark_score_btl.addFloat64(response.landmark_annotations( i ).score() );

                    if ( response.landmark_annotations( i ).has_bounding_poly() ) {
                        yarp::os::Bottle &bounding_poly_btl_landmark = landmark_btl.addList();
                        bounding_poly_btl_landmark.addString("bounding_poly");
                        cv::Point tl, br;
                        for ( int j = 0; j < response.landmark_annotations( i ).bounding_poly().vertices_size(); j++ ) {
                            yarp::os::Bottle &bounding_poly_xy_landmark = bounding_poly_btl_landmark.addList();
                            bounding_poly_xy_landmark.addDouble(response.landmark_annotations( i ).bounding_poly().vertices( j ).x());
                            bounding_poly_xy_landmark.addDouble(response.landmark_annotations( i ).bounding_poly().vertices( j ).y());
                            if (j==0) //for j=0 we have the top-left point
                                tl = cv::Point (
                                   response.landmark_annotations( i ).bounding_poly().vertices( j ).x(), 
                                   response.landmark_annotations( i ).bounding_poly().vertices( j ).y()
                                );
                            else if (j==2) //for j=2 we have the bottom-right point
                                br = cv::Point (
                                    response.landmark_annotations( i ).bounding_poly().vertices( j ).x(), 
                                    response.landmark_annotations( i ).bounding_poly().vertices( j ).y()
                                );
                        }
                        cv::rectangle(input_cv, tl, br, cvScalar(255,0,255), 1, 8);

                        std::string text = adjustAccents(response.landmark_annotations( i ).description());
                        cv::putText(input_cv, //target image
                                    text, //text
                                    cv::Point(tl.x + 5, tl.y + 8),
                                    cv::FONT_HERSHEY_SIMPLEX,
                                    0.3,
                                    CV_RGB(255,0,255), //font color
                                    0.5);
                    }
                }
            }

            // LOGO ANNOTATIONS : color = red
            if (response.logo_annotations_size() > 0) {
                yarp::os::Bottle &logo_annotation_btl = ext_btl.addList();
                logo_annotation_btl.addString("logo_annotation");
                
                for ( int i = 0; i < response.logo_annotations_size(); i++ ) {
                    yarp::os::Bottle &logo_btl = logo_annotation_btl.addList();
                    logo_btl.addString("label");
                    logo_btl.addInt(i+1);

                    yarp::os::Bottle &logo_description_btl = logo_btl.addList();
                    logo_description_btl.addString("description");
                    logo_description_btl.addString(response.logo_annotations( i ).description());

                    yarp::os::Bottle &logo_score_btl = logo_btl.addList();
                    logo_score_btl.addString("score");
                    logo_score_btl.addFloat64(response.logo_annotations( i ).score() );

                    if ( response.logo_annotations( i ).has_bounding_poly() ) {
                        yarp::os::Bottle &bounding_poly_btl_logo = logo_btl.addList();
                        bounding_poly_btl_logo.addString("bounding_poly");

                        cv::Point tl, br;
                        for ( int j = 0; j < response.logo_annotations( i ).bounding_poly().vertices_size(); j++ ) {
                            yarp::os::Bottle &bounding_poly_xy_logo = bounding_poly_btl_logo.addList();
                            bounding_poly_xy_logo.addDouble(response.logo_annotations( i ).bounding_poly().vertices( j ).x());
                            bounding_poly_xy_logo.addDouble(response.logo_annotations( i ).bounding_poly().vertices( j ).y());
                            if (j==0) //for j=0 we have the top-left point
                                tl = cv::Point (
                                   response.logo_annotations( i ).bounding_poly().vertices( j ).x(), 
                                   response.logo_annotations( i ).bounding_poly().vertices( j ).y()
                                );
                            else if (j==2) //for j=2 we have the bottom-right point
                                br = cv::Point (
                                    response.logo_annotations( i ).bounding_poly().vertices( j ).x(), 
                                    response.logo_annotations( i ).bounding_poly().vertices( j ).y()
                                );
                        }
                        cv::rectangle(input_cv, tl, br, cvScalar(0, 0, 255), 1, 8);

                        std::string text = response.logo_annotations( i ).description();
                        cv::putText(input_cv, //target image
                                    text, //text
                                    cv::Point(tl.x + 5, tl.y + 8),
                                    cv::FONT_HERSHEY_SIMPLEX,
                                    0.3,
                                    CV_RGB(255, 0, 0), //font color
                                    0.5);
                    }
                }
            }

            // TEXT ANNOTATIONS : color = blue
            if ( response.text_annotations_size() > 0 ) { 
                yarp::os::Bottle &text_annotation_btl = ext_btl.addList();
                text_annotation_btl.addString("text_annotation");
                
                for ( int i = 0; i < response.text_annotations_size(); i++ ) {
                    yarp::os::Bottle &text_btl = text_annotation_btl.addList();
                    text_btl.addString("label");
                    text_btl.addInt(i+1);

                    yarp::os::Bottle &text_language_btl = text_btl.addList();
                    text_language_btl.addString("language");
                    text_language_btl.addString(response.text_annotations( i ).locale());

                    yarp::os::Bottle &text_description_btl = text_btl.addList();
                    text_description_btl.addString("description");
                    text_description_btl.addString(response.text_annotations( i ).description());

                    if ( response.text_annotations( i ).has_bounding_poly() ) {
                        yarp::os::Bottle &bounding_poly_btl_text = text_btl.addList();
                        bounding_poly_btl_text.addString("bounding_poly");

                        cv::Point tl, br;
                        for ( int j = 0; j < response.text_annotations( i ).bounding_poly().vertices_size(); j++ ) {
                            yarp::os::Bottle &bounding_poly_xy_text = bounding_poly_btl_text.addList();
                            bounding_poly_xy_text.addDouble(response.text_annotations( i ).bounding_poly().vertices( j ).x());
                            bounding_poly_xy_text.addDouble(response.text_annotations( i ).bounding_poly().vertices( j ).y());
                            if (j==0) //for j=0 we have the top-left point
                                tl = cv::Point (
                                   response.text_annotations( i ).bounding_poly().vertices( j ).x(), 
                                   response.text_annotations( i ).bounding_poly().vertices( j ).y()
                                );
                            else if (j==2) //for j=2 we have the bottom-right point
                                br = cv::Point (
                                    response.text_annotations( i ).bounding_poly().vertices( j ).x(), 
                                    response.text_annotations( i ).bounding_poly().vertices( j ).y()
                                );
                        }
                        cv::rectangle(input_cv, tl, br, cvScalar(255, 0, 0), 1, 8);

                        if (response.text_annotations( i ).locale() != "") {
                            std::string text = "Language: " + response.text_annotations( i ).locale();

                            cv::putText(input_cv, //target image
                                    text, //text
                                    cv::Point(tl.x + 5, tl.y + 8),
                                    cv::FONT_HERSHEY_SIMPLEX,
                                    0.3,
                                    CV_RGB(0, 0, 255), //font color
                                    0.5);
                        }

                    }
                }
            }

            // SAFE SEARCH ANNOTATION
            if ( response.has_safe_search_annotation() ) {
                yarp::os::Bottle &text_annotation_btl = ext_btl.addList();
                text_annotation_btl.addString("safe_search_annotation");

                yarp::os::Bottle &safe_search_adult_btl = text_annotation_btl.addList();
                safe_search_adult_btl.addString("adult");
                safe_search_adult_btl.addString(Likelihood_Name(response.safe_search_annotation().adult()) );

                yarp::os::Bottle &safe_search_medical_btl = text_annotation_btl.addList();
                safe_search_medical_btl.addString("medical");
                safe_search_medical_btl.addString(Likelihood_Name(response.safe_search_annotation().medical()) );

                yarp::os::Bottle &safe_search_spoof_btl = text_annotation_btl.addList();
                safe_search_spoof_btl.addString("spoofed");
                safe_search_spoof_btl.addString(Likelihood_Name(response.safe_search_annotation().spoof()) );

                yarp::os::Bottle &safe_search_violence_btl = text_annotation_btl.addList();
                safe_search_violence_btl.addString("violence");
                safe_search_violence_btl.addString(Likelihood_Name(response.safe_search_annotation().violence()) );

                yarp::os::Bottle &safe_search_racy_btl = text_annotation_btl.addList();
                safe_search_racy_btl.addString("racy");
                safe_search_racy_btl.addString(Likelihood_Name(response.safe_search_annotation().racy()) );
            }

           
         }
       
        return result_btl;
   }


    /********************************************************/
    yarp::os::Bottle get_logo_annotation() {
        yarp::os::Bottle result;
        result.clear();
        yarp::os::Bottle element_btl = result_btl.get(0).asList()->findGroup("logo_annotation");

        if (element_btl.isNull()) { return result; }

        // skip the first element which is logo_annotation
        for (size_t i = 1; i < element_btl.size(); i++) {

            result.add(element_btl.get(i).asList()->get(0));   // label
            result.add(element_btl.get(i).asList()->get(1));   // #

            // skip the first two elements which are label #
            for (size_t j = 2; j < element_btl.get(i).asList()->size(); j++) {
                yarp::os::Bottle& value_btl = result.addList();
                value_btl.add(element_btl.get(i).asList()->get(j).asList()->get(0)); // description / score / bounding_poly 
                value_btl.add(element_btl.get(i).asList()->get(j).asList()->get(1)); // values
            }
        }
        return result;
    }

    /********************************************************/
    yarp::os::Bottle get_text_annotation() {
        yarp::os::Bottle result;
        result.clear();
        yarp::os::Bottle element_btl = result_btl.get(0).asList()->findGroup("text_annotation");

        if (element_btl.isNull()) { return result; }

        // skip the first element which is text_annotation
        for (size_t i = 1; i < element_btl.size(); i++) {

            result.add(element_btl.get(i).asList()->get(0));   // label
            result.add(element_btl.get(i).asList()->get(1));   // #

            // skip the first two elements which are label #
            for (size_t j = 2; j < element_btl.get(i).asList()->size(); j++) {
                yarp::os::Bottle& value_btl = result.addList();
                value_btl.add(element_btl.get(i).asList()->get(j).asList()->get(0)); // language / description / ... 
                value_btl.add(element_btl.get(i).asList()->get(j).asList()->get(1)); // values
            }
        }
        return result;
    }

    /********************************************************/
    yarp::os::Bottle get_safe_search_annotation() {
        yarp::os::Bottle result;
        result.clear();
        yarp::os::Bottle element_btl = result_btl.get(0).asList()->findGroup("safe_search_annotation");

        if (element_btl.isNull()) { return result; }

        // skip the first element which is safe_search_annotation
        for (size_t i = 1; i < element_btl.size(); i++) {
            /*
            std::cout << "Element: " 
                      << element_btl.get(i).asList()->get(0).toString().c_str() 
                      << " " 
                      << element_btl.get(i).asList()->get(1).toString().c_str()
                      << std::endl;
            */

            yarp::os::Bottle& value_btl = result.addList();
            value_btl.addString(element_btl.get(i).asList()->get(0).toString());
            value_btl.addString(element_btl.get(i).asList()->get(1).toString());
        }
        return result;
    }

    /********************************************************/
    yarp::os::Bottle get_face_features(const int32_t face_index) {

        yarp::os::Bottle face_features_result;
        face_features_result.clear();
        //std::cout << "result_btl " <<result_btl.toString().c_str() << std::endl; //((face_annotation (face 1 (...)...(...))(face 2 (...)...(...))) (label_annotation (label 1(...)...(...))(label 2(...)...(...)))(safe ....))
        yarp::os::Value val = result_btl.get(0);
        //std::cout << "val " << val.toString().c_str() << std::endl;//(face_annotation (face 1 (...)...(...)) (face 2 (...)...(...))) (label_annotation (label 1(...)...(...))(label 2(...)...(...))) (safe ....) 
        //size_t element = val.asList()->size();//3
        //std::cout << "element1 " <<  val.asList()->get(0).asList()->toString().c_str() << std::endl;//face_annotation (face 1 (...)...(...)) (face 2 (...)...(...))
        //std::cout << "element2 " <<  val.asList()->get(1).asList()->toString().c_str() << std::endl;//label_annotation (label 1(...)...(...))(label 2(...)...(...))
        //std::cout << "element3 " <<  val.asList()->get(2).asList()->toString().c_str() << std::endl;//(safe ....)

        yarp::os::Bottle face_annotation_found_btl = val.asList()->findGroup("face_annotation");
        //std::cout << "face_annotation_found_btl " << face_annotation_found_btl.toString().c_str() << std::endl;//face_annotation (face 1 (...)...(...)) (face 2 (...)...(...))
       if (!face_annotation_found_btl.isNull()){
	    size_t n_features = face_annotation_found_btl.get(face_index).asList()->size();//44
	    for( size_t i=2; i<n_features; i++) { //i starts from 2 to not take 'face 1'
	        yarp::os::Value face = face_annotation_found_btl.get(face_index);//face 1 (...)...(...), if face_index=1
	        face_features_result.add(face.asList()->get(i));//adding all the features related to a specific face 
	    }      
       }
       return face_features_result;
    }

    /********************************************************/
    yarp::os::Bottle get_face_annotation() {

        yarp::os::Bottle face_annotation_result;
        face_annotation_result.clear();
        yarp::os::Value val = result_btl.get(0);
        yarp::os::Bottle face_annotation_found_btl = val.asList()->findGroup("face_annotation");
        //std::cout << "face_annotation_found_btl " << face_annotation_found_btl.toString().c_str() << std::endl;//face_annotation (face 1 (...)...(...)) (face 2 (...)...(...))
       if (!face_annotation_found_btl.isNull()){
            size_t n_faces = face_annotation_found_btl.size();
	    for( size_t i=1; i<n_faces; i++) { //i starts from 1 to not take 'face_annotation'
	        yarp::os::Value each_face = face_annotation_found_btl.get(i);
	        face_annotation_result.add(each_face);
	    }     
       }

       return face_annotation_result;
    }


    /********************************************************/
    yarp::os::Bottle get_label_annotation() {

        yarp::os::Bottle label_annotation_result;
        label_annotation_result.clear();
        yarp::os::Value val = result_btl.get(0);
        yarp::os::Bottle label_annotation_found_btl = val.asList()->findGroup("label_annotation");
        //std::cout << "label_annotation_found_btl " << label_annotation_found_btl.toString() << std::endl;//label_annotation (label 1 (...)...(...)) (label 2 (...)...(...))
        if (!label_annotation_found_btl.isNull()){
            size_t n_labels = label_annotation_found_btl.size();
	    for( size_t i=1; i<n_labels; i++) { //i starts from 1 to not take 'label_annotation'
	        yarp::os::Value each_label = label_annotation_found_btl.get(i);
	        label_annotation_result.add(each_label);
	    }     
       }

       return label_annotation_result;
    }

    /********************************************************/
    yarp::os::Bottle get_landmark_annotation() {

        yarp::os::Bottle landmark_annotation_result;
        landmark_annotation_result.clear();
        yarp::os::Value val = result_btl.get(0);
        yarp::os::Bottle landmark_annotation_found_btl = val.asList()->findGroup("landmark_annotation");
        //std::cout << "landmark_annotation_found_btl " << landmark_annotation_found_btl.toString().c_str()  << std::endl;//landmark_annotation (label 1 (...)...(...)) (label 2 (...)...(...))
       if (!landmark_annotation_found_btl.isNull()){
            size_t n_landmarks = landmark_annotation_found_btl.size();
	    for( size_t i=1; i<n_landmarks; i++) { //i starts from 1 to not take 'landmark_annotation'
	        yarp::os::Value each_landmark = landmark_annotation_found_btl.get(i);
	        landmark_annotation_result.add(each_landmark);
	    }     
       }

       return landmark_annotation_result;
    }

    /********************************************************/
    bool annotate()
    {
        yarp::os::Bottle &outTargets = targetPort.prepare();
        yarp::sig::ImageOf<yarp::sig::PixelRgb> &outImage  = outPort.prepare();
        
        outTargets.clear();
        std::lock_guard<std::mutex> lg(mtx);
         
        //IF: load with stream of images
        cv::Mat input_cv = yarp::cv::toCvMat(annotate_img);
    
        // ELSE IF: image URI
        //cv::Mat input_cv = cv::imread("/projects/human-sensing/build/people.jpg", cv::IMREAD_COLOR);
        //ENDIF
        cv::Mat new_cv = input_cv.clone();
        outImage.resize(input_cv.cols, input_cv.rows);
        outTargets = queryGoogleVisionAI(new_cv);
        targetPort.write();

        outImage=yarp::cv::fromCvMat<yarp::sig::PixelRgb>(new_cv);
        outPort.write(); // In outPort we have a screenshot of the image input with the added bounding boxes
        yarp::os::Time::delay(0.05); // If we do not put a delay, as soon as we write into the port, a new input is coming, which will broke the output

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
    bool                        got_annotation;//true if annotate() has been executed

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
        
        yDebug() << "Module name" << moduleName;
        setName(moduleName.c_str());

        rpcPort.open(("/"+getName("/rpc")).c_str());

        closing = false;
        got_annotation = false;
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
        //processing->stop();

        if (processing) delete processing;
        std::cout << "The module is closed." << std::endl;
        rpcPort.close();
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
        got_annotation = true;
        return true;
    }

    /********************************************************/
    yarp::os::Bottle get_face_annotation()
    {   
        yarp::os::Bottle answer;
        if (got_annotation) {answer = processing->get_face_annotation();}      

	return answer;
    }
    
    /********************************************************/
    yarp::os::Bottle get_label_annotation()
    {   
        yarp::os::Bottle answer;
        if (got_annotation) {answer = processing->get_label_annotation();}
        
	return answer;
    }

    /********************************************************/
    yarp::os::Bottle get_landmark_annotation()
    {   
        yarp::os::Bottle answer;
        if (got_annotation) {answer = processing->get_landmark_annotation();}

	return answer;
    }

    /********************************************************/
    yarp::os::Bottle get_logo_annotation()
    {   
        yarp::os::Bottle answer;
        if (got_annotation) {answer = processing->get_logo_annotation();}

	return answer;
    }

    /********************************************************/
    yarp::os::Bottle get_text_annotation()
    {   
        yarp::os::Bottle answer;
        if (got_annotation) {answer = processing->get_text_annotation();}

	return answer;
    }

    /********************************************************/
    yarp::os::Bottle get_safe_search_annotation()
    {   
        yarp::os::Bottle answer;
        if (got_annotation) {answer = processing->get_safe_search_annotation();}

	return answer;
    }

    /********************************************************/
    yarp::os::Bottle get_face_features(const int32_t face_index)
    {   
        yarp::os::Bottle answer;
        if (got_annotation) {answer = processing->get_face_features(face_index);}

	return answer;
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
