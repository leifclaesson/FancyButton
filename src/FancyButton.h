/*
 * FancyButton.h
 *
 *  Created on: 26 May 2019
 *      Author: Leif
 */

#ifndef FANCYBUTTON_H_
#define FANCYBUTTON_H_

#include <WString.h>

class CFancyButton;

void ButtonDispatch(CFancyButton * pSource);

enum eFancyButtonMode
{
	unfiltered,
	single,
	repeat,
	tally,
	dual,
};

class CFancyButton
{
public:
	CFancyButton(int group_id, int code, eFancyButtonMode mode, const char * ident, const char * special);
	virtual ~CFancyButton();

	void Match();

	int GetPayload();
	String MakePayloadString();

	int group_id;
	int code;
	String ident;
	eFancyButtonMode mode;
	int thresh;
	int timeout;
	void Maintenance(unsigned long ulTimestamp);

private:

	void runSingle();
	void runRepeat();
	void runTally();
	void runDual();

	bool bActive=false;
	unsigned long ulActivity;

	void mxSingle(unsigned long ulTimestamp);
	void mxRepeat(unsigned long ulTimestamp);
	void mxTally(unsigned long ulTimestamp);
	void mxDual(unsigned long ulTimestamp);

	void Dispatch(int iPayload);

	int iCounter=0;
	unsigned long ulRepeatTimestamp;

	bool bAcceptTally=false;

	int iPayload;

	bool bTallyHeld=false;

};

#endif /* FANCYBUTTON_H_ */
