/**
 * @file proj4.cc
 * \mainpage
 *
 * CISC 2200, Data Structures<br>
 * Project 4: The Banking Simulation <p>
 * A project where we simulate a bank queue, while tracking how long
 * the wait time for each customer is.
 * 
 * @author Justin Chan
 * @date 18 April 2016
 */
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <queue>

using namespace std;

/**
 * This holds the information needed for an arrival or departure
 * event.
 */
struct Event
{
    enum EventKind{arrival,departure};
    EventKind type;
    int time,length;

    Event(EventKind theType = arrival,int theTime = 0,int theLength = 0):
	type(theType),
	time(theTime),
	length(theLength)
	{}
};

/**
 * holds the arrival events that haven't arrived at the bank yet
 */
struct EventList
{
    void fill (istream& is);
    void simulate();
};

using EventPqType = priority_queue<Event>;
using BankQType = queue<Event>;

BankQType arrivalQ;
BankQType departureQ;
EventPqType finalQueue;

bool operator<(const Event &e0,const Event &e1);
istream& operator>>(istream &is, Event &e);
void processArrival(const Event &e,BankQType &arrivalQ,
		    BankQType &departureQ,bool &tellerAvailable);
void processDeparture(const Event &e,BankQType &arrivalQ,
		     BankQType &departureQ,bool &tellerAvailable);

int main(int argc, char** argv)
{
    EventList eventlist;
    
    char* progname = argv[0];

    switch (argc)
    {
    case 1:
	eventlist.fill(cin);
	break;
    case 2:
    {
	ifstream ifs(argv[1]);
	if(!ifs)
	{
	    cerr << progname << ": couldn't open " << argv[1] << endl;
	    return 1;
	}
	eventlist.fill(ifs);
	break;
    }
    default:
	cerr << "Usage: " << progname << " [datafile]\n";
	return 2;
    }
    eventlist.simulate();
    return 0;
}

/**
 * fill the arrival event list from the input stream
 * @param is the input streamm
 * @post the arrival event list has been filled
 */
void EventList::fill(istream& is)
{    
    char c[4];

    while(is.getline(c,5))
    {
	Event arrivalE;
	int arrival,waitTime;

	if(isdigit(c[1]))
	{
	    arrival = (((int)c[0] - 48) * 10) + ((int)c[1] - 48);
	    waitTime = ((int)c[3] - 48);
	}
	else
	{
	    arrival = ((int)c[0] - 48);
	    waitTime = ((int)c[2] - 48);
	}
	arrivalE.type = Event::arrival;
	arrivalE.time = arrival;
	arrivalE.length = waitTime;
	arrivalQ.push(arrivalE);
    }
}

/**
 * actually does the bank simulation
 * @post all customers have been processed
 */
void EventList::simulate()
{
    bool tellerAvailable = true;
    int customerNum = 1;
    double avgWaitTime;
    while(true)
    {
	Event departureE;
	Event bqFront = arrivalQ.front();

	arrivalQ.pop();
	//start filling the departure queue
	departureE.type = Event::departure;
	if(arrivalQ.empty() || departureQ.empty() ||
	   ((departureQ.back()).time + bqFront.length) > bqFront.time)
	{
	    if(!departureQ.empty())
		departureE.time = (departureQ.back()).time + bqFront.length;
	    else
		departureE.time = bqFront.time + bqFront.length;
	}
	else 
	    departureE.time = (departureQ.back()).time + bqFront.length;
	departureE.length = 0;
	departureQ.push(departureE);

	if(arrivalQ.empty())
	{
	    while(!departureQ.empty())
		processDeparture(departureQ.front(),arrivalQ,departureQ,tellerAvailable);
	    finalQueue.push(bqFront);
	    break;

	}
	if(customerNum != 1)
	{
	    if(operator<(bqFront,departureQ.front()))
		tellerAvailable = false; //must process departure first
	    if(operator<(bqFront,arrivalQ.front()))
		if(!arrivalQ.empty())
		{
		    cerr << "customer #" << customerNum+1 << " out of order (time = "
			 << (arrivalQ.front()).time << ", previous = " << bqFront.time << ")\n";
		    exit(0);
		}
	}

	processArrival(bqFront,arrivalQ,departureQ,tellerAvailable);
	
	customerNum++; //keep track of customer number for avgWaitTime
    }
    //start output
    while(!finalQueue.empty())
    {
	string eventType;
	Event e = finalQueue.top();
	finalQueue.pop();
	avgWaitTime += e.length;

	if(e.time == (finalQueue.top()).time)
	{
	    cout << "Processing an arrival event at time:    "
		 << setw(2) << (finalQueue.top()).time << endl;
	    cout << "Processing a departure event at time:   "
		 << setw(2) << e.time << endl;
	    avgWaitTime += (finalQueue.top()).length;
	    finalQueue.pop();
	}
	else
	{
	    if(e.type == Event::arrival)
		cout << "Processing an arrival event at time:    "
		     << setw(2) << e.time << endl;
	    if(e.type == Event::departure)
		cout << "Processing a departure event at time:   "
		     << setw(2) << e.time << endl;
	    if(finalQueue.size() == 1)
	    {
		e = finalQueue.top();
		eventType = "departure"; //final event will always be departure
		cout << "Processing a departure event at time:   "
		     << setw(2) << e.time << endl;
		finalQueue.pop();
	    }
	}
    }
    
    avgWaitTime = avgWaitTime / customerNum;	    
    
    cout << "\nFinal statistics:\n";
    cout << "  Total number of people processed:     " << customerNum << endl;
    cout << "  Average amount of time spent waiting: " << avgWaitTime << endl;
}

/**
 * Less-than operator for Events
 * @param e0 an event
 * @param e1 an event
 * @return true iff arrival time for e0 is earlier than e1
 */
bool operator< (const Event &e0,const Event &e1)
{
    if(e0.time < e1.time)
	return false;
    if(e0.time > e1.time)
	return true;
    else
	if(e0.type == Event::arrival && e1.type == Event::departure)
	    return true;
    return 0; //just in case
}

/**
 * Process an arrival event
 * @param e the event
 * @param epq the event list
 * @param bq the queue of customers who are actually in line
 * @param tellerAvailable is teller available?
 */
void processArrival(const Event &e,BankQType &arrivalQ,
		    BankQType &departureQ,bool &tellerAvailable)
{
    if(!tellerAvailable)
	processDeparture(e,arrivalQ,departureQ,tellerAvailable);
    finalQueue.push(e);
    if((departureQ.front()).time < (arrivalQ.front()).time)
	processDeparture(arrivalQ.front(),arrivalQ,departureQ,tellerAvailable);
}

/**
 * Process a departure event
 * @param e the event
 * @param epq the event list
 * @param bq the queue of customers who are actually in line
 * @param tellerAvailable is teller available?
 */
void processDeparture(const Event &e,BankQType &arrivalQ,
		      BankQType &departureQ,bool &tellerAvailable)
{
    tellerAvailable = true;
    while((departureQ.front()).time <= e.time)
    {
	finalQueue.push(departureQ.front());
	departureQ.pop();
	if((departureQ.front()).time == 0)
	    break;
    }
}
