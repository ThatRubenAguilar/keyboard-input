// KeyboardInput.cpp 
// Author: Ruben Aguilar
// License: Software provided as is with no guarantees, you are free to use for whatever purposes you wish.

#include "MainInclude.h"
// Supports only ASCII input, Unicode will explode.
int main(int argc, char* argv[])
{
	// Use message
	if (argc == 1) {
		printf("KeyboardInput plays keyboard input back in the sequence of passed arguments.\n");
		printf("Arguments may be one of the following: \n");
		printf("A hex number representing the key code of the key to press or unpress, ex: 0xA4 \n");
		printf("or a decimal number prefixed by w with no spaces representing the wait time in ms at that point in the sequence, ex: w150 \n");
		
		printf("------------------------------------------------------------------------------\n");
		printf("Keys must be pressed and unpressed, the sequence 0x44 w150 0x45 0x45 w150 0x44 \n");
		printf("Will press key 0x44, wait 150ms, press 0x45, unpress 0x45, wait 150ms, and unpress 0x44 \n");
		return 1;
	}

	// Init variables
	int InputPress[0xFF+1];
	for (int x = 0; x < 0xFF+1; x++)
		InputPress[x] = 0;

	std::vector<int> IndexTime = std::vector<int>();
	std::vector<int> WaitTime = std::vector<int>();
	std::vector<INPUT> InputSequence = std::vector<INPUT>();

	for (int i = 1; i < argc; i++) {
		if (strlen(argv[i]) > 0 && argv[i][0] != '\0') {
			// Add to wait time
			if (argv[i][0] == 'w') {
				long msWaitTime = strtol (&argv[i][1], NULL, 10);
				if (msWaitTime < 0)
					msWaitTime *= -1;
				if (msWaitTime != 0 ) {
					WaitTime.push_back(msWaitTime);
					IndexTime.push_back(InputSequence.size());
				}
			}
			// Parse key
			else {
				INPUT KeyEvent = { 0 };
				long VKey = strtol (argv[i], NULL, 16);
				if (VKey != 0) {
					VKey = VKey & 0xFF;
					// Alternate Up Down Presses
					if (InputPress[VKey] % 2 == 0)
						KeyEvent.ki.dwFlags = 0;
					else 
						KeyEvent.ki.dwFlags = KEYEVENTF_KEYUP;

					KeyEvent.type = INPUT_KEYBOARD;
					KeyEvent.ki.wVk=VKey;
					InputPress[VKey]++;
					InputSequence.push_back(KeyEvent);
				}
			}
		}
	}

	// Push INPUTs/waits to send input in order
	int WaitIndex = 0;
	for (int InputIndex = 0; InputIndex < InputSequence.size(); ) {
		while (WaitIndex < IndexTime.size() && IndexTime[WaitIndex] == InputIndex) {
			Sleep(WaitTime[WaitIndex]);
			WaitIndex++;
		}
		// Create Delta to skip to next WaitInputIndex, or the End of the sequence.
		int WaitInputIndex = WaitIndex < IndexTime.size() ? IndexTime[WaitIndex] : InputSequence.size();

		SendInput(WaitInputIndex-InputIndex, &InputSequence[InputIndex], sizeof(INPUT));
		InputIndex+=(WaitInputIndex-InputIndex);
	}
	
	// Grab final wait statements
	while (WaitIndex < IndexTime.size() && IndexTime[WaitIndex] == InputSequence.size()) {
		Sleep(WaitTime[WaitIndex]);
		WaitIndex++;
	}

	// Display Errors
	HRESULT hresult = HRESULT_FROM_WIN32(GetLastError());
	if (FAILED(hresult)) {
	LPTSTR errorText = NULL;
	FormatMessage(
	   // use system message tables to retrieve error text
	   FORMAT_MESSAGE_FROM_SYSTEM
	   // allocate buffer on local heap for error text
	   |FORMAT_MESSAGE_ALLOCATE_BUFFER
	   // Important! will fail otherwise, since we're not 
	   // (and CANNOT) pass insertion parameters
	   |FORMAT_MESSAGE_IGNORE_INSERTS,  
	   NULL,    // unused with FORMAT_MESSAGE_FROM_SYSTEM
	   hresult,
	   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
	   (LPTSTR)&errorText,  // output 
	   0, // minimum size for output buffer
	   NULL);   // arguments - see note 

		if ( NULL != errorText )
		{
		   printf("ERROR: %s", errorText);

		   // release memory allocated by FormatMessage()
		   LocalFree(errorText);
		   errorText = NULL;
		} 
	}

	return 0;
}

