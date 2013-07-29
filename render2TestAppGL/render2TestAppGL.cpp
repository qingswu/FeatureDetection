// For memory leak debugging: http://msdn.microsoft.com/en-us/library/x98tx3cf(v=VS.100).aspx
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>

#ifdef WIN32
	#include <crtdbg.h>
#endif

#ifdef _DEBUG
   #ifndef DBG_NEW
	  #define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
	  #define new DBG_NEW
   #endif
#endif  // _DEBUG

#include "render/MorphableModel.hpp"
#include "render/Renderer.hpp"
#include "render/SoftwareDevice.hpp"
#include "render/OpenGlDevice.hpp"
#include "render/MeshUtils.hpp"

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#ifdef WIN32
	#define BOOST_ALL_DYN_LINK	// Link against the dynamic boost lib. Seems to be necessary because we use /MD, i.e. link to the dynamic CRT.
	#define BOOST_ALL_NO_LIB	// Don't use the automatic library linking by boost with VS2010 (#pragma ...). Instead, we specify everything in cmake.
#endif
#include "boost/program_options.hpp"

#include <iostream>
#include <fstream>


namespace po = boost::program_options;
using namespace std;
using namespace cv;

template<class T>
ostream& operator<<(ostream& os, const vector<T>& v)
{
	copy(v.begin(), v.end(), ostream_iterator<T>(cout, " ")); 
	return os;
}

int main(int argc, char *argv[])
{
	#ifdef WIN32
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF ); // dump leaks at return
	//_CrtSetBreakAlloc(3759128);
	#endif

	std::string filename; // Create vector to hold the filenames
	
	try {
		po::options_description desc("Allowed options");
		desc.add_options()
			("help,h", "produce help message")
			("input-file,i", po::value<string>(), "input image")
		;

		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).
				  options(desc).run(), vm);
		po::notify(vm);
	
		if (vm.count("help")) {
			cout << "[renderTestApp] Usage: options_description [options]\n";
			cout << desc;
			return 0;
		}
		if (vm.count("input-file"))
		{
			cout << "[renderTestApp] Using input images: " << vm["input-file"].as< vector<string> >() << "\n";
			filename = vm["input-file"].as<string>();
		}
	}
	catch(std::exception& e) {
		cout << e.what() << "\n";
		return 1;
	}

#pragma comment(lib, "opengl32")
#pragma comment(lib, "glu32")

	//render::MorphableModel mmHeadL4 = render::utils::MeshUtils::readFromScm("C:\\Users\\Patrik\\Cloud\\PhD\\MorphModel\\ShpVtxModelBin.scm");

	render::Mesh cube = render::utils::MeshUtils::createCube();
	render::Mesh pyramid = render::utils::MeshUtils::createPyramid();
	render::Mesh plane = render::utils::MeshUtils::createPlane();

	cv::VideoCapture webcam(CV_CAP_ANY);

	const float& aspect = 640.0f/480.0f;
	shared_ptr<render::RenderDevice> swr = make_shared<render::OpenGlDevice>(640, 480);
	render::Renderer r(swr);
	r.renderDevice->getCamera().setFrustum(-1.0f*aspect, 1.0f*aspect, 1.0f, -1.0f, 0.1f, 100.0f);
	Vec4f test(0.5f, 0.5f, 0.5f, 1.0f);
	cv::Mat bg = imread("C:/Users/Patrik/Cloud/may_prag_vs_basel.png");
	resize(bg, bg, Size(200, 150));
	//r.renderDevice->setBackgroundImage(bg);
	r.renderDevice->renderVertex(test);
	Mat frame;
	while(cv::waitKey(30) < 0)
	{
		webcam >> frame;
		//r.renderDevice->setBackgroundImage(frame);
		//r.renderDevice->renderMesh(plane);
		//r.renderDevice->renderMesh(pyramid);
		r.renderDevice->updateWindow(); // move that to setBackgroundImage(), private (if possible / opengl is already ready then)
	}

	return 0;
}

// TODO: All those vec3 * mat4 cases... Do I have to add the homogeneous coordinate and make it vec4 * mat4, instead of how I'm doing it now? Difference?