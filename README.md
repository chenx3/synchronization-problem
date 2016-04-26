# synchronization-problem
OS homework
Algorithm:

children: 
acquire boat 

if child is on Oahu
   if boat isn't on Oahu or boat is full or just one child on Oahu
      waitingOnOahu goes to sleep
   wake up all the persons on Oahu
   if only one child left in Oahu, this guy will pilot to Molokai directly
   if more than one child on Oahu
      this child gets on boat. 
      if there are two children on boat
         notify the first guy to row to Molokai via waitingOboatFull.wake()
         this guy will pilot to Molokai
         two children arrives on Molokai, set location and boat's place accordingly
         speak number of people on Molokai to main thread via communicator
         wake all the people on Molokai
         this guy will go to sleep via waitingOnMolokai
      else
         only one child on boat, wait for next child coming via waitingOnBoatFull
else   
  the child is on Molokai now      
  wake one child to bring boat back to Oahu
  the child arrives on Oahu
  wake up all people on Oahu
  the child goes to sleep via waitingOnOahu

release the boat lock


adult:

acuire boat lock

if this adult is on Oahu
   if boat is full or more than one child on Oahu or boat is not on Oahu
      go to sleep via waitingOnOahu

   this adult row to Molokai
   speak number of people on Molokai to main thread via communicator
   wake all the people on Molokai
   go to sleep via waitingOnMolokai
else if this adult is on Molokai
   go to sleep via waitingOnMolokai

release boat lock
