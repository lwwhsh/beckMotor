/*
 * BeckDriver.h
 *
 *  Created on: Jun 17, 2016
 *      Author: davide.marcato@lnl.infn.it
 */

#ifndef MOTORAPP_BECKHOFFSRC_BECKDRIVER_H_
#define MOTORAPP_BECKHOFFSRC_BECKDRIVER_H_

#include "asynMotorController.h"
#include "asynMotorAxis.h"
#include <asynPortClient.h>



class asynInt32ClientSyncIO : public asynInt32Client {
private:
	epicsEvent event_;
	epicsInt32 value_;
	static void newvaluecb(void *userPvt, asynUser *pasynUser, epicsInt32 data) {
		asynInt32ClientSyncIO *obj=static_cast<asynInt32ClientSyncIO *>(userPvt);
		obj->value_ = data;
		obj->event_.signal();
	}

public:
	asynInt32ClientSyncIO(const char *portName, int addr, const char *drvInfo, double timeout=DEFAULT_TIMEOUT)
	    : asynInt32Client(portName, addr, drvInfo, timeout),
		  event_(epicsEventEmpty), value_(0)
	{
		registerInterruptUser(asynInt32ClientSyncIO::newvaluecb);
	}
	void readWait(epicsInt32 *ret){
		event_.wait();
		*ret = value_;
	}


};



class epicsShareClass BeckAxis : public asynMotorAxis {
public:
  /* These are the methods we override from the base class */
  BeckAxis(class BeckController *pC, int axis);

  void report(FILE *fp, int level);
  asynStatus move(double position, int relative, double min_velocity, double max_velocity, double acceleration);
//  asynStatus moveVelocity(double min_velocity, double max_velocity, double acceleration);
  asynStatus home(double min_velocity, double max_velocity, double acceleration, int forwards); //this may become goUp goDown
//  asynStatus stop(double acceleration);
  asynStatus poll(bool *moving); //pool and read infos from beckhoff
//  asynStatus setPosition(double position);
//  asynStatus setClosedLoop(bool closedLoop);

private:
  BeckController *pC_;          /**< Pointer to the asynMotorController to which this axis belongs.
                                   *   Abbreviated because it is used very frequently */

  //asynUsers representing the 6 registers of the KL2541
  asynInt32Client statusByte_;
  asynInt32ClientSyncIO dataIn_;
  asynInt32Client statusWord_;
  asynInt32Client controlByte_;
  asynInt32Client dataOut_;
  asynInt32Client controlWord_;

  //asynUsers to access the registers at bit level
  asynUser *statusByteBits_;
  asynUser *statusWordBits_;
  asynUser *controlByteBits_;
  asynUser *controlWordBits_;


  //an asynUser to trigger the modbusIo of the input registers
  asynInt32Client triggerRead_;

  //util methods
  asynStatus readPosition(double *position);
  asynStatus endMove();
//  asynStatus sendAccelAndVelocity(double accel, double velocity);

friend class BeckController;
};


class epicsShareClass BeckController : public asynMotorController {
protected:
  asynStatus move(double position, int relative, double minVelocity, double maxVelocity, double acceleration);
  asynStatus home(double min_velocity, double max_velocity, double acceleration, int forwards);
  char *inModbusPName_;
  char *outModbusPName_;


public:
  BeckController(const char *portName, const int numAxis, const char *inModbusPName, const char *outModbusPName, double movingPollPeriod, double idlePollPeriod );

  void report(FILE *fp, int level);
  BeckAxis* getAxis(asynUser *pasynUser);
  BeckAxis* getAxis(int axisNo);

  friend class BeckAxis;
};




#endif /* MOTORAPP_BECKHOFFSRC_BECKDRIVER_H_ */