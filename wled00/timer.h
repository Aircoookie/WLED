#ifndef __TIMER_H
#define __TIMER_H

class Timer {

public:

	Timer(unsigned long interval);

	unsigned long getInterval();
	void setOrigin(unsigned long origin);
	void setOriginToNow();
	void reset(unsigned long interval);
	float progress(bool constrain);
	unsigned long elapsed();
	void setEnabled(bool enabled);
	bool fire();

private:
	unsigned long _origin;
	unsigned long _interval;
	bool _enabled;

	unsigned long _millis();
};

#endif
