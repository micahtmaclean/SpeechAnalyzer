#include "stddsp.h"
#include <assert.h>
#include "epksrc.h"

EnergyPeakSource::EnergyPeakSource(PeakSource & ps)
  : myPS(ps), myQ(0)
{
}

EnergyPeakSource::~EnergyPeakSource()
{
  delete myQ;
  myQ = 0;
}

void
EnergyPeakSource::Search(const float * start, const float * end)
{   
  delete myQ;
  myQ = new pq_pair_float_float;
  pair_float_float point;
    
  for(myPS.Search(start, end); !myPS.IsDone(); myPS.Next())
  {
    // Note that the points from the source 
    // are (location, energy) but the points in the power queue
    // must be (energy, location),
    // so the .first is y and the .second is x so to speak.
    // Pull another peak, push it on the queue.
    myPS.Get(point.second, point.first);
    myQ->push(point);      
  }
}

void
EnergyPeakSource::Next()
{
  assert(!IsDone());
  myQ->pop();    
}

void
EnergyPeakSource::Get(float & location, float & value) const
{
  assert(!IsDone());
  // Remember, .first is y and .second is x.
  location = myQ->top().second;
  value = myQ->top().first;
}

int32
EnergyPeakSource::IsDone() const
{
  return myQ->empty();
}