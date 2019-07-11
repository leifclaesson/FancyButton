#include <FancyButton.h>
#include <RCSwitch.h>

// FancyButton 433 MHz remote example.
// By Leif Claesson 2019-07-11.
//
// Enjoy!


//using group IDs lets us use a switch statement in the callback function, to guard against performance ever becoming an issue.
enum eGroupId
{
	eGroup_Kitchen,
	eGroup_Volume,
	eGroup_Station,
	eGroup_Handoff,
};


RCSwitch rcSwitch = RCSwitch();	//instantiate an RCSwitch object

void setup()
{
	Serial.begin(115200);

	Serial.printf("\n\nWelcome to this FancyButton example sketch!\n\n");
	Serial.printf("This sketch requires a 433.92 MHz receiver module and some 433.92 MHz remotes, and depends on the rc-switch library!\n");
	Serial.printf("It will of course also work with other RF frequencies (such as 330 MHz) as long as your receiver matches your remotes.\n");
	Serial.printf("\n");
	Serial.printf("Any unknown codes will be printed to this console so you can add more FancyButton objects or modify the existing ones.\n");

	rcSwitch.enableReceive(digitalPinToInterrupt(D2));  // 433 MHz receiver is connected to Pin D2.
	// Please note that the D0-D8 assignments are not the same on every ESP8266 board! This is an _endless_ source of confusion when working with the ESP8266. Just be aware.

}

CFancyButton * fancybuttons[100];	//the FancyButton constructor will add itself to this array. Make sure it's long enough for all your buttons!
int iNumFancyButtons=0;		//the FancyButton constructor increment this value.


void HandleReceivedCode(unsigned long code)
{
	/*	For performance reasons we will NOT iterate through the list.
	 *  We'll instead need to build a switch statement like the example below. This should scale well as we add more buttons.
	 *
	 *	switch(code)
	 * 	{
	 * 	case 100200:
	 * 		static CFancyButton btn100200;
	 * 		btn100200.Match();
	 * 		break;
	 *	case 12345:
	 * 		static CFancyButton btn12345;
	 * 		btn12345.Match();
	 * 		break;
	 *	case 16777216:
	 * 		static CFancyButton btn16777216;
	 * 		btn16777216.Match();
	 * 		break;
	 * 	}
	 *
	 *  The macros below will help us do just that!
	 *
	 *  BUTTON is to define a button with default settings.
	 *  BTNSPC adds a fifth parameter and is used to define a "special" button where we need a custom threshold or timeout.
	 *
	 *	The button codes are hard-coded from the factory on most current remote models from AliExpress.
	 *	I have yet to have a collision -- knock on wood.
	 *
	 */

#define BTNSPC(a,b,c,d,e) case b: static CFancyButton btn##b(a,b,c,d,e); btn##b.Match(); break;
#define BUTTON(a,b,c,d) BTNSPC(a,b,c,d,NULL);

	switch(code)
	{

		/* These buttons all have special handling code in the ButtonDispatch function below.
		 *
		 * The MCU could be sending HTTP requests directly to other devices.
		 *
		 * In this example we really just print stuff.
		 */

		BUTTON(eGroup_Kitchen,1456500,single,"WallSwitch1_A");
		BUTTON(eGroup_Kitchen,1456498,single,"WallSwitch1_B");
		BUTTON(eGroup_Kitchen,1456497,dual,"WallSwitch1_C");

		BUTTON(eGroup_Volume,1456452,repeat,"WallSwitch2_A");
		BUTTON(eGroup_Volume,1456450,repeat,"WallSwitch2_B");
		BUTTON(eGroup_Station,1456449,tally,"WallSwitch2_C");



		/* We'll put the following buttons in the "handoff" group.
		 * All of them will be handled by ONE common piece of code below.
		 * By calling CFancyButton::MakePayloadString() we'll get a string that includes the identifier and describes the event.
		 * This is ideal for publishing to an MQTT topic (or a Homie string property).
		 * Then we can do the rest of the decision-making in a system like openHAB.
		 * We could also send it by HTTP request to a web server somewhere! That's the beauty of MCUs. If you can think it and code it, it can do it.
		 */

		BTNSPC(eGroup_Handoff,2413762,single,"PoolPump","thresh=10 timeout=2000");	//Must hold button down to do anything
		BUTTON(eGroup_Handoff,2413768,dual,"DualButton");
		BUTTON(eGroup_Handoff,2413764,tally,"TallyButton");
		//We probably don't want to use a "repeat" mode button here because it would be a  large number of events.. but I suppose we _could_.



		break;
	default: //trap to catch our unknown codes, so we can add more handlers later.
		Serial.printf("UNKNOWN code: %li\n",code);
		break;
	}

}

// FancyButton will call this function when there's an event.
// It's not the most elegant way to make a callback, but this saves us from having to define a callback function every time we define a button.
// I haven't found it to be a limitation. You can always just add more group_id's and use a switch statement.

//Once we've divided and conquered by way of our switch(group_id) we can do string matching to match the exact button.
//It's inefficient to do naive string matching but there aren't many buttons in each group so we're fine.

void ButtonDispatch(CFancyButton * pSource)
{

	switch((eGroupId) pSource->group_id)
	{
	default:
		break;
	case eGroup_Kitchen:

		if(pSource->ident=="WallSwitch1_A")
		{
			static bool bSwitch1A=false;
			bSwitch1A ^= true;

			Serial.printf("WAX %s\n",bSwitch1A?"ON":"OFF");
		}

		if(pSource->ident=="WallSwitch1_B")
		{
			static bool bSwitch1B=false;
			bSwitch1B ^= true;

			Serial.printf("GET ON %s\n",bSwitch1B?"UP":"DOWN");
		}

		if(pSource->ident=="WallSwitch1_C")
		{
			static int iFanSpeed=0;

			if(pSource->GetPayload())	//this button was defined as "dual" above. Payload is 0 at first press, 1 when held.
			{
				iFanSpeed=0;
			}
			else
			{
				iFanSpeed++;
				iFanSpeed%=4;
			}

			const char * pszFanSpeed[]={"OFF","LOW","MID","HIGH"};

			Serial.printf("FAN %s\n",pszFanSpeed[iFanSpeed]);

		}


		break;
	case eGroup_Volume:
		{

			int iPayload=pSource->GetPayload();	//These buttons were defined as "repeat" above. Payload is 0 at first press and counts up as the button is held.

			int iStep=1;	//A single push means one click
			if(iPayload>=1) iStep=2;	//Accelerate as we hold the button down!
			if(iPayload>=10) iStep=3;

			static int iVolume=50;

			if(pSource->ident=="WallSwitch2_A")	// Volume DOWN
			{
				iVolume-=iStep;
				if(iVolume<0) iVolume=0;
			}

			if(pSource->ident=="WallSwitch2_B")	// Volume UP
			{
				iVolume+=iStep;
				if(iVolume>100) iVolume=100;
			}


			Serial.printf("VOLUME: %i\n",iVolume);
		}
		break;
	case eGroup_Station:	//there's only one button in this group, so we don't need to string match.
		{
			int iPayload=pSource->GetPayload();	/* And here is where we earn the name "Fancy". This button was defined as "tally" above.
												 *
												 * We count pushes to determine the action.
												 * Payload is negative and counts down while we're counting.
												 * When we stop pushing, the payload is positive and reflects the number of pushes.
												 * If we hold the button down, payload is zero.
												 *
												 */

			if(iPayload<0)
			{
				Serial.printf("DING! (acknowledge registered push)\n");
			}
			else if(iPayload>0)
			{
				switch(iPayload)
				{
				case 1: Serial.printf("Tune in NPR\n"); break;
				case 2: Serial.printf("Tune in BBC\n"); break;
				case 3: Serial.printf("Tune in Pulse FM\n"); break;
				case 4: Serial.printf("Tune in Echoes\n"); break;
				case 5: Serial.printf("Tune in Summer XL\n"); break;
				case 6: Serial.printf("Play the latest 99%% invisible\n"); break;
				case 7: Serial.printf("Play the latest Throughline\n"); break;
				case 8: Serial.printf("Play the latest Planet Money\n"); break;
				default:
					break;
				}
			}
			else
			{
				Serial.printf("STOP the player.\n");
			}

			//..this is how I use it. The only limitation is your imagination :).

		}
		break;
	case eGroup_Handoff:
		Serial.printf("Publish to MQTT: %s\n",pSource->MakePayloadString().c_str());
		break;
	};

}


unsigned long ulMaintenanceTimestamp=0;


void loop()
{

	if (rcSwitch.available())	//if we've received a code
	{
		unsigned long value=rcSwitch.getReceivedValue();	//save the code
		rcSwitch.resetAvailable();	//release the receiver to listen for a new code
		HandleReceivedCode(value);	//run our switch statement
	}


	if(millis()-ulMaintenanceTimestamp>=100)	//once every 100 milliseconds is plenty
	{
		ulMaintenanceTimestamp=millis();

		for(int i=0;i<iNumFancyButtons;i++)	//call the maintenance function on each FancyButton.
		{
			fancybuttons[i]->Maintenance(ulMaintenanceTimestamp);	//we include a cached timestamp so the maintenance function doesn't have to waste CPU cycles calling millis()
		}
	}
}


