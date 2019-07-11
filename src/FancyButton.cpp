/*
 * FancyButton.cpp
 *
 *  Created on: 26 May 2019
 *      Author: Leif
 */

#include "Arduino.h"
#include "FancyButton.h"

extern CFancyButton * fancybuttons[];
extern int iNumFancyButtons;

CFancyButton::CFancyButton(int group_id, int code, eFancyButtonMode mode, const char * ident, const char * special)
{
	thresh=1;
	timeout=300;

	this->group_id=group_id;
	this->code=code;
	this->mode=mode;
	this->ident=ident;

	if(mode==dual)
	{
		thresh=10;
	}

	if(special)
	{
		if(const char * temp=strstr(special,"thresh"))
		{
			thresh=atoi(temp+6+1);
		}

		if(const char * temp=strstr(special,"timeout"))
		{
			timeout=atoi(temp+7+1);
		}

	}


	fancybuttons[iNumFancyButtons]=this;
	iNumFancyButtons++;

}

CFancyButton::~CFancyButton()
{
}


void CFancyButton::runSingle()
{
	if(!bActive)
	{
		iCounter=0;
	}

	iCounter++;
	if(iCounter==thresh)
	{
		Dispatch(0);
	}


	bActive=true;
	ulActivity=millis();

}

void CFancyButton::mxSingle(unsigned long ulTimestamp)
{
	if(!bActive) return;
	if(ulTimestamp-ulActivity>(unsigned long) timeout)
	{
		bActive=false;
		iCounter=-1;
	}
}


void CFancyButton::runDual()
{
	if(!bActive)
	{
		iCounter=0;
		Dispatch(0);
	}

	iCounter++;
	if(iCounter==thresh)
	{
		Dispatch(1);
	}

	bActive=true;
	ulActivity=millis();
}



void CFancyButton::mxDual(unsigned long ulTimestamp)
{
	if(!bActive) return;
	if(ulTimestamp-ulActivity>300)
	{
		bActive=false;
		iCounter=-1;
	}
}


void CFancyButton::runRepeat()
{
	if(!bActive)
	{
		Dispatch(0);
		iCounter=0;
		ulRepeatTimestamp=millis();
	}

	bActive=true;
	ulActivity=millis();

	if(millis()-ulRepeatTimestamp>=300)
	{
		iCounter++;
		if(thresh<=1)
		{
			Dispatch(iCounter);
		}
		else
		{
			if(iCounter & 1)
			{
				Dispatch(iCounter/2);
			}
		}
	}

}

void CFancyButton::mxRepeat(unsigned long ulTimestamp)
{
	if(!bActive) return;
	if(ulTimestamp-ulActivity>300)
	{
		bActive=false;
		iCounter=-1;
	}
}



void CFancyButton::runTally()
{
	ulActivity=millis();

	if(!bActive)
	{
		bActive=true;
		bTallyHeld=false;
		iCounter=1;
		ulRepeatTimestamp=ulActivity;
		Dispatch(-iCounter);
		bAcceptTally=false;
	}


	if(bActive && bAcceptTally)
	{
		iCounter++;
		ulRepeatTimestamp=ulActivity;
		Dispatch(-iCounter);
		bAcceptTally=false;
	}
}

void CFancyButton::mxTally(unsigned long ulTimestamp)
{
	if(!bActive) return;
	if(ulTimestamp-ulActivity>=250)
	{
		bAcceptTally=true;
	}

	if(ulTimestamp-ulActivity>=(bTallyHeld?250:1000))
	{
		if(!bTallyHeld)
		{
			Dispatch(iCounter);
		}
		bTallyHeld=false;
		bActive=false;
	}

	if(bActive && ulTimestamp-ulRepeatTimestamp>=1000 && ulTimestamp-ulActivity<250 && !bTallyHeld)
	{
		Dispatch(0);
		bTallyHeld=true;
	}

}


void CFancyButton::Match()
{

	switch(mode)
	{
	case single: runSingle(); break;
	case repeat: runRepeat(); break;
	case tally: runTally(); break;
	case unfiltered: Dispatch(0); break;
	case dual: runDual(); break;
	};

}


void CFancyButton::Maintenance(unsigned long ulTimestamp)
{
	if(!bActive) return;

	switch(mode)
	{
	case single: mxSingle(ulTimestamp); break;
	case repeat: mxRepeat(ulTimestamp); break;
	case tally: mxTally(ulTimestamp); break;
	case unfiltered: break;
	case dual: mxDual(ulTimestamp); break;
	};
}




void CFancyButton::Dispatch(int iPayload)
{
	this->iPayload=iPayload;
	ButtonDispatch(this);
}

int CFancyButton::GetPayload()
{
	return iPayload;
}

String CFancyButton::MakePayloadString()
{
	switch(mode)
	{
	case dual:
		switch(iPayload)
		{
		case 0: return ident+"*push";
		case 1: return ident+"*long";
		}
		break;
	case repeat:
		if(iPayload)
		{
			char temp[32]; sprintf(temp,"*rep%i",iPayload);
			return ident+temp;
		}
		else
		{
			return ident+"*push";
		}
		break;
	case single:
		return ident+"*push";
		break;
	case tally:
		if(iPayload<0)
		{
			char temp[32]; sprintf(temp,"*count%i",-iPayload);
			return ident+temp;
		}
		else if(iPayload==0)
		{
			return ident+"*long";
		}
		else
		{
			char temp[32]; sprintf(temp,"*tally%i",iPayload);
			return ident+temp;
		}
		break;
	default:
		break;
	}

	return "";

}
