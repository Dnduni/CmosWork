#include "ASICamera2.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
	using std::ofstream;
#include <limits>
#include <vector>


#define wait_time 500 //Minimum time in ms between consecutive acquisitions


//Workflow: 
//Get number of connected cameras 1
//Get info on camera via properties 2
//Open and initialise camera 3
//Get number and type of controls (ie. properties that can be changed) 4
//Set control values 5
//Set Resolution bin image type 5a
//Switch Camera mode 6
//Capture Image 7
//Save to buffer (binary of txt) matrix 8 
//Close camera 9




int main(){
	ofstream stream;
	//Step 1
	int connected_cam = ASIGetNumOfConnectedCameras(); //Checks # of cameras connected
	std::cout << "\n" << "connected:" << connected_cam << std::endl; //Prints number of connected cameras


	//Step 2
	ASI_CAMERA_INFO info; //Struct containing camera info
	//Camera info contains: Camera name (char),CameraID (int), MAX Height (int), MAX width (int) and other less useful variables (see documentation) 

	ASIGetCameraProperty(&info, 0); //Writes relevant camera information to the info vector via pointer
	std::cout << "Camera id:" << info.CameraID<<"\n" << "Max image Height:" << info.MaxHeight <<"\t" << "Max image Width:" << info.MaxWidth << std::endl ;

	//Step 3
	ASI_ERROR_CODE except; //Checks for errors when opening and initialising camera, will warn user if problems are detected
	except = ASIOpenCamera(info.CameraID); //Try Open
	except = ASIInitCamera(info.CameraID); //Try init
	if (except != 0 ){ //Warn
		std::cout << "Problem with connected camera, check connection" << std::endl;
		return 1;
	}
	else{ //Success

		std::cout << "Camera connection successful. \t Connected camera:" << info.CameraID << std::endl;
	}

	//Step 4 Get number and type of controls and list them
	int Ncontrols;
	ASIGetNumOfControls(info.CameraID, &Ncontrols); //Get number
	std::cout << "Number of available controls:" << Ncontrols << std::endl; //Print control number
	std::cout << "Would you like to list them? (0/1)" << std::endl; //Ask for list
	int response;
	std::cin >> response; //Wait for response
	std::cout << std::endl;
	//Control Caps is a struct contaning name, value etc..
	ASI_CONTROL_CAPS ControlCaps[Ncontrols]; //define vector of available controls
	int i;
	for(i = 0; i < Ncontrols; i++){
		except = ASIGetControlCaps(info.CameraID, i, &ControlCaps[i]); //Fill the vector with controls
	}

	switch(response){ //Error handling
		case 0:
			break;
		case 1:

				if (except != 0){
					std::cout << "Something went wrong, restart the program and check connection" << std::endl;
					return 1;
				}
				else{
					for (i = 0; i < Ncontrols; i++){
					std::cout << "Variable Number: \t" << i << " \t Variable: \t" << ControlCaps[i].Name << " \t Value: \t \t" << ControlCaps[i].DefaultValue << std::endl;
					}
					break;
				}
			};



	//Step 5 Set control Values
	//Ask user if variable is to be changed
	std::cout << "Would you like to set a Variable to a different value? (0/1)" << std::endl;
	response = 0;
	std::cin >> response;
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	std::cout << std::endl;
	




	while(response){
			std::cout << "Select Variable from 0 to \t" << Ncontrols-1 << std::endl; //Select variable to be changed

			int variable;
			long value;
			ASI_BOOL AutoAdjust = ASI_FALSE ; //Removes automatic setting for that variable
			std::cin >> variable;
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			std::cout << std::endl;
			std::cout << "Set Value" << std::endl;
			std::cin >> value;
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			std::cout << std::endl;



			//This is where the magic happens
			except = ASISetControlValue(info.CameraID, ControlCaps[i].ControlType, value, AutoAdjust); //this sets the value of the variable 

			long check;
			ASI_BOOL isTrue;
			//Checks if there were errors setting the value
			ASIGetControlValue(info.CameraID, ControlCaps[i].ControlType, &check, &isTrue);
			if(check == value && isTrue == ASI_FALSE){
				std::cout << "Value set successfully" << std::endl;
			}
			else{
				std::cout << "Value was not set successfully, check errors while typing" << std::endl;

			}

			std::cout << "Would you like to set another variable? (0/1)" << std::endl; //Ask again
			std::cin >> response;
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			std::cout << std::endl;


	}

	//Set Resolution

	int NativeResX = 0;
	int NativeResY = 0;
	ASI_IMG_TYPE NativeType; //init resolution variables
	int NativeBin;
	int flag_res = 0;
	int flag_bin = 0;
	ASIGetROIFormat(info.CameraID, &NativeResX, &NativeResY, &NativeBin, &NativeType); //Get status of resolution
	
	std::cout << "Resolution: \t" << NativeResX << "*" << NativeResY << "\t Binning: \t" << NativeBin << "\t Image Type: \t" << NativeType << std::endl; //print current values

	std::cout << "Would you like to change resolution? (0/1)" << std::endl; //Ask user to change resolution
	std::cin >> response;
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	std::cout << std::endl;
	//Set resolution
	int NewX = 0;
	int NewY = 0;
	int isin = 1;
	if(response){
		while(isin){
		std::cout << "Set new Width" << std::endl;
		std::cin >> NewX;
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		std::cout << "Set new Height" << std::endl;
		std::cin >> NewY;
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		int raise_error = (NewX*NewY)%1024; //Error handling
		isin = 0;
		if (raise_error!=0){
			std::cout << "Warning: resolution must be integer multiple of 1024" << std::endl; //Checks if resolution is supported by the camera
			isin = 1;
		}
		ASISetROIFormat(info.CameraID, NewX, NewY, NativeBin, NativeType); //Sets new resolution
		flag_res = 1; //flags changes to resolution
		}
	}
	//Set binning
	response = 0;
	std::cout << "Would you like to change binning? (0/1)" << std::endl; //Asks to change binning
	std::cin >> response;
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	std::cout << std::endl;
	if(response){
		int NewBin;
		std::cout << "Set new bin value" << std::endl;
		std::cin >> NewBin;
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		if(flag_res == 1){
			ASISetROIFormat(info.CameraID, NewX, NewY, NewBin, NativeType);
			flag_bin = 1; //flags changes to bin number
		}
		else{
			ASISetROIFormat(info.CameraID, NativeResX, NativeResY, NewBin, NativeType);
			flag_bin = 1;
		}
	}

	//Set Camera mode
	response = 0;
	ASI_CAMERA_MODE DefaultCameraMode;
	ASIGetCameraMode(info.CameraID, &DefaultCameraMode);
	std::cout << "Camera Mode: \t" << DefaultCameraMode << std::endl;
	std::cout << "Change Camera Mode? (0/1)" << std::endl; //Asks to change camera mode (pixel trigger treshold)
	std::cin >> response;
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	std::cout << std::endl;
	if(response){

		ASI_CAMERA_MODE CameraMode;
		int selection;
		std::cout << "Available Camera Modes:" << "\t Normal (default) 0" << "\t Soft Edge 1" << "\t" << "\t Rise Edge 2" << "\t Fall Edge 3" << "\t Soft Level 3" << "\t High Level 4" << "\t Low Level 5" << std::endl;
		std::cout << "Select a number from above" << std::endl;
		std::cin >> selection;
		
		switch(selection){
			case 0:
				CameraMode = ASI_MODE_NORMAL;
				ASISetCameraMode(info.CameraID, CameraMode); //Lists all available camera modes
			case 1:
				CameraMode = ASI_MODE_TRIG_SOFT_EDGE;
				ASISetCameraMode(info.CameraID, CameraMode);
			case 2:
				CameraMode = ASI_MODE_TRIG_RISE_EDGE;
				ASISetCameraMode(info.CameraID, CameraMode);
			case 3:
				CameraMode = ASI_MODE_TRIG_FALL_EDGE;
				ASISetCameraMode(info.CameraID, CameraMode);
			case 4:
				CameraMode = ASI_MODE_TRIG_SOFT_LEVEL;
				ASISetCameraMode(info.CameraID, CameraMode);
			case 5:
				CameraMode = ASI_MODE_TRIG_HIGH_LEVEL;
				ASISetCameraMode(info.CameraID, CameraMode);
			case 6:
				CameraMode = ASI_MODE_TRIG_LOW_LEVEL;
				ASISetCameraMode(info.CameraID, CameraMode);
		}

	}

//Capture image FINALLY
long int buffer_size = 0; //init buffer size




if(flag_res){
	buffer_size =NewX*NewY;
}

else{	//defines buffer size depending on resolution 

	buffer_size = NativeResX*NativeResY;
}
std::cout << buffer_size;
std::cout << std::endl;

int shutters; 
std::cout << "Select number of acquisitions" << std::endl; //Asks for number of photos to be snapped
std::cin >> shutters;
std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
std::cout << "Starting Video Capture" << std::endl;
int j;

ASIStartVideoCapture(info.CameraID);	//Initialise Video Capture mode


for(i = 0; i < shutters; i++){
	unsigned char * image = new unsigned char[buffer_size](); //Declare image buffer
	ASIGetVideoData(info.CameraID, image, buffer_size, wait_time); //Get video data
	std::stringstream ss;
	ss << "./output/image" << i << ".txt";
	std::string s = ss.str();
	stream.open(s, std::fstream::out);		
	if(!stream){
		std::cout << "Cannot open output file" << std::endl;
		}
	int j;
	for(j = 0; j < buffer_size ; j++){
		stream << (int)image[j];
		stream << ' ';
		if((j+1)%NativeResX == 0 && j > 1){		//Write to file txt according to resolution set
			stream << std::endl;
		}

	}
	stream.close();	//Release Stream

}
	
ASICloseCamera(info.CameraID); //Release camera





	return 0;
}

