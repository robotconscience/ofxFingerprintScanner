/* 
	FPS_GT511C3.h v1.0 - Library for controlling the GT-511C3 Finger Print Scanner (FPS)
	Created by Josh Hawley, July 23rd 2013
	Licensed for non-commercial use, must include this license message
	basically, Feel free to hack away at it, but just give me credit for my work =)
	TLDR; Wil Wheaton's Law
 
    Modified by Brett Renfer 9/11/2014 for openFrameworks support!
*/

#include "ofxFPS_GT511C3.h"

#pragma region -= Command_Packet Definitions =-

// returns the 12 chars of the generated command packet
// remember to call delete on the returned array
byte* Command_Packet::GetPacketBytes()
{
	byte* packetchars= new byte[12];
	
	// update command before calculating checksum (important!)
	word cmd = Command;
	command[0] = GetLowByte(cmd);
	command[1] = GetHighByte(cmd);
    
	word checksum = _CalculateChecksum();

	packetchars[0] = COMMAND_START_CODE_1;
	packetchars[1] = COMMAND_START_CODE_2;
	packetchars[2] = COMMAND_DEVICE_ID_1;
	packetchars[3] = COMMAND_DEVICE_ID_2;
	packetchars[4] = Parameter[0];
	packetchars[5] = Parameter[1];
	packetchars[6] = Parameter[2];
	packetchars[7] = Parameter[3];
	packetchars[8] = command[0];
	packetchars[9] = command[1];
	packetchars[10] = GetLowByte(checksum);
	packetchars[11] = GetHighByte(checksum);

	return packetchars;
}

// Converts the int to chars and puts them into the paramter array
void Command_Packet::ParameterFromInt(int i)
{
	Parameter[0] = (i & 0x000000ff);
	Parameter[1] = (i & 0x0000ff00) >> 8;
	Parameter[2] = (i & 0x00ff0000) >> 16;
	Parameter[3] = (i & 0xff000000) >> 24;
}

// Returns the high byte from a unsigned int
byte Command_Packet::GetHighByte(word w)
{
    return (byte)(w>>8)&0x00FF;
}

// Returns the low byte from a unsigned int
byte Command_Packet::GetLowByte(word w)
{
    return (byte)w&0x00FF;
}

word Command_Packet::_CalculateChecksum()
{
	word w = 0;
	w += COMMAND_START_CODE_1;
	w += COMMAND_START_CODE_2;
	w += COMMAND_DEVICE_ID_1;
	w += COMMAND_DEVICE_ID_2;
	w += Parameter[0];
	w += Parameter[1];
	w += Parameter[2];
	w += Parameter[3];
	w += command[0];
	w += command[1];

	return w;
}

Command_Packet::Command_Packet()
{
};
#pragma endregion

Image_Packet::Image_Packet(byte* buffer){
    // be chill
    for ( int i=0; i<51840; i++){
        ImageData[i] = buffer[i + 4]; // bc of command offset
    }
}

#pragma region -= Response_Packet Definitions =-
// creates and parses a response packet from the finger print scanner
Response_Packet::Response_Packet(byte* buffer, bool UseSerialDebug)
{
	CheckParsing(buffer[0], COMMAND_START_CODE_1, COMMAND_START_CODE_1, "COMMAND_START_CODE_1", UseSerialDebug);
	CheckParsing(buffer[1], COMMAND_START_CODE_2, COMMAND_START_CODE_2, "COMMAND_START_CODE_2", UseSerialDebug);
	CheckParsing(buffer[2], COMMAND_DEVICE_ID_1, COMMAND_DEVICE_ID_1, "COMMAND_DEVICE_ID_1", UseSerialDebug);
	CheckParsing(buffer[3], COMMAND_DEVICE_ID_2, COMMAND_DEVICE_ID_2, "COMMAND_DEVICE_ID_2", UseSerialDebug);
	CheckParsing(buffer[8], 0x30, 0x31, "AckNak_LOW", UseSerialDebug);
	if (buffer[8] == 0x30) ACK = true; else ACK = false;
	CheckParsing(buffer[9], 0x00, 0x00, "AckNak_HIGH", UseSerialDebug);

	word checksum = CalculateChecksum(buffer, 10);
	byte checksum_low = GetLowByte(checksum);
	byte checksum_high = GetHighByte(checksum);
	CheckParsing(buffer[10], checksum_low, checksum_low, "Checksum_LOW", UseSerialDebug);
	CheckParsing(buffer[11], checksum_high, checksum_high, "Checksum_HIGH", UseSerialDebug);
	
	Error = ErrorCodes::ParseFromBytes(buffer[5], buffer[4]);

	ParameterBytes[0] = buffer[4];
	ParameterBytes[1] = buffer[5];
	ParameterBytes[2] = buffer[6];
	ParameterBytes[3] = buffer[7];
	ResponseBytes[0]=buffer[8];
	ResponseBytes[1]=buffer[9];
	for (int i=0; i < 12; i++)
	{
		RawBytes[i]=buffer[i];
	}
}

// parses chars into one of the possible errors from the finger print scanner
Response_Packet::ErrorCodes::Errors_Enum Response_Packet::ErrorCodes::ParseFromBytes(byte high, byte low)
{
	Errors_Enum e = INVALID;
	if (high == 0x00)
	{
	}
	if (high == 0x01)
	{
		switch(low)
		{
			case 0x00: e = NO_ERROR; break;
			case 0x01: e = NACK_TIMEOUT; break;
			case 0x02: e = NACK_INVALID_BAUDRATE; break;		
			case 0x03: e = NACK_INVALID_POS; break;			
			case 0x04: e = NACK_IS_NOT_USED; break;		
			case 0x05: e = NACK_IS_ALREADY_USED; break;
			case 0x06: e = NACK_COMM_ERR; break;
			case 0x07: e = NACK_VERIFY_FAILED; break;
			case 0x08: e = NACK_IDENTIFY_FAILED; break;
			case 0x09: e = NACK_DB_IS_FULL; break;
			case 0x0A: e = NACK_DB_IS_EMPTY; break;
			case 0x0B: e = NACK_TURN_ERR; break;
			case 0x0C: e = NACK_BAD_FINGER; break;
			case 0x0D: e = NACK_ENROLL_FAILED; break;
			case 0x0E: e = NACK_IS_NOT_SUPPORTED; break;
			case 0x0F: e = NACK_DEV_ERR; break;
			case 0x10: e = NACK_CAPTURE_CANCELED; break;
			case 0x11: e = NACK_INVALID_PARAM; break;
			case 0x12: e = NACK_FINGER_IS_NOT_PRESSED; break;
		}
	}
	return e;
}

// Gets an int from the parameter chars
int Response_Packet::IntFromParameter()
{
	int retval = 0;
	retval = (retval << 8) + ParameterBytes[3];
	retval = (retval << 8) + ParameterBytes[2];
	retval = (retval << 8) + ParameterBytes[1];
	retval = (retval << 8) + ParameterBytes[0];
	return retval;
}

// calculates the checksum from the chars in the packet
word Response_Packet::CalculateChecksum(byte* buffer, int length)
{
	word checksum = 0;
	for (int i=0; i<length; i++)
	{
		checksum +=buffer[i];
	}
	return checksum;
}

// Returns the high byte from a unsigned int
byte Response_Packet::GetHighByte(word w)
{
    return (byte)(w>>8)&0x00FF;
}

// Returns the low byte from a unsigned int
byte Response_Packet::GetLowByte(word w)
{
    return (byte)w&0x00FF;
}

// checks to see if the byte is the proper value, and logs it to the serial channel if not
bool Response_Packet::CheckParsing(byte b, byte propervalue, byte alternatevalue, string varname, bool UseSerialDebug)
{
	bool retval = (b != propervalue) && (b != alternatevalue);
	if ((UseSerialDebug) && (retval))
	{
		ofLogError()<<"Response_Packet parsing error "<<varname<<" "<<ofToHex(propervalue)<<" || "<<ofToHex(alternatevalue)<<" != "<<ofToHex(b);
	}
	
}
#pragma endregion

#pragma region -= Data_Packet =- 
//void Data_Packet::StartNewPacket()
//{
//	Data_Packet::NextPacketID = 0;
//	Data_Packet::CheckSum = 0;
//}
#pragma endregion

#pragma region -= ofxFPS_GT511C3 Definitions =-

#pragma region -= Constructor/Destructor =-
// Creates a new object to interface with the fingerprint scanner
ofxFPS_GT511C3::ofxFPS_GT511C3()
{
};

// destructor
ofxFPS_GT511C3::~ofxFPS_GT511C3()
{
    if ( _serial->isOpen() ){
        _serial->close();
    }
}
#pragma endregion

#pragma region -= Device Commands =-
//Initialises the device and gets ready for commands
bool ofxFPS_GT511C3::Open (std::string portName, int baudRate)
{
    this->UseSerialDebug = false;
    this->_portName = portName;
    
    // open serial port!
    bool bOpenSuccess = false;
    
    try {
        _serial = SharedSerial(new serial::Serial( portName,
                                               baudRate,
                                               serial::Timeout::simpleTimeout(1000),
                                               serial::eightbits,
                                               serial::parity_none,
                                               serial::stopbits_one,
                                               serial::flowcontrol_none ));
        bOpenSuccess = true;
    }
    
    catch (const std::exception& exc){
        ofLogError("ofxFPS_GT511C3:: Fail to open") << exc.what();
        bOpenSuccess = false;
        return bOpenSuccess;
    }
    _serial->flushInput();
    _serial->flushOutput();
    ofLogVerbose("ofxFPS_GT511C3", "FPS - Open");
    
    Command_Packet* cp = new Command_Packet();
    cp->Command = Command_Packet::Commands::Open;
    cp->Parameter[0] = 0x00;
    cp->Parameter[1] = 0x00;
    cp->Parameter[2] = 0x00;
    cp->Parameter[3] = 0x00;
    byte* packetchars = cp->GetPacketBytes();
    
    SendCommand(packetchars, 12);
    Response_Packet* rp = GetResponse();
    delete rp;
    delete packetchars;
    return bOpenSuccess;
}

// According to the DataSheet, this does nothing... 
// Implemented it for completeness.
void ofxFPS_GT511C3::Close()
{
	ofLogVerbose("ofxFPS_GT511C3", "FPS - Close");
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::Close;
	cp->Parameter[0] = 0x00;
	cp->Parameter[1] = 0x00;
	cp->Parameter[2] = 0x00;
	cp->Parameter[3] = 0x00;
	byte* packetchars = cp->GetPacketBytes();
	SendCommand(packetchars, 12);
	Response_Packet* rp = GetResponse();
	delete rp;
	delete packetchars;
    
    if ( _serial->isOpen() ){
        _serial->close();
    }
};

// Turns on or off the LED backlight
// Parameter: true turns on the backlight, false turns it off
// Returns: True if successful, false if not
bool ofxFPS_GT511C3::SetLED(bool on)
{
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::CmosLed;
	if (on)
	{
		ofLogVerbose("ofxFPS_GT511C3", "FPS - LED on");
		cp->Parameter[0] = 0x01;
	}
	else
	{
		ofLogVerbose("ofxFPS_GT511C3", "FPS - LED off");
		cp->Parameter[0] = 0x00;
	}
	cp->Parameter[1] = 0x00;
	cp->Parameter[2] = 0x00;
	cp->Parameter[3] = 0x00;
	byte* packetchars = cp->GetPacketBytes();
	SendCommand(packetchars, 12);
	Response_Packet* rp = GetResponse();
	bool retval = true;
	if (rp->ACK == false) retval = false;
	delete rp;
	delete packetchars;
	delete cp;
	return retval;
};

// Changes the baud rate of the connection
// Parameter: 9600, 19200, 38400, 57600, 115200
// Returns: True if success, false if invalid baud
// NOTE: Untested (don't have a logic level changer and a voltage divider is too slow)
bool ofxFPS_GT511C3::ChangeBaudRate(int baud)
{
	if ((baud == 9600) || (baud == 19200) || (baud == 38400) || (baud == 57600) || (baud == 115200))
	{
        if ( _serial->isOpen() && _serial->getBaudrate() != baud ){
            ofLogVerbose("ofxFPS_GT511C3", "Changing baudrate to " + ofToString(baud));
            Command_Packet* cp = new Command_Packet();
            cp->Command = Command_Packet::Commands::ChangeEBaudRate;
            cp->ParameterFromInt(baud);
            byte* packetchars = cp->GetPacketBytes();
            SendCommand(packetchars, 12);
            Response_Packet* rp = GetResponse();
            bool retval = rp->ACK;
            if (retval) 
            {
                //TODO: test
                _serial->flush();
                _serial->setBaudrate(baud);
                ofLogVerbose("ofxFPS_GT511C3") <<"baudrate changed. "<<_serial->getBaudrate()<<endl;

            }
            delete rp;
            delete packetchars;
            return retval;
        } else if (_serial->isOpen()){
            ofLogVerbose("ofxFPS_GT511C3", "Serial already at this baudrate");
            return true;
        }
    } else {
        ofLogWarning("ofxFPS_GT511C3", "Invalid baudrate");
    }
	return false;
}

// Gets the number of enrolled fingerprints
// Return: The total number of enrolled fingerprints
int ofxFPS_GT511C3::GetEnrollCount()
{
	ofLogVerbose("ofxFPS_GT511C3", "FPS - GetEnrolledCount");
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::GetEnrollCount;
	cp->Parameter[0] = 0x00;
	cp->Parameter[1] = 0x00;
	cp->Parameter[2] = 0x00;
	cp->Parameter[3] = 0x00;
	byte* packetchars = cp->GetPacketBytes();
	SendCommand(packetchars, 12);
	Response_Packet* rp = GetResponse();

	int retval = rp->IntFromParameter();
	delete rp;
	delete packetchars;
	return retval;
}

// checks to see if the ID number is in use or not
// Parameter: 0-199
// Return: True if the ID number is enrolled, false if not
bool ofxFPS_GT511C3::CheckEnrolled(int id)
{
	ofLogVerbose("ofxFPS_GT511C3", "FPS - CheckEnrolled");
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::CheckEnrolled;
	cp->ParameterFromInt(id);
	byte* packetchars = cp->GetPacketBytes();
	delete cp;
	SendCommand(packetchars, 12);
	delete packetchars;
	Response_Packet* rp = GetResponse();
	bool retval = false;
	retval = rp->ACK;
	delete rp;
	return retval;
}

// Starts the Enrollment Process
// Parameter: 0-199
// Return:
//	0 - ACK
//	1 - Database is full
//	2 - Invalid Position
//	3 - Position(ID) is already used
int ofxFPS_GT511C3::EnrollStart(int id)
{
	ofLogVerbose("ofxFPS_GT511C3", "FPS - EnrollStart");
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::EnrollStart;
	cp->ParameterFromInt(id);
	byte* packetchars = cp->GetPacketBytes();
	delete cp;
	SendCommand(packetchars, 12);
	delete packetchars;
	Response_Packet* rp = GetResponse();
	int retval = 0;
	if (rp->ACK == false)
	{
		if (rp->Error == Response_Packet::ErrorCodes::NACK_DB_IS_FULL) retval = 1;
		if (rp->Error == Response_Packet::ErrorCodes::NACK_INVALID_POS) retval = 2;
		if (rp->Error == Response_Packet::ErrorCodes::NACK_IS_ALREADY_USED) retval = 3;
	}
	delete rp;
	return retval;
}

// Gets the first scan of an enrollment
// Return: 
//	0 - ACK
//	1 - Enroll Failed
//	2 - Bad finger
//	3 - ID in use
int ofxFPS_GT511C3::Enroll1()
{
	ofLogVerbose("ofxFPS_GT511C3", "FPS - Enroll1");
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::Enroll1;
	byte* packetchars = cp->GetPacketBytes();
	delete cp;
	SendCommand(packetchars, 12);
	delete packetchars;
	Response_Packet* rp = GetResponse();
	int retval = rp->IntFromParameter();
	if (retval < 200) retval = 3; else retval = 0;
	if (rp->ACK == false)
	{
		if (rp->Error == Response_Packet::ErrorCodes::NACK_ENROLL_FAILED) retval = 1;
		if (rp->Error == Response_Packet::ErrorCodes::NACK_BAD_FINGER) retval = 2;
	}
	delete rp;
	if (rp->ACK) return 0; else return retval;
}

// Gets the Second scan of an enrollment
// Return: 
//	0 - ACK
//	1 - Enroll Failed
//	2 - Bad finger
//	3 - ID in use
int ofxFPS_GT511C3::Enroll2()
{
	ofLogVerbose("ofxFPS_GT511C3", "FPS - Enroll2");
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::Enroll2;
	byte* packetchars = cp->GetPacketBytes();
	delete cp;
	SendCommand(packetchars, 12);
	delete packetchars;
	Response_Packet* rp = GetResponse();
	int retval = rp->IntFromParameter();
	if (retval < 200) retval = 3; else retval = 0;
	if (rp->ACK == false)
	{
		if (rp->Error == Response_Packet::ErrorCodes::NACK_ENROLL_FAILED) retval = 1;
		if (rp->Error == Response_Packet::ErrorCodes::NACK_BAD_FINGER) retval = 2;
	}
	delete rp;
	if (rp->ACK) return 0; else return retval;
}

// Gets the Third scan of an enrollment
// Finishes Enrollment
// Return: 
//	0 - ACK
//	1 - Enroll Failed
//	2 - Bad finger
//	3 - ID in use
int ofxFPS_GT511C3::Enroll3()
{
	ofLogVerbose("ofxFPS_GT511C3", "FPS - Enroll3");
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::Enroll3;
	byte* packetchars = cp->GetPacketBytes();
	delete cp;
	SendCommand(packetchars, 12);
	delete packetchars;
	Response_Packet* rp = GetResponse();
	int retval = rp->IntFromParameter();
	if (retval < 200) retval = 3; else retval = 0;
	if (rp->ACK == false)
	{
		if (rp->Error == Response_Packet::ErrorCodes::NACK_ENROLL_FAILED) retval = 1;
		if (rp->Error == Response_Packet::ErrorCodes::NACK_BAD_FINGER) retval = 2;
	}
	delete rp;
	if (rp->ACK) return 0; else return retval;
}

// Checks to see if a finger is pressed on the FPS
// Return: true if finger pressed, false if not
bool ofxFPS_GT511C3::IsPressFinger()
{
	ofLogVerbose("ofxFPS_GT511C3", "FPS - IsPressFinger");
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::IsPressFinger;
	byte* packetchars = cp->GetPacketBytes();
	SendCommand(packetchars, 12);
	Response_Packet* rp = GetResponse();
	bool retval = false;
	int pval = rp->ParameterBytes[0];
	pval += rp->ParameterBytes[1];
	pval += rp->ParameterBytes[2];
	pval += rp->ParameterBytes[3];
    if (pval == 0) retval = true;
	delete rp;
	delete packetchars;
	delete cp;
	return retval;
}

// Deletes the specified ID (enrollment) from the database
// Parameter: 0-199 (id number to be deleted)
// Returns: true if successful, false if position invalid
bool ofxFPS_GT511C3::DeleteID(int id)
{
	ofLogVerbose("ofxFPS_GT511C3", "FPS - DeleteID");
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::DeleteID;
	cp->ParameterFromInt(id);
	byte* packetchars = cp->GetPacketBytes();
	SendCommand(packetchars, 12);
	Response_Packet* rp = GetResponse();
	bool retval = rp->ACK;
	delete rp;
	delete packetchars;
	delete cp;
	return retval;
}

// Deletes all IDs (enrollments) from the database
// Returns: true if successful, false if db is empty
bool ofxFPS_GT511C3::DeleteAll()
{
	ofLogVerbose("ofxFPS_GT511C3", "FPS - DeleteAll");
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::DeleteAll;
	byte* packetchars = cp->GetPacketBytes();
	SendCommand(packetchars, 12);
	Response_Packet* rp = GetResponse();
	bool retval = rp->ACK;
	delete rp;
	delete packetchars;
	delete cp;
	return retval;
}

// Checks the currently pressed finger against a specific ID
// Parameter: 0-199 (id number to be checked)
// Returns:
//	0 - Verified OK (the correct finger)
//	1 - Invalid Position
//	2 - ID is not in use
//	3 - Verified FALSE (not the correct finger)
int ofxFPS_GT511C3::Verify1_1(int id)
{
	ofLogVerbose("ofxFPS_GT511C3", "FPS - Verify1_1");
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::Verify1_1;
	cp->ParameterFromInt(id);
	byte* packetchars = cp->GetPacketBytes();
	SendCommand(packetchars, 12);
	Response_Packet* rp = GetResponse();
	int retval = 0;
	if (rp->ACK == false)
	{
		if (rp->Error == Response_Packet::ErrorCodes::NACK_INVALID_POS) retval = 1;
		if (rp->Error == Response_Packet::ErrorCodes::NACK_IS_NOT_USED) retval = 2;
		if (rp->Error == Response_Packet::ErrorCodes::NACK_VERIFY_FAILED) retval = 3;
	}
	delete rp;
	delete packetchars;
	delete cp;
	return retval;
}

// Checks the currently pressed finger against all enrolled fingerprints
// Returns:
//	0-199: Verified against the specified ID (found, and here is the ID number)
//	200: Failed to find the fingerprint in the database
int ofxFPS_GT511C3::Identify1_N()
{
	ofLogVerbose("ofxFPS_GT511C3", "FPS - Identify1_N");
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::Identify1_N;
	byte* packetchars = cp->GetPacketBytes();
	SendCommand(packetchars, 12);
	Response_Packet* rp = GetResponse();
	int retval = rp->IntFromParameter();
	if (retval > 200) retval = 200;
	delete rp;
	delete packetchars;
	delete cp;
	return retval;
}

// Captures the currently pressed finger into onboard ram use this prior to other commands
// Parameter: true for high quality image(slower), false for low quality image (faster)
// Generally, use high quality for enrollment, and low quality for verification/identification
// Returns: True if ok, false if no finger pressed
bool ofxFPS_GT511C3::CaptureFinger(bool highquality)
{
	ofLogVerbose("ofxFPS_GT511C3", "FPS - CaptureFinger");
	Command_Packet* cp = new Command_Packet();
	cp->Command = Command_Packet::Commands::CaptureFinger;
	if (highquality)
	{
		cp->ParameterFromInt(1);
	}
	else
	{
		cp->ParameterFromInt(0);
	}
	byte* packetchars = cp->GetPacketBytes();
	SendCommand(packetchars, 12);
	Response_Packet* rp = GetResponse();
	bool retval = rp->ACK;
	delete rp;
	delete packetchars;
	delete cp;
	return retval;

}
#pragma endregion

#pragma region -= Not imlemented commands =-
// Gets an image that is 258x202 (52116 chars)
Image_Packet* ofxFPS_GT511C3::GetImage()
{
    ofLogVerbose("ofxFPS_GT511C3", "FPS - Get Image");
    Command_Packet* cp = new Command_Packet();
    cp->Command = Command_Packet::Commands::GetImage;
    byte* packetchars = cp->GetPacketBytes();
    SendCommand(packetchars, 12);
    Image_Packet* rp = GetImageResponse();
    bool retval = false;
    if (rp == NULL)
    {
//        retval = true;
    }
    delete rp;
    delete packetchars;
    delete cp;
    return rp;
//	// Not implemented due to memory restrictions on the arduino
//	// may revisit this if I find a need for it
//	return false;
}

// Gets an image that is qvga 160x120 (19200 chars) and returns it
bool ofxFPS_GT511C3::GetRawImage()
{
	// Not implemented due to memory restrictions on the arduino
	// may revisit this if I find a need for it
	return false;
}

// Gets a template from the fps (498 chars) in 4 Data_Packets
// Use StartDataDownload, and then GetNextDataPacket until done
// Parameter: 0-199 ID number
// Returns: 
//	0 - ACK Download starting
//	1 - Invalid position
//	2 - ID not used (no template to download
	// Not implemented due to memory restrictions on the arduino
	// may revisit this if I find a need for it
//int ofxFPS_GT511C3::GetTemplate(int id)
//{
	// Not implemented due to memory restrictions on the arduino
	// may revisit this if I find a need for it
	//return false;
//}

// Uploads a template to the fps 
// Parameter: the template (498 chars)
// Parameter: the ID number to upload
// Parameter: Check for duplicate fingerprints already on fps
// Returns: 
//	0-199 - ID duplicated
//	200 - Uploaded ok (no duplicate if enabled)
//	201 - Invalid position
//	202 - Communications error
//	203 - Device error
	// Not implemented due to memory restrictions on the arduino
	// may revisit this if I find a need for it
//int ofxFPS_GT511C3::SetTemplate(byte* tmplt, int id, bool duplicateCheck)
//{
	// Not implemented due to memory restrictions on the arduino
	// may revisit this if I find a need for it
	//return -1;
//}

// resets the Data_Packet class, and gets ready to download
	// Not implemented due to memory restrictions on the arduino
	// may revisit this if I find a need for it
//void ofxFPS_GT511C3::StartDataDownload()
//{
	// Not implemented due to memory restrictions on the arduino
	// may revisit this if I find a need for it
//}

// Returns the next data packet 
	// Not implemented due to memory restrictions on the arduino
	// may revisit this if I find a need for it
//Data_Packet GetNextDataPacket()
//{
//    static Data_Packet dummy;
//	return dummy;
//}

// Commands that are not implemented (and why)
// VerifyTemplate1_1 - Couldn't find a good reason to implement this on an arduino
// IdentifyTemplate1_N - Couldn't find a good reason to implement this on an arduino
// MakeTemplate - Couldn't find a good reason to implement this on an arduino
// UsbInternalCheck - not implemented - Not valid config for arduino
// GetDatabaseStart - historical command, no longer supported
// GetDatabaseEnd - historical command, no longer supported
// UpgradeFirmware - Data sheet says not supported
// UpgradeISOCDImage - Data sheet says not supported
// SetIAPMode - for upgrading firmware (which data sheet says is not supported)
// Ack and Nack	are listed as commands for some unknown reason... not implemented
#pragma endregion


#pragma region -= Private Methods =-
// Sends the command to the software serial channel
void ofxFPS_GT511C3::SendCommand(byte* cmd, int length)
{
    ofLogVerbose("ofxFPS_GT511C3", "FPS - SEND: ")<<":"<<ofToString(cmd);
	int wrote = _serial->write(cmd, length);
    ofLogVerbose("WROTE ")<<wrote;
};

// Gets the response to the command from the software serial channel (and waits for it)
Response_Packet* ofxFPS_GT511C3::GetResponse()
{
	byte firstbyte = 0;
	bool done = false;
    
	while (done == false)
	{
        int nRead = _serial->read(&firstbyte, sizeof(byte));
		if (firstbyte == Response_Packet::COMMAND_START_CODE_1)
		{
			done = true;
        } else {
            cout << nRead << ":"<< (char) firstbyte <<":"<<(char)Response_Packet::COMMAND_START_CODE_1<< endl;
        }
	}
	byte* resp = new byte[12];
	resp[0] = firstbyte;
	for (int i=1; i < 12; i++)
	{
        while (_serial->available() == 0){
            // to-do: thread
            ofSleepMillis(10);
        }
		_serial->read(&resp[i],1);
	}
	Response_Packet* rp = new Response_Packet(resp, UseSerialDebug);
	delete resp;
	ofLogVerbose("ofxFPS_GT511C3", "FPS - RECV: ");
    ofLogVerbose()<<rp->RawBytes;
    
	return rp;
};

Image_Packet* ofxFPS_GT511C3::GetImageResponse(){
    byte firstbyte = 0;
    bool done = false;
    
    // get response packet first!
    Response_Packet * rp = GetResponse();
    if ( rp->ACK ){
        int cnt = 0;
        while (done == false)
        {
            int nRead = _serial->read(&firstbyte, sizeof(byte));
            if (firstbyte == Image_Packet::DATA_START_CODE_1)
            {
                done = true;
            } else {
                ofLogVerbose() << "got weird byte "<<nRead<<":"<<cnt++<<endl;
            }
        }
        byte* resp = new byte[51840 + 6];
        resp[0] = firstbyte;
        for (int i=1; i < 51840 + 6; i++)
        {
            while (_serial->available() == 0){
                // to-do: thread
                ofSleepMillis(10);
//                cout<<"sleep?"<<endl;
            }
            _serial->read(&resp[i],1);
//                    cout<<i<<endl;
        }
        Image_Packet* ip = new Image_Packet(resp);
    
        return ip;
    } else {
        return NULL;
    }
}

// sends the bye aray to the serial debugger in our hex format EX: "00 AF FF 10 00 13"
void ofxFPS_GT511C3::SendToSerial(byte data[], int length)
{
    bool first=true;
    _serial->write("\"");
    
    for(int i=0; i<length; i++){
        if (first) first=false;
        else _serial->write(" ");
        //serialPrintHex(data[i]);
    }
    _serial->write("\"");
}

// sends a byte to the serial debugger in the hex format we want EX "0F"
void ofxFPS_GT511C3::serialPrintHex(byte data)
{
//  char tmp[16];
//  sprintf(tmp, "%.2X",data); 
//  _serial->write(tmp, 16);
}
#pragma endregion

#pragma endregion

